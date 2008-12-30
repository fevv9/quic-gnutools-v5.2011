/* qdsp6 support for 32-bit ELF
   Copyright 2008 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf/qdsp6.h"
#include "opcode/qdsp6.h"
#include "libiberty.h"
#include "elf32-qdsp6.h"


static reloc_howto_type elf_qdsp6_howto_table [] =
{
  /* This reloc does nothing.  */
  HOWTO (R_QDSP6_NONE,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 32,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 bfd_elf_generic_reloc,	/* special_function  */
	 "R_QDSP6_NONE",		/* name  */
	 TRUE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0,			/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 22 bit branch. */
  HOWTO (R_QDSP6_B22_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 22,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B22_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x01ff3ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 15 bit branch. */
  HOWTO (R_QDSP6_B15_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 15,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B15_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00df20fe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* v2: A relative 7 bit branch. */
  HOWTO (R_QDSP6_B7_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 7,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B7_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
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
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High 16 bits of a 32 bit number. */
  HOWTO (R_QDSP6_HI16,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_HI16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
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
	 0,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for halves. */
  HOWTO (R_QDSP6_GPREL16_1,	/* type  */
	 1,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_GPREL16_1",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for words. */
  HOWTO (R_QDSP6_GPREL16_2,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_GPREL16_2",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* An GP-relative offset for doublewords. */
  HOWTO (R_QDSP6_GPREL16_3,	/* type  */
	 3,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_bitfield, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_GPREL16_3",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x061f2ff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* High and low 16 bits of a 32 bit number applied to 2 insns back-to-back. */
  HOWTO (R_QDSP6_HL16,		/* type  */
	 0,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 16,			/* bitsize  */
	 FALSE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_dont, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_HL16",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00c03fff,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

  /* A relative 13 bit branch. */
  HOWTO (R_QDSP6_B13_PCREL,	/* type  */
	 2,			/* rightshift  */
	 2,			/* size (0 = byte, 1 = short, 2 = long)  */
	 13,			/* bitsize  */
	 TRUE,			/* pc_relative  */
	 0,			/* bitpos  */
	 complain_overflow_signed, /* complain_on_overflow  */
	 qdsp6_elf_reloc,	/* special_function  */
	 "R_QDSP6_B13_PCREL",	/* name  */
	 FALSE,			/* partial_inplace  */
	 0,			/* src_mask  */
	 0x00202ffe,		/* dst_mask  */
	 FALSE),		/* pcrel_offset  */

};

/* QDSP6 has two small data sectons, .sdata and .sbss,
   analogous to the regular .data and .bss, respectively,
   but they can be accessed using the GP register. */
static asection qdsp6_sdata_section;
static asymbol  qdsp6_sdata_symbol;
static asection qdsp6_sbss_section;
static asymbol  qdsp6_sbss_symbol;

/* QDSP6 ELF uses two common sections.  One is the usual one, and the
   other is for small objects.  All the small objects are kept
   together, and then referenced via the GP-register, which yields
   faster assembler code.  This is what we use for the small common
   section.  This approach is copied from ecoff.c.  */
static asection    qdsp6_scom_section [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON + 1];
static asymbol     qdsp6_scom_symbol  [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON + 1];
static const char *qdsp6_scom_name    [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON + 1] =
  {".scommon", ".scommon.1", ".scommon.2", ".scommon.4", ".scommon.8"};
static const char *qdsp6_sdata_name   [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON + 1] =
  {".sdata", ".sdata.1", ".sdata.2", ".sdata.4", ".sdata.8"};
static const char *qdsp6_sbss_name    [SHN_QDSP6_SCOMMON_8 - SHN_QDSP6_SCOMMON + 1] =
  {".sbss", ".sbss.1", ".sbss.2", ".sbss.4", ".sbss.8"};

/* Map BFD reloc types to QDSP6 ELF reloc types.  */

struct qdsp6_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
};

static const struct qdsp6_reloc_map qdsp6_reloc_map [] =
 {
   { BFD_RELOC_NONE,                  R_QDSP6_NONE },
   { BFD_RELOC_QDSP6_B22_PCREL,       R_QDSP6_B22_PCREL },
   { BFD_RELOC_QDSP6_B15_PCREL,       R_QDSP6_B15_PCREL },
   { BFD_RELOC_QDSP6_B13_PCREL,       R_QDSP6_B13_PCREL },
   { BFD_RELOC_QDSP6_B7_PCREL,        R_QDSP6_B7_PCREL },
   { BFD_RELOC_QDSP6_LO16,            R_QDSP6_LO16 },
   { BFD_RELOC_QDSP6_HI16,            R_QDSP6_HI16 },
   { BFD_RELOC_QDSP6_HL16,            R_QDSP6_HL16 },
   { BFD_RELOC_32,                    R_QDSP6_32 },
   { BFD_RELOC_16,                    R_QDSP6_16 },
   { BFD_RELOC_8,                     R_QDSP6_8 },
   { BFD_RELOC_QDSP6_GPREL16_0,       R_QDSP6_GPREL16_0 },
   { BFD_RELOC_QDSP6_GPREL16_1,       R_QDSP6_GPREL16_1 },
   { BFD_RELOC_QDSP6_GPREL16_2,       R_QDSP6_GPREL16_2 },
   { BFD_RELOC_QDSP6_GPREL16_3,       R_QDSP6_GPREL16_3 },
};

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup
(bfd *abfd ATTRIBUTE_UNUSED, bfd_reloc_code_real_type code)
{
  unsigned int i;

  for (i = ARRAY_SIZE (qdsp6_reloc_map); i--;)
    if (qdsp6_reloc_map[i].bfd_reloc_val == code)
      return elf_qdsp6_howto_table + qdsp6_reloc_map[i].elf_reloc_val;

  return NULL;
}


static bfd_reloc_code_real_type qdsp6_elf_reloc_val_lookup
(unsigned char elf_reloc_val)
{
  unsigned int i;

  for (i = ARRAY_SIZE (qdsp6_reloc_map); i--;)
    if (qdsp6_reloc_map[i].elf_reloc_val == elf_reloc_val)
      return qdsp6_reloc_map[i].bfd_reloc_val;

  return BFD_RELOC_NONE;
}

/* --- SM 12/19/08 */
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

      switch (arch)
	{
          case E_QDSP6_MACH_V2:
            mach = bfd_mach_qdsp6_v2;
            break;

          case E_QDSP6_MACH_V3:
            mach = bfd_mach_qdsp6_v3;
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
        val = E_QDSP6_MACH_V2;
        break;

      case bfd_mach_qdsp6_v3:
        val = E_QDSP6_MACH_V3;
        break;

      default:
        fprintf (stderr, "Unsupported machine architecure.\n");
        abort ();
    }

  elf_elfheader (abfd)->e_flags &=~ EF_QDSP6_MACH;
  elf_elfheader (abfd)->e_flags |= val;
}

static struct bfd_link_hash_entry *_sda_base = (struct bfd_link_hash_entry *)0;

static bfd_byte *
qdsp6_elf_get_relocated_section_contents
(bfd *output_bfd, struct bfd_link_info *link_info,
 struct bfd_link_order *link_order, bfd_byte *data, bfd_boolean relocateable,
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
      (output_bfd, link_info, link_order, data, relocateable, symbols));
}

/* We have to figure out the SDA_BASE value, so that we can adjust the
   symbol value correctly.  We look up the symbol _SDA_BASE_ in the output
   BFD.  If we can't find it, we're stuck.  We cache it in the ELF
   target data.  We don't need to adjust the symbol value for an
   external symbol if we are producing relocateable output.  */

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

/* --- SM 12/19/08 */


static int
qdsp6_reloc_operand
(reloc_howto_type *howto, qdsp6_insn *insn, bfd_vma offset, char **errmsg)
{
  bfd_reloc_code_real_type type = qdsp6_elf_reloc_val_lookup (howto->type);
  const qdsp6_opcode *opcode = qdsp6_lookup_insn (*insn);
  const qdsp6_operand *operand = qdsp6_lookup_reloc (type);
  int value;

  value = offset;
  if ((opcode) && (operand))
  {
    if (!qdsp6_encode_operand (operand, insn,
			       opcode->enc, value, errmsg))
    {
      /*
      if (errmsg && *errmsg)
        {
	  fprintf (stderr, "Error when encoding operand of \"%s\"\n",
	                   opcode->syntax);
	}
      */
      return 0;
    }
  }
  else
  {
    return 0;
  }

  return 1;
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

  // RK: Handle PC relative relocatable output
  if (output_bfd != (bfd *) NULL
      && reloc_entry->howto->pc_relative
      && (   !reloc_entry->howto->partial_inplace
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
  //printf("%d: bfd_is_com_section: %s relocation: 0x%x\n",__LINE__,(bfd_is_com_section(symbol->section)) ? "YES":"NO",relocation);

  reloc_target_output_section = symbol->section->output_section;

  /* Convert input-section-relative symbol value to absolute.  */
  if ((output_bfd && ! howto->partial_inplace)
      || reloc_target_output_section == NULL)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;
  //printf("%d: relocation: 0x%x output_base: 0x%x\n",__LINE__,relocation,output_base);

  relocation += output_base + symbol->section->output_offset;
  //printf("%d: relocation: 0x%x symbol->section->output_offset: 0x%x\n",__LINE__,relocation,symbol->section->output_offset);

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;
  //printf("%d: relocation: 0x%x Reloc address: 0x%x\n",__LINE__,relocation,reloc_entry->address);

  /* Here the variable relocation holds the final address of the
     symbol we are relocating against, plus any addend.  */


  if ((howto->type == R_QDSP6_GPREL16_0) ||
      (howto->type == R_QDSP6_GPREL16_1) ||
      (howto->type == R_QDSP6_GPREL16_2) ||
      (howto->type == R_QDSP6_GPREL16_3))
    {
      bfd_vma sda_base;

      if ((strcmp(symbol->section->name, ".sdata") != 0) &&
           (strcmp(symbol->section->name, ".sbss") != 0))
	{
	  //fprintf(stderr,"GP relative reloc for non-GP section (%s) var `%s'\n",symbol->section->name,symbol->name);
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

	 If we are producing relocateable output, then we must ensure
	 that this reloc will be correctly computed when the final
	 relocation is done.  If pcrel_offset is FALSE we want to wind
	 up with the negative of the location within the section,
	 which means we must adjust the existing addend by the change
	 in the location within the section.  If pcrel_offset is TRUE
	 we do not want to adjust the existing addend at all.

	 FIXME: This seems logical to me, but for the case of
	 producing relocateable output it is not what the code
	 actually does.  I don't want to change it, because it seems
	 far too likely that something will break.  */

      relocation -=
	input_section->output_section->vma + input_section->output_offset;

      if (howto->pcrel_offset)
	relocation -= reloc_entry->address;
    }

  if (output_bfd != (bfd *) NULL)
    {
      if (! howto->partial_inplace)
	{
	  /* This is a partial relocation, and we want to apply the relocation
	     to the reloc entry rather than the raw data. Modify the reloc
	     inplace to reflect what we now know.  */
	  reloc_entry->addend = relocation;
	  reloc_entry->address += input_section->output_offset;
	  //printf("%d: relocation: 0x%x Reloc address: 0x%x\n",__LINE__,relocation,reloc_entry->address);
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

	  /* WTF?? */
	  if (abfd->xvec->flavour == bfd_target_coff_flavour
	      && strcmp (abfd->xvec->name, "coff-Intel-little") != 0
	      && strcmp (abfd->xvec->name, "coff-Intel-big") != 0)
	    {
#if 1
	      /* For m68k-coff, the addend was being subtracted twice during
		 relocation with -r.  Removing the line below this comment
		 fixes that problem; see PR 2953.

However, Ian wrote the following, regarding removing the line below,
which explains why it is still enabled:  --djm

If you put a patch like that into BFD you need to check all the COFF
linkers.  I am fairly certain that patch will break coff-i386 (e.g.,
SCO); see coff_i386_reloc in coff-i386.c where I worked around the
problem in a different way.  There may very well be a reason that the
code works as it does.

Hmmm.  The first obvious point is that bfd_perform_relocation should
not have any tests that depend upon the flavour.  It's seem like
entirely the wrong place for such a thing.  The second obvious point
is that the current code ignores the reloc addend when producing
relocateable output for COFF.  That's peculiar.  In fact, I really
have no idea what the point of the line you want to remove is.

A typical COFF reloc subtracts the old value of the symbol and adds in
the new value to the location in the object file (if it's a pc
relative reloc it adds the difference between the symbol value and the
location).  When relocating we need to preserve that property.

BFD handles this by setting the addend to the negative of the old
value of the symbol.  Unfortunately it handles common symbols in a
non-standard way (it doesn't subtract the old value) but that's a
different story (we can't change it without losing backward
compatibility with old object files) (coff-i386 does subtract the old
value, to be compatible with existing coff-i386 targets, like SCO).

So everything works fine when not producing relocateable output.  When
we are producing relocateable output, logically we should do exactly
what we do when not producing relocateable output.  Therefore, your
patch is correct.  In fact, it should probably always just set
reloc_entry->addend to 0 for all cases, since it is, in fact, going to
add the value into the object file.  This won't hurt the COFF code,
which doesn't use the addend; I'm not sure what it will do to other
formats (the thing to check for would be whether any formats both use
the addend and set partial_inplace).

When I wanted to make coff-i386 produce relocateable output, I ran
into the problem that you are running into: I wanted to remove that
line.  Rather than risk it, I made the coff-i386 relocs use a special
function; it's coff_i386_reloc in coff-i386.c.  The function
specifically adds the addend field into the object file, knowing that
bfd_perform_relocation is not going to.  If you remove that line, then
coff-i386.c will wind up adding the addend field in twice.  It's
trivial to fix; it just needs to be done.

The problem with removing the line is just that it may break some
working code.  With BFD it's hard to be sure of anything.  The right
way to deal with this is simply to build and test at least all the
supported COFF targets.  It should be straightforward if time and disk
space consuming.  For each target:
    1) build the linker
    2) generate some executable, and link it using -r (I would
       probably use paranoia.o and link against newlib/libc.a, which
       for all the supported targets would be available in
       /usr/cygnus/progressive/H-host/target/lib/libc.a).
    3) make the change to reloc.c
    4) rebuild the linker
    5) repeat step 2
    6) if the resulting object files are the same, you have at least
       made it no worse
    7) if they are different you have to figure out which version is
       right
*/
	      relocation -= reloc_entry->addend;
#endif
	      reloc_entry->addend = 0;
	      //printf("%d: relocation: 0x%x Reloc address: 0x%x\n",__LINE__,relocation,reloc_entry->address);
	    }
	  else
	    {
	      reloc_entry->addend = relocation;
	    }
	}
    }
  else
    {
      reloc_entry->addend = 0;
      //printf("%d: relocation: 0x%x Reloc address: 0x%x addend=0\n",__LINE__,relocation,reloc_entry->address);
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
//        printf("16-bit instruction: 0x%x\n", insn);

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
//        printf("32-bit instruction: 0x%x\n", insn);

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
//	  insn = bfd_get_32 (abfd, (bfd_byte *) data + octets);
//        printf("  after relocation: 0x%x\n", insn);
        }
      break;

      default:
      return bfd_reloc_other;
    }

  return bfd_reloc_ok;
}

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
qdsp6_elf_add_symbol_hook
(bfd *abfd, struct bfd_link_info *info, const Elf_Internal_Sym *sym,
 const char **namep ATTRIBUTE_UNUSED, flagword *flagsp ATTRIBUTE_UNUSED,
 asection **secp, bfd_vma *valp)
{
  if (   !info->relocatable
      && bfd_get_flavour (abfd) == bfd_target_elf_flavour)
    {
    }

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

  return TRUE;
}

/* Handle the special QDSP6 section numbers that a symbol may use. */
static void
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
                                         | (  elfsym->internal_elf_sym.st_shndx
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

/* Work over a section just before writing it out. FIXME: We recognize
   sections that need the SHF_QDSP6_GPREL flag by name; there has to be
   a better way.  */
static bfd_boolean
qdsp6_elf_section_processing
(bfd *abfd ATTRIBUTE_UNUSED, Elf_Internal_Shdr *hdr)
{
  if (hdr->bfd_section != NULL)
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
static bfd_boolean
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
static bfd_boolean
qdsp6_elf_link_output_symbol_hook
(bfd *abfd ATTRIBUTE_UNUSED, struct bfd_link_info *info ATTRIBUTE_UNUSED,
 const char *name ATTRIBUTE_UNUSED, Elf_Internal_Sym *sym, asection *input_sec)
{
  /* If we see a common symbol, which implies a relocatable link, then
     if a symbol was small common in an input file, mark it as small
     common in the output file.  */
  if (   sym->st_shndx == SHN_COMMON
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

static asection *
qdsp6_elf_gc_mark_hook ( asection *sec,
                         struct bfd_link_info *info ATTRIBUTE_UNUSED,
                         Elf_Internal_Rela *rel ATTRIBUTE_UNUSED,
                         struct elf_link_hash_entry *h,
                         Elf_Internal_Sym *sym)
{
  if (h != NULL)
    {
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
    return bfd_section_from_elf_index (sec->owner, sym->st_shndx);

  return NULL;
}

/* XXX_SM try to remove */
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

/* Look through the relocs for a section during the first phase.
   We only need to consider the virtual table relocs for garbage collection.*/
static bfd_boolean
qdsp6_elf_check_relocs (bfd *abfd,
                        struct bfd_link_info *info,
                        asection *sec,
                        const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes, **sym_hashes_end;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;

  if (info->relocatable)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  sym_hashes_end =
    sym_hashes + symtab_hdr->sh_size / sizeof (Elf32_External_Sym);
  if (!elf_bad_symtab (abfd))
    sym_hashes_end -= symtab_hdr->sh_info;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
	h = NULL;
      else
	h = sym_hashes[r_symndx - symtab_hdr->sh_info];
    }

  return TRUE;
}

static bfd_boolean
qdsp6_elf_relocate_section ( bfd *output_bfd,
                             struct bfd_link_info *info,
                             bfd *input_bfd,
                             asection *input_section,
                             bfd_byte *contents,
                             Elf_Internal_Rela *relocs,
                             Elf_Internal_Sym *local_syms,
                             asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend = relocs + input_section->reloc_count;

  for (rel = relocs; rel < relend; rel++)
    {
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      const char *name = NULL;
      int r_type;

      /* This is a final link.  */
      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);
      howto = elf_qdsp6_howto_table + ELF32_R_TYPE (rel->r_info);

      if (info->relocatable)
	{
	  /* This is a relocateable link.  We don't have to change
	     anything, unless the reloc is against a section symbol,
	     in which case we have to adjust according to where the
	     section symbol winds up in the output section.  */
	  if (r_symndx < symtab_hdr->sh_info)
	    {
	      sym = local_syms + r_symndx;
	      if (ELF_ST_TYPE (sym->st_info) == STT_SECTION)
		{
		  sec = local_sections[r_symndx];
		  rel->r_addend += sec->output_offset + sym->st_value;
		}
	    }

	  continue;
	}

      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sec = local_sections[r_symndx];
	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = (name == NULL) ? bfd_section_name (input_bfd, sec) : name;
	}
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];

	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  name = h->root.root.string;

	  if (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	    {
	      sec = h->root.u.def.section;
	      relocation = (h->root.u.def.value
			    + sec->output_section->vma + sec->output_offset);
	    }
	  else if (h->root.type == bfd_link_hash_undefweak)
	    {
	      relocation = 0;
	    }
	  else
	    {
	      if (!((*info->callbacks->undefined_symbol)
		    (info, h->root.root.string, input_bfd,
		     input_section, rel->r_offset, TRUE)))
		return FALSE;
	      relocation = 0;
	    }
	}

      switch (r_type)
	{
	  qdsp6_insn insn;
	  bfd_vma ioffset;

	case R_QDSP6_LO16:
	case R_QDSP6_HI16:
	  ioffset = relocation + rel->r_addend;

	  insn = qdsp6_get_insn (input_bfd, howto, contents + rel->r_offset);

	  if (!qdsp6_reloc_operand (howto, &insn, ioffset, NULL))
	    {
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root : NULL), name, howto->name,
		(bfd_vma) 0, input_bfd, input_section, rel->r_offset);

	      //printf("%s Reloc overflow,offset=0x%x\n",howto->name,ioffset);
	    }
	  else
	      qdsp6_put_insn (input_bfd, howto, contents + rel->r_offset, insn);

          continue;
	  break;

        case R_QDSP6_HL16:
	  ioffset = relocation + rel->r_addend;

          /* First instruction (HI). */
	  insn = qdsp6_get_insn (input_bfd, elf_qdsp6_howto_table + R_QDSP6_HI16,
                                 contents + rel->r_offset + 0);

	  if (!qdsp6_reloc_operand (elf_qdsp6_howto_table + R_QDSP6_HI16, &insn,
                                    ioffset, NULL))
	    {
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root : NULL), name,
                 elf_qdsp6_howto_table [R_QDSP6_HI16].name, 0,
		 input_bfd, input_section, rel->r_offset + 0);

	      //printf("%s Reloc overflow,offset=0x%x\n",howto->name,ioffset);
	    }
	  else
	      qdsp6_put_insn (input_bfd, elf_qdsp6_howto_table + R_QDSP6_HI16,
                              contents + rel->r_offset + 0, insn);

          /* Second instruction (LO). */
	  insn = qdsp6_get_insn (input_bfd, elf_qdsp6_howto_table + R_QDSP6_LO16,
                                 contents + rel->r_offset + sizeof (insn));

	  if (!qdsp6_reloc_operand (elf_qdsp6_howto_table + R_QDSP6_LO16, &insn,
                                    ioffset, NULL))
	    {
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root : NULL), name,
                 elf_qdsp6_howto_table [R_QDSP6_LO16].name, 0,
		 input_bfd, input_section, rel->r_offset + sizeof (insn));

	      //printf("%s Reloc overflow,offset=0x%x\n",howto->name,ioffset);
	    }
	  else
	      qdsp6_put_insn (input_bfd, elf_qdsp6_howto_table + R_QDSP6_LO16,
                              contents + rel->r_offset + sizeof (insn), insn);

          continue;
          break;

	case R_QDSP6_GPREL16_0:
	case R_QDSP6_GPREL16_1:
	case R_QDSP6_GPREL16_2:
	case R_QDSP6_GPREL16_3:
	  {
	    bfd_vma base;
	    struct bfd_link_hash_entry *h;

/*
 * Relocation is expressed in absolute terms however GP will
 * point to the base, __default_sda_base__ which must be 512K
 * aligned (0x80000).  Knowing this and that the relocation is
 * GP relative we need to subtract sda_base.
 */

	    insn =
	      qdsp6_get_insn (input_bfd, howto, contents + rel->r_offset);

/* XXX_SM */
/*
 * As seen in : qdsp6_elf_get_relocated_section_contents
 * XXX_SM this need not be re-hashed for every reloc.  Once would
 * be enough... Change later and put into a static inline func.
 */
	    h =
	      bfd_link_hash_lookup (info->hash, SDA_BASE, FALSE, FALSE, TRUE);
	    if (h == (struct bfd_link_hash_entry *) NULL)
	      {
		h = bfd_link_hash_lookup (info->hash, DEFAULT_SDA_BASE,
					  FALSE, FALSE, TRUE);
	      }

	    if (h != (struct bfd_link_hash_entry *) NULL &&
		h->type == bfd_link_hash_defined)
	      {
		base = elf_gp (output_bfd) = (h->u.def.value
					      +
					      h->u.def.section->
					      output_section->vma +
					      h->u.def.section->
					      output_offset);
		relocation -= base;
	      }
	    else
	      {
		/* At this point, if no _SDA_BASE_, either the alternate or
		 * the proper one, is resolved, then GP-relocaiton overflow
		 * errors are likely.
		 */
	      }
/* XXX_SM */

	    ioffset = relocation + rel->r_addend;

	    if (!qdsp6_reloc_operand (howto, &insn,
				      relocation + rel->r_addend, NULL))
	      {
		r = info->callbacks->reloc_overflow
		  (info, (h ? &h->root : NULL),
                   name, howto->name, (bfd_vma) 0,
		   input_bfd, input_section, rel->r_offset);
	      }
	    else
	      {
		qdsp6_put_insn (input_bfd, howto, contents + rel->r_offset, insn);
	      }
	    continue;
	    break;
	  }

	case R_QDSP6_B7_PCREL:
	case R_QDSP6_B13_PCREL:
	case R_QDSP6_B15_PCREL:
	case R_QDSP6_B22_PCREL:
	  insn = qdsp6_get_insn (input_bfd, howto, contents + rel->r_offset);

/* relocation is in absolute terms, so we convert to abs terms */

	  ioffset = (relocation + rel->r_addend) -
	    (input_section->output_section->vma +
	     input_section->output_offset + rel->r_offset);

	  if (!qdsp6_reloc_operand (howto, &insn, ioffset, NULL))
	    {
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root : NULL),
                 name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	    }
	  else
	    {
	      qdsp6_put_insn (input_bfd, howto, contents + rel->r_offset, insn);
	    }
	  continue;
	  break;

	case R_QDSP6_32:
	case R_QDSP6_16:
	case R_QDSP6_8:
	case R_QDSP6_NONE:
	  break;

	default:
	  {
	    const char *msg = (const char *) NULL;
	    msg = _("internal error: unrecognized relocation type");
	    info->callbacks->warning (info, msg, name,
				      input_bfd, input_section,
				      rel->r_offset);
	    return FALSE;
	    break;
	  }
	}

      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
				    contents, rel->r_offset,
				    relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  const char *msg = (const char *) NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root : NULL),
                 name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	      break;

	    case bfd_reloc_undefined:
	      r = info->callbacks->undefined_symbol
		(info, name, input_bfd, input_section, rel->r_offset, TRUE);
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
    }

  return TRUE;
}

