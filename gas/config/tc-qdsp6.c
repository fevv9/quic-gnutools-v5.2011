/*****************************************************************
* Copyright (c) $Date$ Qualcomm Innovation Center, Inc.
* All Rights Reserved.
* Modified by Qualcomm Innovation Center, Inc. on $Date$
*****************************************************************/

/* tc-qdsp6.c -- Assembler for the QDSP6
   Copyright 1994, 1995, 1997, 1999, 2000, 2001, 2002, 2006-2008
   Free Software Foundation, Inc.

   This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

   QDSP6 machine-specific port contributed by Qualcomm, Inc.
*/

#include <stdint.h>
#include <stdio.h>
#include <sys/param.h>
#include <assert.h>
#include "as.h"
#include "dwarf2dbg.h"
#include "elf/qdsp6.h"
#include "libiberty.h"
#include "libbfd.h"
#include "opcode/qdsp6.h"
#include "struc-symbol.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include <xregex.h>

#if defined (__MINGW32__)
#  ifndef MAX
#    define MAX(a,b) ((a) > (b) ? (a) : (b))
#  endif
#  ifndef MIN
#    define MIN(a,b) ((a) < (b) ? (a) : (b))
#  endif

#define regcomp xregcomp
#define regexec xregexec
#endif
#define MAYBE (TRUE + 1)
#define TRUER(a, b) ((a) == TRUE \
                     ? (a) \
                     : (b) == TRUE \
                       ? (b) \
                       : (a) == MAYBE \
                         ? (a) \
                         : (b))

#ifdef __CYGWIN__
#  define REGEX_LEFT(re) "[[:<:]]" re
#  define REGEX_RITE(re) re "[[:>:]]"
#else
#  define REGEX_LEFT(re) "\\<" re
#  define REGEX_RITE(re) re "\\>"
#endif
#define REGEX_ENCLOSE(re) REGEX_LEFT (REGEX_RITE (re))

#define DEFAULT_CODE_ALIGNMENT (2) /* log2 (4) */
#define DEFAULT_CODE_FALIGN    (4) /* log2 (16) */
#define DEFAULT_DATA_ALIGNMENT (3) /* log2 (8) */

/* General limits. */
#define MAX_MESSAGE           (66) /* Message limit. */
#define MAX_DATA_ALIGNMENT    (16)
#define MAX_DCFETCH           (2) /* DCFETCH in a packet. */
/* Number of packets to look back at an .falign. */
#define MAX_FALIGN_PACKETS    (1) /* Must be at least 1. */
#if MAX_FALIGN_PACKETS < 1
#error MAX_FALIGN_PACKETS must be at least 1.
#endif
/* Include the current packet. */
#define MAX_PACKETS           (MAX_FALIGN_PACKETS + 1)
/* Per insn. */
#define MAX_FIXUPS            (1)

#define QDSP6_NOP         "nop"
#define QDSP6_NOP_LEN     (sizeof (QDSP6_NOP) - 1)
#define QDSP6_DCFETCH     "dcfetch"
#define QDSP6_DCFETCH_LEN (sizeof (QDSP6_DCFETCH) - 1)

/* QDSP6 CR aliases. */
#define QDSP6_SA0 0
#define QDSP6_LC0 1
#define QDSP6_SA1 2
#define QDSP6_LC1 3
#define QDSP6_P30 4
#define QDSP6_M0  6
#define QDSP6_M1  7
#define QDSP6_SR  8
#define QDSP6_PC  9
#define QDSP6_GP 10

/* QDSP6 GPR aliases. */
#define QDSP6_SP 29
#define QDSP6_FP 30
#define QDSP6_LR 31

/* Literals limits. */
/* prefix + "_" + hex + NUL */
#define MAX_LITERAL_NAME      (LITERAL_PREFIX_LEN + 1 + 16 + 1)
/* Total numer of literals. */
#define MAX_LITERAL_POOL      (65536)
/* section + literal (assumes that the literal starts with ".") */
#define MAX_LITERAL_SECTION   \
  (  MAX (MAX (MAX (LITERAL_LEN,   LITERAL_ONCE_LEN),   MAX (LITERAL_A_LEN, LITERAL_ONCE_A_LEN)), \
          MAX (MAX (LITERAL_4_LEN, LITERAL_ONCE_4_LEN), MAX (LITERAL_8_LEN, LITERAL_ONCE_8_LEN))) \
   + MAX_LITERAL_NAME)
#define MAX_LITTLENUMS        (6) /* Equal to MAX_PRECISION in atof-ieee.c.  */
#define MAX_OPERANDS          (10) /* Safe limit. */
#define MAX_SMALL_SECTION     \
  (1 + 1 + 1 + \
   MAX (MAX (sizeof (SMALL_COM_SECTION), sizeof (SMALL_BSS_SECTION)), \
        sizeof (SMALL_DATA_SECTION)))

#define LITERAL_SECTION          SMALL_DATA_SECTION
#define LITERAL_LEN              (sizeof (LITERAL_SECTION))
#define LITERAL_SECTION_A        ".lita"
#define LITERAL_A_LEN            (sizeof (LITERAL_SECTION_A))
#define LITERAL_SECTION_4        ".lit4"
#define LITERAL_4_LEN            (sizeof (LITERAL_SECTION_4))
#define LITERAL_SECTION_8        ".lit8"
#define LITERAL_8_LEN            (sizeof (LITERAL_SECTION_8))
#define LITERAL_SECTION_ONCE     ".gnu.linkonce.s"
#define LITERAL_ONCE_LEN         (sizeof (LITERAL_SECTION_ONCE))
#define LITERAL_SECTION_ONCE_A   ".gnu.linkonce.la"
#define LITERAL_ONCE_A_LEN       (sizeof (LITERAL_SECTION_ONCE_A))
#define LITERAL_SECTION_ONCE_4   ".gnu.linkonce.l4"
#define LITERAL_ONCE_4_LEN       (sizeof (LITERAL_SECTION_ONCE_4))
#define LITERAL_SECTION_ONCE_8   ".gnu.linkonce.l8"
#define LITERAL_ONCE_8_LEN       (sizeof (LITERAL_SECTION_ONCE_8))
/* Must start with "." to be concatenated nicely with the literal "once" section. */
#define LITERAL_PREFIX           ".CONST"
#define LITERAL_PREFIX_LEN       (sizeof (LITERAL_PREFIX))
#define LITERAL_SUBSECTION       (8191 - 8)

#define SMALL_DATA_SECTION ".sdata"
#define SMALL_DATA_LEN     (sizeof (SMALL_DATA_SECTION))

#define SMALL_BSS_SECTION ".sbss"
#define SMALL_BSS_LEN     (sizeof (SMALL_BSS_SECTION))

#define SMALL_COM_SECTION ".scommon"
#define SMALL_COM_LEN     (sizeof (SMALL_COM_SECTION))

/** Literal data structure. */
typedef struct _qdsp6_literal
  {
    expressionS e;
    segT        sec;
    int         sub;
    symbolS    *sym;
    struct _qdsp6_literal
               *next;
    size_t      size;
    char        name [MAX_LITERAL_NAME];
    char        secname [MAX_LITERAL_SECTION];
  } qdsp6_literal;

/** Packet instruction. */
typedef struct
  {
    qdsp6_insn insn;
    size_t ndx;
    const qdsp6_opcode *opcode;
    const qdsp6_operand *ioperand;
    unsigned ireg, oreg;
    unsigned lineno;
    unsigned flags;
    char padded;
    char used;
    size_t fc;
    qdsp6_operand operand;
    expressionS exp;
    fixS *fix;
    char string [QDSP6_MAPPED_LEN];
  } qdsp6_packet_insn;

/** Pair of instruction packets. */
typedef struct
  {
    qdsp6_packet_insn prefix, insn;
  } qdsp6_packet_insn2;

/** Flags for packet instruction. */
enum _qdsp6_insn_flag
  {
    QDSP6_INSN_IS_KXED  = 0x01, /* K-extension prefixed insn. */
    QDSP6_INSN_IS_R64   = 0x02, /* Insn operates on register pairs. */
    QDSP6_INSN_IS_RELAX = 0x04, /* Insn will be relaxed. */
    QDSP6_INSN_IN_RNEW  = 0x08, /* Insn has an operand of type R.NEW. */
    QDSP6_INSN_OUT_RNEW = 0x10, /* Insn produces a result GPR. */
  };

/** Packet house keeping. */
typedef struct
  {
    size_t size; /* Number of insns. */
    size_t prefix; /* Number of k-extenders. */
    size_t duplex; /* Number of duplex insns. */
    size_t relax; /* Number of reserved slots. */
    size_t drlx, ddrlx; /* Deltas in extensions. */
    size_t dpad, ddpad; /* Deltas in padding NOPs. */
    size_t dpkt, ddpkt; /* Deltas in padding NOPs in packets. */
    char faligned; /* Packet should be fetch-aligned. */
    char is_inner; /* Packet has :endloop0. */
    char is_outer; /* Packet has :endloop1. */
    qdsp6_packet_insn prefixes [MAX_PACKET_INSNS]; /* k-extender insns. */
    qdsp6_packet_insn insns [MAX_PACKET_INSNS]; /* Insns. */
    qdsp6_packet_insn2 pairs [MAX_PACKET_INSNS][2]; /* Original paired prefix+insn. */
    int reason; /* Reason for last history flush. */
  } qdsp6_packet;

typedef struct qdsp6_frag_data
  {
    qdsp6_packet packet;
    fragS *previous;
  } qdsp6_frag_data;

/** Reason for flushing the packet history. */
enum _qdsp6_flush_reason
  {
    QDSP6_FLUSH_INIT   = 0x00, /* Initilization. */
    QDSP6_FLUSH_LABEL  = 0x01, /* A label was defined. */
    QDSP6_FLUSH_ALIGN  = 0x02, /* An alignment was performed. */
    QDSP6_FLUSH_NOP    = 0x04,  /* A NOP packet was added. */
  };

/** .falign counter types. */
enum _qdsp6_falign_counters
  {
    QDSP6_FALIGN_TOTAL,   /* # of .falign directives. */
    QDSP6_FALIGN_INC,     /* ... which incorporated into previous packets. */
    QDSP6_FALIGN_INS,     /* ... which inserted new NOP packets. */
    QDSP6_FALIGN_FALIGN,  /* # of times that the history stopped due to a fetch-alignment. */
    QDSP6_FALIGN_FALIGN1, /* ... due to a single-insn fetch-alignment. */
    QDSP6_FALIGN_SECTION, /* ... due to a section change. */
    QDSP6_FALIGN_END,     /* ... due to reaching the end of the history. */
    QDSP6_FALIGN_TOP,     /* ... due to reaching the top of the history. */
    QDSP6_FALIGN_LABEL,   /* ... due to a label. */
    QDSP6_FALIGN_ALIGN,   /* ... due to an alignment performed. */
    QDSP6_FALIGN_NOP,     /* ... due to a NOP packet inserted. */
    QDSP6_FALIGN_SHUF,    /* ... due to failure inserting a NOP. */
    QDSP6_FALIGN_COUNTERS
  };

/** Pairing counter types. */
enum _qdsp6_pairs_counters
  {
    QDSP6_PAIRS_TOTAL,   /* # of total pairings. */
    QDSP6_PAIRS_UNDONE,  /* # of pairings undone. */
    QDSP6_PAIRS_COUNTERS
  };

extern int qdsp6_get_mach (char *);
extern void qdsp6_code_symbol (expressionS *);

int qdsp6_assemble (qdsp6_packet *, char *, int, int);
int qdsp6_assemble_pair (qdsp6_packet *, qdsp6_packet_insn *, qdsp6_packet_insn *);
void qdsp6_insert_operand (char *, const qdsp6_operand *, offsetT, fixS*);
void qdsp6_common (int);
void qdsp6_option (int);
void qdsp6_falign (int);
void qdsp6_init (int);
int qdsp6_is_nop (qdsp6_insn);
int qdsp6_is_prefix (qdsp6_insn);
qdsp6_insn qdsp6_find_nop (void);
qdsp6_insn qdsp6_find_kext (void);
qdsp6_insn qdsp6_find_insn (const char *);
void qdsp6_init_reg (void);
int qdsp6_check_operand_args (const qdsp6_operand_arg [], size_t);
void qdsp6_check_insn (qdsp6_packet *, size_t);
int qdsp6_check_new_predicate (void);
int qdsp6_pair_open (void);
int qdsp6_pair_close (void);
void qdsp6_insn_init (qdsp6_packet_insn *);
void qdsp6_packet_init (qdsp6_packet *);
void qdsp6_packet_open (qdsp6_packet *);
void qdsp6_packet_close (qdsp6_packet *);
void qdsp6_packet_begin (qdsp6_packet *);
void qdsp6_packet_end (qdsp6_packet *);
void qdsp6_packet_end_inner (qdsp6_packet *);
void qdsp6_packet_end_outer (qdsp6_packet *);
void qdsp6_packet_end_lookahead (int *inner_p, int *);
void qdsp6_packet_unfold (qdsp6_packet *);
void qdsp6_packet_fold (qdsp6_packet *);
void qdsp6_packet_write (qdsp6_packet *);
void qdsp6_packet_finish (qdsp6_packet *);
int qdsp6_packet_falign (size_t);
size_t qdsp6_packet_size (const qdsp6_packet *);
size_t qdsp6_packet_length (const qdsp6_packet *);
size_t qdsp6_packet_count (const qdsp6_packet *);
size_t qdsp6_packet_insns (const qdsp6_packet *);
int qdsp6_packet_insert
  (qdsp6_packet *, const qdsp6_packet_insn *, const qdsp6_packet_insn *,
   const qdsp6_packet_insn2 *, int);
int qdsp6_packet_cram
  (qdsp6_packet *, qdsp6_packet_insn *, const qdsp6_packet_insn *,
   const qdsp6_packet_insn2 *, int);
size_t qdsp6_insert_nops (size_t);
int qdsp6_prefix_kext (qdsp6_packet_insn *, long);
char *qdsp6_insn_write
  (qdsp6_insn, size_t, const qdsp6_operand *, expressionS *,
   char *, size_t, fixS **, int);
char *qdsp6_parse_immediate
  (qdsp6_packet_insn *, qdsp6_packet_insn *, const qdsp6_operand *,
   char *, long *, char **);
int qdsp6_gp_const_lookup (char *str, char *);
segT qdsp6_create_sbss_section (const char *, flagword, unsigned int);
segT qdsp6_create_scom_section (const char *, flagword, unsigned int);
segT qdsp6_create_literal_section (const char *, flagword, unsigned int);
qdsp6_literal *qdsp6_add_to_lit_pool (expressionS *, size_t);
void qdsp6_shuffle_packet (qdsp6_packet *, size_t *);
void qdsp6_shuffle_handle (qdsp6_packet *);
void qdsp6_shuffle_do (qdsp6_packet *);
int qdsp6_shuffle_helper (qdsp6_packet *, size_t, size_t *);
int qdsp6_discard_dcfetch (qdsp6_packet *, int);
int qdsp6_has_single (const qdsp6_packet *);
int qdsp6_has_prefix (const qdsp6_packet *);
int qdsp6_has_pair (const qdsp6_packet *);
int qdsp6_has_duplex (const qdsp6_packet *);
int qdsp6_has_duplex_hits
  (const qdsp6_packet *, const qdsp6_packet_insn *, size_t *);
int qdsp6_has_rnew (const qdsp6_packet *, qdsp6_packet_insn **);
int qdsp6_has_mem (const qdsp6_packet *);
int qdsp6_has_store (const qdsp6_packet *);
int qdsp6_has_store_not (const qdsp6_packet *);
addressT qdsp6_frag_fix_addr (void);
int qdsp6_is_nop_keep (const qdsp6_packet *, int);
int qdsp6_find_noslot1 (const qdsp6_packet *, size_t);
void qdsp6_check_register
  (qdsp6_reg_score *, int, int, const qdsp6_operand *, qdsp6_packet_insn *, size_t);
void qdsp6_check_predicate (int, const qdsp6_opcode *);
void qdsp6_check_implicit
  (const qdsp6_opcode *, unsigned int, int, qdsp6_reg_score *, const char *);
void qdsp6_check_implicit_predicate (const qdsp6_opcode *, unsigned int, int);
int qdsp6_packet_check_solo (const qdsp6_packet *);
int qdsp6_relax_branch (fragS *);
int qdsp6_relax_falign (fragS *);
long qdsp6_relax_branch_try (fragS *, segT, long);
long qdsp6_relax_falign_try (fragS *, segT, long);
long qdsp6_relax_frag (segT, fragS *, long);

static segT qdsp6_sdata_section, qdsp6_sbss_section;
static asection qdsp6_scom_section;
static asymbol  qdsp6_scom_symbol;

static qdsp6_literal *qdsp6_pool;
static size_t qdsp6_pool_counter;

/* Special insns created by GAS. */
static qdsp6_packet_insn qdsp6_nop_insn, qdsp6_kext_insn;

const pseudo_typeS md_pseudo_table [] =
{
  { "align", s_align_bytes, 0 }, /* Defaulting is invalid (0).  */
  { "comm", qdsp6_common, 0 },
  { "common", qdsp6_common, 0 },
  { "lcomm", qdsp6_common, 1 },
  { "lcommon", qdsp6_common, 1 },
  { "2byte", cons, 2 },
  { "half", cons, 2 },
  { "short", cons, 2 },
  { "3byte", cons, 3 },
  { "4byte", cons, 4 },
  { "word", cons, 4 },
  { "option", qdsp6_option, 0 },
  { "cpu", qdsp6_option, 0 },
  { "block", s_space, 0 },
  { "file", (void (*) PARAMS ((int))) dwarf2_directive_file, 0 },
  { "loc", dwarf2_directive_loc, 0 },
  { "falign", qdsp6_falign, 0 },
  { NULL, 0, 0 },
};

/* Valid characters that make up a symbol. */
const char qdsp6_symbol_chars [] = "";

/* This array holds the chars that always start a comment.  If the
   pre-processor is disabled, these aren't very useful.  */
const char comment_chars [] = "";

/* This array holds the chars that only start a comment at the beginning of
   a line.  If the line seems to have the form '# 123 filename'
   .line and .file directives will appear in the pre-processed output */
/* Note that input_file.c hand checks for '#' at the beginning of the
   first line of the input file.  This is because the compiler outputs
   #NO_APP at the beginning of its output.  */
/* Also note that comments started like this one will always
   work if '/' isn't otherwise defined.  */
const char line_comment_chars [] = "#";

const char line_separator_chars [] = ";";

/* This array should perhaps contain "{}" so that the code handling them does
   not have to be duplicated in md_assemble (). */
const char qdsp6_parallel_separator_chars [] = "{}";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS [] = "eE";

/* Chars that mean this number is a floating point constant
   As in 0f12.456 or 0d1.2345e12.  */
const char FLT_CHARS [] = "rRsSfFdD";

/* Byte order.  */
extern int target_big_endian;
const char *qdsp6_target_format = DEFAULT_TARGET_FORMAT;
static int byte_order = DEFAULT_BYTE_ORDER;

static int qdsp6_mach_type = bfd_mach_qdsp6_v2;
static int cmdline_set_qdsp6_mach_type = 0;

/* Non-zero if the cpu type has been explicitly specified.  */
static int mach_type_specified_p = 0;

/* Non-zero if opcode tables have been initialized.
   A .option command must appear before any instructions.  */
static int cpu_tables_init_p = 0;

const char *md_shortopts = "G:";
struct option md_longopts [] =
  {
#define OPTION_EB (OPTION_MD_BASE + 0)
    { "EB", no_argument, NULL, OPTION_EB },
#define OPTION_EL (OPTION_MD_BASE + 1)
    { "EL", no_argument, NULL, OPTION_EL },
#define OPTION_QDSP6_PAIR_INFO (OPTION_MD_BASE + 2)
    { "mpairing-info", no_argument, NULL, OPTION_QDSP6_PAIR_INFO },
#define OPTION_QDSP6_FALIGN_INFO_NEW (OPTION_MD_BASE + 3)
    { "mfalign-info", no_argument, NULL, OPTION_QDSP6_FALIGN_INFO_NEW },
#define OPTION_QDSP6_FALIGN_MORE_INFO (OPTION_MD_BASE + 4)
    { "mfalign-more-info", no_argument, NULL, OPTION_QDSP6_FALIGN_MORE_INFO },
#define OPTION_QDSP6_NO_2MEMORY (OPTION_MD_BASE + 5)
    { "mno-dual-memory", no_argument, NULL, OPTION_QDSP6_NO_2MEMORY },
/* Code in md_parse_option () assumes that the -mv* options, are sequential. */
#define OPTION_QDSP6_MQDSP6V2 (OPTION_MD_BASE + 6)
    { "mv2", no_argument, NULL, OPTION_QDSP6_MQDSP6V2 },
#define OPTION_QDSP6_MQDSP6V3 (OPTION_MD_BASE + 7)
    { "mv3", no_argument, NULL, OPTION_QDSP6_MQDSP6V3 },
#define OPTION_QDSP6_MQDSP6V4 (OPTION_MD_BASE + 8)
    { "mv4", no_argument, NULL, OPTION_QDSP6_MQDSP6V4 },
#define OPTION_QDSP6_MARCH (OPTION_MD_BASE + 9)
    { "march", required_argument, NULL, OPTION_QDSP6_MARCH },
#define OPTION_QDSP6_MCPU (OPTION_MD_BASE + 10)
    { "mcpu", required_argument, NULL, OPTION_QDSP6_MCPU },
#define OPTION_QDSP6_MSORT_SDA (OPTION_MD_BASE + 11)
    { "msort-sda", no_argument, NULL, OPTION_QDSP6_MSORT_SDA },
#define OPTION_QDSP6_MNO_SORT_SDA (OPTION_MD_BASE + 12)
    { "mno-sort-sda", no_argument, NULL, OPTION_QDSP6_MNO_SORT_SDA },
#define OPTION_QDSP6_MNO_EXTENDER (OPTION_MD_BASE + 13)
    { "mno-extender", no_argument, NULL, OPTION_QDSP6_MNO_EXTENDER },
#define OPTION_QDSP6_MNO_PAIRING (OPTION_MD_BASE + 14)
    { "mno-pairing", no_argument, NULL, OPTION_QDSP6_MNO_PAIRING },
#define OPTION_QDSP6_MNO_PAIRING_B (OPTION_MD_BASE + 15)
    { "mno-pairing-branch", no_argument, NULL, OPTION_QDSP6_MNO_PAIRING_B },
#define OPTION_QDSP6_MNO_PAIRING_2 (OPTION_MD_BASE + 16)
    { "mno-pairing-duplex", no_argument, NULL, OPTION_QDSP6_MNO_PAIRING_2 },
#define OPTION_QDSP6_MNO_JUMPS (OPTION_MD_BASE + 17)
    { "mno-jumps", no_argument, NULL, OPTION_QDSP6_MNO_JUMPS },
  };
