/*****************************************************************
* Copyright (c) $Date$ Qualcomm Innovation Center, Inc.
* All Rights Reserved.
* Modified by Qualcomm Innovation Center, Inc. on $Date$
*****************************************************************/

/* QDSP6-specific support for 32-bit ELF
   Copyright 1994, 1995, 1997, 1999, 2001, 2002
   Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/qdsp6.h"
#include "libiberty.h"
#include "opcode/qdsp6.h"
#include <assert.h>

#define SDA_BASE "_SDA_BASE_"
/* Early alias for _SDA_BASE_. */
#define DEFAULT_SDA_BASE "__default_sda_base__"

#define get_section_size_now(abfd,sec) \
    bfd_section_size (abfd,sec)

#define QDSP6_TRAMPOLINE_PREFIX     ".PAD"
#define QDSP6_TRAMPOLINE_PREFIX_LEN (sizeof (QDSP6_TRAMPOLINE_PREFIX))
#define QDSP6_TRAMPLINE_NEEDED(D, B) \
  (abs (D) \
   > (~(~(bfd_signed_vma) 0 << ((B) - 1)) & -(MAX_PACKET_INSNS * QDSP6_INSN_LEN)))

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER ""

/* If ELIMINATE_COPY_RELOCS is non-zero, the linker will try to avoid
   copying dynamic variables from a shared lib into an app's dynbss
   section, and instead use a dynamic relocation to point into the
   shared lib.  */
#define ELIMINATE_COPY_RELOCS 1

#define qdsp6_hash_entry(E) ((qdsp6_elf_link_hash_entry *) (E))
#define qdsp6_elf_hash_table(I) ((qdsp6_elf_link_hash_table *) ((I)->hash))

static void init_qdsp6_howto_table
  PARAMS ((void)) __attribute__ ((constructor));

static reloc_howto_type *qdsp6_elf_reloc_type_lookup
  PARAMS ((bfd *abfd, bfd_reloc_code_real_type code));
static void qdsp6_info_to_howto_rel
  PARAMS ((bfd *, arelent *, Elf_Internal_Rela *));
static bfd_boolean qdsp6_elf_object_p
  PARAMS ((bfd *));
static void qdsp6_elf_final_write_processing
  PARAMS ((bfd *, bfd_boolean));
static bfd_reloc_status_type qdsp6_elf_reloc
  PARAMS ((bfd *, arelent *, asymbol *, PTR, asection *, bfd *, char **));
static bfd_reloc_status_type qdsp6_elf_final_sda_base
  PARAMS ((bfd *, char **, bfd_vma *));
static bfd_boolean qdsp6_elf_link_output_symbol_hook
  PARAMS ((struct bfd_link_info *info ATTRIBUTE_UNUSED,
           const char *name ATTRIBUTE_UNUSED,
           Elf_Internal_Sym *sym,
           asection *input_sec,
           struct elf_link_hash_entry *h ATTRIBUTE_UNUSED));
static bfd_boolean qdsp6_elf_section_from_bfd_section
  PARAMS ((bfd *, asection *, int *));
static bfd_boolean qdsp6_elf_section_processing
  PARAMS ((bfd *, Elf_Internal_Shdr *));
static void qdsp6_elf_symbol_processing
  PARAMS ((bfd *, asymbol *));
static bfd_boolean qdsp6_elf_common_definition
  PARAMS ((Elf_Internal_Sym *));
static bfd_boolean qdsp6_elf_add_symbol_hook
  PARAMS ((bfd *, struct bfd_link_info *i,
           Elf_Internal_Sym *, const char **, flagword *,
           asection **, bfd_vma *));
static bfd_boolean qdsp6_elf_section_from_shdr
  PARAMS ((bfd *, Elf_Internal_Shdr *, const char *, int));
static bfd_boolean qdsp6_elf_fake_sections
  PARAMS ((bfd *, Elf_Internal_Shdr *, asection *));
static bfd_boolean qdsp6_elf_relocate_section
  PARAMS ((bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
           Elf_Internal_Rela *, Elf_Internal_Sym *, asection **));
static bfd_boolean qdsp6_elf_gc_sweep_hook
  PARAMS ((bfd *, struct bfd_link_info *,
           asection *, const Elf_Internal_Rela *));
static bfd_boolean qdsp6_elf_check_relocs
  PARAMS ((bfd *, struct bfd_link_info *,
           asection *, const Elf_Internal_Rela *));
static bfd_boolean qdsp6_elf_relax_section
  PARAMS ((bfd *, asection *, struct bfd_link_info *, bfd_boolean *));
static bfd_boolean qdsp6_elf_adjust_dynamic_symbol
  PARAMS ((struct bfd_link_info *, struct elf_link_hash_entry *));
static bfd_boolean qdsp6_elf_finish_dynamic_symbol
  PARAMS ((bfd *, struct bfd_link_info *,
           struct elf_link_hash_entry *, Elf_Internal_Sym *));
static bfd_boolean qdsp6_elf_create_dynamic_sections
  PARAMS ((bfd *, struct bfd_link_info *));
static bfd_boolean qdsp6_elf_size_dynamic_sections
  PARAMS ((bfd *, struct bfd_link_info *));
static bfd_boolean qdsp6_elf_finish_dynamic_sections
  PARAMS ((bfd *output_bfd, struct bfd_link_info *info));
static void qdsp6_elf_copy_indirect_symbol
  PARAMS ((struct bfd_link_info *,
           struct elf_link_hash_entry *, struct elf_link_hash_entry *));
static int qdsp6_elf_additional_program_headers
  PARAMS ((bfd *, struct bfd_link_info *));
static bfd_boolean qdsp6_elf_ignore_discarded_relocs
  PARAMS ((asection *));

typedef struct _qdsp6_elf_dyn_reloc
  {
    struct _qdsp6_elf_dyn_reloc *next;

    asection *sec; /* A .rela section. */
    unsigned int rtype; /* The relocation type. */
    bfd_boolean rtext; /* If against a read-only section. */
    size_t count, pc_count;
  } qdsp6_elf_dyn_reloc;

typedef struct _qdsp6_elf_link_hash_entry
  {
    struct elf_link_hash_entry elf;
    /* QDSP6 Data. */
    qdsp6_elf_dyn_reloc *dyn_relocs;
  } qdsp6_elf_link_hash_entry;

#define qdsp6_elf_hash_entry(ent) ((qdsp6_elf_link_hash_entry *) (ent))
#define streq(a, b)     (strcmp ((a), (b)) == 0)
#define strneq(a, b)     (strcmp ((a), (b)) != 0)

typedef struct _qdsp6_elf_link_hash_table
  {
    struct elf_link_hash_table elf;
    /* QDSP6 data. */
  /* Small local sym cache.  */
  struct sym_cache sym_cache;
  } qdsp6_elf_link_hash_table;

typedef struct
  {
    bfd_vma rtype, offset;
  } qdsp6_trampoline_rel;

static const qdsp6_trampoline_rel qdsp6_trampoline_rels [] =
  {
    {R_QDSP6_HL16, 2 * sizeof (qdsp6_insn)},
  };

static const qdsp6_insn qdsp6_trampoline [] =
  {
    /* This trampoline requires 1 special
       relocation, but takes a cycle longer.   */
    0xbffd7f1d, /*  { sp = add (sp, #-8)       */
    0xa79dfcfe, /*    memw (sp + #-8) = r28 }  */
    0x723cc000, /*  r28.h = #HI (foo)          */
    0x713cc000, /*  r28.l = #LO (foo)          */
    0xb01d411d, /*  { sp = add (sp, #8)        */
    0x529c4000, /*    jumpr r28                */
    0x919dc01c, /*    r28 = memw (sp) }        */
  };

    /* This trampoline requires 1 relocation,
       but mixes code and data in a page.     */
    /* 0x6a09401c, { r28 = pc                 */
    /* 0xbffd7f1d,   r29 = add (r29, #-8)     */
    /* 0xa79dfcfe,   memw (r29 + #-8) = r28 } */
    /* 0x919cc0fc, r28 = memw (r28 + #28)     */
    /* 0xb01d411d, { r29 = add (r29, #8)      */
    /* 0x529c4000,   jumpr r28                */
    /* 0x919dc01c,   r28 = memw (r29) }       */
    /* 0x00000000, .word foo                  */

    /* This trampoline requires 2 relocations. */
    /* 0xbffd7f1d,  { sp = add (sp, #-8)       */
    /* 0x723c4000,    r28.h = #HI (foo)        */
    /* 0xa79dfcfe,    memw (sp + #-8) = r28 }  */
    /* 0x713cc000,  r28.l = #LO (foo)          */
    /* 0xb01d411d,  { sp = add (sp, #8)        */
    /* 0x529c4000,    jumpr r28                */
    /* 0x919dc01c,    r28 = memw (sp) }        */

