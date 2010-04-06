/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/* Opcode table for the QDSP6.
   Copyright 2004
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "sysdep.h"
#include "ansidecl.h"
#include "bfd.h"
#include "opcode/qdsp6.h"
#include "safe-ctype.h"
#include "libiberty.h"

static long qdsp6_extend (long *, unsigned, int);
static char *qdsp6_parse_reg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_dreg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_reg16
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_dreg16
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_reg8
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_dreg8
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_reg2
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_preg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_creg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_dcreg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_greg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_dgreg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_sreg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_dsreg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);
static char *qdsp6_parse_mreg
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   char *, long *, int *, char **);

int qdsp6_verify_hw;

qdsp6_insn qdsp6_nop, qdsp6_kext;

/* Various types of QDSP6 operands */
const qdsp6_operand qdsp6_operands [] =
{
  { "Rs32",       5, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_REGISTER | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg },
  { "Rt32",       5, 't', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_REGISTER | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg },
  { "Ru32",       5, 'u', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_REGISTER | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg },
  { "Rd32",       5, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_REGISTER | QDSP6_OPERAND_IS_WRITE,
    "r%u", NULL, qdsp6_parse_reg },
  { "Re32",       5, 'e', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_REGISTER | QDSP6_OPERAND_IS_WRITE,
    "r%u", NULL, qdsp6_parse_reg },
  { "Rx32",       5, 'x', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_REGISTER | QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE,
    "r%u", NULL, qdsp6_parse_reg },

  { "Rss32",      5, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ,
    "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Rtt32",      5, 't', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ,
    "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Rdd32",      5, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_WRITE,
    "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Rxx32",      5, 'x', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE,
    "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Ryy32",      5, 'y', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE,
    "r%u:%u", NULL, qdsp6_parse_dreg },

  { "Rd16",        4, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_WRITE,
    "r%u", NULL, qdsp6_parse_reg16 },
  { "Re16",        4, 'e', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_WRITE,
    "r%u", NULL, qdsp6_parse_reg16 },
  { "Rs16",        4, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg16 },
  { "Rt16",        4, 't', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg16 },
  { "Ru16",        4, 'u', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg16 },
  { "Rv16",        4, 'v', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg16 },
  { "Rx16",        4, 'x', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE,
    "r%u", NULL, qdsp6_parse_reg16 },
  { "Ry16",        4, 'y', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE,
    "r%u", NULL, qdsp6_parse_reg16 },

  { "Rdd16",       4, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_WRITE,
    "r%u:%u", NULL, qdsp6_parse_dreg16 },

  { "Rd8",        3, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_WRITE,
    "r%u", NULL, qdsp6_parse_reg8 },

  { "Rdd8",       3, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_WRITE,
    "r%u:%u", NULL, qdsp6_parse_dreg8 },
  { "Ree8",       3, 'e', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_WRITE,
    "r%u:%u", NULL, qdsp6_parse_dreg8 },
  { "Rtt8",       3, 't', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ,
    "r%u:%u", NULL, qdsp6_parse_dreg8 },
  { "Rvv8",       3, 'v', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ,
    "r%u:%u", NULL, qdsp6_parse_dreg8 },

  { "Rs2",        1, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMPLIED | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg2 },

  { "Ns8",        3, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMPLIED | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg2 },
  { "Nt8",        3, 't', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMPLIED | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg2 },

  { "Rs0",        0, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SUBSET | QDSP6_OPERAND_IS_READ,
    "r%u", NULL, qdsp6_parse_reg },

  { "Mu2",        1, 'u', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_MODIFIER | QDSP6_OPERAND_IS_READ,
    "m%u", NULL, qdsp6_parse_mreg },

  { "Ps4",        2, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PREDICATE | QDSP6_OPERAND_IS_READ,
    "p%u", NULL, qdsp6_parse_preg },
  { "Pt4",        2, 't', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PREDICATE | QDSP6_OPERAND_IS_READ,
    "p%u", NULL, qdsp6_parse_preg },
  { "Pu4",        2, 'u', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PREDICATE | QDSP6_OPERAND_IS_READ,
    "p%u", NULL, qdsp6_parse_preg },
  { "Pv4",        2, 'v', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PREDICATE | QDSP6_OPERAND_IS_READ,
    "p%u", NULL, qdsp6_parse_preg },
  { "Pd4",        2, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PREDICATE | QDSP6_OPERAND_IS_WRITE,
    "p%u", NULL, qdsp6_parse_preg },
  { "Px4",        2, 'x', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PREDICATE | QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE,
    "p%u", NULL, qdsp6_parse_preg },

  { "Cs32",       5, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONTROL | QDSP6_OPERAND_IS_READ,
    "", NULL, qdsp6_parse_creg },
  { "Cd32",       5, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONTROL | QDSP6_OPERAND_IS_WRITE,
    "", NULL, qdsp6_parse_creg },

  { "Css32",       5, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONTROL | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ,
    "c%u:%u", NULL, qdsp6_parse_dcreg },
  { "Cdd32",       5, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONTROL | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_WRITE,
    "c%u:%u", NULL, qdsp6_parse_dcreg },

  { "Gs32",       5, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_GUEST | QDSP6_OPERAND_IS_READ,
    "", NULL, qdsp6_parse_greg },
  { "Gd32",       5, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_GUEST | QDSP6_OPERAND_IS_WRITE,
    "", NULL, qdsp6_parse_greg },

  { "Gss32",       5, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_GUEST | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ,
    "g%u:%u", NULL, qdsp6_parse_dgreg },
  { "Gdd32",       5, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_GUEST | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_WRITE,
    "g%u:%u", NULL, qdsp6_parse_dgreg },

  { "Ss64",       6, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SYSTEM | QDSP6_OPERAND_IS_READ,
    "", NULL, qdsp6_parse_sreg },
  { "Sd64",       6, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SYSTEM | QDSP6_OPERAND_IS_WRITE,
    "", NULL, qdsp6_parse_sreg },

  { "Sss64",      6, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SYSTEM | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_READ,
    "s%u:%u", NULL, qdsp6_parse_dsreg },
  { "Sdd64",      6, 'd', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_SYSTEM | QDSP6_OPERAND_IS_PAIR | QDSP6_OPERAND_IS_WRITE,
    "s%u:%u", NULL, qdsp6_parse_dsreg },

  { "#g16:0",    16, 'i', 0,
    BFD_RELOC_QDSP6_GPREL16_0, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_16_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%g", "#0x%x", NULL },
  { "#g16:1",    16, 'i', 1,
    BFD_RELOC_QDSP6_GPREL16_1, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_16_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%g", "#0x%x", NULL },
  { "#g16:2",    16, 'i', 2,
    BFD_RELOC_QDSP6_GPREL16_2, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_16_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%g", "#0x%x", NULL },
  { "#g16:3",    16, 'i', 3,
    BFD_RELOC_QDSP6_GPREL16_3, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_16_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%g", "#0x%x", NULL },

  { "#u26:6",    26, 'i', 6,
    BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_KXER,
    "#%u", NULL, NULL },
  { "#u16:0",    16, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u16:1",    16, 'i', 1,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u16:2",    16, 'i', 2,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u16:3",    16, 'i', 3,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u16",      16, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_16_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u11:3",    11, 'i', 3,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u9",        9, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_9_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u8",        8, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_8_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u7",        7, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u6:0",      6, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u6:1",      6, 'i', 1,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u6:2",      6, 'i', 2,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u6:3",      6, 'i', 3,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u6",        6, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u5:2",      5, 'i', 2,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u5:3",      5, 'i', 3,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u5",        5, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u4:0",      4, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u4:2",      4, 'i', 2,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u4",        4, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u3:0",      3, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u3:1",      3, 'i', 1,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u3",        3, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#u2",        2, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U10",      10, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_10_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U6:2",      6, 'I', 2,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U6",        6, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U5:2",      5, 'I', 2,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U5:3",      5, 'I', 3,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U5",        5, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U4:0",      4, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U4:2",      4, 'I', 2,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U4",        4, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U3:0",      3, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U3:1",      3, 'I', 1,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U3:2",      3, 'I', 2,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U3:3",      3, 'I', 3,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U3",        3, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },
  { "#U2",        2, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE,
    "#%u", NULL, NULL },

  { "#s16",      16, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_16_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s12",      12, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_12_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s11:0",    11, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_11_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s11:1",    11, 'i', 1,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_11_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s11:2",    11, 'i', 2,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_11_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s11:3",    11, 'i', 3,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_11_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s10",      10, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_10_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s8",        8, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_8_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s7",        7, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_7_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s6:3",      6, 'i', 3,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s6",        6, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s4:0",      4, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s4:1",      4, 'i', 1,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s4:2",      4, 'i', 2,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s4:3",      4, 'i', 3,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#s4",        4, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#S8",        8, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_8_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#S7",        7, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_7_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#S6:3",      6, 'I', 3,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },
  { "#S6",        6, 'I', 0,
    BFD_RELOC_NONE, BFD_RELOC_QDSP6_32_6_X, BFD_RELOC_QDSP6_6_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED,
    "#%d", NULL, NULL },

  { "#m9",        9, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED | QDSP6_OPERAND_IS_NEGATIVE,
    "#%d", NULL, NULL },

  { "#r22:2",    22, 'i', 2,
    BFD_RELOC_QDSP6_B22_PCREL, BFD_RELOC_QDSP6_B32_PCREL_X, BFD_RELOC_QDSP6_B22_PCREL_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_PC_RELATIVE | QDSP6_OPERAND_IS_SIGNED,
    "@%u", NULL, NULL },
  { "#r15:2",    15, 'i', 2,
    BFD_RELOC_QDSP6_B15_PCREL, BFD_RELOC_QDSP6_B32_PCREL_X, BFD_RELOC_QDSP6_B15_PCREL_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_PC_RELATIVE | QDSP6_OPERAND_IS_SIGNED,
    "@%u", NULL, NULL },
  { "#r13:2",    13, 'i', 2,
    BFD_RELOC_QDSP6_B13_PCREL, BFD_RELOC_QDSP6_B32_PCREL_X, BFD_RELOC_QDSP6_B13_PCREL_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_PC_RELATIVE | QDSP6_OPERAND_IS_SIGNED,
    "@%u", NULL, NULL },
  { "#r9:2",      9, 'i', 2,
    BFD_RELOC_QDSP6_B9_PCREL, BFD_RELOC_QDSP6_B32_PCREL_X, BFD_RELOC_QDSP6_B9_PCREL_X,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_PC_RELATIVE | QDSP6_OPERAND_IS_SIGNED,
    "@%u", NULL, NULL },
  { "#r7:2",      7, 'i', 2,
    BFD_RELOC_QDSP6_B7_PCREL,  BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_PC_RELATIVE | QDSP6_OPERAND_IS_SIGNED,
    "@%u", NULL, NULL },

  /* These do appear in instructions, but are only matched by the relocation. */
  { "#u26:6",    26, 'i', 6,
    BFD_RELOC_QDSP6_B32_PCREL_X, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_PC_RELATIVE | QDSP6_OPERAND_IS_SIGNED | QDSP6_OPERAND_IS_KXER,
    "#%u", NULL, NULL },

  /* These don't appear in any instructions.  We get these by mapping
     from the previous operands when we see the lo/hi modifier in the
     assembly. */
  { "#u16_lo16", 16, 'i', 0,
    BFD_RELOC_QDSP6_LO16, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_LO16,
    "#%u", "#0x%04x", NULL },
  { "#u16_hi16", 16, 'i', 0,
    BFD_RELOC_QDSP6_HI16, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_HI16,
    "#%u", "#0x%04x", NULL },
  { "#s16_lo16", 16, 'i', 0,
    BFD_RELOC_QDSP6_LO16, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED | QDSP6_OPERAND_IS_LO16,
    "#%d", NULL, NULL },
  { "#s16_hi16", 16, 'i', 0,
    BFD_RELOC_QDSP6_HI16, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_IMMEDIATE | QDSP6_OPERAND_IS_SIGNED | QDSP6_OPERAND_IS_HI16,
    "#%d", NULL, NULL },

  /* These don't appear in any instructions.  They are used by the packet header. */
  { "Hs",         4, 's', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    0,
    "0x%x", NULL, NULL },
  { "Hi",         1, 'i', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    0,
    "%d", NULL, NULL },
  { "Ho",         1, 'o', 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    0,
    "%d", NULL, NULL },

  /* These are just place-holders for implied operands. */
  { "#-1",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONSTANT,
    "#-1", NULL, NULL },
  { "#0",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONSTANT,
    "#0", NULL, NULL },
  { "#1",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONSTANT,
    "#1", NULL, NULL },
  { "#2",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONSTANT,
    "#2", NULL, NULL },
  { "#3",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONSTANT,
    "#3", NULL, NULL },
  { "#255",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_CONSTANT,
    "#255", NULL, NULL },
  { "p0",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PREDICATE | QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE,
    "p0", NULL, NULL },
  { "p1",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_PREDICATE | QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE,
    "p1", NULL, NULL },
  { "r31",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_READ,
    "lr", NULL, NULL },
  { "lr",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_READ,
    "lr", NULL, NULL },
  { "r29",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_READ,
    "sp", NULL, NULL },
  { "sp",         0,   0, 0,
    BFD_RELOC_NONE, BFD_RELOC_NONE, BFD_RELOC_NONE,
    QDSP6_OPERAND_IS_READ,
    "sp", NULL, NULL },
};

const size_t qdsp6_operand_count =
  sizeof (qdsp6_operands) / sizeof (qdsp6_operands [0]);

/* QDSP6 instructions.

   Longer versions of insns must appear before shorter ones.

   Instructions that are really macros based on other insns must appear
   before the real insn so they're chosen when disassembling. */

/* Top level variables. Point to correct variables based on the type of
   QDSP6 architecture */
qdsp6_opcode *qdsp6_opcodes;
size_t qdsp6_opcodes_count;

#define QDSP6_MAP_FNAME(TAG) qdsp6_map_##TAG
#define QDSP6_MAP_SIGN(TAG) void QDSP6_MAP_FNAME (TAG) \
  (char *i, size_t n, const qdsp6_operand_arg args [])
#define MAP_FUNCTION(TAG) QDSP6_MAP_SIGN (TAG); \
  QDSP6_MAP_SIGN (TAG)
#define DEST (i), (n)
#define GET_OP_VAL(N) ((int) args [N].value)
#define GET_OP_STR(N) (args [N].string [0] == '#' \
                       ? args [N].string + 1 \
                       : args [N].string)

#define sprintf snprintf
#  include "opcode/qdsp6_asm_maps.h"
#undef sprintf

/* V2 */
qdsp6_opcode qdsp6_opcodes_v2 [] =
{
#include "opcode/qdsp6_iset_v2.h"
};

const size_t qdsp6_opcodes_count_v2 =
  sizeof (qdsp6_opcodes_v2) / sizeof (qdsp6_opcodes_v2 [0]);

/* V3 */
qdsp6_opcode qdsp6_opcodes_v3 [] =
{
#include "opcode/qdsp6_iset_v3.h"
};

const size_t qdsp6_opcodes_count_v3 =
  sizeof (qdsp6_opcodes_v3) / sizeof (qdsp6_opcodes_v3 [0]);

/* V4 */
qdsp6_opcode qdsp6_opcodes_v4 [] =
{
#include "opcode/qdsp6_iset_v4.h"
};

const size_t qdsp6_opcodes_count_v4 =
  sizeof (qdsp6_opcodes_v4) / sizeof (qdsp6_opcodes_v4 [0]);

#define QDSP6_HASH_SIZE (5 * QDSP6_HASH_1 + 17) /* Add special groups and hashes. */
#define QDSP6_HASH_0    (0 * QDSP6_HASH_1)      /* 1st group. */
#define QDSP6_HASH_1    (1 * ('z' - 'a' + 1))   /* 2nd group. */
#define QDSP6_HASH_2    (2 * QDSP6_HASH_1)      /* 3rd group. */
#define QDSP6_HASH_3    (3 * QDSP6_HASH_1)      /* 3rd group. */
#define QDSP6_HASH_4    (4 * QDSP6_HASH_1)      /* 3rd group. */
#define QDSP6_HASH_5    (5 * QDSP6_HASH_1)      /* 3rd group. */
#define QDSP6_HASH_A    (QDSP6_HASH_SIZE -  1)
#define QDSP6_HASH_B    (QDSP6_HASH_SIZE -  2)
#define QDSP6_HASH_C    (QDSP6_HASH_SIZE -  3)
#define QDSP6_HASH_D    (QDSP6_HASH_SIZE -  4)
#define QDSP6_HASH_E    (QDSP6_HASH_SIZE -  5)
#define QDSP6_HASH_F    (QDSP6_HASH_SIZE -  6)
#define QDSP6_HASH_G    (QDSP6_HASH_SIZE -  7)
#define QDSP6_HASH_H    (QDSP6_HASH_SIZE -  8)
#define QDSP6_HASH_I    (QDSP6_HASH_SIZE -  9)
#define QDSP6_HASH_J    (QDSP6_HASH_SIZE - 10)
#define QDSP6_HASH_K    (QDSP6_HASH_SIZE - 11)
#define QDSP6_HASH_L    (QDSP6_HASH_SIZE - 12)
#define QDSP6_HASH_M    (QDSP6_HASH_SIZE - 13)
#define QDSP6_HASH_N    (QDSP6_HASH_SIZE - 14)
#define QDSP6_HASH_O    (QDSP6_HASH_SIZE - 15)
#define QDSP6_HASH_P    (QDSP6_HASH_SIZE - 16)
#define QDSP6_HASH_Z    (QDSP6_HASH_SIZE - 17)  /* Miscellanea. */

/* Indexed by first letter of opcode.  Points to chain of opcodes with same
   first letter.  */
static qdsp6_opcode *opcode_map [QDSP6_HASH_SIZE];

static size_t qdsp6_icode_hash_bits;

/* Indexed by insn code.  Points to chain of opcodes with same insn code.  */
static qdsp6_opcode **icode_map;

static const qdsp6_reg qdsp6_gp_regs [] =
{
  { "sp", 29, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "fp", 30, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "lr", 31, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
};

static const size_t qdsp6_gp_regs_count =
  sizeof (qdsp6_gp_regs) / sizeof (qdsp6_gp_regs [0]);

/* They can also be named by Sx, where x is
   the supervisor register number */

static qdsp6_reg qdsp6_supervisor_regs [] =
{
  /* Remapped V4 system registers */
  {"sgp0",          0, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"sgp1",          1, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"stid",          2, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"elr",           3, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"badva0",        4, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"badva1",        5, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"ssr",           6, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"ccr",           7, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"htid",          8, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"badva",         9, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"imask",        10, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"modectl",      17, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"ipend",        20, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"vid",          21, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"iad",          22, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"iel",          24, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"iahl",         26, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"cfgbase",      27, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"diag",         28, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"rev",          29, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"pcyclelo",     30, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pcyclehi",     31, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"brkptpc0",     36, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"brkptcfg0",    37, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"brkptpc1",     38, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"brkptcfg1",    39, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"isdbmbxin",    40, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"isdbmbxout",   41, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"isdben",       42, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"isdbgpr",      43, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s40",          40, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"s39",          39, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s29",          29, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"s25",          25, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s22",          22, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  {"s19",          19, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},

  /* Legacy system register map */
  {"sgp",           0, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"sgpr1",         1, 0},			// ****** reserved ****** enabled for DSP team
  {"ssr",           2, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"imask",         3, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"badva",         4, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"elr",           5, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"tid",           6, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"evb",          16, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"ipend",        17, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"syscfg",       18, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"modectl",      19, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"rev",          20, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"tlbhi",        21, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"tlblo",        22, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"tlbidx",       23, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"diag",         24, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"iad",          25, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"iel",          26, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"iahl",         27, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pcyclehi",     30, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pcyclelo",     31, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"isdbst",       32, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"isdbcfg0",     33, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"isdbcfg1",     34, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"brkptpc0",     35, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"brkptcfg0",    36, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"brkptpc1",     37, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"brkptcfg1",    38, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"isdbmbxin",    39, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"isdbmbxout",   40, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"isdben",       41, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"isdbgpr",      42, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pmucnt0",      48, QDSP6_V3_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pmucnt1",      49, QDSP6_V3_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pmucnt2",      50, QDSP6_V3_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pmucnt3",      51, QDSP6_V3_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pmuevtcfg",    52, QDSP6_V3_AND_UP | QDSP6_REG_IS_READWRITE},
  {"pmucfg",       53, QDSP6_V3_AND_UP | QDSP6_REG_IS_READWRITE},

  {"acc0",         61, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"acc1",         62, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"chicken",      63, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},

  {"s63",          63, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s62",          62, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s61",          61, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s60",          60, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s59",          59, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s58",          58, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s57",          57, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s56",          56, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},

  {"s55",          55, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s54",          54, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s53",          53, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s52",          52, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s51",          51, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s50",          50, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s49",          49, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s48",          48, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},

  {"s47",          47, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s46",          46, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s45",          45, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s44",          44, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s43",          43, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},	// ****** reserved ****** enabling some for the DSP team
  {"s42",          42, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s41",          41, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s40",          40, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},

  {"s39",          39, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"s38",          38, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s37",          37, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s36",          36, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s35",          35, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s34",          34, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s33",          33, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s32",          32, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},

  {"s31",          31, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s30",          30, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s29",          29, 0}, 		// ****** reserved ******
  {"s28",          28, 0}, 		// ****** reserved ****** enabled for DSP team
  {"s27",          27, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s26",          26, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s25",          25, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"s24",          24, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},

  {"s23",          23, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s22",          22, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s21",          21, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s20",          20, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"s19",          19, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"s18",          18, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s17",          17, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  {"s16",          16, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},

  {"s15",          15, 0}, 		// ****** reserved ******
  {"s14",          14, 0}, 		// ****** reserved ******
  {"s13",          13, 0}, 		// ****** reserved ******
  {"s12",          12, 0}, 		// ****** reserved ******
  {"s11",          11, 0}, 		// ****** reserved ******
  {"s10",          10, 0}, 		// ****** reserved ******
  {"s9",            9, 0}, 		// ****** reserved ******
  {"s8",            8, 0}, 		// ****** reserved ******

  {"s7",            7, 0}, 			// ****** reserved ****** enabled for DSP team
  {"s6",            6, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s5",            5, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s4",            4, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s3",            3, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s2",            2, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  {"s1",            1, 0},
  {"s0",            0, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
};

static const size_t qdsp6_supervisor_regs_count =
  sizeof (qdsp6_supervisor_regs) / sizeof (qdsp6_supervisor_regs [0]);

static const qdsp6_reg qdsp6_control_regs [] =
{
  { "sa0",         0, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "lc0",         1, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "sa1",         2, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "lc1",         3, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "p3:0",        4, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
//{ "p3:2",        5 },     ***** reserved *****
  { "m0",          6, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "m1",          7, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "usr",         8, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "pc",          9, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  { "ugp",        10, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "gp",         11, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "cs0",        12, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  { "cs1",        13, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  { "upcyclelo",  24, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "upcyclehi",  25, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "upmucnt0",   26, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "upmucnt1",   27, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "upmucnt2",   28, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "upmucnt3",   29, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  /* They can also be named by cX, where X is
     the control register number */
  /* Note that they are in reverse order so that they
     are parsed properly (e.g., c10 is ahead of c1) */
//  { "c31",       31 },     ***** reserved *****
//  { "c30",       30 },     ***** reserved *****
  { "c29",       29, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "c28",       28, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "c27",       27, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "c26",       26, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "c25",       25, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "c24",       24, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
//  { "c23",       23 },     ***** reserved *****
//  { "c22",       22 },     ***** reserved *****
//  { "c21",       21 },     ***** reserved *****
//  { "c20",       20 },     ***** reserved *****
//  { "c19",       19 },     ***** reserved *****
//  { "c18",       18 },     ***** reserved *****
//  { "c17",       17 },     ***** reserved *****
//  { "c16",       16 },     ***** reserved *****
//  { "c15",       15 },     ***** reserved *****
//  { "c14",       14 },     ***** reserved *****
  { "c13",       13, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c12",       12, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c11",       11, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c10",       10, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c9",         9, QDSP6_V2_AND_UP | QDSP6_REG_IS_READONLY},
  { "c8",         8, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c7",         7, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c6",         6, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
//  { "c5",         5 },     ***** reserved *****
  { "c4",         4, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c3",         3, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c2",         2, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c1",         1, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
  { "c0",         0, QDSP6_V2_AND_UP | QDSP6_REG_IS_READWRITE},
};

static const size_t qdsp6_control_regs_count =
  sizeof (qdsp6_control_regs) / sizeof (qdsp6_control_regs [0]);

static const qdsp6_reg qdsp6_guest_regs [] =
{
  { "gpcyclelo",  24, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "gpcyclehi",  25, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "gpmucnt0",   26, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "gpmucnt1",   27, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "gpmucnt2",   28, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "gpmucnt3",   29, QDSP6_V4_AND_UP | QDSP6_REG_IS_READONLY},
  { "g3",          3, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  { "g2",          2, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  { "g1",          1, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
  { "g0",          0, QDSP6_V4_AND_UP | QDSP6_REG_IS_READWRITE},
};

static const size_t qdsp6_guest_regs_count =
  sizeof (qdsp6_guest_regs) / sizeof (qdsp6_guest_regs [0]);

/** Create an extended value.

An extended value is made up by the extra bits required by immediate extension
"insn".

@param value The input value and output for its fixed up result.
@return The extended value.
*/
long
qdsp6_extend
(long *value, unsigned bits, int is_signed)
{
  long xvalue = 0;

  if (bits < 6)
    return (0);

  if (qdsp6_if_arch_kext ())
    {
      /* TODO: It would be better to use the insn table to figure out the number of bits. */
      xvalue = *value &  (~0L << 6);
      *value = *value & ~(~0L << 6);

      if (is_signed && *value > (1L << (bits - 1)))
        *value -= (1L << bits);
    }

  return (xvalue);
}

/** Create a hash for the opcode.

The generic scheme is to use the first letter of the operation as the hash,
with some exceptions for register transfers and other specific cases.

All in all, it could be improved to result in a more balanced depth.

@param syntax The forming syntax of the opcode.
*/
qdsp6_hash
qdsp6_hash_opcode
(const char *syntax)
{
  const char *p1, *p2, *pair, *peq;
  size_t token;
  int is_if;
  char ch;

  if (!strcmp (syntax, "Rd8 = #s6 ; Re8 = #S6")
      || !strcmp (syntax, "Rd8 = #s6 ; Re8 = aslh (Rs8 )"))
    {
      p1 = NULL;
      p2 = p1;
    }

  pair = strchr (syntax, PACKET_PAIR);
  peq  = strchr (syntax, '=');

  token = strcspn (syntax, " \t`~!@#$%^&*()_-=+[{]}\\|;:'\",<.>/?"); /* BLANK || PUNCT */
  is_if = !strncmp (syntax, "if", token);

#if 0
  if ((p1 = strncmp (syntax, "mem", 3)))
    {
      /* Loads and stores. */
      if (peq)
        /* Conditional and unconditional stores. */
        return (pif? (pnew? QDSP6_HASH_A: QDSP6_HASH_B): QDSP6_HASH_C);
      else
        /* Conditional and unconditional loads. */
        return (pif? (pnew? QDSP6_HASH_D: QDSP6_HASH_E): QDSP6_HASH_F);
    }
#endif

  if (pair)
    {
      p1 = pair + 1;
      while (ISSPACE (*p1))
        p1++;

      if (!strncmp (p1, "jump", 4) || !strncmp (syntax, "jump", 4)
          || !strncmp (p1, "call", 4) || !strncmp (syntax, "call", 4)
          || (qdsp6_if_arch_v4 ()
              && (!strncmp (p1, "return", token) || !strncmp (syntax, "return", token))))
        /* Conditional and unconditioonal branches. */
        return (is_if? QDSP6_HASH_G: QDSP6_HASH_H);
    }
  else
    {
      p1 = syntax;

      if (!strncmp (p1, "jump", 4)
          || !strncmp (p1, "call", 4)
          || (qdsp6_if_arch_v4 ()
              && (!strncmp (p1, "return", token))))
        /* Conditional and unconditioonal branches. */
        return (is_if? QDSP6_HASH_I: QDSP6_HASH_J);
    }

  if (peq)
    {
      /* Assignments. */
      p1 = peq + 1;
      while (ISSPACE (*p1))
        p1++;

      if (*p1 != '#' && (p2 = strchr (p1, '(')))
        {
          /* Operations. */
          while (ISSPACE (*p1) || ISPUNCT (*p1))
            p1++;

          if (ISALPHA (ch = TOLOWER (*p1)))
            {
              if (ch == 'v')
                {
                  /* Vector insns use the operation initial instead of the prefix. */
                  ch = TOLOWER (p1 [1]);

                  return (pair
                          ? (QDSP6_HASH_2 + ch - 'a' + (is_if? QDSP6_HASH_1: 0)) /* Paired insns. */
                          : (QDSP6_HASH_4 + ch - 'a' + (is_if? QDSP6_HASH_1: 0)) /* Vector insns. */ );
                }
              else
                return (pair
                        ? (QDSP6_HASH_2 + ch - 'a' + (is_if? QDSP6_HASH_1: 0)) /* Paired insns. */
                        : (QDSP6_HASH_0 + ch - 'a' + (is_if? QDSP6_HASH_1: 0)) /* Single insns. */ );
            }
        }
      else
        /* Conditional and unconditional transfers. */
        return (pair
                ? is_if? QDSP6_HASH_K: QDSP6_HASH_L
                : is_if? QDSP6_HASH_M: QDSP6_HASH_N);
    }

  if (!strncmp (syntax, "allocframe", 10)
      || !strncmp (syntax, "deallocframe", token)
      || !strncmp (syntax, "nop", token))
    return (pair? QDSP6_HASH_O: QDSP6_HASH_P);

  /* Miscellanea. */
  return (QDSP6_HASH_Z);
}

/* Convert the encoding string to an instruction (bit pattern) */

qdsp6_insn
qdsp6_encode_opcode
(const char *enc)
{
  qdsp6_insn insn;
  int num_bits;
  int pair;

  for (insn = 0, num_bits = 0, pair = 0; *enc; enc++)
    {
      while (ISSPACE (*enc))
        enc++;

      if (!*enc)
        break;

      insn = (insn << 1) + ((*enc == '1')? 1: 0);

      num_bits++;

      pair |= ((enc [0] == 'E' && enc [1] == 'E'));
    }

  if (num_bits != QDSP6_INSN_LEN * 8)
    {
      printf ("invalid number of bits: %d\n", num_bits);
      assert (FALSE);
    }

  return (QDSP6_END_PACKET_SET (insn, pair? QDSP6_END_PAIR: QDSP6_END_NOT));
}

qdsp6_hash
qdsp6_hash_icode
(qdsp6_insn insn)
{
  const qdsp6_hash xtra = ~(-(QDSP6_END_PACKET_MASK >> QDSP6_END_PACKET_POS));
  qdsp6_hash hash;

  hash  = insn >> (QDSP6_INSN_LEN * 8 - (qdsp6_icode_hash_bits - xtra));
  hash |= (QDSP6_END_PACKET_GET (insn) >> QDSP6_END_PACKET_POS)
          << (qdsp6_icode_hash_bits - xtra);

  return (hash);
}

/* Configuration flags.  */

/* Various QDSP6_HAVE_XXX bits.  */
#define Q6_CPU_TYPE_UNINIT (~0 << sizeof (cpu_type))
static int cpu_type;
static int cpu_flag;

/* Return nonzero if we are assembling for V1 */
int
qdsp6_if_arch
(int arch)
{
  return (cpu_type == arch);
}

/** Query support for immediate extender by the current architecture.

@return True if supported.
*/
int
qdsp6_if_arch_kext
(void)
{
  return (qdsp6_if_arch_v4 ());
}

/** Query support for non-GPR pairs.

@return True if supported.
*/
int
qdsp6_if_arch_pairs
(void)
{
  return (qdsp6_if_arch_v4 ());
}

/** Query support for auto-anding of multiple predicate changes
    by the current architecture.

@return True if supported.
*/
int
qdsp6_if_arch_autoand
(void)
{
  return (qdsp6_if_arch_v4 ());
}

/* Translate a bfd_mach_qdsp6_xxx value to a QDSP6_MACHXX value.  */
int
qdsp6_get_opcode_mach
(int bfd_mach, int big_p)
{
  static int mach_type_map [] =
  {
    QDSP6_MACH_V2,
    QDSP6_MACH_V3,
    QDSP6_MACH_V4,
    /* Leaving space for future cores */
  };

  // RK: Handle cases when bfd_mach is not correctly set
  if (bfd_mach < bfd_mach_qdsp6_v2 || bfd_mach > bfd_mach_qdsp6_v4)
    return Q6_CPU_TYPE_UNINIT;

  return (mach_type_map [bfd_mach - bfd_mach_qdsp6_v2]
          | (big_p? QDSP6_MACH_BIG: 0));
}

/* Initialize any tables that need it.
   Must be called once at start up (or when first needed).

   FLAGS is a set of bits that say what version of the cpu we have,
   and in particular at least (one of) QDSP6_MACHXX.  */

void
qdsp6_opcode_init_tables
(int cpu)
{
  size_t i;

  if (cpu_type == cpu)
    return;

  cpu_type = cpu;
  if (cpu_type == Q6_CPU_TYPE_UNINIT)
    return;

  cpu_flag = qdsp6_if_arch_v2 ()? QDSP6_IS_V2
           : qdsp6_if_arch_v3 ()? QDSP6_IS_V3
           : qdsp6_if_arch_v4 ()? QDSP6_IS_V4
           : 0;

  /* We may be intentionally called more than once (for example gdb will call
     us each time the user switches cpu).  These tables only need to be init'd
     once though.  */

  /* Based on cpu type, set: operand, operand_array_size */
  qdsp6_opcodes               = qdsp6_if_arch_v2 ()? qdsp6_opcodes_v2
                                : qdsp6_if_arch_v3 ()? qdsp6_opcodes_v3
                                                     : qdsp6_opcodes_v4;
  qdsp6_opcodes_count         = qdsp6_if_arch_v2 ()? qdsp6_opcodes_count_v2
                                : qdsp6_if_arch_v3 ()? qdsp6_opcodes_count_v3
                                                     : qdsp6_opcodes_count_v4;

  /* Scan the opcodes table to determine the number of bits
    that can reliably be used to hash opcodes */
  qdsp6_icode_hash_bits = QDSP6_INSN_LEN * 8;
  for (i = 0; i < qdsp6_opcodes_count; i++)
    {
      unsigned int bits;
      const char *str;

      for (bits = 0, str = qdsp6_opcodes [i].enc; *str; str++)
        {
          if (*str == '0' || *str == '1' || *str == 'P' || *str == 'E')
            bits++;
          else
            break;
        }

      if (bits < qdsp6_icode_hash_bits)
        qdsp6_icode_hash_bits = bits;
    }
  qdsp6_icode_hash_bits += ~(-(QDSP6_END_PACKET_MASK >> QDSP6_END_PACKET_POS));

  /* Initialize hash maps. */
  memset (opcode_map, 0, sizeof (opcode_map));
  icode_map = xcalloc (1 << qdsp6_icode_hash_bits, sizeof (*icode_map));

  /* Scan the table backwards so macros appear at the front.  */
  for (i = qdsp6_opcodes_count; i > 0; --i)
    {
      qdsp6_hash opcode;
      qdsp6_insn insn;
      qdsp6_hash icode;

      opcode = QDSP6_HASH_OPCODE (qdsp6_opcodes [i - 1].syntax);
      insn   = qdsp6_encode_opcode (qdsp6_opcodes [i - 1].enc);
      icode  = QDSP6_HASH_ICODE (insn);

      qdsp6_opcodes [i - 1].next_asm = opcode_map [opcode];
      opcode_map [opcode] = qdsp6_opcodes + i - 1;

      qdsp6_opcodes [i - 1].next_dis = icode_map [icode];
      icode_map [icode] = qdsp6_opcodes + i - 1;

      if ((qdsp6_opcodes [i - 1].attributes & A_IT_NOP)
          || !strcmp (qdsp6_opcodes [i - 1].syntax, "nop"))
        {
          qdsp6_opcodes [i - 1].attributes |= A_IT_NOP;
          qdsp6_nop = insn;
        }

      if ((qdsp6_opcodes [i - 1].attributes & A_IT_EXTENDER))
        {
          qdsp6_kext = insn;
          qdsp6_opcodes [i - 1].attributes |= PREFIX;
        }

      if (QDSP6_END_PACKET_GET (insn) == QDSP6_END_PAIR)
        qdsp6_opcodes [i - 1].attributes |= DUPLEX;
    }

#if 0
    puts ("Hash Table Depth:");
    for (i = 0; i < QDSP6_HASH_SIZE; i++)
      {
        qdsp6_opcode *op;
        size_t j;

        for (j = 0, op = opcode_map [i]; op; j++)
          op = QDSP6_OPCODE_NEXT_ASM (op);

        printf ("#%02ld ('%c'): %02ld\t", i, (i % QDSP6_HASH_1) + 'a', j);
      }
    puts ("");

    puts ("Hash Table Syntax:");
    for (i = 0; i < QDSP6_HASH_SIZE; i++)
      {
        qdsp6_opcode *op;
        size_t j;

        printf ("#%02ld ('%c')\n", i, (i % QDSP6_HASH_1) + 'a');

        for (j = 0, op = opcode_map [i]; op; j++, op = QDSP6_OPCODE_NEXT_ASM (op))
          puts (op->syntax);

        puts ("");
      }
#endif
}

/* Return non-zero if OPCODE is supported on the specified cpu.
   Cpu selection is made when calling `qdsp6_opcode_init_tables'.  */

int
qdsp6_opcode_supported
(const qdsp6_opcode *opcode)
{
  return (QDSP6_OPCODE_CPU (opcode->flags) <= cpu_type);
}

/* Return the first insn in the chain for assembling INSN.  */

const qdsp6_opcode *
qdsp6_opcode_lookup_asm
(const char *insn)
{
  return (opcode_map [QDSP6_HASH_OPCODE (insn)]);
}

/* Return the first insn in the chain for disassembling INSN.  */

const qdsp6_opcode *
qdsp6_opcode_lookup_dis
(qdsp6_insn insn)
{
  return (icode_map [QDSP6_HASH_ICODE (insn)]);
}

const qdsp6_opcode *
qdsp6_lookup_insn
(qdsp6_insn insn)
{
  const qdsp6_opcode *opcode;
  qdsp6_insn encode;
  qdsp6_insn mask;

  /* Reset the packet bits. */
  insn = QDSP6_END_PACKET_RESET (insn);

  for (opcode = qdsp6_opcode_lookup_dis (insn);
       opcode;
       opcode = QDSP6_OPCODE_NEXT_DIS (opcode))
    {
      if (opcode->map)
        continue;

      encode = qdsp6_encode_opcode (opcode->enc);
      mask   = qdsp6_encode_mask   (opcode->enc);

      if ((insn & mask) == (encode & mask))
        return (opcode);
    }

  return NULL;
}

/** Encode the operand in its insn bits.

@param operand The operand.
@param insn The insn.
@param enc The encoding string.
@param value Value to encode.
@param xvalue Optional request for an extended value and its output.
@param is_x Request for an extended value.
@param errmsg Optional error message.

@return true if successful.
*/

int
qdsp6_encode_operand
(const qdsp6_operand *operand, qdsp6_insn *insn,
 const qdsp6_opcode *opcode, long avalue, long *xvalue, int is_x, int is_rel,
 char **errmsg)
{
  char enc_letter;
  int shift_count = 0;
  int num_bits = 0;
  size_t len = strlen (opcode->enc);
  unsigned bits;
  long smin, smax;
  unsigned long umin, umax;
  long xer, xed;
  union
    {
      long s;
      unsigned long u;
    } value;
  int is_s;
  ptrdiff_t i;
  static char buf [500];

  value.s = avalue;

  is_x = is_x && xvalue;
  is_s = (operand->flags & QDSP6_OPERAND_IS_SIGNED);

  enc_letter = operand->enc_letter;

  /* TODO: there are issues with this... */
  if (FALSE && is_rel)
    switch (opcode->attributes
            & (EXTENDABLE_LOWER_CASE_IMMEDIATE | EXTENDABLE_UPPER_CASE_IMMEDIATE))
      {
      case EXTENDABLE_LOWER_CASE_IMMEDIATE:
        enc_letter = TOLOWER (enc_letter);
        break;

      case EXTENDABLE_UPPER_CASE_IMMEDIATE:
        enc_letter = TOUPPER (enc_letter);
        break;
      }

  if (operand->shift_count)
    {
      /* Make sure the low bits are zero */
      if (value.s & (~(~0L << operand->shift_count)))
        {
          if (errmsg)
            {
              sprintf (buf, "low %d bits of immediate %ld must be zero",
                       operand->shift_count, value.s);
              *errmsg = buf;
            }
          return FALSE;
        }
    }

  if (operand->flags & QDSP6_OPERAND_IS_LO16)
    value.s = QDSP6_LO16 (value.s);
  else if (operand->flags & QDSP6_OPERAND_IS_HI16)
    value.s = QDSP6_HI16 (value.s);
  else if (operand->flags & QDSP6_OPERAND_IS_SUBSET)
    value.s = QDSP6_SUBREGS_TO (value.s, operand->flags & QDSP6_OPERAND_IS_PAIR);

  /* Make sure the value is within the proper range
     Must include the shift count */
  bits = operand->bits + operand->shift_count;
  smax = ~(~0L << (bits - 1));
  smin =  (~0L << (bits - 1)) + ((operand->flags & QDSP6_OPERAND_IS_NEGATIVE)? 1: 0);
  umax = ~(~0UL << bits);
  umin = 0UL;

  xed = value.s;
  xer = qdsp6_extend (&xed, bits, is_s);

  if (is_x)
    {
      is_s = (xed < 0);

      *xvalue = xer;
      value.s = xed;
    }

  if (is_s)
    {
      if (value.s < smin || (smax > 0 && value.s > smax))
        {
          if (errmsg)
            {
              sprintf (buf, "value %ld out of range: %ld-%ld", value.s, smin, smax);
              *errmsg = buf;
            }
          return FALSE;
        }
    }
  else
    {
      if (value.u < umin || (umax > 0 && value.u > umax))
        {
          if (errmsg)
            {
              sprintf (buf, "value %lu out of range: %lu-%lu", value.u, umin, umax);
              *errmsg = buf;
            }
          return FALSE;
        }
    }

  /* In the presence of an extender, the value is not shifted. */
  if (!is_x)
    value.s >>= operand->shift_count;

  /* We'll read the encoding string backwards
     and put the LSB of the value in each time */
  for (i = len - 1; i >= 0; i--)
    if (!ISSPACE (opcode->enc [i]))
      {
        if (opcode->enc [i] == enc_letter)
          {
            // Clear the bit
            (*insn) &= ~(1 << shift_count);
            // Insert the new bit
            (*insn) |= (value.s & 1) << shift_count;
            value.s >>= 1;
            num_bits++;
          }
        shift_count++;
      }

  /* Make sure we encode the expected number of bits */
  if (num_bits != operand->bits)
    {
      if (errmsg)
        {
          sprintf (buf, "did not encode expected number of bits: %d != %d\n"
                   "\tenc = %s\n\toperand = %s",
                   num_bits, operand->bits, opcode->enc, operand->fmt);
          *errmsg = buf;
        }
      return FALSE;
    }

  return TRUE;
}

static int
qdsp6_reg_num
(const qdsp6_operand *operand ATTRIBUTE_UNUSED, char **input, char reg,
 const qdsp6_reg alias [], size_t count, size_t *aliasn, char prefix)
{
  unsigned long regn;
  char *regs;
  size_t i;

  regs = *input;

  if (TOLOWER (regs [0]) == TOLOWER (reg) && ISDIGIT (regs [1]))
    {
      /* Skip the letter */
      regs++;

      regn = strtoul (regs, input, 10);
      if (regn != ULONG_MAX && regs != *input && !ISALPHA (**input))
        return ((int) regn);
    }
  else
    {
      char name [4];

      if (*regs == ':')
        /* Skip the register pair separator. */
        regs++;

      regn = strtoul (regs, NULL, 10);
      if ((int) sizeof (name)
          <= snprintf (name, sizeof (name), "%c%ld", prefix, regn))
        *name = 0;

      /* Find an alias for the register.
         TODO: this code is crap. */
      for (i = 0; i < count; i++)
        {
          size_t len;

          if (!(alias [i].flags & cpu_flag))
            continue;

          len = strlen (alias [i].name);

          if (!strncasecmp (regs, alias [i].name, len)
              || !strncasecmp (name, alias [i].name, len))
            {
              regn = alias [i].reg_num;

              if (ISALNUM (regs [len - (*name? 1: 0)])) /* e.g. "call splice" should not match "sp" */
                continue;

              *input = regs + len - (*name? 1: 0);
              return ((int) (aliasn? *aliasn = i: i), (int) regn);
            }
        }
    }

  return (-1);
}

static char *
qdsp6_parse_dreg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg_odd;
  int reg_even;

  reg_odd  = qdsp6_reg_num (operand, &input, 'r', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  reg_even = qdsp6_reg_num (operand, &input, ':', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  if (reg_odd < 0 || reg_even < 0)
    return (NULL);

  /* Make sure the register number is odd */
  if (reg_odd % 2 != 1)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "invalid odd register number: %d", reg_odd);
            *errmsg = buf;
        }
      return NULL;
    }

  /* Make sure the register number is even */
  if (reg_even % 2 != 0)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "invalid even register number: %d", reg_even);
            *errmsg = buf;
        }
      return NULL;
    }

  /* Make sure the registers are consecutive */
  if (reg_odd != reg_even + 1)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "registers must be consecutive: %d:%d",
                      reg_odd, reg_even);
            *errmsg = buf;
        }
      return NULL;
    }

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg_even, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_PAIR;

      *val = reg_even;
      return (input);
    }

  return NULL;
}

static char *
qdsp6_parse_dreg16
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg_odd;
  int reg_even;

  reg_odd  = qdsp6_reg_num (operand, &input, 'r', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  reg_even = qdsp6_reg_num (operand, &input, ':', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  if (reg_odd < 0 || reg_even < 0)
    return (NULL);

  /* Make sure the register number is odd */
  if (reg_odd % 2 != 1)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "invalid odd register number: %d", reg_odd);
            *errmsg = buf;
        }
      return (NULL);
    }

  /* Make sure the register number is even */
  if (reg_even % 2 != 0)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "invalid even register number: %d", reg_even);
            *errmsg = buf;
        }
      return (NULL);
    }

  /* Make sure the registers are consecutive */
  if (reg_odd != reg_even + 1)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "registers must be consecutive: %d:%d",
                      reg_odd, reg_even);
            *errmsg = buf;
        }
      return (NULL);
    }

  if (((reg_odd  < 1 || reg_odd  > 31))
      || ((reg_even < 0 || reg_even > 30)))
    return (NULL);

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg_even, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_PAIR;

      *val = reg_even;
      return (input);
    }

  return (NULL);
}

static char *
qdsp6_parse_dreg8
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg_odd;
  int reg_even;

  reg_odd  = qdsp6_reg_num (operand, &input, 'r', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  reg_even = qdsp6_reg_num (operand, &input, ':', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  if (reg_odd < 0 || reg_even < 0)
    return (NULL);

  /* Make sure the register number is odd */
  if (reg_odd % 2 != 1)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "invalid odd register number: %d", reg_odd);
            *errmsg = buf;
        }
      return (NULL);
    }

  /* Make sure the register number is even */
  if (reg_even % 2 != 0)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "invalid even register number: %d", reg_even);
            *errmsg = buf;
        }
      return (NULL);
    }

  /* Make sure the registers are consecutive */
  if (reg_odd != reg_even + 1)
    {
      if (errmsg)
        {
            static char buf [100];
            sprintf (buf, "registers must be consecutive: %d:%d",
                      reg_odd, reg_even);
            *errmsg = buf;
        }
      return (NULL);
    }

  if (((reg_odd <  1 || reg_odd > 7)
          && (reg_odd < 17 || reg_odd > 23))
      || ((reg_even > 6)
          && (reg_even < 16 || reg_even > 22)))
    return (NULL);

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg_even, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_PAIR;

      *val = reg_even;
      return (input);
    }

  return (NULL);
}

