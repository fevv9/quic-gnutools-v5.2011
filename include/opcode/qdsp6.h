

/* Opcode table for the QDSP6.
   Copyright 2004 Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler, GDB, the GNU debugger, and
   the GNU Binutils.

   GAS/GDB is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS/GDB is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS or GDB; see the file COPYING.	If not, write to
   the Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef OPCODES_QDSP6_H
#define OPCODES_QDSP6_H 1

/* List of the various cpu types.
   The tables currently use bit masks to say whether the instruction or
   whatever is supported by a particular cpu.  This lets us have one entry
   apply to several cpus.

   The `base' cpu must be 0. The cpu type is treated independently of
   endianness. The complete `mach' number includes endianness.
   These values are internal to opcodes/bfd/binutils/gas.  */
#define QDSP6_MACH    0
#define QDSP6_MACH_V2 2
#define QDSP6_MACH_V3 3
#define QDSP6_MACH_V4 4
/* Additional cpu values can be inserted here and QDSP6_MACH_BIG moved down.  */
#define QDSP6_MACH_BIG 16

/* Mask of number of bits necessary to record cpu type.  */
#define QDSP6_MACH_CPU_MASK (QDSP6_MACH_BIG - 1)
/* Mask of number of bits necessary to record cpu type + endianness.  */
#define QDSP6_MACH_MASK ((QDSP6_MACH_BIG << 1) - 1)

/* Qualifier for several table entries. */
#define QDSP6_IS_V2 (1 << 31)
#define QDSP6_IS_V3 (1 << 30)
#define QDSP6_IS_V4 (1 << 29)
#define QDSP6_V2_AND_V3 (QDSP6_IS_V3 | QDSP6_IS_V2)
#define QDSP6_V2_AND_UP (QDSP6_IS_V4 | QDSP6_IS_V3 | QDSP6_IS_V2)
#define QDSP6_V3_AND_UP (QDSP6_IS_V4 | QDSP6_IS_V3)
#define QDSP6_V4_AND_UP (QDSP6_IS_V4)

/* This is the instruction size in bytes. */
#define QDSP6_INSN_LEN (4)

/** Maximum number of insns in a packet.
*/
#define MAX_PACKET_INSNS 4

/* This is the mnemonic length for mapped insns. */
#define QDSP6_MAPPED_LEN (256)

/* Determine if a number can be represented in 16 bits (unsigned) */
#define QDSP6_IS16BITS(num) 0	/* not yet implemented */

/* Determine if a number is a 16-bit instruction */
#define QDSP6_IS16INSN(num) 0	/* not yet implemented */

/* Extract the low 16 bits */
#define QDSP6_LO16(num) ((num) & ~(-1 << 16))

/* Extract the high 16 bits */
#define QDSP6_HI16(num) (QDSP6_LO16 ((num) >> 16))

/* Extract the extender bits. */
#define QDSP6_KXER_MASK(num) ((num) & (-1 << 6))

/* Extract the extended bits. */
#define QDSP6_KXED_MASK(num) ((num) & ~(-1 << 6))

/* Registers. */
#define QDSP6_NUM_GENERAL_PURPOSE_REGS 32
#define QDSP6_NUM_CONTROL_REGS         32
#define QDSP6_NUM_SYS_CTRL_REGS        64
#define QDSP6_NUM_PREDICATE_REGS       4
#define QDSP6_NUM_GUEST_REGS           32

/* Specify the register sub-ranges. */
#define QDSP6_SUBREGS    (16)
#define QDSP6_SUBREGS_LO  (0)
#define QDSP6_SUBREGS_HI (16)
#define QDSP6_SUBREGS_TO(r, p) (((r) < QDSP6_SUBREGS_HI \
                                 ? (r) \
                                 : (r) - QDSP6_SUBREGS / 2) \
                                / ((p)? 2: 1))
#define QDSP6_SUBREGS_FROM(r, p) ((r) * ((p)? 2: 1) < QDSP6_SUBREGS / 2 \
                                  ? (r) * ((p)? 2: 1) \
                                  : (r) * ((p)? 2: 1) - QDSP6_SUBREGS / 2 + QDSP6_SUBREGS_HI)