static reloc_howto_type elf_qdsp6_howto_table_v2 [] =
{
  /* This reloc does nothing.  */
  EMPTY_HOWTO (R_QDSP6_NONE),

  /* A relative 22 bit branch. */
  HOWTO (R_QDSP6_B22_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 24,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B22_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x01ff3ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 15 bit branch. */
  HOWTO (R_QDSP6_B15_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 17,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B15_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00df20fe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* v2: A relative 7 bit branch. */
  HOWTO (R_QDSP6_B7_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 9,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B7_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00001f18,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* Low 16 bits of a 32 bit number. */
  HOWTO (R_QDSP6_LO16,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_LO16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High 16 bits of a 32 bit number. */
  HOWTO (R_QDSP6_HI16,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_HI16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A 32 bit number. */
  HOWTO (R_QDSP6_32,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_QDSP6_32",		/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A 16 bit number. */
  HOWTO (R_QDSP6_16,		/* type  */
	 0,			/* rightshift  */
	 1,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_QDSP6_16",		/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An 8 bit number. */
  HOWTO (R_QDSP6_8,		/* type  */
	 0,			/* rightshift  */
	 0,			/* size (0 = byte, 1 = short, 2 = long)  */
	 8,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_QDSP6_8",		/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xff,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for bytes. */
  HOWTO (R_QDSP6_GPREL16_0,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_GPREL16_0",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for halves. */
  HOWTO (R_QDSP6_GPREL16_1,	/* type  */
	 1,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 17,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_GPREL16_1",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for words. */
  HOWTO (R_QDSP6_GPREL16_2,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 18,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 -1L,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_GPREL16_2",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for doublewords. */
  HOWTO (R_QDSP6_GPREL16_3,	/* type  */
	 3,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 19,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_GPREL16_3",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High and low 16 bits of a 32 bit number applied to 2 insns back-to-back. */
  HOWTO (R_QDSP6_HL16,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_HL16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 13 bit branch. */
  HOWTO (R_QDSP6_B13_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 15,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B13_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00202ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 9 bit branch. */
  HOWTO (R_QDSP6_B9_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 11,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B9_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00c000ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 32 bit extended branch. */
  HOWTO (R_QDSP6_B32_PCREL_X,	/* type  */
	 6,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B32_PCREL_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 ~0x3f,			/* src_mask  */
	 0x0fff3fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended 32 bit number. */
  HOWTO (R_QDSP6_32_6_X,	/* type  */
	 6,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_32_6_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 ~0x3f,			/* src_mask  */
	 0x0fff3fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 22 bit branch. */
  HOWTO (R_QDSP6_B22_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B22_PCREL_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x01ff3ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 15 bit branch. */
  HOWTO (R_QDSP6_B15_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B15_PCREL_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x00df20fe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 13 bit branch. */
  HOWTO (R_QDSP6_B13_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B13_PCREL_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x00202ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 9 bit branch. */
  HOWTO (R_QDSP6_B9_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B9_PCREL_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x00c000ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 16 bit number. */
  HOWTO (R_QDSP6_16_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_16_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 12 bit number. */
  HOWTO (R_QDSP6_12_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_12_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended 11 bit number for bytes. */
  HOWTO (R_QDSP6_11_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_11_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 16 bit number. */
  HOWTO (R_QDSP6_10_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_10_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 9 bit number. */
  HOWTO (R_QDSP6_9_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_9_X",		/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 8 bit number. */
  HOWTO (R_QDSP6_8_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_8_X",		/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 7 bit number. */
  HOWTO (R_QDSP6_7_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_7_X",		/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 6 bit number. */
  HOWTO (R_QDSP6_6_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_6_X",		/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A 32 bit PC-relative number. */
  HOWTO (R_QDSP6_32_PCREL,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_QDSP6_32_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

};

static reloc_howto_type *elf_qdsp6_howto_table;

/* QDSP6 ELF uses two common sections.  One is the usual one, and the
   other is for small objects.  All the small objects are kept
   together, and then referenced via the GP-register, which yields
   faster assembler code.  This is what we use for the small common
   section.  This approach is copied from ecoff.c.  */
static asection    qdsp6_scom_section [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON + 1];
static asymbol     qdsp6_scom_symbol  [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON + 1];
static const char *qdsp6_scom_name    [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON + 1] =
  {".scommon", ".scommon.1", ".scommon.2", ".scommon.4", ".scommon.8"};

/* Map BFD reloc types to QDSP6 ELF reloc types.  */

struct qdsp6_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char            elf_reloc_val;
  int                      operand_flag;
};

static const struct qdsp6_reloc_map qdsp6_reloc_map [] =
{
  { BFD_RELOC_NONE,              R_QDSP6_NONE,        0 },
  { BFD_RELOC_QDSP6_B32_PCREL_X, R_QDSP6_B32_PCREL_X, 0 }, /* K-ext */
  { BFD_RELOC_QDSP6_B22_PCREL,   R_QDSP6_B22_PCREL,   0 },
  { BFD_RELOC_QDSP6_B22_PCREL_X, R_QDSP6_B22_PCREL_X, QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_B15_PCREL,   R_QDSP6_B15_PCREL,   0 },
  { BFD_RELOC_QDSP6_B15_PCREL_X, R_QDSP6_B15_PCREL_X, QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_B13_PCREL,   R_QDSP6_B13_PCREL,   0 },
  { BFD_RELOC_QDSP6_B13_PCREL_X, R_QDSP6_B13_PCREL_X, QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_B9_PCREL,    R_QDSP6_B9_PCREL,    0 },
  { BFD_RELOC_QDSP6_B9_PCREL_X,  R_QDSP6_B9_PCREL_X,  QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_B7_PCREL,    R_QDSP6_B7_PCREL,    0 },
  { BFD_RELOC_QDSP6_LO16,        R_QDSP6_LO16,        0 },
  { BFD_RELOC_QDSP6_HI16,        R_QDSP6_HI16,        0 },
  { BFD_RELOC_QDSP6_HL16,        R_QDSP6_HL16,        0 },
  { BFD_RELOC_QDSP6_32_6_X,      R_QDSP6_32_6_X,      0 }, /* K-ext */
  { BFD_RELOC_32_PCREL,          R_QDSP6_32_PCREL,    0 },
  { BFD_RELOC_32,                R_QDSP6_32,          0 },
  { BFD_RELOC_16,                R_QDSP6_16,          0 },
  { BFD_RELOC_8,                 R_QDSP6_8,           0 },
  { BFD_RELOC_QDSP6_16_X,        R_QDSP6_16_X,        QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_12_X,        R_QDSP6_12_X,        QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_11_X,        R_QDSP6_11_X,        QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_10_X,        R_QDSP6_10_X,        QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_9_X,         R_QDSP6_9_X,         QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_8_X,         R_QDSP6_8_X,         QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_7_X,         R_QDSP6_7_X,         QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_6_X,         R_QDSP6_6_X,         QDSP6_OPERAND_IS_KXED },
  { BFD_RELOC_QDSP6_GPREL16_0,   R_QDSP6_GPREL16_0,   0 },
  { BFD_RELOC_QDSP6_GPREL16_1,   R_QDSP6_GPREL16_1,   0 },
  { BFD_RELOC_QDSP6_GPREL16_2,   R_QDSP6_GPREL16_2,   0 },
  { BFD_RELOC_QDSP6_GPREL16_3,   R_QDSP6_GPREL16_3,   0 },
};

static void
init_qdsp6_howto_table
(void)
{
  elf_qdsp6_howto_table = elf_qdsp6_howto_table_v2;
}

static reloc_howto_type *
qdsp6_elf_reloc_type_lookup
(bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  size_t i;

  for (i = ARRAY_SIZE (qdsp6_reloc_map); i--;)
    if (qdsp6_reloc_map [i].bfd_reloc_val == code)
      return (elf_qdsp6_howto_table + qdsp6_reloc_map [i].elf_reloc_val);

  return NULL;
}

static bfd_reloc_code_real_type
qdsp6_elf_reloc_val_lookup
(unsigned char elf_reloc_val, int *flag)
{
  unsigned int i;

  for (i = ARRAY_SIZE (qdsp6_reloc_map); i--;)
    if (qdsp6_reloc_map [i].elf_reloc_val == elf_reloc_val)
      {
        if (flag)
          *flag = qdsp6_reloc_map [i].operand_flag;

        return (qdsp6_reloc_map [i].bfd_reloc_val);
      }

  if (flag)
    *flag = 0;

  return (BFD_RELOC_NONE);
}

static reloc_howto_type *
qdsp6_elf_reloc_name_lookup
(bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < sizeof (elf_qdsp6_howto_table_v2) /  sizeof (elf_qdsp6_howto_table [0]);
       i++)
    if (elf_qdsp6_howto_table [i].name
        && !strcasecmp (elf_qdsp6_howto_table [i].name, r_name))
    return (elf_qdsp6_howto_table + i);

  return NULL;
}

/* Set the howto pointer for a QDSP6 ELF reloc.  */

static void
qdsp6_info_to_howto_rel
(bfd *abfd ATTRIBUTE_UNUSED, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_QDSP6_max);
  cache_ptr->howto = &elf_qdsp6_howto_table[r_type];
}

/* Set the right machine number for an QDSP6 ELF file.  */

static bfd_boolean
qdsp6_elf_object_p
(bfd *abfd)
{
  unsigned int mach = bfd_mach_qdsp6_v2;

  /* Set our target machine number. */
  if (elf_elfheader (abfd)->e_machine == EM_QDSP6)
    {
      unsigned long arch = elf_elfheader (abfd)->e_flags & EF_QDSP6_MACH;

      switch (EF_QDSP6_MACH_VER (arch))
	{
          case EF_QDSP6_MACH_V2:
            mach = bfd_mach_qdsp6_v2;
            break;

          case EF_QDSP6_MACH_V3:
            mach = bfd_mach_qdsp6_v3;
            break;

          case EF_QDSP6_MACH_V4:
            mach = bfd_mach_qdsp6_v4;
            break;

          default:
            if (elf_elfheader (abfd)->e_type != ET_QDSP6_IR)
              {
                /* Unless the object is a WHIRL, barf. */
                fprintf (stderr, "Unsupported machine architecure %ld\n", arch);
                abort ();
              }
	}
    }

  return bfd_default_set_arch_mach (abfd, bfd_arch_qdsp6, mach);
}

/* The final processing done just before writing out an QDSP6 ELF object file.
   This gets the QDSP6 architecture right based on the machine number.  */

static void
qdsp6_elf_final_write_processing
(bfd *abfd, bfd_boolean linker ATTRIBUTE_UNUSED)
{
  unsigned long val;

  switch (bfd_get_mach (abfd))
    {
      case bfd_mach_qdsp6_v2:
        val = EF_QDSP6_MACH_V2;
        break;

      case bfd_mach_qdsp6_v3:
        val = EF_QDSP6_MACH_V3;
        break;

      case bfd_mach_qdsp6_v4:
        val = EF_QDSP6_MACH_V4;
        break;

      default:
        fprintf (stderr, "Unsupported machine architecure.\n");
        abort ();
    }

  elf_elfheader (abfd)->e_flags &= ~EF_QDSP6_MACH;
  elf_elfheader (abfd)->e_flags |= val;
}

static struct bfd_link_hash_entry *_sda_base = (struct bfd_link_hash_entry *)0;

static bfd_byte *
qdsp6_elf_get_relocated_section_contents
(bfd *output_bfd, struct bfd_link_info *link_info,
 struct bfd_link_order *link_order, bfd_byte *data, bfd_boolean relocatable,
 asymbol **symbols)
{
  struct bfd_link_hash_entry *hash;
  bfd_vma vma;

  /* Get the hash table entry for _SDA_BASE_. */
  if ((hash =
         bfd_link_hash_lookup (link_info->hash, SDA_BASE, FALSE, FALSE, FALSE)))
    _sda_base = hash;
  /* In case of relying on _SDA_BASE_ being provided by the linker script,
     the look up above will fail, for it's too soon to provide its value.
     Instead, look up the alternate symbol that will be provided later on. */
  else if ((hash =
             bfd_link_hash_lookup (link_info->hash, DEFAULT_SDA_BASE,
                                   FALSE, FALSE, FALSE)))
    _sda_base = hash;
  /* At this point, if no _SDA_BASE_, either the alternate or the proper one,
     is resolved, then GP-relocaiton overflow errors are likely. */

  /* Try to validate the effective value of _SDA_BASE_. */
  if (_sda_base)
    {
      if (bfd_reloc_ok != qdsp6_elf_final_sda_base (output_bfd, NULL, &vma))
        {
          fprintf (stderr, "_SDA_BASE_ must be defined.\n");
          abort ();
        }
      else if (vma % 0x80000)
        {
          fprintf (stderr, "_SDA_BASE_ not 512K-aligned.\n");
        }
    }

  /* call the original */
  return
    (bfd_generic_get_relocated_section_contents
      (output_bfd, link_info, link_order, data, relocatable, symbols));
}

/* We have to figure out the SDA_BASE value, so that we can adjust the
   symbol value correctly.  We look up the symbol _SDA_BASE_ in the output
   BFD.  If we can't find it, we're stuck.  We cache it in the ELF
   target data.  We don't need to adjust the symbol value for an
   external symbol if we are producing relocatable output.  */

static bfd_reloc_status_type
qdsp6_elf_final_sda_base
(bfd *output_bfd, char **error_message, bfd_vma *psb)
{
  if (output_bfd != (bfd *) NULL)
    {
      *psb = 0;
      return bfd_reloc_ok;
    }
  else if (_sda_base != 0 && _sda_base->type == bfd_link_hash_defined)
    {
      *psb = _sda_base->u.def.value
	   + _sda_base->u.def.section->output_section->vma
	   + _sda_base->u.def.section->output_offset;
      return (bfd_reloc_ok);
    }
  else
    {
      if (error_message)
        *error_message = _("GP-relative relocation when _SDA_BASE_ not defined");

      *psb = 0;
      return (bfd_reloc_dangerous);
    }
}

static int
qdsp6_reloc_operand
(reloc_howto_type *howto, qdsp6_insn *insn, bfd_vma offset, char **errmsg)
{
  bfd_reloc_code_real_type type;
  const qdsp6_opcode *opcode;
  const qdsp6_operand *operand;
  int flag, is_x;
  long value, xvalue;

  opcode  = qdsp6_lookup_insn (*insn);
  type    = qdsp6_elf_reloc_val_lookup (howto->type, &flag);
  operand = qdsp6_lookup_reloc (type, flag, opcode);

  is_x = (flag & QDSP6_OPERAND_IS_KXED);

  value = offset;

  if ((opcode) && (operand))
    {
      if (!qdsp6_encode_operand
            (operand, insn, opcode, value, &xvalue, flag & QDSP6_OPERAND_IS_KXED,
             operand->flags & QDSP6_OPERAND_PC_RELATIVE, errmsg))
        {
          /*
          fprintf (stderr, "Error when encoding operand `%s' of `%s'.\n",
                   operand->fmt, opcode->syntax);
          */

          return FALSE;
        }
    }
  else
    return FALSE;

  return TRUE;
}

static bfd_reloc_status_type
qdsp6_elf_reloc
(bfd *abfd, arelent *reloc_entry, asymbol *symbol, PTR data,
 asection *input_section, bfd *output_bfd, char **error_message)
{
  reloc_howto_type *howto = reloc_entry->howto;
  bfd_size_type octets_per_byte = bfd_octets_per_byte(abfd);
  bfd_size_type octets = reloc_entry->address * octets_per_byte;
  bfd_reloc_status_type status;
  bfd_reloc_status_type flag = bfd_reloc_ok;
  bfd_vma relocation;
  bfd_vma output_base = 0;
  asection *reloc_target_output_section;

  /* bfd_perform_relocation() performed this test before calling
     howto->special_function(), but that gets ignored if we return
	 bfd_reloc_ok (which we do, below) so we need to re-do this
	 test here. The real bug is probably in bfd_perform_relocation()
	 but it is better not to modify the common code. */
  if (bfd_is_und_section (symbol->section)
      && (symbol->flags & BSF_WEAK) == 0
      && output_bfd == (bfd *) NULL)
    return bfd_reloc_undefined;

  /* If linking, back up the final symbol address by the address of the
     reloc.  This cannot be accomplished by setting the pcrel_offset
     field to TRUE, as bfd_install_relocation will detect this and refuse
     to install the offset in the first place, but bfd_perform_relocation
     will still insist on removing it.  */
  if (output_bfd == (bfd *) NULL && howto->pc_relative)
    reloc_entry->addend -= reloc_entry->address;

  /* Fall through to the default elf reloc handler.  */
  status = bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
                                  input_section, output_bfd, error_message);
  if (status != bfd_reloc_continue)
    return status;

  /* Handle PC relative relocatable output. */
  if (output_bfd != (bfd *) NULL
      && reloc_entry->howto->pc_relative
      && (!reloc_entry->howto->partial_inplace
          || !reloc_entry->addend))
    {
      reloc_entry->address += input_section->output_offset;
      return bfd_reloc_ok;
    }

  // NOTE:  This code was pasted/modified from bfd_perform_relocation
  //        We  can't use that code (which would happen if we returned
  //        bfd_reloc_continue) because it assumes the operand goes
  //        into a contiguous range of bits.

  /* Is the address of the relocation really within the section?  */
  if (reloc_entry->address > (input_section->size
			      / bfd_octets_per_byte (abfd)))
    return bfd_reloc_outofrange;

  /* Work out which section the relocation is targetted at and the
     initial relocation command value.  */

  /* Get symbol value.  (Common symbols are special.)  */
  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  reloc_target_output_section = symbol->section->output_section;

  /* Convert input-section-relative symbol value to absolute.  */
  if ((output_bfd && !howto->partial_inplace)
      || reloc_target_output_section == NULL)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;

  relocation += output_base + symbol->section->output_offset;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
     symbol we are relocating against, plus any addend.  */


  if ((howto->type == R_QDSP6_GPREL16_0) ||
      (howto->type == R_QDSP6_GPREL16_1) ||
      (howto->type == R_QDSP6_GPREL16_2) ||
      (howto->type == R_QDSP6_GPREL16_3))
    {
      bfd_vma sda_base;

      if ((strcmp (symbol->section->name, ".sdata"))
          && (strcmp (symbol->section->name, ".sbss")))
	{
        }

      status = qdsp6_elf_final_sda_base (output_bfd, error_message, &sda_base);
      if (status != bfd_reloc_ok)
        return status;

      /* At this point `relocation' contains the object's address.  */
      relocation -= sda_base;
    }

  if (howto->pc_relative)
    {
      /* This is a PC relative relocation.  We want to set RELOCATION
	 to the distance between the address of the symbol and the
	 location.  RELOCATION is already the address of the symbol.

	 We start by subtracting the address of the section containing
	 the location.

	 If pcrel_offset is set, we must further subtract the position
	 of the location within the section.  Some targets arrange for
	 the addend to be the negative of the position of the location
	 within the section; for example, i386-aout does this.  For
	 i386-aout, pcrel_offset is FALSE.  Some other targets do not
	 include the position of the location; for example, m88kbcs,
	 or ELF.  For those targets, pcrel_offset is TRUE.

	 If we are producing relocatable output, then we must ensure
	 that this reloc will be correctly computed when the final
	 relocation is done.  If pcrel_offset is FALSE we want to wind
	 up with the negative of the location within the section,
	 which means we must adjust the existing addend by the change
	 in the location within the section.  If pcrel_offset is TRUE
	 we do not want to adjust the existing addend at all.

	 FIXME: This seems logical to me, but for the case of
	 producing relocatable output it is not what the code
	 actually does.  I don't want to change it, because it seems
	 far too likely that something will break.  */

      relocation -=
	input_section->output_section->vma + input_section->output_offset;

      if (howto->pcrel_offset)
	relocation -= reloc_entry->address;
    }

  if (output_bfd != (bfd *) NULL)
    {
      if (!howto->partial_inplace)
	{
	  /* This is a partial relocation, and we want to apply the relocation
	     to the reloc entry rather than the raw data. Modify the reloc
	     inplace to reflect what we now know.  */
	  reloc_entry->addend = relocation;
	  reloc_entry->address += input_section->output_offset;
	  return flag;
	}
      else
	{
	  /* This is a partial relocation, but inplace, so modify the
	     reloc record a bit.

	     If we've relocated with a symbol with a section, change
	     into a ref to the section belonging to the symbol.  */

	  reloc_entry->address += input_section->output_offset;
	  //printf("%d: relocation: 0x%x Reloc address: 0x%x\n",__LINE__,relocation,reloc_entry->address);
	  reloc_entry->addend = relocation;
	}
    }
  else
    {
      reloc_entry->addend = 0;
    }

  /* If we have a PC-relative reference to an undefined weak symbol,
     the relocation may exceed the range of the call/jump instruction.
     Since the symbol is not really supposed to be branched to, it
     doesn't matter how we relocate the instruction, so just insert
     0x00000000 into the call/jump. */

     if (howto->pc_relative
         && bfd_is_und_section (symbol->section)
         && (symbol->flags & BSF_WEAK)
         && output_bfd == (bfd *) NULL)
       {
         relocation = 0;
       }

  /* FIXME: This overflow checking is incomplete, because the value
     might have overflowed before we get here.  For a correct check we
     need to compute the value in a size larger than bitsize, but we
     can't reasonably do that for a reloc the same size as a host
     machine word.
     FIXME: We should also do overflow checking on the result after
     adding in the value contained in the object file.  */
  if (howto->complain_on_overflow != complain_overflow_dont
      && flag == bfd_reloc_ok)
    flag = bfd_check_overflow (howto->complain_on_overflow,
			       howto->bitsize,
			       howto->rightshift,
			       bfd_arch_bits_per_address (abfd),
			       relocation);

  /* Either we are relocating all the way, or we don't want to apply
     the relocation to the reloc entry (probably because there isn't
     any room in the output format to describe addends to relocs).  */

  /* The cast to bfd_vma avoids a bug in the Alpha OSF/1 C compiler
     (OSF version 1.3, compiler version 3.11).  It miscompiles the
     following program:

     struct str
     {
       unsigned int i0;
     } s = { 0 };

     int
     main ()
     {
       unsigned long x;

       x = 0x100000000;
       x <<= (unsigned long) s.i0;
       if (x == 0)
	 printf ("failed\n");
       else
	 printf ("succeeded (%lx)\n", x);
     }
     */

  switch (howto->size)
    {
      case 1:
        {
	  qdsp6_insn insn = bfd_get_16 (abfd, (bfd_byte *) data + octets);

	  if (!qdsp6_reloc_operand (howto, &insn, relocation, error_message))
	    {
	      /*
	      fprintf (stderr, "\tfor \"%s\" in \"%s\", %s.\n",
		       symbol->name, symbol->the_bfd->filename,
		       *error_message);
	      */

	      return bfd_reloc_overflow;
	    }

	  bfd_put_16 (abfd, (bfd_vma) insn, (unsigned char *) data);
        }
      break;

      case 2:
        {
	  qdsp6_insn insn = bfd_get_32 (abfd, (bfd_byte *) data + octets);

	  if (!qdsp6_reloc_operand (howto, &insn, relocation, error_message))
	    {
	      /*
	      fprintf (stderr, "\tat offset 0x%08x of section \"%s\" in \"%s\",\n\t%s.\n",
		       (bfd_byte *) octets, input_section->name,
		       symbol->the_bfd->filename,
		       *error_message);
	      */

               return bfd_reloc_overflow;
	     }

	  bfd_put_32 (abfd, (bfd_vma) insn, (bfd_byte *) data + octets);
        }
      break;

      default:
      return bfd_reloc_other;
    }

  return bfd_reloc_ok;
}

/* Wrapper for _bfd_elf_set_arch_mach
   Also calls qdsp6_opcode_init_tables */

static bfd_boolean
qdsp6_elf_set_arch_mach
(bfd *abfd, enum bfd_architecture arch, unsigned long machine)
{
  if (!_bfd_elf_set_arch_mach (abfd, arch, machine))
    return FALSE;

  qdsp6_opcode_init_tables (qdsp6_get_opcode_mach (machine, bfd_big_endian (abfd)));

  return TRUE;
}

/* Set up any other section flags and such that may be necessary.  */
static bfd_boolean
qdsp6_elf_fake_sections
(bfd *abfd ATTRIBUTE_UNUSED, Elf_Internal_Shdr *shdr, asection *asect)
{
  if ((asect->flags & SEC_SORT_ENTRIES))
    shdr->sh_type = SHT_QDSP6_ORDERED;

  return TRUE;
}

/* Handle a specific sections when reading an object file.  This
   is called when elfcode.h finds a section with an unknown type.  */
static bfd_boolean
qdsp6_elf_section_from_shdr
(bfd *abfd, Elf_Internal_Shdr *hdr, const char *name, int shindex)
{
  asection *newsect;
  flagword flags;

  if (!_bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return FALSE;

  newsect = hdr->bfd_section;
  flags = bfd_get_section_flags (abfd, newsect);

  if (hdr->sh_type == SHT_QDSP6_ORDERED)
    flags |= SEC_SORT_ENTRIES;

  bfd_set_section_flags (abfd, newsect, flags);
  return TRUE;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */
static bfd_boolean
qdsp6_elf_add_symbol_hook (bfd *abfd,
		           struct bfd_link_info *info ATTRIBUTE_UNUSED,
			   Elf_Internal_Sym *sym,
                           const char **namep ATTRIBUTE_UNUSED,
                           flagword *flagsp ATTRIBUTE_UNUSED,
                           asection **secp,
                           bfd_vma *valp)
{
  /* This step must be performed even for partial links because otherwise
     the special sections would not be created, resulting in a subsequent
     error check higher up failing. */
  if (bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    {
      switch (sym->st_shndx)
        {
          case SHN_COMMON:
            /* Common symbols less than the GP size are automatically
              treated as SHN_QDSP6S_SCOMMON symbols.  */
            if (sym->st_size > elf_gp_size (abfd))
              break;

            /* Choose which section to place them in. */
            if (sym->st_size > 8)
              ((Elf_Internal_Sym *) sym)->st_shndx = SHN_QDSP6_SCOMMON;
            if (sym->st_size > 4)
              ((Elf_Internal_Sym *) sym)->st_shndx = SHN_QDSP6_SCOMMON_8;
            else if (sym->st_size > 2)
              ((Elf_Internal_Sym *) sym)->st_shndx = SHN_QDSP6_SCOMMON_4;
            else if (sym->st_size > 1)
              ((Elf_Internal_Sym *) sym)->st_shndx = SHN_QDSP6_SCOMMON_2;
            else
              ((Elf_Internal_Sym *) sym)->st_shndx = SHN_QDSP6_SCOMMON_1;

            /* Fall through. */

          case SHN_QDSP6_SCOMMON:
          case SHN_QDSP6_SCOMMON_1:
          case SHN_QDSP6_SCOMMON_2:
          case SHN_QDSP6_SCOMMON_4:
          case SHN_QDSP6_SCOMMON_8:
            /* Common symbols less than or equal to -G x bytes-long are
               put into .scommon.  */
            *secp = bfd_make_section_old_way
                      (abfd, qdsp6_scom_name [sym->st_shndx - SHN_QDSP6_SCOMMON]);
            bfd_set_section_flags
              (abfd, *secp, SEC_ALLOC | SEC_IS_COMMON | SEC_LINKER_CREATED);
            *valp = sym->st_size;
            break;
        }
    }

  return TRUE;
}

/* Handle the special QDSP6 section numbers that a symbol may use. */
void
qdsp6_elf_symbol_processing
(bfd *abfd, asymbol *asym)
{
  elf_symbol_type *elfsym = (elf_symbol_type *) asym;

  /* Decide if a common symbol should be considered small or not. */
  switch (elfsym->internal_elf_sym.st_shndx)
    {
    case SHN_COMMON:
      /* Common symbols less than the GP size are automatically
	 treated as SHN_QDSP6S_SCOMMON symbols.  */
      if (asym->value > elf_gp_size (abfd))
	break;

        /* Choose which section to place them in. */
        if (elfsym->internal_elf_sym.st_size > 8)
          elfsym->internal_elf_sym.st_shndx = SHN_QDSP6_SCOMMON;
        if (elfsym->internal_elf_sym.st_size > 4)
          elfsym->internal_elf_sym.st_shndx = SHN_QDSP6_SCOMMON_8;
        else if (elfsym->internal_elf_sym.st_size > 2)
          elfsym->internal_elf_sym.st_shndx = SHN_QDSP6_SCOMMON_4;
        else if (elfsym->internal_elf_sym.st_size > 1)
          elfsym->internal_elf_sym.st_shndx = SHN_QDSP6_SCOMMON_2;
        else
          elfsym->internal_elf_sym.st_shndx = SHN_QDSP6_SCOMMON_1;

      /* Fall through.  */

    case SHN_QDSP6_SCOMMON:
    case SHN_QDSP6_SCOMMON_1:
    case SHN_QDSP6_SCOMMON_2:
    case SHN_QDSP6_SCOMMON_4:
    case SHN_QDSP6_SCOMMON_8:
      {
        asection *scom_section = qdsp6_scom_section
                               + elfsym->internal_elf_sym.st_shndx - SHN_QDSP6_SCOMMON;
        asymbol *scom_symbol = qdsp6_scom_symbol
                             + elfsym->internal_elf_sym.st_shndx - SHN_QDSP6_SCOMMON;

        if (!scom_section->name)
          {
            const char *scom_name =
              qdsp6_scom_name [elfsym->internal_elf_sym.st_shndx - SHN_QDSP6_SCOMMON];

            /* Initialize the small common section.  */
            scom_section->name           = scom_name;
            scom_section->flags          = SEC_IS_COMMON | SEC_SMALL_DATA
                                           | (elfsym->internal_elf_sym.st_shndx
                                              > SHN_QDSP6_SCOMMON
                                              ? SEC_LOAD | SEC_DATA: 0);
            scom_section->output_section = scom_section;
            scom_section->symbol         = scom_symbol;
            scom_section->symbol_ptr_ptr = &scom_section->symbol;

            scom_symbol->name    = scom_name;
            scom_symbol->flags   = BSF_SECTION_SYM;
            scom_symbol->section = scom_section;
          }

        asym->section = scom_section;
        asym->value = elfsym->internal_elf_sym.st_size;
      }
      break;
    }
}

static bfd_boolean
qdsp6_elf_common_definition (Elf_Internal_Sym *sym)
{
  return (sym->st_shndx == SHN_COMMON
          || sym->st_shndx == SHN_QDSP6_SCOMMON
          || sym->st_shndx == SHN_QDSP6_SCOMMON_1
          || sym->st_shndx == SHN_QDSP6_SCOMMON_2
          || sym->st_shndx == SHN_QDSP6_SCOMMON_4
          || sym->st_shndx == SHN_QDSP6_SCOMMON_8);
}


/* Work over a section just before writing it out. FIXME: We recognize
   sections that need the SHF_QDSP6_GPREL flag by name; there has to be
   a better way.  */

bfd_boolean
qdsp6_elf_section_processing
(bfd *abfd ATTRIBUTE_UNUSED, Elf_Internal_Shdr *hdr)
{
  if (hdr->bfd_section)
    {
      const char *name = bfd_get_section_name (abfd, hdr->bfd_section);

      if (!strcmp (name, ".sdata"))
	{
	  hdr->sh_flags |= SHF_ALLOC | SHF_WRITE | SHF_QDSP6_GPREL;
	  hdr->sh_type = SHT_PROGBITS;
	}
      else if (!strcmp (name, ".sbss"))
	{
	  hdr->sh_flags |= SHF_ALLOC | SHF_WRITE | SHF_QDSP6_GPREL;
	  hdr->sh_type = SHT_NOBITS;
	}
    }

  return TRUE;
}

/* Given a BFD section, try to locate the corresponding ELF section index. */
bfd_boolean
qdsp6_elf_section_from_bfd_section
(bfd *abfd ATTRIBUTE_UNUSED, asection *sec, int *retval)
{
  const char *name = bfd_get_section_name (abfd, sec);

  if (!strncmp (name, qdsp6_scom_name [0], sizeof (qdsp6_scom_name [0]) - 1))
    {
      if      (!strcmp (name, qdsp6_scom_name [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON]))
        *retval = SHN_QDSP6_SCOMMON_8;
      else if (!strcmp (name, qdsp6_scom_name [SHN_QDSP6_SCOMMON_4 - SHN_QDSP6_SCOMMON]))
        *retval = SHN_QDSP6_SCOMMON_4;
      else if (!strcmp (name, qdsp6_scom_name [SHN_QDSP6_SCOMMON_2 - SHN_QDSP6_SCOMMON]))
        *retval = SHN_QDSP6_SCOMMON_2;
      else if (!strcmp (name, qdsp6_scom_name [SHN_QDSP6_SCOMMON_1 - SHN_QDSP6_SCOMMON]))
        *retval = SHN_QDSP6_SCOMMON_1;
      else
        *retval = SHN_QDSP6_SCOMMON;

      return TRUE;
    }
  else
    return FALSE;
}

/* This hook function is called before the linker writes out a global
   symbol.  We mark symbols as small common if appropriate. */
bfd_boolean
qdsp6_elf_link_output_symbol_hook (struct bfd_link_info *info
                                       ATTRIBUTE_UNUSED,
                                   const char *name ATTRIBUTE_UNUSED,
                                   Elf_Internal_Sym *sym,
                                   asection *input_sec,
                                   struct elf_link_hash_entry *h
                                       ATTRIBUTE_UNUSED)
{
  /* If we see a common symbol, which implies a relocatable link, then
     if a symbol was small common in an input file, mark it as small
     common in the output file.  */
  if (sym->st_shndx == SHN_COMMON
      && !strncmp (input_sec->name, qdsp6_scom_name [0], sizeof (qdsp6_scom_name [0]) - 1))
    {
      if      (!strcmp (name, qdsp6_scom_name [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON]))
        sym->st_shndx = SHN_QDSP6_SCOMMON_8;
      else if (!strcmp (name, qdsp6_scom_name [SHN_QDSP6_SCOMMON_4 - SHN_QDSP6_SCOMMON]))
        sym->st_shndx = SHN_QDSP6_SCOMMON_4;
      else if (!strcmp (name, qdsp6_scom_name [SHN_QDSP6_SCOMMON_2 - SHN_QDSP6_SCOMMON]))
        sym->st_shndx = SHN_QDSP6_SCOMMON_2;
      else if (!strcmp (name, qdsp6_scom_name [SHN_QDSP6_SCOMMON_1 - SHN_QDSP6_SCOMMON]))
        sym->st_shndx = SHN_QDSP6_SCOMMON_1;
      else
        sym->st_shndx = SHN_QDSP6_SCOMMON;
    }

  return TRUE;
}

static inline qdsp6_insn
qdsp6_get_insn (bfd * ibfd, reloc_howto_type * howto, bfd_byte *offset)
{
  switch (howto->size)
    {
    case 0:
      return (bfd_get_8 (ibfd, offset));

    case 1:
      return (bfd_get_16 (ibfd, offset));

    case 2:
      return (bfd_get_32 (ibfd, offset));

    default:
      fprintf (stderr,
	       "%s: Unrecognized howto->size  ==%d\n", __func__, howto->size);
      abort ();
    }
  return 0;
}

static inline void
qdsp6_put_insn (bfd * ibfd,
		reloc_howto_type * howto, bfd_byte *offset, qdsp6_insn insn)
{
  switch (howto->size)
    {
    case 0:
      return (bfd_put_8 (ibfd, insn, offset));

    case 1:
      return (bfd_put_16 (ibfd, insn, offset));

    case 2:
      return (bfd_put_32 (ibfd, insn, offset));

    default:
      fprintf (stderr,
	       "%s: Unrecognized howto->size  ==%d\n", __func__, howto->size);
      abort ();
    }
}

static bfd_boolean
qdsp6_elf_gc_sweep_hook (
     bfd *abfd ATTRIBUTE_UNUSED,
     struct bfd_link_info *info ATTRIBUTE_UNUSED,
     asection *sec ATTRIBUTE_UNUSED,
     const Elf_Internal_Rela *relocs ATTRIBUTE_UNUSED)
{
  /* we don't use got and plt entries */
  return TRUE;
}

static bfd_boolean
qdsp6_kept_hash_lookup
(bfd *obfd ATTRIBUTE_UNUSED, struct bfd_link_info *info,
 bfd *ibfd ATTRIBUTE_UNUSED, Elf_Internal_Rela *rel ATTRIBUTE_UNUSED, struct elf_link_hash_entry *h)
{
  asection *s_new;
  bfd *b;

  /* Check for a relocation that is actually dangling. */
  if ((h->root.type != bfd_link_hash_defined
          && h->root.type != bfd_link_hash_defweak)
      || !(elf_discarded_section (h->root.u.def.section)))
    return TRUE;

  for (b = info->input_bfds; b; b = b->link_next)
    {
      s_new = bfd_get_section_by_name (b, h->root.u.def.section->name);
      if (s_new)
        if (!(elf_discarded_section (s_new)))
          {
            h->root.u.def.section = s_new;

            return TRUE;
          }
    }

  return FALSE;
}

static bfd_boolean
qdsp6_elf_relocate_section
(bfd *output_bfd, struct bfd_link_info *info,
 bfd *input_bfd, asection *input_section, bfd_byte *contents,
 Elf_Internal_Rela *relocs, Elf_Internal_Sym *local_syms,
 asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  qdsp6_elf_link_hash_table *htab;

  if (info->relocatable)
    return TRUE;

  htab = qdsp6_elf_hash_table (info);
  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);

  for (rel = relocs; rel < relocs + input_section->reloc_count; rel++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym = NULL;
      asection *sec = NULL;
      struct elf_link_hash_entry *h = NULL;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      bfd_boolean is_abs = FALSE, is_rel = FALSE;
      bfd_boolean is_und = FALSE, is_loc = FALSE;
      asection *sreloc;
      const char *name = NULL;
      int r_type;
      qdsp6_insn insn;
      bfd_vma ioffset, lmask, rmask;

      /* This is a final link.  */
      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);
      howto = elf_qdsp6_howto_table + ELF32_R_TYPE (rel->r_info);

      if (r_symndx < symtab_hdr->sh_info)
	{
          /* This is a local symbol. */
          h = NULL;
          is_loc = TRUE;

	  sym = local_syms + r_symndx;
	  sec = local_sections [r_symndx];

          if (sec && elf_discarded_section (sec))
            {
              /* For relocs against symbols from removed linkonce sections,
                 or sections discarded by a linker script, avoid
                 any special processing. */
              rel->r_info   = 0;
              rel->r_addend = 0;
              continue;
            }

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  if (!name || !*name)
            name = bfd_section_name (input_bfd, sec);

	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
          /* This is a global symbol. */
	  h = sym_hashes [r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  name = h->root.root.string;

	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
              if (elf_discarded_section (h->root.u.def.section))
                if (!(qdsp6_kept_hash_lookup (output_bfd, info, input_bfd, rel, h)))
                  {
                    if (input_section->flags & SEC_DEBUGGING)
                      {
                        /* For relocs in debug section against symbols from
                           discarded sections, avoid any special processing. */
                        rel->r_info   = 0;
                        rel->r_addend = 0;
                        continue;
                      }
                    else
                      {
                        info->callbacks->undefined_symbol
                          (info, h->root.root.string, input_bfd,
                          input_section, rel->r_offset, 0);
                        return FALSE;
                      }
                  }

	      relocation = h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
                           + h->root.u.def.section->output_offset;
	    }
	  else if (h->root.type == bfd_link_hash_undefweak)
	    {
              is_und = TRUE;

	      relocation = 0;
	    }
	  else if (h->root.type == bfd_link_hash_undefined
                   && info->shared
		   && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
            {
              is_und = TRUE;

	      relocation = input_section->output_section->vma
                           + input_section->output_offset;
            }
	  else
	    {
	      if (!((*info->callbacks->undefined_symbol)
		    (info, h->root.root.string, input_bfd,
		     input_section, rel->r_offset,
                     (!info->shared
                      || ELF_ST_VISIBILITY (h->other)))))
		return FALSE;
	      relocation = 0;
	    }
	}

      lmask = rmask = (bfd_vma) -1;

      switch (r_type)
	{
        case R_QDSP6_32_6_X:
          rmask = howto->src_mask;
          /* Fall through. */

        case R_QDSP6_6_X:
        case R_QDSP6_7_X:
        case R_QDSP6_8_X:
        case R_QDSP6_9_X:
        case R_QDSP6_10_X:
        case R_QDSP6_11_X:
        case R_QDSP6_12_X:
        case R_QDSP6_16_X:
          lmask = ~((bfd_vma) -1 << howto->bitsize);
          /* Fall through. */

	case R_QDSP6_LO16:
	case R_QDSP6_HI16:
          is_abs = TRUE;

          ioffset = (relocation + rel->r_addend) & lmask & rmask;

	  insn = qdsp6_get_insn (input_bfd, howto, contents + rel->r_offset);

	  if (!qdsp6_reloc_operand (howto, &insn, ioffset, NULL)
              && (!h || h->root.type != bfd_link_hash_undefined))
            r = info->callbacks->reloc_overflow
                  (info, h? &h->root: NULL, name, howto->name, 0,
                   input_bfd, input_section, rel->r_offset);
	  else
            qdsp6_put_insn (input_bfd, howto, contents + rel->r_offset, insn);

	  break;

        case R_QDSP6_HL16:
          is_abs = TRUE;

	  ioffset = relocation + rel->r_addend;

          /* First instruction (HI). */
	  insn = qdsp6_get_insn (input_bfd, elf_qdsp6_howto_table + R_QDSP6_HI16,
                                 contents + rel->r_offset + 0);

	  if (!qdsp6_reloc_operand (elf_qdsp6_howto_table + R_QDSP6_HI16, &insn,
                                    ioffset, NULL)
               && (!h || h->root.type != bfd_link_hash_undefined))
            r = info->callbacks->reloc_overflow
                  (info, (h ? &h->root : NULL), name,
                   elf_qdsp6_howto_table [R_QDSP6_HI16].name, 0,
                   input_bfd, input_section, rel->r_offset + 0);
	  else
            qdsp6_put_insn (input_bfd, elf_qdsp6_howto_table + R_QDSP6_HI16,
                            contents + rel->r_offset + 0, insn);

          /* Second instruction (LO). */
	  insn = qdsp6_get_insn (input_bfd, elf_qdsp6_howto_table + R_QDSP6_LO16,
                                 contents + rel->r_offset + sizeof (insn));

	  if (!qdsp6_reloc_operand (elf_qdsp6_howto_table + R_QDSP6_LO16, &insn,
                                    ioffset, NULL)
               && (!h || h->root.type != bfd_link_hash_undefined))
            r = info->callbacks->reloc_overflow
                  (info, (h ? &h->root : NULL), name,
                   elf_qdsp6_howto_table [R_QDSP6_LO16].name, 0,
                   input_bfd, input_section, rel->r_offset + sizeof (insn));
	  else
            qdsp6_put_insn (input_bfd, elf_qdsp6_howto_table + R_QDSP6_LO16,
                            contents + rel->r_offset + sizeof (insn), insn);

          break;

	case R_QDSP6_GPREL16_0:
	case R_QDSP6_GPREL16_1:
	case R_QDSP6_GPREL16_2:
	case R_QDSP6_GPREL16_3:
	  {
	    bfd_vma base;
	    struct bfd_link_hash_entry *bfdhash;

/*
 * Relocation is expressed in absolute terms however GP will
 * point to the base, __default_sda_base__ which must be 512K
 * aligned (0x80000).  Knowing this and that the relocation is
 * GP relative we need to subtract sda_base.
 */

	    insn = qdsp6_get_insn (input_bfd, howto, contents + rel->r_offset);

	    bfdhash = bfd_link_hash_lookup
                        (info->hash, SDA_BASE, FALSE, FALSE, TRUE);
	    if (!bfdhash)
              bfdhash = bfd_link_hash_lookup
                          (info->hash, DEFAULT_SDA_BASE, FALSE, FALSE, TRUE);

	    if (bfdhash
		&& bfdhash->type == bfd_link_hash_defined)
	      {
                base = (bfdhash->u.def.value
                        + bfdhash->u.def.section->output_section->vma
                        + bfdhash->u.def.section->output_offset);
                elf_gp (output_bfd) = base;
		relocation -= base;
	      }
	    else
	      {
		/* At this point, if no _SDA_BASE_, either the alternate or
		 * the proper one, is resolved, then GP-relocaiton overflow
		 * errors are likely.
		 */
	      }

	    ioffset = relocation + rel->r_addend;

	    if (!qdsp6_reloc_operand (howto, &insn, ioffset, NULL)
                && (!h || h->root.type != bfd_link_hash_undefined))
              {
                qdsp6_reloc_operand (howto, &insn, ioffset, NULL);
                r = info->callbacks->reloc_overflow
                      (info, (h? &h->root: NULL), name, howto->name, 0,
                       input_bfd, input_section, rel->r_offset);
              }
	    else
              qdsp6_put_insn (input_bfd, howto, contents + rel->r_offset, insn);

            break;
	  }

        case R_QDSP6_B9_PCREL_X:
        case R_QDSP6_B13_PCREL_X:
        case R_QDSP6_B15_PCREL_X:
        case R_QDSP6_B22_PCREL_X:
        case R_QDSP6_B32_PCREL_X:
          if (r_type == R_QDSP6_B32_PCREL_X)
            rmask = ~(bfd_vma) 0 << howto->rightshift;
          else
            lmask = ~(~(bfd_vma) 0 << howto->bitsize);
          /* Fall through. */

        case R_QDSP6_B7_PCREL:
        case R_QDSP6_B9_PCREL:
	case R_QDSP6_B13_PCREL:
	case R_QDSP6_B15_PCREL:
	case R_QDSP6_B22_PCREL:
          /* These relocations may be used to refer to dynamic symbols, though
             the smaller ones should probably require a trampoline to be safe. */
          is_rel = TRUE;

	  insn = qdsp6_get_insn (input_bfd, howto, contents + rel->r_offset);

          /* Relocation is in absolute terms. */
          ioffset = ((relocation + rel->r_addend)
                     - (input_section->output_section->vma
                        + input_section->output_offset + rel->r_offset))
                    & lmask & rmask;

	  if (!qdsp6_reloc_operand (howto, &insn, ioffset, NULL)
               && (!h || h->root.type != bfd_link_hash_undefined))
            r = info->callbacks->reloc_overflow
                (info, (h ? &h->root : NULL), name,
                 howto->name, (bfd_vma) 0,
                 input_bfd, input_section, rel->r_offset);
	  else
            qdsp6_put_insn (input_bfd, howto, contents + rel->r_offset, insn);

	  break;

        case R_QDSP6_32_PCREL:
	case R_QDSP6_32:
	case R_QDSP6_16:
	case R_QDSP6_8:
          if (r_type == R_QDSP6_32_PCREL)
            is_rel = TRUE;
          else
            is_abs = TRUE;
          /* Fall through. */

	case R_QDSP6_NONE:
          /* Default relocation handling. */
          r = _bfd_final_link_relocate (howto, input_bfd, input_section,
                                        contents, rel->r_offset,
                                        relocation, rel->r_addend);

          if (r != bfd_reloc_ok)
            {
              const char *msg = NULL;

              switch (r)
                {
                case bfd_reloc_overflow:
                  r = info->callbacks->reloc_overflow
                        (info, (h? &h->root: NULL), name,
                         howto->name, (bfd_vma) 0,
                         input_bfd, input_section, rel->r_offset);
                  break;

                case bfd_reloc_undefined:
                  r = info->callbacks->undefined_symbol
                        (info, name, input_bfd, input_section, rel->r_offset,
                         (!info->shared
                          || ELF_ST_VISIBILITY (h->other)));
                  break;

                case bfd_reloc_outofrange:
                  msg = _("internal error: out of range error");
                  break;

                case bfd_reloc_notsupported:
                  msg = _("internal error: unsupported relocation error");
                  break;

                case bfd_reloc_dangerous:
                  msg = _("internal error: dangerous relocation");
                  break;

                default:
                  msg = _("internal error: unknown error");
                  break;
                }

              if (msg)
                r = info->callbacks->warning
                      (info, msg, name, input_bfd, input_section, rel->r_offset);

              if (!r)
                return FALSE;
            }
	  break;

	default:
          info->callbacks->warning
            (info, _("internal error: unrecognized relocation type"),
             name, input_bfd, input_section, rel->r_offset);
          return FALSE;
	}


      /* Absolute relocations must be applied at load-time and relative
          ones must be applied then only if to undefined symbols. */
      if (info->shared)
        {
          /* Some relocations must be copied to the output file to be applied
             when the DSO is loaded. */
          Elf_Internal_Rela outrel;

          outrel.r_offset =
            _bfd_elf_section_offset
              (output_bfd, info, input_section, rel->r_offset);

          if (h
              && h->dynindx != -1
              && (h->root.type == bfd_link_hash_defined
                  || h->root.type == bfd_link_hash_defweak))
            {
              /* This symbol is globally defined. */
              outrel.r_info = ELF32_R_INFO (0, r_type);
              outrel.r_offset += input_section->output_section->vma
                                 + input_section->output_offset;
              outrel.r_addend = relocation + rel->r_addend;
            }
          else if (h
                   && h->dynindx != -1
                   && (h->root.type == bfd_link_hash_undefined
                       || h->root.type == bfd_link_hash_undefweak))
            {
              /* This symbol is undefined. */
              outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
              outrel.r_offset += input_section->output_section->vma
                                 + input_section->output_offset;
              outrel.r_addend = rel->r_addend;
            }
          else
            {
              /* This symbol is local, or marked to become local.  */
              outrel.r_info = ELF32_R_INFO (0, r_type);
              outrel.r_offset += input_section->output_section->vma
                                 + input_section->output_offset;
              outrel.r_addend = relocation + rel->r_addend;
            }

          sreloc = elf_section_data (input_section)->sreloc;
          if (!sreloc)
            {
              const char *name;

              name = bfd_elf_string_from_elf_section
                       (input_bfd,
                        elf_elfheader (input_bfd)->e_shstrndx,
                        elf_section_data (input_section)->rel_hdr.sh_name);
              if (!name)
                return FALSE;

              BFD_ASSERT (!strncmp (name, ".rela", 5)
                          && !strcmp
                                (bfd_get_section_name (input_bfd, input_section),
                                 name + 5));

              sreloc = elf_section_data (input_section)->sreloc =
                bfd_get_section_by_name (htab->elf.dynobj, name);
              BFD_ASSERT (sreloc);
            }

          if (sreloc->contents)
            if (is_abs || (is_rel && is_und))
              {
                bfd_byte *loc;

                loc = sreloc->contents
                      + sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
                bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
              }
        }
    }

  return TRUE;
}

static bfd_boolean
qdsp6_elf_relax_section (bfd *input_bfd,
                         asection *isec,
                         struct bfd_link_info *link_info,
                         bfd_boolean *again)
{
  Elf_Internal_Shdr *symtab_hdr = NULL;
  Elf_Internal_Rela *irelbuf = NULL;
  Elf_Internal_Rela *irel;
  unsigned int ireloc, creloc;
  bfd_byte *contents = NULL;
  Elf_Internal_Sym *isymbuf = NULL;
  struct elf_link_hash_entry **sym_hashes;
  unsigned long r_symndx;
  struct elf_link_hash_entry *h;
  struct bfd_link_hash_entry *t_h;
  struct bfd_link_hash_table t_hash;
  bfd_boolean rc = TRUE;
  bfd_size_type isec_size = 0;
  char *name, *t_name;
  qdsp6_insn insn;
  size_t i = 0, j = 0;

  *again = FALSE;

  if (link_info->relocatable
      || (isec->flags & SEC_RELOC) == 0
      || isec->reloc_count == 0)
    return rc;

  /* If needed, initialize this section's cooked size.  */
  if (isec->size == 0)
    isec->size = isec->rawsize;

  BFD_ASSERT (bfd_hash_table_init ((struct bfd_hash_table *) &t_hash,
                                   _bfd_link_hash_newfunc,
				   sizeof (struct bfd_link_hash_table)));


  /* It's quite hard to get rid of the relocation table once it's been read.
     Ideally, any relocations required by the trampoline should be added to it,
     but it seems that everything falls off if the table is changed in any way.
     Since the original relocation is voided, it and only it may be reused by
     the trampoline.
     TODO: instead of keeping the memory around, perhaps manually setting
           "elf_section_data (o)->relocs" if there's any change would use up
           less memory. */
  irelbuf = _bfd_elf_link_read_relocs (input_bfd, isec, NULL, NULL,
                                         link_info->keep_memory);
  if (!irelbuf)
    goto error_return;

  for (ireloc = 0; ireloc < isec->reloc_count; ireloc++)
    {
      bfd_vma at_base, to_base, t_base;
      bfd_vma at, t_at, from, to;
      bfd_signed_vma ioffset;
      bfd_vma r_type;
      bfd_boolean is_def;

      irel = irelbuf + ireloc;

      /* Look into relocation overflows at branches and add trampolines if needed. */
      r_type = ELF32_R_TYPE (irel->r_info);
      if (link_info->qdsp6_trampolines
          && (r_type == R_QDSP6_B22_PCREL
              || r_type == R_QDSP6_B15_PCREL
              || r_type == R_QDSP6_B13_PCREL
              || r_type == R_QDSP6_B9_PCREL))
        {
          isec_size = bfd_section_size (input_bfd, isec);

          at      = irel->r_offset;
          at_base = isec->output_section->vma + isec->output_offset;
          if (at >= isec_size)
	    {
/*
	      (*_bfd_error_handler) (_("%s: 0x%lx: warning: bad relocation"),
                                     bfd_archive_filename (input_bfd),
                                     (unsigned long) irel->r_offset);
*/
	      continue;
	    }
          /* This will do for now, but it must be corrected since the addendum
             is also offset by the position of the branch in the packet. */
          from = irel->r_offset;

          symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
          r_symndx   = ELF32_R_SYM (irel->r_info);
          sym_hashes = elf_sym_hashes (input_bfd);

          /* Read this BFD's symbols if we haven't done so already. */
          if (isymbuf == NULL && symtab_hdr->sh_info != 0)
	    {
	      isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	      if (isymbuf == NULL)
	        isymbuf = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
					        symtab_hdr->sh_info, 0,
					        NULL, NULL, NULL);
	      if (isymbuf == NULL)
	        goto error_return;
	    }

          /* Get the value of the symbol referred to by the reloc.  */
          if (r_symndx < symtab_hdr->sh_info)
	    {
	      /* A local symbol.  */
              static size_t l_count;
	      Elf_Internal_Sym *isym;
              asection *asec;

	      isym = isymbuf + ELF32_R_SYM (irel->r_info);
              asec = bfd_section_from_elf_index (input_bfd, isym->st_shndx);

              name = bfd_malloc (sizeof (l_count) * 2 + 1);
              sprintf (name, "%0*lx", (int) sizeof (l_count) * 2, (long)l_count++);

              is_def = TRUE;

              to      = isym->st_value;
              to_base = asec? (asec->output_section->vma + asec->output_offset): 0;
	    }
	  else
	    {
              /* A global symbol. */
	      h = sym_hashes [r_symndx - symtab_hdr->sh_info];
	      while (h && (h->root.type == bfd_link_hash_indirect
                           || h->root.type == bfd_link_hash_warning))
	        h = (struct elf_link_hash_entry *) h->root.u.i.link;

	      if (!h)
	        continue;

              name = (char *) h->root.root.string;

              switch (h->root.type)
                {
                  case bfd_link_hash_defined:
                  case bfd_link_hash_defweak:
                    is_def = TRUE;

                    to      = h->root.u.def.value;
                    to_base = h->root.u.def.section->output_section->vma
                              + h->root.u.def.section->output_offset;
                    break;

                  case bfd_link_hash_undefined:
                  case bfd_link_hash_undefweak:
                    if (link_info->shared)
                      {
                        /* Force trampolines for undefined symbols in dynamic objects. */
                        is_def = FALSE;

                        to = to_base = 0;
                      }
                    else
                      continue;
                    break;

                  default:
                    continue;
                    break;
                }
            }

          /* Check if the target is beyond reach. */
          ioffset = abs ((to + to_base) - (from + at_base));
          if ((is_def && (((r_type == R_QDSP6_B22_PCREL)
                                 && QDSP6_TRAMPLINE_NEEDED (ioffset, 24))
                             || ((r_type == R_QDSP6_B15_PCREL)
                                 && QDSP6_TRAMPLINE_NEEDED (ioffset, 17))
                             || ((r_type == R_QDSP6_B13_PCREL)
                                 && QDSP6_TRAMPLINE_NEEDED (ioffset, 15))
                             || ((r_type == R_QDSP6_B9_PCREL)
                                 && QDSP6_TRAMPLINE_NEEDED (ioffset, 11))))
              || !is_def)
	    {
              /* Try to add a trampoline. */
/*
              printf ("Trampoline required at: %s (%s+0x%lx) for `%s'\n",
                      input_bfd->filename, isec->name, at, h->root.root.string);
*/

              /* Allocate new contents. */
              contents = elf_section_data (isec)->this_hdr.contents;
	      if (contents == NULL)
	      {
	          contents = bfd_malloc (isec_size);
	          if (contents == NULL)
		    goto error_return;

	          if (!bfd_get_section_contents (input_bfd, isec, contents,
					          (file_ptr) 0,
					          isec_size))
		    goto error_return;

                elf_section_data (isec)->this_hdr.contents = contents;
	      }

            /* Subtract the offset of the branch into the packet from
               the addendum. */
            for (;
                 from >= QDSP6_INSN_LEN;
                 from -= QDSP6_INSN_LEN, irel->r_addend -= QDSP6_INSN_LEN)
              {
                qdsp6_insn insn;

                insn = qdsp6_get_insn (input_bfd, elf_qdsp6_howto_table + r_type,
                                       contents + from - QDSP6_INSN_LEN);
                if (QDSP6_END_PACKET_GET (insn) == QDSP6_END_PACKET
                    || QDSP6_END_PACKET_GET (insn) == QDSP6_END_PAIR)
                  break;
              }

              /* Create a symbol for the trampoline. */
              t_name = bfd_malloc (QDSP6_TRAMPOLINE_PREFIX_LEN + 1
                                   + strlen (name) + 1);
              sprintf (t_name, "%s_%s", QDSP6_TRAMPOLINE_PREFIX, name);

              /* Try to find it, otherwise, create it. */
              t_h = bfd_link_hash_lookup (&t_hash, t_name, FALSE, FALSE, FALSE);
              if (!t_h)
                {
                  t_at = isec_size;

                  if (((r_type == R_QDSP6_B22_PCREL)
                          && QDSP6_TRAMPLINE_NEEDED (t_at - from, 23))
                      || ((r_type == R_QDSP6_B15_PCREL)
                          && QDSP6_TRAMPLINE_NEEDED (t_at - from, 16))
                      || ((r_type == R_QDSP6_B13_PCREL)
                          && QDSP6_TRAMPLINE_NEEDED (t_at - from, 14))
                      || ((r_type == R_QDSP6_B9_PCREL)
                          && QDSP6_TRAMPLINE_NEEDED (t_at - from, 10)))
                    /* No room for a trampoline. */
                    goto error_return;

                  isec_size += sizeof (qdsp6_trampoline);

                  /* Add room for the trampoline. */
                  elf_section_data (isec)->this_hdr.contents = contents =
                    bfd_realloc (contents, isec_size);

                  rc = bfd_set_section_size (input_bfd, isec, isec_size);
                  if (rc != TRUE)
                    goto error_return;

                  isec_size = get_section_size_now (input_bfd, isec);

                  /* Create the trampoline symbol. */
                  BFD_ASSERT
                    (t_h = bfd_link_hash_lookup (&t_hash, t_name, TRUE, TRUE, FALSE));
                  t_h->u.def.value   = t_at;
                  t_h->u.def.section = isec;

                  /* Add trampoline at the end of the section. */
                  for (i = j = 0;
                       i < sizeof (qdsp6_trampoline);
                       i += sizeof (*qdsp6_trampoline), j++)
                    bfd_put_32 (input_bfd, qdsp6_trampoline [j],
                                contents + t_at + i);

                  /* Add relocations for the trampoline. */
                  creloc = sizeof (qdsp6_trampoline_rels)
                           / sizeof (qdsp6_trampoline_rels [0]);
                  if (creloc > 0)
                    {
                      creloc = 0;
                        {
                          /* Reuse the original relocation. */
                          irel->r_info
                            = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
                                            qdsp6_trampoline_rels [creloc].rtype);
                          irel->r_offset = t_h->u.def.value
                                           + qdsp6_trampoline_rels [creloc].offset;
                          /* The relocation addendum remains the same. */
                        }
                    }
                }
              else
                /* Remove the offending relocation. */
                /* !!!Could this relocation be zeroed up??? */
                irel->r_info =
                  ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_QDSP6_NONE);

              free (t_name);

              /* Get the effective address of the trampoline. */
              t_at   = t_h->u.def.value;
              t_base = t_h->u.def.section->vma +
                       + t_h->u.def.section->output_offset;

              /* Fix up the offending branch by pointing it to the trampoline. */
              insn = qdsp6_get_insn (input_bfd,
                                     elf_qdsp6_howto_table + r_type,
                                     contents + at);
              if (qdsp6_reloc_operand (elf_qdsp6_howto_table + r_type, &insn,
                                       t_at - from, NULL))
                qdsp6_put_insn (input_bfd, elf_qdsp6_howto_table + r_type,
                                contents + at, insn);

              /* Done adding the trampolines.
                 Relax again in case other branches were pushed out of range. */
	      *again = TRUE;
	    }
        }
    }

  ok_return:
  if (isymbuf && symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (contents && elf_section_data (isec)->this_hdr.contents != contents)
    free (contents);
  if (irelbuf && elf_section_data (isec)->relocs != irelbuf)
    free (irelbuf);

  bfd_hash_table_free ((struct bfd_hash_table *) &t_hash);
  return (rc);

  error_return:
  rc = FALSE;
  goto ok_return;
}

/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */

static bfd_boolean
qdsp6_elf_check_relocs
(bfd *abfd, struct bfd_link_info *info,
 asection *sec, const Elf_Internal_Rela *relocs)
{
  qdsp6_elf_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  asection *sreloc;

  if (info->relocatable)
    return TRUE;

#if 0
  /* Create the linker generated sections all the time so that the
     special symbols are created.  */
  if (!elf_hash_table (info)->dynamic_sections_created)
    {
      if (!qdsp6_elf_create_dynamic_sections (abfd, info))
	return FALSE;
    }
#endif

  htab = qdsp6_elf_hash_table (info);

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  sreloc = NULL;

  for (rel = relocs; rel < relocs + sec->reloc_count; rel++)
    {
      size_t r_symndx;
      struct elf_link_hash_entry *h;
      bfd_boolean is_abs = FALSE, is_rel = FALSE;
      bfd_boolean is_und = FALSE, is_loc = FALSE;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
        {
          /* Symbol is local. */
          h = NULL;

          is_loc = TRUE;
        }
      else
        {
          /* Symbol is global. */
          h = sym_hashes [r_symndx - symtab_hdr->sh_info];
          while (h->root.type == bfd_link_hash_indirect
                 || h->root.type == bfd_link_hash_warning)
            h = (struct elf_link_hash_entry *) h->root.u.i.link;

          is_und = (h->root.type == bfd_link_hash_undefweak
                    || h->root.type == bfd_link_hash_undefined);
        }

      switch (ELF32_R_TYPE (rel->r_info))
	{
	case R_QDSP6_B9_PCREL:
	case R_QDSP6_B13_PCREL:
	case R_QDSP6_B15_PCREL:
	case R_QDSP6_B22_PCREL:
        case R_QDSP6_32_PCREL:
          is_rel = TRUE;
          break;

        case R_QDSP6_HL16:
        case R_QDSP6_HI16:
        case R_QDSP6_LO16:
        case R_QDSP6_32:
        case R_QDSP6_16:
        case R_QDSP6_8:
          is_abs = TRUE;
          break;
        }

      if (h && !info->shared)
        {
          /* We may need a copy reloc too.  */
          h->non_got_ref = 1;
          h->pointer_equality_needed = 1;
        }

      /* If we are creating a shared library, and this is a reloc
        against a global symbol, or a non PC relative reloc
        against a local symbol, then we need to copy the reloc
        into the shared library.  However, if we are linking with
        -Bsymbolic, we do not need to copy a reloc against a
        global symbol which is defined in an object we are
        including in the link (i.e., DEF_REGULAR is set).  At
        this point we have not seen all the input files, so it is
        possible that DEF_REGULAR is not set now but will be set
        later (it is never cleared).  In case of a weak definition,
        DEF_REGULAR may be cleared later by a strong definition in
        a shared library.  We account for that possibility below by
        storing information in the dyn_relocs field of the hash
        table entry.  A similar situation occurs when creating
        shared libraries and symbol visibility changes render the
        symbol local.

        If on the other hand, we are creating an executable, we
        may need to keep relocations for symbols satisfied by a
        dynamic library if we manage to avoid copy relocs for the
        symbol.  */
      if ((info->shared
              && (is_abs
                  || (is_rel && is_und)))
          || (ELIMINATE_COPY_RELOCS
              && !info->shared
              && htab->elf.dynobj
              && (sec->flags & SEC_ALLOC)
              && (h
                  && (h->root.type == bfd_link_hash_defweak
                      || !h->def_regular))))
        {
          qdsp6_elf_dyn_reloc *p;
          qdsp6_elf_dyn_reloc **head;

          sreloc = elf_section_data (sec)->sreloc;
          if (!sreloc)
            {
              const char *name;

              name = bfd_elf_string_from_elf_section
                      (abfd,
                       elf_elfheader (abfd)->e_shstrndx,
                       elf_section_data (sec)->rel_hdr.sh_name);
              if (!name)
                return FALSE;

              BFD_ASSERT (!strncmp (name, ".rela", 5)
                          && !strcmp (bfd_get_section_name (abfd, sec), name + 5));

              if (!htab->elf.dynobj)
                htab->elf.dynobj = abfd;

              sreloc = bfd_get_section_by_name (htab->elf.dynobj, name);
              if (!sreloc)
                {
                  flagword flags;

                  flags = (SEC_HAS_CONTENTS | SEC_READONLY
                             | SEC_IN_MEMORY | SEC_LINKER_CREATED)
                          | ((sec->flags & SEC_ALLOC)? SEC_ALLOC | SEC_LOAD: 0);

                  sreloc = bfd_make_section (htab->elf.dynobj, name);
                  if (!sreloc
                      || !bfd_set_section_flags (htab->elf.dynobj, sreloc, flags)
                      || !bfd_set_section_alignment (htab->elf.dynobj, sreloc, 2))
                    return FALSE;
                }
              elf_section_data (sec)->sreloc = sreloc;
            }

          /* If this is a global symbol, we count the number of
            relocations we need for this symbol.  */
          if (h)
            {
              head = &(qdsp6_hash_entry (h)->dyn_relocs);
            }
          else
            {
              /* Track dynamic relocs needed for local syms too.
                We really need local syms available to do this
                easily.  Oh well.  */
              asection *s;
              void *vpp;
              Elf_Internal_Sym *isym;

              isym = bfd_sym_from_r_symndx (&htab->sym_cache,
                                         abfd, r_symndx);
              if (!isym)
                return FALSE;

              s = bfd_section_from_elf_index (abfd, isym->st_shndx);
              if (s == NULL)
                s = sec;

              vpp = &elf_section_data (s)->local_dynrel;
              head = (qdsp6_elf_dyn_reloc **) vpp;
            }

          if (!(p = *head) || p->sec != sec)
            {
              if (!(p = bfd_zalloc (htab->elf.dynobj, sizeof *p)))
                return FALSE;

              p->next = *head;
              p->sec = sec;

              *head = p;
            }
          p->count++;
        }
    }

  return TRUE;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
qdsp6_elf_copy_indirect_symbol (struct bfd_link_info *info,
                                struct elf_link_hash_entry *dir,
                                struct elf_link_hash_entry *ind)
{
  qdsp6_elf_link_hash_entry *edir, *eind;

  edir = (qdsp6_elf_link_hash_entry *) dir;
  eind = (qdsp6_elf_link_hash_entry *) ind;

  if (eind->dyn_relocs != NULL)
    {
      if (edir->dyn_relocs != NULL)
	{
	  qdsp6_elf_dyn_reloc **pp;
	  qdsp6_elf_dyn_reloc *p;

	  if (ind->root.type == bfd_link_hash_indirect)
	    abort ();

	  /* Add reloc counts against the weak sym to the strong sym
	     list.  Merge any entries against the same section.  */
	  for (pp = &eind->dyn_relocs; (p = *pp) != NULL; )
	    {
	      qdsp6_elf_dyn_reloc *q;

	      for (q = edir->dyn_relocs; q != NULL; q = q->next)
		if (q->sec == p->sec)
		  {
		    q->count += p->count;
		    *pp = p->next;
		    break;
		  }
	      if (q == NULL)
		pp = &p->next;
	    }
	  *pp = edir->dyn_relocs;
	}

      edir->dyn_relocs = eind->dyn_relocs;
      eind->dyn_relocs = NULL;
    }

    /* If called to transfer flags for a weakdef during processing
       of elf_adjust_dynamic_symbol, don't copy ELF_LINK_NON_GOT_REF.
       We clear it ourselves for ELIMINATE_COPY_RELOCS.  */
  if (!(ELIMINATE_COPY_RELOCS
        && ind->root.type != bfd_link_hash_indirect
        && edir->elf.dynamic_adjusted))
      edir->elf.non_got_ref |= eind->elf.non_got_ref;

  edir->elf.ref_dynamic |= eind->elf.ref_dynamic;
  edir->elf.ref_regular |= eind->elf.ref_regular;
  edir->elf.ref_regular_nonweak |= eind->elf.ref_regular_nonweak;
  edir->elf.needs_plt |= eind->elf.needs_plt;
  edir->elf.pointer_equality_needed |= eind->elf.pointer_equality_needed;
  /* If we were called to copy over info for a weak sym, that's all.  */
  if (eind->elf.root.type != bfd_link_hash_indirect)
    return;

  /* Copy over the GOT refcount entries that we may have already seen to
     the symbol which just became indirect.  */
  edir->elf.got.refcount += eind->elf.got.refcount;
  eind->elf.got.refcount = 0;

  if (eind->elf.dynindx != -1)
    {
      if (edir->elf.dynindx != -1)
        _bfd_elf_strtab_delref (elf_hash_table (info)->dynstr,
                                edir->elf.dynstr_index);
      edir->elf.dynindx = eind->elf.dynindx;
      edir->elf.dynstr_index = eind->elf.dynstr_index;
      eind->elf.dynindx = -1;
      eind->elf.dynstr_index = 0;
    }

}

/* Return true if we have dynamic relocs that apply to read-only sections.  */

static bfd_boolean
readonly_dynrelocs (struct elf_link_hash_entry *h)
{
  qdsp6_elf_dyn_reloc *p;

  for (p = qdsp6_elf_hash_entry (h)->dyn_relocs;
	p != NULL;
	p = p->next)
    {
      asection *s = p->sec->output_section;

      if (s != NULL
          && ((s->flags & (SEC_READONLY | SEC_ALLOC))
              == (SEC_READONLY | SEC_ALLOC)))
        return TRUE;
    }
  return FALSE;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bfd_boolean
qdsp6_elf_adjust_dynamic_symbol
(struct bfd_link_info *info, struct elf_link_hash_entry *h)
{
  qdsp6_elf_link_hash_table *htab;
  asection *sec = (asection *)0;

  /* Make sure we know what is going on here.  */
  htab = qdsp6_elf_hash_table (info);
  BFD_ASSERT (htab->elf.dynobj != NULL
              && (h->needs_plt
                  || h->type == STT_GNU_IFUNC
                  || h->u.weakdef != NULL
                  || (h->def_dynamic
                      && h->ref_regular
                      && !h->def_regular)));


  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->u.weakdef)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
		  || h->u.weakdef->root.type == bfd_link_hash_defweak);

      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;

      if (ELIMINATE_COPY_RELOCS)
        h->non_got_ref = h->u.weakdef->non_got_ref;

      return TRUE;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (info->shared)
    return TRUE;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return TRUE;

   /* If we didn't find any dynamic relocs in read-only sections, then
      we'll be keeping the dynamic relocs and avoiding the copy reloc.
      We can't do this if there are any small data relocations.  */
  if (ELIMINATE_COPY_RELOCS
      && !h->def_regular
      && !readonly_dynrelocs (h))
    {
      h->non_got_ref = 0;
      return TRUE;
    }

  if (h->size == 0)
    {
      (*_bfd_error_handler) (_("dynamic variable `%s' is zero size"),
			     h->root.root.string);
      return TRUE;
    }

  return _bfd_elf_adjust_dynamic_copy (h, sec);

}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bfd_boolean
qdsp6_elf_finish_dynamic_symbol
(bfd *output_bfd ATTRIBUTE_UNUSED, struct bfd_link_info *info,
 struct elf_link_hash_entry *h, Elf_Internal_Sym *sym)
{
  qdsp6_elf_link_hash_table *htab;

  htab = qdsp6_elf_hash_table (info);

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (!strcmp (h->root.root.string, "_DYNAMIC")
      || !strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_"))
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

/* Allocate space in associated reloc sections for dynamic relocs.  */

static bfd_boolean
qdsp6_alloc_dynrel
(struct elf_link_hash_entry *h, struct bfd_link_info *info)
{
  qdsp6_elf_link_hash_entry *eh;
  qdsp6_elf_link_hash_table *htab;
  qdsp6_elf_dyn_reloc *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  htab = qdsp6_elf_hash_table (info);

  eh = (qdsp6_elf_link_hash_entry *) h;
  if (!eh->dyn_relocs)
    return TRUE;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for relocs that have become local due to symbol visibility
     changes.  */
  if (info->shared)
    {
      if ((h->def_regular)
	  && ((h->forced_local)
	      || info->symbolic))
	{
	  qdsp6_elf_dyn_reloc **pp;

	  for (pp = &eh->dyn_relocs; (p = *pp); )
	    {
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}
    }
  else if (ELIMINATE_COPY_RELOCS)
    {
      /* For the non-shared case, discard space for relocs against
	 symbols which turn out to need copy relocs or are not
	 dynamic.  */

      if (!h->non_got_ref
	  && (h->def_dynamic) /* XXX_SM what about fpic executables */
	  && !(h->def_regular))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if ((h->dynindx == -1) && (!h->forced_local))
	    {
	      if (!bfd_elf_link_record_dynamic_symbol (info, h))
		return FALSE;
	    }
	}

      /* If that succeeded, we know we'll be keeping all the
          relocs.  */
      if (h->dynindx == -1)
        eh->dyn_relocs = NULL;
    }

  /* Finally, allocate space.  */
  for (p = eh->dyn_relocs; p; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;

      sreloc->size += p->count * sizeof (Elf32_External_Rela); /* !!!_raw_size perhaps? */
    }

  return TRUE;
}

/* Find any dynamic relocs that apply to read-only sections.  */

static bfd_boolean
qdsp6_readonly_dynrel
(struct elf_link_hash_entry *h, struct bfd_link_info *info)
{
  qdsp6_elf_dyn_reloc *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  for (p = qdsp6_hash_entry (h)->dyn_relocs; p; p = p->next)
    {
      asection *s = p->sec->output_section;

      if (s && ((s->flags & (SEC_READONLY | SEC_ALLOC))))
	{
	  info->flags |= DF_TEXTREL;

	  /* Not an error, just cut short the traversal.  */
	  return FALSE;
	}
    }
  return TRUE;
}

static bfd_boolean
qdsp6_elf_create_dynamic_sections
(bfd *abfd, struct bfd_link_info *info)
{
  asection *s;
  flagword flags;

  flags = SEC_ALLOC | SEC_LOAD | SEC_LINKER_CREATED;

  if (!(_bfd_elf_create_dynamic_sections (abfd, info)))
    return FALSE;

  /* Make sure that the .dynamic section is read-only, for somehow it's not
     when it's created in elf_link_create_dynamic_sections (). */
  s = bfd_get_section_by_name (abfd, ".dynamic");
  if (s)
    bfd_set_section_flags (abfd, s, bfd_get_section_flags (abfd, s) | SEC_READONLY);

  /*
  s = bfd_make_section (abfd, ".rela.sdata");
  if (!s
      || !bfd_set_section_flags (abfd, s, flags)
      || !bfd_set_section_alignment (abfd, s, 2))
    return FALSE;
  */

  return TRUE;
}

/* Set the sizes of the dynamic sections.  */

static bfd_boolean
qdsp6_elf_size_dynamic_sections
(bfd *output_bfd ATTRIBUTE_UNUSED, struct bfd_link_info *info)
{
  qdsp6_elf_link_hash_table *htab;
  asection *s;
  bfd_boolean relocs;
  bfd *ibfd;

  htab = qdsp6_elf_hash_table (info);
  BFD_ASSERT (htab->elf.dynobj);

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
        {
          s = bfd_get_section_by_name (htab->elf.dynobj, ".interp");
          BFD_ASSERT (s != NULL);
          s->size = sizeof ELF_DYNAMIC_INTERPRETER;
          s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
        }
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link_next)
    {
      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  qdsp6_elf_dyn_reloc *p;

	  for (p = (qdsp6_elf_dyn_reloc *) elf_section_data (s)->local_dynrel;
	       p;
	       p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (p->count != 0)
		{
		  elf_section_data (p->sec)->sreloc->size
		    += p->count * sizeof (Elf32_External_Rela);
		  if ((p->sec->output_section->flags & (SEC_READONLY | SEC_ALLOC))
		      == (SEC_READONLY | SEC_ALLOC))
		    info->flags |= DF_TEXTREL;
		}
	    }
	}
    }

  /* Allocate space for global sym dynamic relocs.  */
  elf_link_hash_traverse (elf_hash_table (info), qdsp6_alloc_dynrel, info);

  /* We've now determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = FALSE;
  for (s = htab->elf.dynobj->sections; s != NULL; s = s->next)
    {
      if (!strncmp (bfd_get_section_name (dynobj, s), ".rela", 5))
	{
	  if (s->size == 0)
	    {
	      /* If we don't need this section, strip it from the
		 output file.  This is mostly to handle .rela.bss and
		 .rela.plt.  We must create both sections in
		 create_dynamic_sections, because they must be created
		 before the linker maps input sections to output
		 sections.  The linker does that before
		 adjust_dynamic_symbol is called, and it is that
		 function which decides whether anything needs to go
		 into these sections.  */
	    }
	  else
	    {
	      /* Remember whether there are any relocation sections.  */
	      relocs = TRUE;

	      /* We use the reloc_count field as a counter if we need
		 to copy relocs into the output file.  */
	      s->reloc_count = 0;
	    }
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
          /* If we don't need this section, strip it from the
             output file. */
	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      /* Allocate memory for the section contents.  */
      s->contents = (bfd_byte *) bfd_zalloc (htab->elf.dynobj, s->size);
      if (!s->contents)
	return FALSE;
    }

  if (htab->elf.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
	 values later, in qdsp6_elf_finish_dynamic_sections, but we
	 must add the entries now so that we get the correct size for
	 the .dynamic section.  The DT_DEBUG entry is filled in by the
	 dynamic linker and used by the debugger.  */

      if (!info->shared)
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_DEBUG, 0))
	    return FALSE;
	}

      if (relocs)
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_RELA, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_RELASZ, 0)
	      || !_bfd_elf_add_dynamic_entry
                    (info, DT_RELAENT, sizeof (Elf32_External_Rela)))
	    return FALSE;
	}

      /* If any dynamic relocs apply to a read-only section, then we
	 need a DT_TEXTREL entry.  */
      if ((info->flags & DF_TEXTREL) == 0)
	elf_link_hash_traverse (elf_hash_table (info), qdsp6_readonly_dynrel, info);

      if ((info->flags & DF_TEXTREL) != 0)
	{
	  if (!_bfd_elf_add_dynamic_entry (info, DT_TEXTREL, 0))
	    return FALSE;
	}

      if (TRUE)
        {
          if (!_bfd_elf_add_dynamic_entry (info, DT_QDSP6_SYMSZ, 0))
            return FALSE;
        }
    }
  return TRUE;
}

/* Finish up the dynamic sections.  */

static bfd_boolean
qdsp6_elf_finish_dynamic_sections
(bfd *output_bfd, struct bfd_link_info *info)
{
  qdsp6_elf_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;

  htab = qdsp6_elf_hash_table (info);
  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      Elf32_External_Dyn *dyncon, *dynconend;

      BFD_ASSERT (sdyn);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  /* Read in the current dynamic entry.  */
	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    case DT_QDSP6_SYMSZ:
	      s = bfd_get_section_by_name (output_bfd, ".dynsym");
	      BFD_ASSERT (s);

              dyn.d_un.d_val = get_section_size_now (output_bfd, s);
	      break;
	    }

	  bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
	}
    }

  return TRUE;
}

/* Create an entry in a PPC ELF linker hash table.  */

static struct bfd_hash_entry *
qdsp6_elf_link_hash_newfunc
(struct bfd_hash_entry *entry, struct bfd_hash_table *table, const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (!entry
      && !(entry = bfd_hash_allocate (table, sizeof (*qdsp6_hash_entry (entry)))))
    return entry;

  /* Call the allocation method of the superclass.  */
  if ((entry = _bfd_elf_link_hash_newfunc (entry, table, string)))
    {
      qdsp6_hash_entry (entry)->dyn_relocs = NULL;
    }

  return (entry);
}

/* Create an ELF linker hash table.  */

static struct bfd_link_hash_table *
qdsp6_elf_link_hash_table_create
(bfd *abfd)
{
  qdsp6_elf_link_hash_table *htab;

  if (!(htab = bfd_zmalloc (sizeof (*htab))))
    return NULL;

if (!_bfd_elf_link_hash_table_init (&htab->elf, abfd,
                                    qdsp6_elf_link_hash_newfunc,
                                    sizeof (struct _qdsp6_elf_link_hash_entry)))

    {
      free (htab);
      return NULL;
    }

  return (&htab->elf.root);
}

/* We may need to bump up the number of program headers beyond .text and .data. */

static int
qdsp6_elf_additional_program_headers (bfd *abfd, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  asection *s;
  int ret = 0;

  /* In a good family, it would be merged with text. */
  s = bfd_get_section_by_name (abfd, ".hash");
  if (s && (s->flags & SEC_LOAD))
    ret++;

  /* Just in case, as used only by stand-alone mode. */
  s = bfd_get_section_by_name (abfd, ".start");
  if (s && (s->flags & SEC_LOAD))
    ret++;

  s = bfd_get_section_by_name (abfd, ".text");
  if (s && (s->flags & SEC_LOAD))
    ret++;

  s = bfd_get_section_by_name (abfd, ".rodata");
  if (s && (s->flags & SEC_LOAD))
    ret++;

  /* The SDA shows up even if it's empty. */
  s = bfd_get_section_by_name (abfd, ".sdata");
  if (s && (s->flags & SEC_LOAD))
    ret++;

  return ret;
}

/* Check if section can be ignored for the purpose of checking for
   dangling relocations against discarded sections. */

static bfd_boolean
qdsp6_elf_ignore_discarded_relocs (asection *s ATTRIBUTE_UNUSED)
{
  /* Always ignore because relocate_section () will get rid of
     dangling relocations. */
  return TRUE;
}

#define TARGET_LITTLE_SYM  bfd_elf32_littleqdsp6_vec
#define TARGET_LITTLE_NAME "elf32-littleqdsp6"
#define TARGET_BIG_SYM     bfd_elf32_bigqdsp6_vec
#define TARGET_BIG_NAME    "elf32-bigqdsp6"
#define ELF_ARCH           bfd_arch_qdsp6
#define ELF_MACHINE_CODE   EM_QDSP6
#define ELF_MAXPAGESIZE    0x1000

/* We need to use RELAs to get the computations for the HI16/LO16
   relocations to be correct in the presence of addends. */
#define elf_backend_may_use_rel_p	0
#define	elf_backend_may_use_rela_p	1
#define	elf_backend_default_use_rela_p	1
#define elf_backend_rela_normal         1

#define elf_backend_can_gc_sections     1

#define elf_backend_plt_readonly        1

#define elf_info_to_howto               0
#define bfd_elf32_bfd_reloc_type_lookup qdsp6_elf_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup qdsp6_elf_reloc_name_lookup
#define elf_info_to_howto_rel           qdsp6_info_to_howto_rel

#define elf_backend_object_p            qdsp6_elf_object_p
#define elf_backend_gc_sweep_hook       qdsp6_elf_gc_sweep_hook
#define elf_backend_check_relocs        qdsp6_elf_check_relocs
#define elf_backend_relocate_section    qdsp6_elf_relocate_section
#define elf_backend_final_write_processing \
                                        qdsp6_elf_final_write_processing
#define elf_backend_section_from_shdr   qdsp6_elf_section_from_shdr
#define elf_backend_add_symbol_hook     qdsp6_elf_add_symbol_hook
#define elf_backend_fake_sections       qdsp6_elf_fake_sections
#define elf_backend_symbol_processing   qdsp6_elf_symbol_processing
#define elf_backend_section_processing  qdsp6_elf_section_processing
#define elf_backend_section_from_bfd_section  qdsp6_elf_section_from_bfd_section
#define elf_backend_link_output_symbol_hook \
                                        qdsp6_elf_link_output_symbol_hook
#define elf_backend_copy_indirect_symbol \
                                        qdsp6_elf_copy_indirect_symbol
#define elf_backend_adjust_dynamic_symbol \
                                        qdsp6_elf_adjust_dynamic_symbol
#define elf_backend_finish_dynamic_symbol \
                                        qdsp6_elf_finish_dynamic_symbol
#define elf_backend_create_dynamic_sections \
                                        qdsp6_elf_create_dynamic_sections
#define elf_backend_size_dynamic_sections \
                                        qdsp6_elf_size_dynamic_sections
#define elf_backend_finish_dynamic_sections \
                                        qdsp6_elf_finish_dynamic_sections
#define elf_backend_additional_program_headers \
                                        qdsp6_elf_additional_program_headers
#define elf_backend_ignore_discarded_relocs \
                                        qdsp6_elf_ignore_discarded_relocs

#define bfd_elf32_bfd_relax_section     qdsp6_elf_relax_section
#define bfd_elf32_bfd_get_relocated_section_contents \
                                        qdsp6_elf_get_relocated_section_contents
#define bfd_elf32_bfd_link_hash_table_create \
                                        qdsp6_elf_link_hash_table_create

#ifdef IPA_LINK
#define bfd_elf32_bfd_link_hash_table_create \
                                        _bfd_elf_link_hash_table_create
#define bfd_elf32_bfd_link_add_symbols  bfd_elf32_bfd_link_add_symbols
#endif
#define elf_backend_common_definition   qdsp6_elf_common_definition

/* This is a bit of a hack
   It installs our wrapper for _bfd_elf_set_arch_mach
 */
#undef BFD_JUMP_TABLE_WRITE
#define BFD_JUMP_TABLE_WRITE(NAME) \
  qdsp6_elf_set_arch_mach, \
  bfd_elf32_set_section_contents

#include "elf32-target.h"