size_t md_longopts_size = sizeof (md_longopts);

#define IS_SYMBOL_OPERAND(o) \
 ((o) == 'b' || (o) == 'c' || (o) == 's' || (o) == 'o' || (o) == 'O')

/* Relax states. */
typedef enum _qdsp6_relax_state
  {
    /* Matching the respective entries in qdsp6_relax_table. */
    QDSP6_RELAX_NONE = 0,
    /* Relax state for BFD_RELOC_QDSP6_B9_PCREL. */
    QDSP6_RELAX_B9 = 1,
    /* Other odd-numbered relax states go here. */
    /* Done relaxing. */
    QDSP6_RELAX_DONE
  } qdsp6_relax_state;

/* Encode relax state from relocation type. */
#define ENCODE_RELAX(R) \
  ({qdsp6_relax_state r;\
    if ((R) == BFD_RELOC_QDSP6_B9_PCREL) \
      r = QDSP6_RELAX_B9; \
    else \
      r = QDSP6_RELAX_NONE; \
    r;})

#define QDSP6_RELAXED(R) ((R) + 1)

#define QDSP6_RANGE(B) (~(~0L << ((B) - 1)) \
                        & -(2 * MAX_PACKET_INSNS * QDSP6_INSN_LEN))

/* State table for relaxing branches.
   Note that since an extender is used, the insn is moved up,
   so the limits are offset by that. */
const struct relax_type qdsp6_relax_table [] =
  {
    /* Dummy entry. */
    {0L, 0L,
     0, 0},
    /* Entries for BFD_RELOC_QDSP6_B9_PCREL. */
    {QDSP6_RANGE (11), -QDSP6_RANGE (11),
     0, QDSP6_RELAXED (QDSP6_RELAX_B9)},
    {0L, 0L,
     QDSP6_INSN_LEN, QDSP6_RELAX_NONE},
    /* Entries for other relocations go here. */
  };

static int qdsp6_autoand = TRUE;
static int qdsp6_extender = TRUE;
static int qdsp6_pairing  = TRUE,
           qdsp6_pairing_branch = TRUE,
           qdsp6_pairing_duplex = TRUE;
static int qdsp6_relax = TRUE;
static int qdsp6_sort_sda = TRUE;
static int qdsp6_fetch_align = TRUE;

static int qdsp6_falign_info; /* Report statistics about .falign usage. */
static int qdsp6_falign_more; /* Report more statistics about .falign. */
static int qdsp6_pairs_info;   /* Report statistics about pairings. */

static size_t qdsp6_gp_size = QDSP6_SMALL_GPSIZE;
static int qdsp6_no_dual_memory = FALSE;

static int qdsp6_in_packet;

qdsp6_packet qdsp6_packets [MAX_PACKETS]; /* Includes current packet. */

static int qdsp6_faligning; /* 1 => .falign next packet we see */
static int qdsp6_falign_info; /* 1 => report statistics about .falign usage */
static int qdsp6_falign_more; /* report more statistics about .falign. */
static char *falign_file;
static unsigned falign_line;

static size_t n_falign [QDSP6_FALIGN_COUNTERS]; /* .falign statistics. */
static size_t n_pairs  [QDSP6_PAIRS_COUNTERS];   /* Pairing statistics. */

// Arrays to keep track of register writes
static qdsp6_reg_score gArray [QDSP6_NUM_GENERAL_PURPOSE_REGS],
                       cArray [QDSP6_NUM_CONTROL_REGS],
                       sArray [QDSP6_NUM_SYS_CTRL_REGS],
                       pArray [QDSP6_NUM_PREDICATE_REGS],
// To keep track of register reads
           pNewArray [QDSP6_NUM_PREDICATE_REGS],
           pLateArray [QDSP6_NUM_PREDICATE_REGS];

static int implicit_sr_ovf_bit_flag;  /* keeps track of the ovf bit in SR */
static int numOfBranchAddr;
static int numOfBranchRelax;
static int numOfBranchAddrMax1;
static int numOfBranchMax1;
static int numOfLoopMax1;

// To support --march options
struct qdsp6_march
  {
	char *march_name_fe, *march_short_fe;
	unsigned int march_name_be;
  };

static struct qdsp6_march qdsp6_marchs [] =
  {
    {"qdsp6v2", "v2", bfd_mach_qdsp6_v2},
    {"qdsp6v3", "v3", bfd_mach_qdsp6_v3},
    {"qdsp6v4", "v4", bfd_mach_qdsp6_v4},
  };

static size_t qdsp6_marchs_size =
  sizeof (qdsp6_marchs) / sizeof (*qdsp6_marchs);

/* Invocation line includes a switch not recognized by the base assembler.
   See if it's a processor-specific option.  */
int
md_parse_option
(int c, char *arg)
{
  unsigned int i;
  int temp_qdsp6_mach_type = 0;

  switch (c)
    {
    case OPTION_EB:
      byte_order = BIG_ENDIAN;
      qdsp6_target_format = "elf32-bigqdsp6";
      break;

    case OPTION_EL:
      byte_order = LITTLE_ENDIAN;
      qdsp6_target_format = "elf32-littleqdsp6";
      break;

    case OPTION_QDSP6_PAIR_INFO:
      qdsp6_pairs_info = TRUE;
      break;

    case OPTION_QDSP6_FALIGN_MORE_INFO:
      qdsp6_falign_more = TRUE;
      /* Fall through. */

    case OPTION_QDSP6_FALIGN_INFO_NEW:
      qdsp6_falign_info = TRUE;
      break;

    case OPTION_QDSP6_MQDSP6V2:
    case OPTION_QDSP6_MQDSP6V3:
    case OPTION_QDSP6_MQDSP6V4:
    case OPTION_QDSP6_MARCH:
    case OPTION_QDSP6_MCPU:
      switch (c)
        {
          case OPTION_QDSP6_MQDSP6V2:
          case OPTION_QDSP6_MQDSP6V3:
          case OPTION_QDSP6_MQDSP6V4:
            /* -mv* options. */
            temp_qdsp6_mach_type
              = qdsp6_marchs [c - OPTION_QDSP6_MQDSP6V2].march_name_be;
            break;

          default:
            /* -march and- mcpu options. */
            for (i = 0; i < qdsp6_marchs_size; i++)
              if (!strcmp (arg, qdsp6_marchs [i].march_name_fe)
                  || !strcmp (arg, qdsp6_marchs [i].march_short_fe))
                {
                  temp_qdsp6_mach_type = qdsp6_marchs [i].march_name_be;
                  break;
                }

            if (i == qdsp6_marchs_size)
              as_fatal (_("invalid architecture specified."));
            break;
        }

      if (cmdline_set_qdsp6_mach_type)
        {
          if (qdsp6_mach_type != temp_qdsp6_mach_type)
            as_fatal (_("conflicting architectures specified."));
        }

      qdsp6_mach_type = temp_qdsp6_mach_type;
      cmdline_set_qdsp6_mach_type = TRUE;
      break;

    case 'G':
      {
        int x;

        x = atoi (arg);

        if (x < 0)
          as_warn (_("invalid value for \"-%c\"; defaulting to %d."),
                   c, x = QDSP6_SMALL_GPSIZE);
        else
          qdsp6_gp_size = x;
      }
      break;

    case OPTION_QDSP6_MSORT_SDA:
      qdsp6_sort_sda = TRUE;
      break;

    case OPTION_QDSP6_MNO_SORT_SDA:
      qdsp6_sort_sda = FALSE;
      break;

    case OPTION_QDSP6_NO_2MEMORY:
      qdsp6_no_dual_memory = TRUE;
      break;

    case OPTION_QDSP6_MNO_EXTENDER:
      qdsp6_extender = FALSE;
      break;

    case OPTION_QDSP6_MNO_PAIRING:
      qdsp6_pairing = qdsp6_pairing_branch = qdsp6_pairing_duplex = FALSE;
      break;

    case OPTION_QDSP6_MNO_PAIRING_B:
      qdsp6_pairing_branch = FALSE;
      break;

    case OPTION_QDSP6_MNO_PAIRING_2:
      qdsp6_pairing_duplex = FALSE;
      break;

    case OPTION_QDSP6_MNO_JUMPS:
      qdsp6_relax = FALSE;
      break;

    default:
      return FALSE;
    }
  return TRUE;
}

void
md_show_usage (
     FILE *stream
)
{
  fprintf (stream, "\
QDSP6 Options:\n\
  -EB                      select big-endian output\n\
  -EL                      select little-endian ouptut (default)\n\
  -G SIZE                  small-data size limit (default is %d)\n\
  -mfalign-info            report \".falign\" statistics\n\
  -mno-extender            disable the use of constant extenders\n\
  -mno-jumps               disable automatic extension of branch instructions\n\
  -mno-pairing             disable pairing of packet instructions\n\
  -mno-pairing-duplex      disable pairing to duplex instructions\n\
  -mno-pairing-branch      disable pairing of branch instructions\n\
  -mpairing-info           report instruction pairing statistics\n\
  -msort-sda               enable sorting the small-data area (default)\n\
  -mv2                     assemble code for the QDSP6 V2 architecture (default)\n\
  -mv3                     assemble code for the QDSP6 V3 architecture\n\
  -mv4                     assemble code for the QDSP6 V4 architecture\n\
  -march={v2|v3|v4}        assemble code for the specified QDSP6 architecture\n\
  -mcpu={v2|v3|v4}         equivalent to \"-march\"\n",
           QDSP6_SMALL_GPSIZE);
}

/* This function is called once, at assembler startup time.  It should
   set up all the tables, etc. that the MD part of the assembler will need.
   Opcode selection is deferred until later because we might see a .option
   command.  */

void
md_begin ()
{
  segT current_section = now_seg;
  int  current_subsec  = now_subseg;
  flagword applicable;

  /* The endianness can be chosen "at the factory".  */
  target_big_endian = byte_order == BIG_ENDIAN;

  if (!bfd_set_arch_mach (stdoutput, bfd_arch_qdsp6, qdsp6_mach_type))
    as_warn (_("architecture and machine types not set; using default settings."));

  /* This call is necessary because we need to initialize `qdsp6_operand_map'
     which may be needed before we see the first insn.  */
  qdsp6_opcode_init_tables
    (qdsp6_get_opcode_mach (qdsp6_mach_type, target_big_endian));

  /* Initialize global NOP. */
  qdsp6_find_nop ();
  /* Initialize global k-extension. */
  qdsp6_find_kext ();

  /* Set the default alignment for the default sections. */
  record_alignment (text_section, DEFAULT_CODE_ALIGNMENT);

  if (!qdsp6_if_arch_v1 ())
    {
      /* QDSP6 V2 */
      applicable = bfd_applicable_section_flags (stdoutput);

      /* Create the sdata section. */
      qdsp6_sdata_section = subseg_new (SMALL_DATA_SECTION, 0);
      bfd_set_section_flags
        (stdoutput, qdsp6_sdata_section,
         applicable & (  SEC_ALLOC | SEC_LOAD | SEC_RELOC
                       | SEC_DATA | SEC_SMALL_DATA));
      record_alignment (qdsp6_sdata_section, DEFAULT_DATA_ALIGNMENT);
      symbol_table_insert (section_symbol (qdsp6_sdata_section));

      /* Create the sbss section. */
      qdsp6_sbss_section = subseg_new (SMALL_BSS_SECTION, 0);
      bfd_set_section_flags
        (stdoutput, qdsp6_sbss_section,
         applicable & (SEC_ALLOC | SEC_SMALL_DATA));
      seg_info (qdsp6_sbss_section)->bss = TRUE;
      record_alignment (qdsp6_sbss_section, DEFAULT_DATA_ALIGNMENT);
      symbol_table_insert (section_symbol (qdsp6_sbss_section));

      /* We must construct a fake section similar to bfd_com_section,
        but with the name .scommon.  */
      qdsp6_scom_section                = bfd_com_section;
      qdsp6_scom_section.name           = SMALL_COM_SECTION;
      qdsp6_scom_section.flags         |= SEC_SMALL_DATA;
      qdsp6_scom_section.output_section = &qdsp6_scom_section;
      qdsp6_scom_section.symbol         = &qdsp6_scom_symbol;
      qdsp6_scom_section.symbol_ptr_ptr = &qdsp6_scom_section.symbol;

      qdsp6_scom_symbol                 = *bfd_com_section.symbol;
      qdsp6_scom_symbol.name            = SMALL_COM_SECTION;
      qdsp6_scom_symbol.section         = &qdsp6_scom_section;
    }
  else
    /* QDSP6 V1 */
    qdsp6_gp_size = 0;

  /* Set the GP size. */
  bfd_set_gp_size (stdoutput, qdsp6_gp_size);

  /* Reset default section. */
  subseg_set (current_section, current_subsec);
}

int
qdsp6_relax_branch
(fragS *fragP)
{
  qdsp6_packet *apacket;
  qdsp6_operand *operand;
  fixS *fx, fxup;
  int fix;
  size_t i, j;

  fix = fragP->fr_fix;

  apacket = &fragP->tc_frag_data->packet;
  if (!fragP->fr_subtype || !apacket->relax || !apacket->drlx)
    return FALSE;

  /* Sanity check. */
  assert (apacket->drlx + qdsp6_packet_size (apacket) - apacket->relax
          <= MAX_PACKET_INSNS);

  for (i = 0, fx = NULL; i < apacket->size; i++)
    if (apacket->insns [i].flags & QDSP6_INSN_IS_RELAX
        && apacket->insns [i].fc)
      {
        fx = apacket->insns [i].fix;
        break;
      }

  if (!fx)
    return FALSE;

  fxup = *fx;
  operand = fx->tc_fix_data;

  /* Adjusted extended insn. */
  operand->flags |= QDSP6_OPERAND_IS_KXED;
  apacket->insns [i].flags |= QDSP6_INSN_IS_KXED;
  fx->fx_r_type = operand->reloc_kxed;

  /* Make room for extender. */
  for (j = apacket->size; j > i; j--)
    {
      qdsp6_insn *pi;

      pi = ((qdsp6_insn *) fragP->fr_literal) + j;
      pi [0] = pi [-1];

      apacket->insns [j] = apacket->insns [j - 1];
      if (apacket->insns [j].fc && apacket->insns [j].fix)
        {
          apacket->insns [j].fix->fx_where += QDSP6_INSN_LEN;
          apacket->insns [j].fix->fx_offset
            += apacket->insns [j].fix->fx_pcrel? QDSP6_INSN_LEN: 0;
        }
    }

  /* Create extender. */
  apacket->insns [i] = qdsp6_kext_insn;
  apacket->insns [i].operand
    = *(qdsp6_operand *) qdsp6_lookup_reloc (operand->reloc_kxer, 0, apacket->insns [i].opcode);
  apacket->insns [i].fc++;
  apacket->insns [i].fix
    = fix_new (fragP, fxup.fx_where, QDSP6_INSN_LEN,
               fxup.fx_addsy, fxup.fx_offset, fxup.fx_pcrel, operand->reloc_kxer);
  apacket->insns [i].fix->fx_line = fxup.fx_line;
  apacket->insns [i].fix->tc_fix_data = &apacket->insns [i].operand;

  /* Add extender. */
  md_number_to_chars (fragP->fr_literal + i * QDSP6_INSN_LEN,
                      apacket->insns [i].insn, QDSP6_INSN_LEN);
  apacket->size++;
  fragP->fr_subtype = QDSP6_RELAX_DONE;

  fragP->fr_fix += QDSP6_INSN_LEN;
  return (fragP->fr_fix - fix);
}

/** Relax branch by extending it and adjusting it to accomodate the extension.

@param fragP Relaxable fragment with room for the extension.
*/

long
qdsp6_relax_branch_try
(fragS *fragP, segT segment, long stretch)
{
  const relax_typeS *before, *now;
  relax_substateT this = QDSP6_RELAX_NONE, next = QDSP6_RELAX_NONE;
  qdsp6_packet *apacket;
  symbolS *sym;
  addressT from, to;
  offsetT aim;
  long growth;
  size_t i;

  apacket = &fragP->tc_frag_data->packet;

  if (!fragP->fr_subtype || !apacket->relax)
    return FALSE;

  for (i = 0, sym = NULL; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].flags & QDSP6_INSN_IS_RELAX
        && apacket->insns [i].fc)
      {
        sym = apacket->insns [i].fix->fx_addsy;
        break;
      }

  if (sym)
    {
      to = S_GET_VALUE (sym);

      if (stretch
          && symbol_get_frag (sym)->relax_marker != fragP->relax_marker
          && S_GET_SEGMENT (sym) == segment)
        to += stretch;
    }
  else
    to = 0;

  if (apacket->insns [i].fc)
    to += apacket->insns [i].fix->fx_offset;

  before = now = qdsp6_relax_table + fragP->fr_subtype;

  from = fragP->fr_address;
  aim  = to - from;
  for (next = now->rlx_more; next; )
    if ((aim <  0 && aim >= now->rlx_backward)
        || (aim >= 0 && aim <= now->rlx_forward))
      next = 0;
    else
      {
        /* Grow to next state.  */
        this = next;
        now = qdsp6_relax_table + this;
        next = now->rlx_more;
      }

  growth = now->rlx_length - before->rlx_length;
  if (growth)
    {
      fragP->fr_subtype = this;
      apacket->drlx   = now->rlx_length / QDSP6_INSN_LEN;
      apacket->ddpad -= apacket->dpad? apacket->drlx: 0;
    }

  return (growth);
}

/**

Process the resulting padding delta.
*/
int
qdsp6_relax_falign
(fragS *fragP)
{
  qdsp6_packet *apacket, packet;
  size_t after, i;

  apacket = &fragP->tc_frag_data->packet;
  if (!apacket->dpad && !apacket->dpkt)
    return FALSE;

  /* Sanity checks. */
  assert (apacket->dpad + qdsp6_packet_size (apacket) - apacket->relax
          <= MAX_PACKET_INSNS);
  assert (apacket->dpkt <= MAX_PACKET_INSNS);

  packet = *apacket;
  packet.relax = FALSE; /* At this point, it won't be extended. */
  packet.dpad = apacket->dpad;
  packet.dpkt = apacket->dpkt;

  if (packet.dpad)
    {
      qdsp6_packet_unfold (&packet);

      /* Pad the packet. */
      while (packet.dpad)
        {
          if (!(qdsp6_packet_insert (&packet, &qdsp6_nop_insn, NULL, NULL, FALSE)))
            packet.dpkt++;
          packet.dpad--;
        }

      /* Try to shuffle modified packet. */
      if (qdsp6_shuffle_helper (&packet, 0, NULL))
        {
          qdsp6_packet_fold (&packet);
          after = packet.size;

          /* Re-emit packet. */
          for (i = 0; i < packet.size; i++, after = i)
            {
              packet.insns [i].insn = QDSP6_END_PACKET_RESET (packet.insns [i].insn);

              if (i == 0 && packet.is_inner)
                packet.insns [i].insn =
                  QDSP6_END_PACKET_SET (packet.insns [i].insn, QDSP6_END_LOOP);
              else if (i == 1 && packet.is_outer)
                packet.insns [i].insn =
                  QDSP6_END_PACKET_SET (packet.insns [i].insn, QDSP6_END_LOOP);
              else if (i >= packet.size - 1
                      && QDSP6_END_PACKET_GET (packet.insns [i].insn) != QDSP6_END_PAIR)
                packet.insns [i].insn =
                  QDSP6_END_PACKET_SET (packet.insns [i].insn, QDSP6_END_PACKET);

              md_number_to_chars ((void *)(((qdsp6_insn *) fragP->fr_literal) + i),
                                  packet.insns [i].insn, QDSP6_INSN_LEN);
            }
        }
      else
        {
          qdsp6_packet_fold (&packet);
          after = apacket->size;

          packet.dpkt = apacket->dpkt + apacket->dpad;
        }
    }
  else
    after = apacket->size;

  /* Emit padding packet. */
  while (packet.dpkt)
    {
      if (packet.dpkt % MAX_PACKET_INSNS != 1)
        md_number_to_chars ((void *)(((qdsp6_insn *) fragP->fr_literal) + after++),
                            qdsp6_nop_insn.insn, QDSP6_INSN_LEN);
      else
        md_number_to_chars ((void *)(((qdsp6_insn *) fragP->fr_literal) + after++),
                            QDSP6_END_PACKET_SET (qdsp6_nop_insn.insn, QDSP6_END_PACKET),
                            QDSP6_INSN_LEN);
      packet.dpkt--;
    }

  fragP->fr_fix += (apacket->dpad + apacket->dpkt) * QDSP6_INSN_LEN;
  return TRUE;
}