/** Slots used by some insns.
*/
#define QDSP6_SLOTS_DUPLEX (0x3) /** < Paired isns. */
#define QDSP6_SLOTS_STORES (0x2) /** < In-order dual-stores. */
#define QDSP6_SLOTS_1      (0x2) /** < Slot #1. */
#define QDSP6_SLOTS_MEM1   (0x1) /** < Preference for single memory access. */

/** Insn type opcode ranges.
*/
#define QDSP6_INSN_TYPE_A7   (0x70000000) /** A-type. */
#define QDSP6_INSN_TYPE_X8   (0x80000000) /** X-type. */
#define QDSP6_INSN_TYPE_A11  (0xb0000000) /** A-type. */
#define QDSP6_INSN_TYPE_X12  (0xc0000000) /** X-type. */
#define QDSP6_INSN_TYPE_X13  (0xd0000000) /** X-type. */
#define QDSP6_INSN_TYPE_X14  (0xe0000000) /** X-type. */
#define QDSP6_INSN_TYPE_A15  (0xf0000000) /** A-type. */
#define QDSP6_INSN_TYPE_MASK(i) ((i) & 0xf0000000) /** Insn type mask. */
#define QDSP6_INSN_TYPE_A(i) ((QDSP6_INSN_TYPE_MASK (i) == QDSP6_INSN_TYPE_A7) \
                              || (QDSP6_INSN_TYPE_MASK (i) == QDSP6_INSN_TYPE_A11) \
                              || (QDSP6_INSN_TYPE_MASK (i) == QDSP6_INSN_TYPE_A15))
#define QDSP6_INSN_TYPE_X(i) ((QDSP6_INSN_TYPE_MASK (i) == QDSP6_INSN_TYPE_X8) \
                              || (QDSP6_INSN_TYPE_MASK (i) == QDSP6_INSN_TYPE_X12) \
                              || (QDSP6_INSN_TYPE_MASK (i) == QDSP6_INSN_TYPE_X13) \
                              || (QDSP6_INSN_TYPE_MASK (i) == QDSP6_INSN_TYPE_X14))

/* Type to denote an QDSP6 instruction (at least a 32 bit unsigned int).  */
typedef unsigned int qdsp6_insn;

