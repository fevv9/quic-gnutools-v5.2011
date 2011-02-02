/* Hexagon-specific support for 32-bit ELF.
   PIC and DSO support based primarily on the CRIS, i386, PPC, x86-64 ports.

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
#include "elf/hexagon.h"
#include "libiberty.h"
#include "opcode/hexagon.h"
#include <assert.h>

#define SDA_BASE "_SDA_BASE_"
/* Early alias for _SDA_BASE_. */
#define DEFAULT_SDA_BASE "__default_sda_base__"

#define get_section_size_now(abfd,sec) \
    bfd_section_size (abfd,sec)

#define HEXAGON_TRAMPOLINE_PREFIX     ".PAD"
#define HEXAGON_TRAMPOLINE_PREFIX_LEN (sizeof (HEXAGON_TRAMPOLINE_PREFIX))
#define HEXAGON_TRAMPOLINE_NEEDED(D, B) \
  ((bfd_signed_vma) llabs (D) \
   > (~(~(bfd_signed_vma) 0 << ((B) - 1)) & -(MAX_PACKET_INSNS * HEXAGON_INSN_LEN)))

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER "/lib/ld.so"

/* The size in insns of an entry in the PLT, per the ABI. */
#define PLT_ENTRY_LENGTH (6)
/* The size in bytes of an entry in the PLT, per the ABI. */
#define PLT_ENTRY_SIZE (PLT_ENTRY_LENGTH * sizeof (hexagon_insn))
/* The size of the PLT first few entries reserved for the dynamic linker,
   per the ABI. */
#define PLT_RESERVED_ENTRIES (4)
#define PLT_INITIAL_ENTRY_LENGTH (PLT_ENTRY_LENGTH * PLT_RESERVED_ENTRIES)
#define PLT_INITIAL_ENTRY_SIZE (PLT_ENTRY_SIZE * PLT_RESERVED_ENTRIES)

/* The size in bytes of an entry in the GOT, per the ABI. */
#define GOT_ENTRY_SIZE (4)
/* The size of the of the first few GOT entries reserved for the dynamic linker,
   per the ABI. */
#define GOT_RESERVED_ENTRIES (4)
#define GOT_INITIAL_ENTRY_SIZE (GOT_ENTRY_SIZE * GOT_RESERVED_ENTRIES)

/* If ELIMINATE_COPY_RELOCS is non-zero, the linker will try to avoid
   copying dynamic variables from a shared lib into an app's dynbss
   section, and instead use a dynamic relocation to point into the
   shared lib.  */
#define ELIMINATE_COPY_RELOCS 1

#define hexagon_hash_entry(E) ((hexagon_link_hash_entry *) (E))
#define hexagon_hash_table(I) ((hexagon_link_hash_table *) ((I)->hash))

static reloc_howto_type *hexagon_elf_reloc_type_lookup
  PARAMS ((bfd *abfd, bfd_reloc_code_real_type code));
static void hexagon_info_to_howto_rel
  PARAMS ((bfd *, arelent *, Elf_Internal_Rela *));
static bfd_boolean hexagon_elf_object_p
  PARAMS ((bfd *));
static void hexagon_elf_final_write_processing
  PARAMS ((bfd *, bfd_boolean));
static bfd_reloc_status_type hexagon_elf_reloc
  PARAMS ((bfd *, arelent *, asymbol *, PTR, asection *, bfd *, char **));
static bfd_reloc_status_type hexagon_elf_final_sda_base
  PARAMS ((bfd *, char **, bfd_vma *));
static bfd_boolean hexagon_elf_link_output_symbol_hook
  PARAMS ((struct bfd_link_info *info ATTRIBUTE_UNUSED,
           const char *name ATTRIBUTE_UNUSED,
           Elf_Internal_Sym *sym,
           asection *input_sec,
           struct elf_link_hash_entry *h ATTRIBUTE_UNUSED));
static bfd_boolean hexagon_elf_section_from_bfd_section
  PARAMS ((bfd *, asection *, int *));
static bfd_boolean hexagon_elf_section_processing
  PARAMS ((bfd *, Elf_Internal_Shdr *));
static void hexagon_elf_symbol_processing
  PARAMS ((bfd *, asymbol *));
static bfd_boolean hexagon_elf_common_definition
  PARAMS ((Elf_Internal_Sym *));
static bfd_boolean hexagon_elf_add_symbol_hook
  PARAMS ((bfd *, struct bfd_link_info *, Elf_Internal_Sym *,
           const char **, flagword *, asection **, bfd_vma *));
static bfd_boolean hexagon_elf_section_from_shdr
  PARAMS ((bfd *, Elf_Internal_Shdr *, const char *, int));
static bfd_boolean hexagon_elf_fake_sections
  PARAMS ((bfd *, Elf_Internal_Shdr *, asection *));
static bfd_boolean hexagon_elf_relocate_section
  PARAMS ((bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
           Elf_Internal_Rela *, Elf_Internal_Sym *, asection **));
static bfd_boolean hexagon_elf_gc_sweep_hook
  PARAMS ((bfd *, struct bfd_link_info *,
           asection *, const Elf_Internal_Rela *));
static bfd_boolean hexagon_elf_check_relocs
  PARAMS ((bfd *, struct bfd_link_info *,
           asection *, const Elf_Internal_Rela *));
static bfd_boolean hexagon_elf_relax_section
  PARAMS ((bfd *, asection *, struct bfd_link_info *, bfd_boolean *));
static void hexagon_elf_copy_indirect_symbol
  PARAMS ((struct bfd_link_info *, struct elf_link_hash_entry *,
           struct elf_link_hash_entry *));
static int hexagon_elf_additional_program_headers
  PARAMS ((bfd *, struct bfd_link_info *));
static bfd_boolean hexagon_elf_ignore_discarded_relocs
  PARAMS ((asection *));
/* PIC & DSO */
static bfd_boolean hexagon_elf_adjust_dynamic_symbol
  PARAMS ((struct bfd_link_info *, struct elf_link_hash_entry *));
static bfd_boolean hexagon_elf_finish_dynamic_symbol
  PARAMS ((bfd *, struct bfd_link_info *,
           struct elf_link_hash_entry *, Elf_Internal_Sym *));
static bfd_boolean hexagon_elf_create_dynamic_sections
  PARAMS ((bfd *, struct bfd_link_info *));
static bfd_boolean hexagon_elf_size_dynamic_sections
  PARAMS ((bfd *, struct bfd_link_info *));
static bfd_boolean hexagon_elf_finish_dynamic_sections
  PARAMS ((bfd *, struct bfd_link_info *));
static enum elf_reloc_type_class hexagon_elf_reloc_type_class
  PARAMS ((const Elf_Internal_Rela *));
static bfd_vma hexagon_elf_plt_sym_val
  PARAMS ((bfd_vma, const asection *, const arelent *));
static bfd_boolean hexagon_elf_hash_symbol
  PARAMS ((struct elf_link_hash_entry *));
/* TLS */
static bfd_vma hexagon_elf_dtpoff
  PARAMS ((struct bfd_link_info *, bfd_vma));
static bfd_vma hexagon_elf_tpoff
  PARAMS ((struct bfd_link_info *, bfd_vma));

typedef struct elf_dyn_relocs hexagon_dyn_reloc;

typedef struct _hexagon_link_hash_entry
  {
    struct elf_link_hash_entry elf;
    /* Hexagon data. */
    hexagon_dyn_reloc *dyn_relocs;
    /* Break up of GOT references, including TLS-specific ones. */
    struct
      {
	bfd_signed_vma refcount;
      } ok_got, gd_got, ie_got;
  } hexagon_link_hash_entry;

#define LGOT_OK(S, N) ((N) + 1 * (S)->sh_info)
#define LGOT_GD(S, N) ((N) + 2 * (S)->sh_info)
#define LGOT_IE(S, N) ((N) + 3 * (S)->sh_info)
#define LGOT_SZ(S)    (      4 * (S)->sh_info)

typedef struct _hexagon_link_hash_table
  {
    struct elf_link_hash_table elf;
    /* Hexagon data. */
    /* Shortcuts to get to dynamic linker sections.  */
    struct
      {
        asection *s, *r;
      } bss, sbss;
    /* Small local symbol to section mapping cache.  */
    struct sym_cache sym_cache;
  } hexagon_link_hash_table;

typedef struct
  {
    unsigned r_type;
    bfd_vma offset;
    size_t length;
    hexagon_insn insns [8]; /* Actually <length> insns. */
  } hexagon_trampoline;

static const hexagon_trampoline hexagon_trampolines [] =
  {
    {
      R_HEXAGON_HL16, 2 * sizeof (hexagon_insn), 7,
      {
        /* This trampoline requires 1 special relocation. */
        0xbffd7f1d, /*  { sp = add (sp, #-8)      */
        0xa79dfcfe, /*    memw (sp + #-8) = r28 } */
        0x723cc000, /*  r28.h = #HI (foo)         */
        0x713cc000, /*  r28.l = #LO (foo)         */
        0xb01d411d, /*  { sp = add (sp, #8)       */
        0x529c4000, /*    jumpr r28               */
        0x919dc01c, /*    r28 = memw (sp) }       */
      }
    },
  };

/* Reserved entry that is placed at the start of the PLT, used for lazy
   binding. */
static const hexagon_insn hexagon_plt_initial_entry [PLT_INITIAL_ENTRY_LENGTH] =
  {
    0x6a09400c, /*  { r12 = pc                # address of PLT          */
    0x723cc000, /*    r28.h = #hi (PLT@GOTOFF) }                        */
    0x713cc000, /*  r28.l = #lo (PLT@GOTOFF)  # offset of PLT from GOT  */
    0xf33ccc1c, /*  r28 = sub (r12, r28)      # address of GOT          */
    0xf33c4e0e, /*  { r14 = sub (r14, r28)    # offset of @GOT from GOT */
    0x919cc04f, /*    r15 = memw (r28 + #8) } # object ID at GOT [2]    */
    0xbfee7e0d, /*  { r13 = add (r14, #-16)                             */
    0x919cc03c, /*    r28 = memw (r28 + #4) } # dynamic link at GOT [1] */
    0x8c0d420d, /*  { r13 = asr (r13, #2)     # index of @PLT           */
    0x529cc000, /*    jumpr r28 }             # call dynamic link       */
  };

/* Default PLT entry */
static const hexagon_insn hexagon_plt_entry [PLT_ENTRY_LENGTH] =
  {
    0x6a09400f, /* { r15 = pc                  # address of @PLT          */
    0x723cc000, /*   r28.h = #hi (@GOT - @PLT) }                          */
    0x713cc000, /* r28.l = #lo (@GOT - @PLT)   # offset of @GOT from @PLT */
    0xf30fdc0e, /* r14 = add (r15, r28)        # address of @GOT          */
    0x918ec01c, /* r28 = memw (r14)            # contents of @GOT         */
    0x529cc000, /* jumpr r28                   # call it                  */
  };