/**

Determine if packet is not fetch-aligned and then request previous packets to
grow through padding NOPs, if possible or insert a NOP-packet.
*/
long
qdsp6_relax_falign_try
(fragS *fragP, segT segment ATTRIBUTE_UNUSED, long stretch ATTRIBUTE_UNUSED)
{
  qdsp6_packet *apacket, *bpacket, *zpacket;
  fragS *previous;
  addressT first, next;
  size_t size, delta, over, left, room;

  apacket = &fragP->tc_frag_data->packet;
  bpacket = fragP->tc_frag_data->previous
            ? &fragP->tc_frag_data->previous->tc_frag_data->packet: NULL;

  delta = 0;

  size  = apacket->size + apacket->dpad + apacket->drlx;
  first = (fragP->fr_address / QDSP6_INSN_LEN) % MAX_PACKET_INSNS;
  next  = first + size;

  /* Check fetch-alignment. */
  over = next > MAX_PACKET_INSNS? next % MAX_PACKET_INSNS: 0;
  if (over)
    {
      /* Check if not fetch-aligned because of padding. */
      size           = MIN (over, apacket->dpad);
      delta         -= size;
      over          -= size;
      apacket->dpad -= size;

      /* Skim excessive NOP-packet padding. */
      if (over && bpacket && bpacket->dpkt)
        {
          size            = MIN (over, bpacket->dpkt);
          first          -= size;
          over           -= size;
          bpacket->ddpkt -= size;
        }

      /* Skim excessive padding. */
      if (over && bpacket && bpacket->dpad)
        {
          size            = MIN (over, bpacket->dpad);
          first          -= size;
          over           -= size;
          bpacket->ddpad -= size;
        }

      /* Check if still not fetch-aligned. */
      if (over)
        {
          left = MAX_PACKET_INSNS - first;
          for (previous = fragP->tc_frag_data->previous;
               previous && left;
               previous = previous->tc_frag_data->previous)
            {
              zpacket = &previous->tc_frag_data->packet;

              if (previous->fr_type == rs_fill)
                /* Skip standard frags. */
                continue;
              else if (previous->fr_type != rs_machine_dependent)
                /* Other frags must not be trespassed. */
                break;

              if (left)
                {
                  size = qdsp6_packet_size (zpacket) - zpacket->relax
                         + zpacket->dpad + zpacket->ddpad
                         + zpacket->drlx + zpacket->ddrlx;
                  assert (size <= MAX_PACKET_INSNS);

                  if (zpacket->faligned)
                    {
                      /* The room in a fetch-aligned packet must be
                        within its fetch window. */
                      next = (previous->fr_address / QDSP6_INSN_LEN + size)
                             % MAX_PACKET_INSNS;
                      room = MAX_PACKET_INSNS - MAX (size, next);
                    }
                  else
                    room = MAX_PACKET_INSNS - size;

                  if (room)
                    {
                      zpacket->ddpad += MIN (left, room);
                      left           -= MIN (left, room);
                    }

                  size = qdsp6_packet_size (zpacket) - zpacket->relax
                         + zpacket->dpad + zpacket->ddpad
                         + zpacket->drlx + zpacket->ddrlx;
                  assert (size <= MAX_PACKET_INSNS);
                }

              if (zpacket->faligned)
                /* Cannot go past a fetch-aligned packet. */
                break;
            }

          if (left && bpacket)
            /* Force fetch-alignment by inserting a NOP-packet. */
            bpacket->ddpkt += left;
        }
    }
  else if (bpacket && bpacket->dpkt)
    /* Remove excess from NOP-packet if no fetch-window crossing. */
    bpacket->ddpkt -= MIN (bpacket->dpkt, first);
  else if (bpacket && bpacket->dpad)
    /* Remove excess padding if no fetch-window crossing. */
    bpacket->ddpad -= MIN (bpacket->dpad, first);

  return (QDSP6_INSN_LEN * (int) delta);
}

/**

Extend some branches when otherwise the destination is out of reach.
*/

int
qdsp6_estimate_size_before_relax
(fragS *fragP, segT segment)
{
  qdsp6_packet *apacket;
  size_t ddpad, ddpkt;
  int delta;

  /* Avoid the empty fragments that result from frag_more (). */
  if (!fragP->fr_fix)
    {
      frag_wane (fragP);
      return FALSE;
    }

  apacket = &fragP->tc_frag_data->packet;

  ddpad = apacket->dpad + apacket->ddpad <= SSIZE_MAX
          ? apacket->ddpad: -apacket->dpad;
  ddpkt = apacket->dpkt + apacket->ddpkt <= SSIZE_MAX
          ? apacket->ddpkt: -apacket->dpkt;

  apacket->dpad += ddpad;
  apacket->dpkt += ddpkt;
  apacket->ddpad = apacket->ddpkt = 0;
  delta = QDSP6_INSN_LEN * (apacket->dpad + apacket->dpkt);

  if (apacket->relax)
    {
      symbolS *sym;
      size_t i;

      for (i = 0, sym = NULL; i < MAX_PACKET_INSNS; i++)
        if (apacket->insns [i].flags & QDSP6_INSN_IS_RELAX
            && apacket->insns [i].fc)
          {
            sym = apacket->insns [i].fix->fx_addsy;
            break;
          }

      if (sym)
        {
          if (fragP->fr_subtype
              || ((fragP->fr_subtype
                   = ENCODE_RELAX (apacket->insns [i].operand.reloc_type))))
            {
              if (S_GET_SEGMENT (sym) != segment
                  || (OUTPUT_FLAVOR == bfd_target_elf_flavour
                      && ((S_IS_EXTERNAL (sym) && !S_IS_DEFINED (sym))
                          || S_IS_WEAK (sym))))
                /* Symbol is external, in another segment, or we need to keep a
                  relocation so that weak symbols can be overridden.  Regardless, add
                  the extender. */
                {
                  apacket->drlx++;
                  delta += qdsp6_relax_branch (fragP);
                  frag_wane (fragP);
                }
              else
                delta += qdsp6_relax_table [fragP->fr_subtype].rlx_length;
            }
        }
    }
  else if (apacket->faligned)
    /* !!! */
    apacket->faligned = apacket->faligned;

  return (delta);
}

long
qdsp6_relax_frag (segT segment, fragS *fragP, long stretch)
{
  qdsp6_packet *apacket;
  size_t ddpad, ddpkt;
  long delta;

  apacket = &fragP->tc_frag_data->packet;

  ddpad = apacket->dpad + apacket->ddpad <= SSIZE_MAX
          ? apacket->ddpad: -apacket->dpad;
  ddpkt = apacket->dpkt + apacket->ddpkt <= SSIZE_MAX
          ? apacket->ddpkt: -apacket->dpkt;
  delta = QDSP6_INSN_LEN * (ddpad + ddpkt);

  apacket->dpad += ddpad;
  apacket->dpkt += ddpkt;
  apacket->ddpad = apacket->ddpkt = 0;

  if (apacket->relax)
    delta += qdsp6_relax_branch_try (fragP, segment, stretch);
  else if (apacket->faligned)
    delta += qdsp6_relax_falign_try (fragP, segment, stretch);

  return (delta);
}

void
qdsp6_convert_frag
(bfd *abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED, fragS *fragP)
{
  qdsp6_packet *apacket;

  apacket = &fragP->tc_frag_data->packet;

  if (apacket->drlx)
    qdsp6_relax_branch (fragP);
  if (apacket->dpad || apacket->dpkt)
    qdsp6_relax_falign (fragP);
}

/* We need to distinguish a register pair name (e.g., r1:0 or p3:0)
   from a label. */
int qdsp6_start_label
(char c, char *before, char *after)
{
  static int re_ok;
  static const char *ex_before;
  static const char ex_before_legacy [] =
    REGEX_LEFT ("((r((0*[12]?[13579])|31))|sp|lr|p3)$");
  static const char ex_before_pairs [] =
    REGEX_LEFT ("(((c|g|r|s)((0*[12]?[13579])|31))|sp|lr|p3)$");
  static const char ex_after [] =
    REGEX_RITE ("^((0*[12]?[02468])|30|fp)");
  static regex_t re_before, re_after;
  int er_before, er_after;

  /* Labels require a colon. */
  if (c != ':')
    return FALSE;

  /* The reader puts '\0' where the colon was. */
  after++;

  if (!re_ok)
    {
      ex_before = qdsp6_if_arch_pairs ()? ex_before_pairs: ex_before_legacy;

      /* Compile RE for GPR or predicate pairs. */
      assert (!regcomp (&re_before, ex_before, REG_EXTENDED | REG_ICASE | REG_NOSUB));
      assert (!regcomp (&re_after,  ex_after,  REG_EXTENDED | REG_ICASE | REG_NOSUB));

      re_ok = TRUE;
    }

  /* Register pairs are not labels. */
  if ( !(er_before = regexec (&re_before, before, 0, NULL, 0))
       && !(er_after  = regexec (&re_after,  after,  0, NULL, 0)))
    return FALSE;

  /* After everything else has been tried, this must be a label,
     but if it's inside a packet,
     its address might not be what one expects. */
  if (qdsp6_in_packet)
    {
      /* Commented out because the compiler may emmit some labels inside packets
         used for debug information and for EH. */
      /*
      as_warn (_("label inside packet assigned address of packet."));
	return FALSE;
      */
    }

  /* This IS a label. */
  return TRUE;
}

addressT
qdsp6_frag_fix_addr
(void)
{
  fragS *fragP;
  addressT addr;

  for (fragP  = frchain_now->frch_root, addr = 0;
       fragP != frag_now;
       fragP  = fragP->fr_next)
    switch (fragP->fr_type)
      {
        case rs_org: /* Reset position. */
          addr = fragP->fr_offset;
          break;

        case rs_align_code: /* Adjust position. */
          /* First check if within limits, if any. */
          if (!fragP->fr_subtype
              || (  (1 << fragP->fr_offset)
                  - (addr & ~(-1 << fragP->fr_offset))
                  <= fragP->fr_subtype))
            addr = (addr + ~(-1 << fragP->fr_offset))
                 & (-1 << fragP->fr_offset);
          break;

        default: /* Increment position. */
          addr += fragP->fr_fix;
          break;
      }

  addr += frag_now_fix_octets ();

  return (addr / OCTETS_PER_BYTE);
}

/* Initialize the various opcode and operand tables. */
void
qdsp6_init
(int mach)
{
  static int init_p;

  if (!init_p)
    {
      if (!bfd_set_arch_mach (stdoutput, bfd_arch_qdsp6, mach))
        as_warn (_("architecture and machine types not set; using default settings."));

      /* This initializes a few things in qdsp6-opc.c that we need.
        This must be called before the various qdsp6_xxx_supported fns.  */
      qdsp6_opcode_init_tables (qdsp6_get_opcode_mach (mach, target_big_endian));

      /* Initialize global NOP. */
      qdsp6_find_nop ();
      /* Initialize global k-extension. */
      qdsp6_find_kext ();

      /* Initialize ISA-specific features. */
      qdsp6_autoand        &= qdsp6_if_arch_autoand ();
      qdsp6_extender       &= qdsp6_if_arch_kext ();
      qdsp6_pairing        &= qdsp6_if_arch_pairs ();
      qdsp6_pairing_branch &= qdsp6_pairing;
      qdsp6_pairing_duplex &= qdsp6_pairing;
      qdsp6_relax          &= qdsp6_extender & qdsp6_pairing_branch;

      /* Tell `.option' it's too late.  */
      cpu_tables_init_p = TRUE;

      init_p = TRUE;
    }
}

/* Insert an operand value into an instruction. */

void
qdsp6_insert_operand
(char *where, const qdsp6_operand *operand, offsetT val, fixS *fixP)
{
  qdsp6_insn insn;
  const qdsp6_opcode *opcode;
  long xval;
  char *errmsg = NULL;

  if (target_big_endian)
    insn = bfd_getb32 ((unsigned char *) where);
  else
    insn = bfd_getl32 ((unsigned char *) where);

  opcode = qdsp6_lookup_insn (insn);
  if (!opcode)
    as_bad ("opcode not found.");

  if (!qdsp6_encode_operand
         (operand, &insn, opcode, val,
          &xval, operand->flags & QDSP6_OPERAND_IS_KXED,
          TRUE, &errmsg))
    if (errmsg)
      {
        if (fixP && fixP->fx_file)
          {
            char tmpError [200];
  	    sprintf(tmpError, " when resolving symbol in file %s at line %d.", fixP->fx_file, fixP->fx_line);
            strcat (errmsg, tmpError);
          }

        as_bad (errmsg);
      }

  if (target_big_endian)
    bfd_putb32 ((bfd_vma) insn, (unsigned char *) where);
  else
    bfd_putl32 ((bfd_vma) insn, (unsigned char *) where);
}

/** Create a k-extend prefix.

@param kext Where to store the resulting insn.
@param xvalue The value to be used in the extension.

@return True if successful.
*/
int
qdsp6_prefix_kext
(qdsp6_packet_insn *kext, long xvalue)
{
  const qdsp6_operand *operand;
  char *syntax;

  if (!kext)
    return FALSE;

  *kext = qdsp6_kext_insn;

  for (syntax = kext->opcode->syntax;
       *syntax;
       syntax++)
    if (ISSPACE (*syntax))
      {
        while (*syntax && ISSPACE (*syntax))
          syntax++;

        break;
      }
  /* Skip the '('. */
  syntax++;
  while (*syntax && ISSPACE (*syntax))
    syntax++;

  operand = qdsp6_lookup_operand (syntax);
  assert (operand);

  /* Truncate to as many bits as in the extension. */
  xvalue &= ~(-1L << (operand->bits + operand->shift_count));
  if (qdsp6_encode_operand
        (operand, &kext->insn, kext->opcode, xvalue, NULL, FALSE, FALSE, NULL))
    {
      kext->used = TRUE;

      return TRUE;
    }

  return FALSE;
}

char *
qdsp6_parse_immediate
(qdsp6_packet_insn *insn, qdsp6_packet_insn *prefix, const qdsp6_operand *operand,
 char *str, long *val, char **errmsg)
{
  char *hold;
  expressionS exp;
  const qdsp6_operand *operandx;
  long value = 0;
  long xvalue = 0;
  int is_x = FALSE, is_relax = FALSE;
  int is_lo16 = FALSE, is_hi16 = FALSE;

  /* We only have the mandatory '#' for immediates that are NOT pc relative */
  if (*str == '#')
    {
      /* Skip over the 1st '#' */
      str++;
      if (*str == '#')
        {
          /* Skip over the 2nd '#' */
          str++;
          is_x = qdsp6_extender;
        }
    }
  else if (!(operand->flags & QDSP6_OPERAND_PC_RELATIVE))
    return NULL;

  is_relax = qdsp6_relax && ENCODE_RELAX (operand->reloc_type);

  if (is_x && !insn->opcode->map)
    {
      /* Check if the operand can truly be extended. */
      if ((!(insn->opcode->attributes & EXTENDABLE_LOWER_CASE_IMMEDIATE)
              && !(insn->opcode->attributes & EXTENDABLE_UPPER_CASE_IMMEDIATE))
          || ((insn->opcode->attributes & EXTENDABLE_LOWER_CASE_IMMEDIATE)
              && !ISLOWER (operand->enc_letter))
          || ((insn->opcode->attributes & EXTENDABLE_UPPER_CASE_IMMEDIATE)
              && !ISUPPER (operand->enc_letter)))
        {
          if (errmsg)
            *errmsg = _("operand cannot be extended.");
          return NULL;
        }
    }

  /* QDSP6 TODO: Allow white space between lo/hi and the paren */
  if (TOLOWER (str [0]) == 'l' && TOLOWER (str [1]) == 'o' && str [2] == '(')
    {
      /* Make sure we have a corresponding lo16 operand */
      operand = qdsp6_operand_find_lo16 (operand);
      if (!operand)
        return NULL;

      is_lo16 = 1;
      str += 3;
    }
  else if (TOLOWER (str [0]) == 'h' && TOLOWER (str [1]) == 'i' && str [2] == '(')
    {
      /* Make sure we have a corresponding hi16 operand */
      operand = qdsp6_operand_find_hi16 (operand);
      if (!operand)
        return NULL;

      is_hi16 = 1;
      str += 3;
    }

  hold = input_line_pointer;
  input_line_pointer = str;
  expression (&exp);
  str = input_line_pointer;
  input_line_pointer = hold;

  if (is_lo16 || is_hi16)
    {
      while (ISSPACE (*str))
        str++;

      /* Check for closing paren */
      if (*str != ')')
        {
          if (errmsg)
            *errmsg = _("missing `)'.");
          return NULL;
        }
      else
        str++;
    }

  if (exp.X_op == O_illegal)
    {
      if (errmsg)
        *errmsg = _("illegal operand.");
      return NULL;
    }
  else if (exp.X_op == O_absent)
    {
      if (errmsg)
        *errmsg = _("missing operand.");
      return NULL;
    }
  else if (exp.X_op == O_register)
    {
      if (errmsg)
        *errmsg = _("unexpected register.");
      return NULL;
    }
  else if (exp.X_op == O_constant)
    {
      value = exp.X_add_number;

      if (!qdsp6_encode_operand
             (operand, &insn->insn, insn->opcode,
              value, is_x? &xvalue: NULL, is_x, FALSE, errmsg))
          return NULL;

      if (is_x || xvalue)
        /* Emit prefix only if requested or needed. */
        insn->flags |= qdsp6_prefix_kext (prefix, xvalue)? QDSP6_INSN_IS_KXED: 0;
    }
  else
    {
      if (prefix && is_x)
        {
          assert ((is_x = qdsp6_prefix_kext (prefix, 0)));

          operandx = qdsp6_lookup_reloc (operand->reloc_kxer, 0, prefix->opcode);
          if (operandx)
            {
              prefix->exp                = exp;
              prefix->operand            = *operandx;
              prefix->operand.reloc_kxer = operand->reloc_type;

              prefix->fc++;
            }
          else
            {
              if (errmsg)
                *errmsg = _("expression too complex.");
              return NULL;
            }
        }

      if ((is_x? operand->reloc_kxed: operand->reloc_type) == BFD_RELOC_NONE)
        {
          if (errmsg)
            *errmsg = _("unexpected non-constant expression.");
          return NULL;
        }

      /* This expression involves one or more symbols.
         Record a fixup to process later */
      if (insn->fc >= MAX_FIXUPS)
        if (errmsg)
          *errmsg = _("expression too complex.");

      insn->flags         |= (is_x? QDSP6_INSN_IS_KXED: 0)
                             | (is_relax? QDSP6_INSN_IS_RELAX: 0);
      insn->exp            = exp;
      insn->operand        = *operand;
      insn->operand.flags |= (is_x? QDSP6_OPERAND_IS_KXED: 0)
                             | (is_relax? QDSP6_OPERAND_IS_RELAX: 0);
      insn->fc++;
    }

  /* Return the value before extension, if any, so that aliased insns
     can pick up the original value. */
  *val = value;
  return (str);
}

/** Initialize an insn.
*/
void
qdsp6_insn_init
(qdsp6_packet_insn *ainsn)
{
  memset (ainsn, 0, sizeof (*ainsn));
  *ainsn = qdsp6_nop_insn;
  ainsn->padded = TRUE;
}