char *
qdsp6_parse_reg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg;

  reg = qdsp6_reg_num (operand, &input, 'r', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  if (reg < 0)
    return (NULL);

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_REGISTER;

      *val = reg;
      return (input);
    }

  return (NULL);
}

char *
qdsp6_parse_reg16
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg;

  reg = qdsp6_reg_num (operand, &input, 'r', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  if (reg < 0)
    return (NULL);

  if ((reg > 7) && (reg < 16 || reg > 23))
    return (NULL);

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_REGISTER;

      *val = reg;
      return (input);
    }

  return (NULL);
}

char *
qdsp6_parse_reg8
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg;

  reg = qdsp6_reg_num (operand, &input, 'r', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  if (reg < 0)
    return (NULL);

  if ((reg > 3) && (reg < 16 || reg > 19))
    return (NULL);

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_REGISTER;

      *val = reg;
      return (input);
    }

  return (NULL);
}

char *
qdsp6_parse_reg2
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg;

  reg = qdsp6_reg_num (operand, &input, 'r', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  if (reg < 0 || reg > 31)
    return (NULL);

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg % 2, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_RNEW;

      *val = reg;
      return (input);
    }

  return (NULL);
}

char *
qdsp6_parse_preg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg;

  reg = qdsp6_reg_num (operand, &input, 'p', qdsp6_gp_regs, qdsp6_gp_regs_count, NULL, '\0');
  if (reg < 0)
    return (NULL);

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_PREDICATE;

      *val = reg;
      return (input);
    }

  return (NULL);
}

