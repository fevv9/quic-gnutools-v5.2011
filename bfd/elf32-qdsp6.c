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


#define TARGET_LITTLE_SYM  bfd_elf32_littleqdsp6_vec
#define TARGET_LITTLE_NAME "elf32-littleqdsp6"
#define TARGET_BIG_SYM     bfd_elf32_bigqdsp6_vec
#define TARGET_BIG_NAME    "elf32-bigqdsp6"
#define ELF_ARCH           bfd_arch_qdsp6
#define ELF_MACHINE_CODE   EM_QDSP6
#define ELF_MAXPAGESIZE    0x1000
#define bfd_elf32_bfd_reloc_type_lookup bfd_default_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup _bfd_norelocs_bfd_reloc_name_lookup
#define elf_info_to_howto               0
#define elf_info_to_howto_rel           qdsp6_info_to_howto_rel