char*
qdsp6_insn_write
(qdsp6_insn insn, size_t fc, const qdsp6_operand *op, expressionS *exp,
 char *stream, size_t offset, fixS **fixSP, int lineno ATTRIBUTE_UNUSED)
{
  fixS *fixP = NULL;
  addressT pc;

  /* Create any fixups */
  if (fc)
    {
      bfd_reloc_code_real_type reloc_type;
      qdsp6_operand *operand;
      int var;

      operand = xmalloc (sizeof (*operand));
      *operand = *op;

      if (operand->flags & QDSP6_OPERAND_IS_KXED)
        reloc_type = operand->reloc_kxed;
      else
        reloc_type = operand->reloc_type;

      var = qdsp6_relax
            && (operand->flags & QDSP6_OPERAND_IS_RELAX)
            && ENCODE_RELAX (operand->reloc_type);

      dwarf2_emit_insn (QDSP6_INSN_LEN);
      pc = frag_now->fr_address + frag_now_fix ();

      fixP = fix_new_exp
        (frag_now, stream + offset - frag_now->fr_literal, QDSP6_INSN_LEN, exp,
         (operand->flags & QDSP6_OPERAND_PC_RELATIVE) == QDSP6_OPERAND_PC_RELATIVE,
          reloc_type);
      fixP->tc_fix_data = operand;

      if (operand->flags & (QDSP6_OPERAND_IS_LO16 | QDSP6_OPERAND_IS_HI16))
        fixP->fx_no_overflow = TRUE;

      if (operand->flags & QDSP6_OPERAND_PC_RELATIVE)
        /* Any PC relative operands within a packet must be relative
           to the address of the packet, so the offset is adjusted to
           account for that. */
        fixP->fx_offset += offset;
    }
  else
    {
      /* Allocate space for an insn. */
      dwarf2_emit_insn (QDSP6_INSN_LEN);
      pc = frag_now->fr_address + frag_now_fix ();
    }

  /* Write out the instruction. */
  md_number_to_chars (stream + offset, insn, QDSP6_INSN_LEN);

  if (pc % QDSP6_INSN_LEN)
    as_warn (_("current location is not %d-byte aligned."), QDSP6_INSN_LEN);

  /* QDSP6 insns will never have more than 1 fixup? */
  if (fixSP)
    *fixSP = fixP;

  return (stream + offset);
}

/** Mark the next packet for .falign.

@see qdsp6_packet_falign ().
*/
void
qdsp6_falign
(int ignore ATTRIBUTE_UNUSED)
{
  if (!qdsp6_faligning)
    {
      qdsp6_faligning = qdsp6_fetch_align;
      n_falign [QDSP6_FALIGN_TOTAL]++;
    }

  as_where (&falign_file, &falign_line);

  demand_empty_rest_of_line ();

  /* Just in case out of .text. */
  record_alignment (now_seg, DEFAULT_CODE_FALIGN);
}

void
qdsp6_frob_label
(symbolS *label ATTRIBUTE_UNUSED)
{
  /* Do not frob literal symbols. */
  if (!strncmp (now_seg->name, LITERAL_SECTION, LITERAL_LEN)
      || !strncmp (now_seg->name, LITERAL_SECTION_ONCE, LITERAL_ONCE_LEN))
    return;

  return;
}

/** Insert a NOP packet to align code.

@param fragP pointer to current code fragment.
@param count of bytes to fill up.
*/
void
qdsp6_align_code
(fragS *fragP, size_t count)
{
  const qdsp6_insn a_nop = qdsp6_nop_insn.insn | QDSP6_END_NOT,
                   z_nop = qdsp6_nop_insn.insn | QDSP6_END_PACKET,
                   *p_nop;
  size_t skip, here;

  if (count > 0)
    {
      if ((fragP->fr_address + fragP->fr_fix + count) % QDSP6_INSN_LEN)
        as_warn (_("code alignment must be a multiple of %d bytes."), QDSP6_INSN_LEN);

      if ((skip = count % QDSP6_INSN_LEN))
        /* Pad to 4-byte boundary first. */
        memset (fragP->fr_literal + fragP->fr_fix, 0, skip);
      /* Add NOPs to packet. */
      for (here = 0; skip + here + QDSP6_INSN_LEN < count; here += QDSP6_INSN_LEN)
        {
          if (  (skip + here) / QDSP6_INSN_LEN % MAX_PACKET_INSNS
              < MAX_PACKET_INSNS - 1)
            p_nop = &a_nop;
          else
            p_nop = &z_nop;
          memcpy (fragP->fr_literal + fragP->fr_fix + skip + here, p_nop, QDSP6_INSN_LEN);
        }
      /* Finalize packet. */
      if (skip + here + QDSP6_INSN_LEN <= count)
        memcpy (fragP->fr_literal + fragP->fr_fix + skip + here, &z_nop, QDSP6_INSN_LEN);

      fragP->fr_var = count;
    }
}

/** Check if packet has single memory operation.

Bug #832: if there is only one memory op in a packet,
it should go in ndx 0.
@param packet Packet to examine.
@return True if so.
*/
int
qdsp6_has_single
(const qdsp6_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of memory ops in this packet. */
  for (i = count = 0; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].opcode->attributes & A_RESTRICT_SINGLE_MEM_FIRST)
      count++;

  return (count == 1);
}

/** Check if packet has a prefix insn.

@param packet Packet to examine.
@return True if so.
*/
int
qdsp6_has_prefix
(const qdsp6_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of memory ops in this packet. */
  for (i = count = 0; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].opcode->attributes & A_IT_EXTENDER)
      count++;

  return (count);
}

/** Check if packet has a paired insn.

@param packet Packet to examine.
@return True if so.
*/
int
qdsp6_has_pair
(const qdsp6_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of duplex insns in this packet. */
  for (i = count = 0; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].opcode->attributes & PACKED)
      count++;

  return (count);
}

/** Check if packet has a duplex insn.

@param packet Packet to examine.
@return True if so.
*/
int
qdsp6_has_duplex
(const qdsp6_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of duplex insns in this packet. */
  for (i = count = 0; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].opcode->attributes & DUPLEX)
      count++;

  return (count);
}

/** Check if packet has a duplex insn and if a new insn would conflict with it
    or vice-versa.

@param apacket Packet to examine.
@param ainsn Insn to consider.
@param which Which insn conflicts (optional).
@return True if so.
*/
int
qdsp6_has_duplex_hits
(const qdsp6_packet *apacket, const qdsp6_packet_insn *ainsn, size_t *which)
{
  size_t i;

  /* Count number of memory ops in this packet. */
  for (i = 0; i < qdsp6_packet_count (apacket); i++)
    if (((apacket->insns [i].opcode->attributes & DUPLEX)
         && (ainsn->opcode->slot_mask & QDSP6_DUPLEX_SLOTS)
         && !(ainsn->opcode->slot_mask & ~QDSP6_DUPLEX_SLOTS))
        || ((ainsn->opcode->attributes & DUPLEX)
            && (apacket->insns [i].opcode->slot_mask & QDSP6_DUPLEX_SLOTS)
            && !(apacket->insns [i].opcode->slot_mask & ~QDSP6_DUPLEX_SLOTS)))
      {
        if (which)
          *which = i;

        return TRUE;
      }

  return FALSE;
}

/** Check if packet has a R.NEW insn.

@param packet Packet to examine.
@param reg Which register is expected.
@return True if so.
*/
int
qdsp6_has_rnew
(const qdsp6_packet *apacket, qdsp6_packet_insn **insn)
{
  size_t i;

  /* Count number of R.NEW insns in this packet. */
  for (i = 0; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].flags & QDSP6_INSN_IN_RNEW)
      {
        if (insn)
          *insn = ((qdsp6_packet *) apacket)->insns + i;

        return TRUE;
      }

  return FALSE;
}

/** Check if packet has memory operation insns.

@param packet Packet to examine.
@return Number of such insns.
*/
int
qdsp6_has_mem
(const qdsp6_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of memory ops in this packet. */
  for (i = count = 0; i < MAX_PACKET_INSNS; i++)
    if (   apacket->insns [i].opcode
        && (apacket->insns [i].opcode->attributes & A_RESTRICT_SINGLE_MEM_FIRST))
      count++;

  return (count);
}

/** Check if packet has a store.

@param packet Packet to examine.
@return True if so.
*/
int
qdsp6_has_store
(const qdsp6_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of duplex insns in this packet. */
  for (i = count = 0; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].opcode->attributes & A_STORE)
      count++;

  return (count);
}

/** Check if packet has a store restriction.

@param packet Packet to examine.
@return True if so.
*/
int
qdsp6_has_store_not
(const qdsp6_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of duplex insns in this packet. */
  for (i = count = 0; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].opcode->attributes & A_RESTRICT_NOSLOT1_STORE)
      count++;

  return (count);
}

int
qdsp6_is_nop_keep
(const qdsp6_packet *apacket, int current)
{
  int found = FALSE;
  int prev = current - 1;
  int next = current + 1;

  if (next < MAX_PACKET_INSNS)
    {
      while (prev >= 0)
        {
          if (!qdsp6_is_nop (apacket->insns [prev].insn))
            {
              found = TRUE;
              break;
            }
          prev--;
        }

      /* A_RESTRICT_NOSLOT1: the next instruction cannot be in ndx 1
         For arch_v1, the current nop should be kept (not sure why)
         For arch_v2, only when the next instruction can be put in ndx 1
         that should the current nop be kept. */
      if (found
	  && (apacket->insns [next].opcode->attributes & A_RESTRICT_NOSLOT1))
        {
              if ((apacket->insns [next].opcode->slot_mask & 2)
                  || qdsp6_if_arch_v1 ())
                return TRUE;
        }
    }

  return FALSE;
}

/** Add insn to current packet.

@param insn Insn to add.
@param prefix Respective prefix to add, if any.
@param packet Packet to insn to.
@param pad True if the insn is merely padding.
@return True if successful.
*/
int
qdsp6_packet_insert
(qdsp6_packet *packet,
 const qdsp6_packet_insn *insn, const qdsp6_packet_insn *prefix,
 const qdsp6_packet_insn2 *pair, int pad)
{
  int prefixed = !pad && prefix && (prefix->opcode->attributes & A_IT_EXTENDER)? 1: 0;
  int duplex   = (insn->opcode->attributes & DUPLEX)? 1: 0;
  int relax    = (insn->flags & QDSP6_INSN_IS_RELAX)? 1: 0;

  if (duplex && packet->duplex)
    /* Limit duplex insns to one per packet. */
    return FALSE;

  if (  (pad? qdsp6_packet_count (packet): qdsp6_packet_size (packet))
      + prefixed + duplex + relax < MAX_PACKET_INSNS)
    {
      packet->insns [qdsp6_packet_count (packet)] = *insn;
      packet->insns [qdsp6_packet_count (packet)].padded = pad;

      if (prefixed)
        {
          packet->insns [qdsp6_packet_count (packet)].flags |= QDSP6_INSN_IS_KXED;

          packet->prefixes [qdsp6_packet_count (packet)] = *prefix;
          packet->prefixes [qdsp6_packet_count (packet)].padded = pad;
          packet->prefixes [qdsp6_packet_count (packet)].lineno
            = packet->insns [qdsp6_packet_count (packet)].lineno;

          packet->prefix++;
        }

      if (pair)
        {
          packet->pairs [qdsp6_packet_count (packet)][0] = pair [0];
          packet->pairs [qdsp6_packet_count (packet)][1] = pair [1];
        }

      packet->duplex += duplex;
      packet->relax  += relax;
      packet->size++;
      return TRUE;
    }

  return FALSE;
}

/** Add insn to current packet foregoing branch relaxation.

@param insn Insn to add.
@param prefix Respective prefix to add, if any.
@param packet Packet to insn to.
@param pad True if the insn is merely padding.
@return True if successful.
*/
int
qdsp6_packet_cram
(qdsp6_packet *apacket,
 qdsp6_packet_insn *ainsn, const qdsp6_packet_insn *prefix,
 const qdsp6_packet_insn2 *pair, int pad)
{
  qdsp6_packet packet;
  qdsp6_packet_insn insn;

  /* Try to insert insn. */
  if (qdsp6_packet_insert (apacket, ainsn, prefix, pair, pad))
    return TRUE;
  else
    {
      insn = *ainsn;
      insn.flags &= ~QDSP6_INSN_IS_RELAX;

      /* Try again, but without branch relaxation. */
      if (qdsp6_packet_insert (apacket, &insn, prefix, pair, pad))
        {
          *ainsn = insn;
          return TRUE;
        }
      else
        {
          if (apacket->relax)
            {
              size_t i;

              packet = *apacket;
              /* Remove all branch relaxations. */
              for (i = 0; i < packet.size; i++)
                if ((packet.insns [i].flags & ~QDSP6_INSN_IS_RELAX)
                    || (packet.insns [i].operand.flags & ~QDSP6_OPERAND_IS_RELAX))
                  {
                    packet.insns [i].flags         &= ~QDSP6_INSN_IS_RELAX;
                    packet.insns [i].operand.flags &= ~QDSP6_OPERAND_IS_RELAX;
                    packet.relax--;
                  }

              /* Sanity check. */
              assert (!packet.relax);

              /* Try again, but without any branch relaxation. */
              if (qdsp6_packet_insert (&packet, &insn, prefix, pair, pad))
                {
                  *apacket = packet;
                  return TRUE;
                }
            }
        }
    }

  return FALSE;
}

/** Initialize a packet.

@param apacket Pointer to packet to be initialized.
*/
void
qdsp6_packet_init
(qdsp6_packet *apacket)
{
  size_t i;

  memset (apacket, 0, sizeof (*apacket));

  for (i = 0; i < MAX_PACKET_INSNS; i++)
    {
      qdsp6_insn_init (&apacket->insns [i]);
      qdsp6_insn_init (&apacket->prefixes [i]);
      qdsp6_insn_init (&apacket->pairs [i][0].insn);
      qdsp6_insn_init (&apacket->pairs [i][0].prefix);
      qdsp6_insn_init (&apacket->pairs [i][1].insn);
      qdsp6_insn_init (&apacket->pairs [i][1].prefix);
    }
}

/** Move prefixes to the side-lines.

@param apacket Reference to a packet.
*/
void
qdsp6_packet_unfold
(qdsp6_packet *apacket)
{
  qdsp6_packet packet;
  qdsp6_packet_insn prefix;
  size_t size;
  size_t i;

  size = qdsp6_packet_count (apacket);

  /* Ignore an empty packet. */
  if (!qdsp6_packet_count (apacket))
    return;

  qdsp6_packet_init (&packet);
  qdsp6_insn_init (&prefix);

  for (i = 0; i < size; i++)
    {
      if (apacket->insns [i].padded)
        continue;

      if (apacket->insns [i].opcode->attributes & A_IT_EXTENDER)
        {
          if (apacket->insns [i].fc && apacket->insns [i].fix)
            apacket->insns [i].fix->fx_offset -= apacket->insns [i].fix->fx_pcrel
                                                 ? i * QDSP6_INSN_LEN: 0;

          prefix = apacket->insns [i];
          continue;
        }

      if ((apacket->insns [i].flags & QDSP6_INSN_IS_KXED))
        {
          if (prefix.fc && prefix.fix)
            {
              prefix.fix->fx_where   = packet.size * QDSP6_INSN_LEN;
              prefix.fix->fx_offset += prefix.fix->fx_pcrel
                                       ? packet.size * QDSP6_INSN_LEN: 0;
            }

          /* Insert the prefix. */
          packet.prefixes [packet.size] = prefix;
          packet.prefix++;
        }

      if (apacket->insns [i].fc && apacket->insns [i].fix)
        {
          apacket->insns [i].fix->fx_where   = packet.size * QDSP6_INSN_LEN;
          apacket->insns [i].fix->fx_offset += apacket->insns [i].fix->fx_pcrel
                                               ? (packet.size - i) * QDSP6_INSN_LEN
                                               : 0;
        }

      /* Insert the insn. */
      packet.insns [packet.size] = apacket->insns [i];
      packet.size++;
      /* Sanity check. */
      assert (packet.size <= MAX_PACKET_INSNS);
    }

  /* Copy new insn array and new prefix array. */
  memcpy (apacket->insns,    packet.insns,    sizeof (apacket->insns));
  memcpy (apacket->prefixes, packet.prefixes, sizeof (apacket->prefixes));

  /* Update housekeeping. */
  apacket->size   = packet.size;
  apacket->prefix = packet.prefix;
}

/** Move prefixes inline with the insns.

Assumes that the packet has already been shuffled and padded.

@param apacket Reference to a packet.
*/
void
qdsp6_packet_fold
(qdsp6_packet *apacket)
{
  qdsp6_packet packet;
  size_t size, prefix;
  size_t i;

  size = qdsp6_packet_count (apacket);
  prefix = apacket->prefix;
  if (!size || !prefix)
    return;

  qdsp6_packet_init (&packet);

  /* Merge the insn and the prefix arrays. */
  for (i = 0; i < size; i++)
    {
      if (prefix && apacket->insns [i].padded)
        {
          /* Skip a padding insn to make room for a prefix. */
          prefix--;
          continue;
        }

      if (apacket->insns [i].flags & QDSP6_INSN_IS_KXED)
        {
          if (apacket->prefixes [i].fc && apacket->prefixes [i].fix)
            {
              apacket->prefixes [i].fix->fx_where
                = packet.size * QDSP6_INSN_LEN;
              apacket->prefixes [i].fix->fx_offset
                += apacket->prefixes [i].fix->fx_pcrel
                   ? (packet.size - i) * QDSP6_INSN_LEN: 0;
            }

          /* Insert the prefix. */
          packet.insns [packet.size++] = apacket->prefixes [i];
          /* Sanity check. */
          assert (packet.size < MAX_PACKET_INSNS);
          packet.prefix++;
        }

      if (apacket->insns [i].fc && apacket->insns [i].fix)
        {
          apacket->insns [i].fix->fx_where   = packet.size * QDSP6_INSN_LEN;
          apacket->insns [i].fix->fx_offset += apacket->insns [i].fix->fx_pcrel
                                               ? (packet.size - i) * QDSP6_INSN_LEN
                                               : 0;
        }

      /* Insert the insn. */
      packet.insns [packet.size++] = apacket->insns [i];
      /* Sanity check. */
      assert (packet.size <= MAX_PACKET_INSNS);
    }

  /* Copy new insn array and clear the prefix array. */
  memcpy (apacket->insns,    packet.insns,    sizeof (apacket->insns));
  memcpy (apacket->prefixes, packet.prefixes, sizeof (apacket->prefixes));

  /* Update housekeeping. */
  apacket->size   = packet.size;
  apacket->prefix = 0;
}

/** Write the a packet out.

@param apacket Reference to a packet.
*/
void
qdsp6_packet_write
(qdsp6_packet *apacket)
{
  fragS *previous;
  char *first;
  size_t max_skip, num_nops, num_padded_nops;
  size_t req_insns;
  size_t i;

  apacket->faligned = qdsp6_faligning;
  qdsp6_faligning   = FALSE;

  qdsp6_packet_fold (apacket);

  /* Determine if we can skip any NOP, for
     at least 2 instructions are needed for :endloop0 and
     at least 3 instructions for :endloop1; otherwise
     all can be skipped and nothing is emitted. */
  max_skip = qdsp6_packet_count (apacket)
             - ((apacket->is_inner || apacket->is_outer)
                ? (apacket->is_outer? 3: 2) : 0);

  /* Calculate the number of NOP needed. */
  for (i = 0, num_nops = 0, num_padded_nops = 0;
       i < qdsp6_packet_count (apacket);
       i++)
    {
      if (qdsp6_is_nop (apacket->insns [i].insn)
          && !qdsp6_is_nop_keep (apacket, i))
	{
	  num_nops++;

	  if (apacket->insns [i].padded)
	    num_padded_nops++;
	}
    }
  max_skip = MIN (max_skip, MIN (num_nops, num_padded_nops));

  /* Keep track of the number of emitted instructions to
     determine which packet bits to set. */
  req_insns = qdsp6_packet_count (apacket) - max_skip;

  previous = frag_now;
  frag_grow (2 * MAX_PACKET_INSNS * QDSP6_INSN_LEN);
  first = frag_more (req_insns * QDSP6_INSN_LEN);
  if (!frag_now->tc_frag_data)
    {
      frag_now->tc_frag_data = xmalloc (sizeof (*frag_now->tc_frag_data));
      frag_now->tc_frag_data->previous = previous != frag_now? previous: NULL;
    }

  /* Initialize scratch packet. */
  frag_now->tc_frag_data->packet = *apacket;
  frag_now->tc_frag_data->packet.size = 0;

  for (i = 0; i < qdsp6_packet_count (apacket); i++)
    {
      size_t size = qdsp6_packet_count (&frag_now->tc_frag_data->packet);

      /* Skip the padded NOP, not every NOP. */
      if (max_skip
	  && apacket->insns [i].padded
	  && qdsp6_is_nop (apacket->insns [i].insn)
	  && !qdsp6_is_nop_keep (apacket, i))
	{
	  max_skip--;
	  continue;
        }

      /* What's left is legit. */
      apacket->insns [i].padded = FALSE;

      /* Set proper packet bits. */
      if (size == (1 - 1) && apacket->is_inner)
	apacket->insns [i].insn =
          QDSP6_END_PACKET_SET (apacket->insns [i].insn, QDSP6_END_LOOP);
      else if (size == (2 - 1) && apacket->is_outer)
	apacket->insns [i].insn =
          QDSP6_END_PACKET_SET (apacket->insns [i].insn, QDSP6_END_LOOP);
      else if (size >= req_insns - 1
               && QDSP6_END_PACKET_GET (apacket->insns [i].insn) != QDSP6_END_PAIR)
	apacket->insns [i].insn =
          QDSP6_END_PACKET_SET (apacket->insns [i].insn, QDSP6_END_PACKET);
      /* Otherwise, leave the packet bits alone. */

      frag_now->tc_frag_data->packet.insns [size] = apacket->insns [i];
      qdsp6_insn_write (frag_now->tc_frag_data->packet.insns [size].insn,
                        frag_now->tc_frag_data->packet.insns [size].fc,
                        &frag_now->tc_frag_data->packet.insns [size].operand,
                        &frag_now->tc_frag_data->packet.insns [size].exp,
                        first, size * QDSP6_INSN_LEN,
                        &frag_now->tc_frag_data->packet.insns [size].fix,
                        frag_now->tc_frag_data->packet.insns [size].lineno);

      /* Count insn as legit. */
      frag_now->tc_frag_data->packet.size++;
    }

  assert (req_insns == frag_now->tc_frag_data->packet.size);

  previous = frag_now;
  frag_var (rs_machine_dependent,
            (2 * MAX_PACKET_INSNS - req_insns) * QDSP6_INSN_LEN,
            0, 0, NULL, 0, first);
  frag_now->tc_frag_data = xmalloc (sizeof (*frag_now->tc_frag_data));
  frag_now->tc_frag_data->previous = previous;
  qdsp6_packet_init (&frag_now->tc_frag_data->packet);

  /* Prepare packet for next insns. */
  qdsp6_packet_init (apacket);
}