char *
qdsp6_parse_mreg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  int reg;

  reg = qdsp6_reg_num (operand, &input, 'm', NULL, 0, NULL, '\0');
  if (reg < 0)
    return (NULL);

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag = QDSP6_OPERAND_IS_MODIFIER;

      *val = reg;
      return (input);
    }

  return (NULL);
}

static char *
qdsp6_parse_creg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  size_t regn;
  int reg;
  static char buf [150];

  reg = qdsp6_reg_num (operand, &input, '\0', qdsp6_control_regs, qdsp6_control_regs_count, &regn, '\0');
  if (reg < 0)
    return (NULL);

  if (flag)
    *flag = 0;

  if (!qdsp6_verify_hw)
    if ((operand->flags & QDSP6_OPERAND_IS_WRITE)
        && (qdsp6_control_regs [regn].flags & QDSP6_REG_IS_READONLY))
      {
        if (errmsg)
          {
            sprintf (buf, "cannot write to read-only register `%s'.",
                      qdsp6_control_regs [regn].name);
            *errmsg = buf;
          }
        if (flag)
          *flag |= QDSP6_OPERAND_IS_INVALID;
      }

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag |= QDSP6_OPERAND_IS_CONTROL;

      *val = reg;
      return (input);
    }

  return (NULL);
}

