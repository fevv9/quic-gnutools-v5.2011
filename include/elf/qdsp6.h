/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/* QDSP6 ELF support for BFD.
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

/* This file holds definitions specific to the QDSP6 ELF ABI.  */

#ifndef _ELF_QDSP6_H
#define _ELF_QDSP6_H

#include "elf/reloc-macros.h"

/* Relocations.  */
START_RELOC_NUMBERS (elf_qdsp6_reloc_type)

  RELOC_NUMBER (R_QDSP6_NONE,          0)
  RELOC_NUMBER (R_QDSP6_B22_PCREL,     1)
  RELOC_NUMBER (R_QDSP6_B15_PCREL,     2)
  RELOC_NUMBER (R_QDSP6_B7_PCREL,      3)
  RELOC_NUMBER (R_QDSP6_LO16,          4)
  RELOC_NUMBER (R_QDSP6_HI16,          5)
  RELOC_NUMBER (R_QDSP6_32,            6)
  RELOC_NUMBER (R_QDSP6_16,            7)
  RELOC_NUMBER (R_QDSP6_8,             8)
  RELOC_NUMBER (R_QDSP6_GPREL16_0,     9)
  RELOC_NUMBER (R_QDSP6_GPREL16_1,    10)
  RELOC_NUMBER (R_QDSP6_GPREL16_2,    11)
  RELOC_NUMBER (R_QDSP6_GPREL16_3,    12)
  RELOC_NUMBER (R_QDSP6_HL16,         13)
  RELOC_NUMBER (R_QDSP6_B13_PCREL,    14)

END_RELOC_NUMBERS (R_QDSP6_max)

/* Processor-specific flags for the ELF header e_TYPE field.  */

/* Object is WHIRL, matching ET_SGI_IR. */
#define ET_QDSP6_IR (ET_LOPROC)

/* Processor-specific flags for the ELF header e_flags field.  */

/* Four bit QDSP6 machine type field.  */
#define EF_QDSP6_MACH 0x0f
/* Limit to 16 CPU types for now. */

/* Various CPU types.  */
#define E_QDSP6_MACH    0
#define E_QDSP6_MACH_V2 1
#define E_QDSP6_MACH_V3 2

/* File contains position independent code.  */
#define EF_QDSP6_PIC 0x00000100

/* Processor-specific section indices.  These sections do not actually
   exist.  Symbols with a st_shndx field corresponding to one of these
   values have a special meaning.  */

/* Small common symbol.  */
#define SHN_QDSP6_SCOMMON   (SHN_LOPROC + 0)
#define SHN_QDSP6_SCOMMON_1 (SHN_LOPROC + 1)
#define SHN_QDSP6_SCOMMON_2 (SHN_LOPROC + 2)
#define SHN_QDSP6_SCOMMON_4 (SHN_LOPROC + 3)
#define SHN_QDSP6_SCOMMON_8 (SHN_LOPROC + 4)

/* Processor-specific section flags.  */

/* This section must be in the global data area.  */
#define SHF_QDSP6_GPREL (~SHF_MASKPROC + 1)

/* Processor-specific section type.  */

/* Link editor is to sort the entries in this section based on their sizes. */
#define SHT_QDSP6_ORDERED SHT_LOPROC

/* Processor specific program header types.  */

/* Different memory types. */
#define PT_QDSP6_EBI (PT_LOAD)
#define PT_QDSP6_SMI (PT_LOPROC + 0)
#define PT_QDSP6_TCM (PT_LOPROC + 1)

/* Processor specific program header flags.  */

/* Cacheability memory flags. */
#define PF_QDSP6_UC 0x80000000  /* Cacheable/uncacheable */
#define PF_QDSP6_WT 0x40000000  /* Write-back/write-through */
#define PF_QDSP6_N2 0x20000000  /* L2 cacheable/uncacheable */

/* Various port-specific parameters. */

/* Default alignments. */
#define DEFAULT_CODE_ALIGNMENT (2) /* log2 (4) */
#define DEFAULT_CODE_FALIGN    (4) /* log2 (16) */
#define DEFAULT_DATA_ALIGNMENT (3) /* log2 (8) */

#define QDSP6_NOP     "nop"
/* Default GP size. */
#define QDSP6_SMALL_GPSIZE 8

#endif /* _ELF_QDSP6_H */