/* Create a BSS section in the global data area. */
segT
qdsp6_create_sbss_section
(const char *name, flagword flags, unsigned int access)
{
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  segT small_section = NULL;
  symbolS *small_symbol;
  char small_name [MAX_SMALL_SECTION];

  if (!access)
    strcpy (small_name, name);
  else
    if ((size_t) snprintf (small_name, sizeof (small_name), "%s.%d", name, access)
        >= sizeof (small_name))
      return (NULL);

  small_section = bfd_get_section_by_name (stdoutput, small_name);
  if (!small_section)
    {
      char *small_section_name;

      assert (small_section_name = xmalloc (MAX_SMALL_SECTION));
      strcpy (small_section_name, small_name);

      /* Create an sbss section. */
      small_section = subseg_new (small_section_name, 0);
      bfd_set_section_flags
        (stdoutput, small_section, flags | (access? SEC_LOAD: 0));
      /* record_alignment (qdsp6_small_section, DEFAULT_DATA_ALIGNMENT); */

      if ((small_symbol = symbol_find (small_section_name)))
        symbol_set_bfdsym (small_symbol, small_section->symbol);
      else
        symbol_table_insert (section_symbol (small_section));
    }

  subseg_set (current_section, current_subsec);

  return (small_section);
}

/* Create a COMMON section in the global data area. */
segT
qdsp6_create_scom_section
(const char *name, flagword flags, unsigned int access)
{
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  segT small_section = NULL;
  asymbol *small_symbol;
  char small_name [MAX_SMALL_SECTION];

  if (!access)
    strcpy (small_name, name);
  else
    if ((size_t) snprintf (small_name, sizeof (small_name), "%s.%d", name, access)
        >= sizeof (small_name))
      return (NULL);

  small_section = bfd_get_section_by_name (stdoutput, small_name);
  if (!small_section)
    {
      char *small_section_name;

      small_section = xmalloc (sizeof (*small_section));
      small_symbol = xmalloc (sizeof (*small_symbol));
      small_section_name = xmalloc (MAX_SMALL_SECTION);

      strcpy (small_section_name, small_name);

      (*small_section)                = qdsp6_scom_section;
      (*small_section).name           = small_section_name;
      (*small_section).flags         |= flags | (access? SEC_LOAD: 0);
      (*small_section).output_section = small_section;
      (*small_section).symbol         = small_symbol;
      (*small_section).symbol_ptr_ptr = &(*small_section).symbol;

      (*small_symbol)                 = qdsp6_scom_symbol;
      (*small_symbol).name            = small_section_name;
      (*small_symbol).section         = small_section;
    }

  subseg_set (current_section, current_subsec);

  return (small_section);
}

/* Create a literal section with the appropriate flags. */
segT
qdsp6_create_literal_section
(const char *name, flagword flags, unsigned int align2)
{
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  segT new_section;
  flagword applicable = bfd_applicable_section_flags (stdoutput);

  new_section = subseg_new (name, 0);
  bfd_set_section_flags
    (stdoutput, new_section,
     applicable & (flags | SEC_ALLOC | SEC_LOAD | SEC_DATA | SEC_SMALL_DATA));

  record_alignment (now_seg, align2);

  symbol_table_insert (section_symbol (new_section));

  subseg_set (current_section, current_subsec);

  return (new_section);
}

/* add the expression exp to the .sdata literal pool */
qdsp6_literal *
qdsp6_add_to_lit_pool
(expressionS *exp, size_t size)
{
  qdsp6_literal *literal, *last;
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  offsetT mask = (size == 8)? 0xffffffffffffffffULL: 0xffffffffULL;

  /* Sanity check for illegal constant literals. */
  if (size != 8 && size != 4)
    as_fatal (_("invalid literal constant."));

  /* Try to find an existing literal for the expression in order to reuse it. */
  for (literal = last = qdsp6_pool; literal != NULL; literal = literal->next)
    {
      /* Save the last node. */
      last = literal;

      if ((literal->e.X_op == exp->X_op)
          && (literal->e.X_op == O_constant)
          && ((literal->e.X_add_number & mask) == (exp->X_add_number & mask))
          && (literal->size == size))
        break;

      if ((literal->e.X_op == exp->X_op)
          && (literal->e.X_op == O_symbol)
          && (literal->e.X_op_symbol == exp->X_op_symbol)
          && (literal->e.X_add_symbol == exp->X_add_symbol)
          && (literal->e.X_add_number == exp->X_add_number)
          && (literal->size == size))
        break;
    }

  /* Do we need to create a new entry? */
  if (!literal)
    {
      if (qdsp6_pool_counter >= MAX_LITERAL_POOL)
        {
          as_bad (_("too many literal constants."));
          return NULL;
        }

      literal = xmalloc (sizeof (*literal));
      literal->next = NULL;
      literal->size = size;

      /* save the head pointer to "qdsp6_pool" */
      if (!qdsp6_pool)
        qdsp6_pool = last = literal;

      /* get the expression */
      literal->e = *exp;

      if (literal->e.X_op == O_constant)
        /* Create a constant symbol (with its value as the suffix). */
        {
          if (size == 8)
            {
              sprintf (literal->name,
#if defined (__MINGW32__)
                       "%s_%016I64x",
#else
                       "%s_%016llx",
#endif
                     LITERAL_PREFIX, (long long) literal->e.X_add_number);
              sprintf (literal->secname, "%s%s", LITERAL_SECTION_ONCE_8, literal->name);
            }
          else
            {
              sprintf (literal->name, "%s_%08x",
                       LITERAL_PREFIX, (int) literal->e.X_add_number);
              sprintf (literal->secname, "%s%s", LITERAL_SECTION_ONCE_4, literal->name);
            }

          literal->sec =
            qdsp6_create_literal_section (literal->secname, SEC_LINK_ONCE, bfd_log2 (size));
          literal->sub = LITERAL_SUBSECTION + (size == 8? 2: 1);
        }
      else /* if (literal->e.X_op == O_symbol) */
        /* Create a label symbol (with the literal order as the suffix). */
        {
          sprintf (literal->name, "%s_%04lx", LITERAL_PREFIX, qdsp6_pool_counter);
          strcpy (literal->secname, LITERAL_SECTION_A);

          literal->sec = qdsp6_create_literal_section (literal->secname, 0, bfd_log2 (size));
          literal->sub = LITERAL_SUBSECTION;
        }

      /* Group the literals per size and type in separate subsections
         in order to minimize wasted space due to alignment. */
      subseg_set (literal->sec, literal->sub);

      /* Set the alignment. */
      frag_align (bfd_log2 (size), 0, 0);

      /* Define the symbol. */
      literal->sym =
        symbol_new (literal->name, now_seg, (valueT) frag_now_fix (), frag_now);
      symbol_table_insert (literal->sym);

      /* Set the size and type. */
      S_SET_SIZE (literal->sym, size);
      symbol_get_bfdsym (literal->sym)->flags |= BSF_OBJECT;

      /* Set the scope. */
      if (literal->e.X_op == O_constant)
        S_SET_EXTERNAL (literal->sym);

      /* Emit the data definition. */
      emit_expr (&(literal->e), size);

      /* Restore the last section. */
      subseg_set (current_section, current_subsec);

      qdsp6_pool_counter++;

      /* Add literal to the literal pool list. */
      if (qdsp6_pool != literal)
        last->next = literal;
    }

  return (literal);
}

/* This function searches the instruction string for GP related
 * instructions:
 * Rd32 = CONST32(#imm), Rd32 = CONST32(label)
 * Rdd32 = CONST64(#imm), Rdd32 = CONST64(label)
 * If it is identified as such an instruction, following translation
 * will be performed:
 * 1. Add an internal reference to "#imm" or "label"
 * 2. Store the reference to ".sdata" section
 * 3. Translate the instruction into following format:
 * Rd32 = memw(#internal_reference)
 * Rdd32 = memd(#internal_reference)
 * The new instruction will be stored in "new_str"
 * A few details:
 * 1. If ".sdata" section does not exist, create one
 * 2. Keep track of the internal references created ("literal pool"),
 *    and reuse it whenever possible
 * 3. Only one ".sdata" will be created/used.
 * 4. The size of ".sdata" will be 1024 bytes, anything beyond that
 * will trigger an error message "literal pool (.sdata) overflow"
 *
 * Returns:
 * 1 if success (found and translated),
 * 0 for anything else.
 */

int
qdsp6_gp_const_lookup
(char *str, char *new_str)
{
  static const char ex_c32 []
    = "=[[:space:]]*" REGEX_ENCLOSE ("const32") "[[:space:]]*\\(#?(.+)\\)";
  static const char ex_c64 []
    = "=[[:space:]]*" REGEX_ENCLOSE ("const64") "[[:space:]]*\\(#?(.+)\\)";
  static const char ex_r32 [] =
    "^" REGEX_ENCLOSE ("(r((0*[12]?[0-9])|30|31))|sp|fp|lr");
  static const char ex_r64 [] =
    "^" REGEX_ENCLOSE ("((r((0*[12]?[13579])|31))|sp|lr):"
                       "((0*[12]?[02468])|30|fp)");
  static regex_t re_c32, re_c64, re_r32, re_r64;
  regmatch_t rm_left [1], rm_right [2];
  static int re_ok;
  int er_re;
  int size;
  expressionS exp;
  qdsp6_literal *litcurptr;
  char *save;
  segT seg;

  if (!re_ok)
    {
      /* Compile REs. */
      assert (!regcomp (&re_c32, ex_c32, REG_EXTENDED | REG_ICASE));
      assert (!regcomp (&re_c64, ex_c64, REG_EXTENDED | REG_ICASE));
      assert (!regcomp (&re_r32, ex_r32, REG_EXTENDED | REG_ICASE));
      assert (!regcomp (&re_r64, ex_r64, REG_EXTENDED | REG_ICASE));

      re_ok = TRUE;
    }

  /* Get the left and right-side expressions and
     distinguish between CONST32 and CONST64. */
  if (!(er_re = regexec (&re_c32, str, 2, rm_right, 0)))
    {
      if (rm_right [1].rm_so < 0 || rm_right [1].rm_eo < 0
          || (er_re = regexec (&re_r32, str, 1, rm_left, 0)))
        return FALSE;
      else
        size = 4;
    }
  else if (!(er_re = regexec (&re_c64, str, 2, rm_right, 0)))
    {
      if (rm_right [1].rm_so < 0 || rm_right [1].rm_eo < 0
          || (er_re = regexec (&re_r64, str, 1, rm_left, 0)))
        return FALSE;
      else
        size = 8;
    }
  else
    return FALSE;

  /* Parse right-side expression. */
  save = input_line_pointer;
  input_line_pointer = str + rm_right [1].rm_so;
  seg = expression (&exp);
  input_line_pointer = save;

  /* 64-bit literals must be constants. */
  if (exp.X_op != O_constant && size == 8)
    {
      as_bad (_("64-bit expression `%.*s' is not constant."),
              rm_right [1].rm_eo - rm_right [1].rm_so,
              str + rm_right [1].rm_so);

      return FALSE;
    }

  /* Add a literal for the expression. */
  litcurptr = qdsp6_add_to_lit_pool (&exp, size);

  /* Replace original "insn" with a GP-relative load from the literal. */
  if (litcurptr)
    snprintf (new_str, QDSP6_MAPPED_LEN, "%.*s = mem%c (#%s)",
              rm_left [0].rm_eo - rm_left [0].rm_so, str,
              size == 4? 'w' : 'd', litcurptr->name);
  else
    return FALSE;

  return TRUE;
}

/* This routine is called for each instruction to be assembled. */
void
md_assemble
(char *str)
{
  qdsp6_assemble (qdsp6_packets + 0, str, FALSE, FALSE);
}

/* This routine is called for each instruction to be assembled. */
int
qdsp6_assemble
(qdsp6_packet *apacket, char *str, int padded, int pair)
{
  const qdsp6_opcode *opcode;
  qdsp6_packet_insn prefix, insn;
  char *start, *syn;
  int is_id, is_paired;
  /* to store the new instruction string for following instructions:
   * Rd32 = CONST32(#imm), Rd32 = CONST32(label)
   * Rdd32 = CONST64(#imm), Rdd32 = CONST64(label)
   */
  char new_str [QDSP6_MAPPED_LEN];
  /* For instruction mapping */
  char mapped [QDSP6_MAPPED_LEN];
  long op_val;
  char *op_str;
  qdsp6_operand_arg op_args [MAX_OPERANDS];
  size_t op_ndx;
  char *errmsg = NULL;

  /* Opcode table initialization is deferred until here because we have to
     wait for a possible .option command. */
  qdsp6_init (qdsp6_mach_type);

/*
  if (qdsp6_packet_size (apacket) >= MAX_PACKET_INSNS)
    {
      as_bad (_("too many instructions in packet (maximum is %d)."),
              MAX_PACKET_INSNS);
      return FALSE;
    }
*/

  /* Skip leading white space.  */
  while (ISSPACE (*str))
    str++;

  /* Special handling of GP-related syntax:
     - Rd32 = CONST32(#imm) or Rd32 = CONST32(label)
     - Rdd32 = CONST64(#imm) or Rdd32 = CONST64(label) */
  if (qdsp6_gp_const_lookup (str, new_str))
    str = new_str;

  if (strlen (str) > QDSP6_MAPPED_LEN - 1)
  {
    as_bad (_("source line too long (maximum length is %d)."),
            QDSP6_MAPPED_LEN - 1);
    return FALSE;
  }

  /* Keep looking until we find a match.  */
  start = str;
  for (opcode = qdsp6_opcode_lookup_asm (str);
       opcode;
       opcode = QDSP6_OPCODE_NEXT_ASM (opcode))
    {
      /* Is this opcode supported by the selected cpu?  */
      if (!qdsp6_opcode_supported (opcode))
	continue;

      /* Initialize the tentative insn. */
      qdsp6_insn_init (&prefix);
      qdsp6_insn_init (&insn);
      insn.opcode = opcode;
      strncpy (insn.string, start, sizeof (insn.string));
      /* Yank the packet termination out. */
      if ((str = strchr (insn.string, PACKET_END)))
        *str = 0;

      /* Scan the syntax string.  If it doesn't match, try the next one.  */
      insn.insn = qdsp6_encode_opcode (insn.opcode->enc);
      is_id = 0;
      op_ndx = 0;

      for (str = start, syn = insn.opcode->syntax;
           *str && *syn; )
        {
          const qdsp6_operand *operand;
          ptrdiff_t op_len;

          if ((ISUPPER (*syn) && (*syn != 'I')) || *syn == '#')
	    {
	      /* We have an operand */
	      if ((operand = qdsp6_lookup_operand (syn)))
	        {
		  size_t len = strlen (operand->fmt);

                  op_str = str;

                  if (operand->flags & QDSP6_OPERAND_IS_CONSTANT)
                    goto NEXT_SYN;
		  else if (operand->flags & QDSP6_OPERAND_IS_IMMEDIATE)
		    {
		      str = qdsp6_parse_immediate
			      (&insn, &prefix, operand, str, &op_val,
                               !pair? &errmsg: NULL);
		      if (!str)
			goto NEXT_OPCODE;
                    }
                  else
                    {
                      int is_op;

                      str = operand->parse (operand, &insn.insn, insn.opcode,
                                            str, &op_val, &is_op,
                                            !pair? &errmsg: NULL);
                      if (!str)
                        /* This instruction doesn't match the syntax */
                        goto NEXT_OPCODE;

                      if (is_op & QDSP6_OPERAND_IS_RNEW)
                        {
                          insn.flags    |= QDSP6_INSN_IN_RNEW;
                          insn.ioperand  = operand;
                          insn.ireg      = op_val;
                        }
                    }

                  /* Store the operand value in case the insn is an alias. */
                  assert (op_ndx < MAX_OPERANDS);

                  op_args [op_ndx].operand = operand;
                  op_args [op_ndx].value = op_val;
                  op_len = MIN (str - op_str,
                                (ptrdiff_t) sizeof (op_args [op_ndx].string) - 1);
                  strncpy (op_args [op_ndx].string, op_str, op_len);
                  op_args [op_ndx].string [op_len] = 0;
                  op_ndx++;

                  /* Move past the opcode specifier */
                  syn += len;
                  is_id = 0;
                }
              else
		as_bad (_("operand not found for `%s'."), syn);
            }
          else
            {
              /* Non-operand chars must match exactly.  */
              NEXT_SYN:

              if ((TOLOWER (*str) == *syn) || ((*str == 'I') && (*str == *syn)))
                {
                  is_id = is_part_of_name (*syn);
                  ++syn;
                  ++str;
                }
              else
                /* This instruction doesn't match the syntax */
                break;
            }

          if (ISSPACE (*syn))
            {
              while (ISSPACE (*syn))
                syn++;

              /* If whitespace is required, make sure it's present */
              if (is_id
                  && !ISSPACE (*str)
                  && (is_name_beginner (*syn) || *syn == '#')
                     /* TODO: make exception for "p0.new" in the syntax. */
                  && syn [0] != '.' && syn [1] != 'n' && syn [2] != 'e' && syn [3] != 'w')
                goto NEXT_OPCODE;

              while (ISSPACE (*str))
                str++;
            }
          else
            while (ISSPACE (*str))
              str++;
        }

      /* If we're at the end of the syntax string, we're done.  */
      if ((!*str || *str == PACKET_END) && !*syn)
        {
          char *cur_file;
          unsigned int cur_line;

          if (!qdsp6_check_operand_args (op_args, op_ndx))
            goto NEXT_OPCODE;

          /* Find where we are, will be used to dump to the dwarf2 debug info*/
          as_where (&cur_file, &cur_line);

          if (insn.opcode->map)
            {
              /* Remap insn. */
              ((qdsp6_mapping) insn.opcode->map)
                (mapped, QDSP6_MAPPED_LEN, op_args);

              /* Concatenate the rest of the original insn
                 before assembling the remapped insn. */
              strncat (mapped, str, QDSP6_MAPPED_LEN - strlen (mapped) - 1);
              return (qdsp6_assemble (apacket, mapped, padded, pair));
            }

          /* TODO: Perform various error and warning tests.  */

          insn.used   = FALSE;
          insn.padded = padded;
          insn.lineno = cur_line;

          is_paired = FALSE;
          if (!pair && qdsp6_in_packet)
            is_paired = qdsp6_assemble_pair (apacket, &insn, &prefix);

          if (!is_paired && !qdsp6_packet_cram (apacket, &insn, &prefix, NULL, FALSE))
            as_bad (_("too many instructions in packet (maximum is %d)."),
                    MAX_PACKET_INSNS);

          if (!pair && !qdsp6_in_packet)
            {
              qdsp6_packet_end (apacket);
              qdsp6_packet_write (apacket);
            }

          while (ISSPACE (*str))
            ++str;

          /* Check for the packet end string */
          if (*str == PACKET_END)
            {
              str++;

              if (!qdsp6_in_packet)
                {
                  as_warn (_("found `%c' before opening a packet."), PACKET_END);
                  return TRUE;
                }

              qdsp6_packet_end (apacket);

              while (ISSPACE (*str))
                ++str;

              /* Check for the end inner/outer modifiers.
                 Note that they can appear in any order. */
              while (1)
                {
                  if (!strncasecmp (str, PACKET_END_INNER,
                                    strlen (PACKET_END_INNER)))
                    {
                      qdsp6_packet_end_inner (qdsp6_packets + 0);
                      str += strlen (PACKET_END_INNER);

                      while (ISSPACE (*str))
                        ++str;
                    }
                  else if (!strncasecmp (str, PACKET_END_OUTER,
			                  strlen (PACKET_END_OUTER)))
                    {
                      qdsp6_packet_end_outer (qdsp6_packets + 0);
                      str += strlen (PACKET_END_OUTER);

                      while (ISSPACE (*str))
                        ++str;
                    }
                  else
                    {
                      break;
                    }
                }

              if (!is_end_of_line [(unsigned char) *str])
                {
                  if (!pair)
                    as_bad (_("extra symbols at end of line: `%s'"), str);
                  else
                    return FALSE;
                }
              else if (!apacket->is_inner || !apacket->is_outer)
                {
                  /* May need to lookahead in the input stream for
                     (more) inner/outer modifiers. */
                  int inner, outer;

                  qdsp6_packet_end_lookahead (&inner, &outer); /* may make 'str' invalid */
                  --input_line_pointer;

                  if (inner)
                    qdsp6_packet_end_inner (apacket);

                  if (outer)
                    qdsp6_packet_end_outer (apacket);
                }

              qdsp6_packet_write (apacket);
            }
          else if (!is_end_of_line [(unsigned char) *str])
            {
              if (!pair)
                as_bad (_("extra symbols at end of line: `%s'"), str);
              else
                return FALSE;
            }

          /* All done.  */
          return TRUE;
        }

      /* Try the next entry.  */
      NEXT_OPCODE:
        continue;
    }

  if (!pair)
    {
      int big = strlen (start) > MAX_MESSAGE - 3;

      if (errmsg)
        as_bad (errmsg);
      as_bad (_("invalid instruction `%.*s%s'"),
              big? MAX_MESSAGE - 3: MAX_MESSAGE, start, big? "...": "");
    }

  return FALSE;
}

