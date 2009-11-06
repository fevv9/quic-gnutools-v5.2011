/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
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
/* Additional cpu values can be inserted here and QDSP6_MACH_BIG moved down.  */
#define QDSP6_MACH_BIG 16

/* Mask of number of bits necessary to record cpu type.  */
#define QDSP6_MACH_CPU_MASK (QDSP6_MACH_BIG - 1)
/* Mask of number of bits necessary to record cpu type + endianness.  */
#define QDSP6_MACH_MASK ((QDSP6_MACH_BIG << 1) - 1)

/* Qualifier for several table entries. */
#define QDSP6_IS_V2 (1 << 31)
#define QDSP6_IS_V3 (1 << 30)

/* Determine if a number can be represented in 16 bits (unsigned) */
#define QDSP6_IS16BITS(num)    0	/* not yet implemented */

/* Determine if a number is a 16-bit instruction */
#define QDSP6_IS16INSN(num) 0	/* not yet implemented */

/* Extract the low 16 bits */
#define QDSP6_LO16(num) \
  ((num) & 0xffff)

/* Extract the high 16 bits */
#define QDSP6_HI16(num) \
  (QDSP6_LO16 ((num) >> 16))

/* This is the instruction size. */
#define QDSP6_INSN_LEN (4)

/* Type to denote an QDSP6 instruction (at least a 32 bit unsigned int).  */
typedef unsigned int qdsp6_insn;

typedef struct qdsp6_opcode {
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

  struct qdsp6_opcode *next_asm;    /* Next instr to try during assembly.  */
  struct qdsp6_opcode *next_dis;    /* Next instr to try during disassembly.  */

/* Macros to create the hash values for the lists.  */
#define QDSP6_HASH_OPCODE(string) \
  qdsp6_hash_opcode(string)
#define QDSP6_HASH_ICODE(insn) \
  qdsp6_hash_icode(insn)

 /* Macros to access `next_asm', `next_dis' so users needn't care about the
    underlying mechanism.  */
#define QDSP6_OPCODE_NEXT_ASM(op) ((op)->next_asm)
#define QDSP6_OPCODE_NEXT_DIS(op) ((op)->next_dis)

  unsigned int slot_mask;          /* Slots onto which the instruction can go */

  unsigned int implicit_reg_def;   /* specifies implicit register writes */

  /* Values for implicit register definitions */
#define IMPLICIT_LR     (1 <<  0)
#define IMPLICIT_SP     (1 <<  1)
#define IMPLICIT_FP     (1 <<  2)
#define IMPLICIT_PC     (1 <<  3)
#define IMPLICIT_LC0    (1 <<  4)
#define IMPLICIT_SA0    (1 <<  5)
#define IMPLICIT_LC1    (1 <<  6)
#define IMPLICIT_SA1    (1 <<  7)
#define IMPLICIT_SR_OVF (1 <<  8)
#define IMPLICIT_P3     (1 <<  9)
  /* V3 */
#define IMPLICIT_P0     (1 << 10)
  unsigned int attributes;

  /* Values for 'attributes' */
#define A_IT_NOP                      (1 <<  0)
#define A_RESTRICT_NOSRMOVE           (1 <<  1)
#define A_RESTRICT_LOOP_LA            (1 <<  2)
#define A_RESTRICT_COF_MAX1           (1 <<  3)
#define A_RESTRICT_NOPACKET           (1 <<  4)
#define A_RESTRICT_NOSLOT1            (1 <<  5)
#define A_RESTRICT_NOCOF              (1 <<  6)
#define A_COF                         (1 <<  7)
#define A_RESTRICT_BRANCHADDER_MAX1   (1 <<  8)
#define A_BRANCHADDER                 (1 <<  9)
#define A_RESTRICT_SINGLE_MEM_FIRST   (1 << 10)
#define CONDITIONAL_EXEC              (1 << 11)
#define CONDITION_SENSE_INVERTED      (1 << 12)
#define CONDITION_DOTNEW              (1 << 13)
#define A_RESTRICT_PREFERSLOT0        (1 << 14)
#define A_RESTRICT_LATEPRED           (1 << 15)
  /* V3 */
#define A_RESTRICT_PACKET_AXOK        (1 << 16)
#define A_RESTRICT_PACKET_SOMEREGS_OK (1 << 17)
#define A_RELAX_COF_1ST               (1 << 18)
#define A_RELAX_COF_2ND               (1 << 19)

  /* If this opcode is remapped, then the function that
     performs the remapping */
  void (*map_func) (char *dest, int *operand_vals);
} qdsp6_opcode;

extern unsigned int qdsp6_hash_opcode (const char *);
extern unsigned int qdsp6_hash_icode (qdsp6_insn);
extern qdsp6_insn qdsp6_encode_opcode (const char *);
extern qdsp6_insn qdsp6_enc_mask (char *);