static char *
qdsp6_parse_dcreg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  static char buf [100];
  size_t rege, rego;
  int reg_odd;
  int reg_even;

  reg_odd  = qdsp6_reg_num (operand, &input, '\0', qdsp6_control_regs, qdsp6_control_regs_count, &rego, '\0');
  reg_even = qdsp6_reg_num (operand, &input, '\0', qdsp6_control_regs, qdsp6_control_regs_count, &rege, 'c');
  if (reg_odd < 0 || reg_even < 0)
    return (NULL);

  /* Make sure the register number is odd */
  if (reg_odd % 2 != 1)
    {
      if (errmsg)
        {
            sprintf (buf, "invalid odd register number: %d", reg_odd);
            *errmsg = buf;
        }
      return NULL;
    }

  /* Make sure the register number is even */
  if (reg_even % 2 != 0)
    {
      if (errmsg)
        {
            sprintf (buf, "invalid even register number: %d", reg_even);
            *errmsg = buf;
        }
      return NULL;
    }

  /* Make sure the registers are consecutive */
  if (reg_odd != reg_even + 1)
    {
      if (errmsg)
        {
            sprintf (buf, "registers must be consecutive: %d:%d",
                     reg_odd, reg_even);
            *errmsg = buf;
        }
      return NULL;
    }

  if (flag)
    *flag = 0;

  if (!qdsp6_verify_hw)
    if ((operand->flags & QDSP6_OPERAND_IS_WRITE)
        && ((qdsp6_control_regs [rege].flags & QDSP6_REG_IS_READONLY)
            || (qdsp6_control_regs [rego].flags & QDSP6_REG_IS_READONLY)))
      {
        if (errmsg)
          {
            sprintf (buf, "cannot write to read-only register `%s:%s'.",
                     qdsp6_control_regs [rego].name,
                     qdsp6_control_regs [rege].name);
            *errmsg = buf;
          }
        if (flag)
          *flag |= QDSP6_OPERAND_IS_INVALID;
      }

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg_even, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag |= QDSP6_OPERAND_IS_PAIR;

      *val = reg_even;
      return (input);
    }

  return NULL;
}