int
qdsp6_assemble_pair
(qdsp6_packet *apacket, qdsp6_packet_insn *ainsn, qdsp6_packet_insn *aprefix)
{
  /* Order of pairing restrictions to try. */
  static const unsigned attr [] =
    {
      /* Refrain from relative branches as they may overflow. */
      A_BRANCHADDER,
      /* The last attempt is free for all. */
      0
    };
  qdsp6_packet packet;
  qdsp6_packet_insn insn, prefix;
  qdsp6_packet_insn2 prepair [2];
  char pair [QDSP6_MAPPED_LEN], unpair [QDSP6_MAPPED_LEN];
  size_t i, j, k, a_branch, a_duplex;
  size_t is_duplex, is_prefix;
  int has_hits, has_ommited, has_room;

  /* Do nothing if pairing disabled. */
  if (!qdsp6_pairing)
    return FALSE;

  /* Branches cannot be paired out of source order, so find the last one. */
  for (i = a_branch = qdsp6_packet_count (apacket); i > 0; i--)
    if ((ainsn->opcode->implicit_reg_def & IMPLICIT_PC)
        && (apacket->insns [i - 1].opcode->implicit_reg_def & IMPLICIT_PC))
      {
        a_branch = i - 1;
        break;
      }

  /* Break a previous duplex if it conflicts with other insns in the packet. */
  if (qdsp6_has_duplex_hits (apacket, ainsn, &a_duplex))
    {
      qdsp6_packet_init (&packet);

      for (i = 0; i < qdsp6_packet_count (apacket); i++)
        {
          if (i == a_duplex)
            {
              qdsp6_packet_insert
                (&packet,
                 &apacket->pairs [i][0].insn, &apacket->pairs [i][0].prefix, NULL,
                 FALSE);
              qdsp6_packet_insert
                (&packet,
                 &apacket->pairs [i][1].insn, &apacket->pairs [i][1].prefix, NULL,
                 FALSE);
            }
          else
            qdsp6_packet_insert
              (&packet,
               apacket->insns + i, apacket->prefixes + i, &apacket->pairs [i][0],
               FALSE);
        }

      *apacket = packet;

      n_pairs [QDSP6_PAIRS_TOTAL]--;
      n_pairs [QDSP6_PAIRS_UNDONE]++;
    }

  /* Go over the pairing restrictions. */
  for (j = 0; j < sizeof (attr) / sizeof (*attr); j++)
    /* Go over the previous insns. */
    for (i = 0; i < qdsp6_packet_count (apacket); i++)
      {
        /* Skip certain insns. */
        if (
            /* After the restriction vector. */
            ((ainsn->opcode->attributes & attr [j])
            || (apacket->insns [i].opcode->attributes & attr [j]))
            /* Packed or duplex insns. */
            || ((apacket->insns [i].opcode->attributes & PACKED)
                || (apacket->insns [i].opcode->attributes & DUPLEX))
            /* Prefix insns. */
            || (apacket->insns [i].opcode->attributes & PREFIX)
            /* Selectively, direct branch insns. */
            || ((!qdsp6_pairing_branch
                 && (ainsn->opcode->attributes & A_BRANCHADDER))
                || (!qdsp6_pairing_branch
                    && (apacket->insns [i].opcode->attributes & A_BRANCHADDER)))
            /* Extended insns. */
            || ((ainsn->flags & QDSP6_INSN_IS_KXED)
                && (apacket->insns [i].flags & QDSP6_INSN_IS_KXED))
            /* If both insns have symbolic references. */
            || (ainsn->fc && apacket->insns [i].fc))
          continue;

        /* Skip certain combinations. */
        /*
        if ((ainsn->opcode->implicit_reg_def & IMPLICIT_PC)
            && (apacket->insns [i].opcode->implicit_reg_def & IMPLICIT_PC))
          continue;
        */

        if (strlen (apacket->insns [i].string) + 1 + strlen (ainsn->string)
            >= sizeof (pair))
          /* Skip a too long a pair. */
          continue;

        /* Create a pair and its mirror. */
        snprintf (pair, sizeof (pair), "%s%c%s",
                  apacket->insns [i].string, PACKET_PAIR, ainsn->string);
        snprintf (unpair, sizeof (unpair), "%s%c%s",
                  ainsn->string, PACKET_PAIR, apacket->insns [i].string);

        prepair [0].prefix = apacket->prefixes [i];
        prepair [0].insn   = apacket->insns [i];
        prepair [1].prefix = *aprefix;
        prepair [1].insn   = *ainsn;

        qdsp6_packet_init (&packet);
        if (qdsp6_assemble (&packet, pair,   FALSE, TRUE)
            || qdsp6_assemble (&packet, unpair, FALSE, TRUE))
          {
            is_duplex = (packet.insns [0].opcode->attributes & DUPLEX)? 1: 0;
            is_prefix = (packet.insns [0].flags & QDSP6_INSN_IS_KXED)? 1: 0;

            /* Keep trying if the result is an unwanted duplex. */
            if (!qdsp6_pairing_duplex && is_duplex)
              continue;

            /* Check for existing duplex insns. */
            if (is_duplex && qdsp6_has_duplex_hits (apacket, packet.insns, NULL))
              continue;

            insn   = packet.insns    [0];
            prefix = packet.prefixes [0];
            qdsp6_packet_init (&packet);

            /* Set up the new packet. */
            for (k = 0, has_hits = has_ommited = FALSE, has_room = TRUE;
                 k < qdsp6_packet_count (apacket) && !has_hits && has_room;
                 k++)
              if (k <= a_branch)
                {
                  /* Before last branch, if any. */
                  if (k == i)
                    {
                      /* Ommit the paired insn. */
                      has_ommited = TRUE;
                      continue;
                    }
                  else
                    {
                      if ((has_hits =
                             qdsp6_has_duplex_hits
                               (&packet, apacket->insns + k, NULL)))
                        /* Bail if the new pair causes conflicts. */
                        break;
                      else
                        /* Insert original insn. */
                        has_room = qdsp6_packet_cram
                                     (&packet,
                                      apacket->insns + k, apacket->prefixes + k,
                                      NULL, FALSE);
                    }
                  }
              else
                {
                  /* Past last branch, if any. */
                  if (k == i)
                    {
                      if ((has_hits = qdsp6_has_duplex_hits (&packet, &insn, NULL)))
                        /* Bail if the new pair causes conflicts. */
                        break;
                      else
                        /* Insert paired insn instead. */
                        has_room = qdsp6_packet_cram
                                     (&packet, &insn, &prefix, prepair, FALSE);
                    }
                  else
                    {
                      if ((has_hits =
                             qdsp6_has_duplex_hits (&packet, apacket->insns + k, NULL)))
                        /* Bail if the new pair causes conflicts. */
                        break;
                      else
                        /* Insert original insn. */
                        has_room = qdsp6_packet_cram
                                     (&packet,
                                      apacket->insns + k, apacket->prefixes + k,
                                      NULL, FALSE);
                    }
                }

            if (has_hits || !has_room)
              continue;

            if (has_ommited)
              {
                if (qdsp6_has_duplex_hits (&packet, &insn, NULL))
                  /* Bail if the new pair causes conflicts. */
                  continue;
                else
                  /* Add paired insn after a previous branch. */
                  if (!(has_room = qdsp6_packet_cram
                                     (&packet, &insn, &prefix, prepair, FALSE)))
                    /* Bail if not enough room. */
                    continue;
              }

            /* Override the old packet. */
            *apacket = packet;

            /* Cancel the original insn. */
            qdsp6_insn_init (ainsn);
            qdsp6_insn_init (aprefix);

            n_pairs [QDSP6_PAIRS_TOTAL]++;
            return TRUE;
          }
      }

  return FALSE;
}

void
qdsp6_common
(int localScope)
{
  segT old_sec;
  int old_subsec;
  segT new_sec;
  int new_subsec;
  char *name;
  char c;
  char *p;
  offsetT align, align2, alignb, size, access, temp;
  symbolS *symbolP;

  name = input_line_pointer;
  c = get_symbol_end ();
  /* just after name is now '\0'  */
  p = input_line_pointer;
  *p = c;
  SKIP_WHITESPACE ();

  if (*input_line_pointer != ',')
    {
      as_bad (_("expected comma after symbol name."));
      ignore_rest_of_line ();
      return;
    }

  input_line_pointer++;		/* skip ','  */
  size = get_absolute_expression ();

  if (size < 0)
    {
      as_bad (_("invalid symbol length."));
      ignore_rest_of_line ();
      return;
    }

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (S_IS_DEFINED (symbolP) && !S_IS_COMMON (symbolP))
    {
      as_bad (_("symbol `%s' is already defined."), S_GET_NAME (symbolP));
      ignore_rest_of_line ();
      return;
    }

  if ((S_GET_VALUE (symbolP) != 0)
      && (S_GET_VALUE (symbolP) != (valueT) size))
    {
      as_warn (_("length of symbol `%s' is %ld, ignoring `%ld'."),
	       S_GET_NAME (symbolP), (long) S_GET_VALUE (symbolP), (long) size);
    }

  assert (symbolP->sy_frag == &zero_address_frag);

  /* Start assuming that the alignment is 0, i.e.
     use the natural alignment for the symbol size. */
  align = 0;
  /* Now parse the optional alignment field. */
  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      align = get_absolute_expression ();

      if (align < 0)
        as_warn (_("invalid symbol alignment, defaulting to %ld."),
                 (long) (align = 0));
    }

  /* Convert alignment to a power of 2,
     if not provided, use the natural size alignment. */
  temp = align? align: MIN (size, MAX_DATA_ALIGNMENT);

  align2 = bfd_log2 (temp);
  alignb = 1 << align2;

  if (align && alignb != temp)
    as_warn (_("invalid symbol alignment, defaulting to %ld."),
             (long) (alignb = 1 << ++align2));

  /* Start assuming that the access is 0, i.e.
     use the natural access for the symbol size and alignment. */
  access = 0;
  /* Now parse the optional access field. */
  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      access = get_absolute_expression ();

      if (access < 0 || access != (1 << bfd_log2 (access)))
        as_warn (_("invalid symbol access size, defaulting to %ld."),
                 (long) (access = 0));
    }

  if (!access)
    access = 1 << MIN ((offsetT) bfd_log2 (size), align2);

  if (localScope || symbol_get_obj (symbolP)->local)
    {
      /* Create a local unitialized symbol. */
      char *pfrag;

      old_sec    = now_seg;
      old_subsec = now_subseg;

      /* Check the symbol size against the specified small data limit. */
      if (qdsp6_if_arch_v1 () || (size_t) size > qdsp6_gp_size)
        {
          /* Allocate in .bss. */
          new_sec = bss_section;
          new_subsec = 0;
        }
      else
        {
          /* Allocate in .sbss. */
          new_sec =
            qdsp6_create_sbss_section (SMALL_BSS_SECTION,
                                       SEC_ALLOC | SEC_DATA | SEC_SMALL_DATA,
                                       qdsp6_sort_sda? access: 0);
          assert (new_sec);
          seg_info (new_sec)->bss = TRUE;
          new_subsec = 0;
        }

      record_alignment (new_sec, align2);
      subseg_set (new_sec, new_subsec);

      if (align2)
	/* Do alignment.  */
	frag_align (align2, 0, 0);

      /* Detach from old frag.  */
      if (S_GET_SEGMENT (symbolP) == new_sec)
	symbolP->sy_frag->fr_symbol = NULL;

      symbolP->sy_frag = frag_now;
      pfrag = frag_var (rs_org, 1, 1, (relax_substateT) 0, symbolP,
			(offsetT) size, (char *) 0);
      *pfrag = 0;

      S_SET_SIZE       (symbolP, size);
      symbolP->local = localScope;
      S_CLEAR_EXTERNAL (symbolP);
      S_SET_SEGMENT    (symbolP, new_sec);

      subseg_set (old_sec, old_subsec);
    }
  else
    {
      /* Create a global unitialized common symbol. */
      old_sec    = now_seg;
      old_subsec = now_subseg;

      /* Check the symbol size against the specified small data limit. */
      if (qdsp6_if_arch_v1 () || (size_t) size > qdsp6_gp_size)
        new_sec = bfd_com_section_ptr;
      else
        {
          new_sec = // &qdsp6_scom_section;
            qdsp6_create_scom_section (SMALL_COM_SECTION,
                                          SEC_ALLOC | SEC_IS_COMMON
                                        | SEC_DATA | SEC_SMALL_DATA,
                                       qdsp6_sort_sda? access: 0);
          assert (new_sec);
        }

      /* Createa global common symbol. */
      S_SET_VALUE    (symbolP, (valueT) size);
      S_SET_ALIGN    (symbolP, alignb);
      S_SET_EXTERNAL (symbolP);
      S_SET_SEGMENT  (symbolP, new_sec);

      subseg_set (old_sec, old_subsec);
    }

  symbol_get_bfdsym (symbolP)->flags |= BSF_OBJECT;

  demand_empty_rest_of_line ();
  return;
}

/** Check if the operands are consistent. */
int
qdsp6_check_operand_args
(const qdsp6_operand_arg args [], size_t n)
{
  size_t i, j;

  for (i = 0; i < n; i++)
    for (j = i + 1; j < n; j++)
      {
        if (args [i].operand->enc_letter == args [j].operand->enc_letter)
          if (args [i].value != args [j].value)
            return FALSE;
      }

  return TRUE;
}

/* Select the cpu we're assembling for.  */
void
qdsp6_option
(int ignore ATTRIBUTE_UNUSED)
{
  int mach;
  char c;
  char *cpu;

  cpu = input_line_pointer;
  c = get_symbol_end ();
  mach = qdsp6_get_mach (cpu);
  *input_line_pointer = c;

  /* If an instruction has already been seen, it's too late.  */
  if (cpu_tables_init_p)
    {
      as_bad (_("\".option\" directive must appear before any instructions."));
      ignore_rest_of_line ();
      return;
    }

  if (mach < 0)
    {
      as_bad (_("invalid architecture specified for \".option\"."));
      ignore_rest_of_line ();
      return;
    }
  else if (mach_type_specified_p && mach != qdsp6_mach_type)
    {
      as_bad (_("architecture specified for \".option\" conflicts with current setting."));
      ignore_rest_of_line ();
      return;
    }
  else
    {
      /* The cpu may have been selected on the command line.  */
      if (mach != qdsp6_mach_type)
	as_warn (_("\".option\" directive overrides default or command-line setting."));
      qdsp6_mach_type = mach;

      if (!bfd_set_arch_mach (stdoutput, bfd_arch_qdsp6, mach))
	as_fatal (_("assembler internal error."));

      mach_type_specified_p = TRUE;
    }

  demand_empty_rest_of_line ();
  return;
}

/* Turn a string in input_line_pointer into a floating point constant
   of type TYPE, and store the appropriate bytes in *LITP.  The number
   of LITTLENUMS emitted is stored in *SIZEP.  An error message is
   returned, or NULL on OK.  */

char *
md_atof
(int type, char *litP, int *sizeP)
{
  int prec;
  LITTLENUM_TYPE words[MAX_LITTLENUMS];
  LITTLENUM_TYPE *wordP;
  char *t;
  char * atof_ieee PARAMS ((char *, int, LITTLENUM_TYPE *));

  switch (type)
    {
    case 'f':
    case 'F':
      prec = 2;
      break;

    case 'd':
    case 'D':
      prec = 4;
      break;

    default:
      *sizeP = 0;
      return "bad call to md_atof";
    }

  t = atof_ieee (input_line_pointer, type, words);
  if (t)
    input_line_pointer = t;
  *sizeP = prec * sizeof (LITTLENUM_TYPE);
  for (wordP = words; prec--;)
    {
      md_number_to_chars (litP, (valueT) (*wordP++), sizeof (LITTLENUM_TYPE));
      litP += sizeof (LITTLENUM_TYPE);
    }

  return NULL;
}

/* Round up a section size to the appropriate boundary.  */
valueT
md_section_align
(segT segment ATTRIBUTE_UNUSED, valueT size)
{
  return (size);
}

void
qdsp6_code_symbol
(expressionS *expressionP)
{
  if (expressionP->X_op == O_symbol && expressionP->X_add_number == 0)
    {
      expressionS two;
      expressionP->X_op = O_right_shift;
      expressionP->X_add_symbol->sy_value.X_op = O_constant;
      two.X_op = O_constant;
      two.X_add_symbol = two.X_op_symbol = NULL;
      two.X_add_number = 2;
      expressionP->X_op_symbol = make_expr_symbol (&two);
    }
  /* Allow %st(sym1-sym2)  */
  else if (expressionP->X_op == O_subtract
	   && expressionP->X_add_symbol != NULL
	   && expressionP->X_op_symbol != NULL
	   && expressionP->X_add_number == 0)
    {
      expressionS two;
      expressionP->X_add_symbol = make_expr_symbol (expressionP);
      expressionP->X_op = O_right_shift;
      two.X_op = O_constant;
      two.X_add_symbol = two.X_op_symbol = NULL;
      two.X_add_number = 2;
      expressionP->X_op_symbol = make_expr_symbol (&two);
    }
  else
    {
      as_bad (_("expression too complex."));
      return;
    }
}

/* Parse an operand that is machine-specific. */

void
md_operand
(expressionS *expressionP ATTRIBUTE_UNUSED)
{
  return;
}

/* We have no need to default values of symbols. */

symbolS *
md_undefined_symbol
(char *name ATTRIBUTE_UNUSED)
{
  return NULL;
}

/* Functions concerning expressions.  */

/* Parse a .byte, .word, etc. expression.

   Values for the status register are specified with %st(label).
   `label' will be right shifted by 2.  */

void
qdsp6_parse_cons_expression(
     expressionS *exp,
     unsigned int nbytes ATTRIBUTE_UNUSED
)
{
  char *p = input_line_pointer;
  int code_symbol_fix = 0;

  for (; ! is_end_of_line[(unsigned char) *p]; p++)
    if (*p == '@' && !strncmp (p, "@h30", 4))
      {
	code_symbol_fix = 1;
	strcpy (p, ";   ");
      }
  expression (exp);

  if (code_symbol_fix)
    {
      qdsp6_code_symbol (exp);
      input_line_pointer = p;
    }
}

/* Functions concerning relocs.  */

/* A PC-relative jump should be calculated from the beginning of the packet.  */

long
md_pcrel_from (fixS *fixP)
{
  return (fixP->fx_frag->fr_address + fixP->fx_where + fixP->fx_pcrel_adjust);
}

/* Apply a fixup to the object code.  This is called for all the
   fixups we generated by the call to fix_new_exp, above.  In the call
   above we used a reloc code which was the largest legal reloc code
   plus the operand index.  Here we undo that to recover the operand
   index.  At this point all symbol values should be fully resolved,
   and we attempt to completely resolve the reloc.  If we can not do
   that, we determine the correct reloc code and put it back in the fixup.  */

void
md_apply_fix
(fixS *fixP, valueT *valP, segT seg)
{
  const qdsp6_operand *operand;
  valueT value = *valP;

  if (fixP->fx_addsy == (symbolS *) NULL)
    fixP->fx_done = 1;
  else if (fixP->fx_pcrel)
    {
      /* Hack around bfd_install_relocation brain damage.  */
      if (S_GET_SEGMENT (fixP->fx_addsy) != seg)
        value += md_pcrel_from (fixP);
      else
        fixP->fx_done = 1;
    }

  /* We can't actually support subtracting a symbol.  */
  if (fixP->fx_subsy != NULL)
    as_bad_where (fixP->fx_file, fixP->fx_line, _("expression too complex."));

  operand = (qdsp6_operand *) fixP->tc_fix_data;
  if (operand)
    {
      /* Fetch the instruction, insert the fully resolved operand
        value, and stuff the instruction back again.  */
      if (fixP->fx_done)
        {
          bfd_reloc_code_real_type rtype = operand->reloc_type;
          reloc_howto_type *howto = bfd_reloc_type_lookup (seg->owner, rtype);

          assert (howto);
          value &= howto->src_mask;

          qdsp6_insert_operand (fixP->fx_frag->fr_literal + fixP->fx_where,
                                operand, (offsetT) value, fixP);
        }
      else
        /* If any relocation is to be applied later, don't modify
          the section contents here; we will use the addend in
          the relocation to fix it up at link time... */
        qdsp6_insert_operand (fixP->fx_frag->fr_literal + fixP->fx_where,
                              operand, (offsetT) 0, fixP);
    }
  else if (fixP->fx_done)
    {
      switch (fixP->fx_r_type)
        {
          case BFD_RELOC_8:
            md_number_to_chars (fixP->fx_frag->fr_literal + fixP->fx_where,
                                value, 1);
            break;

          case BFD_RELOC_16:
            md_number_to_chars (fixP->fx_frag->fr_literal + fixP->fx_where,
                                value, 2);
            break;

          case BFD_RELOC_32:
            md_number_to_chars (fixP->fx_frag->fr_literal + fixP->fx_where,
                                value, 4);
            break;

          default:
            {
              asymbol *sym = symbol_get_bfdsym (fixP->fx_addsy);
              as_bad_where (fixP->fx_file, fixP->fx_line,
                            "unknown relocation for symbol `%s'.",
                            sym->name);
            }
        }
    }
}