static bfd_boolean
qdsp6_elf_relax_section (bfd *input_bfd,
                         asection *isec,
                         struct bfd_link_info *link_info,
                         bfd_boolean *again)
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
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

  if (   link_info->relocatable
      || (isec->flags & SEC_RELOC) == 0
      || isec->reloc_count == 0)
    return rc;

  /* If needed, initialize this section's size.  */
  if (isec->size == 0)
    isec->size = isec->rawsize;

  BFD_ASSERT (bfd_hash_table_init ((struct bfd_hash_table *) &t_hash,
                                   _bfd_link_hash_newfunc,
/* XXX_SM : verify */
				  sizeof (struct bfd_link_hash_entry)));
/* XXX_SM : verify */


  /* It's quite hard to get rid of the relocation table once it's been read.
     Ideally, any relocations required by the trampoline should be added to it,
     but it seems that everything falls off if the table is changed in any way.
     Since the original relocation is voided, it and only it may be reused by
     the trampoline. */
  // elf_section_data (isec)->relocs = bfd_get_section_userdata (input_bfd, isec);
  internal_relocs = _bfd_elf_link_read_relocs (input_bfd, isec, NULL, NULL,
                                                 link_info->keep_memory);
  // elf_section_data (isec)->relocs = internal_relocs;
  // bfd_set_section_userdata (input_bfd, isec, internal_relocs);
  if (!internal_relocs)
    goto error_return;

  for (ireloc = 0; ireloc < isec->reloc_count; ireloc++)
    {
      bfd_vma at_base, to_base, t_base;
      bfd_vma at, t_at, from, to;
      bfd_signed_vma ioffset;
      bfd_vma r_type;

      irel = internal_relocs + ireloc;

      /* Look into relocation overflows at branches and add trampolines if needed. */
      r_type = ELF32_R_TYPE (irel->r_info);
      if (   link_info->qdsp6_trampolines
          && (   r_type == R_QDSP6_B22_PCREL
              || r_type == R_QDSP6_B15_PCREL
              || r_type == R_QDSP6_B13_PCREL))
        {
	/*
 	 * XXX_SM: using standard macro.
 	 * cooked_size is gone size should just work.
 	 */
          isec_size = bfd_section_size (input_bfd, isec);

          at      = irel->r_offset;
          from    = irel->r_offset - irel->r_addend;
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
          if (r_symndx < symtab_hdr->sh_info) /* Local to this input_bfd  */
	    {
	      /* A local symbol.  */
	      Elf_Internal_Sym *isym;

	      isym = isymbuf + ELF32_R_SYM (irel->r_info);

              name = bfd_malloc (sizeof (isym->st_name) * 2 + 1);
              sprintf (name, "%0*lx", (int) sizeof (isym->st_name) * 2, isym->st_name);

              to      = isym->st_value;
              to_base =   isec->output_section->vma
                        + isec->output_offset;
	    }
	  else
	    {
	      h = sym_hashes [r_symndx - symtab_hdr->sh_info];
	      while (h && (   h->root.type == bfd_link_hash_indirect
                           || h->root.type == bfd_link_hash_warning))
	        h = (struct elf_link_hash_entry *) h->root.u.i.link;

	      if (!h || (   h->root.type != bfd_link_hash_defined
                         && h->root.type != bfd_link_hash_defweak))
	        continue;

              name = (char *) h->root.root.string;

	      to      = h->root.u.def.value;
              to_base =   h->root.u.def.section->output_section->vma
                        + h->root.u.def.section->output_offset;
            }

          /* Check if the target is beyond reach. */
          ioffset = abs ((to + to_base) - (from + at_base));

          if (   (   (r_type == R_QDSP6_B22_PCREL)
                  && (ioffset > ~(~(bfd_signed_vma) 0 << 23)))
              || (   (r_type == R_QDSP6_B15_PCREL)
                  && (ioffset > ~(~(bfd_signed_vma) 0 << 16)))
              || (   (r_type == R_QDSP6_B13_PCREL)
                  && (ioffset > ~(~(bfd_signed_vma) 0 << 14))))
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

	          if (! bfd_get_section_contents (input_bfd, isec, contents,
					          (file_ptr) 0,
					          isec_size))
		    goto error_return;

                elf_section_data (isec)->this_hdr.contents = contents;
	      }

              /* Create a symbol for the trampoline. */
              t_name = bfd_malloc (  QDSP6_TRAMPOLINE_PREFIX_LEN + 1
                                   + strlen (name) + 1);
              sprintf (t_name, "%s_%s", QDSP6_TRAMPOLINE_PREFIX, name);

              /* Try to find it, otherwise, create it. */
              t_h = bfd_link_hash_lookup (&t_hash, t_name, FALSE, FALSE, FALSE);
              if (!t_h)
                {
                  t_at = isec_size;

                  if (   (   (r_type == R_QDSP6_B22_PCREL)
                          && ((t_at - from) > ~(~(bfd_signed_vma)0 << 23)))
                      || (   (r_type == R_QDSP6_B15_PCREL)
                          && ((t_at - from) > ~(~(bfd_signed_vma)0 << 16))))
                    /* No room for a trampoline. */
                    goto error_return;

                  isec_size += sizeof (qdsp6_trampoline);

                  /* Add room for the trampoline. */
                  elf_section_data (isec)->this_hdr.contents = contents =
                    bfd_realloc (contents, isec_size);

                  rc = bfd_set_section_size (input_bfd, isec, isec_size);
                  if (rc != TRUE)
                    goto error_return;

		/*
 		 * XXX_SM: using standard macro.
 		 * cooked_size is gone size should just work.
 		 */
                  isec_size = bfd_section_size (input_bfd, isec);

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
                  creloc =   sizeof (qdsp6_trampoline_rels)
                           / sizeof (qdsp6_trampoline_rels [0]);
                  if (creloc > 0)
                    {
#if 0
                      /* Save symbol information. */
                      treloc = isec->reloc_count;

                      /* Expand the relocation table. */
                      internal_relocs =
                        bfd_realloc (elf_section_data (isec)->relocs,
                                        (isec->reloc_count + creloc)
                                     * sizeof (*internal_relocs));
                      elf_section_data (isec)->relocs = internal_relocs;
                      isec->reloc_count += creloc;
                      irel = elf_section_data (isec)->relocs + ireloc;

                      /* Set the new relocation entries. */
                      while (creloc--)
#else
                      creloc = 0;
#endif
                        {
                          /* Reuse the original relocation. */
                          irel->r_offset =   t_h->u.def.value
                                           + qdsp6_trampoline_rels [creloc].offset;
                          irel->r_info
                            = ELF32_R_INFO (ELF32_R_SYM (irel->r_info),
                                            qdsp6_trampoline_rels [creloc].rtype);
                          irel->r_addend = 0;
                        }
                    }
                }
              else
                /* Remove the offending relocation. */
                irel->r_info =
                  ELF32_R_INFO (ELF32_R_SYM (irel->r_info), R_QDSP6_NONE);

              free (t_name);

              /* Get the effective address of the trampoline. */
              t_at   = t_h->u.def.value;
              t_base =   t_h->u.def.section->vma +
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

  bfd_hash_table_free ((struct bfd_hash_table *) &t_hash);
  return rc;

  error_return:
    rc = FALSE;

  if (isymbuf != NULL
      && symtab_hdr->contents != (unsigned char *) isymbuf)
    free (isymbuf);
  if (contents != NULL
      && elf_section_data (isec)->this_hdr.contents != contents)
    free (contents);
  if (internal_relocs != NULL
      && elf_section_data (isec)->relocs != internal_relocs)
    free (internal_relocs);

  bfd_hash_table_free ((struct bfd_hash_table *) &t_hash);
  return rc;
}