static char *
qdsp6_parse_greg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  size_t regn;
  int reg;
  static char buf [150];

  reg = qdsp6_reg_num (operand, &input, '\0', qdsp6_guest_regs, qdsp6_guest_regs_count, &regn, '\0');
  if (reg < 0)
    return (NULL);

  if (flag)
    *flag = 0;

  if (!qdsp6_verify_hw)
    if ((operand->flags & QDSP6_OPERAND_IS_WRITE)
        && (qdsp6_guest_regs [regn].flags & QDSP6_REG_IS_READONLY))
      {
        if (errmsg)
          {
            sprintf (buf, "cannot write to read-only register `%s'.",
                     qdsp6_guest_regs [regn].name);
            *errmsg = buf;
          }
        if (flag)
          *flag |= QDSP6_OPERAND_IS_INVALID;
      }

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag |= QDSP6_OPERAND_IS_CONTROL;

      *val = reg;
      return (input);
    }

  return (NULL);
}

static char *
qdsp6_parse_dgreg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  static char buf [100];
  size_t rege, rego;
  int reg_odd;
  int reg_even;

  reg_odd  = qdsp6_reg_num (operand, &input, '\0', qdsp6_guest_regs, qdsp6_guest_regs_count, &rego, '\0');
  reg_even = qdsp6_reg_num (operand, &input, '\0', qdsp6_guest_regs, qdsp6_guest_regs_count, &rege, 'g');
  if (reg_odd < 0 || reg_even < 0)
    return (NULL);

  /* Make sure the register number is odd */
  if (reg_odd % 2 != 1)
    {
      if (errmsg)
        {
            sprintf (buf, "invalid odd register number: %d", reg_odd);
            *errmsg = buf;
        }
      return NULL;
    }

  /* Make sure the register number is even */
  if (reg_even % 2 != 0)
    {
      if (errmsg)
        {
            sprintf (buf, "invalid even register number: %d", reg_even);
            *errmsg = buf;
        }
      return NULL;
    }

  /* Make sure the registers are consecutive */
  if (reg_odd != reg_even + 1)
    {
      if (errmsg)
        {
            sprintf (buf, "registers must be consecutive: %d:%d",
                     reg_odd, reg_even);
            *errmsg = buf;
        }
      return NULL;
    }

  if (flag)
    *flag = 0;

  if (!qdsp6_verify_hw)
    if ((operand->flags & QDSP6_OPERAND_IS_WRITE)
        && ((qdsp6_guest_regs [rege].flags & QDSP6_REG_IS_READONLY)
            || (qdsp6_guest_regs [rego].flags & QDSP6_REG_IS_READONLY)))
      {
        if (errmsg)
          {
            sprintf (buf, "cannot write to read-only register `%s:%s'.",
                     qdsp6_guest_regs [rego].name,
                     qdsp6_guest_regs [rege].name);
            *errmsg = buf;
          }
        if (flag)
          *flag |= QDSP6_OPERAND_IS_INVALID;
      }

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg_even, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag |= QDSP6_OPERAND_IS_PAIR;

      *val = reg_even;
      return (input);
    }

  return NULL;
}

