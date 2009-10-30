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

#include "sysdep.h"
#include <stdio.h>
#include "ansidecl.h"
#include "bfd.h"
#include "opcode/qdsp6.h"
#include "safe-ctype.h"

int qdsp6_verify_hw;

#define PARSE_FN(fn) \
static char * fn PARAMS ((const qdsp6_operand *operand, \
                          qdsp6_insn *insn, \
                          char *enc, \
                          char *input, \
                          char **errmsg, \
                          int *val))

PARSE_FN (qdsp6_parse_reg);
PARSE_FN (qdsp6_parse_dreg);
PARSE_FN (qdsp6_parse_creg);
PARSE_FN (qdsp6_parse_sreg);
#if 0
PARSE_FN(qdsp6_parse_dup);
PARSE_FN(qdsp6_throw_litimm);
#endif

/* Various types of QDSP6 operands */
const qdsp6_operand qdsp6_operands [] =
{
#define READ                    QDSP6_OPERAND_IS_READ
#define WRITE                   QDSP6_OPERAND_IS_WRITE
#define UPDATE                  (QDSP6_OPERAND_IS_READ|QDSP6_OPERAND_IS_WRITE)
#define PREDICATE               QDSP6_OPERAND_IS_PREDICATE_REG
#define MODIFIER                QDSP6_OPERAND_IS_MODIFIER_REG

  { "Rs32",       5, 's', 0, BFD_RELOC_NONE, READ, "r%u", NULL, qdsp6_parse_reg },
  { "Rt32",       5, 't', 0, BFD_RELOC_NONE, READ, "r%u", NULL, qdsp6_parse_reg },
  { "Ru32",       5, 'u', 0, BFD_RELOC_NONE, READ, "r%u", NULL, qdsp6_parse_reg },
  { "Rv32",       5, 'v', 0, BFD_RELOC_NONE, READ, "r%u", NULL, qdsp6_parse_reg },
  { "Rd32",       5, 'd', 0, BFD_RELOC_NONE, WRITE, "r%u", NULL, qdsp6_parse_reg },
  { "Rx32",       5, 'x', 0, BFD_RELOC_NONE, UPDATE, "r%u", NULL, qdsp6_parse_reg },

#if 0
  { "Rs16",       4, 's', 0, BFD_RELOC_NONE, 0, "r%u", qdsp6_parse_reg },
  { "Rt16",       4, 't', 0, BFD_RELOC_NONE, 0, "r%u", qdsp6_parse_reg },
  { "Rd16",       4, 'd', 0, BFD_RELOC_NONE, 0, "r%u", qdsp6_parse_reg },
  { "Rx16",       4, 'x', 0, BFD_RELOC_NONE, 0, "r%u", qdsp6_parse_reg },

  { "DUPRs16",     4, 's', 0, BFD_RELOC_NONE, 0, "r%u", qdsp6_parse_dup },
  { "DUPRt16",     4, 't', 0, BFD_RELOC_NONE, 0, "r%u", qdsp6_parse_dup },
  { "DUPRd16",     4, 'd', 0, BFD_RELOC_NONE, 0, "r%u", qdsp6_parse_dup },
  { "DUPRx16",     4, 'x', 0, BFD_RELOC_NONE, 0, "r%u", qdsp6_parse_dup },
#endif

#define PAIR                    QDSP6_OPERAND_IS_REGISTER_PAIR

  { "Rss32",      5, 's', 0, BFD_RELOC_NONE, PAIR|READ, "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Rtt32",      5, 't', 0, BFD_RELOC_NONE, PAIR|READ, "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Ruu32",      5, 'u', 0, BFD_RELOC_NONE, PAIR|READ, "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Rvv32",      5, 'v', 0, BFD_RELOC_NONE, PAIR|READ, "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Rdd32",      5, 'd', 0, BFD_RELOC_NONE, PAIR|WRITE, "r%u:%u", NULL, qdsp6_parse_dreg },
  { "Rxx32",      5, 'x', 0, BFD_RELOC_NONE, PAIR|UPDATE, "r%u:%u", NULL, qdsp6_parse_dreg },
  //{ "Css32",      5, 's', 0, BFD_RELOC_NONE, PAIR|READ, "c%u:%u", qdsp6_parse_dreg },
  //{ "Cdd32",      5, 'd', 0, BFD_RELOC_NONE, PAIR|WRITE, "c%u:%u", qdsp6_parse_dreg },

//  { "Ms4",        2, 's', 0, BFD_RELOC_NONE, 0, "m%u", qdsp6_parse_reg },
//  { "Mt4",        2, 't', 0, BFD_RELOC_NONE, 0, "m%u", qdsp6_parse_reg },
//  { "Ms2",        1, 's', 0, BFD_RELOC_NONE, 0, "m%u", qdsp6_parse_reg },
//  { "Mt2",        1, 't', 0, BFD_RELOC_NONE, 0, "m%u", qdsp6_parse_reg },
  { "Mu2",        1, 'u', 0, BFD_RELOC_NONE, MODIFIER|READ, "m%u", NULL, qdsp6_parse_reg },
  { "Ps4",        2, 's', 0, BFD_RELOC_NONE, PREDICATE|READ, "p%u", NULL, qdsp6_parse_reg },
  { "Pt4",        2, 't', 0, BFD_RELOC_NONE, PREDICATE|READ, "p%u", NULL, qdsp6_parse_reg },
  { "Pu4",        2, 'u', 0, BFD_RELOC_NONE, PREDICATE|READ, "p%u", NULL, qdsp6_parse_reg },
  { "Pv4",        2, 'v', 0, BFD_RELOC_NONE, PREDICATE|READ, "p%u", NULL, qdsp6_parse_reg },
  { "Pd4",        2, 'd', 0, BFD_RELOC_NONE, PREDICATE|WRITE, "p%u", NULL, qdsp6_parse_reg },
  { "Px4",        2, 'x', 0, BFD_RELOC_NONE, PREDICATE|UPDATE, "p%u", NULL, qdsp6_parse_reg },
//  { "Xu2",        1, 'u', 0, BFD_RELOC_NONE, 0, "x%u", qdsp6_parse_reg },

#define CTRL                    QDSP6_OPERAND_IS_CONTROL_REG

  { "Cs32",       5, 's', 0, BFD_RELOC_NONE, CTRL|READ, "", NULL, qdsp6_parse_creg },
  { "Ct32",       5, 't', 0, BFD_RELOC_NONE, CTRL|READ, "", NULL, qdsp6_parse_creg },
  { "Cd32",       5, 'd', 0, BFD_RELOC_NONE, CTRL|WRITE, "", NULL, qdsp6_parse_creg },
  { "Cx32",       5, 'x', 0, BFD_RELOC_NONE, CTRL|UPDATE, "", NULL, qdsp6_parse_creg },

#define SYS                     QDSP6_OPERAND_IS_SYSCONTROL_REG

  { "Ss64",       6, 's', 0, BFD_RELOC_NONE,  SYS|READ, "", NULL, qdsp6_parse_sreg },
  { "Sd64",       6, 'd', 0, BFD_RELOC_NONE,  SYS|WRITE, "", NULL, qdsp6_parse_sreg },
  { "Sx64",       6, 'x', 0, BFD_RELOC_NONE,  SYS|UPDATE, "", NULL, qdsp6_parse_sreg },

#define UIMM                     QDSP6_OPERAND_IS_IMMEDIATE
#define SIMM                     QDSP6_OPERAND_IS_IMMEDIATE | \
                                 QDSP6_OPERAND_IS_SIGNED
#define NIMM                     QDSP6_OPERAND_IS_IMMEDIATE | \
                                 QDSP6_OPERAND_IS_SIGNED | \
                                 QDSP6_OPERAND_IS_NEGATIVE_MAG

#if 0
  { "#8",         0, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", qdsp6_throw_litimm },
  { "#4",         0, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", qdsp6_throw_litimm },
  { "#2",         0, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", qdsp6_throw_litimm },
  { "#1",         0, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", qdsp6_throw_litimm },
#endif
  { "#g16:0",    16, 'i', 0, BFD_RELOC_QDSP6_GPREL16_0, UIMM, "#%g", "#0x%x", NULL },
  { "#g16:1",    16, 'i', 1, BFD_RELOC_QDSP6_GPREL16_1, UIMM, "#%g", "#0x%x", NULL },
  { "#g16:2",    16, 'i', 2, BFD_RELOC_QDSP6_GPREL16_2, UIMM, "#%g", "#0x%x", NULL },
  { "#g16:3",    16, 'i', 3, BFD_RELOC_QDSP6_GPREL16_3, UIMM, "#%g", "#0x%x", NULL },
  { "#u16:0",    16, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u16:1",    16, 'i', 1, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u16:2",    16, 'i', 2, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u16:3",    16, 'i', 3, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u16",      16, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#s11:0",    11, 'i', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s11:1",    11, 'i', 1, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s11:2",    11, 'i', 2, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s11:3",    11, 'i', 3, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#u11:3",    11, 'i', 3, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u10",      10, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u9",        9, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u8",        8, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u7",        8, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u6:0",      6, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u6:1",      6, 'i', 1, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u6:2",      6, 'i', 2, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u6:3",      6, 'i', 3, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u6",        6, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u5",        5, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u4",        4, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u3",        3, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#u2",        2, 'i', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#U10",      10, 'I', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#U8",        8, 'I', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#U6",        6, 'I', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#U5",        5, 'I', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#U4",        4, 'I', 0, BFD_RELOC_NONE, UIMM, "#%u", NULL, NULL },
  { "#s16",      16, 'i', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s12",      12, 'i', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s10",      10, 'i', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s8",        8, 'i', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s6",        6, 'i', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#S10",      10, 'I', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#S8",        8, 'I', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#S6",        6, 'I', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s4:0",      4, 'i', 0, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s4:1",      4, 'i', 1, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s4:2",      4, 'i', 2, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#s4:3",      4, 'i', 3, BFD_RELOC_NONE, SIMM, "#%d", NULL, NULL },
  { "#m9",        9, 'i', 0, BFD_RELOC_NONE, NIMM, "#%d", NULL, NULL },

#define IMM_PCREL               QDSP6_OPERAND_IS_IMMEDIATE | \
                                QDSP6_OPERAND_PC_RELATIVE |  \
                                QDSP6_OPERAND_IS_SIGNED

  { "#r22:2",    22, 'i', 2, BFD_RELOC_QDSP6_B22_PCREL, IMM_PCREL, "@%u", NULL, NULL },
  { "#r15:2",    15, 'i', 2, BFD_RELOC_QDSP6_B15_PCREL, IMM_PCREL, "@%u", NULL, NULL },
  { "#r13:2",    13, 'i', 2, BFD_RELOC_QDSP6_B13_PCREL, IMM_PCREL, "@%u", NULL, NULL },
  { "#r7:2",      7, 'i', 2, BFD_RELOC_QDSP6_B7_PCREL,  IMM_PCREL, "@%u", NULL, NULL },

  /* These are just place-holders for implicit operands. */
  { "#0",         0,   0, 0, BFD_RELOC_NONE, UIMM, "#0", NULL, NULL },

  /* These don't appear in any instructions.  We get these by mapping
     from the previous operands when we see the lo/hi modifier in the
     assembly.
   */
#define UIMM_LO16               UIMM | QDSP6_OPERAND_IS_LO16
#define UIMM_HI16               UIMM | QDSP6_OPERAND_IS_HI16
#define SIMM_LO16               SIMM | QDSP6_OPERAND_IS_LO16
#define SIMM_HI16               SIMM | QDSP6_OPERAND_IS_HI16

  { "#u16_lo16", 16, 'i', 0, BFD_RELOC_QDSP6_LO16, UIMM_LO16, "#%u", "#0x%04x", NULL },
  { "#u16_hi16", 16, 'i', 0, BFD_RELOC_QDSP6_HI16, UIMM_HI16, "#%u", "#0x%04x", NULL },
  { "#s16_lo16", 16, 'i', 0, BFD_RELOC_QDSP6_LO16, SIMM_LO16, "#%d", NULL, NULL },
  { "#s16_hi16", 16, 'i', 0, BFD_RELOC_QDSP6_HI16, SIMM_HI16, "#%d", NULL, NULL },

  /* These don't appear in any instructions.  They are used by the
     packet header.
   */
  { "Hs",         4, 's', 0, BFD_RELOC_NONE, 0, "0x%x", NULL, NULL },
  { "Hi",         1, 'i', 0, BFD_RELOC_NONE, 0, "%d", NULL, NULL },
  { "Ho",         1, 'o', 0, BFD_RELOC_NONE, 0, "%d", NULL, NULL },
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

#define MAP_FUNCTION(TAG) void qdsp6_map_##TAG (char *dest, int *operands_val)
#define DEST dest
#define GET_OP_VAL(N) operands_val [N]
#include "opcode/qdsp6_asm_maps.h"

#define QDSP6_MAP_FNAME(TAG) qdsp6_map_##TAG

/* V2 */
qdsp6_opcode qdsp6_opcodes_v2 [] =
{
#include "opcode/qdsp6_iset.h"
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

const qdsp6_opcode qdsp6_packet_header_opcode =
//{ PACKET_BEGIN " /* o = Ho, i = Hi, s = Hs */", "1000000-oi--ssss", QDSP6_MACH, NULL, NULL };
{ PACKET_BEGIN " /* o = Ho, i = Hi, s = Hs */", "1111 11-- oi--ssss 00------ --------", QDSP6_MACH, NULL, NULL, 0, 0, 0, NULL };

/* Indexed by first letter of opcode.  Points to chain of opcodes with same
   first letter.  */
static qdsp6_opcode *opcode_map[26 + 1];

static unsigned int qdsp6_icode_hash_bits;

/* Indexed by insn code.  Points to chain of opcodes with same insn code.  */
static qdsp6_opcode *icode_map [32];

static const qdsp6_reg qdsp6_gp_regs [] =
{
  { "sp", 29, QDSP6_REG_IS_READWRITE},
  { "fp", 30, QDSP6_REG_IS_READWRITE},
  { "lr", 31, QDSP6_REG_IS_READWRITE},
};

static const size_t qdsp6_gp_regs_count =
  sizeof (qdsp6_gp_regs) / sizeof (qdsp6_gp_regs [0]);

/* They can also be named by Sx, where x is
   the supervisor register number */

static qdsp6_reg qdsp6_supervisor_regs [] =
{
  {"sgp",           0, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"sgpr1",         1, 0},			// ****** reserved ****** enabled for DSP team
  {"ssr",           2, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"imask",         3, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"badva",         4, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"elr",           5, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"tid",           6, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"evb",          16, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"ipend",        17, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"syscfg",       18, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"modectl",      19, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"rev",          20, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"tlbhi",        21, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"tlblo",        22, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"tlbidx",       23, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"diag",         24, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"iad",          25, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"iel",          26, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"iahl",         27, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"pcyclehi",     30, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"pcyclelo",     31, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"isdbst",       32, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"isdbcfg0",     33, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"isdbcfg1",     34, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"brkptpc0",     35, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"brkptcfg0",    36, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"brkptpc1",     37, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"brkptcfg1",    38, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"isdbmbxin",    39, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"isdbmbxout",   40, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"isdben",       41, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"isdbgpr",      42, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"pmucnt0",      48, QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"pmucnt1",      49, QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"pmucnt2",      50, QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"pmucnt3",      51, QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"pmuevtcfg",    52, QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"pmucfg",       53, QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},

  {"acc0",         61, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"acc1",         62, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"chicken",      63, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},

  {"s63",          63, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s62",          62, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s61",          61, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s60",          60, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s59",          59, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s58",          58, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s57",          57, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s56",          56, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},

  {"s55",          55, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s54",          54, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s53",          53, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s52",          52, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s51",          51, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s50",          50, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s49",          49, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s48",          48, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},

  {"s47",          47, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s46",          46, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s45",          45, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s44",          44, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s43",          43, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},	// ****** reserved ****** enabling some for the DSP team
  {"s42",          42, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s41",          41, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s40",          40, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},

  {"s39",          39, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"s38",          38, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s37",          37, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s36",          36, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s35",          35, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s34",          34, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s33",          33, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s32",          32, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},

  {"s31",          31, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s30",          30, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s29",          29, 0}, 		// ****** reserved ******
  {"s28",          28, 0}, 		// ****** reserved ****** enabled for DSP team
  {"s27",          27, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s26",          26, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s25",          25, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"s24",          24, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},

  {"s23",          23, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s22",          22, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s21",          21, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s20",          20, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"s19",          19, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"s18",          18, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s17",          17, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READONLY},
  {"s16",          16, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},

  {"s15",          15, 0}, 		// ****** reserved ******
  {"s14",          14, 0}, 		// ****** reserved ******
  {"s13",          13, 0}, 		// ****** reserved ******
  {"s12",          12, 0}, 		// ****** reserved ******
  {"s11",          11, 0}, 		// ****** reserved ******
  {"s10",          10, 0}, 		// ****** reserved ******
  {"s9",            9, 0}, 		// ****** reserved ******
  {"s8",            8, 0}, 		// ****** reserved ******

  {"s7",            7, 0}, 			// ****** reserved ****** enabled for DSP team
  {"s6",            6, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s5",            5, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s4",            4, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s3",            3, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s2",            2, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
  {"s1",            1, 0},
  {"s0",            0, QDSP6_IS_V2 | QDSP6_IS_V3 | QDSP6_REG_IS_READWRITE},
};

static const size_t qdsp6_supervisor_regs_count =
  sizeof (qdsp6_supervisor_regs) / sizeof (qdsp6_supervisor_regs [0]);

static const qdsp6_reg qdsp6_control_regs [] =
{
  { "sa0",        0, QDSP6_REG_IS_READWRITE},
  { "lc0",        1, QDSP6_REG_IS_READWRITE},
  { "sa1",        2, QDSP6_REG_IS_READWRITE},
  { "lc1",        3, QDSP6_REG_IS_READWRITE},
  { "p3:0",       4, QDSP6_REG_IS_READWRITE},
//{ "p3:2",       5 },     ***** reserved *****
  { "m0",         6, QDSP6_REG_IS_READWRITE},
  { "m1",         7, QDSP6_REG_IS_READWRITE},
  { "usr",        8, QDSP6_REG_IS_READWRITE},
  { "pc",         9, QDSP6_REG_IS_READONLY},
  { "ugp",        10, QDSP6_REG_IS_READWRITE},
  { "gp",         11, QDSP6_REG_IS_READWRITE},
  /* They can also be named by cX, where X is
     the control register number */
  /* Note that they are in reverse order so that they
     are parsed properly (e.g., c10 is ahead of c1) */
//  { "c31",       31 },     ***** reserved *****
//  { "c30",       30 },     ***** reserved *****
//  { "c29",       29 },     ***** reserved *****
//  { "c28",       28 },     ***** reserved *****
//  { "c27",       27 },     ***** reserved *****
//  { "c26",       26 },     ***** reserved *****
//  { "c25",       25 },     ***** reserved *****
//  { "c24",       24 },     ***** reserved *****
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
//  { "c13",       13 },     ***** reserved *****
  //  { "c12",       12},    ***** reserved *****
  { "c11",       11, QDSP6_REG_IS_READWRITE},
  { "c10",       10, QDSP6_REG_IS_READWRITE},
  { "c9",         9, QDSP6_REG_IS_READONLY},
  { "c8",         8, QDSP6_REG_IS_READWRITE},
  { "c7",         7, QDSP6_REG_IS_READWRITE},
  { "c6",         6, QDSP6_REG_IS_READWRITE},
//  { "c5",         5 },     ***** reserved *****
  { "c4",         4, QDSP6_REG_IS_READWRITE},
  { "c3",         3, QDSP6_REG_IS_READWRITE},
  { "c2",         2, QDSP6_REG_IS_READWRITE},
  { "c1",         1, QDSP6_REG_IS_READWRITE},
  { "c0",         0, QDSP6_REG_IS_READWRITE},
};

static const size_t qdsp6_control_regs_count =
  sizeof (qdsp6_control_regs) / sizeof (qdsp6_control_regs [0]);

unsigned int
qdsp6_hash_opcode(
    const char *syntax
)
{
  /* We don't want the instructions of the form:
         Rd32 = op ( Rs32 , Rt32 )
     to hash to the same value, so we'll look at
     the first non-whitespace character after the
     equal (if present).

     We also have to be careful with instructions
     of the form:
         Rd32 = Cs64
     There is no opcode (and no left paren), so
     we'll hash that to 26.
   */

  const char *p1 = strchr(syntax, '=');
  const char *p2;
  char ch;

  if (p1) {
    p1++;          // Skip the =

    p2 = strchr(p1, '(');
    if (p2) {
      while (ISSPACE(*p1)) {
        p1++;
      }

      ch = TOLOWER(*p1);
      return ch >= 'a' && ch <= 'z' ? ch - 'a' : 26;
    }
  }

  return 26;
}

/* Convert the encoding string to an instruction (bit pattern) */

qdsp6_insn
qdsp6_encode_opcode(
    const char *enc
)
{
  qdsp6_insn insn = 0;
  int num_bits = 0;
  const char *p = enc;
  int bit;

  while (ISSPACE(*p)) {
    p++;
  }
  while (*p) {
    bit = (*p++ == '1') ? 1 : 0;
    insn = (insn << 1) + bit;
    num_bits++;

    while (ISSPACE(*p)) {
      p++;
    }
  }

  if (num_bits == 16 || num_bits == 32) {
    return insn;
  }
  else {
    printf("invalid number of bits: %d\n", num_bits);
    abort();
  }
}

unsigned int
qdsp6_hash_icode
(qdsp6_insn insn)
{
  return (insn >> (32 - qdsp6_icode_hash_bits));
}

/* Configuration flags.  */

/* Various QDSP6_HAVE_XXX bits.  */
#define Q6_CPU_TYPE_UNINIT (~0 << sizeof (cpu_type))
static int cpu_type;
static int cpu_flag;

/* Return if we are assembling for V1 */
int
qdsp6_if_arch_v1
(void)
{
  return (cpu_type < QDSP6_MACH_V2);
}

/* Return if we are assembling for V2 */
int
qdsp6_if_arch_v2
(void)
{
  return (cpu_type == QDSP6_MACH_V2);
}

/* Return if we are assembling for V2 */
int
qdsp6_if_arch_v3
(void)
{
  return (cpu_type == QDSP6_MACH_V3);
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
    /* Leaving space for future cores */
  };

  // RK: Handle cases when bfd_mach is not correctly set
  if (bfd_mach < bfd_mach_qdsp6_v2 || bfd_mach > bfd_mach_qdsp6_v3)
    return Q6_CPU_TYPE_UNINIT;

  return (  mach_type_map [bfd_mach - bfd_mach_qdsp6_v2]
          | (big_p? QDSP6_MACH_BIG: 0));
}

/* Initialize any tables that need it.
   Must be called once at start up (or when first needed).

   FLAGS is a set of bits that say what version of the cpu we have,
   and in particular at least (one of) QDSP6_MACHXX.  */

void
qdsp6_opcode_init_tables
(int flags)
{
  size_t i;
  cpu_type = flags;

  if (cpu_type == Q6_CPU_TYPE_UNINIT)
    return;

  cpu_flag = qdsp6_if_arch_v2 ()? QDSP6_IS_V2: qdsp6_if_arch_v3 ()? QDSP6_IS_V3: 0;

  /* We may be intentionally called more than once (for example gdb will call
     us each time the user switches cpu).  These tables only need to be init'd
     once though.  */

  /* Based on cpu type, set: operand, operand_array_size */
  qdsp6_opcodes               =   qdsp6_if_arch_v2 ()
                                ? qdsp6_opcodes_v2: qdsp6_opcodes_v3;
  qdsp6_opcodes_count         =   qdsp6_if_arch_v2 ()
                                ? qdsp6_opcodes_count_v2: qdsp6_opcodes_count_v3;

  memset (opcode_map, 0, sizeof (opcode_map));
  memset (icode_map,  0, sizeof (icode_map));

  /* Scan the opcodes table to determine the number of bits
    that can reliably be used to hash opcodes */
  qdsp6_icode_hash_bits = 32;
  for (i = 0; i < qdsp6_opcodes_count; i++)
    {
      unsigned int bits = 0;
      const char *str = qdsp6_opcodes [i].enc;

      while (*str)
        {
          char ch = *str++;

          if (!ISSPACE (ch))
            {
              if (ch == '1' || ch == '0')
                bits++;
              else
                break;
            }
        }

      if (bits < qdsp6_icode_hash_bits)
        qdsp6_icode_hash_bits = bits;
    }

  /* Scan the table backwards so macros appear at the front.  */
  for (i = qdsp6_opcodes_count; i > 0; --i)
    {
      int opcode_hash = QDSP6_HASH_OPCODE (qdsp6_opcodes [i - 1].syntax);
      qdsp6_insn insn = qdsp6_encode_opcode (qdsp6_opcodes [i - 1].enc);
      int icode_hash = QDSP6_HASH_ICODE (insn);

      qdsp6_opcodes [i - 1].next_asm = opcode_map [opcode_hash];
      opcode_map [opcode_hash] = &qdsp6_opcodes [i - 1];

      qdsp6_opcodes [i - 1].next_dis = icode_map [icode_hash];
      icode_map [icode_hash] = &qdsp6_opcodes [i - 1];
    }
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
qdsp6_opcode_lookup_asm(
    const char *insn
)
{
  return opcode_map[QDSP6_HASH_OPCODE (insn)];
}

/* Return the first insn in the chain for disassembling INSN.  */

const qdsp6_opcode *
qdsp6_opcode_lookup_dis(
    qdsp6_insn insn
)
{
  return icode_map[QDSP6_HASH_ICODE(insn)];
}

const qdsp6_opcode *
qdsp6_lookup_insn(
    qdsp6_insn insn
)
{
  const qdsp6_opcode *opcode;
  qdsp6_insn encode;
  qdsp6_insn mask;

  // Clear the packet bits for matching
  insn &= ~QDSP6_PACKET_BIT_MASK;

  for (opcode = qdsp6_opcode_lookup_dis(insn);
       opcode;
       opcode = QDSP6_OPCODE_NEXT_DIS(opcode)) {
    encode = qdsp6_encode_opcode(opcode->enc);
    mask = qdsp6_enc_mask(opcode->enc);

      if ((insn & mask) == encode) {
        return opcode;
      }
  }

  return NULL;
}

/* Helper for parse functions
   Put the value in the specified slot in the encoding */

int
qdsp6_encode_operand(
    const qdsp6_operand *operand,
    qdsp6_insn *insn,
    char *enc,
    int value,
    char **errmsg
)
{
  char enc_letter = operand->enc_letter;
  int shift_count = 0;
  int num_bits = 0;
  size_t len = strlen(enc);
  unsigned int bits;
  int min;
  int max;
  int i;
  static char buf[500];

  if (operand->shift_count) {
    /* Make sure the low bits are zero */
    if (value & ((1 << operand->shift_count) - 1)) {
      if (errmsg) {
        sprintf(buf, "low %d bits of immediate %d must be zero",
                operand->shift_count, value);
        *errmsg = buf;
      }
      return 0;
    }
  }

  if (operand->flags & QDSP6_OPERAND_IS_LO16) {
    value = QDSP6_LO16(value);
  }
  else if (operand->flags & QDSP6_OPERAND_IS_HI16) {
    value = QDSP6_HI16(value);
  }

  /* Make sure the value is within the proper range
     Must include the shift count */
  bits = operand->bits + operand->shift_count;
  if (operand->flags & QDSP6_OPERAND_IS_SIGNED) {
	max = (1 << (bits-1)) - 1;
    min = -(1 << (bits-1));

	if (operand->flags & QDSP6_OPERAND_IS_NEGATIVE_MAG)
		min += 1;
  }
  else {
    max = (1 << bits) - 1;
    min = 0;
  }
  if (value < min || value > max) {
    if (errmsg) {
      sprintf(buf, "value %d out of range: %d-%d", value, min, max);
      *errmsg = buf;
    }
    return 0;
  }

  value >>= operand->shift_count;

  /* We'll read the encoding string backwards
     and put the LSB of the value in each time */
  for (i = len-1; i >= 0; i--) {
    if (!ISSPACE(enc[i])) {
      if (enc[i] == enc_letter) {
        // Clear the bit
        (*insn) &= ~(1 << shift_count);
        // Insert the new bit
        (*insn) |= (value & 1) << shift_count;
        value >>= 1;
        num_bits++;
      }
      shift_count++;
    }
  }

  /* Make sure we encode the expected number of bits */
  if (num_bits != operand->bits) {
    if (errmsg) {
      sprintf(buf, "did not encode expected number of bits: %d != %d\n"
              "\tenc = %s\n\toperand = %s",
              num_bits, operand->bits, enc, operand->fmt);
      *errmsg = buf;
    }
    return 0;
  }

  return 1;
}

#if 0
static char *
qdsp6_parse_dup(
    const qdsp6_operand *operand,
    qdsp6_insn *insn,
    char *enc,
    char *input,
    char **errmsg
)
{
  char letter = TOLOWER(operand->fmt[3]);
  int regid;

  if (TOLOWER(*input) == letter && ISDIGIT(input[1])) {
    unsigned int reg_num = 0;

    /* Skip the letter */
    input++;

    /* Read the register number */
    while (ISDIGIT(*input)) {
      char ch = *input++;
      reg_num = 10*reg_num + (ch - '0');
    }
    if (!qdsp6_extract_operand(operand, *insn, 0, enc, &regid, errmsg)) {
      return NULL;
    }
    if (regid == reg_num) {
      return input;
    } else {
      return NULL;
    }
  }

  return NULL;
}

static char *
qdsp6_throw_litimm(
    const qdsp6_operand *operand,
    qdsp6_insn *insn,
    char *enc,
    char *input,
    char **errmsg
)
{
  int i;
  for (i = 0; operand->fmt[i]; i++) {
    if (operand->fmt[i] != input[i]) {
      return NULL;
    }
  }
  input += i;
  return input;
}
#endif

static char *
qdsp6_parse_dreg
(const qdsp6_operand *operand, qdsp6_insn *insn,
 char *enc, char *input, char **errmsg, int *val)
{
  size_t i;

  char letter = TOLOWER(operand->fmt[0]);

  if (TOLOWER(*input) == letter && ISDIGIT(input[1])) {
    unsigned int reg_num_odd = 0;
    unsigned int reg_num_even = 0;

    /* Skip the letter */
    input++;

    /* Read the register number */
    while (ISDIGIT(*input)) {
      char ch = *input++;
      reg_num_odd = 10*reg_num_odd + (ch - '0');
    }

    /* Read the colon */
    if (*input == ':' && ISDIGIT(input[1])) {
      input++;
    }
    else {
      return NULL;
    }

    /* Read the register number */
    while (ISDIGIT(*input)) {
      char ch = *input++;
      reg_num_even = 10*reg_num_even + (ch - '0');
    }

    /* Make sure the register number is odd */
    if (reg_num_odd % 2 != 1) {
      if (errmsg) {
          static char buf[100];
          sprintf(buf, "invalid odd register number: %d", reg_num_odd);
          *errmsg = buf;
      }
      return NULL;
    }

    /* Make sure the register number is even */
    if (reg_num_even % 2 != 0) {
      if (errmsg) {
          static char buf[100];
          sprintf(buf, "invalid even register number: %d", reg_num_even);
          *errmsg = buf;
      }
      return NULL;
    }

    /* Make sure the registers are consecutive */
    if (reg_num_odd != reg_num_even+1) {
      if (errmsg) {
          static char buf[100];
          sprintf(buf, "registers must be consecutive: %d:%d",
                  reg_num_odd, reg_num_even);
          *errmsg = buf;
      }
      return NULL;
    }

    if (!qdsp6_encode_operand(operand, insn, enc, reg_num_even, errmsg)) {
      return NULL;
    }

	*val = reg_num_even;
    return input;
  }
  else
    //need to match lr:fp with Rxx32
    {
      if(letter=='r' && (operand->flags & QDSP6_OPERAND_IS_REGISTER_PAIR))
	//Rxx32, Rdd32, Rss32
	{
	  if(!strncasecmp(input, "lr:fp", 5))
	    //lr:fp found
	    {
	      unsigned int reg_num_even = 0;

	      for(i = 0; i < qdsp6_gp_regs_count; i++)
		{
		  if(!strcmp(qdsp6_gp_regs[i].name, "fp"))
		    {
		      reg_num_even=qdsp6_gp_regs[i].reg_num;
		      break;
		    }
	        }

	      if (!qdsp6_encode_operand(operand, insn, enc, reg_num_even,
					errmsg)) {
		return NULL;
	      }
	      *val = reg_num_even;
	      return input+5;
	    }
	}
    }

  return NULL;
}

static char *
qdsp6_parse_reg
(const qdsp6_operand *operand, qdsp6_insn *insn,
 char *enc, char *input, char **errmsg, int *val)
{
  char letter = TOLOWER(operand->fmt[0]);

  if (TOLOWER(*input) == letter && ISDIGIT(input[1])) {
    unsigned int reg_num = 0;

    /* Skip the letter */
    input++;

    /* Read the register number */
    while (ISDIGIT(*input)) {
      char ch = *input++;
      reg_num = 10*reg_num + (ch - '0');
    }
    if (ISALPHA(*input)) return NULL; /* e.g. "r1s" */
    if (reg_num >= (unsigned int)(1<<operand->bits)) {
        /* doesn't fit */
        return NULL;
    }

    if (!qdsp6_encode_operand(operand, insn, enc, reg_num, errmsg)) {
      return NULL;
    }

	*val = reg_num;
    return input;
  }

  if (TOLOWER(operand->fmt[0]) == 'r') {
    /* find synonyms for general-purpose registers */
    /* this code is crap, but no worse than the rest of the register handling... */
    size_t i;

    for (i = 0; i < qdsp6_gp_regs_count; i++) {
      const char *name = qdsp6_gp_regs[i].name;
      size_t len = strlen(name);
      if (!strncasecmp(input, name, len)) {
        unsigned int reg_num = qdsp6_gp_regs[i].reg_num;
        if (ISALNUM(input[len])) /* e.g. "call splice" should not match "sp" */
	  continue;
        if (reg_num >= (unsigned int)(1<<operand->bits))
	  return NULL;
        if (!qdsp6_encode_operand(operand, insn, enc, reg_num, errmsg))
          return NULL;
		*val = reg_num;
        return input + len;
      }
    }
  }

  return NULL;
}

static char *
qdsp6_parse_creg
(const qdsp6_operand *operand, qdsp6_insn *insn,
 char *enc, char *input, char **errmsg, int *val)
{
  size_t i;
  int reg_num;
  static char buf [150];

  for (i = 0; i < qdsp6_control_regs_count; i++)
    {
      const char *name = qdsp6_control_regs [i].name;
      size_t len = strlen(name);

      if (!strncasecmp (name, "pc", 2) && !strncasecmp (input, "pcycle", 6))
        //control register is "pc", source code is "pcyclehi" or "pcyclelo",
        //make it don't match
        continue;

      if (!strncasecmp (input, name, len))
        {
          //Check write to read-only reg
          if (!qdsp6_verify_hw)
            if (operand->flags & QDSP6_OPERAND_IS_WRITE)
              //this is to write register
              {
                if(qdsp6_control_regs[i].flags & QDSP6_REG_IS_READONLY)
                  {
                    sprintf (buf, "Can not write to read-only register `%s'.", name);
                    *errmsg = buf;
                    break;
                  }
              }

          reg_num = qdsp6_control_regs [i].reg_num;

          if (!qdsp6_encode_operand (operand, insn, enc, reg_num, errmsg))
            return NULL;

          *val = reg_num;
          return (input + len);
        }
    }

  return NULL;
}

static char *
qdsp6_parse_sreg
(const qdsp6_operand *operand, qdsp6_insn *insn,
 char *enc, char *input, char **errmsg, int *val)
{
  size_t i, reg_num;
  static char buf [150];

  for (i = 0; i < qdsp6_supervisor_regs_count; i++) {
    const char *name = qdsp6_supervisor_regs[i].name;
    size_t len = strlen(name);
    if (!strncasecmp(input, name, len)) {

      //Check write to read-only reg
      if(!qdsp6_verify_hw)
	if(operand->flags & QDSP6_OPERAND_IS_WRITE)
	  //this is to write register
	  {
	    if(qdsp6_supervisor_regs[i].flags & QDSP6_REG_IS_READONLY)
	      {
		sprintf(buf, "Can not write to read-only register `%s'.",name);
		*errmsg = buf;
		break;
	      }
	  }

      reg_num = qdsp6_supervisor_regs[i].reg_num;
      if (!qdsp6_encode_operand(operand, insn, enc, reg_num, errmsg)) {
        return NULL;
      }
	  *val = reg_num;
      return input + len;
    }
  }

  return NULL;
}

/* Produce a bit mask from the encoding string such that
   the bits that must be 1/0 are set to 1.
   Then
       insn & mask == qdsp6_encode_opcode(enc)
   if and only if insn matches the instruction */

qdsp6_insn
qdsp6_enc_mask(
    char *enc
)
{
  qdsp6_insn insn = 0;
  int num_bits = 0;
  char *p = enc;
  char ch;
  int bit;

  while (ISSPACE(*p)) {
    p++;
  }
  while (*p) {
    ch = *p++;
    bit = (ch == '1' || ch == '0') ? 1 : 0;
    insn = (insn << 1) + bit;
    num_bits++;

    while (ISSPACE(*p)) {
      p++;
    }
  }

  if (num_bits == 16 || num_bits == 32) {
    return insn;
  }
  else {
    abort();
  }
}

const qdsp6_operand *
qdsp6_lookup_operand
(const char *name)
{
  size_t i;

  for (i = 0; i < qdsp6_operand_count; i++)
    if (!strncmp (name, qdsp6_operands [i].fmt, strlen (qdsp6_operands [i].fmt)))
      return (qdsp6_operands + i);

  return NULL;
}

const qdsp6_operand *
qdsp6_lookup_reloc
(bfd_reloc_code_real_type reloc_type)
{
  size_t i;

  if (reloc_type == BFD_RELOC_NONE)
    return NULL;

  for (i = 0; i < qdsp6_operand_count; i++)
    if (qdsp6_operands [i].reloc_type == reloc_type)
      return (qdsp6_operands + i);

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

  mask = 1 << 31;

  /* Grab the bits from the instruction */
  bits_found = 0;
  while (*enc_ptr) {
    char ch = *enc_ptr++;
    if (!ISSPACE(ch)) {
      if (ch == enc_letter) {
        unsigned int bit = (insn & mask) ? 1 : 0;
        value = (value << 1) + bit;
        bits_found++;
      }
      mask >>= 1;
    }
  }

  /* Check that we got the right number of bits */
  if (bits_found != operand->bits) {
    if (errmsg) {
      static char xx[100];
      sprintf(xx, "operand %c wrong number of bits found in %s, %d != %d",
              operand->enc_letter, enc, bits_found, operand->bits);
      *errmsg = xx;
    }
    return 0;
  }

  if (operand->flags & QDSP6_OPERAND_IS_SIGNED) {
    /* Might need to sign extend */
    if (value & (1 << (bits_found-1))) {
      unsigned int shift_count = 32 - bits_found;
      value <<= shift_count;
      value >>= shift_count;
    }
  }

  value <<= operand->shift_count;

  if (operand->flags & QDSP6_OPERAND_PC_RELATIVE) {
    value += addr;
  }

  *return_value = value;
  return 1;
}

static char *
qdsp6_dis_named_reg
(int value, int count, const qdsp6_reg *regs, char *buf, char **errmsg)
{
  int n = 0;
  int found = 0;
  int i;

  for (i = 0; i < count; i++) {
    if (value == regs[i].reg_num) {
      found = 1;
      n = sprintf(buf, "%s", regs[i].name);
      break;
    }
  }

  if (!found) {
    if (errmsg) {
      static char xx[100];
      sprintf(xx, "control register not found: %u", value);
      *errmsg = xx;
    }
    return NULL;
  }

  return buf + n;
}


char *
qdsp6_dis_operand
(const qdsp6_operand *operand, qdsp6_insn insn, bfd_vma addr,
 char *enc, char *buf, char **errmsg)
{
  int value;
  int n = 0;

  if (!qdsp6_extract_operand(operand, insn, addr, enc, &value, errmsg)) {
    return NULL;
  }

  if (operand->flags & QDSP6_OPERAND_IS_REGISTER_PAIR) {
    n = sprintf(buf, operand->dis_fmt, value+1, value);
  }
  else if (operand->flags & QDSP6_OPERAND_IS_CONTROL_REG) {
    return qdsp6_dis_named_reg(value,
                               qdsp6_control_regs_count,
                               qdsp6_control_regs,
                               buf, errmsg);
  }
  else if (operand->flags & QDSP6_OPERAND_IS_SYSCONTROL_REG) {
    return qdsp6_dis_named_reg(value,
                               qdsp6_supervisor_regs_count,
                               qdsp6_supervisor_regs,
                               buf, errmsg);
  }
  else {
    n = sprintf(buf, operand->dis_fmt, value);
  }
  if (n > 0) {
    return buf + n;
  }
  else {
    if (errmsg) {
      static char xx[100];
      sprintf(xx, "bad return code from sprintf: %d", n);
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
  unsigned int packet_bits = (insn & QDSP6_PACKET_BIT_MASK);
  int end_packet = FALSE;
  char *src = opcode->syntax;
  char *dst = buf;

  switch (packet_bits)
    {
      case QDSP6_END_PACKET:
        if (in_packet)
          dst += sprintf (dst, "%*c ", (int) strlen (PACKET_BEGIN), ' ');
        end_packet = TRUE;
        break;

      case QDSP6_END_LOOP:
      case QDSP6_END_NOT:
        if (!in_packet)
          {
            dst += sprintf (dst, PACKET_BEGIN " ");
            packet_addr = address;
          }
        else
          dst += sprintf (dst, "%*c ", (int) strlen (PACKET_BEGIN), ' ');

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
              const qdsp6_operand *operand = &qdsp6_operands [i];
              size_t len = strlen (operand->fmt);

              if (!strncmp (src, operand->fmt, len))
                {
                  found = TRUE;

                  dst = qdsp6_dis_operand (operand, insn, address,
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

#if 0
  dst += sprintf(dst, "\t/* packet bits: 0x%x */",
                 packet_bits >> QDSP6_PACKET_BIT_NUM);
#endif

  if (end_packet)
    {
      if (in_packet)
        {
          dst += sprintf(dst, " " PACKET_END);

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
qdsp6_operand_find_xx16(
    const qdsp6_operand *operand,
    char *suffix
)
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
qdsp6_operand_find_lo16(
    const qdsp6_operand *operand
)
{
  return qdsp6_operand_find_xx16(operand, "_lo16");
}

/* Given an operand, find the corresponding operand
   that will extract the high 16 bits.

   By convention, we append "_BFD_RELOC_QDSP6_HI16" to the format.
 */

const qdsp6_operand *
qdsp6_operand_find_hi16(
    const qdsp6_operand *operand
)
{
  return qdsp6_operand_find_xx16(operand, "_hi16");
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
qdsp6_extract_predicate_operand(
    const qdsp6_operand *operand,
    qdsp6_insn insn,
    char *enc,
    int *return_value,
    char **errmsg
)
{
  char reg_name[100];
  int reg_num;

  // Get the predicate register name from the instruction
  if(!qdsp6_dis_operand(operand, insn, 0, enc, reg_name, errmsg))
    return 0;

  // Get the predicate register number
  if(!qdsp6_extract_operand(operand, insn, 0, enc, &reg_num, errmsg))
    return 0;

  *return_value = reg_num;
  return 1;
}