typedef struct _qdsp6_opcode {
  char *syntax;              /* syntax of insn  */
  char *enc;                 /* string representing the encoding */
  int flags;                 /* various flag bits  */

/* Values for `flags'.  */

/* Return CPU number, given flag bits.  */
#define QDSP6_OPCODE_CPU(bits) ((bits) & QDSP6_MACH_CPU_MASK)

/* Return MACH number, given flag bits.  */
#define QDSP6_OPCODE_MACH(bits) ((bits) & QDSP6_MACH_MASK)

/* First opcode flag bit available after machine mask.  */
#define QDSP6_OPCODE_FLAG_START (QDSP6_MACH_MASK + 1)

/* These values are used to optimize assembly and disassembly.  Each insn
   is on a list of related insns (same first letter for assembly, same
   insn code for disassembly).  */

  struct _qdsp6_opcode *next_asm;    /* Next instr to try during assembly.  */
  struct _qdsp6_opcode *next_dis;    /* Next instr to try during disassembly.  */

/* Macros to create the hash values for the lists.  */
#define QDSP6_HASH_OPCODE(string) \
  qdsp6_hash_opcode (string)
#define QDSP6_HASH_ICODE(insn) \
  qdsp6_hash_icode (insn)

 /* Macros to access `next_asm', `next_dis' so users needn't care about the
    underlying mechanism.  */
#define QDSP6_OPCODE_NEXT_ASM(op) ((op)? (op)->next_asm: (op))
#define QDSP6_OPCODE_NEXT_DIS(op) ((op)? (op)->next_dis: (op))

  unsigned int slot_mask;          /* Slots onto which the instruction can go */

  unsigned int implicit;   /* specifies implicit register writes */

  /* Values for implicit register definitions */
#define IMPLICIT_LR     0x0001
#define IMPLICIT_SP     0x0002
#define IMPLICIT_FP     0x0004
#define IMPLICIT_PC     0x0008
#define IMPLICIT_LC0    0x0010
#define IMPLICIT_SA0    0x0020
#define IMPLICIT_LC1    0x0040
#define IMPLICIT_SA1    0x0080
#define IMPLICIT_SR_OVF 0x0100
#define IMPLICIT_P3     0x0200
  /* V3 */
#define IMPLICIT_P0     0x0400
  /* V4 */
#define IMPLICIT_P1     0x0800

  unsigned int attributes;

  /* Values for 'attributes' */
#define A_IT_NOP                        0x00000001
#define A_RESTRICT_NOSRMOVE             0x00000002
#define A_RESTRICT_LOOP_LA              0x00000004
#define A_RESTRICT_COF_MAX1             0x00000008
#define A_RESTRICT_NOPACKET             0x00000010
#define A_RESTRICT_NOSLOT1              0x00000020
#define A_RESTRICT_NOCOF                0x00000040
#define A_COF                           0x00000080
#define A_RESTRICT_BRANCHADDER_MAX1     0x00000100
#define A_BRANCHADDER                   0x00000200
#define A_RESTRICT_SINGLE_MEM_FIRST     0x00000400
#define CONDITIONAL_EXEC                0x00000800
#define CONDITION_SENSE_INVERTED        0x00001000
#define CONDITION_DOTNEW                0x00002000
#define A_RESTRICT_PREFERSLOT0          0x00004000
#define A_RESTRICT_LATEPRED             0x00008000
  /* V3 */
#define A_RESTRICT_PACKET_AXOK          0x00010000
#define A_RESTRICT_PACKET_SOMEREGS_OK   0x00000000
#define A_RELAX_COF_1ST                 0x00020000
#define A_RELAX_COF_2ND                 0x00040000
  /* V4 */
#define PACKED                          0x00080000
#define A_IT_EXTENDER                   0x00100000
#define EXTENDABLE_LOWER_CASE_IMMEDIATE 0x00200000
#define EXTENDABLE_UPPER_CASE_IMMEDIATE 0x00400000
#define A_RESTRICT_SLOT0ONLY            0x00800000
#define A_STORE                         0x01000000
#define A_STOREIMMED                    0x02000000
#define A_RESTRICT_NOSLOT1_STORE        0x04000000
#define MUST_EXTEND                     0x08000000
  /* Internal */
#define DUPLEX                          0x40000000
#define PREFIX                          0x80000000

  /* If this opcode is mapped, then the function that performs the mapping */
  void *map;
} qdsp6_opcode;

typedef size_t qdsp6_hash;

typedef struct _qdsp6_operand
{
/* Format that will appear in the qdsp6_opcode syntax */
  char *fmt;

/* The number of bits in the operand (may be unused for a modifier).  */
  unsigned char bits;

/* The letter that identifies this operand in the qdsp6_opcode enc string */
  char enc_letter;

/* For immediate arguments, the value should be shifted right by this amount */
  unsigned int shift_count;

/* The relocation type and that of the extension and itself after extended. */
  bfd_reloc_code_real_type reloc_type, reloc_kxer, reloc_kxed;

/* Various flag bits.  */
  int flags;

/* Values for `flags'.  */
#define QDSP6_OPERAND_IS_IMMEDIATE 0x00000001
#define QDSP6_OPERAND_PC_RELATIVE  0x00000002
#define QDSP6_OPERAND_IS_SIGNED    0x00000004
#define QDSP6_OPERAND_IS_PAIR      0x00000008
#define QDSP6_OPERAND_IS_SUBSET    0x00000010
#define QDSP6_OPERAND_IS_MODIFIER  0x00000020
#define QDSP6_OPERAND_IS_PREDICATE 0x00000040
#define QDSP6_OPERAND_IS_CONTROL   0x00000080
#define QDSP6_OPERAND_IS_SYSTEM    0x00000100
#define QDSP6_OPERAND_IS_LO16      0x00000200
#define QDSP6_OPERAND_IS_HI16      0x00000400
#define QDSP6_OPERAND_IS_READ      0x00000800
#define QDSP6_OPERAND_IS_WRITE     0x00001000
#define QDSP6_OPERAND_IS_MODIFIED  0x00002000
#define QDSP6_OPERAND_IS_NEGATIVE  0x00004000
#define QDSP6_OPERAND_IS_CONSTANT  0x00008000
#define QDSP6_OPERAND_IS_NEW       0x00010000
#define QDSP6_OPERAND_IS_GUEST     0x00020000
#define QDSP6_OPERAND_IS_REGISTER  0x00040000
#define QDSP6_OPERAND_IS_RNEW      0x00080000

#define QDSP6_OPERAND_IS_RELAX     0x10000000
#define QDSP6_OPERAND_IS_KXER      0x20000000
#define QDSP6_OPERAND_IS_KXED      0x40000000
#define QDSP6_OPERAND_IS_INVALID   0x80000000

/* Format string and alternate format string for disassembly. */
  char *dis_fmt, *alt_fmt;

/* Function used to change the original insn into another semantically equivalent. */
  char *(*parse) (const struct _qdsp6_operand *, qdsp6_insn *,
                  const qdsp6_opcode *, char *, long *, int *, char **);
} qdsp6_operand;