static char *
qdsp6_parse_sreg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  size_t regn;
  int reg;
  static char buf [150];

  reg = qdsp6_reg_num (operand, &input, '\0', qdsp6_supervisor_regs, qdsp6_supervisor_regs_count, &regn, '\0');
  if (reg < 0)
    return (NULL);

  if (flag)
    *flag = 0;

  if (!qdsp6_verify_hw)
    if ((operand->flags & QDSP6_OPERAND_IS_WRITE)
        && (qdsp6_supervisor_regs [regn].flags & QDSP6_REG_IS_READONLY))
      {
        if (errmsg)
          {
            sprintf (buf, "cannot write to read-only register `%s'.",
                     qdsp6_supervisor_regs [regn].name);
            *errmsg = buf;
          }
        if (flag)
          *flag |= QDSP6_OPERAND_IS_INVALID;
      }

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag |= QDSP6_OPERAND_IS_SYSTEM;

      *val = reg;
      return (input);
    }

  return (NULL);
}

static char *
qdsp6_parse_dsreg
(const qdsp6_operand *operand, qdsp6_insn *insn, const qdsp6_opcode *opcode,
 char *input, long *val, int *flag, char **errmsg)
{
  static char buf [100];
  size_t rege, rego;
  int reg_odd;
  int reg_even;

  reg_odd  = qdsp6_reg_num (operand, &input, '\0', qdsp6_supervisor_regs, qdsp6_supervisor_regs_count, &rego, '\0');
  reg_even = qdsp6_reg_num (operand, &input, '\0', qdsp6_supervisor_regs, qdsp6_supervisor_regs_count, &rege, 's');
  if (reg_odd < 0 || reg_even < 0)
    return (NULL);

  /* Make sure the register number is odd */
  if (reg_odd % 2 != 1)
    {
      if (errmsg)
        {
            sprintf (buf, "invalid odd register number: %d", reg_odd);
            *errmsg = buf;
        }
      return NULL;
    }

  /* Make sure the register number is even */
  if (reg_even % 2 != 0)
    {
      if (errmsg)
        {
            sprintf (buf, "invalid even register number: %d", reg_even);
            *errmsg = buf;
        }
      return NULL;
    }

  /* Make sure the registers are consecutive */
  if (reg_odd != reg_even + 1)
    {
      if (errmsg)
        {
            sprintf (buf, "registers must be consecutive: %d:%d",
                     reg_odd, reg_even);
            *errmsg = buf;
        }
      return NULL;
    }

  if (flag)
    *flag = 0;

  if (!qdsp6_verify_hw)
    if ((operand->flags & QDSP6_OPERAND_IS_WRITE)
        && ((qdsp6_supervisor_regs [rege].flags & QDSP6_REG_IS_READONLY)
            || (qdsp6_supervisor_regs [rego].flags & QDSP6_REG_IS_READONLY)))
      {
        if (errmsg)
          {
            sprintf (buf, "cannot write to read-only register `%s:%s'.",
                     qdsp6_supervisor_regs [rego].name,
                     qdsp6_supervisor_regs [rege].name);
            *errmsg = buf;
          }
        if (flag)
          *flag |= QDSP6_OPERAND_IS_INVALID;
      }

  if (qdsp6_encode_operand
        (operand, insn, opcode, reg_even, NULL, FALSE, FALSE, errmsg))
    {
      if (flag)
        *flag |= QDSP6_OPERAND_IS_PAIR;

      *val = reg_even;
      return (input);
    }

  return NULL;
}