#define TARGET_LITTLE_SYM  bfd_elf32_littleqdsp6_vec
#define TARGET_LITTLE_NAME "elf32-littleqdsp6"
#define TARGET_BIG_SYM     bfd_elf32_bigqdsp6_vec
#define TARGET_BIG_NAME    "elf32-bigqdsp6"
#define ELF_ARCH           bfd_arch_qdsp6
#define ELF_MACHINE_CODE   EM_QDSP6
#define ELF_MAXPAGESIZE    0x1000
#define bfd_elf32_bfd_reloc_name_lookup _bfd_norelocs_bfd_reloc_name_lookup
#define elf_backend_can_gc_sections	1
#define elf_info_to_howto               0
#define elf_info_to_howto_rel           qdsp6_info_to_howto_rel

#define elf_backend_object_p		qdsp6_elf_object_p
#define elf_backend_gc_mark_hook        qdsp6_elf_gc_mark_hook
#define elf_backend_gc_sweep_hook       qdsp6_elf_gc_sweep_hook
#define elf_backend_check_relocs        qdsp6_elf_check_relocs
#define elf_backend_relocate_section    qdsp6_elf_relocate_section
#define bfd_elf32_bfd_relax_section     qdsp6_elf_relax_section
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

#define bfd_elf32_bfd_get_relocated_section_contents \
                                        qdsp6_elf_get_relocated_section_contents

/* We need to use RELAs to get the computations for the HI16/LO16
   relocations to be correct in the presence of addends; for now
   we default to using them everywhere. Eventually we should fix
   this so we only use RELA for the .text sections. Ideally, we
   would have both REL and RELA relocation sections for a .text
   section, but that is a lot of mucking about...
*/
#define elf_backend_may_use_rel_p	0
#define	elf_backend_may_use_rela_p	1
#define	elf_backend_default_use_rela_p	1

/* This is a bit of a hack
   It installs our wrapper for _bfd_elf_set_arch_mach
*/
#undef BFD_JUMP_TABLE_WRITE
#define BFD_JUMP_TABLE_WRITE(NAME) \
   qdsp6_elf_set_arch_mach, \
   bfd_elf32_set_section_contents

#include "elf32-target.h"