/* Translate internal representation of relocation info to BFD target
   format.  */

arelent *
tc_gen_reloc
(asection *section ATTRIBUTE_UNUSED, fixS *fixP)
{
  arelent *reloc;

  reloc = (arelent *) xmalloc (sizeof (arelent));

  reloc->sym_ptr_ptr = (asymbol **) xmalloc (sizeof (asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixP->fx_addsy);
  reloc->address = fixP->fx_frag->fr_address + fixP->fx_where;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);
  if (reloc->howto == (reloc_howto_type *) NULL) {
    as_bad_where (fixP->fx_file, fixP->fx_line,
                  "assembler internal error: can't export reloc type %d (\"%s\").",
                  fixP->fx_r_type,
                  bfd_get_reloc_code_name (fixP->fx_r_type));
    return NULL;
  }

  assert (fixP->fx_pcrel == reloc->howto->pc_relative);

  /* fixP->fx_offset was not handled in md_apply_fix() so we
     need to propagate it into the (RELA) relocation */
  reloc->addend = fixP->fx_offset;

  return reloc;
}

/** Look for insns in the packet which restricts slot #1.

@return The insn index in the packet, otherwise -1.
*/
int
qdsp6_find_noslot1
(const qdsp6_packet *packet, size_t current)
{
  size_t i;

  for (i = 0; i < qdsp6_packet_count (packet); i++)
    if (i != current && !packet->insns [i].used )
      if (packet->insns [i].opcode->attributes & A_RESTRICT_NOSLOT1)
	return (i);

  return (-1);
}

/** For V2, discard specified number of DCFETCH.

It must not be called after packet has been written out.
@param number # DCFETCH to discard, with "0" meaning all but one.
@return # replacements performed.
 */
int
qdsp6_discard_dcfetch
(qdsp6_packet *apacket, int number)
{
  int count = 0, found = 0;
  size_t i;

    // dcfetch fix:
    // for V2, there cannot be 2 or more dcfetch instructions
    // and a dcfetch cannot go into ndx 0
  if (!qdsp6_if_arch_v1 () && qdsp6_packet_count (apacket) > 1)
    {
      if (!number)
        /* Replace all but one DCFETCH instruction. */
        {
          for (i = 0; i < qdsp6_packet_count (apacket); i++)
            {
              if (!strncasecmp (apacket->insns [i].opcode->syntax,
                                QDSP6_DCFETCH, QDSP6_DCFETCH_LEN))
                {
                  found++;

                  if (found > MAX_DCFETCH)
                    as_warn (_("more than one `dcfetch' instruction in packet."));

                  if (found > 1)
                    /* Delete extra DCFETCH. */
                    {
                      as_warn_where (NULL, apacket->insns [i].lineno,
                                    _("extra `dcfetch' removed."));

                      apacket->insns [i]        = qdsp6_nop_insn;
                      apacket->insns [i].padded = TRUE;

                      count++;
                    }
                }
            }
        }
      else
        /* Replace specified number of DCFETCH. */
        {
          for (i = 0; number && i < qdsp6_packet_count (apacket); i++)
            {
              if (!strncasecmp (apacket->insns [i].opcode->syntax,
                                QDSP6_DCFETCH, QDSP6_DCFETCH_LEN))
                {
                  found++;

                  as_warn_where (NULL, apacket->insns [i].lineno,
                                _("`dcfetch' removed."));

                  apacket->insns [i]        = qdsp6_nop_insn;
                  apacket->insns [i].padded = TRUE;

                  number--;
                  count++;
                }
            }
        }
    }

  return (count);
}

/** Shuffle packet accoding to from-to mapping.

@param packet reference.
@param fromto mapping array.
*/
void
qdsp6_shuffle_packet
(qdsp6_packet *packet, size_t *fromto)
{
  qdsp6_packet_insn insns [MAX_PACKET_INSNS], prefixes [MAX_PACKET_INSNS];
  size_t from, ndx;
  size_t i;

  for (i = 0, ndx = 0; i < MAX_PACKET_INSNS; i++)
    {
      from = fromto [i];
      if (from >= MAX_PACKET_INSNS)
	from = i;

      /* Shuffle insns, prefixes, fix-ups... */
      insns [i]    = packet->insns    [from];
      prefixes [i] = packet->prefixes [from];

      /* Fix up fix ups. */
      if (insns [i].fc && insns [i].fix)
        {
          insns [i].fix->fx_where   = i * QDSP6_INSN_LEN;
          insns [i].fix->fx_offset += insns [i].fix->fx_pcrel
                                      ? (i - from) * QDSP6_INSN_LEN
                                      : 0;
        }
      if (prefixes [i].fc && prefixes [i].fix)
        {
          prefixes [i].fix->fx_where   = i * QDSP6_INSN_LEN;
          prefixes [i].fix->fx_offset += prefixes [i].fix->fx_pcrel
                                         ? (i - from) * QDSP6_INSN_LEN
                                         : 0;
        }

      insns [i].ndx = ndx;
      ndx += packet->insns [from].padded? 0: 1;
    }

  memcpy (packet->insns,    insns,    sizeof (packet->insns));
  memcpy (packet->prefixes, prefixes, sizeof (packet->prefixes));
}

/** Shuffle a packet according to architectural restrictions.

@param packet Packet reference.
@param slot_num Slot number (initially called for slot 0).
@param shuffle Optional pre-initialized from-to helper array (typically NULL).
@return True if packet was shuffled succesfully.
*/
int
qdsp6_shuffle_helper
(qdsp6_packet *packet, size_t slot_num, size_t *shuffle)
{
  size_t ndx, slot_mask, temp_mask, store_mask;
  size_t *fromto, aux [MAX_PACKET_INSNS];
  qdsp6_packet_insn *inew = NULL;
  int single, prefix, rnew, store, nostore;
  int changed;
  size_t i;

  if (slot_num >= qdsp6_packet_count (packet))
    return TRUE;

  /* Don't touch a packet with prefixes. */
  if ((prefix = qdsp6_has_prefix (packet)))
    return FALSE;

  single  = qdsp6_has_single (packet);
  store   = qdsp6_has_store (packet);
  nostore = qdsp6_has_store_not (packet);

  rnew = qdsp6_has_rnew (packet, &inew);

  ndx       = MAX_PACKET_INSNS - slot_num - 1;
  slot_mask = 1 << ndx;

  /* Choose from-to map. */
  if (shuffle)
    fromto = shuffle;
  else
    {
      for (i = 0; i < MAX_PACKET_INSNS; i++)
        {
          aux [i] = MAX_PACKET_INSNS;
          packet->insns [i].used = FALSE;
        }

      fromto = aux;
    }

  store_mask = 2;
  for (i = 0, changed = FALSE;
       i < qdsp6_packet_count (packet) && !changed;
       i++)
    {
      temp_mask = packet->insns [i].opcode->slot_mask;

      /* If there is only one memory insn, it requires ndx #0. */
      if (single
	  && (packet->insns [i].opcode->attributes & A_RESTRICT_SINGLE_MEM_FIRST))
	temp_mask &= 1;

      /* If there is a store restriction, make sure that none makes into slot #1. */
      if (nostore
          && !(packet->insns [i].opcode->attributes & A_RESTRICT_NOSLOT1_STORE)
          && (packet->insns [i].opcode->attributes & A_STORE))
        temp_mask &= ~2;

      /* Make sure that several stores follow source order. */
      if ((packet->insns [i].opcode->attributes & A_STORE))
        {
          if (store > 1)
            {
              temp_mask &= store_mask;
              store_mask >>= 1;
            }
          else
            temp_mask &= 1;
        }

      /* If there is a R.NEW insn, then the insn whose output GPR is the same
         requires ndx #1. */
      if (rnew
          && (packet->insns [i].flags & QDSP6_INSN_OUT_RNEW)
          && (packet->insns [i].oreg == inew->ireg))
	temp_mask &= QDSP6_RNEW_SLOTS;

      if (!packet->insns [i].used && (temp_mask & slot_mask))
	{
	  /* Check for NOSLOT1 restriction. */
	  if (ndx == 1
              && !qdsp6_is_nop (packet->insns [i].insn)
              && qdsp6_find_noslot1 (packet, i) >= 0)
            /* If so, then skip this ndx. */
            continue;

          /* Allocate this slot. */
	  fromto [slot_num] = i;
	  packet->insns [i].used = TRUE;

          /* Try to shuffle remaining slots. */
	  if (!(changed = qdsp6_shuffle_helper (packet, slot_num + 1, fromto)))
            /* Deallocate ndx if unsuccessful. */
            packet->insns [i].used = FALSE;
        }
    }

  /* Shuffle packet if successful and at the top-most call. */
  if (changed && !shuffle)
    {
      /* Shuffle the packet according to the from-to map. */
      qdsp6_shuffle_packet (packet, fromto);
      /* Finish up the insns in the packet. */
      qdsp6_packet_finish (packet);
    }

  return (changed);
}

void
qdsp6_shuffle_handle
(qdsp6_packet *apacket)
{
  if (!qdsp6_shuffle_helper (apacket, 0, NULL))
    {
      /* Try to discard a DCFETCH before trying again. */
      if (!(qdsp6_discard_dcfetch (apacket, 1)
            && qdsp6_shuffle_helper (apacket, 0, NULL)))
        as_bad (_("unable to shuffle instructions in packet."));
    }
}

/** Main function for packet shuffling.
*/
void
qdsp6_shuffle_do
(qdsp6_packet *apacket)
{
  qdsp6_packet_insn insn;
  size_t fromto [MAX_PACKET_INSNS];
  char *file;
  char found, has_prefer_slot0;
  size_t i, j, k;

      /* Initialize shuffle map. */
      for (i = 0; i < MAX_PACKET_INSNS; i++)
        fromto [i] = MAX_PACKET_INSNS;

      /* Get rid of extra insns. */
      qdsp6_discard_dcfetch (apacket, 0);

      /* Reorder the instructions in the packet so that they start with
         slot #3 (index #0) instead of slot #0.

         1 - Go through all non-padded instructions, find the highest slot each
             insn can go to and put it in the corresponding position.  If an
             position is already assigned, then try the next one.  If all
             positions after the wanted one are taken, then try again from the
             beginning of the array.

         2 - For all padded instructions, put them one by one starting from the
             first available position (or higher slot). */

      /* Step 1. */
      for (i = 0; i < MAX_PACKET_INSNS; i++)
        {
          if (!apacket->insns [i].padded)
            {
              /* Find the highest ndx that this instruction can go to. */
              for (j = 0, found = FALSE;
                   j < MAX_PACKET_INSNS && !found;
                   j++)
                if ((  apacket->insns [i].opcode->slot_mask
                     & (1 << (MAX_PACKET_INSNS - j - 1))))
                  {
                    /* Try to allocate the ndx found or a lower one. */
                    for (k = j;
                          k < MAX_PACKET_INSNS && !found;
                          k++)
                      if (fromto [k] >= MAX_PACKET_INSNS)
                        {
                          /* Slot is free. */
                          fromto [k] = i;
                          found = TRUE;
                        }
                  }

              /* If all the positions after the wanted one are taken,
                 go back to the beginning of the array. */
	      for (j = 0;
		   j < MAX_PACKET_INSNS && !found;
		   j++)
		if (fromto [j] >= MAX_PACKET_INSNS)
		  {
		    fromto [j] = i;
		    found = TRUE;
                  }
            }
        }

      /* Step 2. */
      for (i = 0; i < MAX_PACKET_INSNS; i++)
	{
          if (apacket->insns [i].padded)
	    {
              /* Find the first unused position for this padded insn. */
              for (j = 0; j < MAX_PACKET_INSNS; j++)
		{
                  if (fromto [j] >= MAX_PACKET_INSNS)
		    {
                      fromto [j] = i;
                      break;
                    }
                }
            }
        }

      qdsp6_shuffle_packet (apacket, fromto);

      for (i = 0, has_prefer_slot0 = FALSE;
           i < MAX_PACKET_INSNS && !has_prefer_slot0;
           i++)
	{
          if ((apacket->insns [i].opcode->attributes & A_RESTRICT_PREFERSLOT0))
	    {
              /* Swap with insn at slot #0. */
              insn = apacket->insns [i];
              apacket->insns [i] = apacket->insns [MAX_PACKET_INSNS - 1];
              apacket->insns [MAX_PACKET_INSNS - 1] = insn;

              has_prefer_slot0 = TRUE;
            }
        }


  qdsp6_shuffle_handle (apacket);

  if (has_prefer_slot0)
    {
      as_where (&file, NULL);

      for (i = 0; i < MAX_PACKET_INSNS - 1; i++)
	  if ((  apacket->insns [i].opcode->attributes & A_RESTRICT_PREFERSLOT0))
	      as_warn_where (file, apacket->insns [i].lineno,
	                     _("instruction `%s' prefers ndx #0, "
                               "but has been assigned to ndx #%lu."),
			     apacket->insns [i].opcode->syntax, i);
    }
}

/** Finish up packet after shuffling.

Finish encoding any insn in the packet that may depend of shuffling completed.

@param apacket Packet to finish.
*/
void
qdsp6_packet_finish
(qdsp6_packet *packet)
{
  size_t i;

  /* Finish R.NEW insns. */
  {
    qdsp6_packet_insn *inew, *onew;
    size_t off;

    if (qdsp6_has_rnew (packet, &inew))
      {
        for (i = 0, off = 0, onew = NULL; i < MAX_PACKET_INSNS; i++)
          {
            if ((packet->insns [i].flags & QDSP6_INSN_OUT_RNEW)
                && (packet->insns [i].oreg == inew->ireg))
              {
                off = inew->ndx - packet->insns [i].ndx;
                onew = packet->insns + i;
                break;
              }
          }

        if (onew)
          qdsp6_encode_operand
            (inew->ioperand, &inew->insn, inew->opcode,
             off * 2 + inew->ireg % 2, NULL, FALSE, FALSE, NULL);
        else
          as_bad_where (NULL, inew->lineno,
                        _("register `r%u' used with `.new' "
                          "but not modified in the same packet."),
                        inew->ireg);
      }

  }
}

/** Determine if an instruction is a NOP.

@param insn An insn.
@return True if NOP.
*/

int
qdsp6_is_nop
(qdsp6_insn insn)
{
  const qdsp6_opcode *opcode = qdsp6_lookup_insn (insn);

  return (opcode->attributes & A_IT_NOP);
}

/** Determine if an instruction is a prefix.

@param insn An insn.
@return True if a prefix.
*/

int
qdsp6_is_prefix
(qdsp6_insn insn)
{
  const qdsp6_opcode *opcode = qdsp6_lookup_insn (insn);

  return (opcode->attributes & A_IT_EXTENDER);
}

qdsp6_insn
qdsp6_find_insn
(const char *insn)
{
  const qdsp6_opcode *opcode = qdsp6_opcode_lookup_asm (insn);

  /* Keep looking until we find a match.  */
  for (; opcode != NULL; opcode = QDSP6_OPCODE_NEXT_ASM (opcode))
    if (!strncasecmp (opcode->syntax, insn, strlen (insn)))
      break;

  assert (opcode);
  return (qdsp6_encode_opcode (opcode->enc));
}

qdsp6_insn
qdsp6_find_nop
(void)
{
  qdsp6_nop_insn.insn   = qdsp6_nop;
  qdsp6_nop_insn.opcode = qdsp6_lookup_insn (qdsp6_nop_insn.insn);

  return (qdsp6_nop);
}

qdsp6_insn
qdsp6_find_kext
(void)
{
  if (qdsp6_if_arch_kext ())
    {
      qdsp6_kext_insn.insn   = qdsp6_kext;
      qdsp6_kext_insn.opcode = qdsp6_lookup_insn (qdsp6_kext_insn.insn);

      return (qdsp6_kext);
    }
  else
    {
      memset (&qdsp6_kext_insn, 0, sizeof (qdsp6_kext_insn));
      return FALSE;
    }
}

/** Check if a predicate register with the .new suffix is updated in the packet.

@return True if it is, false otherwise.
*/
int
qdsp6_check_new_predicate
(void)
{
  size_t i;

  /* No need to check for V1, since P.new read array will never be set. */
  for (i = 0; i < QDSP6_NUM_PREDICATE_REGS; i++)
    if (pNewArray [i].used && (!pArray [i].used || pLateArray [i].used))
      {
        if (cArray [QDSP6_P30].used)
          as_warn (_("modifying `C4/P3:0' does not apply to `p%lu.new'."),
                   (unsigned long) i);

        if (pArray [i].used && pLateArray [i].used)
          as_bad
            (_("cannot use register `p%lu' with `.new' in the packet."),
             (unsigned long) i);
        else
          as_bad
            (_("register `p%lu' used with `.new' but not modified in the same packet."),
             (unsigned long) i);

        return FALSE;
      }

  return TRUE;
}

/** Check if a predicate register is implicitly updated in the packet.
*/
void
qdsp6_check_implicit_predicate
(const qdsp6_opcode *opcode, unsigned int implicit, int reg)
{
  int is_used;

  if (opcode->implicit_reg_def & implicit)
    {
      is_used = !qdsp6_autoand || (opcode->attributes & A_RESTRICT_LATEPRED)
                ? TRUE: MAYBE;

      if (pArray [reg].used == TRUE || (pArray [reg].used == MAYBE && is_used == TRUE))
        as_bad (_("register `p%d' modified more than once."), reg);

      pArray [reg].used = TRUER (pArray [reg].used, is_used);
    }
}

void
qdsp6_check_implicit
(const qdsp6_opcode *opcode, unsigned int implicit, int reg,
 qdsp6_reg_score *array, const char *name)
{
  if (opcode->implicit_reg_def & implicit)
    {
      if (array [reg].used)
        as_bad (_("register `%s' modified more than once."), name);
      else
        array [reg].used++;
    }
}

void
qdsp6_check_register
(qdsp6_reg_score array [], int reg_num, int pred_reg_rd_mask,
 const qdsp6_operand *operand, qdsp6_packet_insn *insn, size_t n)
{
  char *errmsg = NULL;
  char buff [100];
  char *reg_name;
  int prev_pred_reg_rd_mask;
  int a_pred_mask, prev_a_pred_mask;
  int mult_wr_err = FALSE;

  if (array [reg_num].used)
    {
      prev_pred_reg_rd_mask = array [reg_num].pred;

      if (!pred_reg_rd_mask || !prev_pred_reg_rd_mask)
        {
          if (array [reg_num].letter != operand->enc_letter
              ||    (operand->flags & (QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE))
                 != (QDSP6_OPERAND_IS_READ | QDSP6_OPERAND_IS_WRITE)
              || array [reg_num].ndx != n)
            mult_wr_err = TRUE;
        }
      else
        {
          size_t i;

          // Or same non-zero mask for same predicate
          for (i = 0; i < QDSP6_NUM_PREDICATE_REGS; i++)
            {
              a_pred_mask = QDSP6_PRED_GET (pred_reg_rd_mask, i);
              prev_a_pred_mask = QDSP6_PRED_GET (prev_pred_reg_rd_mask, i);
              if (a_pred_mask && a_pred_mask == prev_a_pred_mask)
                {
                  mult_wr_err = TRUE;
                  break;
                }
            }
        }

      if (mult_wr_err)
        {
          reg_name
            = qdsp6_dis_operand
                (operand, insn [n].insn, 0, insn [n].opcode->enc, buff, &errmsg);

          if (reg_name)
            as_bad (_("register `%s' modified more than once."), buff);
          else if (errmsg)
            as_bad (errmsg);
        }
      else
        {
          array [reg_num].used    = TRUE;
          array [reg_num].letter  = operand->enc_letter;
          array [reg_num].pred   |= pred_reg_rd_mask;
          array [reg_num].ndx     = n;
        }
    }
  else
    {
      array [reg_num].used   = TRUE;
      array [reg_num].letter = operand->enc_letter;
      array [reg_num].pred   = pred_reg_rd_mask;
      array [reg_num].ndx    = n;
    }
}

void
qdsp6_check_predicate
(int reg_num, const qdsp6_opcode *opcode)
{
  if (pArray [reg_num].used == TRUE)
    as_bad (_("register `p%d' modified more than once."), reg_num);
  else
    pArray [reg_num].used =
      (!qdsp6_autoand || (opcode->attributes & A_RESTRICT_LATEPRED))
      ? TRUE: MAYBE;
}