/** Produce a bit mask from the encoding string such that
    the bits that must be 1 or 0 are set to 1.
    Then
      insn & mask == qdsp6_encode_opcode (enc)
    if and only if insn matches the insn.
*/

qdsp6_insn
qdsp6_encode_mask
(char *enc)
{
  qdsp6_insn mask;
  int num_bits;

  for (mask = 0, num_bits = 0; *enc; enc++)
    {
      while (ISSPACE (*enc))
        enc++;

      if (!*enc)
        break;

      mask = (mask << 1) + ((*enc == '1' || *enc == '0')? 1: 0);

      num_bits++;
    }

  assert (num_bits == QDSP6_INSN_LEN * 8);
  return (mask);
}

const qdsp6_operand *
qdsp6_lookup_operand
(const char *name)
{
  size_t op_len, fm_len;
  size_t i;

  /* Get the length of the operand syntax up to a separator. */
  op_len = strcspn (name, " .");

  for (i = 0; i < qdsp6_operand_count; i++)
    {
      fm_len = strlen  (qdsp6_operands [i].fmt);

      if (op_len == fm_len && !strncmp (name, qdsp6_operands [i].fmt, op_len))
        return (qdsp6_operands + i);
    }

  return NULL;
}

const qdsp6_operand *
qdsp6_lookup_reloc
(bfd_reloc_code_real_type reloc_type, int flags, const qdsp6_opcode *opcode)
{
  bfd_reloc_code_real_type r;
  size_t i;

  if (reloc_type == BFD_RELOC_NONE || !opcode)
    return NULL;

  for (i = 0; i < qdsp6_operand_count; i++)
    {
      if (flags & QDSP6_OPERAND_IS_KXED)
        {
          if (qdsp6_operands [i].flags & QDSP6_OPERAND_IS_IMMEDIATE)
            switch (opcode->attributes
                    & (EXTENDABLE_LOWER_CASE_IMMEDIATE
                       | EXTENDABLE_UPPER_CASE_IMMEDIATE))
              {
              case EXTENDABLE_LOWER_CASE_IMMEDIATE:
                if (qdsp6_operands [i].enc_letter
                    != TOLOWER (qdsp6_operands [i].enc_letter)
                    || !strstr (opcode->syntax, qdsp6_operands [i].fmt))
                  continue;

                break;

              case EXTENDABLE_UPPER_CASE_IMMEDIATE:
                if (qdsp6_operands [i].enc_letter
                    != TOUPPER (qdsp6_operands [i].enc_letter)
                    || !strstr (opcode->syntax, qdsp6_operands [i].fmt))
                  continue;

                break;
              }

          r = qdsp6_operands [i].reloc_kxed;
        }
      else
        r = qdsp6_operands [i].reloc_type;

      if (r == reloc_type)
        return (qdsp6_operands + i);
    }

  return NULL;
}

int
qdsp6_extract_operand
(const qdsp6_operand *operand, qdsp6_insn insn,
 bfd_vma addr, char *enc, int *return_value,
 char **errmsg)
{
  int value = 0;
  char enc_letter = operand->enc_letter;
  char *enc_ptr = enc;
  unsigned int bits_found;
  qdsp6_insn mask;

  if (QDSP6_IS16BITS(insn))
    mask = 1 << 15;
  else
    mask = 1 << 31;

  /* Grab the bits from the instruction */
  bits_found = 0;
  while (*enc_ptr)
    {
      char ch = *enc_ptr++;

      if (!ISSPACE(ch))
        {
          if (ch == enc_letter)
            {
              value = (value << 1) + ((insn & mask)? 1: 0);
              bits_found++;
            }
          mask >>= 1;
        }
    }

  /* Check that we got the right number of bits */
  if (bits_found != operand->bits)
    {
      if (errmsg)
        {
          static char xx[100];

          sprintf (xx, "operand %c wrong number of bits found in %s, %d != %d",
                   operand->enc_letter, enc, bits_found, operand->bits);
          *errmsg = xx;
        }
    return FALSE;
  }

  if (operand->flags & QDSP6_OPERAND_IS_SIGNED)
    {
      /* Might need to sign extend */
      if (value & (1 << (bits_found - 1)))
        {
          value <<= 32 - bits_found;
          value >>= 32 - bits_found;
        }
    }

  value <<= operand->shift_count;

  if (operand->flags & QDSP6_OPERAND_PC_RELATIVE)
    value += addr;

  if (operand->flags & QDSP6_OPERAND_IS_SUBSET)
    value = QDSP6_SUBREGS_FROM (value, operand->flags & QDSP6_OPERAND_IS_PAIR);

  *return_value = value;
  return TRUE;
}

static char *
qdsp6_dis_named_reg
(int value, int count, const qdsp6_reg *regs, char *buf, char **errmsg)
{
  int n = 0;
  int found = 0;
  int i;

  for (i = 0; i < count; i++)
    {
      if (!(regs [i].flags & cpu_flag))
        continue;

      if (value == regs [i].reg_num)
        {
          found = 1;
          n = sprintf (buf, "%s", regs [i].name);
          break;
        }
    }

  if (!found)
    {
      if (errmsg)
        {
          static char xx [100];
          sprintf(xx, "control register not found: %d", value);
          *errmsg = xx;
        }
      return NULL;
    }

  return (buf + n);
}