typedef struct qdsp6_operand_arg
{
  const qdsp6_operand *operand;
  long value;
  char string [QDSP6_MAPPED_LEN];
} qdsp6_operand_arg;

typedef void (*qdsp6_mapping) (char *, size_t, const qdsp6_operand_arg []);

typedef struct qdsp6_reg
{
  const char *name;
  int reg_num;
  int flags;

  /* Values for flags. */
#define QDSP6_REG_IS_READONLY  (1 << 0)
#define QDSP6_REG_IS_WRITEONLY (1 << 1)
#define QDSP6_REG_IS_READWRITE (1 << 2)
} qdsp6_reg;

typedef struct qdsp6_reg_score
{
  char used, letter;
#define QDSP6_PRED_LEN (3)
#define QDSP6_PRED_MSK (~(-1 << QDSP6_PRED_LEN))
#define QDSP6_PRED_YES (0x01)
#define QDSP6_PRED_NOT (0x02)
#define QDSP6_PRED_NEW (0x04)
#define QDSP6_PRED_GET(P, I) (((P) >> ((I) * QDSP6_PRED_LEN)) & QDSP6_PRED_MSK)
#define QDSP6_PRED_SET(P, I, N) \
  ((QDSP6_PRED_GET (P, I) | ((N) & QDSP6_PRED_MSK)) << ((I) * QDSP6_PRED_LEN))
  long pred: (QDSP6_NUM_PREDICATE_REGS * QDSP6_PRED_LEN);
  size_t ndx;
} qdsp6_reg_score;

/* Bits that say what version of cpu we have. These should be passed to
   qdsp6_init_opcode_tables. At present, all there is is the cpu type.  */

/* CPU number, given value passed to `qdsp6_init_opcode_tables'.  */
#define QDSP6_HAVE_CPU(bits) ((bits) & QDSP6_MACH_CPU_MASK)
/* MACH number, given value passed to `qdsp6_init_opcode_tables'.  */
#define QDSP6_HAVE_MACH(bits) ((bits) & QDSP6_MACH_MASK)

extern const qdsp6_operand qdsp6_operands [];
extern const size_t qdsp6_operand_count;
extern qdsp6_opcode *qdsp6_opcodes;
extern size_t qdsp6_opcodes_count;
extern int qdsp6_verify_hw;
extern qdsp6_insn qdsp6_nop, qdsp6_kext;

/** Packet delimeters.
*/
#define PACKET_BEGIN     '{'         /** < Beginning of packet. */
#define PACKET_END       '}'         /** < End of packet. */
#define PACKET_END_INNER ":endloop0" /** < End of inner loop. */
#define PACKET_END_OUTER ":endloop1" /** < End of outer loop. */
#define PACKET_PAIR      ';'         /** < Sub-insn separator. */