void
qdsp6_check_insn
(qdsp6_packet *apacket, size_t n)
{
  char *errmsg = NULL;
  int reg_num;
  qdsp6_reg_score *arrayPtr;
  char *cp;
  int pred_reg_rd_mask = 0;

  // Check whether the instruction is legal inside the packet
  // But allow single instruction in packet
  if((apacket->insns [n].opcode->attributes & A_RESTRICT_NOPACKET)
     && qdsp6_packet_insns (apacket) > 1)
    {
      as_bad (_("instruction cannot appear in packet with other instructions."));
      qdsp6_in_packet = 0;
      return;
    }

  //Check loop can not exist in the same packet as branch label instructions
  //they are using the same adder
  if ((apacket->insns [n].opcode->attributes & A_RESTRICT_BRANCHADDER_MAX1))
    numOfBranchAddrMax1++;

  if ((apacket->insns [n].opcode->attributes & A_BRANCHADDER))
    numOfBranchAddr++;

  if ((apacket->insns [n].opcode->attributes & A_RESTRICT_COF_MAX1))
    numOfBranchMax1++;

  if ((apacket->insns [n].opcode->attributes & A_RELAX_COF_1ST)
      || (apacket->insns [n].opcode->attributes & A_RELAX_COF_2ND))
    numOfBranchRelax++;

  if ((apacket->insns [n].opcode->attributes & A_RESTRICT_LOOP_LA))
    numOfLoopMax1++;

  // check for implicit register definitions
  if (apacket->insns [n].opcode->implicit_reg_def)
    {
      if (apacket->insns [n].opcode->implicit_reg_def & IMPLICIT_PC)
        {
          /* Look into multiple implicit references to the PC in order to allow
             slots with two branches in V3. */
          cArray [QDSP6_PC].used++;
          if (cArray [QDSP6_PC].used > 1 && cArray [QDSP6_PC].used > numOfBranchRelax)
            qdsp6_check_implicit (apacket->insns [n].opcode, IMPLICIT_PC, QDSP6_PC, cArray, "c9/pc");
        }

      qdsp6_check_implicit
        (apacket->insns [n].opcode, IMPLICIT_LR,  QDSP6_LR,  gArray, "r31/lr");
      qdsp6_check_implicit
        (apacket->insns [n].opcode, IMPLICIT_SP,  QDSP6_SP,  gArray, "r29/sp");
      qdsp6_check_implicit
        (apacket->insns [n].opcode, IMPLICIT_FP,  QDSP6_FP,  gArray, "r30/fp");
      qdsp6_check_implicit
        (apacket->insns [n].opcode, IMPLICIT_LC0, QDSP6_LC0, cArray, "c1/lc0");
      qdsp6_check_implicit
        (apacket->insns [n].opcode, IMPLICIT_SA0, QDSP6_SA0, cArray, "c0/sa0");
      qdsp6_check_implicit
        (apacket->insns [n].opcode, IMPLICIT_LC1, QDSP6_LC1, cArray, "c3/lc1");
      qdsp6_check_implicit
        (apacket->insns [n].opcode, IMPLICIT_SA1, QDSP6_SA1, cArray, "c2/sa1");
      qdsp6_check_implicit_predicate
        (apacket->insns [n].opcode, IMPLICIT_P3,  3);
      qdsp6_check_implicit_predicate
        (apacket->insns [n].opcode, IMPLICIT_P1,  1); /* V4 */
      qdsp6_check_implicit_predicate
        (apacket->insns [n].opcode, IMPLICIT_P0,  0); /* V3 */
#if 0
      qdsp6_check_implicit
        (apacket->insns [n].opcode,
         (apacket->insns [n].opcode->attributes & A_RESTRICT_LATEPRED)
         ? IMPLICIT_P3: 0, 3, pLateArray, NULL);
      qdsp6_check_implicit
        (apacket->insns [n].opcode,
         (apacket->insns [n].opcode->attributes & A_RESTRICT_LATEPRED)
         ? IMPLICIT_P1: 0, 1, pLateArray, NULL); /* V4 */
      qdsp6_check_implicit
        (apacket->insns [n].opcode,
         (apacket->insns [n].opcode->attributes & A_RESTRICT_LATEPRED)
         ? IMPLICIT_P0: 0, 0, pLateArray, NULL); /* V3 */
#endif
    }

  /* Check for attributes. */
  if ((apacket->insns [n].opcode->attributes)
      && (apacket->insns [n].opcode->attributes & A_RESTRICT_NOSRMOVE))
    implicit_sr_ovf_bit_flag = 1;

  for (cp = apacket->insns [n].opcode->syntax; *cp; cp++)
    {
      // Walk the syntax string for the opcode
      if (ISUPPER (*cp))
        {
          // Check for register operand
          // Get the operand from operand lookup table
          const qdsp6_operand *operand = qdsp6_lookup_operand (cp);
          if(operand == NULL)
            break;

          cp += strlen (operand->fmt); // Move the pointer to the end of the operand

          if ((operand->flags & QDSP6_OPERAND_IS_READ)
              && (operand->flags & QDSP6_OPERAND_IS_PREDICATE))
            {
              if (!qdsp6_extract_predicate_operand
                   (operand, apacket->insns [n].insn,
                    apacket->insns [n].opcode->enc, &reg_num, &errmsg))
                {
                  if (errmsg)
                    as_bad (errmsg);
                  break;
                }

              pred_reg_rd_mask = QDSP6_PRED_SET (0, reg_num, QDSP6_PRED_YES);
              if (apacket->insns [n].opcode->attributes & CONDITION_SENSE_INVERTED)
                pred_reg_rd_mask
                = QDSP6_PRED_SET (pred_reg_rd_mask, reg_num, QDSP6_PRED_NOT);
              if (apacket->insns [n].opcode->attributes & CONDITION_DOTNEW)
                {
                  pred_reg_rd_mask
                  = QDSP6_PRED_SET (pred_reg_rd_mask, reg_num, QDSP6_PRED_NEW);
                  pNewArray [reg_num].used = TRUE;
                }
            }

          if (operand->flags & QDSP6_OPERAND_IS_WRITE)
            {
              if (operand->flags & QDSP6_OPERAND_IS_PREDICATE)
                {
                  if (!qdsp6_extract_predicate_operand
                        (operand, apacket->insns [n].insn,
                         apacket->insns [n].opcode->enc, &reg_num, &errmsg))
                    {
                      if (errmsg)
                        as_bad (errmsg);
                      break;
                    }

                  if (cArray [QDSP6_P30].used)
                    {
                      as_bad (_("register `P%d' modified more than once."), reg_num);
                      break;
                    }

                  if (apacket->insns [n].opcode->attributes & A_RESTRICT_LATEPRED)
                    pLateArray [reg_num].used++;
                  qdsp6_check_predicate (reg_num, apacket->insns [n].opcode);

                  if (operand->flags & QDSP6_OPERAND_IS_PAIR)
                    qdsp6_check_predicate (reg_num + 1, apacket->insns [n].opcode);

                  continue;
                }
              else if (operand->flags & QDSP6_OPERAND_IS_MODIFIER)
                {
                  arrayPtr = cArray;

                  if (!qdsp6_extract_modifier_operand
                         (operand, apacket->insns [n].insn,
                          apacket->insns [n].opcode->enc, &reg_num, &errmsg))
                    {
                      if (errmsg)
                        as_bad (errmsg);
                      break;
                    }
                }
              else
                {
                  if (!qdsp6_extract_operand
                         (operand, apacket->insns [n].insn, 0,
                          apacket->insns [n].opcode->enc, &reg_num, &errmsg))
                    {
                      if (errmsg)
                        as_bad (errmsg);
                      break;
                    }

                  if (operand->flags & QDSP6_OPERAND_IS_CONTROL)
                    {
                      if (reg_num == QDSP6_P30)
                        {
                          // Set pArray completely
                          size_t j;

                          for (j = 0; j < QDSP6_NUM_PREDICATE_REGS; j++)
                            if (pArray [j].used)
                              {
                                //as_bad (("register `C%d' modified more than once."), reg_num);
                                cArray [reg_num].used++;
                                break;
                              }
                        }
                      arrayPtr = cArray;
                    }
                  else if (operand->flags & QDSP6_OPERAND_IS_SYSTEM)
                    arrayPtr = sArray;
                  else
                    {
                      arrayPtr = gArray;

                      if (!(apacket->insns [n].flags & QDSP6_INSN_OUT_RNEW))
                        {
                          /* Record the first modified GPR. */
                          apacket->insns [n].flags |= QDSP6_INSN_OUT_RNEW;
                          apacket->insns [n].oreg   = reg_num;
                        }
                    }
                }

              qdsp6_check_register
                (arrayPtr, reg_num, pred_reg_rd_mask,
                 operand, apacket->insns, n);
              if (operand->flags & QDSP6_OPERAND_IS_PAIR)  // For register pairs
                qdsp6_check_register
                  (arrayPtr, reg_num + 1, pred_reg_rd_mask,
                   operand, apacket->insns, n);
            }
        } // end if for CAPITAL letters indicating it is a register
      if (!*cp)
        break;
    } // end for loop for walking the syntax string
}

void
qdsp6_init_reg
(void)
{
  memset (gArray,     0, sizeof (gArray));
  memset (cArray,     0, sizeof (cArray));
  memset (sArray,     0, sizeof (sArray));
  memset (pArray,     0, sizeof (pArray));
  memset (pNewArray,  0, sizeof (pNewArray));
  memset (pLateArray, 0, sizeof (pLateArray));
}

/** Return the number of slots.

@param apacket A packet reference.
*/
size_t
qdsp6_packet_size
(const qdsp6_packet *apacket)
{
  return (apacket->size + apacket->duplex + apacket->prefix + apacket->relax);
}

/** Return the number of bytes.

@param apacket A packet reference.
*/
size_t
qdsp6_packet_length
(const qdsp6_packet *apacket)
{
  return (  QDSP6_INSN_LEN
          * (apacket->size + apacket->prefix + apacket->relax));
}

/** Return the number of insns.

@param apacket A packet reference.
*/
size_t
qdsp6_packet_count
(const qdsp6_packet *apacket)
{
  return (apacket->size);
}

/** Return the number of effective insns.

@param apacket A packet reference.
*/
size_t
qdsp6_packet_insns
(const qdsp6_packet *apacket)
{
  size_t i, n;

  for (i = n = 0; i < apacket->size; i++)
    if (!apacket->insns [i].padded)
      n++;

  return (n);
}

void
qdsp6_packet_open
(qdsp6_packet *apacket)
{
  qdsp6_packet_begin (apacket);
}

void
qdsp6_packet_close
(qdsp6_packet *apacket)
{
  int inner, outer;

  qdsp6_packet_end (apacket);

  qdsp6_packet_end_lookahead (&inner, &outer);
  if (inner)
    qdsp6_packet_end_inner (apacket);
  if (outer)
    qdsp6_packet_end_outer (apacket);
}

  /* Used to handle packet begin/end syntax */

void
qdsp6_packet_begin
(qdsp6_packet *apacket)
{
  if (qdsp6_in_packet)
    {
      qdsp6_packet_close (apacket);
      qdsp6_packet_write (apacket);
    }

  qdsp6_in_packet = TRUE;
  qdsp6_packet_init (apacket);
}

/* Function to write packet header, encode instructions in a packet,
   and perform various assembler restriction checks */

void
qdsp6_packet_end
(qdsp6_packet *apacket)
{
  size_t i;

  qdsp6_init_reg ();

  implicit_sr_ovf_bit_flag = 0;
  numOfBranchAddr = 0;
  numOfBranchRelax = 0;
  numOfBranchAddrMax1 = 0;
  numOfBranchMax1 = 0;
  numOfLoopMax1 = 0;

  /* Pad packet with NOPs. */
  while (qdsp6_packet_insert (apacket, &qdsp6_nop_insn, NULL, NULL, TRUE))
    ;

  /* Checking for multiple writes to the same register in a packet. */
  for (i = 0; i < apacket->size; i++)
    qdsp6_check_insn (apacket, i);

  // check for multiple writes to SR (implicit not allowed if explicit writes are present)
  if (implicit_sr_ovf_bit_flag && cArray [QDSP6_SR].used)
    {
      as_bad (_("`OVF' bit in `SR' register cannot be set (implicitly or explicitly) more than once in packet."));
      qdsp6_in_packet = FALSE;
      return;
    }

  if (!qdsp6_check_new_predicate ())
    {
      qdsp6_in_packet = FALSE;
      return;
    }

  if (numOfBranchAddrMax1 && numOfBranchAddr > 1)
    {
      as_bad (_("loop setup and direct branch instructions cannot be in same packet."));
      qdsp6_in_packet = FALSE;
      return;
    }

  if ((numOfBranchMax1 > 1 && numOfBranchMax1 > numOfBranchRelax)
      || (numOfBranchAddr > 1 && numOfBranchAddr > numOfBranchRelax))
    {
      as_bad (_("too many branches in packet."));
      qdsp6_in_packet = FALSE;
      return;
    }

  if (qdsp6_no_dual_memory && qdsp6_has_mem (qdsp6_packets) > 1)
    as_bad (_("multiple memory operations in packet."));

    if (qdsp6_pairs_info && qdsp6_has_pair (apacket))
    as_warn (_("instructions paired."));

  qdsp6_shuffle_do (apacket);

  qdsp6_in_packet = FALSE;
}

int
qdsp6_packet_check_solo
(const qdsp6_packet *apacket)
{
  int solo;
  size_t i;

  /* Solo insns can not exist in a packet. */
  for (i = 0, solo = FALSE; i < apacket->size && !solo; i++)
    if (apacket->insns [i].opcode->attributes & A_RESTRICT_NOPACKET)
      solo = TRUE;

  return (solo);
}

/** Validate end of inner loop packet.
*/
void
qdsp6_packet_end_inner
(qdsp6_packet *apacket)
{
  /* Check whether registers updated by :endloop0 are updated in packet. */
  if (  cArray [QDSP6_P30].used | cArray [QDSP6_SR].used
      | cArray [QDSP6_SA0].used | cArray [QDSP6_LC0].used | cArray [QDSP6_PC].used)
    as_bad (_("packet marked with `:endloop0' cannot contain instructions that " \
              "modify registers `C4/P3:0', `C8/USR', `SA0', `LC0' or `PC'."));

  /* Although it may be dangerous to modify P3 then, legacy code is full of this. */
/*
  if (pArray [3])
    as_warn (_("packet marked with `:endloop0' that contain instructions that " \
               "modify register `p3' may have undefined results if "
               "ending a pipelined loop."));
*/

  /* Check for a solo instruction in a packet with :endloop0. */
  if (qdsp6_packet_check_solo (apacket))
    as_bad (_("packet marked with `:endloop0' cannot contain a solo instruction."));

  apacket->is_inner = TRUE;
}

/** Validate end of outer loop packet.
*/
void
qdsp6_packet_end_outer
(qdsp6_packet *apacket)
{
  /* Check whether registers updated by :endloop1 are updated in packet. */
  if (cArray [QDSP6_SA1].used | cArray [QDSP6_LC1].used | cArray [QDSP6_PC].used)
    as_bad (_("packet marked with `:endloop1' cannot contain instructions that " \
              "modify registers `SA1', `LC1' or `PC'."));

  /* Check for a solo instruction in a packet with :endloop1. */
  if (qdsp6_packet_check_solo (apacket))
    as_bad (_("packet marked with `:endloop1' cannot contain a solo instruction."));

  apacket->is_outer = TRUE;
}

/*
 * At the end of a packet, look ahead in the input stream
 * for any :endloop0 or :endloop1 directives associated with
 * the packet. Stop when we see something that isn't one
 * of those directives, whitespace, or newline. Handle newlines
 * properly.
 *
 */
void
qdsp6_packet_end_lookahead
(int *inner_p, int *outer_p)
{
  char *buffer_limit = get_buffer_limit ();
  int inner = 0;
  int outer = 0;

  for (;;)
    {
      if (input_line_pointer == buffer_limit)
        {
          /* reached the end of this buffer, get a new one? */
          buffer_limit = input_scrub_next_buffer (&input_line_pointer);
	  put_buffer_limit (buffer_limit);

          if (buffer_limit == 0)
            break;
        }
      else if (*input_line_pointer == '\n')
        {
          if (input_line_pointer[-1] == '\n')
            bump_line_counters ();

          input_line_pointer += 1;
        }
      else if (*input_line_pointer == ' ')
        {
          if (input_line_pointer [-1] == '\n')
            bump_line_counters ();

          input_line_pointer += 1;
        }
      else if (*input_line_pointer == '\0')
        {
          /* if we're called from md_assemble() we may still be on the \0 terminator */
          if (input_line_pointer [-1] == '\n')
            bump_line_counters ();

          input_line_pointer += 1;
        }
      else if (!inner
               && !strncmp (input_line_pointer,
                            PACKET_END_INNER, strlen (PACKET_END_INNER)))
        {
          if (input_line_pointer [-1] == '\n')
            bump_line_counters ();

          input_line_pointer += strlen (PACKET_END_INNER);
          inner = 1;

          if (outer)
            break;
        }
      else if (!outer
               && !strncmp (input_line_pointer,
                            PACKET_END_OUTER, strlen (PACKET_END_OUTER)))
        {
          if (input_line_pointer [-1] == '\n')
            bump_line_counters ();

          input_line_pointer += strlen (PACKET_END_OUTER);
          outer = 1;

          if (inner)
            break;
        }
      else
        {
          /*if (input_line_pointer[-1] == '\n') bump_line_counters();*/
          break;
        }
    }

  *inner_p = inner;
  *outer_p = outer;
}

/* Called by the assembler parser when it can't recognize a line ...

   At this point, "ch" is the next character to be processed and
   "input_line_pointer" has already been moved past "ch".
 */

int
qdsp6_unrecognized_line
(int ch ATTRIBUTE_UNUSED)
{
  char *str = input_line_pointer - 1;

  if (*str == PACKET_BEGIN)
    {
      if (qdsp6_in_packet)
        as_warn (_("found `%c' inside a packet."), PACKET_BEGIN);

      qdsp6_packet_open (qdsp6_packets + 0);

      return TRUE;
    }
  else if (*str == PACKET_END)
    {
      if (!qdsp6_in_packet)
        as_warn (_("found `%c' before opening a packet."), PACKET_END);

      qdsp6_packet_close (qdsp6_packets + 0);
      qdsp6_packet_write (qdsp6_packets + 0);

      return TRUE;
    }
  else if (!strncmp (str, PACKET_END_INNER, strlen (PACKET_END_INNER)))
    {
      as_warn (_("found `%s' when not closing a packet."), PACKET_END_INNER);

      input_line_pointer += strlen (PACKET_END_INNER) - 1;
      return TRUE;
    }
  else if (!strncmp (str, PACKET_END_OUTER, strlen (PACKET_END_OUTER)))
    {
      as_warn (_("found `%s' when not closing a packet."), PACKET_END_OUTER);

      input_line_pointer += strlen (PACKET_END_OUTER) - 1;
      return TRUE;
    }

  /* Not a valid line */
  return FALSE;
}

void
qdsp6_cleanup
(void)
{
  size_t n_faligned;

  if (qdsp6_in_packet)
    {
      as_warn (_("reached end of file before closing a packet."));

      qdsp6_packet_close (qdsp6_packets + 0);
      qdsp6_packet_write (qdsp6_packets + 0);
    }

  /* Add up effective instances of .falign. */
  n_faligned = n_falign [QDSP6_FALIGN_INS] + n_falign [QDSP6_FALIGN_INC];

  if (qdsp6_falign_info && n_falign [QDSP6_FALIGN_TOTAL])
    {
      as_warn (_("%lu of %lu `.falign' (%lu%%) inserted new `nop' instructions."),
                n_faligned, n_falign [QDSP6_FALIGN_TOTAL],
                n_faligned * 100 / n_falign [QDSP6_FALIGN_TOTAL]);

      if (n_faligned)
        as_warn (_("%lu of %lu `.falign' (%lu%%) inserted new `nop' packets."),
                n_falign [QDSP6_FALIGN_INS], n_faligned,
                n_falign [QDSP6_FALIGN_INS] * 100 / n_faligned);
    }

  if (qdsp6_falign_more && n_falign [QDSP6_FALIGN_INS])
    {
      as_warn (_("reasons for \".falign\" inserting new `nop' packets:"));
      as_warn (_("  %lu of %lu (%lu%%) reached a packet \".falign\"."),
               n_falign [QDSP6_FALIGN_FALIGN], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_FALIGN] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) reached a single-instruction \".falign\"."),
               n_falign [QDSP6_FALIGN_FALIGN1], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_FALIGN1] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) in different sections."),
               n_falign [QDSP6_FALIGN_SECTION], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_SECTION] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) reached end of history."),
               n_falign [QDSP6_FALIGN_END], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_END] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) exhausted history."),
               n_falign [QDSP6_FALIGN_TOP], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_TOP] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) reached a label."),
               n_falign [QDSP6_FALIGN_LABEL], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_LABEL] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) reached a \".align\"."),
               n_falign [QDSP6_FALIGN_ALIGN], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_ALIGN] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) reached another `nop' packet."),
               n_falign [QDSP6_FALIGN_NOP], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_NOP] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) failed inserting new `nop' instruction."),
               n_falign [QDSP6_FALIGN_SHUF], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_SHUF] * 100 / n_falign [QDSP6_FALIGN_INS]);
    }

  memset (n_falign, 0, sizeof (n_falign));

  if (qdsp6_pairs_info && n_pairs [QDSP6_PAIRS_TOTAL])
    {
      as_warn (_("%lu instruction pairings."), n_pairs [QDSP6_PAIRS_TOTAL]);
    }

  memset (n_pairs, 0, sizeof (n_pairs));
}