char *
qdsp6_dis_operand
(const qdsp6_operand *operand, qdsp6_insn insn, bfd_vma addr,
 char *enc, char *buf, char **errmsg)
{
  static int xer, xvalue;
  int xed, value;
  int n;

  if (!qdsp6_extract_operand (operand, insn, addr, enc, &value, errmsg))
    return NULL;

  xed = FALSE;

  if (operand->flags & QDSP6_OPERAND_IS_KXER)
    {
      xer = TRUE;
      xvalue = value;
    }
  else if (xer && operand->flags & QDSP6_OPERAND_IS_KXED)
    {
      if (operand->flags & QDSP6_OPERAND_PC_RELATIVE)
        {
          xed = FALSE;

          value  -= addr;
          value >>= operand->shift_count;
          value  += xvalue + addr;
        }
      else
        {
          xed = TRUE;

          value >>= operand->shift_count;
          value  += xvalue;
        }

      xer = xvalue = 0;
    }

  if (operand->flags & QDSP6_OPERAND_IS_REGISTER)
    n = sprintf (buf, operand->dis_fmt, value);
  if (operand->flags & QDSP6_OPERAND_IS_PAIR)
    n = sprintf (buf, operand->dis_fmt, value + 1, value);
  else if (operand->flags & QDSP6_OPERAND_IS_CONTROL)
    return (qdsp6_dis_named_reg (value,
                                 qdsp6_control_regs_count, qdsp6_control_regs,
                                 buf, errmsg));
  else if (operand->flags & QDSP6_OPERAND_IS_GUEST)
    return (qdsp6_dis_named_reg (value,
                                 qdsp6_guest_regs_count, qdsp6_guest_regs,
                                 buf, errmsg));
  else if (operand->flags & QDSP6_OPERAND_IS_SYSTEM)
    return (qdsp6_dis_named_reg (value,
                                 qdsp6_supervisor_regs_count, qdsp6_supervisor_regs,
                                 buf, errmsg));
  else
    {
      n = 0;
      if (xed)
        buf [n++] = '#';
      n += sprintf (buf + n, operand->dis_fmt, value);
    }

  if (n > 0)
    return (buf + n);
  else
    {
      if (errmsg)
        {
          static char xx [100];
          sprintf (xx, "bad return code from sprintf: %d", n);
          *errmsg = xx;
        }
      return NULL;
    }
}

int
qdsp6_dis_opcode
(char *buf, qdsp6_insn insn, bfd_vma address,
 const qdsp6_opcode *opcode, char **errmsg)
{
  static char temp [40];
  static int in_packet;
  static int end_inner;
  static int end_outer;
  static bfd_vma packet_addr;
  qdsp6_insn packet_bits = QDSP6_END_PACKET_GET (insn);
  int end_packet = FALSE;
  char *src = opcode->syntax;
  char *dst = buf;

  switch (packet_bits)
    {
      case QDSP6_END_PACKET:
      case QDSP6_END_PAIR:
        if (in_packet)
          dst += sprintf (dst, "  ");
        end_packet = TRUE;
        break;

      case QDSP6_END_LOOP:
      case QDSP6_END_NOT:
        if (!in_packet)
          {
            dst += sprintf (dst, "%c ", PACKET_BEGIN);
            packet_addr = address;
          }
        else
          dst += sprintf (dst, "  ");

        ++in_packet;

        if (packet_bits == QDSP6_END_LOOP)
          {
            if (in_packet == 1) end_inner = TRUE;
            if (in_packet == 2) end_outer = TRUE;
          }
        break;

      default:
        if (errmsg)
          {
            snprintf (temp, sizeof (temp), "invalid packet bits: %.8x", packet_bits);
            *errmsg = temp;
          }
        return FALSE;
        break;
    }

  /* PC relative operands are based on the address of
     the first instruction in the packet */
  if (in_packet)
    address = packet_addr;

  while (*src)
    {
      /* EJP: ignore +I */
      if ((ISUPPER(*src) && (TOUPPER(*src) != 'I')) || *src == '#')
        {
          /* We have an operand */
          int found = FALSE;
          size_t i;

          for (i = 0; i < qdsp6_operand_count; i++)
            {
              qdsp6_operand operand;
              size_t len;

              operand = qdsp6_operands [i];
              len = strlen (operand.fmt);

              if (!strncmp (src, operand.fmt, len))
                {
                  found = TRUE;

                  if (opcode->attributes & A_IT_EXTENDER)
                    operand.flags |= QDSP6_OPERAND_IS_KXER;
                  else if ((operand.flags & QDSP6_OPERAND_IS_IMMEDIATE)
                           && (((opcode->attributes & EXTENDABLE_LOWER_CASE_IMMEDIATE)
                                && (ISLOWER (operand.enc_letter)))
                               || ((opcode->attributes & EXTENDABLE_UPPER_CASE_IMMEDIATE)
                                   && (ISUPPER (operand.enc_letter)))))
                    /* Not necessarily extended, but maybe so. */
                    operand.flags |= QDSP6_OPERAND_IS_KXED;
                  dst = qdsp6_dis_operand (&operand, insn, address,
                                           opcode->enc, dst, errmsg);
                  if (!dst)
                    {
                      /* Some kind of error! */
                      sprintf (buf, "<unknown insn 0x%08x>", insn);
                      return FALSE;
                    }

                  /* Move past the opcode specifier */
                  src += len;
                  break;
                }
            }

          if (!found)
            {
              sprintf (buf, "<unknown insn 0x%08x>", insn);

              if (errmsg)
                {
                  snprintf (temp, sizeof (temp), "unknown operand: %s", src);
                  *errmsg = temp;
                }
              return FALSE;
            }
        }
      else
        {
          /* Beautify disassembly. */
          switch (*src)
            {
              /* Skip space after. */
              case '(':
              case '!':
                *dst++ = *src++;

                while (ISBLANK (src [0]))
                  src++;
                break;

              /* Delete space before. */
              case ')':
              case '.':
              case ',':
              case ':':
                while (ISBLANK (dst [-1]))
                  dst--;

                *dst++ = *src++;
                break;

              /* Nothing, just copy. */
              default:
                *dst++ = *src++;
                break;
            }
        }
    }

  *dst = '\0';

  if (end_packet)
    {
      if (in_packet)
        {
          dst += sprintf (dst, " %c", PACKET_END);

          if (end_inner)
            dst += sprintf (dst, PACKET_END_INNER);
          if (end_outer)
            dst += sprintf (dst, PACKET_END_OUTER);
        }

      in_packet = FALSE;
      end_inner = FALSE;
      end_outer = FALSE;
    }

  return TRUE;
}

static const qdsp6_operand *
qdsp6_operand_find_xx16
(const qdsp6_operand *operand, char *suffix)
{
  char buf[100];
  size_t i;

  sprintf(buf, "%s%s", operand->fmt, suffix);
  for (i = 0; i < qdsp6_operand_count; i++) {
    const qdsp6_operand *oper = &qdsp6_operands[i];
    if (!strcmp(oper->fmt, buf)) {
      return oper;
    }
  }
  return NULL;
}

/* Given an operand, find the corresponding operand
   that will extract the low 16 bits.

   By convention, we append "_BFD_RELOC_QDSP6_LO16" to the format.
 */

const qdsp6_operand *
qdsp6_operand_find_lo16
(const qdsp6_operand *operand)
{
  return qdsp6_operand_find_xx16 (operand, "_lo16");
}

/* Given an operand, find the corresponding operand
   that will extract the high 16 bits.

   By convention, we append "_BFD_RELOC_QDSP6_HI16" to the format.
 */

const qdsp6_operand *
qdsp6_operand_find_hi16
(const qdsp6_operand *operand)
{
  return qdsp6_operand_find_xx16 (operand, "_hi16");
}


/*Function to extract control register number from modifier operand*/
int
qdsp6_extract_modifier_operand(
    const qdsp6_operand *operand,
    qdsp6_insn insn,
    char *enc,
    int *return_value,
    char **errmsg
)
{
  size_t i;
  char reg_name[100];
  static char xx[100];

  // Get the operand i.e. aliased modifier/control register
  if(!qdsp6_dis_operand(operand, insn, 0, enc, reg_name, errmsg))
    return 0;

  /* scroll through control register array and get the corresponding
     control register number */
  for(i=0;i<qdsp6_control_regs_count;i++){
    const char *name = qdsp6_control_regs[i].name;
    size_t len = strlen(name);
    if(!strncasecmp(reg_name,name,len)){
      *return_value = qdsp6_control_regs[i].reg_num;
      return 1;
    }
  }

  sprintf(xx, "Illegal modifier operand: %s", reg_name);
  *errmsg = xx;
  return 0;
}

/* Function to extract the predicate operand from instruction and
   return the predicate register number */
int
qdsp6_extract_predicate_operand
(const qdsp6_operand *operand, qdsp6_insn insn,
 char *enc, int *return_value, char **errmsg)
{
  char reg_name [100];
  int reg_num;

  // Get the predicate register name from the instruction
  if (!qdsp6_dis_operand (operand, insn, 0, enc, reg_name, errmsg))
    return FALSE;

  // Get the predicate register number
  if (!qdsp6_extract_operand (operand, insn, 0, enc, &reg_num, errmsg))
    return FALSE;

  *return_value = reg_num;
  return TRUE;
}

/** Return the next stand-alone opcode.

Function version of the macro QDSP6_OPCODE_NEXT_ASM.

@param op The current opcode.
*/
const qdsp6_opcode *
qdsp6_opcode_next_asm
(const qdsp6_opcode *op)
{
  if (op)
    op = op->next_asm;

  return (op);
}