static reloc_howto_type hexagon_elf_howto_table [] =
{
  /* This reloc does nothing.  */
  EMPTY_HOWTO (R_HEXAGON_NONE),

  /* A relative 22 bit branch. */
  HOWTO (R_HEXAGON_B22_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 24,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B22_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x01ff3ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 15 bit branch. */
  HOWTO (R_HEXAGON_B15_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 17,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B15_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00df20fe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* v2: A relative 7 bit branch. */
  HOWTO (R_HEXAGON_B7_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 9,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B7_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00001f18,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* Low 16-bits of a 32-bit number. */
  HOWTO (R_HEXAGON_LO16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_LO16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High 16-bits of a 32-bit number. */
  HOWTO (R_HEXAGON_HI16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_HI16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A 32-bit number. */
  HOWTO (R_HEXAGON_32,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_HEXAGON_32",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A 16-bit number. */
  HOWTO (R_HEXAGON_16,		/* type  */
	 0,			/* rightshift  */
	 1,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_HEXAGON_16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An 8 bit number. */
  HOWTO (R_HEXAGON_8,		/* type  */
	 0,			/* rightshift  */
	 0,			/* size (0 = byte, 1 = short, 2 = long)  */
	 8,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_HEXAGON_8",		/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xff,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for bytes. */
  HOWTO (R_HEXAGON_GPREL16_0,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GPREL16_0",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for halves. */
  HOWTO (R_HEXAGON_GPREL16_1,	/* type  */
	 1,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 17,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GPREL16_1",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for words. */
  HOWTO (R_HEXAGON_GPREL16_2,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 18,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 -1L,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GPREL16_2",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for doublewords. */
  HOWTO (R_HEXAGON_GPREL16_3,	/* type  */
	 3,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 19,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GPREL16_3",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High and low 16-bits of a 32-bit number applied to 2 insns back-to-back. */
  HOWTO (R_HEXAGON_HL16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_HL16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 13 bit branch. */
  HOWTO (R_HEXAGON_B13_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 15,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B13_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00202ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 9 bit branch. */
  HOWTO (R_HEXAGON_B9_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 11,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B9_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 -1L,			/* src_mask  */
	 0x00c000ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 32-bit extended branch. */
  HOWTO (R_HEXAGON_B32_PCREL_X,	/* type  */
	 6,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B32_PCREL_X", /* name  */
	 FALSE,			/* partial_inplace  */
	 ~0x3f,			/* src_mask  */
	 0x0fff3fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended 32-bit number. */
  HOWTO (R_HEXAGON_32_6_X,	/* type  */
	 6,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_32_6_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 ~0x3f,			/* src_mask  */
	 0x0fff3fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 22 bit branch. */
  HOWTO (R_HEXAGON_B22_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B22_PCREL_X", /* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x01ff3ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 15 bit branch. */
  HOWTO (R_HEXAGON_B15_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B15_PCREL_X", /* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x00df20fe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 13 bit branch. */
  HOWTO (R_HEXAGON_B13_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B13_PCREL_X", /* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x00202ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 9 bit branch. */
  HOWTO (R_HEXAGON_B9_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B9_PCREL_X", /* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x00c000ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended relative 7 bit branch. */
  HOWTO (R_HEXAGON_B7_PCREL_X,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_B7_PCREL_X", /* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0x00001f18,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 16-bit number. */
  HOWTO (R_HEXAGON_16_X,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_16_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 12 bit number. */
  HOWTO (R_HEXAGON_12_X,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_12_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended 11 bit number for bytes. */
  HOWTO (R_HEXAGON_11_X,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_11_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 10 bit number. */
  HOWTO (R_HEXAGON_10_X,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_10_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 9 bit number. */
  HOWTO (R_HEXAGON_9_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_9_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 8 bit number. */
  HOWTO (R_HEXAGON_8_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_8_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 7 bit number. */
  HOWTO (R_HEXAGON_7_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_7_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An extended signed 6 bit number. */
  HOWTO (R_HEXAGON_6_X,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 6,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_6_X",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0x3f,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A 32-bit PC-relative number. */
  HOWTO (R_HEXAGON_32_PCREL,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_HEXAGON_32_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 TRUE),			/* pcrel_offset  */

  /* Copy initial value from copy in DSO. */
  HOWTO (R_HEXAGON_COPY,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_HEXAGON_COPY",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* GOT entry. */
  HOWTO (R_HEXAGON_GLOB_DAT,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_HEXAGON_GLOB_DAT",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* GOT entry for a PLT entry. */
  HOWTO (R_HEXAGON_JMP_SLOT,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_HEXAGON_JMP_SLOT",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* PLT entry. */
  HOWTO (R_HEXAGON_RELATIVE,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_HEXAGON_RELATIVE",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0xffffffff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 22 bit branch to a PLT entry. */
  HOWTO (R_HEXAGON_PLT_B22_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 22,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_PLT_B22_PCREL", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x01ff3ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* Low 16-bits of a 32-bit offset from GOT. */
  HOWTO (R_HEXAGON_GOTREL_LO16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GOTREL_LO16", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High 16-bits of a 32-bit offset from GOT. */
  HOWTO (R_HEXAGON_GOTREL_HI16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GOTREL_HI16", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* 32-bit offset from GOT. */
  HOWTO (R_HEXAGON_GOTREL_32,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         32,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_bitfield, /* complain_on_overflow  */
         bfd_elf_generic_reloc, /* special_function  */
         "R_HEXAGON_GOTREL_32",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0xffffffff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* Low 16-bits of a 32-bit offset of a GOT entry. */
  HOWTO (R_HEXAGON_GOT_LO16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GOT_LO16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High 16-bits of a 32-bit offset of a GOT entry. */
  HOWTO (R_HEXAGON_GOT_HI16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GOT_HI16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* 32-bit offset of a GOT entry. */
  HOWTO (R_HEXAGON_GOT_32,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         32,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_bitfield, /* complain_on_overflow  */
         bfd_elf_generic_reloc, /* special_function  */
         "R_HEXAGON_GOT_32",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0xffffffff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* 16-bit offset of a GOT entry. */
  HOWTO (R_HEXAGON_GOT_16,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         16,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_signed, /* complain_on_overflow  */
         hexagon_elf_reloc,	/* special_function  */
         "R_HEXAGON_GOT_16",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0x00c03fff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* 32-bit TLS module ID. */
  HOWTO (R_HEXAGON_DTPMOD_32,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         32,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_bitfield, /* complain_on_overflow  */
         bfd_elf_generic_reloc, /* special_function  */
         "R_HEXAGON_DTPMOD_32",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0xffffffff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* 32-bit TLS symbol offset. */
  HOWTO (R_HEXAGON_DTPREL_32,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         32,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_bitfield, /* complain_on_overflow  */
         bfd_elf_generic_reloc, /* special_function  */
         "R_HEXAGON_DTPREL_32",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0xffffffff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* Low 16-bits of a 32-bit offset of GOT entries for a TLS GD descriptor. */
  HOWTO (R_HEXAGON_GD_GOT_LO16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GD_GOT_LO16", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High 16-bits of a 32-bit offset of GOT entries for a TLS GD descriptor. */
  HOWTO (R_HEXAGON_GD_GOT_HI16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_GD_GOT_HI16", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* 32-bit offset of GOT entries for a TLS GD descriptor. */
  HOWTO (R_HEXAGON_GD_GOT_32,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         32,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_bitfield, /* complain_on_overflow  */
         bfd_elf_generic_reloc, /* special_function  */
         "R_HEXAGON_GD_GOT_32",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0xffffffff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* 16-bit offset of GOT entries for a TLS GD descriptor. */
  HOWTO (R_HEXAGON_GD_GOT_16,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         16,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_signed, /* complain_on_overflow  */
         hexagon_elf_reloc,	/* special_function  */
         "R_HEXAGON_GD_GOT_16",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0x00c03fff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* Low 16-bits of a 32-bit offset of a GOT entry for a TLS IE symbol. */
  HOWTO (R_HEXAGON_IE_GOT_LO16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_IE_GOT_LO16", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High 16-bits of a 32-bit offset of GOT entry for a TLS IE symbol. */
  HOWTO (R_HEXAGON_IE_GOT_HI16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_IE_GOT_HI16", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* 32-bit offset of GOT entry for a TLS IE symbol. */
  HOWTO (R_HEXAGON_IE_GOT_32,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         32,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_bitfield, /* complain_on_overflow  */
         bfd_elf_generic_reloc, /* special_function  */
         "R_HEXAGON_IE_GOT_32",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0xffffffff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* 16-bit offset of GOT entry for a TLS IE symbol. */
  HOWTO (R_HEXAGON_IE_GOT_16,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         16,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_signed, /* complain_on_overflow  */
         hexagon_elf_reloc,	/* special_function  */
         "R_HEXAGON_IE_GOT_16",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0x00c03fff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* Low 16-bits of a 32-bit offset from the TP. */
  HOWTO (R_HEXAGON_TPREL_LO16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_TPREL_LO16", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High 16-bits of a 32-bit offset from the TP. */
  HOWTO (R_HEXAGON_TPREL_HI16,	/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 hexagon_elf_reloc,	/* special_function  */
	 "R_HEXAGON_TPREL_HI16", /* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* 32-bit offset from the TP. */
  HOWTO (R_HEXAGON_TPREL_32,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         32,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_bitfield, /* complain_on_overflow  */
         bfd_elf_generic_reloc, /* special_function  */
         "R_HEXAGON_TPREL_32",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0xffffffff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

  /* 16-bit offset from the TP. */
  HOWTO (R_HEXAGON_TPREL_16,	/* type  */
         0,                     /* rightshift  */
         2,                     /* size (0 = byte, 1 = short, 2 = long)  */
         16,                    /* bitsize  */
         FALSE,                 /* pc_relative  */
         0,                     /* bitpos  */
         complain_overflow_signed, /* complain_on_overflow  */
         hexagon_elf_reloc,	/* special_function  */
         "R_HEXAGON_TPREL_16",	/* name  */
         FALSE,                 /* partial_inplace  */
         0,                     /* src_mask  */
         0x00c03fff,            /* dst_mask  */
         FALSE),                /* pcrel_offset  */

};

/* Hexagon ELF uses two common sections.  One is the usual one, and the
   other is for small objects.  All the small objects are kept
   together, and then referenced via the GP-register, which yields
   faster assembler code.  This is what we use for the small common
   section.  This approach is copied from ecoff.c.  */
static asection    hexagon_scom_section [SHN_HEXAGON_SCOMMON_8 - SHN_HEXAGON_SCOMMON + 1];
static asymbol     hexagon_scom_symbol  [SHN_HEXAGON_SCOMMON_8 - SHN_HEXAGON_SCOMMON + 1];
static const char *hexagon_scom_name    [SHN_HEXAGON_SCOMMON_8 - SHN_HEXAGON_SCOMMON + 1] =
  {".scommon", ".scommon.1", ".scommon.2", ".scommon.4", ".scommon.8"};

/* Map BFD reloc types to Hexagon ELF reloc types.  */

struct hexagon_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int             elf_reloc_val;
  int                      operand_flag;
};

static const struct hexagon_reloc_map hexagon_reloc_map [] =
{
  { BFD_RELOC_NONE,                  R_HEXAGON_NONE,          0 },
  { BFD_RELOC_HEXAGON_B32_PCREL_X,   R_HEXAGON_B32_PCREL_X,   0 }, /* K-ext */
  { BFD_RELOC_HEXAGON_B22_PCREL,     R_HEXAGON_B22_PCREL,     0 },
  { BFD_RELOC_HEXAGON_B22_PCREL_X,   R_HEXAGON_B22_PCREL_X,   HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_B15_PCREL,     R_HEXAGON_B15_PCREL,     0 },
  { BFD_RELOC_HEXAGON_B15_PCREL_X,   R_HEXAGON_B15_PCREL_X,   HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_B13_PCREL,     R_HEXAGON_B13_PCREL,     0 },
  { BFD_RELOC_HEXAGON_B13_PCREL_X,   R_HEXAGON_B13_PCREL_X,   HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_B9_PCREL,      R_HEXAGON_B9_PCREL,      0 },
  { BFD_RELOC_HEXAGON_B9_PCREL_X,    R_HEXAGON_B9_PCREL_X,    HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_B7_PCREL,      R_HEXAGON_B7_PCREL,      0 },
  { BFD_RELOC_HEXAGON_B7_PCREL_X,    R_HEXAGON_B7_PCREL_X,    HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_LO16,          R_HEXAGON_LO16,          0 },
  { BFD_RELOC_HEXAGON_HI16,          R_HEXAGON_HI16,          0 },
  { BFD_RELOC_HEXAGON_HL16,          R_HEXAGON_HL16,          0 },
  { BFD_RELOC_HEXAGON_32_6_X,        R_HEXAGON_32_6_X,        0 }, /* K-ext */
  { BFD_RELOC_32_PCREL,              R_HEXAGON_32_PCREL,      0 },
  { BFD_RELOC_32,                    R_HEXAGON_32,            0 },
  { BFD_RELOC_16,                    R_HEXAGON_16,            0 },
  { BFD_RELOC_8,                     R_HEXAGON_8,             0 },
  { BFD_RELOC_HEXAGON_16_X,          R_HEXAGON_16_X,          HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_12_X,          R_HEXAGON_12_X,          HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_11_X,          R_HEXAGON_11_X,          HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_10_X,          R_HEXAGON_10_X,          HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_9_X,           R_HEXAGON_9_X,           HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_8_X,           R_HEXAGON_8_X,           HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_7_X,           R_HEXAGON_7_X,           HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_6_X,           R_HEXAGON_6_X,           HEXAGON_OPERAND_IS_KXED },
  { BFD_RELOC_HEXAGON_GPREL16_0,     R_HEXAGON_GPREL16_0,     0 },
  { BFD_RELOC_HEXAGON_GPREL16_1,     R_HEXAGON_GPREL16_1,     0 },
  { BFD_RELOC_HEXAGON_GPREL16_2,     R_HEXAGON_GPREL16_2,     0 },
  { BFD_RELOC_HEXAGON_GPREL16_3,     R_HEXAGON_GPREL16_3,     0 },
  /* DSO */
  { BFD_RELOC_HEXAGON_COPY,          R_HEXAGON_COPY,          0 },
  { BFD_RELOC_HEXAGON_GLOB_DAT,      R_HEXAGON_GLOB_DAT,      0 },
  { BFD_RELOC_HEXAGON_JMP_SLOT,      R_HEXAGON_JMP_SLOT,      0 },
  { BFD_RELOC_HEXAGON_RELATIVE,      R_HEXAGON_RELATIVE,      0 },
  /* PLT */
  { BFD_RELOC_HEXAGON_PLT_B22_PCREL, R_HEXAGON_PLT_B22_PCREL, 0 },
  /* GOT */
  { BFD_RELOC_HEXAGON_GOTREL_LO16,   R_HEXAGON_GOTREL_LO16,   0 },
  { BFD_RELOC_HEXAGON_GOTREL_HI16,   R_HEXAGON_GOTREL_HI16,   0 },
  { BFD_RELOC_32_GOTOFF,             R_HEXAGON_GOTREL_32,     0 },
  { BFD_RELOC_HEXAGON_GOT_LO16,      R_HEXAGON_GOT_LO16,      0 },
  { BFD_RELOC_HEXAGON_GOT_HI16,      R_HEXAGON_GOT_HI16,      0 },
  { BFD_RELOC_HEXAGON_GOT_32,        R_HEXAGON_GOT_32,        0 },
  { BFD_RELOC_HEXAGON_GOT_16,        R_HEXAGON_GOT_16,        0 },
  /* TLS */
  { BFD_RELOC_HEXAGON_DTPMOD_32,     R_HEXAGON_DTPMOD_32,     0 },
  { BFD_RELOC_HEXAGON_DTPREL_32,     R_HEXAGON_DTPREL_32,     0 },
  { BFD_RELOC_HEXAGON_GD_GOT_LO16,   R_HEXAGON_GD_GOT_LO16,   0 },
  { BFD_RELOC_HEXAGON_GD_GOT_HI16,   R_HEXAGON_GD_GOT_HI16,   0 },
  { BFD_RELOC_HEXAGON_GD_GOT_32,     R_HEXAGON_GD_GOT_32,     0 },
  { BFD_RELOC_HEXAGON_GD_GOT_16,     R_HEXAGON_GD_GOT_16,     0 },
  { BFD_RELOC_HEXAGON_IE_GOT_LO16,   R_HEXAGON_IE_GOT_LO16,   0 },
  { BFD_RELOC_HEXAGON_IE_GOT_HI16,   R_HEXAGON_IE_GOT_HI16,   0 },
  { BFD_RELOC_HEXAGON_IE_GOT_32,     R_HEXAGON_IE_GOT_32,     0 },
  { BFD_RELOC_HEXAGON_IE_GOT_16,     R_HEXAGON_IE_GOT_16,     0 },
  { BFD_RELOC_HEXAGON_TPREL_LO16,    R_HEXAGON_TPREL_LO16,    0 },
  { BFD_RELOC_HEXAGON_TPREL_HI16,    R_HEXAGON_TPREL_HI16,    0 },
  { BFD_RELOC_HEXAGON_TPREL_32,      R_HEXAGON_TPREL_32,      0 },
  { BFD_RELOC_HEXAGON_TPREL_16,      R_HEXAGON_TPREL_16,      0 },
};

static reloc_howto_type *
hexagon_elf_reloc_type_lookup
(bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  size_t i;

  for (i = ARRAY_SIZE (hexagon_reloc_map); i--;)
    if (hexagon_reloc_map [i].bfd_reloc_val == code)
      {
        reloc_howto_type *howto =
          hexagon_elf_howto_table + hexagon_reloc_map [i].elf_reloc_val;
        BFD_ASSERT (howto->type == hexagon_reloc_map [i].elf_reloc_val);

        return (howto);
      }

  return NULL;
}

static bfd_reloc_code_real_type
hexagon_elf_reloc_val_lookup
(unsigned char elf_reloc_val, int *flag)
{
  unsigned int i;

  for (i = ARRAY_SIZE (hexagon_reloc_map); i--;)
    if (hexagon_reloc_map [i].elf_reloc_val == elf_reloc_val)
      {
        if (flag)
          *flag = hexagon_reloc_map [i].operand_flag;

        return (hexagon_reloc_map [i].bfd_reloc_val);
      }

  if (flag)
    *flag = 0;

  return (BFD_RELOC_NONE);
}

static reloc_howto_type *
hexagon_elf_reloc_name_lookup
(bfd *abfd ATTRIBUTE_UNUSED, const char *r_name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE (hexagon_elf_howto_table); i++)
    if (hexagon_elf_howto_table [i].name
        && !strcasecmp (hexagon_elf_howto_table [i].name, r_name))
    return (hexagon_elf_howto_table + i);

  return NULL;
}

/* Set the howto pointer for a Hexagon ELF reloc.  */

static void
hexagon_info_to_howto_rel
(bfd *abfd ATTRIBUTE_UNUSED, arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned int r_type;

  r_type = ELF32_R_TYPE (dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_HEXAGON_max);

  cache_ptr->howto = hexagon_elf_howto_table + r_type;
}

/**
Return the DTPREL value.
*/
static bfd_vma
hexagon_elf_dtpoff (struct bfd_link_info *info, bfd_vma address)
{
  if (!elf_hash_table (info)->tls_sec)
    return 0;
  else
   return (address
	   - elf_hash_table (info)->tls_sec->vma);
}


/**
Return the TPREL value.
*/
static bfd_vma
hexagon_elf_tpoff (struct bfd_link_info *info, bfd_vma address)
{
  if (!elf_hash_table (info)->tls_sec)
    return 0;
  else
    return (address
	    - elf_hash_table (info)->tls_sec->vma
	    - elf_hash_table (info)->tls_size);
}

/* Set the right machine number for an Hexagon ELF file.  */
static bfd_boolean
hexagon_elf_object_p
(bfd *abfd)
{
  unsigned int mach = bfd_mach_hexagon_v2;

  /* Set our target machine number. */
  if (elf_elfheader (abfd)->e_machine == EM_HEXAGON)
    {
      unsigned long arch = elf_elfheader (abfd)->e_flags & EF_HEXAGON_MACH;

      switch (EF_HEXAGON_MACH_VER (arch))
	{
          case EF_HEXAGON_MACH_V2:
            mach = bfd_mach_hexagon_v2;
            break;

          case EF_HEXAGON_MACH_V3:
            mach = bfd_mach_hexagon_v3;
            break;

          case EF_HEXAGON_MACH_V4:
            mach = bfd_mach_hexagon_v4;
            break;

          default:
            if (elf_elfheader (abfd)->e_type != ET_HEXAGON_IR)
              {
                /* Unless the object is a WHIRL, barf. */
                fprintf (stderr, "Unsupported machine architecure %ld\n", arch);
                abort ();
              }
	}
    }

  return (bfd_default_set_arch_mach (abfd, bfd_arch_hexagon, mach));
}

/* The final processing done just before writing out an Hexagon ELF object file.
   This gets the Hexagon architecture right based on the machine number.  */

static void
hexagon_elf_final_write_processing
(bfd *abfd, bfd_boolean linker ATTRIBUTE_UNUSED)
{
  unsigned long val;

  switch (bfd_get_mach (abfd))
    {
      case bfd_mach_hexagon_v2:
        val = EF_HEXAGON_MACH_V2;
        break;

      case bfd_mach_hexagon_v3:
        val = EF_HEXAGON_MACH_V3;
        break;

      case bfd_mach_hexagon_v4:
        val = EF_HEXAGON_MACH_V4;
        break;

      default:
        fprintf (stderr, "Unsupported machine architecure.\n");
        abort ();
    }

  elf_elfheader (abfd)->e_flags &= ~EF_HEXAGON_MACH;
  elf_elfheader (abfd)->e_flags |= val;
}

static struct bfd_link_hash_entry *hexagon_sda_base;

static bfd_byte *
hexagon_elf_get_relocated_section_contents
(bfd *obfd, struct bfd_link_info *link_info,
 struct bfd_link_order *link_order, bfd_byte *data, bfd_boolean relocatable,
 asymbol **symbols)
{
  struct bfd_link_hash_entry *hash;
  bfd_vma vma;

  /* Get the hash table entry for _SDA_BASE_. */
  if ((hash = bfd_link_hash_lookup
                (link_info->hash, SDA_BASE, FALSE, FALSE, FALSE)))
    hexagon_sda_base = hash;
  /* In case of relying on _SDA_BASE_ being provided by the linker script,
     the look up above will fail, for it's too soon to provide its value.
     Instead, look up the alternate symbol that will be provided later on. */
  else if ((hash = bfd_link_hash_lookup
                     (link_info->hash, DEFAULT_SDA_BASE, FALSE, FALSE, FALSE)))
    hexagon_sda_base = hash;
  /* At this point, if no _SDA_BASE_, either the alternate or the proper one,
     is resolved, then GP-relocaiton overflow errors are likely. */

  /* Try to validate the effective value of _SDA_BASE_. */
  if (hexagon_sda_base)
    {
      if (bfd_reloc_ok != hexagon_elf_final_sda_base (obfd, NULL, &vma))
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
       (obfd, link_info, link_order, data, relocatable, symbols));
}

/* We have to figure out the SDA_BASE value, so that we can adjust the
   symbol value correctly.  We look up the symbol _SDA_BASE_ in the output
   BFD.  If we can't find it, we're stuck.  We cache it in the ELF
   target data.  We don't need to adjust the symbol value for an
   external symbol if we are producing relocatable output.  */

static bfd_reloc_status_type
hexagon_elf_final_sda_base
(bfd *obfd, char **error_message, bfd_vma *psb)
{
  if (obfd != (bfd *) NULL)
    {
      *psb = 0;
      return bfd_reloc_ok;
    }
  else if (hexagon_sda_base != 0 && hexagon_sda_base->type == bfd_link_hash_defined)
    {
      *psb = hexagon_sda_base->u.def.value
	   + hexagon_sda_base->u.def.section->output_section->vma
	   + hexagon_sda_base->u.def.section->output_offset;
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
hexagon_reloc_operand
(reloc_howto_type *howto, hexagon_insn *insn, bfd_vma offset, char **errmsg)
{
  bfd_reloc_code_real_type type;
  const hexagon_opcode *opcode;
  const hexagon_operand *operand;
  int flag;
  long value, xvalue;

  opcode  = hexagon_lookup_insn (*insn);
  type    = hexagon_elf_reloc_val_lookup (howto->type, &flag);
  operand = hexagon_lookup_reloc (type, flag, opcode);

  value = offset;

  if (!opcode || !operand)
    return FALSE;
  else if (!hexagon_encode_operand
	      (operand, insn, opcode, value, &xvalue,
	       flag & HEXAGON_OPERAND_IS_KXED,
	       operand->flags & HEXAGON_OPERAND_PC_RELATIVE, errmsg))
    return FALSE;
  else
    return TRUE;
}

static bfd_reloc_status_type
hexagon_elf_reloc
(bfd *abfd, arelent *reloc_entry, asymbol *symbol, PTR data,
 asection *isection, bfd *obfd, char **error_message)
{
  reloc_howto_type *howto = reloc_entry->howto;
  bfd_size_type octets_per_byte = bfd_octets_per_byte(abfd);
  bfd_size_type octets = reloc_entry->address * octets_per_byte;
  bfd_reloc_status_type status;
  bfd_reloc_status_type flag = bfd_reloc_ok;
  bfd_vma relocation;
  bfd_vma output_base = 0;
  asection *reloc_target_output_section;

  /* bfd_perform_relocation () performed this test before calling
     howto->special_function (), but that gets ignored if we return
     bfd_reloc_ok (which we do, below) so we need to re-do this
     test here. The real bug is probably in bfd_perform_relocation ()
     but it is better not to modify the common code. */
  if (bfd_is_und_section (symbol->section)
      && (symbol->flags & BSF_WEAK) == 0
      && obfd == (bfd *) NULL)
    return bfd_reloc_undefined;

  /* If linking, back up the final symbol address by the address of the
     reloc.  This cannot be accomplished by setting the pcrel_offset
     field to TRUE, as bfd_install_relocation will detect this and refuse
     to install the offset in the first place, but bfd_perform_relocation
     will still insist on removing it.  */
  if (obfd == (bfd *) NULL && howto->pc_relative)
    reloc_entry->addend -= reloc_entry->address;

  /* Fall through to the default elf reloc handler.  */
  status = bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data,
                                  isection, obfd, error_message);
  if (status != bfd_reloc_continue)
    return status;

  /* Handle PC relative relocatable output. */
  if (obfd != (bfd *) NULL
      && reloc_entry->howto->pc_relative
      && (!reloc_entry->howto->partial_inplace
          || !reloc_entry->addend))
    {
      reloc_entry->address += isection->output_offset;
      return bfd_reloc_ok;
    }

  // NOTE:  This code was pasted/modified from bfd_perform_relocation
  //        We  can't use that code (which would happen if we returned
  //        bfd_reloc_continue) because it assumes the operand goes
  //        into a contiguous range of bits.

  /* Is the address of the relocation really within the section?  */
  if (reloc_entry->address > (isection->size
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
  if ((obfd && !howto->partial_inplace)
      || reloc_target_output_section == NULL)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;

  relocation += output_base + symbol->section->output_offset;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
     symbol we are relocating against, plus any addend.  */


  if ((howto->type == R_HEXAGON_GPREL16_0) ||
      (howto->type == R_HEXAGON_GPREL16_1) ||
      (howto->type == R_HEXAGON_GPREL16_2) ||
      (howto->type == R_HEXAGON_GPREL16_3))
    {
      bfd_vma sda_base;

      status = hexagon_elf_final_sda_base (obfd, error_message, &sda_base);
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
        isection->output_section->vma + isection->output_offset;

      if (howto->pcrel_offset)
        relocation -= reloc_entry->address;
    }

  if (obfd != (bfd *) NULL)
    {
      if (!howto->partial_inplace)
        {
          /* This is a partial relocation, and we want to apply the relocation
             to the reloc entry rather than the raw data. Modify the reloc
             inplace to reflect what we now know.  */
          reloc_entry->addend = relocation;
          reloc_entry->address += isection->output_offset;
          return flag;
        }
      else
        {
          /* This is a partial relocation, but inplace, so modify the
             reloc record a bit.

             If we've relocated with a symbol with a section, change
             into a ref to the section belonging to the symbol.  */

          reloc_entry->address += isection->output_offset;
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
         && obfd == (bfd *) NULL)
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

  switch (howto->size)
    {
      case 1:
        {
          hexagon_insn insn = bfd_get_16 (abfd, (bfd_byte *) data + octets);

          if (!hexagon_reloc_operand (howto, &insn, relocation, error_message))
            {
              return bfd_reloc_overflow;
            }

          bfd_put_16 (abfd, (bfd_vma) insn, (unsigned char *) data);
        }
      break;

      case 2:
        {
          hexagon_insn insn = bfd_get_32 (abfd, (bfd_byte *) data + octets);

          if (!hexagon_reloc_operand (howto, &insn, relocation, error_message))
            {
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
   Also calls hexagon_opcode_init_tables */

static bfd_boolean
hexagon_elf_set_arch_mach
(bfd *abfd, enum bfd_architecture arch, unsigned long machine)
{
  if (!_bfd_elf_set_arch_mach (abfd, arch, machine))
    return FALSE;

  hexagon_opcode_init_tables (hexagon_get_opcode_mach (machine, bfd_big_endian (abfd)));

  return TRUE;
}

/* Set up any other section flags and such that may be necessary.  */
static bfd_boolean
hexagon_elf_fake_sections
(bfd *abfd ATTRIBUTE_UNUSED, Elf_Internal_Shdr *shdr, asection *asect)
{
  if ((asect->flags & SEC_SORT_ENTRIES))
    shdr->sh_type = SHT_HEXAGON_ORDERED;

  return TRUE;
}

/* Handle a specific sections when reading an object file.  This
   is called when elfcode.h finds a section with an unknown type.  */
static bfd_boolean
hexagon_elf_section_from_shdr
(bfd *abfd, Elf_Internal_Shdr *hdr, const char *name, int shindex)
{
  asection *newsect;
  flagword flags;

  if (!_bfd_elf_make_section_from_shdr (abfd, hdr, name, shindex))
    return FALSE;

  newsect = hdr->bfd_section;
  flags = bfd_get_section_flags (abfd, newsect);

  if (hdr->sh_type == SHT_HEXAGON_ORDERED)
    flags |= SEC_SORT_ENTRIES;

  bfd_set_section_flags (abfd, newsect, flags);
  return TRUE;
}

/* Hook called by the linker routine which adds symbols from an object
   file.  We use it to put .comm items in .sbss, and not .bss.  */
static bfd_boolean
hexagon_elf_add_symbol_hook
(bfd *abfd, struct bfd_link_info *info ATTRIBUTE_UNUSED,
 Elf_Internal_Sym *sym, const char **namep ATTRIBUTE_UNUSED,
 flagword *flagsp ATTRIBUTE_UNUSED, asection **secp, bfd_vma *valp)
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
               treated as SHN_HEXAGONS_SCOMMON symbols.  */
            if (sym->st_size > elf_gp_size (abfd))
              break;

            /* Choose which section to place them in. */
            if (sym->st_size > 8)
              sym->st_shndx = SHN_HEXAGON_SCOMMON;
            else if (sym->st_size > 4)
              sym->st_shndx = SHN_HEXAGON_SCOMMON_8;
            else if (sym->st_size > 2)
              sym->st_shndx = SHN_HEXAGON_SCOMMON_4;
            else if (sym->st_size > 1)
              sym->st_shndx = SHN_HEXAGON_SCOMMON_2;
            else
              sym->st_shndx = SHN_HEXAGON_SCOMMON_1;

            /* Fall through. */

          case SHN_HEXAGON_SCOMMON:
          case SHN_HEXAGON_SCOMMON_8:
          case SHN_HEXAGON_SCOMMON_4:
          case SHN_HEXAGON_SCOMMON_2:
          case SHN_HEXAGON_SCOMMON_1:
            /* Small common symbols are placed in their originally intended
	       .scommon sections. */
            *secp = bfd_make_section_old_way
                      (abfd, hexagon_scom_name [sym->st_shndx - SHN_HEXAGON_SCOMMON]);
            bfd_set_section_flags
              (abfd, *secp, SEC_ALLOC | SEC_IS_COMMON | SEC_LINKER_CREATED);
            *valp = sym->st_size;
            break;
        }
    }

  return TRUE;
}

/* Handle the special Hexagon section numbers that a symbol may use. */
void
hexagon_elf_symbol_processing
(bfd *abfd, asymbol *asym)
{
  elf_symbol_type *elfsym = (elf_symbol_type *) asym;

  /* Decide if a common symbol should be considered small or not. */
  switch (elfsym->internal_elf_sym.st_shndx)
    {
    case SHN_COMMON:
      /* Common symbols less than the GP size are automatically
	 treated as SHN_HEXAGON_SCOMMON symbols.  */
      if (asym->value > elf_gp_size (abfd))
        break;

      /* Choose which section to place them in. */
      if (elfsym->internal_elf_sym.st_size > 8)
        elfsym->internal_elf_sym.st_shndx = SHN_HEXAGON_SCOMMON;
      if (elfsym->internal_elf_sym.st_size > 4)
        elfsym->internal_elf_sym.st_shndx = SHN_HEXAGON_SCOMMON_8;
      else if (elfsym->internal_elf_sym.st_size > 2)
        elfsym->internal_elf_sym.st_shndx = SHN_HEXAGON_SCOMMON_4;
      else if (elfsym->internal_elf_sym.st_size > 1)
        elfsym->internal_elf_sym.st_shndx = SHN_HEXAGON_SCOMMON_2;
      else
        elfsym->internal_elf_sym.st_shndx = SHN_HEXAGON_SCOMMON_1;

      /* Fall through.  */

    case SHN_HEXAGON_SCOMMON:
    case SHN_HEXAGON_SCOMMON_8:
    case SHN_HEXAGON_SCOMMON_4:
    case SHN_HEXAGON_SCOMMON_2:
    case SHN_HEXAGON_SCOMMON_1:
        {
	  /* Small common symbols are placed in their originally intended
	     .scommon sections. */
          asection *scom_section = hexagon_scom_section
                                   + elfsym->internal_elf_sym.st_shndx
                                   - SHN_HEXAGON_SCOMMON;
          asymbol *scom_symbol = hexagon_scom_symbol
                                 + elfsym->internal_elf_sym.st_shndx
                                 - SHN_HEXAGON_SCOMMON;

          if (!scom_section->name)
            {
              const char *scom_name =
                hexagon_scom_name [elfsym->internal_elf_sym.st_shndx - SHN_HEXAGON_SCOMMON];

              /* Initialize the small common section.  */
              scom_section->name           = scom_name;
              scom_section->flags          = (SEC_IS_COMMON | SEC_SMALL_DATA)
                                             | (elfsym->internal_elf_sym.st_shndx
                                                > SHN_HEXAGON_SCOMMON
                                                ? (SEC_LOAD | SEC_DATA): 0);
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
hexagon_elf_common_definition
(Elf_Internal_Sym *sym)
{
  return (sym->st_shndx == SHN_COMMON
          || sym->st_shndx == SHN_HEXAGON_SCOMMON
          || sym->st_shndx == SHN_HEXAGON_SCOMMON_8
          || sym->st_shndx == SHN_HEXAGON_SCOMMON_4
          || sym->st_shndx == SHN_HEXAGON_SCOMMON_2
          || sym->st_shndx == SHN_HEXAGON_SCOMMON_1);
}


/* Work over a section just before writing it out. FIXME: We recognize
   sections that need the SHF_HEXAGON_GPREL flag by name; there has to be
   a better way.  */

bfd_boolean
hexagon_elf_section_processing
(bfd *abfd ATTRIBUTE_UNUSED, Elf_Internal_Shdr *hdr)
{
  if (hdr->bfd_section)
    {
      const char *name = bfd_get_section_name (abfd, hdr->bfd_section);

      if (CONST_STRNEQ (name, ".sdata"))
	{
	  hdr->sh_flags |= SHF_ALLOC | SHF_WRITE | SHF_HEXAGON_GPREL;
	  hdr->sh_type = SHT_PROGBITS;
	}
      else if (CONST_STRNEQ (name, ".sbss"))
	{
	  hdr->sh_flags |= SHF_ALLOC | SHF_WRITE | SHF_HEXAGON_GPREL;
	  hdr->sh_type = SHT_NOBITS;
	}
    }

  return TRUE;
}

/* Given a BFD section, try to locate the corresponding ELF section index. */
bfd_boolean
hexagon_elf_section_from_bfd_section
(bfd *abfd ATTRIBUTE_UNUSED, asection *sec, int *retval)
{
  const char *name = bfd_get_section_name (abfd, sec);

  if (CONST_STRNEQ (name, hexagon_scom_name [0]))
    {
      if (!strcmp (name, hexagon_scom_name [SHN_HEXAGON_SCOMMON_8 - SHN_HEXAGON_SCOMMON]))
        *retval = SHN_HEXAGON_SCOMMON_8;
      else if (!strcmp (name, hexagon_scom_name [SHN_HEXAGON_SCOMMON_4 - SHN_HEXAGON_SCOMMON]))
        *retval = SHN_HEXAGON_SCOMMON_4;
      else if (!strcmp (name, hexagon_scom_name [SHN_HEXAGON_SCOMMON_2 - SHN_HEXAGON_SCOMMON]))
        *retval = SHN_HEXAGON_SCOMMON_2;
      else if (!strcmp (name, hexagon_scom_name [SHN_HEXAGON_SCOMMON_1 - SHN_HEXAGON_SCOMMON]))
        *retval = SHN_HEXAGON_SCOMMON_1;
      else
        *retval = SHN_HEXAGON_SCOMMON;

      return TRUE;
    }
  else
    return FALSE;
}

/* This hook function is called before the linker writes out a global
   symbol.  We mark symbols as small common if appropriate. */
bfd_boolean
hexagon_elf_link_output_symbol_hook
(struct bfd_link_info *info ATTRIBUTE_UNUSED, const char *name ATTRIBUTE_UNUSED,
 Elf_Internal_Sym *sym, asection *input_sec,
 struct elf_link_hash_entry *h ATTRIBUTE_UNUSED)
{
  /* If we see a common symbol, which implies a relocatable link, then
     if a symbol was small common in an input file, mark it as small
     common in the output file.  */
  if (sym->st_shndx == SHN_COMMON
      && elf_gp_size (info->output_bfd)
      && sym->st_size > elf_gp_size (info->output_bfd)
      && CONST_STRNEQ (input_sec->name, hexagon_scom_name [0]))
    {
      if (!strcmp (name, hexagon_scom_name [SHN_HEXAGON_SCOMMON_8 - SHN_HEXAGON_SCOMMON]))
        sym->st_shndx = SHN_HEXAGON_SCOMMON_8;
      else if (!strcmp (name, hexagon_scom_name [SHN_HEXAGON_SCOMMON_4 - SHN_HEXAGON_SCOMMON]))
        sym->st_shndx = SHN_HEXAGON_SCOMMON_4;
      else if (!strcmp (name, hexagon_scom_name [SHN_HEXAGON_SCOMMON_2 - SHN_HEXAGON_SCOMMON]))
        sym->st_shndx = SHN_HEXAGON_SCOMMON_2;
      else if (!strcmp (name, hexagon_scom_name [SHN_HEXAGON_SCOMMON_1 - SHN_HEXAGON_SCOMMON]))
        sym->st_shndx = SHN_HEXAGON_SCOMMON_1;
      else
        sym->st_shndx = SHN_HEXAGON_SCOMMON;
    }

  return TRUE;
}

static inline hexagon_insn
hexagon_get_insn (bfd * ibfd, reloc_howto_type * howto, bfd_byte *offset)
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
hexagon_put_insn
(bfd *ibfd, reloc_howto_type *howto, bfd_byte *offset, hexagon_insn insn)
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
hexagon_kept_hash_lookup
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
hexagon_elf_relax_section
(bfd *ibfd, asection *isec, struct bfd_link_info *info, bfd_boolean *again)
{
  Elf_Internal_Shdr *symtab_hdr = NULL;
  Elf_Internal_Rela *irelbuf = NULL;
  Elf_Internal_Rela *irel;
  unsigned int ireloc;
  bfd_byte *contents = NULL;
  Elf_Internal_Sym *isymbuf = NULL;
  struct elf_link_hash_entry **sym_hashes;
  size_t r_symndx;
  struct elf_link_hash_entry *h;
  struct bfd_link_hash_entry *t_h;
  struct bfd_link_hash_table t_hash;
  bfd_boolean rc = TRUE;
  bfd_size_type isec_size = 0;
  char *name, *t_name;
  hexagon_insn insn;
  size_t i = 0, j = 0;

  *again = FALSE;

  if (info->relocatable
      || info->shared
      || !(isec->flags & SEC_RELOC)
      || !isec->reloc_count)
    return rc;

  /* If needed, initialize this section's cooked size.  */
  if (isec->size == 0)
    isec->size = isec->rawsize;

  BFD_ASSERT
    (bfd_hash_table_init
       ((struct bfd_hash_table *) &t_hash, _bfd_link_hash_newfunc,
	sizeof (struct bfd_link_hash_table)));


  /* It's quite hard to get rid of the relocation table once it's been read.
     Ideally, any relocations required by the trampoline should be added to it,
     but it seems that everything falls off if the table is changed in any way.
     Since the original relocation is voided, it and only it may be reused by
     the trampoline.
     TODO: instead of keeping the memory around, perhaps manually setting
           "elf_section_data (o)->relocs" if there's any change would use up
           less memory. */
  irelbuf
    = _bfd_elf_link_read_relocs (ibfd, isec, NULL, NULL, info->keep_memory);
  if (!irelbuf)
    goto error_return;

  for (ireloc = 0; ireloc < isec->reloc_count; ireloc++)
    {
      bfd_vma at_base, to_base, t_base;
      bfd_vma at, t_at, from, to;
      bfd_signed_vma offset;
      unsigned int r_type;
      bfd_boolean is_def;

      irel = irelbuf + ireloc;

      /* Look into relocation overflows at branches and add trampolines if needed. */
      r_type = ELF32_R_TYPE (irel->r_info);
      if (info->hexagon_trampolines
          && (r_type == R_HEXAGON_B32_PCREL_X
              || r_type == R_HEXAGON_B22_PCREL_X
              || r_type == R_HEXAGON_B15_PCREL_X
              || r_type == R_HEXAGON_B13_PCREL_X
              || r_type == R_HEXAGON_B9_PCREL_X
              || r_type == R_HEXAGON_B22_PCREL
              || r_type == R_HEXAGON_B15_PCREL
              || r_type == R_HEXAGON_B13_PCREL
              || r_type == R_HEXAGON_B9_PCREL))
        {
          isec_size = bfd_section_size (ibfd, isec);

          at      = irel->r_offset;
          at_base = isec->output_section->vma + isec->output_offset;
          if (at >= isec_size)
	    {
/*
	      (*_bfd_error_handler) (_("%B: 0x%lx: warning: bad relocation"),
                                     ibfd, NULL,
                                     (unsigned long) irel->r_offset);
*/
	      continue;
	    }
          /* This will do for now, but it must be corrected since the addendum
             is also offset by the position of the branch in the packet. */
          from = irel->r_offset;

          symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
          r_symndx   = ELF32_R_SYM (irel->r_info);
          sym_hashes = elf_sym_hashes (ibfd);

          /* Read this BFD's symbols if we haven't done so already. */
          if (isymbuf == NULL && symtab_hdr->sh_info != 0)
	    {
	      isymbuf = (Elf_Internal_Sym *) symtab_hdr->contents;
	      if (isymbuf == NULL)
	        isymbuf = bfd_elf_get_elf_syms (ibfd, symtab_hdr,
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
              asec = bfd_section_from_elf_index (ibfd, isym->st_shndx);

              name = bfd_malloc (sizeof (l_count) * 2 + 1);
              sprintf (name, "%0*lx",
		       (int) sizeof (l_count) * 2, (long) l_count++);

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
                    if (info->shared)
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
          offset = llabs ((to + to_base) - (from + at_base));
          if ((is_def && (((r_type == R_HEXAGON_B32_PCREL_X
	                    || r_type == R_HEXAGON_B22_PCREL_X
	                    || r_type == R_HEXAGON_B15_PCREL_X
	                    || r_type == R_HEXAGON_B13_PCREL_X
	                    || r_type == R_HEXAGON_B9_PCREL_X)
                           && HEXAGON_TRAMPOLINE_NEEDED (offset, 32))
                          || ((r_type == R_HEXAGON_B22_PCREL)
                              && HEXAGON_TRAMPOLINE_NEEDED (offset, 24))
                          || ((r_type == R_HEXAGON_B15_PCREL)
                              && HEXAGON_TRAMPOLINE_NEEDED (offset, 17))
                          || ((r_type == R_HEXAGON_B13_PCREL)
                              && HEXAGON_TRAMPOLINE_NEEDED (offset, 15))
                          || ((r_type == R_HEXAGON_B9_PCREL)
                              && HEXAGON_TRAMPOLINE_NEEDED (offset, 11))))
              || !is_def)
	    {
              /* Try to add a trampoline. */

              /* Allocate new contents. */
              contents = elf_section_data (isec)->this_hdr.contents;
	      if (contents == NULL)
	      {
	          contents = bfd_malloc (isec_size);
	          if (contents == NULL)
		    goto error_return;

	          if (!bfd_get_section_contents (ibfd, isec, contents,
					         (file_ptr) 0, isec_size))
		    goto error_return;

                elf_section_data (isec)->this_hdr.contents = contents;
	      }

            /* Subtract the offset of the branch into the packet from
               the addendum. */
            for (;
                 from >= HEXAGON_INSN_LEN;
                 from -= HEXAGON_INSN_LEN, irel->r_addend -= HEXAGON_INSN_LEN)
              {
                hexagon_insn insn;

                insn = hexagon_get_insn (ibfd, hexagon_elf_howto_table + r_type,
                                         contents + from - HEXAGON_INSN_LEN);
                if (HEXAGON_END_PACKET_GET (insn) == HEXAGON_END_PACKET
                    || HEXAGON_END_PACKET_GET (insn) == HEXAGON_END_PAIR)
                  break;
              }

              /* Create a symbol for the trampoline. */
              t_name = bfd_malloc (HEXAGON_TRAMPOLINE_PREFIX_LEN + 1
                                   + strlen (name) + 1);
              sprintf (t_name, "%s_%s", HEXAGON_TRAMPOLINE_PREFIX, name);

              /* Try to find it, otherwise, create it. */
              t_h = bfd_link_hash_lookup (&t_hash, t_name, FALSE, FALSE, FALSE);
              if (!t_h)
                {
                  t_at = isec_size;

                  if (((r_type == R_HEXAGON_B32_PCREL_X
		        || r_type == R_HEXAGON_B22_PCREL_X
		        || r_type == R_HEXAGON_B15_PCREL_X
		        || r_type == R_HEXAGON_B13_PCREL_X
		        || r_type == R_HEXAGON_B9_PCREL_X)
                       && HEXAGON_TRAMPOLINE_NEEDED (t_at - from, 31))
                      || ((r_type == R_HEXAGON_B22_PCREL)
                          && HEXAGON_TRAMPOLINE_NEEDED (t_at - from, 23))
                      || ((r_type == R_HEXAGON_B15_PCREL)
                          && HEXAGON_TRAMPOLINE_NEEDED (t_at - from, 16))
                      || ((r_type == R_HEXAGON_B13_PCREL)
                          && HEXAGON_TRAMPOLINE_NEEDED (t_at - from, 14))
                      || ((r_type == R_HEXAGON_B9_PCREL)
                          && HEXAGON_TRAMPOLINE_NEEDED (t_at - from, 10)))
                    /* No room for a trampoline. */
                    goto error_return;

                  isec_size += hexagon_trampolines [0].length
                               * sizeof (hexagon_trampolines [0].insns [0]);

                  /* Add room for the trampoline. */
                  elf_section_data (isec)->this_hdr.contents = contents =
                    bfd_realloc (contents, isec_size);

                  rc = bfd_set_section_size (ibfd, isec, isec_size);
                  if (rc != TRUE)
                    goto error_return;

                  isec_size = get_section_size_now (ibfd, isec);

                  /* Create the trampoline symbol. */
                  BFD_ASSERT (t_h = bfd_link_hash_lookup
                                      (&t_hash, t_name, TRUE, TRUE, FALSE));
                  t_h->u.def.value   = t_at;
                  t_h->u.def.section = isec;

                  /* Add trampoline at the end of the section. */
                  for (i = j = 0;
                       j < hexagon_trampolines [0].length;
                       i += sizeof (hexagon_trampolines [0].insns [j]), j++)
                    bfd_put_32 (ibfd, hexagon_trampolines [0].insns [j],
                                contents + t_at + i);

                  /* Add relocations for the trampoline. */
                  if (hexagon_trampolines [0].r_type)
                    {
                      /* Reuse the original relocation. */
                      irel->r_info
                        = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
                                        hexagon_trampolines [0].r_type);
                      irel->r_offset = t_h->u.def.value
                                       + hexagon_trampolines [0].offset;
                      /* The relocation addendum remains the same. */
                    }
                }
              else
                /* Remove the offending relocation. */
                /* FIXME: Could this relocation be zeroed up??? */
                irel->r_info =
                  ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_HEXAGON_NONE);

              free (t_name);

              /* Get the effective address of the trampoline. */
              t_at   = t_h->u.def.value;
              t_base = t_h->u.def.section->vma +
                       + t_h->u.def.section->output_offset;

              /* Fix up the offending branch by pointing it to the trampoline. */
              insn = hexagon_get_insn (ibfd,
                                       hexagon_elf_howto_table + r_type,
                                       contents + at);
              if (hexagon_reloc_operand (hexagon_elf_howto_table + r_type, &insn,
                                         t_at - from, NULL))
                hexagon_put_insn (ibfd, hexagon_elf_howto_table + r_type,
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

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
hexagon_elf_copy_indirect_symbol
(struct bfd_link_info *info,
 struct elf_link_hash_entry *dir, struct elf_link_hash_entry *ind)
{
  hexagon_link_hash_entry *edir, *eind;

  edir = hexagon_hash_entry (dir);
  eind = hexagon_hash_entry (ind);

  if (eind->dyn_relocs)
    {
      if (edir->dyn_relocs)
	{
	  hexagon_dyn_reloc *p, **pp;

	  if (ind->root.type == bfd_link_hash_indirect)
	    abort ();

	  /* Add relocation counts against the weak symbol to the strong
	     symbol list.  Merge any entries against the same section. */
	  for (pp = &eind->dyn_relocs; (p = *pp); )
	    {
	      hexagon_dyn_reloc *q;

	      for (q = edir->dyn_relocs; q; q = q->next)
		if (q->sec == p->sec)
		  {
		    q->pc_count += p->pc_count;
		    q->count += p->count;
		    *pp = p->next;
		    break;
		  }
	      if (!q)
		pp = &p->next;
	    }
	  *pp = edir->dyn_relocs;
	}

      edir->dyn_relocs = eind->dyn_relocs;
      eind->dyn_relocs = NULL;
    }

  if (ELIMINATE_COPY_RELOCS
      && ind->root.type != bfd_link_hash_indirect
      && dir->dynamic_adjusted)
    {
      /* If called to transfer flags for a weakdef during processing
	 of elf_adjust_dynamic_symbol, don't copy non_got_ref.
	 We clear it ourselves for ELIMINATE_COPY_RELOCS.  */
      dir->ref_dynamic |= ind->ref_dynamic;
      dir->ref_regular |= ind->ref_regular;
      dir->ref_regular_nonweak |= ind->ref_regular_nonweak;
      dir->needs_plt |= ind->needs_plt;
      dir->pointer_equality_needed |= ind->pointer_equality_needed;
    }
  else
    _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Create an entry in an ELF linker hash table.  */

static struct bfd_hash_entry *
hexagon_elf_link_hash_newfunc
(struct bfd_hash_entry *entry, struct bfd_hash_table *table, const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (!entry
      && !(entry = bfd_hash_allocate (table, sizeof (hexagon_link_hash_entry))))
    return (entry);

  /* Call the allocation method of the superclass.  */
  if ((entry = _bfd_elf_link_hash_newfunc (entry, table, string)))
    {
      hexagon_hash_entry (entry)->dyn_relocs = NULL;
      hexagon_hash_entry (entry)->ok_got.refcount = 0;
      hexagon_hash_entry (entry)->gd_got.refcount = 0;
      hexagon_hash_entry (entry)->ie_got.refcount = 0;
    }

  return (entry);
}

/* Create an ELF linker hash table.  */

static struct bfd_link_hash_table *
hexagon_elf_link_hash_table_create
(bfd *abfd)
{
  hexagon_link_hash_table *htab;

  if (!(htab = bfd_zalloc (abfd, sizeof (*htab))))
    return NULL;

  if (!_bfd_elf_link_hash_table_init
         (&htab->elf, abfd, hexagon_elf_link_hash_newfunc,
          sizeof (struct _hexagon_link_hash_entry)))
    {
      free (htab);
      return NULL;
    }

  htab->sym_cache.abfd = NULL;

  return (&htab->elf.root);
}

/* We may need to bump up the number of program headers beyond .text and .data. */

static int
hexagon_elf_additional_program_headers
(bfd *abfd, struct bfd_link_info *info ATTRIBUTE_UNUSED)
{
  asection *s;
  int extra = 0;

  /* In a good family, it would be merged with text. */
  s = bfd_get_section_by_name (abfd, ".hash");
  if (s && (s->flags & SEC_LOAD))
    extra++;

  /* Just in case, as used only by stand-alone mode. */
  s = bfd_get_section_by_name (abfd, ".start");
  if (s && (s->flags & SEC_LOAD))
    extra++;

  s = bfd_get_section_by_name (abfd, ".text");
  if (s && (s->flags & SEC_LOAD))
    extra++;

  s = bfd_get_section_by_name (abfd, ".rodata");
  if (s && (s->flags & SEC_LOAD))
    extra++;

  return (extra);
}

/* Check if section can be ignored for the purpose of checking for
   dangling relocations against discarded sections. */

static bfd_boolean
hexagon_elf_ignore_discarded_relocs
(asection *s ATTRIBUTE_UNUSED)
{
  /* Always ignore because relocate_section () will get rid of
     dangling relocations. */
  return TRUE;
}

static asection *
hexagon_elf_gc_mark_hook
(asection *sec, struct bfd_link_info *info ATTRIBUTE_UNUSED,
 Elf_Internal_Rela *rel ATTRIBUTE_UNUSED, struct elf_link_hash_entry *h,
 Elf_Internal_Sym *sym)
{
  if (h)
    {
      /* Global symbol. */
      switch (h->root.type)
	{
	case bfd_link_hash_defined:
	case bfd_link_hash_defweak:
	  return h->root.u.def.section;

	case bfd_link_hash_common:
	  return h->root.u.c.p->section;

	default:
	  break;
	}
    }
  else
    /* Local symbol. */
    return (bfd_section_from_elf_index (sec->owner, sym->st_shndx));

  return NULL;
}

static bfd_boolean
hexagon_elf_relocate_section
(bfd *obfd, struct bfd_link_info *info,
 bfd *ibfd, asection *isection, bfd_byte *contents,
 Elf_Internal_Rela *relocs, Elf_Internal_Sym *local_syms,
 asection **local_sections)
{
  hexagon_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_vma *local_got_offsets;
  Elf_Internal_Rela *rel;

  if (info->relocatable)
    return TRUE;

  htab = hexagon_hash_table (info);
  symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (ibfd);
  local_got_offsets = elf_local_got_offsets (ibfd);

  for (rel = relocs; rel < relocs + isection->reloc_count; rel++)
    {
      unsigned int r_type;
      reloc_howto_type *howto;
      size_t r_symndx;
      Elf_Internal_Sym *sym = NULL;
      asection *sec = NULL;
      struct elf_link_hash_entry *h = NULL;
      hexagon_link_hash_entry *eh = NULL;
      bfd_vma relocation, offset;
      bfd_vma lmask = ~(bfd_vma) 0, rmask = ~(bfd_vma) 0;
      bfd_reloc_status_type r;
      asection *sreloc = NULL;
      const char *name = NULL;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      if (r_type >= R_HEXAGON_max)
	{
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}
      else
        howto = hexagon_elf_howto_table + r_type;

      if (r_symndx < symtab_hdr->sh_info)
	{
          /* Local symbol. */
	  sym = local_syms + r_symndx;
	  sec = local_sections [r_symndx];

          if (sec && elf_discarded_section (sec))
            {
              /* For relocs against symbols from removed link-once sections,
                 or sections discarded by a linker script, avoid
                 any special processing. */
              rel->r_info   = 0;
              rel->r_addend = 0;
              continue;
            }

	  name = bfd_elf_string_from_elf_section
	    (ibfd, symtab_hdr->sh_link, sym->st_name);
	  if (!name || !*name)
            name = bfd_section_name (ibfd, sec);

	  relocation = _bfd_elf_rela_local_sym (obfd, sym, &sec, rel);
	}
      else
	{
          /* Global symbol. */
	  h = sym_hashes [r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	  eh = hexagon_hash_entry (h);

	  name = h->root.root.string;

	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	    /* TODO: is this condition OK for dynamic linking? */
              if (elf_discarded_section (h->root.u.def.section)
                  && !hexagon_kept_hash_lookup (obfd, info, ibfd, rel, h))
                {
                  if (isection->flags & SEC_DEBUGGING)
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
                        (info, h->root.root.string, ibfd,
                         isection, rel->r_offset, 0);
                      return FALSE;
                    }
                }

              /* TODO: in some cases, the output section will not be knwon,
                 such as when the symbol is in a DSO, otherwise consider them
                 errors. */
              relocation = TRUE;
	      switch (r_type)
		{
	        case R_HEXAGON_B22_PCREL:
	        case R_HEXAGON_B15_PCREL:
	        case R_HEXAGON_B13_PCREL:
	        case R_HEXAGON_B7_PCREL:
                  /* TODO: In PPC the tests in all cases include visibility too. */
	          if (info->shared
	              && (!SYMBOLIC_BIND (info, h)
	                  || !h->def_regular)
	              && (isection->flags & SEC_ALLOC))
	            relocation = 0;

		  /* Fall-through. */

	        case R_HEXAGON_PLT_B22_PCREL:
	          if (h->plt.offset != -(bfd_vma) 1)
	            relocation = 0;
		  break;

	        case R_HEXAGON_GOT_LO16:
	        case R_HEXAGON_GOT_HI16:
                case R_HEXAGON_GOT_32:
	        case R_HEXAGON_GOT_16:
	          if (elf_hash_table (info)->dynamic_sections_created
	              && (info->executable
	                  || !SYMBOLIC_BIND (info, h)
	                  || !h->def_regular))
	            relocation = 0;
		  break;

                case R_HEXAGON_32:
                  if (h->dynindx != -1
                      && (info->executable
                          || !SYMBOLIC_BIND (info, h)
                          || !h->def_regular)
		      && !h->non_got_ref)
                    relocation = 0;
                  break;

	        default:
		  if (!h->root.u.def.section->output_section)
		    {
		      relocation = 0;
		      (*_bfd_error_handler)
			(_("%B: relocation %s for symbol `%s\' in section `%A\' " \
			   "cannot be resolved"),
			 ibfd, isection,
			 hexagon_elf_howto_table [r_type].name, name);
                      bfd_set_error (bfd_error_bad_value);
                      return FALSE;
		    }
		}

              if (relocation)
                relocation = h->root.u.def.value
                             + h->root.u.def.section->output_section->vma
                             + h->root.u.def.section->output_offset;
	    }
	  else if (h->root.type == bfd_link_hash_undefweak)
            relocation = 0;
	  else if (info->shared
		   && info->unresolved_syms_in_objects <= RM_IGNORE
		   && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
            relocation = 0;
          else
	    {
	      if (!((*info->callbacks->undefined_symbol)
		      (info, h->root.root.string, ibfd,
		       isection, rel->r_offset,
		       info->executable
		       || info->unresolved_syms_in_objects >= RM_GENERATE_WARNING
		       || ELF_ST_VISIBILITY (h->other))))
		return FALSE;
	      relocation = 0;
	    }
	}

      /* Check relocations for correctness. */
      switch (r_type)
        {
	case R_HEXAGON_GOTREL_LO16:
	case R_HEXAGON_GOTREL_HI16:
	case R_HEXAGON_GOTREL_32:
	  /* This can happen if we get a link error with the input ELF
	     variant mismatching the output variant.  Emit an error so
	     it's noticed if it happens elsewhere.  */
	  if (!htab->elf.sgotplt)
	    {
	      (*_bfd_error_handler)
		(_("%B: relocation %s for symbol `%s\' in section `%A\' " \
		   "cannot be handled with no GOT created"),
		 ibfd, isection,
		 hexagon_elf_howto_table [r_type].name, name);
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }
          break;

        case R_HEXAGON_B15_PCREL:
        case R_HEXAGON_B13_PCREL:
        case R_HEXAGON_B7_PCREL:
	  if (!h || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
            /* If the symbol was local, we need no DSO-specific handling.  */
	    break;

#if 0
          /* The range for these relocations is too small to be suitable for
             dynamic symbols. */
	  if (h->plt.offset == -(bfd_vma) 1
	      || !htab->plt.s)
	    {
	      (*_bfd_error_handler)
		(_("%s: relocation %s for symbol `%s\' in section `%s\' " \
		   "cannot be handled due to insufficient range"),
		 bfd_archive_filename (input_bfd),
		 hexagon_elf_howto_table [r_type].name,
		 name,
		 bfd_get_section_name (input_bfd, input_section));
	      bfd_set_error (bfd_error_bad_value);
	      return FALSE;
	    }
#endif
          break;
        }

      /* Preprocess relocations for dynamic linking. */
      switch (r_type)
        {
        case R_HEXAGON_GOT_LO16:
        case R_HEXAGON_GOT_HI16:
        case R_HEXAGON_GOT_32:
        case R_HEXAGON_GOT_16:
          if (h)
            {
              /* Global symbol. */
              offset = h->got.offset;
              BFD_ASSERT (offset != -(bfd_vma) 1);

	      if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL
	             (htab->elf.dynamic_sections_created, info->shared, h)
		  || (info->shared
		      && SYMBOL_REFERENCES_LOCAL (info, h))
		  || (ELF_ST_VISIBILITY (h->other)
		      && h->root.type == bfd_link_hash_undefweak))
                {
                  /* This wasn't checked above for non-shared, but must hold
                     true through here: the symbol must be defined in the
                     program, or be weakly undefined. */
                  BFD_ASSERT (!elf_hash_table (info)->dynamic_sections_created
                              || info->shared
                              || h->def_regular
                              || h->root.type == bfd_link_hash_undefweak);

                  /* This is actually a static link, or it is a -Bsymbolic
                     link and the symbol is defined locally, or is weakly
                     undefined, or the symbol was forced to be local because
                     of a version file, or not linking a dynamic object.
                     This GOT entry must be initialized.  Since the offset
                     must always be a multiple of 4, the least significant bit
                     is used to record whether the entry has already been
                     initialized.

                     If this GOT entry should be initialized at run-time,
                     a relocation in .rela.got willbe created then.  This
                     is done in finish_dynamic_symbol. */
                  if ((offset & 1))
                    offset &= ~1;
                  else
		    h->got.offset |= 1;
                }
            }
          else
            {
              /* Local symbol. */
              BFD_ASSERT (local_got_offsets
                          && local_got_offsets [r_symndx] != -(bfd_vma) 1);

              offset = local_got_offsets [r_symndx];

              /* Since the offset must always be a multiple of 4, the least
                 significant bit is used to record whether the GOT entry has
                 already been initialized. */
              if ((offset & 1))
                offset &= ~1;
              else
                {
                  bfd_put_32
                    (obfd, relocation, htab->elf.sgot->contents + offset);

                  if (info->shared)
                    {
                      asection *s;
                      Elf_Internal_Rela outrel;
                      bfd_byte *loc;

                      s = bfd_get_section_by_name
                            (elf_hash_table (info)->dynobj, ".rela.got");
                      BFD_ASSERT (s);

                      outrel.r_offset = htab->elf.sgot->output_section->vma
                                        + htab->elf.sgot->output_offset
                                        + offset;
                      outrel.r_info = ELF32_R_INFO (0, R_HEXAGON_RELATIVE);
                      outrel.r_addend = relocation;
                      loc = s->contents
                            + s->reloc_count++ * sizeof (Elf32_External_Rela);
                      bfd_elf32_swap_reloca_out (obfd, &outrel, loc);
                    }

                  local_got_offsets [r_symndx] |= 1;
                }
            }
          relocation = htab->elf.sgot->output_section->vma
                       + htab->elf.sgot->output_offset
                       - htab->elf.sgotplt->output_section->vma
                       - htab->elf.sgotplt->output_offset
                       + offset;
          break;

	case R_HEXAGON_GOTREL_LO16:
	case R_HEXAGON_GOTREL_HI16:
	case R_HEXAGON_GOTREL_32:
	  /* These relocations are like a PC-relative one, except the
	     reference point is the location of _GLOBAL_OFFSET_TABLE_.  Note
	     that sgot->output_offset is not involved in this calculation.
	     We always want the start of entire GOT section, or wherever
	     _GLOBAL_OFFSET_TABLE_ is, not the position after the reserved
	     header.  */
	  relocation -= htab->elf.sgotplt->output_section->vma
	                + htab->elf.sgotplt->output_offset;
	  break;

        case R_HEXAGON_PLT_B22_PCREL:
        case R_HEXAGON_B22_PCREL:
        case R_HEXAGON_B15_PCREL:
        case R_HEXAGON_B13_PCREL:
        case R_HEXAGON_B7_PCREL:
	  /* Relocation is to the entry for this symbol in the PLT. */
	  if (!h || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
            /* Resolve a PLT relocation against a local symbol directly,
               without using the PLT.  */
	    break;

	  if (h->plt.offset != -(bfd_vma) 1
	      && htab->elf.splt)
            {
              /* We did make a PLT entry for this symbol.  This does not happen
                 when linking PIC code statically or when using -Bsymbolic. */
              relocation = htab->elf.splt->output_section->vma
                           + htab->elf.splt->output_offset
                           + h->plt.offset;
              break;
            }

          if (r_type == R_HEXAGON_PLT_B22_PCREL)
	    break;

	  /* Fall-through.  */

        case R_HEXAGON_LO16:
        case R_HEXAGON_HI16:
        case R_HEXAGON_HL16:
        case R_HEXAGON_32:
        case R_HEXAGON_16:
        case R_HEXAGON_8:
	  /* r_symndx will be zero only for relocations against symbols from
	     removed link-once sections or from sections discarded by the
	     linker script. */
	  if (!r_symndx || !(isection->flags & SEC_ALLOC))
	    break;

	  if ((info->shared
	       && (!h
		   || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
		   || h->root.type != bfd_link_hash_undefweak)
	       && ((r_type != R_HEXAGON_B22_PCREL
		    && r_type != R_HEXAGON_B15_PCREL
		    && r_type != R_HEXAGON_B13_PCREL
		    && r_type != R_HEXAGON_B7_PCREL)
		   || !SYMBOL_CALLS_LOCAL (info, h)))
	      || (ELIMINATE_COPY_RELOCS
		  && info->executable
		  && h
		  && h->dynindx != -1
		  && !h->non_got_ref
		  && ((h->def_dynamic
		       && !h->def_regular)
		      || h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined)))
	    {
	      /* When generating a DSO, these relocations are copied into
	         the output file to be resolved at run-time.  */

	      Elf_Internal_Rela outrel;
	      bfd_byte *loc;
	      bfd_boolean skip, relocate;

	      skip = FALSE;
	      relocate = FALSE;

	      outrel.r_offset =
		_bfd_elf_section_offset
		  (obfd, info, isection, rel->r_offset);
	      if (outrel.r_offset == -(bfd_vma) 1)
		skip = TRUE;
	      else if (outrel.r_offset == -(bfd_vma) 2)
		skip = TRUE, relocate = TRUE;

	      outrel.r_offset += isection->output_section->vma
                                 + isection->output_offset;

	      if (skip)
		memset (&outrel, 0, sizeof (Elf32_External_Rela));
	      /* h->dynindx may be -1 if the symbol was marked to become local.  */
	      else if (h
		       && h->dynindx != -1
		       && (r_type == R_HEXAGON_B22_PCREL
			   || r_type == R_HEXAGON_B15_PCREL
			   || r_type == R_HEXAGON_B13_PCREL
			   || r_type == R_HEXAGON_B7_PCREL
			   || info->executable
			   || !SYMBOLIC_BIND (info, h)
			   || !h->def_regular))
		{
		  outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		  outrel.r_addend = relocation + rel->r_addend;
		}
	      else
		{
		  if (r_type == R_HEXAGON_32)
		    {
		      relocate = TRUE;
		      outrel.r_info = ELF32_R_INFO (0, R_HEXAGON_RELATIVE);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		  else
		    {
		      long indx;

		      if (!h)
			sec = local_sections [r_symndx];
		      else
			{
			  BFD_ASSERT (h->root.type == bfd_link_hash_defined
				      || (h->root.type == bfd_link_hash_defweak));
			  sec = h->root.u.def.section;
			}
		      if (sec && bfd_is_abs_section (sec))
			indx = 0;
		      else if (!sec || !sec->owner)
			{
			  bfd_set_error (bfd_error_bad_value);
			  return FALSE;
			}
		      else
			{
			  asection *osec;

			  osec = sec->output_section;
			  indx = elf_section_data (osec)->dynindx;
			  BFD_ASSERT (indx > 0);
			}

		      outrel.r_info = ELF32_R_INFO (indx, r_type);
		      outrel.r_addend = relocation + rel->r_addend;
		    }
		}

	      sreloc = elf_section_data (isection)->sreloc;
	      /* FIXME: This can happen when creating a DSO and an object
	         contains a non-PLT branch to a symbol in another DSO.
	         The same condition is harmless when creating a program. */
	      BFD_ASSERT (sreloc && sreloc->contents);

	      loc = sreloc->contents
	            + sreloc->reloc_count++ * sizeof (Elf32_External_Rela);
	      bfd_elf32_swap_reloca_out (obfd, &outrel, loc);

	      /* This reloc will be computed at runtime, so there's no
                 need to do anything now, except for R_HEXAGON_32 relocations
                 that have been turned into R_HEXAGON_RELATIVE.  */
	      if (!relocate)
		continue;
	    }
	  break;
        }

      /* Mirrors what is done above for dynamic linking and preprocess the
         relocations for TLS types. */
      switch (r_type)
        {
	case R_HEXAGON_GD_GOT_LO16:
	case R_HEXAGON_GD_GOT_HI16:
	case R_HEXAGON_GD_GOT_32:
	case R_HEXAGON_GD_GOT_16:
	  if (h)
	    {
	      /* Global symbol. */
	      BFD_ASSERT (h->got.offset != -(bfd_vma) 1);
	      offset = h->got.offset;

	      if ((offset & 1))
		offset &= ~1;
	      else
		h->got.offset |= 1;
	    }
	  else
	    {
	      /* Local symbol. */
	      if (!local_got_offsets)
		abort ();
	      offset = local_got_offsets [r_symndx];

	      if ((offset & 1))
		offset &= ~1;
	      else
		local_got_offsets [r_symndx] |= 1;
	    }

	  relocation  = htab->elf.sgot->output_section->vma
			+ htab->elf.sgot->output_offset
			- htab->elf.sgotplt->output_section->vma
			- htab->elf.sgotplt->output_offset
			+ offset;
	  break;

	case R_HEXAGON_IE_GOT_LO16:
	case R_HEXAGON_IE_GOT_HI16:
	case R_HEXAGON_IE_GOT_32:
	case R_HEXAGON_IE_GOT_16:
	  {
	    bfd_vma adjust = 0;

	    if (h)
	      {
		/* Global symbol. */
		if (eh->gd_got.refcount > 0)
		  adjust = GOT_ENTRY_SIZE * 2;

		BFD_ASSERT (h->got.offset != -(bfd_vma) 1);
		offset = h->got.offset;

		if ((offset & 1))
		  offset &= ~1;
		else
		  h->got.offset |= 1;
	      }
	    else
	      {
		/* Local symbol. */
		if (!local_got_offsets)
		  abort ();
		offset = local_got_offsets [r_symndx];

		if ((offset & 1))
		  offset &= ~1;
		else
		  local_got_offsets [r_symndx] |= 1;
	      }

	    bfd_put_32 (obfd, hexagon_elf_tpoff (info, relocation),
			htab->elf.sgot->contents + offset + adjust);

	    relocation  = htab->elf.sgot->output_section->vma
			  + htab->elf.sgot->output_offset
			  - htab->elf.sgotplt->output_section->vma
			  - htab->elf.sgotplt->output_offset
			  + offset + adjust;
	  }
	  break;

	case R_HEXAGON_TPREL_LO16:
	case R_HEXAGON_TPREL_HI16:
	case R_HEXAGON_TPREL_32:
	case R_HEXAGON_TPREL_16:
	  relocation = hexagon_elf_tpoff (info, relocation);
	  break;
        }

      /* Apply relocation. */
      switch (r_type)
	{
	  hexagon_insn insn;

        case R_HEXAGON_32_6_X:
          rmask = howto->src_mask;
          /* Fall through. */

        case R_HEXAGON_6_X:
        case R_HEXAGON_7_X:
        case R_HEXAGON_8_X:
        case R_HEXAGON_9_X:
        case R_HEXAGON_10_X:
        case R_HEXAGON_11_X:
        case R_HEXAGON_12_X:
        case R_HEXAGON_16_X:
          lmask = ~(~(bfd_vma) 0 << howto->bitsize);
          /* Fall through. */

	case R_HEXAGON_LO16:
	case R_HEXAGON_HI16:
	case R_HEXAGON_GOTREL_LO16:
	case R_HEXAGON_GOTREL_HI16:
	case R_HEXAGON_GOT_LO16:
	case R_HEXAGON_GOT_HI16:
	case R_HEXAGON_GOT_16:
        case R_HEXAGON_GD_GOT_LO16:
        case R_HEXAGON_GD_GOT_HI16:
        case R_HEXAGON_IE_GOT_LO16:
        case R_HEXAGON_IE_GOT_HI16:
        case R_HEXAGON_GD_GOT_16:
        case R_HEXAGON_IE_GOT_16:
	  offset = (relocation + rel->r_addend) & lmask & rmask;

	  insn = hexagon_get_insn (ibfd, howto, contents + rel->r_offset);

	  if (!hexagon_reloc_operand (howto, &insn, offset, NULL)
              && h && h->root.type != bfd_link_hash_undefined)
            r = info->callbacks->reloc_overflow
                  (info, h? &h->root: NULL, name, howto->name, 0,
                   ibfd, isection, rel->r_offset);
	  else
            hexagon_put_insn (ibfd, howto, contents + rel->r_offset, insn);

	  break;

        case R_HEXAGON_HL16:
	  offset = relocation + rel->r_addend;

          /* First instruction (HI). */
	  insn = hexagon_get_insn (ibfd, hexagon_elf_howto_table + R_HEXAGON_HI16,
                                 contents + rel->r_offset + 0);

	  if (!hexagon_reloc_operand
	         (hexagon_elf_howto_table + R_HEXAGON_HI16, &insn, offset, NULL)
              && h && h->root.type != bfd_link_hash_undefined)
            r = info->callbacks->reloc_overflow
                  (info, (h ? &h->root : NULL), name,
                   hexagon_elf_howto_table [R_HEXAGON_HI16].name, 0,
                   ibfd, isection, rel->r_offset + sizeof (insn));
	  else
            hexagon_put_insn (ibfd, hexagon_elf_howto_table + R_HEXAGON_HI16,
                            contents + rel->r_offset + 0, insn);

          /* Second instruction (LO). */
	  insn = hexagon_get_insn (ibfd, hexagon_elf_howto_table + R_HEXAGON_LO16,
                                 contents + rel->r_offset + sizeof (insn));

	  if (!hexagon_reloc_operand
	         (hexagon_elf_howto_table + R_HEXAGON_LO16, &insn, offset, NULL)
              && h && h->root.type != bfd_link_hash_undefined)
            r = info->callbacks->reloc_overflow
                  (info, (h ? &h->root : NULL), name,
                   hexagon_elf_howto_table [R_HEXAGON_LO16].name, 0,
                   ibfd, isection, rel->r_offset + sizeof (insn));
	  else
            hexagon_put_insn (ibfd, hexagon_elf_howto_table + R_HEXAGON_LO16,
                            contents + rel->r_offset + sizeof (insn), insn);

          break;

	case R_HEXAGON_GPREL16_0:
	case R_HEXAGON_GPREL16_1:
	case R_HEXAGON_GPREL16_2:
	case R_HEXAGON_GPREL16_3:
	  {
	    bfd_vma base;
	    struct bfd_link_hash_entry *h;

	    insn = hexagon_get_insn (ibfd, howto, contents + rel->r_offset);

	    h = bfd_link_hash_lookup (info->hash, SDA_BASE, FALSE, FALSE, TRUE);
	    if (!h)
              h = bfd_link_hash_lookup (info->hash, DEFAULT_SDA_BASE,
                                        FALSE, FALSE, TRUE);

	    if (h
		&& h->type == bfd_link_hash_defined)
	      {
                base = h->u.def.value
                       + h->u.def.section->output_section->vma
                       + h->u.def.section->output_offset;
                elf_gp (obfd) = base;
		relocation -= base;
	      }
	    else
	      {
		/* At this point, if no _SDA_BASE_, either the alternate or
		   the proper one, is resolved, then GP-relocaiton overflow
		   errors are likely. */
	      }
          }

          offset = relocation + rel->r_addend;

          if (!hexagon_reloc_operand (howto, &insn, offset, NULL)
              && h && h->root.type != bfd_link_hash_undefined)
            r = info->callbacks->reloc_overflow
                  (info, (h? &h->root: NULL), name, howto->name, 0,
                   ibfd, isection, rel->r_offset);
          else
            hexagon_put_insn (ibfd, howto, contents + rel->r_offset, insn);

          break;

        case R_HEXAGON_B32_PCREL_X:
        case R_HEXAGON_B22_PCREL_X:
        case R_HEXAGON_B15_PCREL_X:
        case R_HEXAGON_B13_PCREL_X:
        case R_HEXAGON_B9_PCREL_X:
          if (r_type == R_HEXAGON_B32_PCREL_X)
            rmask = ~(bfd_vma) 0 << howto->rightshift;
          else
            lmask = ~(~(bfd_vma) 0 << howto->bitsize);
          /* Fall through. */

	case R_HEXAGON_PLT_B22_PCREL:
	case R_HEXAGON_B22_PCREL:
	case R_HEXAGON_B15_PCREL:
	case R_HEXAGON_B13_PCREL:
        case R_HEXAGON_B9_PCREL:
	case R_HEXAGON_B7_PCREL:
	  offset = ((relocation + rel->r_addend)
                    - (isection->output_section->vma
                       + isection->output_offset
                       + rel->r_offset))
                   & lmask & rmask;

	  insn = hexagon_get_insn (ibfd, howto, contents + rel->r_offset);

	  if (!hexagon_reloc_operand (howto, &insn, offset, NULL)
              && h && h->root.type != bfd_link_hash_undefined)
            r = info->callbacks->reloc_overflow
                (info, (h ? &h->root : NULL), name, howto->name, 0,
                 ibfd, isection, rel->r_offset);
	  else
            hexagon_put_insn (ibfd, howto, contents + rel->r_offset, insn);

	  break;

	case R_HEXAGON_TPREL_LO16:
	case R_HEXAGON_TPREL_HI16:
	case R_HEXAGON_TPREL_16:
	  offset = relocation + rel->r_addend;

	  insn = hexagon_get_insn (ibfd, howto, contents + rel->r_offset);

	  if (!hexagon_reloc_operand (howto, &insn, offset, NULL)
	      && h && h->root.type != bfd_link_hash_undefined)
	    r = info->callbacks->reloc_overflow
		  (info, (h? &h->root: NULL), name, howto->name, 0,
		    ibfd, isection, rel->r_offset);
	  else
	    hexagon_put_insn (ibfd, howto, contents + rel->r_offset, insn);

	  break;

	case R_HEXAGON_32:
	case R_HEXAGON_16:
	case R_HEXAGON_8:
        case R_HEXAGON_32_PCREL:
        case R_HEXAGON_GOT_32:
        case R_HEXAGON_GOTREL_32:
        case R_HEXAGON_GD_GOT_32:
        case R_HEXAGON_IE_GOT_32:
	case R_HEXAGON_TPREL_32:
          /* Fall through. */

	case R_HEXAGON_NONE:
          /* Default relocation handling. */
          r = _bfd_final_link_relocate (howto, ibfd, isection,
                                        contents, rel->r_offset,
                                        relocation, rel->r_addend);
          if (r != bfd_reloc_ok)
            {
              const char *msg = NULL;

              switch (r)
                {
                case bfd_reloc_overflow:
                  r = info->callbacks->reloc_overflow
                        (info, (h? &h->root: NULL), name, howto->name, 0,
                         ibfd, isection, rel->r_offset);
                  break;

                case bfd_reloc_undefined:
                  r = info->callbacks->undefined_symbol
                        (info, name, ibfd, isection, rel->r_offset,
                         (info->executable
                          || info->unresolved_syms_in_objects >= RM_GENERATE_WARNING
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
                      (info, msg, name, ibfd, isection, rel->r_offset);

              if (!r)
                return FALSE;
            }
	  break;

	default:
          info->callbacks->warning
            (info, _("internal error: unrecognized relocation type"),
             name, ibfd, isection, rel->r_offset);
          return FALSE;
	}
    }

  return TRUE;
}

/* PIC & DSO */

/**
Look through the relocations during the first phase, and allocate space in
the GOT, the PLT and dynamic relocation sections.
*/

static bfd_boolean
hexagon_elf_check_relocs
(bfd *abfd, struct bfd_link_info *info,
 asection *sec, const Elf_Internal_Rela *relocs)
{
  hexagon_link_hash_table *htab;
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
      if (!hexagon_elf_create_dynamic_sections (abfd, info))
	return FALSE;
    }
#endif

  htab = hexagon_hash_table (info);

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  sreloc = NULL;

  for (rel = relocs; rel < relocs + sec->reloc_count; rel++)
    {
      unsigned int r_type;
      size_t r_symndx;
      hexagon_link_hash_entry *h;
      Elf_Internal_Sym *isym;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      if (r_symndx < symtab_hdr->sh_info)
	{
          /* Symbol is local. */
	  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					abfd, r_symndx);
	  if (!isym)
	    return FALSE;

          h = NULL;
        }
      else
        {
          /* Symbol is global. */
	  isym = NULL;

          h = (hexagon_link_hash_entry *)
                (sym_hashes [r_symndx - symtab_hdr->sh_info]);
          while (h->elf.root.type == bfd_link_hash_indirect
                 || h->elf.root.type == bfd_link_hash_warning)
            h = (hexagon_link_hash_entry *) h->elf.root.u.i.link;
        }

      /* Perform GOT, PLT and TLS accounting. */
      switch (r_type)
	{
	case R_HEXAGON_GOT_LO16:
	case R_HEXAGON_GOT_HI16:
	case R_HEXAGON_GOT_32:
	case R_HEXAGON_GOT_16:
	case R_HEXAGON_GD_GOT_LO16:
	case R_HEXAGON_GD_GOT_HI16:
	case R_HEXAGON_GD_GOT_32:
	case R_HEXAGON_GD_GOT_16:
	case R_HEXAGON_IE_GOT_LO16:
	case R_HEXAGON_IE_GOT_HI16:
	case R_HEXAGON_IE_GOT_32:
	case R_HEXAGON_IE_GOT_16:
          /* This symbol requires a GOT entry. */
          if (h)
	    {
	      /* Symbol is global. */
	      h->elf.got.refcount++;

	      switch (r_type)
		{
		case R_HEXAGON_GOT_LO16:
		case R_HEXAGON_GOT_HI16:
		case R_HEXAGON_GOT_32:
		case R_HEXAGON_GOT_16:
		  h->ok_got.refcount++;
		  break;

		case R_HEXAGON_GD_GOT_LO16:
		case R_HEXAGON_GD_GOT_HI16:
		case R_HEXAGON_GD_GOT_32:
		case R_HEXAGON_GD_GOT_16:
		  h->gd_got.refcount++;
		  break;

		case R_HEXAGON_IE_GOT_LO16:
		case R_HEXAGON_IE_GOT_HI16:
		case R_HEXAGON_IE_GOT_32:
		case R_HEXAGON_IE_GOT_16:
		  h->ie_got.refcount++;
		  break;
		}
	    }
          else
            {
              /* Symbol is local. */
              bfd_signed_vma *local_got_refcounts;

              /* This is a GOT entry for a local symbol. */
              local_got_refcounts = elf_local_got_refcounts (abfd);
              if (!local_got_refcounts)
                {
                  bfd_size_type size;

                  size = LGOT_SZ (symtab_hdr) * sizeof (*local_got_refcounts) + 1;
                  local_got_refcounts =
                    (bfd_signed_vma *) bfd_zalloc (abfd, size);
                  if (!local_got_refcounts)
                    return FALSE;
                  elf_local_got_refcounts (abfd) = local_got_refcounts;
                }

              local_got_refcounts [r_symndx]++;

	      switch (r_type)
		{
		case R_HEXAGON_GOT_LO16:
		case R_HEXAGON_GOT_HI16:
		case R_HEXAGON_GOT_32:
		case R_HEXAGON_GOT_16:
		  local_got_refcounts [LGOT_OK (symtab_hdr, r_symndx)]++;
		  break;

		case R_HEXAGON_GD_GOT_LO16:
		case R_HEXAGON_GD_GOT_HI16:
		case R_HEXAGON_GD_GOT_32:
		case R_HEXAGON_GD_GOT_16:
		  local_got_refcounts [LGOT_GD (symtab_hdr, r_symndx)]++;
		  break;

		case R_HEXAGON_IE_GOT_LO16:
		case R_HEXAGON_IE_GOT_HI16:
		case R_HEXAGON_IE_GOT_32:
		case R_HEXAGON_IE_GOT_16:
		  local_got_refcounts [LGOT_IE (symtab_hdr, r_symndx)]++;
		  break;
		}
            }
	  /* Fall through */

        case R_HEXAGON_GOTREL_LO16:
        case R_HEXAGON_GOTREL_HI16:
        case R_HEXAGON_GOTREL_32:
          /* This symbol requires a GOT. */
	  if (!htab->elf.sgotplt)
	    {
	      /* Create the GOT. */
	      if (!htab->elf.dynobj)
		htab->elf.dynobj = abfd;
	      if (!_bfd_elf_create_got_section (htab->elf.dynobj, info))
		return FALSE;
	    }
	  break;

        case R_HEXAGON_GPREL16_0:
        case R_HEXAGON_GPREL16_1:
        case R_HEXAGON_GPREL16_2:
        case R_HEXAGON_GPREL16_3:

        case R_HEXAGON_LO16:
        case R_HEXAGON_HI16:
        case R_HEXAGON_HL16:
	case R_HEXAGON_32:
        case R_HEXAGON_16:
        case R_HEXAGON_8:
	  if (h && info->executable)
	    {
	      /* If this relocation is in a read-only section, we might
		 need a copy-relocation.  We can't reliably check at
		 this stage whether the section is read-only, as input
		 sections have not yet been mapped to output sections.
		 Tentatively set the flag for now, and correct in
		 adjust_dynamic_symbol.  */
	      h->elf.non_got_ref = TRUE;
	      h->elf.pointer_equality_needed = TRUE;
	      /* A PLT entry may be needed if this relocation refers to
	         a function in a shared library.  */
	      h->elf.plt.refcount++;
	    }
          break;

	case R_HEXAGON_B22_PCREL:
	case R_HEXAGON_B15_PCREL:
	case R_HEXAGON_B13_PCREL:
	case R_HEXAGON_B7_PCREL:
	  if (h) /* && info->executable) */
	    {
	      /* A PLT entry may be needed if this relocation refers to
	         a function in a shared library.  */
	      h->elf.plt.refcount++;
	    }
          break;

	case R_HEXAGON_PLT_B22_PCREL:
	  /* This symbol requires a PLT entry.  We actually build the entry
	     in adjust_dynamic_symbol because this might be a case of
	     linking PIC code which is never referenced by a dynamic object,
	     when we don't need to generate a PLT entry after all.	 */
	  if (h)
	    {
	      /* But only if it's a global symbol, otherwise don't use a PLT. */
              h->elf.needs_plt = TRUE;
              h->elf.plt.refcount++;
	    }
          break;
       }

      /* Perform copy-relocation checks. */
      switch (r_type)
	{
        case R_HEXAGON_GPREL16_0:
        case R_HEXAGON_GPREL16_1:
        case R_HEXAGON_GPREL16_2:
        case R_HEXAGON_GPREL16_3:

        case R_HEXAGON_LO16:
        case R_HEXAGON_HI16:
        case R_HEXAGON_HL16:
	case R_HEXAGON_32:
        case R_HEXAGON_16:
        case R_HEXAGON_8:

        /* FIXME: How about R_HEXAGON_32_PCREL? */
	case R_HEXAGON_B22_PCREL:
	case R_HEXAGON_B15_PCREL:
	case R_HEXAGON_B13_PCREL:
	case R_HEXAGON_B7_PCREL:
	  /* If we are creating a shared library, and this is a relocation
	     against a global symbol, or a non PC-relative relocation
	     against a local symbol, then we need to copy the relocation
	     into the shared library.  However, if we are linking with
	     -Bsymbolic, we do not need to copy a relocation against a
	     global symbol which is defined in an object we are
	     including in the link (i.e., DEF_REGULAR is set).  At this
	     point we have not seen all the input files, so it is possible
	     that DEF_REGULAR is not set now but will be set later
	     (it is never cleared).  In case of a weak definition,
	     DEF_REGULAR may be cleared later by a strong definition in a
	     shared library.  We account for that possibility below by
	     storing information in the relocs_copied field of the hash table
	     entry.  A similar situation occurs when creating shared libraries
	     and symbol visibility changes render the symbol local.  If, on
	     the other hand, we are creating an executable, we may need to
	     keep relocations for symbols satisfied by a dynamic library if
	     we manage to avoid copy relocations for the symbol.  */
	  if ((info->shared
	       && (sec->flags & SEC_ALLOC)
	       && (((r_type != R_HEXAGON_B22_PCREL)
		    && (r_type != R_HEXAGON_B15_PCREL)
		    && (r_type != R_HEXAGON_B13_PCREL)
		    && (r_type != R_HEXAGON_B7_PCREL))
		   || (h
		       && (!SYMBOLIC_BIND (info, &h->elf)
			   || h->elf.root.type == bfd_link_hash_defweak
			   || !h->elf.def_regular))))
	      || ((ELIMINATE_COPY_RELOCS || info->nocopyreloc)
		  && info->executable
		  && (sec->flags & SEC_ALLOC)
		  && h
		  && (h->elf.root.type == bfd_link_hash_defweak
		      || !h->elf.def_regular)))
	    {
	      hexagon_dyn_reloc *p, **head;

	      /* We must copy these relocations into the output file. Create
	         a relocation section in the dynamic object and make room for
		 this relocation.  */
	      if (!sreloc)
		{
		  if (!htab->elf.dynobj)
		    htab->elf.dynobj = abfd;

		  sreloc = _bfd_elf_make_dynamic_reloc_section
		             (sec, htab->elf.dynobj, 2, abfd, TRUE);

		  if (!sreloc)
		    return FALSE;
		}

	      /* If this is a global symbol, we count the number of
		 relocations needed for this symbol.  */
	      if (h)
                head = &h->dyn_relocs;
	      else
		{
		  /* Track dynamic relocations needed for local symbols too.
		     We really need local symbols available to do this easily. */
		  asection *s;
		  Elf_Internal_Sym *isym;

		  isym = bfd_sym_from_r_symndx (&htab->sym_cache, abfd, r_symndx);
		  if (!isym)
		    return FALSE;

		  s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		  if (!s)
		    s = sec;

		  head = (hexagon_dyn_reloc **)
		           &elf_section_data (s)->local_dynrel;
		}

	      p = *head;
	      if (!p || p->sec != sec)
		{
		  p = (hexagon_dyn_reloc *) bfd_zalloc (htab->elf.dynobj, sizeof (*p));
		  if (!p)
		    return FALSE;

		  p->next = *head;
		  *head = p;
		  p->sec = sec;
		}

	      if (r_type == R_HEXAGON_32_PCREL
		  || r_type == R_HEXAGON_B22_PCREL
		  || r_type == R_HEXAGON_B15_PCREL
		  || r_type == R_HEXAGON_B13_PCREL
		  || r_type == R_HEXAGON_B7_PCREL)
		p->pc_count++;
              else
                p->count++;
	    }
          break;
        }

      /* Validate relocations. */
      switch (r_type)
	{
        case R_HEXAGON_GPREL16_0:
        case R_HEXAGON_GPREL16_1:
        case R_HEXAGON_GPREL16_2:
        case R_HEXAGON_GPREL16_3:
          /* These relocations cannot be used in shared libraries because
             there should not be an SDA in a shared library for the time being. */

        case R_HEXAGON_LO16:
        case R_HEXAGON_HI16:
          /* These relocations cannot be used in shared libraries. */

        case R_HEXAGON_HL16:
        case R_HEXAGON_16:
        case R_HEXAGON_8:
          /* These relocations cannot be used in shared libraries or for
             symbols in shared libraries. */

	case R_HEXAGON_32_PCREL:
	  /* This relocation cannot be used for symbols in shared libraries. */

          /* Do not complain about sections that we don't care about, such as
             debug or non-constant sections. */
          if (info->shared
              && (sec->flags & SEC_ALLOC)
              && (sec->flags & SEC_READONLY)
              && r_type != R_HEXAGON_32_PCREL)
            {
              (*_bfd_error_handler)
                (_("%B: relocation %s%s%s%s in section `%A\' cannot be handled " \
		    "in a shared object; recompile with `-fpic\'"),
		 abfd, sec, hexagon_elf_howto_table [r_type].name,
		 h? " for symbol `": "", h? h->elf.root.root.string: "",
		 h? "\'": "");
              bfd_set_error (bfd_error_bad_value);
              return FALSE;
            }
          else if ((ELIMINATE_COPY_RELOCS || info->nocopyreloc)
                    && info->executable
                    && (sec->flags & SEC_ALLOC)
                    && (sec->flags & SEC_READONLY)
                    && h
                    && (h->elf.root.type == bfd_link_hash_defined
                        || h->elf.root.type == bfd_link_hash_defweak)
                    && !h->elf.def_regular
                    && !h->elf.non_got_ref)
            {
              (*_bfd_error_handler)
                (_("%B: relocation %s for symbol `%s\' in section `%A\' " \
                   "cannot be handled when it is defined by a shared library"),
                abfd, sec,
                hexagon_elf_howto_table [r_type].name, h->elf.root.root.string);
              bfd_set_error (bfd_error_bad_value);
              return FALSE;
            }
          break;

	case R_HEXAGON_IE_GOT_LO16:
	case R_HEXAGON_IE_GOT_HI16:
	case R_HEXAGON_IE_GOT_32:
	case R_HEXAGON_IE_GOT_16:
	  /* These relocations require that a DSO is of type Initial Exec.
	     Like in other targets, this flag is not reset even if they are
	     garbage-collected. */
	  if (info->shared)
	    info->flags |= DF_STATIC_TLS;
	  break;
	}
    }

  return TRUE;
}

/**
Update the GOT entry reference counts for the section being removed.
*/

static bfd_boolean
hexagon_elf_gc_sweep_hook
(bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED,
 asection *sec, const Elf_Internal_Rela *relocs ATTRIBUTE_UNUSED)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_signed_vma *local_got_refcounts;
  const Elf_Internal_Rela *rel;

  elf_section_data (sec)->local_dynrel = NULL;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  local_got_refcounts = elf_local_got_refcounts (abfd);

  for (rel = relocs; rel < relocs + sec->reloc_count; rel++)
    {
      unsigned int r_type;
      size_t r_symndx, l_symndx;
      struct elf_link_hash_entry *h = NULL;
      hexagon_link_hash_entry *eh = NULL;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx >= symtab_hdr->sh_info)
	{
	  hexagon_dyn_reloc *p, **pp;

	  h = sym_hashes [r_symndx - symtab_hdr->sh_info];
	  eh = hexagon_hash_entry (h);

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL; pp = &p->next)
	    if (p->sec == sec)
	      {
		/* Everything must go for SEC.  */
		*pp = p->next;
		break;
	      }
	}

      r_type = ELF32_R_TYPE (rel->r_info);
      switch (r_type)
	{
	case R_HEXAGON_GOT_LO16:
	case R_HEXAGON_GOT_HI16:
	case R_HEXAGON_GOT_32:
	case R_HEXAGON_GOT_16:
	case R_HEXAGON_GD_GOT_LO16:
	case R_HEXAGON_GD_GOT_HI16:
	case R_HEXAGON_GD_GOT_32:
	case R_HEXAGON_GD_GOT_16:
	case R_HEXAGON_IE_GOT_LO16:
	case R_HEXAGON_IE_GOT_HI16:
	case R_HEXAGON_IE_GOT_32:
	case R_HEXAGON_IE_GOT_16:

	  switch (r_type)
	    {
	    case R_HEXAGON_GOT_LO16:
	    case R_HEXAGON_GOT_HI16:
	    case R_HEXAGON_GOT_32:
	    case R_HEXAGON_GOT_16:
	      l_symndx = LGOT_OK (symtab_hdr, r_symndx);
	      if (eh && eh->ok_got.refcount > 0)
		eh->ok_got.refcount--;
	      break;

	    case R_HEXAGON_GD_GOT_LO16:
	    case R_HEXAGON_GD_GOT_HI16:
	    case R_HEXAGON_GD_GOT_32:
	    case R_HEXAGON_GD_GOT_16:
	      l_symndx = LGOT_GD (symtab_hdr, r_symndx);
	      if (eh && eh->gd_got.refcount > 0)
		eh->gd_got.refcount--;
	      break;

	    case R_HEXAGON_IE_GOT_LO16:
	    case R_HEXAGON_IE_GOT_HI16:
	    case R_HEXAGON_IE_GOT_32:
	    case R_HEXAGON_IE_GOT_16:
	      l_symndx = LGOT_IE (symtab_hdr, r_symndx);
	      if (eh && eh->ie_got.refcount > 0)
		eh->ie_got.refcount--;
	      break;
	    }

	  if (h)
	    {
              /* Global symbol. */
	      if (h->got.refcount > 0)
		h->got.refcount--;
	    }
	  else if (local_got_refcounts)
	    {
	      if (local_got_refcounts [r_symndx] > 0)
		local_got_refcounts [r_symndx]--;
	      if (local_got_refcounts [l_symndx] > 0)
		local_got_refcounts [l_symndx]--;
	    }
	  break;

	case R_HEXAGON_LO16:
	case R_HEXAGON_HI16:
	case R_HEXAGON_HL16:
	case R_HEXAGON_32:
	case R_HEXAGON_16:
	case R_HEXAGON_8:

/*
	case R_HEXAGON_B22_PCREL:
	case R_HEXAGON_B15_PCREL:
	case R_HEXAGON_B13_PCREL:
	case R_HEXAGON_B7_PCREL:
*/
	  if (info->shared)
	    break;

	  /* Fall thru */

	case R_HEXAGON_PLT_B22_PCREL:
	case R_HEXAGON_B22_PCREL:
	case R_HEXAGON_B15_PCREL:
	case R_HEXAGON_B13_PCREL:
	case R_HEXAGON_B7_PCREL:
	  /* FIXME: should symbol visibility matter? */
	  if (h)
            /* Global symbol. */
            if (h->plt.refcount > 0)
              h->plt.refcount--;

	  break;

	default:
	  break;
	}
    }

  return TRUE;
}

/**
Finish up dynamic symbol handling.  We set the contents of various
dynamic sections.
*/
static bfd_boolean
hexagon_elf_finish_dynamic_symbol
(bfd *obfd, struct bfd_link_info *info, struct elf_link_hash_entry *h,
 Elf_Internal_Sym *sym)
{
  hexagon_link_hash_table *htab;
  hexagon_link_hash_entry *eh;
  bfd *dynobj;

  htab = hexagon_hash_table (info);
  eh = hexagon_hash_entry (h);
  dynobj = elf_hash_table (info)->dynobj;

  if (h->plt.offset != -(bfd_vma) 1)
    {
      /* This symbol has a PLT entry. */
      bfd_vma plt_index, got_offset;
      Elf_Internal_Rela rela;

      if (h->dynindx == -1
	  || !htab->elf.splt || !htab->elf.sgotplt || !htab->elf.srelplt)
	abort ();

      /* Get the index in the PLT which corresponds to this symbol.  This is
         the index of this symbol in all the symbols for which we are making
         PLT entries.  The first few entries in the PLT are reserved.  */
      plt_index = h->plt.offset / PLT_ENTRY_SIZE - PLT_RESERVED_ENTRIES;

      /* Get the offset into the GOT of the entry that corresponds to this
         PLT entry.  The first few GOT entries are reserved. */
      got_offset = (plt_index + GOT_RESERVED_ENTRIES) * GOT_ENTRY_SIZE;

      /* Initialize the PLT entry. */
      memcpy (htab->elf.splt->contents + h->plt.offset,
              hexagon_plt_entry, sizeof (hexagon_plt_entry));
      /* A couple of insns contain the difference between the address of
         its corresponding GOT entry and the address of the PLT entry. */
      hexagon_reloc_operand
        (hexagon_elf_howto_table + R_HEXAGON_HI16,
         (hexagon_insn *) (htab->elf.splt->contents + h->plt.offset
                         + 1 * sizeof (*hexagon_plt_entry)),
         htab->elf.sgotplt->output_section->vma + htab->elf.sgotplt->output_offset
         + got_offset
         - htab->elf.splt->output_section->vma - htab->elf.splt->output_offset
         - h->plt.offset,
         NULL);
      hexagon_reloc_operand
        (hexagon_elf_howto_table + R_HEXAGON_LO16,
         (hexagon_insn *) (htab->elf.splt->contents + h->plt.offset
                         + 2 * sizeof (*hexagon_plt_entry)),
         htab->elf.sgotplt->output_section->vma + htab->elf.sgotplt->output_offset
         + got_offset
         - htab->elf.splt->output_section->vma - htab->elf.splt->output_offset
         - h->plt.offset,
         NULL);
/*
      bfd_put_32 (obfd,
                  htab->elf.sgotplt->output_section->vma
                  + htab->elf.sgotplt->output_offset
                  + got_offset
                  - htab->elf.splt->output_section->vma
                  - htab->elf.splt->output_offset
                  - h->plt.offset,
                  htab->elf.splt->contents + h->plt.offset
                  + sizeof (hexagon_plt_entry) - sizeof (*hexagon_plt_entry));
*/

      /* Intialize the GOT entry corresponding to this PLT entry to initially
         point to the 0th PLT entry, which marshalls the dynamic linker
         to resolve this symbol lazily. */
      bfd_put_32 (obfd,
                  htab->elf.splt->output_section->vma
                  + htab->elf.splt->output_offset,
		  htab->elf.sgotplt->contents + got_offset);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = htab->elf.sgotplt->output_section->vma
                      + htab->elf.sgotplt->output_offset
                      + got_offset;
      rela.r_info = ELF32_R_INFO (h->dynindx, R_HEXAGON_JMP_SLOT);
      rela.r_addend = 0;
      bfd_elf32_swap_reloca_out
        (obfd, &rela, htab->elf.srelplt->contents
                      + plt_index * sizeof (Elf32_External_Rela));

      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  Leave the value alone.  This is a clue
	     for the dynamic linker, to make function pointer
	     comparisons work between an application and shared
	     library.  */
	  sym->st_shndx = SHN_UNDEF;
	  /* If the symbol is weak, we do need to clear the value.
	     Otherwise, the PLT entry would provide a definition for
	     the symbol even if the symbol wasn't defined anywhere,
	     and so the symbol would never be NULL.  */
          /* FIXME: Is this still necessary with the addition of the next test? */
	  if (!h->ref_regular_nonweak)
	    sym->st_value = 0;

	  if (!h->pointer_equality_needed)
	    sym->st_value = 0;
	}
    }

  /* We don't emit GOT relocations for symbols that aren't in the
     dynamic-symbols table for an ordinary program and are either defined
     by the program or undefined weak symbols. */
  if (h->got.offset != -(bfd_vma) 1)
    {
      /* This symbol has a GOT entry. */
      Elf_Internal_Rela rela;
      bfd_vma offset = h->got.offset & ~(bfd_vma) 1;

      /* This symbol has an entry in the GOT.  Set it up. */
      if (!htab->elf.sgot || !htab->elf.srelgot)
	abort ();

      rela.r_offset = htab->elf.sgot->output_section->vma
		      + htab->elf.sgot->output_offset
		      + offset;

      /* If this is a static link, or it is a -Bsymbolic link and the
	 symbol is defined locally or was forced to be local because
	 of a version file, we just want to emit a RELATIVE relocation.
	 The entry in the GOT is initialized in relocate_section. */
      if (info->shared
	  && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  BFD_ASSERT ((h->got.offset & 1));

	  if (eh->ok_got.refcount > 0)
	    {
	      rela.r_info = ELF32_R_INFO (0, R_HEXAGON_RELATIVE);
	      rela.r_addend = h->root.u.def.value
			      + h->root.u.def.section->output_section->vma
			      + h->root.u.def.section->output_offset;

	      bfd_elf32_swap_reloca_out
		(obfd, &rela, htab->elf.srelgot->contents
			      + htab->elf.srelgot->reloc_count++
			      * sizeof (Elf32_External_Rela));

	      bfd_put_32 (obfd, rela.r_addend, htab->elf.sgot->contents + offset);
	    }
	  else
	    {
	      bfd_vma adjust = 0;

	      if (eh->gd_got.refcount > 0)
		{
		  bfd_put_32 (obfd, 0, htab->elf.sgot->contents + offset);
		  bfd_put_32
		    (obfd, 0, htab->elf.sgot->contents + offset + GOT_ENTRY_SIZE);

		  adjust = GOT_ENTRY_SIZE * 2;
		}

	      if (eh->ie_got.refcount > 0)
		{
		  bfd_put_32
		    (obfd,
		    hexagon_elf_tpoff
		      (info, h->root.u.def.value
			      + h->root.u.def.section->output_section->vma
			      + h->root.u.def.section->output_offset),
		    htab->elf.sgot->contents + offset + adjust);
		}
	    }
	}
      else
	{
	  BFD_ASSERT (h->got.offset & 1);

	  if (eh->ok_got.refcount > 0)
	    {
	      rela.r_info = ELF32_R_INFO (h->dynindx, R_HEXAGON_GLOB_DAT);
	      rela.r_addend = 0;

	      bfd_elf32_swap_reloca_out
		(obfd, &rela, htab->elf.srelgot->contents
			      + htab->elf.srelgot->reloc_count++
			      * sizeof (Elf32_External_Rela));

	      if (h->def_regular)
		bfd_put_32
		  (obfd,
		   h->root.u.def.value
		   + h->root.u.def.section->output_section->vma
		   + h->root.u.def.section->output_offset,
		   htab->elf.sgot->contents + offset);
	    }
	  else
	    {
	      bfd_vma adjust = 0;

	      if (eh->gd_got.refcount > 0)
		{
		  rela.r_info = ELF32_R_INFO (h->dynindx, R_HEXAGON_DTPMOD_32);
		  rela.r_addend = 0;

		  bfd_elf32_swap_reloca_out
		    (obfd, &rela, htab->elf.srelgot->contents
				  + htab->elf.srelgot->reloc_count++
				  * sizeof (Elf32_External_Rela));

		  rela.r_offset += GOT_ENTRY_SIZE;
		  rela.r_info = ELF32_R_INFO (h->dynindx, R_HEXAGON_DTPREL_32);
		  rela.r_addend = 0;

		  bfd_elf32_swap_reloca_out
		    (obfd, &rela, htab->elf.srelgot->contents
				  + htab->elf.srelgot->reloc_count++
				  * sizeof (Elf32_External_Rela));

		  if (h->def_regular)
		    bfd_put_32
		      (obfd,
		      hexagon_elf_dtpoff
			(info, h->root.u.def.value
				+ h->root.u.def.section->output_section->vma
				+ h->root.u.def.section->output_offset),
			  htab->elf.sgot->contents + offset);

		  adjust = GOT_ENTRY_SIZE * 2;
		}

	      if (eh->ie_got.refcount > 0)
		{
		  rela.r_info = ELF32_R_INFO (h->dynindx, R_HEXAGON_DTPREL_32);
		  rela.r_addend = 0;

		  bfd_elf32_swap_reloca_out
		    (obfd, &rela, htab->elf.srelgot->contents
				  + htab->elf.srelgot->reloc_count++
				  * sizeof (Elf32_External_Rela));

		  if (h->def_regular)
		    bfd_put_32
		      (obfd,
		      hexagon_elf_tpoff
			(info, h->root.u.def.value
				+ h->root.u.def.section->output_section->vma
				+ h->root.u.def.section->output_offset),
			  htab->elf.sgot->contents + offset);
		}
	    }
	}
    }

  if (h->needs_copy)
    {
      /* This symbol needs a copy reloc. */
      Elf_Internal_Rela rela;
      asection *s;

      s = elf_gp_size (htab->elf.dynobj)
          && h->size <= elf_gp_size (htab->elf.dynobj)
          ? htab->sbss.r: htab->bss.r;

      if (h->dynindx == -1
	  || (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	  || !s)
	abort ();

      rela.r_offset = h->root.u.def.value
		      + h->root.u.def.section->output_section->vma
		      + h->root.u.def.section->output_offset;
      rela.r_info = ELF32_R_INFO (h->dynindx, R_HEXAGON_COPY);
      rela.r_addend = 0;
      bfd_elf32_swap_reloca_out
        (obfd, &rela, s->contents
                      + s->reloc_count++ * sizeof (Elf32_External_Rela));
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  */
  if (CONST_STRNEQ (h->root.root.string, "_DYNAMIC")
      || CONST_STRNEQ (h->root.root.string, "_GLOBAL_OFFSET_TABLE_"))
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

/**
Finish up the dynamic sections.
*/

static bfd_boolean
hexagon_elf_finish_dynamic_sections
(bfd *obfd, struct bfd_link_info *info)
{
  hexagon_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn, *sdynsym;

  htab = hexagon_hash_table (info);
  dynobj = htab->elf.dynobj;
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");
  sdynsym = bfd_get_section_by_name (obfd, ".dynsym");

  if (htab->elf.dynamic_sections_created)
    {
      Elf32_External_Dyn *dyncon;

      if (!sdyn || !htab->elf.sgot)
	abort ();

      for (dyncon = (Elf32_External_Dyn *) sdyn->contents;
           dyncon < (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
           dyncon++)
	{
	  Elf_Internal_Dyn dyn;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      continue;

	    case DT_PLTGOT:
	      dyn.d_un.d_ptr = htab->elf.sgotplt->output_section->vma
	                     + htab->elf.sgotplt->output_offset;
	      break;

	    case DT_JMPREL:
	      dyn.d_un.d_ptr = htab->elf.srelplt->output_section->vma
	                     + htab->elf.srelplt->output_offset;
	      break;

	    case DT_PLTRELSZ:
              dyn.d_un.d_val = htab->elf.srelplt->output_section->size;
	      break;

	    case DT_RELASZ:
	      /* The procedure linkage table relocs (DT_JMPREL) should
		 not be included in the overall relocs (DT_RELA).
		 Therefore, we override the DT_RELASZ entry here to
		 make it not include the JMPREL relocs.  Since the
		 linker script arranges for .rela.plt to follow all
		 other relocation sections, we don't have to worry
		 about changing the DT_RELA entry.  */
	      if (htab->elf.srelplt)
		{
                  dyn.d_un.d_val -= htab->elf.srelplt->output_section->size;
                  break;
		}
              else
                continue;

	    case DT_RELA:
	      /* We may not be using the standard ELF linker script.
		 If .rel.plt is the first .rel section, we adjust
		 DT_REL to not include it.  */
	      if (!htab->elf.srelplt)
		continue;
	      else if (dyn.d_un.d_ptr
	               != htab->elf.srelplt->output_section->vma
	                  + htab->elf.srelplt->output_offset)
		continue;
              else
                dyn.d_un.d_ptr += htab->elf.srelplt->size;
	      break;

	    case DT_HEXAGON_SYMSZ:
              if (sdynsym)
                {
                  dyn.d_un.d_val = get_section_size_now (obfd, sdynsym);
                  break;
                }
              else
                continue;

            case DT_HEXAGON_VER:
              dyn.d_un.d_val = 3;
	    }

	  bfd_elf32_swap_dyn_out (obfd, &dyn, dyncon);
	}

      /* Fill in the special first entry in the procedure linkage table.  */
      if (htab->elf.splt && htab->elf.splt->size > 0)
	{
          size_t i;

          /* Initialize the first few reserved entries of the PLT. */
          for (i = 0; i < PLT_INITIAL_ENTRY_SIZE; i += 4)
            bfd_put_32 (obfd,
                        (bfd_vma) 0 /* But it could be NOP (0x7f00c000). */,
                        htab->elf.splt->contents + i);
          /* PLT [0] contains the code that marshalls the dynamic linker to
             resolve symbols used by the other PLT entries. */
	  memcpy (htab->elf.splt->contents, hexagon_plt_initial_entry,
	          sizeof (hexagon_plt_initial_entry));
	  /* A couple of insns contain the difference between the address of the
	     PLT and of the GOT. */
          hexagon_reloc_operand
            (hexagon_elf_howto_table + R_HEXAGON_HI16,
            (hexagon_insn *) (htab->elf.splt->contents
                            + 1 * sizeof (*hexagon_plt_initial_entry)),
            htab->elf.splt->output_section->vma
            + htab->elf.splt->output_offset
            - htab->elf.sgotplt->output_section->vma
            - htab->elf.sgotplt->output_offset,
            NULL);
          hexagon_reloc_operand
            (hexagon_elf_howto_table + R_HEXAGON_LO16,
            (hexagon_insn *) (htab->elf.splt->contents
                            + 2 * sizeof (*hexagon_plt_initial_entry)),
            htab->elf.splt->output_section->vma
            + htab->elf.splt->output_offset
            - htab->elf.sgotplt->output_section->vma
            - htab->elf.sgotplt->output_offset,
            NULL);
/*
	  bfd_put_32 (obfd,
		      htab->elf.splt->output_section->vma
		      + htab->elf.splt->output_offset
		      - htab->elf.sgotplt->output_section->vma
		      - htab->elf.sgotplt->output_offset,
		      htab->elf.splt->contents
		      + sizeof (hexagon_plt_initial_entry)
		      - sizeof (*hexagon_plt_initial_entry));
*/

	  elf_section_data (htab->elf.splt->output_section)->this_hdr.sh_entsize
	    = PLT_ENTRY_SIZE;
	}
    }

  if (htab->elf.sgotplt)
    {
      /* Initialize the first few reserved entries of the GOT. */
      if (htab->elf.sgotplt->size > 0)
	{
	  /* Set GOT [0] to the address of _DYNAMIC.  */
          bfd_put_32 (obfd,
                      sdyn
                      ? sdyn->output_section->vma + sdyn->output_offset
                      : 0,
                      htab->elf.sgotplt->contents + GOT_ENTRY_SIZE * 0);
	  /* Set GOT [1] and GOT [2], needed for the dynamic linker.  */
	  bfd_put_32 (obfd, 0, htab->elf.sgotplt->contents + GOT_ENTRY_SIZE * 1);
	  bfd_put_32 (obfd, 0, htab->elf.sgotplt->contents + GOT_ENTRY_SIZE * 2);
	}

      elf_section_data (htab->elf.sgotplt->output_section)->this_hdr.sh_entsize
        = GOT_ENTRY_SIZE;
    }

  if (htab->elf.sgot && htab->elf.sgot->size > 0)
    elf_section_data (htab->elf.sgot->output_section)->this_hdr.sh_entsize
      = GOT_ENTRY_SIZE;

  return TRUE;
}

/**
Adjust a symbol defined by a dynamic object and referenced by a regular object.
The current definition is in some section of the dynamic object, but
we're not including those sections.  We have to change the definition to
something the rest of the link can understand.
*/

static bfd_boolean
hexagon_elf_adjust_dynamic_symbol
(struct bfd_link_info *info, struct elf_link_hash_entry *h)
{
  hexagon_link_hash_table *htab;
  asection *s;

  htab = hexagon_hash_table (info);

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (htab->elf.dynobj
              && (h->needs_plt
                  || h->type == STT_GNU_IFUNC
                  || h->u.weakdef
                  || (h->def_dynamic
                      && h->ref_regular
                      && !h->def_regular)));

  /* If this is a function, put it in the PLT.  We will fill in the contents
     of the PLT later, when we know the address of the GOT section.  */
  if (h->type == STT_FUNC
      || h->needs_plt)
    {
      /* FIXME: PPC qualifies this step differently. */
      if (h->plt.refcount <= 0
	  || (info->executable
	      && !h->def_dynamic
	      && !h->ref_dynamic
	      && h->root.type != bfd_link_hash_undefweak
	      && h->root.type != bfd_link_hash_undefined))
	{
	  /* This case can occur if we saw a PLT relocation in an input
	     object, but the symbol was never referred to by a dynamic
	     object, or if all references were garbage collected.  In
	     such a case, we don't actually need to build a PLT, and
	     we can just do a PC-RELATIVE relocation instead.  */
	  h->plt.offset = -(bfd_vma) 1;
	  h->needs_plt = FALSE;
	}

      return TRUE;
    }
  else
    /* It's possible that we incorrectly decided that a PLT relocation was
       needed for a PC-relative relocation to a non-function symbol in
       check_relocs.  We can't decide accurately between function and
       non-function symbols in check-relocs; objects loaded later in
       the link may change h->type.  So fix it now.  */
    h->plt.offset = -(bfd_vma) 1;

  /* If this is a weak symbol, and there is a real definition, the
     processor-independent code will have arranged for us to see the
     real definition first, and we can just use the same value.	 */
  if (h->u.weakdef)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
		  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;

      if (ELIMINATE_COPY_RELOCS || info->nocopyreloc)
        h->non_got_ref = h->u.weakdef->non_got_ref;

      return TRUE;
    }

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.	 */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.	*/
  if (info->shared)
    return TRUE;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return TRUE;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
    {
      h->non_got_ref = FALSE;
      return TRUE;
    }

  if (ELIMINATE_COPY_RELOCS)
    {
      hexagon_link_hash_entry * eh;
      hexagon_dyn_reloc *p;

      eh = hexagon_hash_entry (h);
      for (p = eh->dyn_relocs; p; p = p->next)
	{
	  s = p->sec->output_section;
	  if (s && (s->flags & SEC_READONLY))
	    break;
	}

      /* If we didn't find any dynamic relocations in read-only sections, then
	 we'll be keeping the dynamic relocations and avoiding the copy reloc.  */
      if (!p)
	{
          h->non_got_ref = FALSE;
	  return TRUE;
	}
    }

  /* We must allocate the symbol in our .dynbss section, which will become
     part of the .[s]bss section of the executable.  There will be an entry
     for this symbol in the .dynsym section.  The dynamic object will contain
     position-independent code, so all references from the dynamic object to
     this symbol will go through the GOT.  The dynamic linker will use the
     .dynsym entry to determine the address it must put in the GOT, so both
     the dynamic object and the regular object will refer to the same memory
     location for the variable.  */

  /* We must generate a R_HEXAGON_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC))
    {
      s = elf_gp_size (htab->elf.dynobj)
          && h->size <= elf_gp_size (htab->elf.dynobj)
          ? htab->sbss.r: htab->bss.r;
      BFD_ASSERT (s);

      s->size += sizeof (Elf32_External_Rela);
      h->needs_copy = 1;
    }

  /* If a symbol has no size and does not require a PLT entry, R_HEXAGON_COPY
     would probably do the wrong thing. */
  if (!h->size
      && !h->needs_plt)
    {
      (*_bfd_error_handler)
        (_("warning: size of dynamic symbol `%s' is zero"), h->root.root.string);
      return TRUE;
    }

  /* Choose the proper section.  */
  s = elf_gp_size (htab->elf.dynobj)
      && h->size <= elf_gp_size (htab->elf.dynobj)
      ? htab->sbss.s: htab->bss.s;
  return (_bfd_elf_adjust_dynamic_copy (h, s));
}

/**
Allocate space in PLT, GOT and associated relocation sections for dynamic
relocations.
*/

static bfd_boolean
hexagon_allocate_dynrel
(struct elf_link_hash_entry *h, struct bfd_link_info *info)
{
  hexagon_link_hash_table *htab;
  hexagon_link_hash_entry *eh;
  hexagon_dyn_reloc *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  htab = hexagon_hash_table (info);
  eh = hexagon_hash_entry (h);

  if (htab->elf.dynamic_sections_created
      && h->plt.refcount > 0)
    {
       /* Make sure that this symbol is output as a dynamic symbol.
          Undefined weak symbols won't be marked as dynamic just yet.  */
      if (h->dynindx == -1
          && !h->forced_local)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      if (info->shared
          || WILL_CALL_FINISH_DYNAMIC_SYMBOL (TRUE, info->shared, h))
        {
          /* If this is the first PLT entry, make room for the reserved entries. */
          if (!htab->elf.splt->size)
            htab->elf.splt->size = PLT_INITIAL_ENTRY_SIZE;

          h->plt.offset = htab->elf.splt->size;

          /* If this symbol is not defined in a regular file, then set the
             symbol to its PLT entry.  This is required to make function
             pointers compare as equal between the normal executable and
             the shared library.  */
          if (!h->def_regular)
	    {
	      h->root.u.def.section = htab->elf.splt;
	      h->root.u.def.value = h->plt.offset;
	    }

          /* Make room for this entry.  */
          htab->elf.splt->size += PLT_ENTRY_SIZE;

          /* We also need to make an entry in the .got.plt section, which
            will be placed in the GOT section by the linker script.  */
          htab->elf.sgotplt->size += GOT_ENTRY_SIZE;

          /* We also need to make an entry in the .rela.plt section.  */
          htab->elf.srelplt->size += sizeof (Elf32_External_Rela);
        }
      else
        {
          h->plt.offset = -(bfd_vma) 1;
          h->needs_plt = FALSE;
        }
    }
  else
    {
      h->plt.offset = -(bfd_vma) 1;
      h->needs_plt = FALSE;
    }

  if (h->got.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.  Undefined
         weak symbols won't be marked as dynamic just yet.  */
      if (h->dynindx == -1
          && !h->forced_local)
        {
          if (!bfd_elf_link_record_dynamic_symbol (info, h))
            return FALSE;
        }

      h->got.offset = htab->elf.sgot->size;
      htab->elf.sgot->size += GOT_ENTRY_SIZE
                              * (eh->gd_got.refcount > 0
				 ? (eh->ie_got.refcount > 0? 3: 2)
				 : 1);

      if (info->shared
	  || WILL_CALL_FINISH_DYNAMIC_SYMBOL
               (htab->elf.dynamic_sections_created, info->shared, h)
	  || eh->gd_got.refcount > 0 || eh->ie_got.refcount > 0)
	htab->elf.srelgot->size += sizeof (Elf32_External_Rela)
				   * (eh->gd_got.refcount > 0
				      ? (eh->ie_got.refcount > 0? 3: 2)
				      : 1);
    }
  else
    h->got.offset = -(bfd_vma) 1;

  if (!eh->dyn_relocs)
    return TRUE;

  if (info->shared)
    {
      /* In the shared -Bsymbolic case, discard space allocated for dynamic
         PC-relative relocations against symbols which turn out to be defined
         in regular objects.  For the shared case, discard space for PC-relative
         relocations that have become local due to symbol visibility changes.  */
      if (SYMBOL_CALLS_LOCAL (info, h))
        {
          hexagon_dyn_reloc **pp;

          for (pp = &eh->dyn_relocs; (p = *pp); )
            {
              if (p->count)
                p->count -= p->pc_count;
              p->pc_count = 0;
              if (!p->count)
                *pp = p->next;
              else
                pp = &p->next;
            }
        }

      /* Also discard relocs on undefined weak syms with non-default
    	 visibility.  */
      if (eh->dyn_relocs
	  && h->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	    eh->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1
		   && !h->forced_local)
	    {
	      if (!bfd_elf_link_record_dynamic_symbol (info, h))
		return FALSE;
	    }
	}
    }
  else if (ELIMINATE_COPY_RELOCS || info->nocopyreloc)
    {
      /* For the non-shared case, discard space for relocations against
         symbols which turn out to need copy-relocations or are not dynamic. */
      if (!h->non_got_ref
          && ((h->def_dynamic
               && !h->def_regular)
              || (htab->elf.dynamic_sections_created
                  && (h->root.type == bfd_link_hash_undefweak
                      || h->root.type == bfd_link_hash_undefined))))
        {
          /* Make sure this symbol is output as a dynamic symbol.  Undefined
             weak symbols won't be marked as dynamic just yet.  */
          if (h->dynindx == -1
              && !h->forced_local)
            {
              if (!bfd_elf_link_record_dynamic_symbol (info, h))
                return FALSE;
            }

          /* If that succeeded, we know we'll be keeping all the relocations. */
          if (h->dynindx == -1)
            eh->dyn_relocs = NULL;
        }
      else
        eh->dyn_relocs = NULL;
    }

  /* Finally, allocate space.  */
  for (p = eh->dyn_relocs; p; p = p->next)
    elf_section_data (p->sec)->sreloc->size
      += p->count * sizeof (Elf32_External_Rela);

  return TRUE;
}

/**
Find any dynamic relocations that apply to read-only sections and, if so,
mark the object as having relative relocations.
*/

static bfd_boolean
hexagon_readonly_dynrel
(struct elf_link_hash_entry *h, struct bfd_link_info *info)
{
  hexagon_dyn_reloc *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  for (p = hexagon_hash_entry (h)->dyn_relocs; p; p = p->next)
    {
      asection *s = p->sec->output_section;

      if (s && ((s->flags & (SEC_READONLY | SEC_ALLOC))
	        == (SEC_READONLY | SEC_ALLOC)))
	{
	  info->flags |= DF_TEXTREL;

	  /* Not an error, just cut short the traversal.  */
	  return FALSE;
	}
    }
  return TRUE;
}

/**
Set the sizes of the dynamic sections.
*/

static bfd_boolean
hexagon_elf_size_dynamic_sections
(bfd *obfd ATTRIBUTE_UNUSED, struct bfd_link_info *info)
{
  hexagon_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd_boolean relocs;
  bfd *ibfd;
  size_t i;

  htab = hexagon_hash_table (info);
  dynobj = htab->elf.dynobj;
  if (!dynobj)
    abort ();

  if (htab->elf.dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
	{
	  s = bfd_get_section_by_name (dynobj, ".interp");
	  if (!s)
	    abort ();
	  s->size = sizeof (ELF_DYNAMIC_INTERPRETER);
	  s->contents = (bfd_byte *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up GOT offsets for local symbols and space for local dynamic
     relocations.  */
  for (ibfd = info->input_bfds; ibfd; ibfd = ibfd->link_next)
    {
      bfd_signed_vma *local_got_refcounts;
      Elf_Internal_Shdr *symtab_hdr;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
	continue;

      for (s = ibfd->sections; s; s = s->next)
	{
	  hexagon_dyn_reloc *p;

	  for (p = ((hexagon_dyn_reloc *) elf_section_data (s)->local_dynrel);
	       p;
	       p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because it is
		     a copy of a link-once section or due to linker script,
		     so we'll discard the relocations too.  */
		}
	      else if (p->count)
		{
		  elf_section_data (p->sec)->sreloc->size
		    += p->count * sizeof (Elf32_External_Rela);
		  if ((p->sec->output_section->flags & SEC_READONLY))
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      for (local_got_refcounts = elf_local_got_refcounts (ibfd), i = 0;
           local_got_refcounts
           && i < symtab_hdr->sh_info;
           i++)
	{
	  if (local_got_refcounts [i] > 0)
	    {
	      local_got_refcounts [i] = htab->elf.sgot->size;
	      htab->elf.sgot->size
	        += GOT_ENTRY_SIZE
		   * (local_got_refcounts [LGOT_GD (symtab_hdr, i)] > 0? 2: 1);
	      if (info->shared)
		htab->elf.srelgot->size += sizeof (Elf32_External_Rela);
	    }
	  else
	    local_got_refcounts [i] = -(bfd_vma) 1;
	}
    }

  /* Allocate global symbol's PLT and GOT entries, as well as space for global
     symbols dynamic relocations.  */
  elf_link_hash_traverse (&htab->elf, hexagon_allocate_dynrel, (PTR) info);

  /* check_relocs and adjust_dynamic_symbol have determined the sizes of the
     various dynamic sections.  Allocate memory for them.  */
  relocs = FALSE;
  for (s = dynobj->sections; s; s = s->next)
    {
      bfd_boolean strip = TRUE;

      if (!(s->flags & SEC_LINKER_CREATED))
	continue;

      if (s == htab->elf.splt
          || s == htab->elf.sgot || s == htab->elf.sgotplt
          || s == htab->bss.s || s == htab->sbss.s)
	{
	  /* Strip this section if we don't need it; see comment below.  */
	  if (htab->elf.hplt)
	    strip = FALSE;
	}
      else if (CONST_STRNEQ (bfd_get_section_name (dynobj, s), ".rela"))
	{
	  if (s->size && s != htab->elf.srelplt)
	    relocs = TRUE;

	  /* We use the reloc_count field as a counter if we need to copy
	     relocations into the output file.  */
	  s->reloc_count = 0;
	}
      else
        /* It's not one of our sections, so don't allocate space.  */
        continue;

      if (!s->size)
	{
	  /* If we don't need this section, strip it from the output file.
	     This is mostly to handle .rela.[s]bss and .rela.plt.  We must
	     create both sections in create_dynamic_sections, because they
	     must be created before the linker maps input sections to output
	     sections.  The linker does that before adjust_dynamic_symbol is
	     called, and it is that function that decides whether anything
	     needs to go into these sections.  */
	  if (strip)
	    s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if (!(s->flags & SEC_HAS_CONTENTS))
	continue;

      /* Allocate memory for the section contents.  We use bfd_zalloc here
         in case unused entries are not reclaimed before the contents of the
         section are written out.  This should not happen, but if
         it does, we get a R_HEXAGON_NONE relocATION instead of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (!s->contents)
	return FALSE;
    }

  if (htab->elf.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the values
         later, in finish_dynamic_sections, but we must add the entries now
         so that we get the correct size for the .dynamic section.  The
         DT_DEBUG entry is filled in by the dynamic linker and used by the
         debugger.  */
      if (info->executable)
	{
          if (!_bfd_elf_add_dynamic_entry (info, DT_DEBUG, 0))
	    return FALSE;
	}

      if (htab->elf.splt->size)
	{
          if (!_bfd_elf_add_dynamic_entry (info, DT_PLTGOT, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_PLTRELSZ, 0)
	      || !_bfd_elf_add_dynamic_entry (info, DT_PLTREL, DT_RELA)
	      || !_bfd_elf_add_dynamic_entry (info, DT_JMPREL, 0))
	    return FALSE;
	}

      if (relocs)
	{
          if (!_bfd_elf_add_dynamic_entry (info, DT_RELA, 0)
               || !_bfd_elf_add_dynamic_entry (info, DT_RELASZ, 0)
	       || !_bfd_elf_add_dynamic_entry
                     (info, DT_RELAENT, sizeof (Elf32_External_Rela)))
	    return FALSE;

	  /* If any dynamic relocations apply to a read-only section, then we
	     need a DT_TEXTREL entry.  */
	  if (!(info->flags & DF_TEXTREL))
	    elf_link_hash_traverse (&htab->elf, hexagon_readonly_dynrel, info);

	  if ((info->flags & DF_TEXTREL))
	    {
              if (!_bfd_elf_add_dynamic_entry (info, DT_TEXTREL, 0))
		return FALSE;
	    }
	}

      /* FIXME: this dynamic tag is redundant, since it can be infered by
         multiplying DT_SYMENT by nchain in DT_HASH.  If the CStomer is
         adamant about it, then add it. */
      if (FALSE)
        {
          if (!_bfd_elf_add_dynamic_entry (info, DT_HEXAGON_SYMSZ, 0))
            return FALSE;
        }

      if (!_bfd_elf_add_dynamic_entry (info, DT_HEXAGON_VER, 0))
        return FALSE;
    }

  return TRUE;
}

/**
Used to decide how to sort relocs in an optimal manner for the dynamic linker
before writing them out.
*/

static enum elf_reloc_type_class
hexagon_elf_reloc_type_class
(const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_HEXAGON_RELATIVE:
      return reloc_class_relative;
    case R_HEXAGON_JMP_SLOT:
      return reloc_class_plt;
    case R_HEXAGON_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/**
Create .plt, .rela.plt, .got, .got.plt, .rela.got, .dynbss, .dynsbss, .rela.bss
and .rela.sbss sections in the dynamic object and set up shortcuts to them
in our hash table.
*/

static bfd_boolean
hexagon_elf_create_dynamic_sections
(bfd *abfd, struct bfd_link_info *info)
{
  hexagon_link_hash_table *htab;
  const flagword flags = get_elf_backend_data (abfd)->dynamic_sec_flags;

  htab = hexagon_hash_table (info);

  if (!_bfd_elf_create_dynamic_sections (abfd, info))
    return FALSE;

  if (!bfd_set_section_alignment (abfd, htab->elf.splt, 4))
    return FALSE;

  if (get_elf_backend_data (abfd)->want_plt_sym)
    /* Enable pretty disassembly of the PLT. */
    htab->elf.hplt->type = STT_FUNC;

  htab->bss.s = bfd_get_section_by_name (abfd, ".dynbss");
  if (info->executable)
    {
      htab->bss.r = bfd_get_section_by_name (abfd, ".rela.bss");
    }

  if (elf_gp_size (htab->elf.dynobj))
    {
      htab->sbss.s = bfd_make_section_with_flags
                      (abfd, ".dynsbss", SEC_ALLOC | SEC_LINKER_CREATED);
      if (info->executable)
        {
          htab->sbss.r = bfd_make_section_with_flags
                          (abfd, ".rela.sbss", flags | SEC_READONLY);
          if (!bfd_set_section_alignment (abfd, htab->sbss.r, 2))
            return FALSE;
        }

      if (!htab->sbss.s
          || (info->executable && !htab->sbss.r))
        abort ();
    }

  return TRUE;
}

/**
Return address for nth PLT entry in the PLT, for a given relocation,
or (bfd_vma) -1 if it should not be included.
*/

static bfd_vma
hexagon_elf_plt_sym_val
(bfd_vma i, const asection *plt, const arelent *rel ATTRIBUTE_UNUSED)
{
  return (plt->vma + (i + PLT_RESERVED_ENTRIES) * PLT_ENTRY_SIZE);
}

/**
Return TRUE if symbol should be hashed in the `.gnu.hash' section.
*/

static bfd_boolean
hexagon_elf_hash_symbol
(struct elf_link_hash_entry *h)
{
  if (h->plt.offset != (bfd_vma) -1
      && !h->def_regular
      && !h->pointer_equality_needed)
    return FALSE;

  return (_bfd_elf_hash_symbol (h));
}



/* BFD configuration. */

#define TARGET_LITTLE_SYM			bfd_elf32_littlehexagon_vec
#define TARGET_LITTLE_NAME			"elf32-littlehexagon"
#define TARGET_BIG_SYM				bfd_elf32_bighexagon_vec
#define TARGET_BIG_NAME				"elf32-bighexagon"
#define ELF_ARCH				bfd_arch_hexagon
#define ELF_MACHINE_CODE			EM_HEXAGON
#define ELF_MINPAGESIZE				0x1000
#define ELF_MAXPAGESIZE				0x1000
#define ELF_COMMONPAGESIZE			0x1000

#define elf_backend_may_use_rel_p		0
#define	elf_backend_may_use_rela_p		1
#define	elf_backend_default_use_rela_p		1
#define elf_backend_rela_normal			1
#define elf_backend_can_gc_sections		1
#define elf_backend_can_refcount		1
#define elf_backend_got_header_size		GOT_INITIAL_ENTRY_SIZE
#define elf_backend_plt_header_size		PLT_INITIAL_ENTRY_SIZE
#define elf_backend_plt_readonly		1
#define elf_backend_want_got_plt		1
#define elf_backend_want_plt_sym		1

#define elf_info_to_howto			0
#define elf_info_to_howto_rel			hexagon_info_to_howto_rel
#define bfd_elf32_bfd_reloc_type_lookup		hexagon_elf_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup		hexagon_elf_reloc_name_lookup

#define elf_backend_object_p			hexagon_elf_object_p
#define elf_backend_gc_mark_hook		hexagon_elf_gc_mark_hook
#define elf_backend_gc_sweep_hook		hexagon_elf_gc_sweep_hook
#define elf_backend_final_write_processing	hexagon_elf_final_write_processing
#define elf_backend_section_from_shdr		hexagon_elf_section_from_shdr
#define elf_backend_add_symbol_hook		hexagon_elf_add_symbol_hook
#define elf_backend_fake_sections		hexagon_elf_fake_sections
#define elf_backend_symbol_processing		hexagon_elf_symbol_processing
#define elf_backend_section_processing		hexagon_elf_section_processing
#define elf_backend_section_from_bfd_section	hexagon_elf_section_from_bfd_section
#define elf_backend_link_output_symbol_hook	hexagon_elf_link_output_symbol_hook
#define elf_backend_copy_indirect_symbol	hexagon_elf_copy_indirect_symbol
#define elf_backend_additional_program_headers	hexagon_elf_additional_program_headers
#define elf_backend_ignore_discarded_relocs	hexagon_elf_ignore_discarded_relocs
#define elf_backend_reloc_type_class		hexagon_elf_reloc_type_class
#define elf_backend_common_definition		hexagon_elf_common_definition
#define elf_backend_plt_sym_val		        hexagon_elf_plt_sym_val
#define elf_backend_hash_symbol		        hexagon_elf_hash_symbol

#define elf_backend_check_relocs		hexagon_elf_check_relocs
#define elf_backend_adjust_dynamic_symbol	hexagon_elf_adjust_dynamic_symbol
#define elf_backend_size_dynamic_sections	hexagon_elf_size_dynamic_sections
#define bfd_elf32_bfd_relax_section		hexagon_elf_relax_section
#define elf_backend_relocate_section		hexagon_elf_relocate_section
#define elf_backend_finish_dynamic_symbol	hexagon_elf_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections	hexagon_elf_finish_dynamic_sections
#define elf_backend_create_dynamic_sections	hexagon_elf_create_dynamic_sections

#define bfd_elf32_bfd_get_relocated_section_contents \
                                                hexagon_elf_get_relocated_section_contents
#define bfd_elf32_bfd_link_hash_table_create	hexagon_elf_link_hash_table_create
#define bfd_elf32_bfd_reloc_type_lookup		hexagon_elf_reloc_type_lookup

/* This is a bit of a hack to install the wrapper for _bfd_elf_set_arch_mach. */
#undef BFD_JUMP_TABLE_WRITE
#define BFD_JUMP_TABLE_WRITE(NAME) \
  hexagon_elf_set_arch_mach, \
  bfd_elf32_set_section_contents

#include "elf32-target.h"