typedef struct qdsp6_operand {
/* Format that will appear in the qdsp6_opcode syntax */
  char *fmt;

/* The number of bits in the operand (may be unused for a modifier).  */
  unsigned char bits;

/* The letter that identifies this operand in the qdsp6_opcode enc string */
  char enc_letter;

/* For immediate arguments, the value should be shifted right by this amount */
  unsigned int shift_count;

/* The relocation type */
  bfd_reloc_code_real_type reloc_type;

/* Various flag bits.  */
  int flags;

/* Values for `flags'.  */
#define QDSP6_OPERAND_IS_IMMEDIATE      (1 <<  0)
#define QDSP6_OPERAND_PC_RELATIVE       (1 <<  1)
#define QDSP6_OPERAND_IS_SIGNED         (1 <<  2)
#define QDSP6_OPERAND_IS_REGISTER_PAIR  (1 <<  3)
#define QDSP6_OPERAND_IS_CONTROL_REG    (1 <<  4)
#define QDSP6_OPERAND_IS_LO16           (1 <<  5)
#define QDSP6_OPERAND_IS_HI16           (1 <<  6)
#define QDSP6_OPERAND_IS_SYSCONTROL_REG (1 <<  7)
#define QDSP6_OPERAND_IS_MODIFIER_REG   (1 <<  8)
#define QDSP6_OPERAND_IS_PREDICATE_REG  (1 <<  9)
#define QDSP6_OPERAND_IS_READ           (1 << 10)
#define QDSP6_OPERAND_IS_WRITE          (1 << 11)
#define QDSP6_OPERAND_IS_NEGATIVE_MAG   (1 << 12)

/* Format string and alternate format string for disassembly. */
  char *dis_fmt, *alt_fmt;

/* Parse function.
   This is used by the assembler to parse.

   If successful, insert the bits into the instruction and
   return the pointer to the next character of the input.
   Otherwise, return NULL;

   If there is a problem, the function will set *errmsg.
 */

  char * (*parse)(const struct qdsp6_operand *operand,
                  qdsp6_insn *insn,
                  char *enc,
                  char *input,
                  char **errmsg,
                  int *val);
} qdsp6_operand;

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

#define PACKET_BEGIN     "{"
#define PACKET_END       "}"
#define PACKET_END_INNER ":endloop0"
#define PACKET_END_OUTER ":endloop1"

#define MAX_PACKET_INSNS 4

/** Bits 15:14 in the instruction mark boundaries.
*/
#define QDSP6_PACKET_BIT_NUM                14
#define QDSP6_PACKET_BIT_MASK               (0x3<<QDSP6_PACKET_BIT_NUM)
#define QDSP6_END_PACKET                    (0x3<<QDSP6_PACKET_BIT_NUM) /** < End of packet. */
#define QDSP6_END_NOT                       (0x1<<QDSP6_PACKET_BIT_NUM) /** < End of loop. */
#define QDSP6_END_LOOP                      (0x2<<QDSP6_PACKET_BIT_NUM) /** < Neither. */

/** Get, set and reset packet bits in insn.
*/
#define QDSP6_PACKET_BIT_GET(insn) \
  ((insn) & QDSP6_PACKET_BIT_MASK)               /** < Get */
#define QDSP6_PACKET_BIT_SET(insn, bits) \
  (((insn) & ~QDSP6_PACKET_BIT_MASK) | (bits))   /** < Set */
#define QDSP6_PACKET_BIT_RESET(insn) \
  (QDSP6_PACKET_BIT_SET ((insn), QDSP6_END_NOT)) /** < Reset */

/* We don't put the packet header in the opcode table */
extern const qdsp6_opcode qdsp6_packet_header_opcode;

/* Utility fns in qdsp6-opc.c.  */
extern int qdsp6_if_arch_v1 (void);
extern int qdsp6_if_arch_v2 (void);
extern int qdsp6_if_arch_v3 (void);
extern int qdsp6_get_opcode_mach (int, int);

/* `qdsp6_opcode_init_tables' must be called before `qdsp6_xxx_supported'.  */
extern void qdsp6_opcode_init_tables (int);
extern const qdsp6_opcode *qdsp6_opcode_lookup_asm (const char *);
extern const qdsp6_opcode *qdsp6_opcode_lookup_dis (qdsp6_insn);
extern const qdsp6_opcode *qdsp6_lookup_insn (qdsp6_insn);
extern int qdsp6_opcode_supported (const qdsp6_opcode *);
extern int qdsp6_encode_operand
  (const qdsp6_operand *, qdsp6_insn *, char *, int, char **);
extern const qdsp6_operand *qdsp6_lookup_operand (const char *);
extern const qdsp6_operand *qdsp6_lookup_reloc (bfd_reloc_code_real_type);
extern int qdsp6_extract_operand
  (const qdsp6_operand *, qdsp6_insn, bfd_vma, char *, int *, char **);
extern int qdsp6_extract_predicate_operand
  (const qdsp6_operand *, qdsp6_insn, char *, int *, char **);
extern int qdsp6_extract_modifier_operand
  (const qdsp6_operand *, qdsp6_insn, char *, int *, char **);
extern char *qdsp6_dis_operand
  (const qdsp6_operand *, qdsp6_insn, bfd_vma, char *, char *, char **);
extern int qdsp6_dis_opcode
  (char *, qdsp6_insn, bfd_vma, const qdsp6_opcode *, char **);
extern const qdsp6_operand *qdsp6_operand_find_lo16 (const qdsp6_operand *);
extern const qdsp6_operand *qdsp6_operand_find_hi16 (const qdsp6_operand *);
