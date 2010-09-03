/* tc-qdsp6.h - Macros and type defines for the QDSP6.
   Copyright 1994, 1995, 1997, 2000, 2001, 2002
   Free Software Foundation, Inc.
   Contributed by Doug Evans (dje@cygnus.com).

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2,
   or (at your option) any later version.

   GAS is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

#ifndef TC_QDSP6
#define TC_QDSP6 1

#define TARGET_BYTES_BIG_ENDIAN 0

#define LOCAL_LABELS_FB 1

#define TARGET_ARCH bfd_arch_qdsp6

#define DIFF_EXPR_OK
#define REGISTER_PREFIX '\0'
#define IMMEDIATE_PREFIX '#'

#ifdef LITTLE_ENDIAN
#undef LITTLE_ENDIAN
#endif

#ifdef BIG_ENDIAN
#undef BIG_ENDIAN
#endif

#define LITTLE_ENDIAN   1234

#define BIG_ENDIAN      4321

/* The endianness of the target format may change based on command
   line arguments.  */
#define DEFAULT_TARGET_FORMAT "elf32-littleqdsp6"
#define DEFAULT_BYTE_ORDER LITTLE_ENDIAN
#define TARGET_FORMAT qdsp6_target_format
extern const char *qdsp6_target_format;

#define WORKING_DOT_WORD

#define LISTING_HEADER "QDSP6 GAS "

#define DOUBLESLASH_LINE_COMMENTS

#define TC_EXTRA_RELAX 1

/* THIS IS A BIG HACK!!!
   We need to look at stuff before and after the ":"
   so we access a variable here that is not passed to the macro
 */
#define TC_START_LABEL(C, B, A) qdsp6_start_label (C, B, A)
extern int qdsp6_start_label PARAMS ((char c, char *before, char *after));

/* The QDSP6 needs to parse reloc specifiers in .word.  */
#define TC_PARSE_CONS_EXPRESSION(EXP, NBYTES) \
  qdsp6_parse_cons_expression (EXP, NBYTES)
extern void qdsp6_parse_cons_expression PARAMS ((struct expressionS *, unsigned));

#define TC_EQUAL_IN_INSN(C, PTR) (1)

#if 0
#define TC_CONS_FIX_NEW(FRAG, WHERE, NBYTES, EXP) \
  qdsp6_cons_fix_new (FRAG, WHERE, NBYTES, EXP)
extern void
qdsp6_cons_fix_new PARAMS ((struct frag *, int, int, struct expressionS *));
#endif

#define tc_symbol_chars qdsp6_symbol_chars
extern const char qdsp6_symbol_chars [];

/* Actually a pointer to a qdsp6_operand. */
#define TC_FIX_TYPE PTR
#define TC_INIT_FIX_DATA(F) ((F)->tc_fix_data = NULL)

struct qdsp6_frag_data;
#define TC_FRAG_TYPE struct qdsp6_frag_data*
extern void qdsp6_frag_init (fragS *, fragS *);
#define TC_FRAG_INIT(F) qdsp6_frag_init (F, NULL)

/* Make sure we don't resolve fixups for pc relative relocations */
#define TC_FORCE_RELOCATION_LOCAL(FIX) \
  (!(FIX)->fx_pcrel || !IS_PC_RELATIVE_RELOC ((FIX)->fx_r_type))

#define DWARF2_LINE_MIN_INSN_LENGTH 4

/* Values passed to md_apply_fix3 don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

/* ELF wants global symbols left alone, so let the default be assumed. */
/* #define EXTERN_FORCE_RELOC 1 */

#define tc_parallel_separator_chars qdsp6_parallel_separator_chars
extern const char qdsp6_parallel_separator_chars [];

#define tc_unrecognized_line(ch) qdsp6_unrecognized_line (ch)
extern int qdsp6_unrecognized_line (int ch);

#define md_cleanup() qdsp6_cleanup ()
extern void qdsp6_cleanup (void);

/* This avoids the segments from filling up. */
#define SUB_SEGMENT_ALIGN(S, F) (0)

#define md_do_align(n, fill, len, max, around)	\
{						\
  if ((n) 					\
      && !need_pass_2				\
      && !(fill)				\
      && subseg_text_p (now_seg))		\
  {						\
    frag_align_code ((n), (max));		\
    goto around;				\
  }						\
}

#define HANDLE_ALIGN(fragP)					\
{								\
  if (fragP->fr_type == rs_align_code) 				\
    qdsp6_align_code (fragP, (  fragP->fr_next->fr_address	\
                              - fragP->fr_address		\
                              - fragP->fr_fix));		\
}
extern void qdsp6_align_code PARAMS ((fragS *, size_t));

/* #define TC_GENERIC_RELAX_TABLE qdsp6_relax_table */
extern const struct relax_type qdsp6_relax_table [];

#define md_convert_frag qdsp6_convert_frag
extern void qdsp6_convert_frag (bfd *, segT, fragS *);
#define md_estimate_size_before_relax qdsp6_estimate_size_before_relax
extern int qdsp6_estimate_size_before_relax (fragS *, segT);

#define md_relax_frag qdsp6_relax_frag
extern long qdsp6_relax_frag (segT segment, fragS *fragP, long stretch);

#define md_number_to_chars(buf, val, n)		\
  ((target_big_endian)				\
   ? number_to_chars_bigendian (buf, val, n)	\
   : number_to_chars_littleendian (buf, val, n))

#define IS_GP_RELATIVE_RELOC(r) \
  ((r) == BFD_RELOC_QDSP6_GPREL16_3 \
   || (r) == BFD_RELOC_QDSP6_GPREL16_2 \
   || (r) == BFD_RELOC_QDSP6_GPREL16_1 \
   || (r) == BFD_RELOC_QDSP6_GPREL16_0)

/* QDSP6 7 bit pc-relative branch.
   The lowest 2 bits must be zero and are not stored in the instruction.*/
#define IS_PC_RELATIVE_RELOC(r) \
   ((r) == BFD_RELOC_QDSP6_B32_PCREL_X \
    || (r) == BFD_RELOC_QDSP6_B22_PCREL \
    || (r) == BFD_RELOC_QDSP6_B22_PCREL_X \
    || (r) == BFD_RELOC_QDSP6_B15_PCREL \
    || (r) == BFD_RELOC_QDSP6_B15_PCREL_X \
    || (r) == BFD_RELOC_QDSP6_B13_PCREL \
    || (r) == BFD_RELOC_QDSP6_B13_PCREL_X \
    || (r) == BFD_RELOC_QDSP6_B9_PCREL \
    || (r) == BFD_RELOC_QDSP6_B9_PCREL_X \
    || (r) == BFD_RELOC_QDSP6_B7_PCREL)

#ifdef OBJ_ELF
#define ELF_TC_SPECIAL_SECTIONS \
  { ".sdata", SHT_PROGBITS, SHF_ALLOC + SHF_WRITE + SHF_QDSP6_GPREL }, \
  { ".sbss",  SHT_NOBITS,   SHF_ALLOC + SHF_WRITE + SHF_QDSP6_GPREL },
#endif

/* The upper limit is arbitrarily set to 17 because that is what
   it was in 2.14 */
#define TC_ALIGN_LIMIT 17

#endif /* TC_QDSP6 */
