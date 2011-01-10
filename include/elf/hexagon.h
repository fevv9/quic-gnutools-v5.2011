
/* Hexagon ELF support for BFD.
   Copyright 1995, 1997, 1998, 2000, 2001 Free Software Foundation, Inc.

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

/* This file holds definitions specific to the Hexagon ELF ABI.  */

#ifndef _ELF_HEXAGON_H
#define _ELF_HEXAGON_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_hexagon_reloc_type)

  /* V2 */
  RELOC_NUMBER (R_HEXAGON_NONE,           0)
  RELOC_NUMBER (R_HEXAGON_B22_PCREL,      1)
  RELOC_NUMBER (R_HEXAGON_B15_PCREL,      2)
  RELOC_NUMBER (R_HEXAGON_B7_PCREL,       3)
  RELOC_NUMBER (R_HEXAGON_LO16,           4)
  RELOC_NUMBER (R_HEXAGON_HI16,           5)
  RELOC_NUMBER (R_HEXAGON_32,             6)
  RELOC_NUMBER (R_HEXAGON_16,             7)
  RELOC_NUMBER (R_HEXAGON_8,              8)
  RELOC_NUMBER (R_HEXAGON_GPREL16_0,      9)
  RELOC_NUMBER (R_HEXAGON_GPREL16_1,     10)
  RELOC_NUMBER (R_HEXAGON_GPREL16_2,     11)
  RELOC_NUMBER (R_HEXAGON_GPREL16_3,     12)
  RELOC_NUMBER (R_HEXAGON_HL16,          13)
  /* V3 */
  RELOC_NUMBER (R_HEXAGON_B13_PCREL,     14)
  /* V4 */
  RELOC_NUMBER (R_HEXAGON_B9_PCREL,      15)
  /* V4 (extenders) */
  RELOC_NUMBER (R_HEXAGON_B32_PCREL_X,   16)
  RELOC_NUMBER (R_HEXAGON_32_6_X,        17)
  /* V4 (extended) */
  RELOC_NUMBER (R_HEXAGON_B22_PCREL_X,   18)
  RELOC_NUMBER (R_HEXAGON_B15_PCREL_X,   19)
  RELOC_NUMBER (R_HEXAGON_B13_PCREL_X,   20)
  RELOC_NUMBER (R_HEXAGON_B9_PCREL_X,    21)
  RELOC_NUMBER (R_HEXAGON_B7_PCREL_X,    22)
  RELOC_NUMBER (R_HEXAGON_16_X,          23)
  RELOC_NUMBER (R_HEXAGON_12_X,          24)
  RELOC_NUMBER (R_HEXAGON_11_X,          25)
  RELOC_NUMBER (R_HEXAGON_10_X,          26)
  RELOC_NUMBER (R_HEXAGON_9_X,           27)
  RELOC_NUMBER (R_HEXAGON_8_X,           28)
  RELOC_NUMBER (R_HEXAGON_7_X,           29)
  RELOC_NUMBER (R_HEXAGON_6_X,           30)
  /* V2 PIC */
  RELOC_NUMBER (R_HEXAGON_32_PCREL,      31)
  RELOC_NUMBER (R_HEXAGON_COPY,          32)
  RELOC_NUMBER (R_HEXAGON_GLOB_DAT,      33)
  RELOC_NUMBER (R_HEXAGON_JMP_SLOT,      34)
  RELOC_NUMBER (R_HEXAGON_RELATIVE,      35)
  RELOC_NUMBER (R_HEXAGON_PLT_B22_PCREL, 36)
  RELOC_NUMBER (R_HEXAGON_GOTREL_LO16,   37)
  RELOC_NUMBER (R_HEXAGON_GOTREL_HI16,   38)
  RELOC_NUMBER (R_HEXAGON_GOTREL_32,     39)
  RELOC_NUMBER (R_HEXAGON_GOT_LO16,      40)
  RELOC_NUMBER (R_HEXAGON_GOT_HI16,      41)
  RELOC_NUMBER (R_HEXAGON_GOT_32,        42)
  RELOC_NUMBER (R_HEXAGON_GOT_16,        43)

END_RELOC_NUMBERS (R_HEXAGON_max)

/* Processor-specific flags for the ELF header e_type field.  */

/* Object is WHIRL, matching ET_SGI_IR. */
#define ET_HEXAGON_IR (ET_LOPROC)

/* Processor-specific flags for the ELF header e_flags field.  */

/* Four-bit machine type field.  */
#define EF_HEXAGON_MACH 0x0f
/* Limit to 16 CPU types for now. */
#define EF_HEXAGON_MACH_VER(e_flags) ((e_flags) & EF_HEXAGON_MACH)
/* Various CPU types.  */
#define EF_HEXAGON_MACH_V1 0
#define EF_HEXAGON_MACH_V2 1
#define EF_HEXAGON_MACH_V3 2
#define EF_HEXAGON_MACH_V4 3
#define EF_HEXAGON_MACH_V5 4

/* File contains position independent code.  */
#define EF_HEXAGON_PIC 0x00000100

/* Processor-specific section indices.  These sections do not actually
   exist.  Symbols with a st_shndx field corresponding to one of these
   values have a special meaning.  */

/* Small common symbol.  */
#define SHN_HEXAGON_SCOMMON   (SHN_LOPROC + 0)
#define SHN_HEXAGON_SCOMMON_1 (SHN_LOPROC + 1)
#define SHN_HEXAGON_SCOMMON_2 (SHN_LOPROC + 2)
#define SHN_HEXAGON_SCOMMON_4 (SHN_LOPROC + 3)
#define SHN_HEXAGON_SCOMMON_8 (SHN_LOPROC + 4)

/* Processor-specific section flags.  */

/* This section must be in the global data area.  */
#define SHF_HEXAGON_GPREL (~SHF_MASKPROC + 1)

/* Processor-specific section type.  */

/* Link editor is to sort the entries in this section based on their sizes. */
#define SHT_HEXAGON_ORDERED (SHT_LOPROC + 0)

/* Processor specific program header types.  */

/* Different memory types. */
#define PT_HEXAGON_EBI (PT_LOAD)
#define PT_HEXAGON_SMI (PT_LOPROC + 0)
#define PT_HEXAGON_TCM (PT_LOPROC + 1)

/* Processor-specific program header flags.  */

/* Cacheability memory flags. */
#define PF_HEXAGON_UC 0x80000000	/* Cacheable/uncacheable */
#define PF_HEXAGON_WT 0x40000000	/* Write-back/write-through */
#define PF_HEXAGON_U2 0x20000000	/* L2 cacheable/uncacheable */
#define PF_HEXAGON_SH 0x10000000	/* Non-shared/shared */

/* Processor-specific dynamic array tags.  */

#define DT_HEXAGON_SYMSZ (DT_LOPROC + 0) /* Size, in bytes, of the .dynsym section, also DT_SYMTAB. */
#define DT_HEXAGON_VER   (DT_LOPROC + 1) /* Version of the dynamic sections. */

/* Various port-specific parameters. */

/* Default alignments. */
#define DEFAULT_CODE_ALIGNMENT (2) /* log2 (4) */
#define DEFAULT_CODE_FALIGN    (4) /* log2 (16) */
#define DEFAULT_DATA_ALIGNMENT (3) /* log2 (8) */

#define HEXAGON_NOP     "nop"
/* Default GP size. */
#define HEXAGON_SMALL_GPSIZE 8

#endif /* _ELF_HEXAGON_H */