/** Bits 15:14 in the instruction mark boundaries.
*/
#define QDSP6_END_PACKET_POS  (14)
#define QDSP6_END_PACKET_MASK (3 << QDSP6_END_PACKET_POS)
#define QDSP6_END_PACKET      (3 << QDSP6_END_PACKET_POS) /** < End of packet. */
#define QDSP6_END_LOOP        (2 << QDSP6_END_PACKET_POS) /** < End of loop. */
#define QDSP6_END_NOT         (1 << QDSP6_END_PACKET_POS) /** < Neither. */
#define QDSP6_END_PAIR        (0 << QDSP6_END_PACKET_POS) /** < Compound insn. */

/** Get, set and reset packet bits in insn.
*/
#define QDSP6_END_PACKET_GET(insn) \
  ((insn) & QDSP6_END_PACKET_MASK)                          /** < Get */
#define QDSP6_END_PACKET_SET(insn, bits) \
  (((insn) & ~QDSP6_END_PACKET_MASK) | (bits))              /** < Set */
#define QDSP6_END_PACKET_RESET(insn) \
  (QDSP6_END_PACKET_SET ((insn), \
                         QDSP6_END_PACKET_GET (insn) == QDSP6_END_PAIR \
                         ? QDSP6_END_PAIR: QDSP6_END_NOT)) /** < Reset */

/** Test for arch version.
*/
#define qdsp6_if_arch_v1() (FALSE)                         /** < V1 (Obsolete) */
#define qdsp6_if_arch_v2() (qdsp6_if_arch (QDSP6_MACH_V2)) /** < V2 */
#define qdsp6_if_arch_v3() (qdsp6_if_arch (QDSP6_MACH_V3)) /** < V3 */
#define qdsp6_if_arch_v4() (qdsp6_if_arch (QDSP6_MACH_V4)) /** < V4 */

extern int qdsp6_if_arch (int);
extern int qdsp6_if_arch_kext (void);
extern int qdsp6_if_arch_pairs (void);
extern int qdsp6_if_arch_autoand (void);
extern int qdsp6_get_opcode_mach (int, int);
extern qdsp6_hash qdsp6_hash_opcode (const char *);
extern qdsp6_hash qdsp6_hash_icode (qdsp6_insn);
extern qdsp6_insn qdsp6_encode_opcode (const char *);
extern qdsp6_insn qdsp6_encode_mask (char *);
/* `qdsp6_opcode_init_tables' must be called before `qdsp6_xxx_supported'.  */
extern void qdsp6_opcode_init_tables (int);
extern const qdsp6_opcode *qdsp6_opcode_next_asm (const qdsp6_opcode *);
extern const qdsp6_opcode *qdsp6_opcode_lookup_asm (const char *);
extern const qdsp6_opcode *qdsp6_opcode_lookup_dis (qdsp6_insn);
extern const qdsp6_opcode *qdsp6_lookup_insn (qdsp6_insn);
extern int qdsp6_opcode_supported (const qdsp6_opcode *);
extern int qdsp6_encode_operand
  (const qdsp6_operand *, qdsp6_insn *, const qdsp6_opcode *,
   long, long *, int, int, char **);
extern const qdsp6_operand *qdsp6_lookup_operand (const char *);
extern const qdsp6_operand *qdsp6_lookup_reloc
  (bfd_reloc_code_real_type, int, const qdsp6_opcode *);
extern int qdsp6_extract_operand
  (const qdsp6_operand *, qdsp6_insn, bfd_vma, char *, int *, char **);
extern int qdsp6_extract_predicate_operand
  (const qdsp6_operand *, qdsp6_insn, char *, int *, char **);
extern int qdsp6_extract_modifier_operand
  (const qdsp6_operand *, qdsp6_insn, char *, int *, char **);
extern char *qdsp6_dis_operand
  (const qdsp6_operand *, qdsp6_insn, bfd_vma, bfd_vma, char *, char *, char **);
extern int qdsp6_dis_opcode
  (char *, qdsp6_insn, bfd_vma, const qdsp6_opcode *, char **);
extern const qdsp6_operand *qdsp6_operand_find_lo16 (const qdsp6_operand *);
extern const qdsp6_operand *qdsp6_operand_find_hi16 (const qdsp6_operand *);

/* We don't put the packet header in the opcode table */
extern const qdsp6_opcode qdsp6_packet_header_opcode;

#endif /* OPCODES_QDSP6_H */
