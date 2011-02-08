/* tc-hexagon.c -- Assembler for Hexagon
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

   Hexagon machine-specific port contributed by Qualcomm, Inc.
*/

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/param.h>
#include "as.h"
#include "dwarf2dbg.h"
#include "elf/hexagon.h"
#include "libiberty.h"
#include "libbfd.h"
#include "opcode/hexagon.h"
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

#endif
#define MAYBE (TRUE + 1)
#define TRUER(a, b) ((a) == TRUE \
                     ? (a) \
                     : (b) == TRUE \
                       ? (b) \
                       : (a) == MAYBE \
                         ? (a) \
                         : (b))

#define DEFAULT_CODE_ALIGNMENT (2) /* log2 (4) */
#define DEFAULT_CODE_FALIGN    (4) /* log2 (16) */
#define DEFAULT_DATA_ALIGNMENT (3) /* log2 (8) */

/* General limits. */
#define MAX_MESSAGE           (66) /* Message limit. */
#define MAX_DATA_ALIGNMENT    (16)
#define MAX_DCFETCH           (2) /* DCFETCH in a packet. */
#define MAX_PACKETS           (1)
#define MAX_INSNS             (6)
/* Per insn. */
#define MAX_FIXUPS            (1)

#define HEXAGON_NOP         "nop"
#define HEXAGON_NOP_LEN     (sizeof (HEXAGON_NOP) - 1)
#define HEXAGON_DCFETCH     "dcfetch"
#define HEXAGON_DCFETCH_LEN (sizeof (HEXAGON_DCFETCH) - 1)

/* Hexagon CR aliases. */
#define HEXAGON_SA0 0
#define HEXAGON_LC0 1
#define HEXAGON_SA1 2
#define HEXAGON_LC1 3
#define HEXAGON_P30 4
#define HEXAGON_M0  6
#define HEXAGON_M1  7
#define HEXAGON_SR  8
#define HEXAGON_PC  9
#define HEXAGON_GP 10

/* Hexagon GPR aliases. */
#define HEXAGON_SP 29
#define HEXAGON_FP 30
#define HEXAGON_LR 31

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
typedef struct _hexagon_literal
  {
    expressionS e;
    expressionS e1;
    segT        sec;
    int         sub;
    symbolS    *sym;
    struct _hexagon_literal
               *next;
    size_t      size;
    char        name [MAX_LITERAL_NAME];
    char        secname [MAX_LITERAL_SECTION];
  } hexagon_literal;

/** Packet instruction. */
typedef struct
  {
    hexagon_insn insn;
    size_t ndx;
    const hexagon_opcode *opcode;
    const hexagon_operand *ioperand;
    unsigned conditional, predicate;
    unsigned ireg, oreg;
    int opair;
    unsigned lineno;
    unsigned flags;
    char pad;
    char used;
    char is_inner, is_outer;
    hexagon_operand operand;
    expressionS exp;
    size_t fc;
    fixS *fix;
    unsigned relax;
    char *source;
  } hexagon_packet_insn;

/** Pair of instruction packets. */
typedef struct
  {
    struct
      {
        hexagon_packet_insn insn, prefix;
      } left, right;
  } hexagon_packet_pair;

/** Flags for packet instruction. */
enum _hexagon_insn_flag
  {
    HEXAGON_INSN_IS_KXED  = 0x01, /* K-extension prefixed insn. */
    HEXAGON_INSN_IS_R64   = 0x02, /* Insn operates on register pairs. */
    HEXAGON_INSN_IS_RELAX = 0x04, /* Insn will be relaxed. */
    HEXAGON_INSN_IN_RNEW  = 0x08, /* Insn has an operand of type R.NEW. */
    HEXAGON_INSN_OUT_RNEW = 0x10, /* Insn produces a result GPR. */
    HEXAGON_INSN_IS_PAIR  = 0x20, /* Insn is paired. */
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
    unsigned lineno; /* Line number at closing. */
    char faligned; /* Packet should be fetch-aligned. */
    char is_inner; /* Packet has :endloop0. */
    char is_outer; /* Packet has :endloop1. */
    hexagon_packet_insn insns [MAX_PACKET_INSNS]; /* Insns. */
    hexagon_packet_insn prefixes [MAX_PACKET_INSNS]; /* k-extender insns. */
    hexagon_packet_pair pairs [MAX_PACKET_INSNS]; /* Original paired prefix+insn. */
    int stats; /* Packet statistics. */
  } hexagon_packet;

/** Instruction queue for packet formation. */
typedef struct _hexagon_queue
  {
    size_t size;
    hexagon_packet_insn insns [MAX_INSNS]; /* Insns. */
    hexagon_packet_insn prefixes [MAX_INSNS]; /* k-extenders insns. */
    hexagon_packet_pair pairs [MAX_INSNS]; /* Original paired prefix+insn. */
    char faligned; /* Fetch-align request. */
    char is_inner; /* :endloop0 request. */
    char is_outer; /* :endloop1 request. */
  } hexagon_queue;

typedef struct hexagon_frag_data
  {
    hexagon_packet packet;
    fragS *previous;
  } hexagon_frag_data;

/** Packet events tracked for statistics. */
enum _hexagon_packet_stats
  {
    HEXAGON_STATS_FALIGN = 0x01, /* Caused adjustments due to .falign. */
    HEXAGON_STATS_PAD    = 0x02, /* Caused NOP padding inserted due to .falign. */
    HEXAGON_STATS_PACK   = 0x04, /* Caused NOP packet inserted due to .falign. */
  };

/** .falign counter types. */
enum _hexagon_falign_counters
  {
    HEXAGON_FALIGN_TOTAL,   /* # of .falign directives. */
    HEXAGON_FALIGN_NEED,    /* ... which needed adjustments. */
    HEXAGON_FALIGN_PAD,     /* ... which padded previous packets. */
    HEXAGON_FALIGN_PACK,    /* ... which inserted new NOP packets. */
    HEXAGON_FALIGN_FALIGN,  /* # of times that the history stopped due to a fetch-alignment. */
    HEXAGON_FALIGN_FALIGN1, /* ... due to a single-insn fetch-alignment. */
    HEXAGON_FALIGN_SECTION, /* ... due to a section change. */
    HEXAGON_FALIGN_END,     /* ... due to reaching the end of the history. */
    HEXAGON_FALIGN_TOP,     /* ... due to reaching the top of the history. */
    HEXAGON_FALIGN_LABEL,   /* ... due to a label. */
    HEXAGON_FALIGN_ALIGN,   /* ... due to an alignment performed. */
    HEXAGON_FALIGN_NOP,     /* ... due to a NOP packet inserted. */
    HEXAGON_FALIGN_SHUF,    /* ... due to failure inserting a NOP. */
    HEXAGON_FALIGN_COUNTERS
  };

/** Pairing counter types. */
enum _hexagon_pairs_counters
  {
    HEXAGON_PAIRS_TOTAL,   /* # of total pairings. */
    HEXAGON_PAIRS_UNDONE,  /* # of pairings undone. */
    HEXAGON_PAIRS_COUNTERS
  };

typedef enum _hexagon_suffix_type
  {
    SUF_NONE = 0,
    PIC_GOT,
    PIC_GOTREL,
    PIC_PLT,
    TLS_GD_GOT,
    TLS_IE,
    TLS_IE_GOT,
    TLS_DTPREL,
    TLS_TPREL,
  } hexagon_suffix_type;

extern int hexagon_get_mach (char *);
extern void hexagon_code_symbol (expressionS *);

int hexagon_assemble (hexagon_packet_insn *, hexagon_packet_insn *, char *str, int);
int hexagon_assemble_pair
  (hexagon_packet_pair *, hexagon_packet_insn *, hexagon_packet_insn *);
int hexagon_parse_name (char *, expressionS *, char *);
void hexagon_insert_operand (char *, const hexagon_operand *, offsetT, fixS*);
void hexagon_common (int);
void hexagon_option (int);
void hexagon_falign (int);
void hexagon_init (int);
void hexagon_frag_init (fragS *, fragS *);
int hexagon_is_nop (hexagon_insn);
int hexagon_is_prefix (hexagon_insn);
hexagon_insn hexagon_find_nop (void);
hexagon_insn hexagon_find_kext (void);
hexagon_insn hexagon_find_insn (const char *);
int hexagon_check_operand_args (const hexagon_operand_arg [], size_t);
int hexagon_check_new_predicate (void);
int hexagon_pair_open (void);
int hexagon_pair_close (void);
void hexagon_insn_init (hexagon_packet_insn *);
char *hexagon_insn_write
  (hexagon_insn, size_t, const hexagon_operand *, expressionS *,
   char *, size_t, fixS **, int);
void hexagon_packet_init (hexagon_packet *);
void hexagon_packet_begin (hexagon_packet *);
void hexagon_packet_end (hexagon_packet *);
void hexagon_packet_check (hexagon_packet *);
void hexagon_packet_unfold (hexagon_packet *);
void hexagon_packet_fold (hexagon_packet *);
void hexagon_packet_unpad (hexagon_packet *);
void hexagon_packet_write (hexagon_packet *);
void hexagon_packet_finish (hexagon_packet *);
int hexagon_packet_falign (size_t);
size_t hexagon_packet_slots (const hexagon_packet *);
size_t hexagon_packet_length (const hexagon_packet *);
size_t hexagon_packet_count (const hexagon_packet *);
size_t hexagon_packet_size (const hexagon_packet *);
size_t hexagon_packet_insns (const hexagon_packet *);
int hexagon_packet_insert
  (hexagon_packet *, const hexagon_packet_insn *, const hexagon_packet_insn *,
   const hexagon_packet_pair *, int);
int hexagon_packet_cram
  (hexagon_packet *, hexagon_packet_insn *, const hexagon_packet_insn *,
   const hexagon_packet_pair *, int);
size_t hexagon_insert_nops (size_t);
int hexagon_packet_form (hexagon_packet *, hexagon_queue *);
void hexagon_queue_init (hexagon_queue *);
int hexagon_queue_insert (hexagon_queue *, hexagon_packet_insn *, hexagon_packet_insn *);
int hexagon_prefix_kext (hexagon_packet_insn *, long);
char *hexagon_parse_immediate
  (hexagon_packet_insn *, hexagon_packet_insn *, const hexagon_operand *,
   char *, long *, char **);
static char *hexagon_parse_suffix (hexagon_suffix_type *, char **);
int hexagon_gp_const_lookup (char *str, char *);
segT hexagon_create_sbss_section (const char *, flagword, unsigned int);
segT hexagon_create_scom_section (const char *, flagword, unsigned int);
segT hexagon_create_literal_section (const char *, flagword, unsigned int);
hexagon_literal *hexagon_add_to_lit_pool (expressionS *, expressionS *, size_t);
void hexagon_shuffle_packet (hexagon_packet *, size_t *);
void hexagon_shuffle_handle (hexagon_packet *);
void hexagon_shuffle_prepare (hexagon_packet *);
void hexagon_shuffle_do (hexagon_packet *);
int hexagon_shuffle_helper (hexagon_packet *, size_t, size_t *);
int hexagon_discard_dcfetch (hexagon_packet *, int);
int hexagon_has_single (const hexagon_packet *);
int hexagon_has_prefix (const hexagon_packet *);
int hexagon_has_pair (const hexagon_packet *);
int hexagon_has_duplex (const hexagon_packet *);
int hexagon_has_duplex_hits
  (const hexagon_packet *, const hexagon_packet_insn *, size_t *);
int hexagon_has_duplex_clash
  (const hexagon_queue *, const hexagon_packet_insn *);
int hexagon_has_rnew (const hexagon_packet *, hexagon_packet_insn **);
int hexagon_has_mem (const hexagon_packet *);
int hexagon_has_store (const hexagon_packet *);
int hexagon_has_store_not (const hexagon_packet *);
int hexagon_has_but_ax (const hexagon_packet *);
int hexagon_has_solo (const hexagon_packet *);
int hexagon_is_nop_keep (const hexagon_packet *, int);
int hexagon_find_noslot1 (const hexagon_packet *, size_t);
void hexagon_check_register
  (hexagon_reg_score *, int, const char *, int,
   const hexagon_operand *, hexagon_packet_insn *, size_t);
void hexagon_check_predicate (int, const hexagon_opcode *);
void hexagon_check_implicit
  (const hexagon_opcode *, unsigned int, int, hexagon_reg_score *, const char *);
void hexagon_check_implicit_predicate (const hexagon_opcode *, unsigned int, int);
int hexagon_relax_branch (fragS *);
int hexagon_relax_falign (fragS *);
long hexagon_relax_branch_try (fragS *, segT, long);
long hexagon_relax_falign_try (fragS *, segT, long);
long hexagon_relax_frag (segT, fragS *, long);
void hexagon_statistics (void);
addressT hexagon_frag_fix_addr (void);
bfd_reloc_code_real_type hexagon_got_frag (fragS *, int, int, expressionS *);

static segT hexagon_sdata_section, hexagon_sbss_section;
static asection hexagon_scom_section;
static asymbol  hexagon_scom_symbol;

static hexagon_literal *hexagon_pool;
static unsigned hexagon_pool_counter;

/* Special insns created by GAS. */
static hexagon_packet_insn hexagon_nop_insn, hexagon_kext_insn;

static symbolS *hexagon_got_symbol;

const pseudo_typeS md_pseudo_table [] =
{
  { "align", s_align_bytes, 0 }, /* Defaulting is invalid (0).  */
  { "comm", hexagon_common, 0 },
  { "common", hexagon_common, 0 },
  { "lcomm", hexagon_common, 1 },
  { "lcommon", hexagon_common, 1 },
  { "2byte", cons, 2 },
  { "half", cons, 2 },
  { "short", cons, 2 },
  { "3byte", cons, 3 },
  { "4byte", cons, 4 },
  { "word", cons, 4 },
  { "option", hexagon_option, 0 },
  { "cpu", hexagon_option, 0 },
  { "block", s_space, 0 },
  { "file", (void (*) PARAMS ((int))) dwarf2_directive_file, 0 },
  { "loc", dwarf2_directive_loc, 0 },
  { "falign", hexagon_falign, 0 },
  { NULL, 0, 0 },
};

/* Valid characters that make up a symbol. */
const char hexagon_symbol_chars [] = "";

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
const char hexagon_parallel_separator_chars [] = "{}";

/* Chars that can be used to separate mant from exp in floating point nums.  */
const char EXP_CHARS [] = "eE";

/* Chars that mean this number is a floating point constant
   As in 0f12.456 or 0d1.2345e12.  */
const char FLT_CHARS [] = "rRsSfFdD";

/* Byte order.  */
extern int target_big_endian;
const char *hexagon_target_format = DEFAULT_TARGET_FORMAT;
static int byte_order = DEFAULT_BYTE_ORDER;

static int hexagon_mach_type = bfd_mach_hexagon_v2;
static int cmdline_set_hexagon_mach_type = 0;

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
#define OPTION_HEX_PAIR_INFO (OPTION_MD_BASE + 2)
    { "mpairing-info", no_argument, NULL, OPTION_HEX_PAIR_INFO },
#define OPTION_HEX_FALIGN_INFO_NEW (OPTION_MD_BASE + 3)
    { "mfalign-info", no_argument, NULL, OPTION_HEX_FALIGN_INFO_NEW },
#define OPTION_HEX_FALIGN_MORE_INFO (OPTION_MD_BASE + 4)
    { "mfalign-more-info", no_argument, NULL, OPTION_HEX_FALIGN_MORE_INFO },
#define OPTION_HEX_NO_2MEMORY (OPTION_MD_BASE + 5)
    { "mno-dual-memory", no_argument, NULL, OPTION_HEX_NO_2MEMORY },
/* Code in md_parse_option () assumes that the -mv* options, are sequential. */
#define OPTION_HEX_MV2 (OPTION_MD_BASE + 6)
    { "mv2", no_argument, NULL, OPTION_HEX_MV2 },
#define OPTION_HEX_MV3 (OPTION_MD_BASE + 7)
    { "mv3", no_argument, NULL, OPTION_HEX_MV3 },
#define OPTION_HEX_MV4 (OPTION_MD_BASE + 8)
    { "mv4", no_argument, NULL, OPTION_HEX_MV4 },
#define OPTION_HEX_MARCH (OPTION_MD_BASE + 9)
    { "march", required_argument, NULL, OPTION_HEX_MARCH },
#define OPTION_HEX_MCPU (OPTION_MD_BASE + 10)
    { "mcpu", required_argument, NULL, OPTION_HEX_MCPU },
#define OPTION_HEX_MSORT_SDA (OPTION_MD_BASE + 11)
    { "msort-sda", no_argument, NULL, OPTION_HEX_MSORT_SDA },
#define OPTION_HEX_MNO_SORT_SDA (OPTION_MD_BASE + 12)
    { "mno-sort-sda", no_argument, NULL, OPTION_HEX_MNO_SORT_SDA },
#define OPTION_HEX_MNO_EXTENDER (OPTION_MD_BASE + 13)
    { "mno-extender", no_argument, NULL, OPTION_HEX_MNO_EXTENDER },
#define OPTION_HEX_MNO_PAIRING (OPTION_MD_BASE + 14)
    { "mno-pairing", no_argument, NULL, OPTION_HEX_MNO_PAIRING },
#define OPTION_HEX_MNO_PAIRING_B (OPTION_MD_BASE + 15)
    { "mno-pairing-branch", no_argument, NULL, OPTION_HEX_MNO_PAIRING_B },
#define OPTION_HEX_MNO_PAIRING_2 (OPTION_MD_BASE + 16)
    { "mno-pairing-duplex", no_argument, NULL, OPTION_HEX_MNO_PAIRING_2 },
#define OPTION_HEX_MNO_JUMPS (OPTION_MD_BASE + 17)
    { "mno-jumps", no_argument, NULL, OPTION_HEX_MNO_JUMPS },
#define OPTION_HEX_MNO_JUMPS_LONG (OPTION_MD_BASE + 18)
    { "mno-jumps-long", no_argument, NULL, OPTION_HEX_MNO_JUMPS_LONG },
#define OPTION_HEX_MNO_FALIGN (OPTION_MD_BASE + 19)
    { "mno-falign", no_argument, NULL, OPTION_HEX_MNO_FALIGN },
  };
size_t md_longopts_size = sizeof (md_longopts);

#define IS_SYMBOL_OPERAND(o) \
 ((o) == 'b' || (o) == 'c' || (o) == 's' || (o) == 'o' || (o) == 'O')

/* Relax states. */
typedef enum _hexagon_relax_state
  {
    /* Matching the respective entries in hexagon_relax_table. */
    HEXAGON_RELAX_NONE = 0,
    /* Relax state for R_HEX_B7_PCREL. */
    HEXAGON_RELAX_B7, HEXAGON_RELAX_B7_A,
    /* Relax state for R_HEX_B9_PCREL. */
    HEXAGON_RELAX_B9, HEXAGON_RELAX_B9_A,
    /* Relax state for R_HEX_B13_PCREL. */
    HEXAGON_RELAX_B13, HEXAGON_RELAX_B13_A,
    /* Relax state for R_HEX_B15_PCREL. */
    HEXAGON_RELAX_B15, HEXAGON_RELAX_B15_A,
    /* Other relax state pairs go here. */
    /* Done relaxing. */
    HEXAGON_RELAX_DONE
  } hexagon_relax_state;

/* Encode relax state from relocation type. */
#define ENCODE_RELAX(R) \
  ({\
     hexagon_relax_state r;\
     \
     if (hexagon_relax) \
       { \
         if ((R) == BFD_RELOC_HEX_B7_PCREL && hexagon_relax_long) \
           r = HEXAGON_RELAX_B7; \
         else if ((R) == BFD_RELOC_HEX_B9_PCREL) \
           r = HEXAGON_RELAX_B9; \
         else if ((R) == BFD_RELOC_HEX_B13_PCREL && hexagon_relax_long) \
           r = HEXAGON_RELAX_B13; \
         else if ((R) == BFD_RELOC_HEX_B15_PCREL && hexagon_relax_long) \
           r = HEXAGON_RELAX_B15; \
         else \
           r = HEXAGON_RELAX_NONE; \
       } \
     else \
       r = HEXAGON_RELAX_NONE; \
     r;\
  })

#define HEXAGON_RANGE(B) (~(~0L << ((B) - 1)) \
                        & -(2 * MAX_PACKET_INSNS * HEXAGON_INSN_LEN))

/* State table for relaxing branches.
   Note that since an extender is used, the insn is moved up,
   so the limits are offset by that. */
const struct relax_type hexagon_relax_table [] =
  {
    /* Dummy entry. */
    {              0L,                0L,
                    0,  HEXAGON_RELAX_NONE},
    /* Entries for R_HEX_B7_PCREL. */
    {HEXAGON_RANGE  (9),  -HEXAGON_RANGE (9),
                    0, HEXAGON_RELAX_B7_A},
    {              0L,                0L,
       HEXAGON_INSN_LEN,  HEXAGON_RELAX_DONE},
    /* Entries for R_HEX_B9_PCREL. */
    {HEXAGON_RANGE (11), -HEXAGON_RANGE (11),
                    0, HEXAGON_RELAX_B9_A},
    {              0L,                0L,
       HEXAGON_INSN_LEN,  HEXAGON_RELAX_DONE},
    /* Entries for R_HEX_B13_PCREL. */
    {HEXAGON_RANGE (15),  -HEXAGON_RANGE (15),
                    0, HEXAGON_RELAX_B13_A},
    {              0L,                0L,
       HEXAGON_INSN_LEN,  HEXAGON_RELAX_DONE},
    /* Entries for R_HEX_B15_PCREL. */
    {HEXAGON_RANGE (17),  -HEXAGON_RANGE (17),
                    0, HEXAGON_RELAX_B15_A},
    {              0L,                0L,
       HEXAGON_INSN_LEN,  HEXAGON_RELAX_DONE},
    /* Pair of entries for other relocations go here. */
    /* Final entry. */
    {              0L,                0L,
                    0,  HEXAGON_RELAX_NONE},
  };

static int hexagon_autoand = TRUE;
static int hexagon_extender = TRUE;
static int hexagon_pairing  = TRUE,
           hexagon_pairing_branch = TRUE,
           hexagon_pairing_duplex = TRUE;
static int hexagon_relax = TRUE,
           hexagon_relax_long = TRUE;
static int hexagon_sort_sda = TRUE;
static int hexagon_fetch_align = TRUE;

static int hexagon_falign_info; /* Report statistics about .falign usage. */
static int hexagon_falign_more; /* Report more statistics about .falign. */
static int hexagon_pairs_info;   /* Report statistics about pairings. */

static unsigned hexagon_gp_size = HEXAGON_SMALL_GPSIZE;
static int hexagon_no_dual_memory = FALSE;

static int hexagon_in_packet;

hexagon_packet hexagon_apacket; /* Includes current packet. */
hexagon_queue hexagon_aqueue;

static int hexagon_faligning; /* 1 => .falign next packet we see */
static int hexagon_falign_info; /* 1 => report statistics about .falign usage */
static int hexagon_falign_more; /* report more statistics about .falign. */

static unsigned n_falign [HEXAGON_FALIGN_COUNTERS]; /* .falign statistics. */
static unsigned n_pairs  [HEXAGON_PAIRS_COUNTERS];   /* Pairing statistics. */

/* Score-boards to check register dependency conlficts. */
static hexagon_reg_score gArray [HEXAGON_NUM_GENERAL_PURPOSE_REGS],
                       cArray [HEXAGON_NUM_CONTROL_REGS],
                       sArray [HEXAGON_NUM_SYS_CTRL_REGS],
                       pArray [HEXAGON_NUM_PREDICATE_REGS],
                       guArray[HEXAGON_NUM_GUEST_REGS],
                       pNewArray [HEXAGON_NUM_PREDICATE_REGS],
                       pLateArray [HEXAGON_NUM_PREDICATE_REGS];

static int numOfOvf; /* SR:OVR */
static int numOfBranchAddr, numOfBranchAddrMax1, numOfBranchAddrRelax;
static int numOfBranchRelax, numOfBranchRelax2nd;
static int numOfBranchMax1, numOfBranchStiff;
static int numOfLoopMax1;

struct hexagon_march
  {
	char *march_name_fe, *march_alt_fe, *march_short_fe;
	unsigned int march_name_be;
  };

static struct hexagon_march hexagon_marchs [] =
  {
    {"hexagonv2", "qdsp6v2", "v2", bfd_mach_hexagon_v2},
    {"hexagonv3", "qdsp6v3", "v3", bfd_mach_hexagon_v3},
    {"hexagonv4", "qdsp6v4", "v4", bfd_mach_hexagon_v4},
  };

static size_t hexagon_marchs_size =
  sizeof (hexagon_marchs) / sizeof (*hexagon_marchs);

/* Invocation line includes a switch not recognized by the base assembler.
   See if it's a processor-specific option.  */
int
md_parse_option
(int c, char *arg)
{
  static int post_stats;
  unsigned int i;
  int temp_hexagon_mach_type = 0;

  switch (c)
    {
    case OPTION_EB:
      byte_order = BIG_ENDIAN;
      hexagon_target_format = "elf32-bighexagon";
      break;

    case OPTION_EL:
      byte_order = LITTLE_ENDIAN;
      hexagon_target_format = "elf32-littlehexagon";
      break;

    case OPTION_HEX_PAIR_INFO:
      hexagon_pairs_info = TRUE;
      if (!post_stats)
        {
          post_stats = TRUE;
          xatexit (hexagon_statistics);
        }
      break;

    case OPTION_HEX_FALIGN_MORE_INFO:
      hexagon_falign_more = TRUE;
      /* Fall through. */

    case OPTION_HEX_FALIGN_INFO_NEW:
      hexagon_falign_info = TRUE;
      if (!post_stats)
        {
          post_stats = TRUE;
          xatexit (hexagon_statistics);
        }
      break;

    case OPTION_HEX_MV2:
    case OPTION_HEX_MV3:
    case OPTION_HEX_MV4:
    case OPTION_HEX_MARCH:
    case OPTION_HEX_MCPU:
      switch (c)
        {
          case OPTION_HEX_MV2:
          case OPTION_HEX_MV3:
          case OPTION_HEX_MV4:
            /* -mv* options. */
            temp_hexagon_mach_type
              = hexagon_marchs [c - OPTION_HEX_MV2].march_name_be;
            break;

          default:
            /* -march and- mcpu options. */
            for (i = 0; i < hexagon_marchs_size; i++)
              if (!strcmp (arg, hexagon_marchs [i].march_name_fe)
                  || !strcmp (arg, hexagon_marchs [i].march_alt_fe)
                  || !strcmp (arg, hexagon_marchs [i].march_short_fe))
                {
                  temp_hexagon_mach_type = hexagon_marchs [i].march_name_be;
                  break;
                }

            if (i == hexagon_marchs_size)
              as_fatal (_("invalid architecture specified."));
            break;
        }

      if (cmdline_set_hexagon_mach_type)
        {
          if (hexagon_mach_type != temp_hexagon_mach_type)
            as_fatal (_("conflicting architectures specified."));
        }

      hexagon_mach_type = temp_hexagon_mach_type;
      cmdline_set_hexagon_mach_type = TRUE;
      break;

    case 'G':
      {
        int x;

        x = atoi (arg);

        if (x < 0)
          as_warn (_("invalid value for \"-%c\"; defaulting to %d."),
                   c, x = HEXAGON_SMALL_GPSIZE);
        else
          hexagon_gp_size = x;
      }
      break;

    case OPTION_HEX_MSORT_SDA:
      hexagon_sort_sda = TRUE;
      break;

    case OPTION_HEX_MNO_SORT_SDA:
      hexagon_sort_sda = FALSE;
      break;

    case OPTION_HEX_NO_2MEMORY:
      hexagon_no_dual_memory = TRUE;
      break;

    case OPTION_HEX_MNO_EXTENDER:
      hexagon_extender = FALSE;
      break;

    case OPTION_HEX_MNO_PAIRING:
      hexagon_pairing = hexagon_pairing_branch = hexagon_pairing_duplex = FALSE;
      break;

    case OPTION_HEX_MNO_PAIRING_B:
      hexagon_pairing_branch = FALSE;
      break;

    case OPTION_HEX_MNO_PAIRING_2:
      hexagon_pairing_duplex = FALSE;
      break;

    case OPTION_HEX_MNO_JUMPS:
      hexagon_relax = FALSE;
      break;

    case OPTION_HEX_MNO_JUMPS_LONG:
      hexagon_relax_long = FALSE;
      break;

    case OPTION_HEX_MNO_FALIGN:
      hexagon_fetch_align = FALSE;
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
  fprintf (stream,
"\
Hexagon Options:\n\
  -EB                     select big-endian output\n\
  -EL                     select little-endian ouptut (default)\n\
  -G SIZE                 small-data size limit (default is \"%d\")\n\
  -march={v2|v3|v4}       assemble for the specified Hexagon architecture\n\
                          (default is \"v2\")\n\
  -mcpu={v2|v3|v4}        equivalent to \"-march\"\n\
  -m{v2|v3|v4}            equivalent to \"-march\"\n\
  -mfalign-info           report \".falign\" statistics\n\
  -mno-extender           disable the use of constant extenders\n\
  -mno-jumps              disable automatic extension of branch instructions\n\
  -mno-jumps-long         disable automatic extension of non-paired\n\
                          branch instructions\n\
  -mno-pairing            disable pairing of instructions\n\
  -mno-pairing-branch     disable pairing of direct branch instructions\n\
  -mno-pairing-duplex     disable pairing to duplex instructions\n\
  -mpairing-info          report instruction pairing statistics\n\
  -msort-sda              enable sorting the small-data area (default)\n\
",
           HEXAGON_SMALL_GPSIZE);
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

  if (!bfd_set_arch_mach (stdoutput, bfd_arch_hexagon, hexagon_mach_type))
    as_warn (_("architecture and machine types not set; using default settings."));

  /* This call is necessary because we need to initialize `hexagon_operand_map'
     which may be needed before we see the first insn.  */
  hexagon_opcode_init_tables
    (hexagon_get_opcode_mach (hexagon_mach_type, target_big_endian));

  /* Initialize global NOP. */
  hexagon_find_nop ();
  /* Initialize global k-extension. */
  hexagon_find_kext ();

  /* Set the default alignment for the default sections. */
  record_alignment (text_section, DEFAULT_CODE_ALIGNMENT);

  if (!hexagon_if_arch_v1 ())
    {
      /* Hexagon V2 */
      applicable = bfd_applicable_section_flags (stdoutput);

      /* Create the sdata section. */
      hexagon_sdata_section = subseg_new (SMALL_DATA_SECTION, 0);
      bfd_set_section_flags
        (stdoutput, hexagon_sdata_section,
         applicable & (  SEC_ALLOC | SEC_LOAD | SEC_RELOC
                       | SEC_DATA | SEC_SMALL_DATA));
      record_alignment (hexagon_sdata_section, DEFAULT_DATA_ALIGNMENT);
      symbol_table_insert (section_symbol (hexagon_sdata_section));

      /* Create the sbss section. */
      hexagon_sbss_section = subseg_new (SMALL_BSS_SECTION, 0);
      bfd_set_section_flags
        (stdoutput, hexagon_sbss_section,
         applicable & (SEC_ALLOC | SEC_SMALL_DATA));
      seg_info (hexagon_sbss_section)->bss = TRUE;
      record_alignment (hexagon_sbss_section, DEFAULT_DATA_ALIGNMENT);
      symbol_table_insert (section_symbol (hexagon_sbss_section));

      /* We must construct a fake section similar to bfd_com_section,
        but with the name .scommon.  */
      hexagon_scom_section                = bfd_com_section;
      hexagon_scom_section.name           = SMALL_COM_SECTION;
      hexagon_scom_section.flags         |= SEC_SMALL_DATA;
      hexagon_scom_section.output_section = &hexagon_scom_section;
      hexagon_scom_section.symbol         = &hexagon_scom_symbol;
      hexagon_scom_section.symbol_ptr_ptr = &hexagon_scom_section.symbol;

      hexagon_scom_symbol                 = *bfd_com_section.symbol;
      hexagon_scom_symbol.name            = SMALL_COM_SECTION;
      hexagon_scom_symbol.section         = &hexagon_scom_section;
    }
  else
    /* Hexagon V1 */
    hexagon_gp_size = 0;

  /* Set the GP size. */
  bfd_set_gp_size (stdoutput, hexagon_gp_size);

  /* Reset default section. */
  subseg_set (current_section, current_subsec);
}

/** Relax branch by adding the extender.

@param fragP Relaxed fragment with room for the extension.
*/

int
hexagon_relax_branch
(fragS *fragP)
{
  hexagon_packet *apacket;
  hexagon_operand *operand;
  fixS *fx, fxup;
  int fix;
  size_t i, j;

  fix = fragP->fr_fix;

  apacket = &fragP->tc_frag_data->packet;
  if (!apacket->relax || !apacket->drlx)
    return FALSE;

  /* Sanity check. */
  assert (apacket->drlx + hexagon_packet_length (apacket) - apacket->relax
          <= MAX_PACKET_INSNS);

  for (i = 0, fx = NULL; i < apacket->size; i++)
    if (apacket->insns [i].flags & HEXAGON_INSN_IS_RELAX
        && apacket->insns [i].fc)
      {
        if (apacket->insns [i].relax == HEXAGON_RELAX_NONE
            || apacket->insns [i].relax == HEXAGON_RELAX_DONE
            || apacket->insns [i].relax % 2)
          continue;

        fx = apacket->insns [i].fix;
        fxup = *fx;

        operand = fx->tc_fix_data;

        /* Adjusted extended insn. */
        operand->flags |= HEXAGON_OPERAND_IS_KXED;
        apacket->insns [i].flags |= HEXAGON_INSN_IS_KXED;
        fx->fx_r_type = operand->reloc_kxed;

        /* Make room for extender. */
        for (j = apacket->size; j > i; j--)
          {
            hexagon_insn *pi;

            pi = ((hexagon_insn *) fragP->fr_literal) + j;
            pi [0] = pi [-1];

            apacket->insns [j] = apacket->insns [j - 1];
            if (apacket->insns [j].fc && apacket->insns [j].fix)
              {
                apacket->insns [j].fix->fx_where += HEXAGON_INSN_LEN;
                apacket->insns [j].fix->fx_offset
                  += apacket->insns [j].fix->fx_pcrel? HEXAGON_INSN_LEN: 0;
              }
          }

        /* Create extender. */
        apacket->insns [i] = hexagon_kext_insn;
        apacket->insns [i].operand
          = *(hexagon_operand *) hexagon_lookup_reloc (operand->reloc_kxer, 0,
                                                   apacket->insns [i].opcode);
        apacket->insns [i].fc++;
        apacket->insns [i].fix
          = fix_new (fragP, fxup.fx_where, HEXAGON_INSN_LEN, fxup.fx_addsy,
                     fxup.fx_offset, fxup.fx_pcrel, operand->reloc_kxer);
        apacket->insns [i].fix->fx_line = fxup.fx_line;
        apacket->insns [i].fix->tc_fix_data = &apacket->insns [i].operand;

        /* Add extender. */
        md_number_to_chars (fragP->fr_literal + i * HEXAGON_INSN_LEN,
                            apacket->insns [i].insn, HEXAGON_INSN_LEN);

        apacket->relax--;
        apacket->drlx--;
        apacket->size++;
        fragP->fr_fix += HEXAGON_INSN_LEN;

        /* Skip extended branch. */
        i++;
        apacket->insns [i].relax = HEXAGON_RELAX_DONE;
      }

  apacket->ddrlx = 0;

  return (fragP->fr_fix - fix);
}

/** Relax branch by extending it and adjusting it to accomodate the extension.

@param fragP Relaxable fragment with room for the extension.
*/

long
hexagon_relax_branch_try
(fragS *fragP, segT segment, long stretch)
{
  const relax_typeS *before, *now;
  relax_substateT this = HEXAGON_RELAX_NONE, next = HEXAGON_RELAX_NONE;
  hexagon_packet *apacket;
  symbolS *sym;
  addressT from, to;
  offsetT aim;
  long delta, growth;
  size_t i;

  apacket = &fragP->tc_frag_data->packet;

  if (!apacket->relax)
    return FALSE;

  for (i = 0, delta = 0; i < apacket->size; i++)
    if (apacket->insns [i].flags & HEXAGON_INSN_IS_RELAX
        && apacket->insns [i].fc)
      {
        if (apacket->insns [i].relax == HEXAGON_RELAX_DONE)
          continue;

        sym = apacket->insns [i].fix->fx_addsy;
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

        before = now = hexagon_relax_table + apacket->insns [i].relax;

        from = fragP->fr_address;
        aim  = to - from;
        for (next = now->rlx_more;
             next != HEXAGON_RELAX_NONE && next != HEXAGON_RELAX_DONE; )
          if ((aim <  0 && aim >= now->rlx_backward)
              || (aim >= 0 && aim <= now->rlx_forward))
            next = HEXAGON_RELAX_NONE;
          else
            {
              /* Grow to next state.  */
              this = next;
              now = hexagon_relax_table + this;
              next = now->rlx_more;
            }

        growth = now->rlx_length - before->rlx_length;
        if (growth)
          {
            apacket->insns [i].relax = this;

            apacket->drlx += now->rlx_length / HEXAGON_INSN_LEN;

            /* Replace any padding for the extension. */
            if (apacket->dpad)
              {
                apacket->dpad -= now->rlx_length / HEXAGON_INSN_LEN;
                growth        -= now->rlx_length;
              }
            else if (apacket->ddpad)
              apacket->ddpad -= now->rlx_length / HEXAGON_INSN_LEN;
          }
        delta += growth;
      }

  return (delta);
}

/** Process the resulting padding delta.
*/
int
hexagon_relax_falign
(fragS *fragP)
{
  hexagon_packet *apacket, *fpacket, packet;
  fragS *previous;
  size_t after, pad, pkt, i;

  apacket = &fragP->tc_frag_data->packet;
  if (!apacket->dpad && !apacket->dpkt)
    return FALSE;

  /* Sanity checks. */
  assert (apacket->dpad + hexagon_packet_length (apacket) - apacket->relax
          <= MAX_PACKET_INSNS);
  assert (apacket->dpkt <= MAX_PACKET_INSNS);

  packet = *apacket;
  packet.relax = FALSE; /* At this point, it won't be extended. */
  packet.dpad = pad = apacket->dpad;
  packet.dpkt = pkt = apacket->dpkt;

  if (packet.dpad)
    {
      hexagon_packet_unfold (&packet);

      /* Pad the packet. */
      while (packet.dpad)
        {
          if (!(hexagon_packet_insert (&packet, &hexagon_nop_insn, NULL, NULL, FALSE)))
            packet.dpkt++;
          packet.dpad--;
        }

      /* Try to shuffle modified packet. */
      hexagon_shuffle_prepare (&packet);
      if (hexagon_shuffle_helper (&packet, 0, NULL))
        {
          /* Padding resulted in a well-formed packet. */
          hexagon_packet_fold (&packet);
          hexagon_packet_unpad (&packet);
          after = packet.size;

          /* Re-emit packet. */
          for (i = 0; i < packet.size; i++, after = i)
            {
              packet.insns [i].insn = HEXAGON_END_PACKET_RESET (packet.insns [i].insn);

              if (i == 0 && packet.is_inner)
                packet.insns [i].insn =
                  HEXAGON_END_PACKET_SET (packet.insns [i].insn, HEXAGON_END_LOOP);
              else if (i == 1 && packet.is_outer)
                packet.insns [i].insn =
                  HEXAGON_END_PACKET_SET (packet.insns [i].insn, HEXAGON_END_LOOP);
              else if (i >= packet.size - 1
                      && HEXAGON_END_PACKET_GET (packet.insns [i].insn) != HEXAGON_END_PAIR)
                packet.insns [i].insn =
                  HEXAGON_END_PACKET_SET (packet.insns [i].insn, HEXAGON_END_PACKET);

              md_number_to_chars ((void *)(((hexagon_insn *) fragP->fr_literal) + i),
                                  packet.insns [i].insn, HEXAGON_INSN_LEN);
            }
        }
      else
        {
          /* In order to avoid a malformed packet, convert the padding into a
             padding packet. */
          hexagon_packet_fold (&packet);
          after = apacket->size;

          packet.dpkt = pkt = apacket->dpkt + apacket->dpad;
        }
    }
  else
    after = apacket->size;

  /* Emit padding packet. */
  while (packet.dpkt)
    {
      if (packet.dpkt % MAX_PACKET_INSNS != 1)
        md_number_to_chars ((void *)(((hexagon_insn *) fragP->fr_literal) + after++),
                            hexagon_nop_insn.insn, HEXAGON_INSN_LEN);
      else
        md_number_to_chars ((void *)(((hexagon_insn *) fragP->fr_literal) + after++),
                            HEXAGON_END_PACKET_SET (hexagon_nop_insn.insn, HEXAGON_END_PACKET),
                            HEXAGON_INSN_LEN);
      packet.dpkt--;
    }

  fragP->fr_fix += (apacket->dpad + apacket->dpkt) * HEXAGON_INSN_LEN;

  apacket->dpad = apacket->ddpad = 0;
  apacket->dpkt = apacket->ddpkt = 0;

  /* Collect .falign stats. */
    {
      /* Find packet requesting .falign. */
      for (previous = fragP, fpacket = NULL;
          previous
          && previous->tc_frag_data
          && (fpacket = &previous->tc_frag_data->packet)
          && !fpacket->faligned;
          previous = previous->tc_frag_data->previous)
        ;

      if (fpacket)
        {
        if ((pad || pkt)
            && !(fpacket->stats & HEXAGON_STATS_FALIGN)
            && (fpacket->stats |= HEXAGON_STATS_FALIGN))
            n_falign [HEXAGON_FALIGN_NEED]++;

        if (pad
            && !(fpacket->stats & HEXAGON_STATS_PAD)
            && (fpacket->stats |= HEXAGON_STATS_PAD))
            n_falign [HEXAGON_FALIGN_PAD]++;

        if (pkt
            && !(fpacket->stats & HEXAGON_STATS_PACK)
            && (fpacket->stats |= HEXAGON_STATS_PACK))
            n_falign [HEXAGON_FALIGN_PACK]++;
        }
    }

  return TRUE;
}

/** Determine if packet is not fetch-aligned and then request previous packets to
grow through padding NOPs, if possible, or through inserting a NOP-packet.
*/
long
hexagon_relax_falign_try
(fragS *fragP, segT segment, long stretch ATTRIBUTE_UNUSED)
{
  static segT saligning;
  static fragS *faligning;
  fragS *previous;
  hexagon_packet *apacket, *bpacket, *zpacket;
  addressT first, next;
  size_t size, delta, over, left, room;

  if (saligning && saligning == segment
      && faligning && faligning != fragP)
    return (0);
  else
    {
      saligning = NULL;
      faligning = NULL;
    }

  apacket = &fragP->tc_frag_data->packet;
  bpacket = fragP->tc_frag_data->previous
            ? &fragP->tc_frag_data->previous->tc_frag_data->packet: NULL;

  delta = 0;

  size  = apacket->size
          + apacket->dpad + apacket->ddpad
          + apacket->drlx + apacket->ddrlx;
  first = (fragP->fr_address / HEXAGON_INSN_LEN) % MAX_PACKET_INSNS;
  next  = first + size;

  /* Check fetch-alignment. */
  over = next > MAX_PACKET_INSNS? next % MAX_PACKET_INSNS: 0;
  if (over)
    {
      saligning = segment;
      faligning = fragP;

      /* Check if still not fetch-aligned. */
      if (over)
        {
          for (previous = fragP->tc_frag_data->previous,
               left = MAX_PACKET_INSNS - first;
               previous && left;
               previous = previous->tc_frag_data
                          ? previous->tc_frag_data->previous: NULL)
            {
              zpacket = &previous->tc_frag_data->packet;

              if (previous->fr_type == rs_fill) /* TODO: rs_space too? */
                /* Skip standard frags. */
                continue;
              else if (previous->fr_type != rs_machine_dependent)
                /* Other frags must not be trespassed. */
                break;

              if (left)
                {
                  size = hexagon_packet_size (zpacket) - zpacket->relax
                         + zpacket->dpad + zpacket->ddpad
                         + zpacket->drlx + zpacket->ddrlx;
                  assert (size <= MAX_PACKET_INSNS);

                  if (zpacket->faligned)
                    /* The room in a fetch-aligned packet must be
                      within its fetch window. */
                    next = (previous->fr_address / HEXAGON_INSN_LEN + size)
                            % MAX_PACKET_INSNS;
                  else
                    next = 0;

                  room = MAX_PACKET_INSNS - MAX (size, next);
                  if (room && !hexagon_has_solo (zpacket))
                    {
                      zpacket->ddpad += MIN (left, room);
                      left           -= MIN (left, room);
                    }

                  size = hexagon_packet_size (zpacket) - zpacket->relax
                         + zpacket->dpad + zpacket->ddpad
                         + zpacket->drlx + zpacket->ddrlx;
                  assert (size <= MAX_PACKET_INSNS);
                }

              if (zpacket->faligned)
                /* Cannot go past a fetch-aligned packet. */
                break;
            }

          if (left)
            {
              if (bpacket)
                /* Force fetch-alignment by inserting a NOP-packet. */
                bpacket->ddpkt += ((bpacket->dpkt + left) % MAX_PACKET_INSNS)
                                  - bpacket->dpkt;
              else
                {
                  /* Without a previous packet, this packet cannot be
                     fetch-aligned. */
                  saligning = NULL;
                  faligning = NULL;
                }
            }
        }
    }

  return (HEXAGON_INSN_LEN * (int) delta);
}

/** Extend some branches when otherwise the destination is out of reach.
*/

int
hexagon_estimate_size_before_relax
(fragS *fragP, segT segment)
{
  hexagon_packet *apacket;
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

  delta = HEXAGON_INSN_LEN * (apacket->drlx + apacket->dpad + apacket->dpkt);

  if (apacket->relax)
    {
      symbolS *sym;
      size_t i;

      for (i = 0, sym = NULL; i < apacket->size; i++)
        if (apacket->insns [i].flags & HEXAGON_INSN_IS_RELAX
            && apacket->insns [i].fc)
          {
            if (apacket->insns [i].relax == HEXAGON_RELAX_DONE)
              continue;

            sym = apacket->insns [i].fix->fx_addsy;

            if (apacket->insns [i].relax == HEXAGON_RELAX_NONE)
              {
                apacket->insns [i].relax
                  = ENCODE_RELAX (apacket->insns [i].operand.reloc_type);
                if (apacket->insns [i].relax == HEXAGON_RELAX_NONE)
                  continue;

                if (S_GET_SEGMENT (sym) != segment
                    || (OUTPUT_FLAVOR == bfd_target_elf_flavour
                        && ((S_IS_EXTERNAL (sym) && !S_IS_DEFINED (sym))
                            || S_IS_WEAK (sym))))
                  {
                    /* Symbol is external, in another segment, or weak, then
                       an extender should be added. */
                    apacket->drlx++;
                    apacket->insns [i].relax
                      = hexagon_relax_table [apacket->insns [i].relax].rlx_more;
                  }
                delta += hexagon_relax_table [apacket->insns [i].relax].rlx_length;
              }
        }
    }

  return (delta);
}

long
hexagon_relax_frag (segT segment, fragS *fragP, long stretch)
{
  hexagon_packet *apacket;
  size_t ddpad, ddpkt;
  long delta;

  apacket = &fragP->tc_frag_data->packet;

  ddpad = apacket->dpad + apacket->ddpad <= SSIZE_MAX
          ? apacket->ddpad: -apacket->dpad;
  ddpkt = apacket->dpkt + apacket->ddpkt <= SSIZE_MAX
          ? apacket->ddpkt: -apacket->dpkt;

  apacket->dpad += ddpad;
  apacket->dpkt += ddpkt;

  apacket->ddpad = apacket->ddpkt = 0;

  delta = HEXAGON_INSN_LEN * (ddpad + ddpkt);

  if (apacket->relax)
    delta += hexagon_relax_branch_try (fragP, segment, stretch);
  if (apacket->faligned)
    delta += hexagon_relax_falign_try (fragP, segment, stretch);

  return (delta);
}

void
hexagon_convert_frag
(bfd *abfd ATTRIBUTE_UNUSED, segT sec ATTRIBUTE_UNUSED, fragS *fragP)
{
  hexagon_packet *apacket;

  apacket = &fragP->tc_frag_data->packet;

  if (apacket->drlx)
    hexagon_relax_branch (fragP);
  if (apacket->dpad || apacket->dpkt)
    hexagon_relax_falign (fragP);
}

/* We need to distinguish a register pair name (e.g., r1:0 or p3:0)
   from a label. */
int hexagon_start_label
(char c, char *before, char *after)
{
  static const char ex_before_legacy [] =
    "\\<((r((0*[12]?[13579])|31))|sp|lr|p3)$";
  static const char ex_after_legacy [] =
    "^((0*[12]?[02468])|30|fp)\\>";
  static const char ex_before_pairs [] =
    "\\<(((c|g|r)((0*[12]?[13579])|31))|(s((0*[12345]?[13579])|61|63))|sp|lr|p3)$";
  static const char ex_after_pairs [] =
    "^((0*[12345]?[02468])|60|62|fp)\\>";
  static regex_t re_before, re_after;
  static int re_ok;
  int er_before, er_after;

  /* Labels require a colon. */
  if (c != ':')
    return FALSE;

  /* The reader puts '\0' where the colon was. */
  after++;

  if (!re_ok)
    {
      const char *ex_before, *ex_after;

      ex_before = hexagon_if_arch_pairs ()? ex_before_pairs: ex_before_legacy;
      ex_after  = hexagon_if_arch_pairs ()? ex_after_pairs:  ex_after_legacy;

      /* Compile RE for GPR or predicate pairs. */
      assert (!xregcomp (&re_before, ex_before,
                         REG_EXTENDED | REG_ICASE | REG_NOSUB));
      assert (!xregcomp (&re_after,  ex_after,
                         REG_EXTENDED | REG_ICASE | REG_NOSUB));

      re_ok = TRUE;
    }

  /* Register pairs are not labels. */
  if ( !(er_before = xregexec (&re_before, before, 0, NULL, 0))
       && !(er_after = xregexec (&re_after,  after,  0, NULL, 0)))
    return FALSE;

  /* After everything else has been tried, this must be a label,
     but if it's inside a packet,
     its address might not be what one expects. */
  if (hexagon_in_packet)
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
hexagon_frag_fix_addr
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

/**  Allow certain fixups to be adjusted to make them relative to the
beginning of the section instead of the symbol.
*/

int
hexagon_fix_adjustable (fixS *fixP)
{
  /* FIXME: x86-64 adds some other cases here. */

  /* adjust_reloc_syms doesn't know about PIC or TLS.  */
  switch (fixP->fx_r_type)
    {
    /* PLT */
    case BFD_RELOC_HEX_PLT_B22_PCREL:
    /* GOT */
    case BFD_RELOC_HEX_GOTREL_LO16:
    case BFD_RELOC_HEX_GOTREL_HI16:
    case BFD_RELOC_32_GOTOFF:
    case BFD_RELOC_HEX_GOT_LO16:
    case BFD_RELOC_HEX_GOT_HI16:
    case BFD_RELOC_HEX_GOT_32:
    case BFD_RELOC_HEX_GOT_16:
    /* TLS */
    case BFD_RELOC_HEX_GD_GOT_LO16:
    case BFD_RELOC_HEX_GD_GOT_HI16:
    case BFD_RELOC_HEX_GD_GOT_32:
    case BFD_RELOC_HEX_GD_GOT_16:
    case BFD_RELOC_HEX_IE_LO16:
    case BFD_RELOC_HEX_IE_HI16:
    case BFD_RELOC_HEX_IE_32:
    case BFD_RELOC_HEX_IE_GOT_LO16:
    case BFD_RELOC_HEX_IE_GOT_HI16:
    case BFD_RELOC_HEX_IE_GOT_32:
    case BFD_RELOC_HEX_IE_GOT_16:
    case BFD_RELOC_HEX_TPREL_LO16:
    case BFD_RELOC_HEX_TPREL_HI16:
    case BFD_RELOC_HEX_TPREL_32:
      return FALSE;

    default:
      return TRUE;
    }
}

/* Initialize the various opcode and operand tables. */
void
hexagon_init
(int mach)
{
  static int init_p;

  if (!init_p)
    {
      if (!bfd_set_arch_mach (stdoutput, bfd_arch_hexagon, mach))
        as_warn (_("architecture and machine types not set; using default settings."));

      /* This initializes a few things in hexagon-opc.c that we need.
        This must be called before the various hexagon_xxx_supported fns.  */
      hexagon_opcode_init_tables (hexagon_get_opcode_mach (mach, target_big_endian));

      /* Initialize global NOP. */
      hexagon_find_nop ();
      /* Initialize global k-extension. */
      hexagon_find_kext ();

      /* Initialize ISA-specific features. */
      hexagon_autoand        &= hexagon_if_arch_autoand ();
      hexagon_extender       &= hexagon_if_arch_kext ();
      hexagon_pairing        &= hexagon_if_arch_pairs ();
      hexagon_pairing_branch &= hexagon_pairing;
      hexagon_pairing_duplex &= hexagon_pairing;
      hexagon_relax          &= hexagon_extender;
      hexagon_relax_long     &= hexagon_relax;

      /* Tell `.option' it's too late.  */
      cpu_tables_init_p = TRUE;

      init_p = TRUE;
    }
}

/** Initialize custom data in frag.

@param fragP Pointer to frag to be initialized.
@param previus Optional pointer to previous frag.
*/
void
hexagon_frag_init
(fragS *fragP, fragS *previous)
{
  /* Bug# 4443. Use calloc instead of malloc to zero out
   * the previous ptr. Otherwise previous will be set to
   * 0xbaadf00d and loops that terminate on previous == NULL
   * will access bad memory */
  if (!fragP->tc_frag_data)
    fragP->tc_frag_data = xcalloc (1, sizeof (*fragP->tc_frag_data));

  if (previous && fragP != previous)
    fragP->tc_frag_data->previous = previous;
}

/** Insert an operand value into an instruction.
*/
void
hexagon_insert_operand
(char *where, const hexagon_operand *operand, offsetT val, fixS *fixP)
{
  hexagon_insn insn;
  const hexagon_opcode *opcode;
  long xval;
  char *errmsg = NULL;

  if (target_big_endian)
    insn = bfd_getb32 ((unsigned char *) where);
  else
    insn = bfd_getl32 ((unsigned char *) where);

  opcode = hexagon_lookup_insn (insn);
  if (!opcode)
    as_bad (_("opcode not found."));

  if (!hexagon_encode_operand
         (operand, &insn, opcode, val,
          &xval, operand->flags & HEXAGON_OPERAND_IS_KXED,
          TRUE, &errmsg))
    if (errmsg)
      {
        if (fixP && fixP->fx_file)
          {
            char tmpError [200];
  	    sprintf (tmpError, _(" when resolving symbol in file %s at line %d."),
  	             fixP->fx_file, fixP->fx_line);
            strcat (errmsg, tmpError);
          }

        as_bad ("%s\n", errmsg);
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
hexagon_prefix_kext
(hexagon_packet_insn *kext, long xvalue)
{
  const hexagon_operand *operand;
  char *syntax;
  long mask;

  if (!kext)
    return FALSE;

  *kext = hexagon_kext_insn;

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

  operand = hexagon_lookup_operand (syntax);
  assert (operand);

  /* Truncate to as many bits as in the extension. */
  mask = ~(~0L << (operand->bits + operand->shift_count));
  xvalue &= mask? mask: ~0L;
  if (hexagon_encode_operand
        (operand, &kext->insn, kext->opcode, xvalue, NULL, FALSE, FALSE, NULL))
    {
      kext->used = TRUE;
      return TRUE;
    }

  return FALSE;
}

char *
hexagon_parse_immediate
(hexagon_packet_insn *insn, hexagon_packet_insn *prefix, const hexagon_operand *operand,
 char *str, long *val, char **errmsg)
{
  char *hold;
  expressionS exp;
  const hexagon_operand *operandx;
  long value = 0;
  long xvalue = 0;
  int is_may_x = FALSE, is_x = FALSE, is_hash = FALSE;
  int is_relax = FALSE;
  int is_lo16 = FALSE, is_hi16 = FALSE;
  char *suffix_line;
  hexagon_suffix_type suffix_type;

  is_may_x = (((insn->opcode->attributes & EXTENDABLE_LOWER_CASE_IMMEDIATE)
               && ISLOWER (operand->enc_letter))
              || ((insn->opcode->attributes & EXTENDABLE_UPPER_CASE_IMMEDIATE)
                  && ISUPPER (operand->enc_letter)));

  /* We only have the mandatory '#' for immediates that are NOT PC-relative */
  if (*str == '#')
    {
      is_hash = (operand->flags & HEXAGON_OPERAND_PC_RELATIVE);
      str++;
      if (*str == '#')
        {
          is_x = (hexagon_extender);
          str++;
        }
      else
        {
          is_x = (hexagon_extender) && (insn->opcode->attributes & MUST_EXTEND);
        }
      is_hash = is_hash && !is_x;
    }
  else if (!(operand->flags & HEXAGON_OPERAND_PC_RELATIVE))
    return NULL;

  is_relax = is_may_x && !is_x && !is_hash
             && ENCODE_RELAX (operand->reloc_type);

  if (is_x && !insn->opcode->map)
    {
      /* Check if the operand can truly be extended. */
      if (!is_may_x)
        {
          if (errmsg)
            *errmsg = _("operand cannot be extended.");
          return NULL;
        }
    }

  /* Hexagon TODO: Allow white space between lo/hi and the paren */
  if (TOLOWER (str [0]) == 'l' && TOLOWER (str [1]) == 'o' && str [2] == '(')
    {
      /* Make sure we have a corresponding lo16 operand */
      operand = hexagon_operand_find_lo16 (operand);
      if (!operand)
        return NULL;

      is_lo16 = 1;
      str += 3;
    }
  else if (TOLOWER (str [0]) == 'h' && TOLOWER (str [1]) == 'i' && str [2] == '(')
    {
      /* Make sure we have a corresponding hi16 operand */
      operand = hexagon_operand_find_hi16 (operand);
      if (!operand)
        return NULL;

      is_hi16 = 1;
      str += 3;
    }

  hold = input_line_pointer;
    {
      input_line_pointer = str;

      suffix_line = hexagon_parse_suffix (&suffix_type, &str);
      if (suffix_line)
        input_line_pointer = suffix_line;

      expression (&exp);

      if (!suffix_line)
        str = input_line_pointer;
    }
  input_line_pointer = hold;

  operandx = NULL;

  if (suffix_type == PIC_GOT)
    operandx = hexagon_operand_find (operand, "got");
  else if (suffix_type == PIC_GOTREL)
    operandx = hexagon_operand_find (operand, "gotrel");
  else if (suffix_type == PIC_PLT)
    operandx = hexagon_operand_find (operand, "plt");
  else if (suffix_type == TLS_GD_GOT)
    operandx = hexagon_operand_find (operand, "gdgot");
  else if (suffix_type == TLS_IE)
    operandx = hexagon_operand_find (operand, "ie");
  else if (suffix_type == TLS_IE_GOT)
    operandx = hexagon_operand_find (operand, "iegot");
  else if (suffix_type == TLS_DTPREL)
    operandx = hexagon_operand_find (operand, "dtprel");
  else if (suffix_type == TLS_TPREL)
    operandx = hexagon_operand_find (operand, "tprel");

  if (operandx)
    /* Get new PIC operand. */
    operand = operandx;

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
      /* Interpret value according to the 32-bit Hexagon ISA */
      value = (int32_t) exp.X_add_number;

      if (!hexagon_encode_operand
             (operand, &insn->insn, insn->opcode,
              value, is_x? &xvalue: NULL, is_x, FALSE, errmsg))
          return NULL;

      if (is_x || xvalue)
        {
          /* Emit prefix only if requested or needed. */
          if (hexagon_prefix_kext (prefix, xvalue))
            insn->flags |= HEXAGON_INSN_IS_KXED;
          else
            return NULL;
        }
    }
  else
    {
      if (prefix && is_x)
        {
          assert ((is_x = hexagon_prefix_kext (prefix, 0)));

          operandx = hexagon_lookup_reloc (operand->reloc_kxer, 0, prefix->opcode);
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

      insn->flags         |= (is_x? HEXAGON_INSN_IS_KXED: 0)
                             | (is_relax? HEXAGON_INSN_IS_RELAX: 0);
      insn->exp            = exp;
      insn->operand        = *operand;
      insn->operand.flags |= (is_x? HEXAGON_OPERAND_IS_KXED: 0)
                             | (is_relax? HEXAGON_OPERAND_IS_RELAX: 0);
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
hexagon_insn_init
(hexagon_packet_insn *ainsn)
{
  memset (ainsn, 0, sizeof (*ainsn));
  *ainsn = hexagon_nop_insn;
  ainsn->pad = TRUE;
}

char*
hexagon_insn_write
(hexagon_insn insn, size_t fc, const hexagon_operand *op, expressionS *exp,
 char *stream, size_t offset, fixS **fixSP, int lineno ATTRIBUTE_UNUSED)
{
  fixS *fixP = NULL;
  addressT pc;

  /* Create any fixups */
  if (fc)
    {
      bfd_reloc_code_real_type reloc_type;
      hexagon_operand *operand;

      operand = xmalloc (sizeof (*operand));
      *operand = *op;

      if (operand->flags & HEXAGON_OPERAND_IS_KXED)
        reloc_type = operand->reloc_kxed;
      else
        reloc_type = operand->reloc_type;

      pc = frag_now->fr_address + frag_now_fix ();

      fixP
        = fix_new_exp
            (frag_now, stream + offset - frag_now->fr_literal, HEXAGON_INSN_LEN,
             exp,
             (operand->flags & HEXAGON_OPERAND_PC_RELATIVE) == HEXAGON_OPERAND_PC_RELATIVE,
             reloc_type);
      fixP->tc_fix_data = operand;

      if (operand->flags & (HEXAGON_OPERAND_IS_LO16 | HEXAGON_OPERAND_IS_HI16))
        fixP->fx_no_overflow = TRUE;

      if (operand->flags & HEXAGON_OPERAND_PC_RELATIVE)
        /* Any PC relative operands within a packet must be relative
           to the address of the packet, so the offset is adjusted to
           account for that. */
        fixP->fx_offset += offset;
    }
  else
    /* Allocate space for an insn. */
    pc = frag_now->fr_address + frag_now_fix ();

  /* Write out the instruction. */
  md_number_to_chars (stream + offset, insn, HEXAGON_INSN_LEN);

  if (pc % HEXAGON_INSN_LEN)
    as_warn (_("current location is not %d-byte aligned."), HEXAGON_INSN_LEN);

  /* Hexagon insns will never have more than 1 fixup? */
  if (fixSP)
    *fixSP = fixP;

  return (stream + offset);
}

/** Mark the next packet for .falign.

@see hexagon_packet_falign ().
*/
void
hexagon_falign
(int ignore ATTRIBUTE_UNUSED)
{
  if (!hexagon_faligning)
    {
      hexagon_faligning = hexagon_fetch_align;
      n_falign [HEXAGON_FALIGN_TOTAL]++;
    }

  demand_empty_rest_of_line ();

  /* Just in case out of .text. */
  record_alignment (now_seg, DEFAULT_CODE_FALIGN);
}

/** Insert a NOP packet to align code.

@param fragP pointer to current code fragment.
@param count of bytes to fill up.
*/
void
hexagon_align_code
(fragS *fragP, size_t count)
{
  const hexagon_insn a_nop = hexagon_nop_insn.insn | HEXAGON_END_NOT,
                   z_nop = hexagon_nop_insn.insn | HEXAGON_END_PACKET,
                   *p_nop;
  size_t skip, here;

  if (count > 0)
    {
      if ((fragP->fr_address + fragP->fr_fix + count) % HEXAGON_INSN_LEN)
        as_warn (_("code alignment must be a multiple of %d bytes."), HEXAGON_INSN_LEN);

      if ((skip = count % HEXAGON_INSN_LEN))
        /* Pad to 4-byte boundary first. */
        memset (fragP->fr_literal + fragP->fr_fix, 0, skip);
      /* Add NOPs to packet. */
      for (here = 0; skip + here + HEXAGON_INSN_LEN < count; here += HEXAGON_INSN_LEN)
        {
          if (  (skip + here) / HEXAGON_INSN_LEN % MAX_PACKET_INSNS
              < MAX_PACKET_INSNS - 1)
            p_nop = &a_nop;
          else
            p_nop = &z_nop;
          memcpy (fragP->fr_literal + fragP->fr_fix + skip + here, p_nop, HEXAGON_INSN_LEN);
        }
      /* Finalize packet. */
      if (skip + here + HEXAGON_INSN_LEN <= count)
        memcpy (fragP->fr_literal + fragP->fr_fix + skip + here, &z_nop, HEXAGON_INSN_LEN);

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
hexagon_has_single
(const hexagon_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of memory ops in this packet. */
  for (i = count = 0; i < hexagon_packet_count (apacket); i++)
    if (apacket->insns [i].opcode->attributes & A_RESTRICT_SINGLE_MEM_FIRST)
      count++;

  return (count == 1);
}

/** Check if packet has a prefix insn.

@param packet Packet to examine.
@return True if so.
*/
int
hexagon_has_prefix
(const hexagon_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of memory ops in this packet. */
  for (i = count = 0; i < hexagon_packet_count (apacket); i++)
    if (apacket->insns [i].opcode->attributes & A_IT_EXTENDER)
      count++;

  return (count);
}

/** Check if packet has a paired insn.

@param packet Packet to examine.
@return True if so.
*/
int
hexagon_has_pair
(const hexagon_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of duplex insns in this packet. */
  for (i = count = 0; i < hexagon_packet_count (apacket); i++)
    if (apacket->insns [i].opcode->attributes & PACKED)
      count++;

  return (count);
}

/** Check if packet has a duplex insn.

@param packet Packet to examine.
@return True if so.
*/
int
hexagon_has_duplex
(const hexagon_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of duplex insns in this packet. */
  for (i = count = 0; i < hexagon_packet_count (apacket); i++)
    if (apacket->insns [i].opcode->flags & HEXAGON_CODE_IS_DUPLEX)
      count++;

  return (count);
}

/** Check if packet has a duplex insn and if a new insn would conflict with it
    or vice-versa.

@param apacket Packet to examine.
@param ainsn Insn to consider.
@param which Which insn in packet to ignore (in) or conflicts (out) (optional).
@return True if so.
*/
int
hexagon_has_duplex_hits
(const hexagon_packet *apacket, const hexagon_packet_insn *ainsn, size_t *which)
{
  size_t i;

  /* Count number of memory ops in this packet. */
  for (i = 0; i < hexagon_packet_count (apacket); i++)
    if (((apacket->insns [i].opcode->flags & HEXAGON_CODE_IS_DUPLEX)
         && (ainsn->opcode->slots & HEXAGON_SLOTS_DUPLEX)
         && !(ainsn->opcode->slots & ~HEXAGON_SLOTS_DUPLEX))
        || ((ainsn->opcode->flags & HEXAGON_CODE_IS_DUPLEX)
            && (apacket->insns [i].opcode->slots & HEXAGON_SLOTS_DUPLEX)
            && !(apacket->insns [i].opcode->slots & ~HEXAGON_SLOTS_DUPLEX)))
      {
        if (which)
          {
            if (*which == i)
              continue;
            else
              *which = i;
          }

        return TRUE;
      }

  return FALSE;
}

/** Check if queue has a duplex insn and if a new insn would conflict with it
    or vice-versa.

@param aqueue Queue to examine.
@param ainsn Insn to consider.
@return True if so.
*/
int
hexagon_has_duplex_clash
(const hexagon_queue *aqueue, const hexagon_packet_insn *ainsn)
{
  size_t i;

  /* Count number of memory ops in this packet. */
  for (i = 0; i < aqueue->size; i++)
    if (!aqueue->insns [i].pad && !ainsn->pad)
      if (((aqueue->insns [i].opcode->flags & HEXAGON_CODE_IS_DUPLEX)
           && (ainsn->opcode->slots & HEXAGON_SLOTS_DUPLEX)
           && !(ainsn->opcode->slots & ~HEXAGON_SLOTS_DUPLEX))
          || ((ainsn->opcode->flags & HEXAGON_CODE_IS_DUPLEX)
              && (aqueue->insns [i].opcode->slots & HEXAGON_SLOTS_DUPLEX)
              && !(aqueue->insns [i].opcode->slots & ~HEXAGON_SLOTS_DUPLEX)))
        return TRUE;

  return FALSE;
}

/** Check if packet has a R.NEW insn.

@param packet Packet to examine.
@param reg Which register is expected.
@return True if so.
*/
int
hexagon_has_rnew
(const hexagon_packet *apacket, hexagon_packet_insn **insn)
{
  size_t i;

  /* Count number of R.NEW insns in this packet. */
  for (i = 0; i < hexagon_packet_count (apacket); i++)
    if (apacket->insns [i].flags & HEXAGON_INSN_IN_RNEW)
      {
        if (insn)
          *insn = ((hexagon_packet *) apacket)->insns + i;

        return TRUE;
      }

  return FALSE;
}

/** Check if packet has memory operation insns.

@param packet Packet to examine.
@return Number of such insns.
*/
int
hexagon_has_mem
(const hexagon_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of memory ops in this packet. */
  for (i = count = 0; i < hexagon_packet_count (apacket); i++)
    if (apacket->insns [i].opcode
        && (apacket->insns [i].opcode->attributes & A_RESTRICT_SINGLE_MEM_FIRST))
      count++;

  return (count);
}

/** Check if packet has a store.

@param packet Packet to examine.
@return True if so.
*/
int
hexagon_has_store
(const hexagon_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of duplex insns in this packet. */
  for (i = count = 0; i < hexagon_packet_count (apacket); i++)
    if (apacket->insns [i].opcode->attributes & A_STORE)
      count++;

  return (count);
}

/** Check if packet has a store restriction.

@param packet Packet to examine.
@return True if so.
*/
int
hexagon_has_store_not
(const hexagon_packet *apacket)
{
  size_t i;
  int count;

  /* Count number of duplex insns in this packet. */
  for (i = count = 0; i < hexagon_packet_count (apacket); i++)
    if (apacket->insns [i].opcode->attributes & A_RESTRICT_NOSLOT1_STORE)
      count++;

  return (count);
}

/** Check if packet has an insn that precludes others insns in packet,
but A and X-type insns.

@param packet Packet to examine.
@return True if so.
*/
int
hexagon_has_but_ax
(const hexagon_packet *apacket)
{
  size_t i;
  size_t axok, count;

  /* Count number of non-A and non-X insns in this packet. */
  for (i = axok = count = 0; i < hexagon_packet_count (apacket); i++)
    {
      if (apacket->insns [i].opcode->attributes & A_RESTRICT_PACKET_AXOK)
        /* Check for special cases. */
        if (!(apacket->insns [i].opcode->attributes & A_RESTRICT_PACKET_SOMEREGS_OK)
            || ((apacket->insns [i].opcode->attributes & A_RESTRICT_PACKET_SOMEREGS_OK)
                && ((hexagon_if_arch_v3 ()
                     && apacket->insns [i].oreg != 2  /* SSR */
                     && apacket->insns [i].oreg != 21 /* TLBHI */
                     && apacket->insns [i].oreg != 22 /* TLBLO */
                     && apacket->insns [i].oreg != 23 /* TLBIDX */ )
                    || (hexagon_if_arch_v4 ()
                        && apacket->insns [i].oreg != 6 /* SSR */ ))))
          axok++;

      if (!(apacket->insns [i].opcode->attributes & PACKED)
          && (HEXAGON_INSN_TYPE_A (apacket->insns [i].insn)
              || HEXAGON_INSN_TYPE_X (apacket->insns [i].insn)))
        count++;
    }

  return (!axok || ((axok + count) == hexagon_packet_count (apacket)));
}

int
hexagon_is_nop_keep
(const hexagon_packet *apacket, int current)
{
  int found = FALSE;
  int prev = current - 1;
  int next = current + 1;

  if (next < MAX_PACKET_INSNS)
    {
      while (prev >= 0)
        {
          if (!hexagon_is_nop (apacket->insns [prev].insn))
            {
              found = TRUE;
              break;
            }
          prev--;
        }

      /* A_RESTRICT_NOSLOT1: the next instruction cannot be in ndx 1
         For V2, only when the next instruction can be put in ndx 1
         that should the current nop be kept. */
      if (found
	  && (apacket->insns [next].opcode->attributes & A_RESTRICT_NOSLOT1))
        {
              if ((apacket->insns [next].opcode->slots & HEXAGON_SLOTS_1))
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
hexagon_packet_insert
(hexagon_packet *packet,
 const hexagon_packet_insn *insn, const hexagon_packet_insn *prefix,
 const hexagon_packet_pair *pair, int pad)
{
  int prefixed = !pad && (insn->flags & HEXAGON_INSN_IS_KXED)
                 && prefix && (prefix->opcode->attributes & A_IT_EXTENDER)
                 ? 1: 0;
  int duplex   = (insn->opcode->flags & HEXAGON_CODE_IS_DUPLEX)? 1: 0;
  int relax    = (insn->flags & HEXAGON_INSN_IS_RELAX)? 1: 0;
  int size     = pad? hexagon_packet_count (packet): hexagon_packet_slots (packet);
  int length   = pad? hexagon_packet_count (packet): hexagon_packet_length (packet);

  if (duplex && packet->duplex)
    /* Limit duplex insns to one per packet. */
    return FALSE;

  if ((size + duplex + relax < MAX_PACKET_INSNS)
      && (length + prefixed + relax < MAX_PACKET_INSNS))
    {
      packet->insns [hexagon_packet_count (packet)] = *insn;
      packet->insns [hexagon_packet_count (packet)].pad = pad;

      if (prefixed)
        {
          packet->insns [hexagon_packet_count (packet)].flags |= HEXAGON_INSN_IS_KXED;

          packet->prefixes [hexagon_packet_count (packet)] = *prefix;
          packet->prefixes [hexagon_packet_count (packet)].pad = pad;
          packet->prefixes [hexagon_packet_count (packet)].lineno
            = packet->insns [hexagon_packet_count (packet)].lineno;

          packet->prefix++;
        }

      if (pair)
        {
          packet->pairs [hexagon_packet_count (packet)].left  = pair->left;
          packet->pairs [hexagon_packet_count (packet)].right = pair->right;
        }

      packet->lineno = insn->lineno > packet->lineno
                       ? insn->lineno: packet->lineno;

      packet->is_inner |= insn->is_inner;
      packet->is_outer |= insn->is_outer;

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
hexagon_packet_cram
(hexagon_packet *apacket,
 hexagon_packet_insn *ainsn, const hexagon_packet_insn *aprefix,
 const hexagon_packet_pair *pair, int pad)
{
  hexagon_packet packet;
  hexagon_packet_insn insn;
  size_t i;

  /* Try to insert insn. */
  if (hexagon_packet_insert (apacket, ainsn, aprefix, pair, pad))
    return TRUE;
  else
    {
      insn = *ainsn;

      if (!(insn.flags & HEXAGON_INSN_IS_PAIR))
        {
          /* Remove branch relaxation if range has not been reduced
             by pairing. */
          insn.relax          = HEXAGON_RELAX_NONE;
          insn.flags         &= ~HEXAGON_INSN_IS_RELAX;
          insn.operand.flags &= ~HEXAGON_OPERAND_IS_RELAX;

          /* Try again. */
          if (hexagon_packet_insert (apacket, &insn, aprefix, pair, pad))
            {
              *ainsn = insn;
              return TRUE;
            }
        }

      if (apacket->relax)
        {
          packet = *apacket;
          /* Remove any branch relaxation if range has not been reduced
             by pairing. */
          for (i = packet.size; i > 0; i--)
            if (!(packet.insns [i - 1].flags & HEXAGON_INSN_IS_PAIR)
                && packet.insns [i - 1].flags & HEXAGON_INSN_IS_RELAX)
              {
                    packet.insns [i - 1].relax          = HEXAGON_RELAX_NONE;
                    packet.insns [i - 1].flags         &= ~HEXAGON_INSN_IS_RELAX;
                    packet.insns [i - 1].operand.flags &= ~HEXAGON_OPERAND_IS_RELAX;

                    packet.relax--;

                    /* Try again. */
                    if (hexagon_packet_insert (&packet, &insn, aprefix, pair, pad))
                      {
                        *apacket = packet;
                        return TRUE;
                      }
                  }
          }

      /* Remove branch relaxation. */
      insn.relax          = HEXAGON_RELAX_NONE;
      insn.flags         &= ~HEXAGON_INSN_IS_RELAX;
      insn.operand.flags &= ~HEXAGON_OPERAND_IS_RELAX;

      /* Try again. */
      if (hexagon_packet_insert (apacket, &insn, aprefix, pair, pad))
        {
          *ainsn = insn;
          return TRUE;
        }

      if (apacket->relax)
        {
          packet = *apacket;
          /* Remove any branch relaxation. */
          for (i = packet.size - 1; i < packet.size; i--)
            if (packet.insns [i].flags & HEXAGON_INSN_IS_RELAX)
              {
                packet.insns [i].relax          = HEXAGON_RELAX_NONE;
                packet.insns [i].flags         &= ~HEXAGON_INSN_IS_RELAX;
                packet.insns [i].operand.flags &= ~HEXAGON_OPERAND_IS_RELAX;

                packet.relax--;

                /* Try again. */
              if (hexagon_packet_insert (&packet, &insn, aprefix, pair, pad))
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
hexagon_packet_init
(hexagon_packet *apacket)
{
  size_t i;

  memset (apacket, 0, sizeof (*apacket));

  for (i = 0; i < MAX_PACKET_INSNS; i++)
    {
      hexagon_insn_init (&apacket->insns [i]);
      hexagon_insn_init (&apacket->prefixes [i]);
      hexagon_insn_init (&apacket->pairs [i].left.insn);
      hexagon_insn_init (&apacket->pairs [i].left.prefix);
      hexagon_insn_init (&apacket->pairs [i].right.insn);
      hexagon_insn_init (&apacket->pairs [i].right.prefix);
    }
}

/** Move prefixes to the side-lines.

@param apacket Reference to a packet.
*/
void
hexagon_packet_unfold
(hexagon_packet *apacket)
{
  hexagon_packet packet;
  hexagon_packet_insn prefix;
  size_t size;
  size_t i;

  size = hexagon_packet_count (apacket);

  /* Ignore an empty packet. */
  if (!hexagon_packet_count (apacket))
    return;

  hexagon_packet_init (&packet);
  hexagon_insn_init (&prefix);

  for (i = 0; i < size; i++)
    {
      if (apacket->insns [i].pad)
        continue;

      if (apacket->insns [i].opcode->attributes & A_IT_EXTENDER)
        {
          if (apacket->insns [i].fc && apacket->insns [i].fix)
            apacket->insns [i].fix->fx_offset -= apacket->insns [i].fix->fx_pcrel
                                                 ? i * HEXAGON_INSN_LEN: 0;

          prefix = apacket->insns [i];
          continue;
        }

      if ((apacket->insns [i].flags & HEXAGON_INSN_IS_KXED))
        {
          if (prefix.fc && prefix.fix)
            {
              prefix.fix->fx_where   = packet.size * HEXAGON_INSN_LEN;
              prefix.fix->fx_offset += prefix.fix->fx_pcrel
                                       ? packet.size * HEXAGON_INSN_LEN: 0;
            }

          /* Insert the prefix. */
          packet.prefixes [packet.size] = prefix;
          packet.prefix++;
        }

      if (apacket->insns [i].fc && apacket->insns [i].fix)
        {
          apacket->insns [i].fix->fx_where   = packet.size * HEXAGON_INSN_LEN;
          apacket->insns [i].fix->fx_offset += apacket->insns [i].fix->fx_pcrel
                                               ? (packet.size - i) * HEXAGON_INSN_LEN
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
hexagon_packet_fold
(hexagon_packet *apacket)
{
  hexagon_packet packet;
  size_t size, prefix;
  size_t i;

  size = hexagon_packet_count (apacket);
  prefix = apacket->prefix;
  if (!size || !prefix)
    return;

  hexagon_packet_init (&packet);

  /* Merge the insn and the prefix arrays. */
  for (i = 0; i < size; i++)
    {
      if (apacket->insns [i].pad && prefix)
        {
          /* Skip a padding insn to make room for a prefix. */
          prefix--;
          continue;
        }

      if (apacket->insns [i].flags & HEXAGON_INSN_IS_KXED)
        {
          if (apacket->prefixes [i].fc && apacket->prefixes [i].fix)
            {
              apacket->prefixes [i].fix->fx_where
                = packet.size * HEXAGON_INSN_LEN;
              apacket->prefixes [i].fix->fx_offset
                += apacket->prefixes [i].fix->fx_pcrel
                   ? (packet.size - i) * HEXAGON_INSN_LEN: 0;
            }

          /* Insert the prefix. */
          packet.insns [packet.size++] = apacket->prefixes [i];
          /* Sanity check. */
          assert (packet.size < MAX_PACKET_INSNS);
          packet.prefix++;
        }

      if (apacket->insns [i].fc && apacket->insns [i].fix)
        {
          apacket->insns [i].fix->fx_where   = packet.size * HEXAGON_INSN_LEN;
          apacket->insns [i].fix->fx_offset += apacket->insns [i].fix->fx_pcrel
                                               ? (packet.size - i) * HEXAGON_INSN_LEN
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

/** Remove padded insns.

Assumes that the packet is in its final form (no prefixes nor pairs).

@param apacket Reference to a packet.
*/
void
hexagon_packet_unpad
(hexagon_packet *apacket)
{
  hexagon_packet packet;
  size_t size;
  size_t i;

  size = hexagon_packet_count (apacket);
  if (!size)
    return;

  hexagon_packet_init (&packet);

  /* Merge the insn and the prefix arrays. */
  for (i = 0; i < size; i++)
    {
      if (apacket->insns [i].pad)
        continue;

      if (apacket->insns [i].fc && apacket->insns [i].fix)
        {
          apacket->insns [i].fix->fx_where   = packet.size * HEXAGON_INSN_LEN;
          apacket->insns [i].fix->fx_offset += apacket->insns [i].fix->fx_pcrel
                                               ? (packet.size - i) * HEXAGON_INSN_LEN
                                               : 0;
        }

      /* Insert the insn. */
      packet.insns [packet.size++] = apacket->insns [i];
      /* Sanity check. */
      assert (packet.size <= MAX_PACKET_INSNS);
    }

  /* Copy new insn array and clear the prefix array. */
  memcpy (apacket->insns,    packet.insns,    sizeof (apacket->insns));

  /* Update housekeeping. */
  apacket->size = packet.size;
}

/** Write the a packet out.

@param apacket Reference to a packet.
*/
void
hexagon_packet_write
(hexagon_packet *apacket)
{
  fragS *previous;
  char *first;
  size_t max_skip, num_nops, num_padded_nops;
  size_t req_insns;
  size_t i;

  apacket->faligned = hexagon_faligning;
  hexagon_faligning   = FALSE;

  hexagon_packet_fold (apacket);

  /* Determine if we can skip any NOP, for
     at least 2 instructions are needed for :endloop0 and
     at least 3 instructions for :endloop1; otherwise
     all can be skipped and nothing is emitted. */
  max_skip = hexagon_packet_count (apacket)
             - ((apacket->is_inner || apacket->is_outer)
                ? (apacket->is_outer? 3: 2) : 0);

  /* Calculate the number of NOP needed. */
  for (i = 0, num_nops = 0, num_padded_nops = 0;
       i < hexagon_packet_count (apacket);
       i++)
    if (hexagon_is_nop (apacket->insns [i].insn)
        && !hexagon_is_nop_keep (apacket, i))
      {
        num_nops++;

        if (apacket->insns [i].pad)
          num_padded_nops++;
      }
  max_skip = MIN (max_skip, MIN (num_nops, num_padded_nops));

  /* Keep track of the number of emitted instructions to
     determine which packet bits to set. */
  req_insns = hexagon_packet_count (apacket) - max_skip;

  /* Make sure that packet frags have nothing else. */
  if (obstack_object_size (&frchain_now->frch_obstack))
    {
      previous = frag_now;
      frag_wane (frag_now);
      frag_new (0);
      hexagon_frag_init (frag_now, previous);
    }

  /* Make room for maximum packet size and possible padding. */
  previous = frag_now;
  frag_grow (2 * MAX_PACKET_INSNS * HEXAGON_INSN_LEN);
  first = frag_more (req_insns * HEXAGON_INSN_LEN);
  hexagon_frag_init (frag_now, previous);

  /* Initialize scratch packet. */
  frag_now->tc_frag_data->packet = *apacket;
  frag_now->tc_frag_data->packet.size = 0;

  for (i = 0; i < hexagon_packet_count (apacket); i++)
    {
      size_t size = hexagon_packet_count (&frag_now->tc_frag_data->packet);

      /* Skip the padded NOP, not every NOP. */
      if (max_skip
	  && apacket->insns [i].pad
	  && hexagon_is_nop (apacket->insns [i].insn)
	  && !hexagon_is_nop_keep (apacket, i))
	{
	  max_skip--;
	  continue;
        }

      /* Set proper packet bits. */
      if (size == (1 - 1) && apacket->is_inner)
	apacket->insns [i].insn =
          HEXAGON_END_PACKET_SET (apacket->insns [i].insn, HEXAGON_END_LOOP);
      else if (size == (2 - 1) && apacket->is_outer)
	apacket->insns [i].insn =
          HEXAGON_END_PACKET_SET (apacket->insns [i].insn, HEXAGON_END_LOOP);
      else if (size >= req_insns - 1
               && HEXAGON_END_PACKET_GET (apacket->insns [i].insn) != HEXAGON_END_PAIR)
	apacket->insns [i].insn =
          HEXAGON_END_PACKET_SET (apacket->insns [i].insn, HEXAGON_END_PACKET);
      /* Otherwise, leave the packet bits alone. */

      apacket->insns [i].pad = FALSE;
      frag_now->tc_frag_data->packet.insns [size] = apacket->insns [i];
      hexagon_insn_write (frag_now->tc_frag_data->packet.insns [size].insn,
			  frag_now->tc_frag_data->packet.insns [size].fc,
			  &frag_now->tc_frag_data->packet.insns [size].operand,
			  &frag_now->tc_frag_data->packet.insns [size].exp,
			  first, size * HEXAGON_INSN_LEN,
			  &frag_now->tc_frag_data->packet.insns [size].fix,
			  frag_now->tc_frag_data->packet.insns [size].lineno);

      /* Count insn as legit. */
      frag_now->tc_frag_data->packet.size++;
    }

  dwarf2_emit_insn (frag_now->tc_frag_data->packet.size * HEXAGON_INSN_LEN);

  assert (req_insns == frag_now->tc_frag_data->packet.size);

  previous = frag_now;
  frag_var (rs_machine_dependent,
            (2 * MAX_PACKET_INSNS - req_insns) * HEXAGON_INSN_LEN,
            0, 0, NULL, 0, first);
  hexagon_frag_init (frag_now, previous);
  hexagon_packet_init (&frag_now->tc_frag_data->packet);

  /* Prepare packet for next insns. */
  hexagon_packet_init (apacket);
}

/** Form a packet from the previous few insns.
*/
int
hexagon_packet_form
(hexagon_packet *apacket, hexagon_queue *aqueue)
{
  hexagon_packet_insn insn, prefix;
  hexagon_packet_pair both;
  hexagon_packet packet [2]; /* As many packets as restrictions. */
  hexagon_queue queue;
  size_t i, j, k;
  int ok;

  if (hexagon_pairing)
    {
      static const struct
        {
          unsigned in, out;
        }
      restrictions [] =
        {
          { 0, HEXAGON_CODE_IS_DUPLEX },
          { 0, 0 },
        }; /* As many restrictions as packets. */

      /* Form tentative packets according to restrictions. */
      for (k = 0; k < 2; k++)
        {
          queue = *aqueue;

          for (j = 0; j < queue.size; j++)
            {
              if (queue.insns [j].used)
                /* Skip an already used insn. */
                continue;

              both.left.insn   = queue.insns [j];
              both.left.prefix = queue.prefixes [j];

              for (i = j + 1; i < queue.size; i++)
                {
                  size_t m, n;

                  if (/* Skip an already used insn. */
                      queue.insns [i].used
                      /* Skip restricted insns. */
                      || (queue.insns [i].opcode->flags & restrictions [k].in))
                    continue;

                  both.right.insn   = queue.insns [i];
                  both.right.prefix = queue.prefixes [i];

                  if (hexagon_assemble_pair (&both, &insn, &prefix)
                      && !(insn.opcode->flags & restrictions [k].out))
                    {
                      /* Keep source order of certain insns. */
                      if ((queue.insns [i].opcode->flags & HEXAGON_CODE_IS_BRANCH)
                          || (queue.insns [i].opcode->flags & HEXAGON_CODE_IS_STORE))
                        m = i, n = j;
                      else
                        m = j, n = i;

                      queue.insns [m].used = queue.insns [n].used = TRUE;
                      queue.insns [m].pad  = queue.insns [n].pad  = TRUE;

                      if (!hexagon_has_duplex_clash (&queue, &insn))
                        {
                          /* Take pair if no clashes. */
                          insn.used = TRUE;
                          insn.pad  = FALSE;

                          /* Replace insn with pair. */
                          queue.insns [m]    = insn;
                          queue.prefixes [m] = prefix;
                          queue.pairs [m]    = both;

                          break;
                        }
                      else
                        {
                          /* Keep the original insn otherwise. */
                          queue.insns [m].used = queue.insns [n].used = FALSE;
                          queue.insns [m].pad  = queue.insns [n].pad  = FALSE;
                        }
                    }
                }
            }

          /* Form packet with surviving insns. */
          hexagon_packet_init (packet + k);
          packet [k].is_inner = queue.is_inner;
          packet [k].is_outer = queue.is_outer;

          for (ok = TRUE, i = 0; i < queue.size; i++)
            {
              if ((queue.insns [i].pad))
                /* Skip an insn alredy inserted into the packet. */
                continue;

              if (!hexagon_packet_cram (packet + k,
                                      queue.insns + i, queue.prefixes + i,
                                      queue.pairs + i,
                                      FALSE))
                {
                  hexagon_packet_init (packet + k);
                  ok = FALSE;
                  break;
                }
              else
                /* Mark insn as inserted into packet. */
                queue.insns [i].pad = TRUE;
            }
        }

      for (k = 0; k < 2; k++)
        if (!apacket->size
            || (packet [k].size && packet [k].size < apacket->size))
          /* Pick a valid, smaller packet. */
          *apacket = packet [k];
    }
  else
    {
      queue = *aqueue;

      hexagon_packet_init (apacket);
      apacket->is_inner = queue.is_inner;
      apacket->is_outer = queue.is_outer;

      for (ok = TRUE, i = 0; i < queue.size; i++)
        {
          if ((queue.insns [i].pad))
            /* Skip an insn alredy inserted into the packet. */
            continue;

          if (!hexagon_packet_cram (apacket,
                                  queue.insns + i, queue.prefixes + i,
                                  queue.pairs + i,
                                  FALSE))
            {
              ok = FALSE;
              break;
            }
          else
            /* Mark insn as inserted into packet. */
            queue.insns [i].pad = TRUE;
        }
    }

  if (!ok)
    {
      as_bad_where (NULL, queue.insns [i].lineno,
                    _("too many instructions in packet (maximum is %d)."),
                    MAX_PACKET_INSNS);
    }

  n_pairs [HEXAGON_PAIRS_TOTAL] += apacket->duplex;

  hexagon_queue_init (aqueue);
  return (ok);
}

/** Initialize insn queue.
*/
void
hexagon_queue_init
(hexagon_queue *aqueue)
{
  memset (aqueue, 0, sizeof (*aqueue));
}

/** Insert insn into queue.
*/
int
hexagon_queue_insert
(hexagon_queue *aqueue, hexagon_packet_insn *ainsn, hexagon_packet_insn *aprefix)
{
  if (aqueue->size < MAX_INSNS
      && !ainsn->pad)
    {
      aqueue->insns    [aqueue->size] = *ainsn;
      aqueue->prefixes [aqueue->size] = *aprefix;

      aqueue->faligned  = hexagon_faligning;
      aqueue->is_inner |= ainsn->is_inner;
      aqueue->is_outer |= ainsn->is_outer;

      aqueue->size++;
      return TRUE;
    }

  return FALSE;
}

/** Create a BSS section in the global data area.
*/
segT
hexagon_create_sbss_section
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
      /* record_alignment (hexagon_small_section, DEFAULT_DATA_ALIGNMENT); */

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
hexagon_create_scom_section
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

      (*small_section)                = hexagon_scom_section;
      (*small_section).name           = small_section_name;
      (*small_section).flags         |= flags | (access? SEC_LOAD: 0);
      (*small_section).output_section = small_section;
      (*small_section).symbol         = small_symbol;
      (*small_section).symbol_ptr_ptr = &(*small_section).symbol;

      (*small_symbol)                 = hexagon_scom_symbol;
      (*small_symbol).name            = small_section_name;
      (*small_symbol).section         = small_section;
    }

  subseg_set (current_section, current_subsec);

  return (small_section);
}

/* Create a literal section with the appropriate flags. */
segT
hexagon_create_literal_section
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

#define IS_SAME_SYMBOL(e,e1) (((e).X_op == (e1)->X_op) \
                              && (((e).X_op == O_symbol) || (e).X_op == O_absent) \
                              && ((e).X_op_symbol == (e1)->X_op_symbol) \
                              && ((e).X_add_symbol == (e1)->X_add_symbol) \
                              && ((e).X_add_number == (e1)->X_add_number))

/* add the expression exp to the .sdata literal pool */
hexagon_literal *
hexagon_add_to_lit_pool
(expressionS *exp, expressionS *exp1, size_t size)
{
  hexagon_literal *literal, *last;
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  offsetT mask = (size == 8)? 0xffffffffffffffffULL: 0xffffffffULL;

  /* Sanity check for illegal constant literals. */
  if (size != 8 && size != 4)
    as_fatal (_("invalid literal constant."));

  /* Try to find an existing literal for the expression in order to reuse it. */
  for (literal = last = hexagon_pool; literal != NULL; literal = literal->next)
    {
      /* Save the last node. */
      last = literal;

      // check for a constant value
      if ((literal->e.X_op == exp->X_op)
          && (literal->e.X_op == O_constant)
          && ((literal->e.X_add_number & mask) == (exp->X_add_number & mask))
          && (literal->size == size))
        break;

      // check for 32-bit or 64-bit (two 32-bit) symbol
      if (IS_SAME_SYMBOL(literal->e, exp)
          && IS_SAME_SYMBOL(literal->e1, exp1)
          && literal->size == size)
        break;

      // check for a 32-bit symbol in a 64-bit (2 32-bit) symbol
      if (IS_SAME_SYMBOL(literal->e1, exp)
          && literal->size == 8 && size == 4)
        break;
    }

  /* Do we need to create a new entry? */
  if (!literal)
    {
      if (hexagon_pool_counter >= MAX_LITERAL_POOL)
        {
          as_bad (_("too many literal constants."));
          return NULL;
        }

      literal = xmalloc (sizeof (*literal));
      literal->next = NULL;
      literal->size = size;

      /* save the head pointer to "hexagon_pool" */
      if (!hexagon_pool)
        hexagon_pool = last = literal;

      /* get the expression */
      literal->e = *exp;
      literal->e1 = *exp1;

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
            hexagon_create_literal_section (literal->secname, SEC_LINK_ONCE, bfd_log2 (size));
          literal->sub = LITERAL_SUBSECTION + (size == 8? 2: 1);
        }
      else /* if (literal->e.X_op == O_symbol) */
        /* Create a label symbol (with the literal order as the suffix). */
        {
          sprintf (literal->name, "%s_%04x", LITERAL_PREFIX, hexagon_pool_counter);
          strcpy (literal->secname, LITERAL_SECTION_A);

          literal->sec = hexagon_create_literal_section (literal->secname, 0, bfd_log2 (size));
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
      if(literal->e1.X_op == O_symbol || literal->e1.X_op == O_constant){
        emit_expr(&(literal->e), size/2);
        emit_expr(&(literal->e1), size/2);
      }
      else {
        emit_expr (&(literal->e), size);
      }

      /* Restore the last section. */
      subseg_set (current_section, current_subsec);

      hexagon_pool_counter++;

      /* Add literal to the literal pool list. */
      if (hexagon_pool != literal)
        last->next = literal;
    }

  return (literal);
}

/* This function searches the instruction string for GP related
 * instructions:
 * Rd32 = CONST32(#imm), Rd32 = CONST32(label)
 * Rdd32 = CONST64(#imm), Rdd32 = CONST64(label)
 * Rdd32 = CONST64(#imm|label, #imm|label)
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
hexagon_gp_const_lookup
(char *str, char *new_str)
{
  static const char ex_c32 []
    = "=[[:space:]]*" "\\<const32\\>" "[[:space:]]*"
      "\\([[:space:]]*#?[[:space:]]*(.+)[[:space:]]*\\)";
  static const char ex_c64 []
    = "=[[:space:]]*" "\\<const64\\>" "[[:space:]]*"
      "\\([[:space:]]*#?[[:space:]]*(.+)[[:space:]]*\\)";
  static const char ex_c64_2 []
    = "=[[:space:]]*" "\\<const64\\>" "[[:space:]]*"
      "\\([[:space:]]*#?[[:space:]]*(.+)[[:space:]]*,"
         "[[:space:]]*#?[[:space:]]*(.+)[[:space:]]*\\)";
  static const char ex_r32 []
    = "^\\<(r((0*[12]?[0-9])|30|31))|sp|fp|lr\\>";
  static const char ex_r64 []
    = "^\\<((r((0*[12]?[13579])|31))|sp|lr):((0*[12]?[02468])|30|fp)\\>";
  static regex_t re_c32, re_c64, re_c64_2, re_r32, re_r64;
  regmatch_t rm_left [1], rm_right [3];
  static int re_ok;
  int er_re;
  int size;
  expressionS exp, exp1;
  hexagon_literal *litcurptr;
  char *save;
  segT seg;
  int num_args;

  if (!re_ok)
    {
      /* Compile REs. */
      assert (!xregcomp (&re_c32, ex_c32, REG_EXTENDED | REG_ICASE));
      assert (!xregcomp (&re_c64, ex_c64, REG_EXTENDED | REG_ICASE));
      assert (!xregcomp (&re_c64_2, ex_c64_2, REG_EXTENDED | REG_ICASE));
      assert (!xregcomp (&re_r32, ex_r32, REG_EXTENDED | REG_ICASE));
      assert (!xregcomp (&re_r64, ex_r64, REG_EXTENDED | REG_ICASE));

      re_ok = TRUE;
    }

  /* Get the left and right-side expressions and distinguish between
     CONST32 and CONST64. */
  if (!(er_re = xregexec (&re_c32, str, 2, rm_right, 0)))
    {
      if (rm_right [1].rm_so < 0 || rm_right [1].rm_eo < 0
          || (er_re = xregexec (&re_r32, str, 1, rm_left, 0)))
        return FALSE;
      else
        size = 4;
      num_args = 1;
    }
  else if (!(er_re = regexec (&re_c64_2, str, 3, rm_right, 0)))
    {
      if (rm_right [1].rm_so < 0 || rm_right [1].rm_eo < 0
          || (er_re = regexec (&re_r64, str, 1, rm_left, 0)))
        return FALSE;
      else
        size = 8;
      num_args = 2;
    }
  else if (!(er_re = xregexec (&re_c64, str, 2, rm_right, 0)))
    {
      if (rm_right [1].rm_so < 0 || rm_right [1].rm_eo < 0
          || (er_re = xregexec (&re_r64, str, 1, rm_left, 0)))
        return FALSE;
      else
        size = 8;
      num_args = 1;
    }
  else
    return FALSE;

  if (!hexagon_gp_size)
    as_warn (_("using `CONST%d' with \"-G%d\"."), size * 8, hexagon_gp_size);

  /* Parse right-side expression. */
  save = input_line_pointer;
  input_line_pointer = str + rm_right [1].rm_so;
  seg = expression (&exp);
  if (num_args == 2)
    {
      input_line_pointer = str + rm_right [2].rm_so;
      seg = expression(&exp1);
    }
  else
    {
      exp1.X_op = O_absent;
      exp1.X_add_symbol = exp1.X_op_symbol = NULL;
      exp1.X_add_number = 0;
    }
  input_line_pointer = save;

  /* 64-bit literals must be constants. */
  if (exp.X_op != O_constant && size == 8 && num_args == 1)
    {
      as_bad (_("64-bit expression `%.*s' is not constant."),
              rm_right [1].rm_eo - rm_right [1].rm_so,
              str + rm_right [1].rm_so);
      return FALSE;
    }

  if (num_args == 2 && size != 8)
    {
      as_bad (_("two arguments allowed only with `CONST64'."));
      return FALSE;
    }
  /* If both args are constants, then create a single experssion ((exp)<<32)|exp1. */
  if (num_args == 2 && size == 8 && exp.X_op == O_constant && exp1.X_op == O_constant)
    {
      exp.X_add_number = ((exp.X_add_number << 32) | exp1.X_add_number);
      exp1.X_op = O_absent;
      exp1.X_add_number = 0;
    }

  /* Add a literal for the expression. */
  litcurptr = hexagon_add_to_lit_pool (&exp, &exp1, size);

  /* Replace original "insn" with a GP-relative load from the literal. */
  if (litcurptr)
    snprintf (new_str, HEXAGON_MAPPED_LEN, "%.*s = mem%c (#%s)",
              rm_left [0].rm_eo - rm_left [0].rm_so, str,
              size == 4? 'w' : 'd', litcurptr->name);
  else
    return FALSE;

  return TRUE;
}

/** This routine is called for each instruction to be assembled.
*/
void
md_assemble
(char *str)
{
  hexagon_packet_insn insn, prefix;

  hexagon_insn_init (&insn);
  hexagon_insn_init (&prefix);

  if (hexagon_assemble (&insn, &prefix, str, FALSE))
    {
      if (!hexagon_queue_insert (&hexagon_aqueue, &insn, &prefix))
        {
          int in_packet = hexagon_in_packet;

          if (hexagon_packet_form (&hexagon_apacket, &hexagon_aqueue))
            {
              hexagon_packet_end (&hexagon_apacket);
              hexagon_packet_write (&hexagon_apacket);
            }
          else
            as_bad (_("too many instructions in packet."));

          hexagon_in_packet = in_packet;
          hexagon_queue_insert (&hexagon_aqueue, &insn, &prefix);
        }

      if (!hexagon_in_packet)
        if (hexagon_packet_form (&hexagon_apacket, &hexagon_aqueue))
          {
            hexagon_packet_end (&hexagon_apacket);
            hexagon_packet_write (&hexagon_apacket);
          }
    }
}

/** This routine is called for each instruction to be assembled.
*/
int
hexagon_assemble
(hexagon_packet_insn *ainsn, hexagon_packet_insn *aprefix, char *str, int pair)
{
  const hexagon_opcode *opcode;
  char *start, *syn;
  int is_id;
  char *mapped;
  long op_val;
  char *op_str;
  hexagon_operand_arg op_args [MAX_OPERANDS];
  size_t op_ndx;
  char *errmsg = NULL;

  /* Opcode table initialization is deferred until here because we have to
     wait for a possible .option command. */
  hexagon_init (hexagon_mach_type);

  /* Skip leading white space.  */
  while (ISSPACE (*str))
    str++;

  /* Special handling of GP-related syntax:
     - Rd32 = CONST32(#imm) or Rd32 = CONST32(label)
     - Rdd32 = CONST64(#imm) or Rdd32 = CONST64(label) */
  mapped = alloca (strlen (str) + HEXAGON_MAPPED_LEN);
  if (hexagon_gp_const_lookup (str, mapped))
    str = mapped;

  /* Keep looking until we find a match.  */
  start = str;
  ainsn->source = NULL;
  for (opcode = hexagon_opcode_lookup_asm (str);
       opcode;
       opcode = HEXAGON_CODE_NEXT_ASM (opcode))
    {
      if (ainsn->source)
        {
          free (ainsn->source);
          ainsn->source = NULL;
        }

      /* Is this opcode supported by the selected cpu?  */
      if (!hexagon_opcode_supported (opcode))
	continue;

      /* Initialize the tentative insn. */
      hexagon_insn_init (aprefix);
      hexagon_insn_init (ainsn);
      ainsn->opcode = opcode;
      ainsn->source = strdup (start);
      /* Yank the packet termination out. */
      if ((str = strchr (ainsn->source, PACKET_END)))
        *str = 0;

      /* Scan the syntax string.  If it doesn't match, try the next one.  */
      ainsn->insn = hexagon_encode_opcode (ainsn->opcode->enc);
      is_id = 0;
      op_ndx = 0;

      for (str = start, syn = ainsn->opcode->syntax;
           *str && *syn; )
        {
          const hexagon_operand *operand;
          ptrdiff_t op_len;

          if ((ISUPPER (*syn) && (*syn != 'I')) || *syn == '#')
	    {
	      /* We have an operand */
	      if ((operand = hexagon_lookup_operand (syn)))
	        {
		  size_t len = strlen (operand->fmt);

                  op_str = str;

                  if (operand->flags & HEXAGON_OPERAND_IS_CONSTANT)
                    goto NEXT_SYN;
		  else if (operand->flags & HEXAGON_OPERAND_IS_IMMEDIATE)
		    {
		      str = hexagon_parse_immediate
			      (ainsn, aprefix, operand, op_str, &op_val,
                               !pair? &errmsg: NULL);
		      if (!str)
			goto NEXT_OPCODE;
                    }
                  else
                    {
                      int is_op;

                      str = operand->parse (operand, &ainsn->insn, ainsn->opcode,
                                            str, &op_val, &is_op,
                                            !pair? &errmsg: NULL);
                      if (!str)
                        /* This instruction doesn't match the syntax */
                        goto NEXT_OPCODE;

                      if (is_op & HEXAGON_OPERAND_IS_INVALID)
                        goto END_OPCODE;
                      else if (is_op & HEXAGON_OPERAND_IS_PREDICATE
                               && (ainsn->opcode->attributes & CONDITIONAL_EXEC))
                        {
                          ainsn->conditional = (ainsn->opcode->attributes
                                                & (CONDITIONAL_EXEC
                                                   | CONDITION_SENSE_INVERTED
                                                   | CONDITION_DOTNEW));
                          ainsn->predicate  = op_val;
                        }
                      else if (is_op & HEXAGON_OPERAND_IS_RNEW)
                        {
                          ainsn->flags    |= HEXAGON_INSN_IN_RNEW;
                          ainsn->ioperand  = operand;
                          ainsn->ireg      = op_val;
                        }
                    }

                  /* Store the operand value in case the insn is an alias. */
                  assert (op_ndx < MAX_OPERANDS);
                  assert (str);

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
          char *file;
          unsigned lineno;

          if (!hexagon_check_operand_args (op_args, op_ndx))
            goto NEXT_OPCODE;

          if (ainsn->opcode->map)
            {
              size_t l;

              l = strlen (str) + HEXAGON_MAPPED_LEN;
              mapped = alloca (l);

              /* Remap insn. */
              ((hexagon_mapping) ainsn->opcode->map) (mapped, l, op_args);

              /* Concatenate the rest of the original insn
                 before assembling the remapped insn. */
              strncat (mapped, str, l - strlen (mapped) - 1);
              return (hexagon_assemble (ainsn, aprefix, mapped, pair));
            }

          as_where (&file, &lineno);

          ainsn->used   = FALSE;
          ainsn->pad    = FALSE;
          ainsn->lineno = lineno;

          while (ISSPACE (*str))
            ++str;

          /* Check for the packet end string. */
          if (*str == PACKET_END)
            {
              str++;

              if (!hexagon_in_packet)
                {
                  as_warn (_("found `%c' before opening a packet."), PACKET_END);
                  return TRUE;
                }
              else
                hexagon_in_packet = FALSE;

              while (ISSPACE (*str))
                ++str;

              /* Check for the end inner/outer modifiers.
                 Note that they can appear in any order. */
              while (TRUE)
                {
                  if (!strncasecmp (str, PACKET_END_INNER,
                                    strlen (PACKET_END_INNER)))
                    {
                      ainsn->is_inner = TRUE;
                      str += strlen (PACKET_END_INNER);

                      while (ISSPACE (*str))
                        ++str;
                    }
                  else if (!strncasecmp (str, PACKET_END_OUTER,
			                 strlen (PACKET_END_OUTER)))
                    {
                      ainsn->is_outer = TRUE;
                      str += strlen (PACKET_END_OUTER);

                      while (ISSPACE (*str))
                        ++str;
                    }
                  else
                    break;
                }

              if (!is_end_of_line [(unsigned char) *str])
                {
                  if (!pair)
                    as_bad (_("extra symbols at end of line: `%s'"), str);
                  else
                    return FALSE;
                }
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

      /* Stop trying. */
      END_OPCODE:
      break;
    }

  if (!pair)
    {
      int big = strlen (start) > MAX_MESSAGE - 3;

      if (errmsg)
        as_bad ("%s\n", errmsg);
      as_bad (_("invalid instruction `%.*s%s'"),
              big? MAX_MESSAGE - 3: MAX_MESSAGE, start, big? "...": "");
    }

  return FALSE;
}

int
hexagon_assemble_pair
(hexagon_packet_pair *apair, hexagon_packet_insn *ainsn, hexagon_packet_insn *aprefix)
{
  char *pair, *unpair;
  int is_duplex, are_stores;
  size_t l;

  /* Do nothing if pairing disabled. */
  if (!hexagon_pairing)
    return FALSE;

  /* Skip certain insns. */
  if (/* Packed or duplex insns. */
      ((apair->left.insn.opcode->attributes & PACKED)
        || (apair->left.insn.opcode->flags & HEXAGON_CODE_IS_DUPLEX)
        || (apair->right.insn.opcode->attributes & PACKED)
        || (apair->right.insn.opcode->flags & HEXAGON_CODE_IS_DUPLEX))
      /* Prefix insns. */
      || ((apair->left.insn.opcode->flags & HEXAGON_CODE_IS_PREFIX)
          || (apair->right.insn.opcode->flags & HEXAGON_CODE_IS_PREFIX))
      /* Both extended insns. */
      || ((apair->left.insn.flags & HEXAGON_INSN_IS_KXED)
          && (apair->right.insn.flags & HEXAGON_INSN_IS_KXED))
      /* Both insns have symbolic references. */
      || (apair->left.insn.fc && apair->right.insn.fc)
      /* Selectively, direct branch insns. */
      || (!hexagon_pairing_branch
          && ((apair->left.insn.opcode->flags & HEXAGON_CODE_IS_BRANCH)
              || (apair->right.insn.opcode->flags & HEXAGON_CODE_IS_BRANCH)))
      /* Selectively, multiple memory operations. */
      || (hexagon_no_dual_memory
          && (apair->left.insn.opcode->flags & HEXAGON_CODE_IS_MEMORY)
          && (apair->right.insn.opcode->flags & HEXAGON_CODE_IS_MEMORY)))
    return FALSE;

  /* Memory operations must be paired in source order. */
  are_stores = (apair->left.insn.opcode->flags & HEXAGON_CODE_IS_STORE)
                && (apair->right.insn.opcode->flags & HEXAGON_CODE_IS_STORE);

  /* Create a pair and its mirror. */
  l = strlen (apair->left.insn.source) + strlen (apair->right.insn.source)
      + sizeof (PACKET_PAIR) + 1;
  pair = alloca (l);
  unpair = alloca (l);

  snprintf (pair, l, "%s%c%s",
            apair->left.insn.source, PACKET_PAIR, apair->right.insn.source);
  snprintf (unpair, l, "%s%c%s",
            apair->right.insn.source, PACKET_PAIR, apair->left.insn.source);

  if (hexagon_assemble (ainsn, aprefix, pair, TRUE)
      || (!are_stores && hexagon_assemble (ainsn, aprefix, unpair, TRUE)))
    {
      is_duplex = (ainsn->opcode->flags & HEXAGON_CODE_IS_DUPLEX)? 1: 0;

      /* Abandon if the result is an unwanted duplex. */
      if (!hexagon_pairing_duplex && is_duplex)
        return FALSE;

      ainsn->used   = FALSE;
      ainsn->pad    = FALSE;
      ainsn->lineno = apair->left.insn.lineno;
      ainsn->flags |= HEXAGON_INSN_IS_PAIR;

      return TRUE;
    }

  return FALSE;
}

void
hexagon_common
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
      if (hexagon_if_arch_v1 () || (size_t) size > hexagon_gp_size)
        {
          /* Allocate in .bss. */
          new_sec = bss_section;
          new_subsec = 0;
        }
      else
        {
          /* Allocate in .sbss. */
          new_sec =
            hexagon_create_sbss_section (SMALL_BSS_SECTION,
                                       SEC_ALLOC | SEC_DATA | SEC_SMALL_DATA,
                                       hexagon_sort_sda? access: 0);
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
      if (hexagon_if_arch_v1 () || (size_t) size > hexagon_gp_size)
        new_sec = bfd_com_section_ptr;
      else
        {
          new_sec =
            hexagon_create_scom_section (SMALL_COM_SECTION,
                                       SEC_ALLOC | SEC_IS_COMMON
                                       | SEC_DATA | SEC_SMALL_DATA,
                                       hexagon_sort_sda? access: 0);
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
hexagon_check_operand_args
(const hexagon_operand_arg args [], size_t n)
{
  size_t i, j;

  for (i = 0; i < n; i++)
    for (j = i + 1; j < n; j++)
      {
        if (args [i].operand->bits
            && args [i].operand->bits == args [j].operand->bits
            && args [i].operand->enc_letter == args [j].operand->enc_letter)
          if (args [i].value != args [j].value)
            return FALSE;
      }

  return TRUE;
}

/* Select the cpu we're assembling for.  */
void
hexagon_option
(int ignore ATTRIBUTE_UNUSED)
{
  int mach;
  char c;
  char *cpu;

  cpu = input_line_pointer;
  c = get_symbol_end ();
  mach = hexagon_get_mach (cpu);
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
  else if (mach_type_specified_p && mach != hexagon_mach_type)
    {
      as_bad (_("architecture specified for \".option\" conflicts with current setting."));
      ignore_rest_of_line ();
      return;
    }
  else
    {
      /* The cpu may have been selected on the command line.  */
      if (mach != hexagon_mach_type)
	as_warn (_("\".option\" directive overrides default or command-line setting."));
      hexagon_mach_type = mach;

      if (!bfd_set_arch_mach (stdoutput, bfd_arch_hexagon, mach))
	as_fatal (_("assembler internal error."));

      mach_type_specified_p = TRUE;
    }

  demand_empty_rest_of_line ();
  return;
}

char *
md_atof
(int type, char *litP, int *sizeP)
{
  return (ieee_md_atof (type, litP, sizeP, target_big_endian));
}

/* Round up a section size to the appropriate boundary.  */
valueT
md_section_align
(segT segment ATTRIBUTE_UNUSED, valueT size)
{
  return (size);
}

void
hexagon_code_symbol
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
  if (name [0] == GLOBAL_OFFSET_TABLE_NAME [0]
      && name [1] == GLOBAL_OFFSET_TABLE_NAME [1]
      && name [2] == GLOBAL_OFFSET_TABLE_NAME [2]
      && !strcmp (name, GLOBAL_OFFSET_TABLE_NAME))
    {
      if (!hexagon_got_symbol)
	{
	  if (symbol_find (name))
	    as_bad (_("GOT already in symbol table."));
	  hexagon_got_symbol
	    = symbol_new (name, undefined_section, 0, &zero_address_frag);
	};
      return (hexagon_got_symbol);
    }

  return NULL;
}

/* Functions concerning expressions.  */

/* Parse a .byte, .word, etc. expression.

   Values for the status register are specified with %st(label).
   `label' will be right shifted by 2.  */

void
hexagon_parse_cons_expression
(expressionS *exp, unsigned int nbytes ATTRIBUTE_UNUSED)
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
      hexagon_code_symbol (exp);
      input_line_pointer = p;
    }
}

/* FIXME: i386 does this quite differently. */

/* Record a fixup for a cons expression.  */

/* cons_fix_new is called via the expression parsing code when a relocation
   is needed.  This hook is used to get the correct GOT relocation.  */
/* FIXME: May need this.
static enum bfd_reloc_code_real got_reloc = NO_RELOC;
*/

bfd_reloc_code_real_type
hexagon_got_frag
(fragS *frag ATTRIBUTE_UNUSED, int where ATTRIBUTE_UNUSED, int nbytes, expressionS *exp)
{
  bfd_reloc_code_real_type r_type = NO_RELOC;

  if ((exp->X_op == O_subtract)
      && (exp->X_add_symbol == hexagon_got_symbol))
    {
      if (nbytes == 4)
        r_type = BFD_RELOC_32_PCREL;
    }

  return (r_type);
}

/* Parse operands of the form "<symbol>@<suffix>+<nnn>" and other PIC or TLS
   references.  If one is found, set up the correct relocation and
   copy the input string, minus the "@..." into an allocated buffer for
   parsing by the calling routine.  Return this buffer set it to the length
   of the string that was removed from the input line.  Otherwise return NULL.  */

static char *
hexagon_parse_suffix
(hexagon_suffix_type *type, char **extra)
{
  static const struct
    {
      const char *str;
      hexagon_suffix_type type;
    }
  pic [] =
    {
      { "GOTREL", PIC_GOTREL },
      { "GOTOFF", PIC_GOTREL },
      { "GOT",    PIC_GOT },
      { "PLT",    PIC_PLT },
      { "GDGOT",  TLS_GD_GOT },
      { "IEGOT",  TLS_IE_GOT }, /* This must precede "IE". */
      { "IE",     TLS_IE },
      { "DTPREL", TLS_DTPREL },
      { "TPREL",  TLS_TPREL },
    };
  char *cp;
  size_t j;

  *type = SUF_NONE;

  for (cp = input_line_pointer; *cp != '@'; cp++)
    if (is_end_of_line [(unsigned char) *cp])
      return NULL;

  for (j = 0; j < ARRAY_SIZE (pic); j++)
    {
      size_t len;

      len = strlen (pic [j].str);
      if (!(strncasecmp (cp + 1, pic [j].str, len)))
	{
          ptrdiff_t before, after;
          char *tmp, *past;

          /* The length of the before part of our input line.  */
          before = cp - input_line_pointer;

          /* The after part goes from after the relocation token until
             (and including) an end_of_line char.  Don't use strlen
             here as the end_of_line char may not be a NUL.  */
          past = cp + 1 + len;
          for (cp = past; !is_end_of_line [(unsigned char) *cp]; cp++)
            ;
          after = cp - past;

          /* Allocate and form string.  */
          tmp = xmalloc (before + after + 2);

          strncpy (tmp, input_line_pointer, before);
          /* Replace the '@' with ' ', so that errors like "foo@GOTOFF1"
             will be detected.  */
          tmp [before] = ' ';
          strncpy (tmp + before + 1, past, after);
          tmp [before + after + 1] = '\0';

          *type = pic [j].type;
          if (extra)
            *extra = past;

          return (tmp);
	}
    }

  /* Might be a symbol version string.  Don't as_bad here.  */
  return NULL;
}

void
hexagon_cons_fix_new
(fragS *frag, int where, int nbytes, expressionS *exp)
{
  bfd_reloc_code_real_type r_type;

  if ((r_type = hexagon_got_frag (frag, where, nbytes, exp)) == NO_RELOC)
    {
      switch (nbytes)
      {
        case 1:
          r_type = BFD_RELOC_8;
          break;

        case 2:
          r_type = BFD_RELOC_16;
          break;

        case 4:
	  {
	    expressionS tmp;
	    char *save, *suffix_line;
	    hexagon_suffix_type suffix;

            r_type = BFD_RELOC_32;

	    save = input_line_pointer;
	    if (!(strncmp (save, "@GOT", 4))
	        || !(strncmp (save, "@GDGOT", 6))
	        || !(strncmp (save, "@IE",    3))
	        || !(strncmp (save, "@IEGOT", 6))
	        || !(strncmp (save, "@TPREL", 6)))
              {
                /* Handle GOT and PLT expressions. */
                suffix_line = hexagon_parse_suffix (&suffix, NULL);
                if (suffix_line)
                  {
                    input_line_pointer = suffix_line;
                    expression (&tmp);
                    exp->X_add_number += tmp.X_add_number;

                    if (suffix == PIC_GOTREL)
                      r_type = BFD_RELOC_32_GOTOFF;
                    else if (suffix == PIC_GOT)
                      r_type = BFD_RELOC_HEX_GOT_32;
                    else if (suffix == TLS_GD_GOT)
                      r_type = BFD_RELOC_HEX_GD_GOT_32;
                    else if (suffix == TLS_IE)
                      r_type = BFD_RELOC_HEX_IE_32;
                    else if (suffix == TLS_IE_GOT)
                      r_type = BFD_RELOC_HEX_IE_GOT_32;
                    else if (suffix == TLS_DTPREL)
                      r_type = BFD_RELOC_HEX_DTPREL_32;
                    else if (suffix == TLS_TPREL)
                      r_type = BFD_RELOC_HEX_TPREL_32;
                  }

                while (!is_end_of_line [(unsigned char) *save++])
                  ;
                save--;
              }
	    input_line_pointer = save;
	  }
          break;

        case 8:
          r_type = BFD_RELOC_64;
          break;

        default:
          as_bad (_("unsupported BFD relocation size %u"), nbytes);
          r_type = BFD_RELOC_32;
          break;
      }
    }

  fix_new_exp (frag_now, where, (int) nbytes, exp, 0, r_type);
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
  const hexagon_operand *operand;
  valueT value = *valP;

  /* On a 64-bit host, silently truncate to 32 bits for
     consistency with the behaviour on 32-bit hosts.  Remember value
     for emit_reloc. */
  value &= 0xffffffff;
  value ^= 0x80000000;
  value -= 0x80000000;
  fixP->fx_addnumber = *valP = value;

  /* Same treatment again. */
  fixP->fx_offset &= 0xffffffff;
  fixP->fx_offset ^= 0x80000000;
  fixP->fx_offset -= 0x80000000;

  if (!fixP->fx_addsy)
    fixP->fx_done = 1;
  else
    {
      if (fixP->fx_pcrel)
	{
	  /* Symbol is PC-relative. */
	  switch (fixP->fx_r_type)
	    {
	    case BFD_RELOC_32:
	      fixP->fx_r_type = BFD_RELOC_32_PCREL;
	      break;
	    default:
	      break;
	    }

	  /* Hack around bfd_install_relocation brain damage.  */
	  if (S_GET_SEGMENT (fixP->fx_addsy) != seg)
	    value += md_pcrel_from (fixP);
	  else if (fixP->fx_r_type != BFD_RELOC_HEX_PLT_B22_PCREL)
	    fixP->fx_done = 1;
	}
      else
	switch (fixP->fx_r_type)
	  {
	  case BFD_RELOC_HEX_GD_GOT_LO16:
	  case BFD_RELOC_HEX_GD_GOT_HI16:
	  case BFD_RELOC_HEX_GD_GOT_32:
	  case BFD_RELOC_HEX_GD_GOT_16:
	  case BFD_RELOC_HEX_IE_LO16:
	  case BFD_RELOC_HEX_IE_HI16:
	  case BFD_RELOC_HEX_IE_32:
	  case BFD_RELOC_HEX_IE_GOT_LO16:
	  case BFD_RELOC_HEX_IE_GOT_HI16:
	  case BFD_RELOC_HEX_IE_GOT_32:
	  case BFD_RELOC_HEX_IE_GOT_16:
	  case BFD_RELOC_HEX_DTPREL_LO16:
	  case BFD_RELOC_HEX_DTPREL_HI16:
	  case BFD_RELOC_HEX_DTPREL_32:
	  case BFD_RELOC_HEX_DTPREL_16:
	  case BFD_RELOC_HEX_TPREL_LO16:
	  case BFD_RELOC_HEX_TPREL_HI16:
	  case BFD_RELOC_HEX_TPREL_32:
	  case BFD_RELOC_HEX_TPREL_16:
	    /* Implicitly set type to TLS. */
	    S_SET_THREAD_LOCAL (fixP->fx_addsy);
	    break;

	  default:
	    break;
	  }
    }

  /* We can't actually support subtracting a symbol.  */
  if (fixP->fx_subsy != NULL)
    as_bad_where (fixP->fx_file, fixP->fx_line, _("expression too complex."));

  operand = (hexagon_operand *) fixP->tc_fix_data;
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

          hexagon_insert_operand (fixP->fx_frag->fr_literal + fixP->fx_where,
                                operand, (offsetT) value, fixP);
        }
      else
        /* If any relocation is to be applied later, don't modify
          the section contents here; we will use the addend in
          the relocation to fix it up at link time... */
        hexagon_insert_operand (fixP->fx_frag->fr_literal + fixP->fx_where,
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
                            _("unknown relocation for symbol `%s'."),
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
    as_fatal (fixP->fx_file, fixP->fx_line,
              _("cannot export relocation type \"%s\"."),
              fixP->fx_r_type,
              bfd_get_reloc_code_name (fixP->fx_r_type));
    return NULL;
  }

  assert (fixP->fx_pcrel == reloc->howto->pc_relative);

  /* fixP->fx_offset was not handled in md_apply_fix() so we
     need to propagate it into the (RELA) relocation */
  reloc->addend = fixP->fx_offset;

  return (reloc);
}

/** Look for insns in the packet which restricts slot #1.

@return The insn index in the packet, otherwise -1.
*/
int
hexagon_find_noslot1
(const hexagon_packet *packet, size_t current)
{
  size_t i;

  for (i = 0; i < hexagon_packet_count (packet); i++)
    if (i != current && !packet->insns [i].used )
      if (packet->insns [i].opcode->attributes & A_RESTRICT_NOSLOT1)
	return (i);

  return (-1);
}

/** Discard specified number of DCFETCH.

It must not be called after packet has been written out.

@param number # DCFETCH to discard, with "0" meaning all but one.
@return # replacements performed.
 */
int
hexagon_discard_dcfetch
(hexagon_packet *apacket, int number)
{
  int count = 0, found = 0;
  size_t i;

  if (!hexagon_if_arch_v1 () && hexagon_packet_count (apacket) > 1)
    {
      if (!number)
        /* Discard all but one DCFETCH instruction. */
        for (i = 0; i < hexagon_packet_count (apacket); i++)
          {
            if (!strncasecmp (apacket->insns [i].opcode->syntax,
                              HEXAGON_DCFETCH, HEXAGON_DCFETCH_LEN))
              {
                found++;

                if (found > MAX_DCFETCH)
                  as_warn (_("more than one `dcfetch' instruction in packet."));

                if (found > 1)
                  /* Delete extra DCFETCH. */
                  {
                    as_warn_where (NULL, apacket->insns [i].lineno,
                                  _("extra `dcfetch' removed."));

                    apacket->insns [i]        = hexagon_nop_insn;
                    apacket->insns [i].pad = TRUE;

                    count++;
                  }
              }
          }
      else
        /* Discard specified number of DCFETCH. */
        for (i = 0; number && i < hexagon_packet_count (apacket); i++)
          if (!strncasecmp (apacket->insns [i].opcode->syntax,
                            HEXAGON_DCFETCH, HEXAGON_DCFETCH_LEN))
            {
              found++;

              as_warn_where (NULL, apacket->insns [i].lineno,
                            _("`dcfetch' removed."));

              apacket->insns [i]        = hexagon_nop_insn;
              apacket->insns [i].pad = TRUE;

              number--;
              count++;
            }
    }

  return (count);
}

/** Shuffle packet accoding to from-to mapping.

@param packet reference.
@param fromto mapping array.
*/
void
hexagon_shuffle_packet
(hexagon_packet *packet, size_t *fromto)
{
  hexagon_packet_insn insns [MAX_PACKET_INSNS], prefixes [MAX_PACKET_INSNS];
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
          insns [i].fix->fx_where   = i * HEXAGON_INSN_LEN;
          insns [i].fix->fx_offset += insns [i].fix->fx_pcrel
                                      ? (i - from) * HEXAGON_INSN_LEN
                                      : 0;
        }
      if (prefixes [i].fc && prefixes [i].fix)
        {
          prefixes [i].fix->fx_where   = i * HEXAGON_INSN_LEN;
          prefixes [i].fix->fx_offset += prefixes [i].fix->fx_pcrel
                                         ? (i - from) * HEXAGON_INSN_LEN
                                         : 0;
        }

      insns [i].ndx = ndx;
      ndx += packet->insns [from].pad? 0: 1;
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
hexagon_shuffle_helper
(hexagon_packet *packet, size_t slot_num, size_t *shuffle)
{
  size_t ndx, slots, temp_mask, store_mask;
  size_t *fromto, aux [MAX_PACKET_INSNS];
  hexagon_packet_insn *inew = NULL;
  int single, prefix, rnew, store, nostore;
  int changed;
  size_t i;

  if (slot_num >= hexagon_packet_count (packet))
    return TRUE;

  /* Don't touch a packet with prefixes. */
  if ((prefix = hexagon_has_prefix (packet)))
    return FALSE;

  single  = hexagon_has_single (packet);
  store   = hexagon_has_store (packet);
  nostore = hexagon_has_store_not (packet);

  rnew = hexagon_has_rnew (packet, &inew);

  ndx       = MAX_PACKET_INSNS - slot_num - 1;
  slots = 1 << ndx;

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

  store_mask = HEXAGON_SLOTS_STORES;
  for (i = 0, changed = FALSE;
       i < hexagon_packet_count (packet) && !changed;
       i++)
    {
      temp_mask = packet->insns [i].opcode->slots;

      /* If there is only one memory insn, it requires ndx #0. */
      if (single
	  && (packet->insns [i].opcode->attributes & A_RESTRICT_SINGLE_MEM_FIRST))
	temp_mask &= HEXAGON_SLOTS_MEM1;

      /* If there is a store restriction, make sure that none makes into slot #1. */
      if (nostore
          && !(packet->insns [i].opcode->attributes & A_RESTRICT_NOSLOT1_STORE)
          && (packet->insns [i].opcode->attributes & A_STORE))
        temp_mask &= ~HEXAGON_SLOTS_STORES;

      /* Make sure that several stores follow source order. */
      if ((packet->insns [i].opcode->attributes & A_STORE))
        {
          if (store > 1)
            {
              temp_mask &= store_mask;
              store_mask >>= 1;
            }
          else
            temp_mask &= HEXAGON_SLOTS_MEM1;
        }

      if (!packet->insns [i].used && (temp_mask & slots))
	{
	  /* Check for NOSLOT1 restriction. */
	  if (ndx == 1
              && !hexagon_is_nop (packet->insns [i].insn)
              && hexagon_find_noslot1 (packet, i) >= 0)
            /* If so, then skip this ndx. */
            continue;

          /* Allocate this slot. */
	  fromto [slot_num] = i;
	  packet->insns [i].used = TRUE;

          /* Try to shuffle remaining slots. */
	  if (!(changed = hexagon_shuffle_helper (packet, slot_num + 1, fromto)))
            /* Deallocate ndx if unsuccessful. */
            packet->insns [i].used = FALSE;
        }
    }

  /* Shuffle packet if successful and at the top-most call. */
  if (changed && !shuffle)
    {
      /* Shuffle the packet according to the from-to map. */
      hexagon_shuffle_packet (packet, fromto);
      /* Finish up the insns in the packet. */
      hexagon_packet_finish (packet);
    }

  return (changed);
}

void
hexagon_shuffle_handle
(hexagon_packet *apacket)
{
  if (!hexagon_shuffle_helper (apacket, 0, NULL))
    /* Try to discard a DCFETCH before trying again. */
    if (!(hexagon_discard_dcfetch (apacket, 1)
          && hexagon_shuffle_helper (apacket, 0, NULL)))
      as_bad (_("invalid instruction packet."));
}

void
hexagon_shuffle_prepare
(hexagon_packet *apacket)
{
  size_t fromto [MAX_PACKET_INSNS];
  unsigned i, j, k;
  int found;

  /* Initialize shuffle map. */
  for (i = 0; i < MAX_PACKET_INSNS; i++)
    fromto [i] = MAX_PACKET_INSNS;

  /* Pad packet with NOPs. */
  while (hexagon_packet_insert (apacket, &hexagon_nop_insn, NULL, NULL, TRUE))
    ;

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
    if (!apacket->insns [i].pad)
      {
        /* Find the highest ndx that this instruction can go to. */
        for (j = 0, found = FALSE;
              j < MAX_PACKET_INSNS && !found;
              j++)
          if ((  apacket->insns [i].opcode->slots
                & (1 << (MAX_PACKET_INSNS - j - 1))))
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

  /* Step 2. */
  for (i = 0; i < MAX_PACKET_INSNS; i++)
    if (apacket->insns [i].pad)
      /* Find the first unused position for this pad insn. */
      for (j = 0; j < MAX_PACKET_INSNS; j++)
        if (fromto [j] >= MAX_PACKET_INSNS)
          {
            fromto [j] = i;
            break;
          }

  hexagon_shuffle_packet (apacket, fromto);
}

/** Main function for packet shuffling.
*/
void
hexagon_shuffle_do
(hexagon_packet *apacket)
{
  hexagon_packet_insn insn;
  char *file;
  int has_prefer_slot0;
  unsigned i;

  /* Get rid of extra insns. */
  hexagon_discard_dcfetch (apacket, 0);
  hexagon_shuffle_prepare (apacket);

  for (i = 0, has_prefer_slot0 = FALSE;
        i < MAX_PACKET_INSNS && !has_prefer_slot0;
        i++)
    if ((apacket->insns [i].opcode->attributes & A_RESTRICT_PREFERSLOT0))
      {
        /* Swap with insn at slot #0. */
        insn = apacket->insns [i];
        apacket->insns [i] = apacket->insns [MAX_PACKET_INSNS - 1];
        apacket->insns [MAX_PACKET_INSNS - 1] = insn;

        has_prefer_slot0 = TRUE;
      }

  hexagon_shuffle_handle (apacket);

  if (has_prefer_slot0)
    {
      as_where (&file, NULL);

      for (i = 0; i < MAX_PACKET_INSNS - 1; i++)
        if ((apacket->insns [i].opcode->attributes & A_RESTRICT_PREFERSLOT0))
            as_warn_where (file, apacket->insns [i].lineno,
                            _("instruction `%s' prefers slot #0, "
                              "but has been assigned to slot #%u."),
                            apacket->insns [i].opcode->syntax, i);
    }
}

/** Finish up packet after shuffling.

Finish encoding any insn in the packet that may depend of shuffling completed.

@param apacket Packet to finish.
*/
void
hexagon_packet_finish
(hexagon_packet *packet)
{
  size_t i;

  /* Finish R.NEW insns. */
  {
    hexagon_packet_insn *inew, *onew;
    size_t off;

    if (hexagon_has_rnew (packet, &inew))
      {
        for (i = 0, off = 0, onew = NULL; i < MAX_PACKET_INSNS; i++)
          if ((packet->insns [i].flags & HEXAGON_INSN_OUT_RNEW)
              && !packet->insns [i].opair
              && packet->insns [i].oreg == inew->ireg
              && (!packet->insns [i].conditional
                  || (packet->insns [i].conditional == inew->conditional
                      && packet->insns [i].predicate == inew->predicate)))
            {
              off = inew->ndx - packet->insns [i].ndx;
              onew = packet->insns + i;
              break;
            }

        if (onew)
          hexagon_encode_operand
            (inew->ioperand, &inew->insn, inew->opcode,
             off * 2 + inew->ireg % 2, NULL, FALSE, FALSE, NULL);
        else
          as_bad_where (NULL, inew->lineno,
                        _("register `r%u' used with `.new' "
                          "but not validly modified in the same packet."),
                        inew->ireg);
      }

  }
}

/** Determine if an instruction is a NOP.

@param insn An insn.
@return True if NOP.
*/

int
hexagon_is_nop
(hexagon_insn insn)
{
  const hexagon_opcode *opcode = hexagon_lookup_insn (insn);

  return (opcode->attributes & A_IT_NOP);
}

/** Determine if an instruction is a prefix.

@param insn An insn.
@return True if a prefix.
*/

int
hexagon_is_prefix
(hexagon_insn insn)
{
  const hexagon_opcode *opcode = hexagon_lookup_insn (insn);

  return (opcode->attributes & A_IT_EXTENDER);
}

hexagon_insn
hexagon_find_insn
(const char *insn)
{
  const hexagon_opcode *opcode = hexagon_opcode_lookup_asm (insn);

  /* Keep looking until we find a match.  */
  for (; opcode != NULL; opcode = HEXAGON_CODE_NEXT_ASM (opcode))
    if (!strncasecmp (opcode->syntax, insn, strlen (insn)))
      break;

  assert (opcode);
  return (hexagon_encode_opcode (opcode->enc));
}

hexagon_insn
hexagon_find_nop
(void)
{
  hexagon_nop_insn.insn   = hexagon_nop;
  hexagon_nop_insn.opcode = hexagon_lookup_insn (hexagon_nop_insn.insn);

  return (hexagon_nop);
}

hexagon_insn
hexagon_find_kext
(void)
{
  if (hexagon_if_arch_kext ())
    {
      hexagon_kext_insn.insn   = hexagon_kext;
      hexagon_kext_insn.opcode = hexagon_lookup_insn (hexagon_kext_insn.insn);

      return (hexagon_kext);
    }
  else
    {
      memset (&hexagon_kext_insn, 0, sizeof (hexagon_kext_insn));
      return FALSE;
    }
}

/** Check if a predicate register with the .new suffix is updated in the packet.

@return True if it is, false otherwise.
*/
int
hexagon_check_new_predicate
(void)
{
  size_t i;

  /* No need to check for V1, since P.new read array will never be set. */
  for (i = 0; i < HEXAGON_NUM_PREDICATE_REGS; i++)
    if (pNewArray [i].used && (!pArray [i].used || pLateArray [i].used))
      {
        if (cArray [HEXAGON_P30].used)
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
hexagon_check_implicit_predicate
(const hexagon_opcode *opcode, unsigned int implicit, int reg)
{
  int is_used;

  if (opcode->implicit & implicit)
    {
      is_used = !hexagon_autoand || (opcode->attributes & A_RESTRICT_LATEPRED)
                ? TRUE: MAYBE;

      if (pArray [reg].used == TRUE || (pArray [reg].used == MAYBE && is_used == TRUE))
        as_bad (_("register `p%d' modified more than once."), reg);

      pArray [reg].used = TRUER (pArray [reg].used, is_used);
    }
}

void
hexagon_check_predicate
(int reg_num, const hexagon_opcode *opcode)
{
  int used;

  used =  (!hexagon_autoand || (opcode->attributes & A_RESTRICT_LATEPRED))
          ? TRUE: MAYBE;

  if (pArray [reg_num].used == TRUE || pArray [reg_num].used > used)
    as_bad (_("register `p%d' modified more than once."), reg_num);
  else
    pArray [reg_num].used = used;;
}

void
hexagon_check_implicit
(const hexagon_opcode *opcode, unsigned int implicit, int reg,
 hexagon_reg_score *array, const char *name)
{
  if (opcode->implicit & implicit)
    {
      if (array [reg].used)
        as_bad (_("register `%s' modified more than once."), name);
      else
        array [reg].used++;
    }
}

void
hexagon_check_register
(hexagon_reg_score *array, int reg_num, const char *name, int pred_reg_rd_mask,
 const hexagon_operand *operand, hexagon_packet_insn *insn, size_t n)
{
  char *errmsg = NULL;
  char *buff;
  char *reg_name;
  int prev_pred_reg_rd_mask;
  int a_pred_mask, prev_a_pred_mask;
  int mult_wr_err = FALSE;

  if (array [reg_num].used)
    {
      prev_pred_reg_rd_mask = array [reg_num].pred;

      if (!pred_reg_rd_mask || !prev_pred_reg_rd_mask)
        {
          if ((operand
               && (array [reg_num].letter != operand->enc_letter
                   || (operand->flags
                       & (HEXAGON_OPERAND_IS_READ | HEXAGON_OPERAND_IS_WRITE))
                      != (HEXAGON_OPERAND_IS_READ | HEXAGON_OPERAND_IS_WRITE)))
              || array [reg_num].ndx != n)
            mult_wr_err = TRUE;
        }
      else
        {
          size_t i;

          // Or same non-zero mask for same predicate
          for (i = 0; i < HEXAGON_NUM_PREDICATE_REGS; i++)
            {
              a_pred_mask = HEXAGON_PRED_GET (pred_reg_rd_mask, i);
              prev_a_pred_mask = HEXAGON_PRED_GET (prev_pred_reg_rd_mask, i);
              if (a_pred_mask && a_pred_mask == prev_a_pred_mask)
                {
                  mult_wr_err = TRUE;
                  break;
                }
            }
        }

      if (mult_wr_err)
        {
          if (operand)
            {
              buff = alloca (HEXAGON_MAPPED_LEN);

              reg_name
                = hexagon_dis_operand (operand, insn->insn, 0, 0,
                                     insn->opcode->enc, buff, &errmsg);
            }
          else
            reg_name = buff = (char *) name;

          if (reg_name)
            as_bad (_("register `%s' modified more than once."), buff);
          else if (errmsg)
            as_bad ("%s\n", errmsg);
        }
      else
        {
          array [reg_num].used    = TRUE;
          array [reg_num].letter  = operand? operand->enc_letter: 0;
          array [reg_num].pred   |= pred_reg_rd_mask;
          array [reg_num].ndx     = n;
        }
    }
  else
    {
      array [reg_num].used   = TRUE;
      array [reg_num].letter = operand? operand->enc_letter: 0;
      array [reg_num].pred   = pred_reg_rd_mask;
      array [reg_num].ndx    = n;
    }
}

void
hexagon_packet_check
(hexagon_packet *apacket)
{
  size_t i;
  char *errmsg;
  int reg_num;
  hexagon_packet_insn *ainsn, *binsn;
  hexagon_reg_score *arrayPtr;
  char *cp;
  int pred_reg_rd_mask;

  /* Initialize score-boards. */
  {
    numOfOvf = 0;

    numOfBranchAddr  = numOfBranchAddrMax1 = 0;
    numOfBranchMax1  = numOfBranchStiff    = 0;
    numOfBranchRelax = numOfBranchRelax2nd = numOfBranchAddrRelax = 0;

    numOfLoopMax1 = 0;

    memset (gArray,     0, sizeof (gArray));
    memset (cArray,     0, sizeof (cArray));
    memset (sArray,     0, sizeof (sArray));
    memset (pArray,     0, sizeof (pArray));
    memset (guArray,    0, sizeof (guArray));
    memset (pNewArray,  0, sizeof (pNewArray));
    memset (pLateArray, 0, sizeof (pLateArray));
  }

  for (i = 0; i < apacket->size; i++)
    {
      ainsn = apacket->insns + i;
      pred_reg_rd_mask = 0;
      errmsg = NULL;

      /* Check whether the instruction is legal inside the packet,
        but allow single instruction in packet. */
      if ((ainsn->opcode->attributes & A_RESTRICT_NOPACKET)
          && hexagon_packet_insns (apacket) > 1)
        {
          as_bad (_("instruction cannot appear in packet with other instructions."));
          hexagon_in_packet = FALSE;
          continue;
        }

      //Check loop can not exist in the same packet as branch label instructions
      //they are using the same adder
      if ((ainsn->opcode->attributes & A_RESTRICT_BRANCHADDER_MAX1))
        numOfBranchAddrMax1++;

      if ((ainsn->opcode->attributes & A_BRANCHADDER))
        numOfBranchAddr++;

      if ((ainsn->opcode->attributes & A_RESTRICT_COF_MAX1)
          && !(ainsn->opcode->attributes & A_RELAX_COF_1ST)
          && !(ainsn->opcode->attributes & A_RELAX_COF_2ND))
        numOfBranchStiff++;

      if ((ainsn->opcode->attributes & A_RESTRICT_COF_MAX1))
        numOfBranchMax1++;

      if (!(ainsn->opcode->attributes & A_RELAX_COF_1ST)
          && (ainsn->opcode->attributes & A_RELAX_COF_2ND)
          && !numOfBranchRelax)
        numOfBranchRelax2nd++;

      if ((ainsn->opcode->attributes & A_RESTRICT_COF_MAX1)
           && ((ainsn->opcode->attributes & A_RELAX_COF_1ST)
               || (ainsn->opcode->attributes & A_RELAX_COF_2ND)))
        numOfBranchRelax++;

      if ((ainsn->opcode->attributes & A_BRANCHADDER)
           && ((ainsn->opcode->attributes & A_RELAX_COF_1ST)
               || (ainsn->opcode->attributes & A_RELAX_COF_2ND)))
        numOfBranchAddrRelax++;

      if ((ainsn->opcode->attributes & A_RESTRICT_LOOP_LA))
        numOfLoopMax1++;

      if ((ainsn->opcode->attributes & A_RESTRICT_NOSRMOVE))
        numOfOvf = TRUE;

      if ((ainsn->opcode->flags & HEXAGON_CODE_IS_DUPLEX))
        binsn = &apacket->pairs [i].left.insn;
      else
        binsn = ainsn;

      do
        {
          for (cp = binsn->opcode->syntax; *cp; cp++)
            {
              // Walk the syntax string for the opcode
              if (ISUPPER (*cp))
                {
                  // Check for register operand
                  // Get the operand from operand lookup table
                  const hexagon_operand *operand = hexagon_lookup_operand (cp);
                  if (!operand)
                    break;

                  cp += strlen (operand->fmt); // Move the pointer to the end of the operand

                  if ((operand->flags & HEXAGON_OPERAND_IS_READ)
                      && (operand->flags & HEXAGON_OPERAND_IS_PREDICATE))
                    {
                      if (!hexagon_extract_predicate_operand
                          (operand, binsn->insn, binsn->opcode->enc, &reg_num, &errmsg))
                        {
                          if (errmsg)
                            as_bad ("%s\n", errmsg);
                          break;
                        }

                      pred_reg_rd_mask = HEXAGON_PRED_SET (0, reg_num, HEXAGON_PRED_YES);
                      if (binsn->opcode->attributes & CONDITION_SENSE_INVERTED)
                        pred_reg_rd_mask
                          = HEXAGON_PRED_SET (pred_reg_rd_mask, reg_num, HEXAGON_PRED_NOT);
                      if (binsn->opcode->attributes & CONDITION_DOTNEW)
                        {
                          pred_reg_rd_mask
                            = HEXAGON_PRED_SET (pred_reg_rd_mask, reg_num, HEXAGON_PRED_NEW);
                          pNewArray [reg_num].used = TRUE;
                        }
                    }

                  if (operand->flags
                      & (HEXAGON_OPERAND_IS_WRITE | HEXAGON_OPERAND_IS_MODIFIED))
                    {
                      if (operand->flags & HEXAGON_OPERAND_IS_PREDICATE)
                        {
                          if (!hexagon_extract_predicate_operand
                                (operand, binsn->insn, binsn->opcode->enc, &reg_num, &errmsg))
                            {
                              if (errmsg)
                                as_bad ("%s\n", errmsg);
                              break;
                            }

                          if (cArray [HEXAGON_P30].used)
                            {
                              as_bad (_("register `P%d' modified more than once."), reg_num);
                              break;
                            }

                          if (ainsn->opcode->attributes & A_RESTRICT_LATEPRED)
                            pLateArray [reg_num].used++;

                          hexagon_check_predicate (reg_num, ainsn->opcode);
                          if (operand->flags & HEXAGON_OPERAND_IS_PAIR)
                            hexagon_check_predicate (reg_num + 1, ainsn->opcode);

                          continue;
                        }
                      else if (operand->flags & HEXAGON_OPERAND_IS_MODIFIER)
                        {
                          arrayPtr = cArray;

                          if (!hexagon_extract_modifier_operand
                                (operand, binsn->insn, binsn->opcode->enc, &reg_num, &errmsg))
                            {
                              if (errmsg)
                                as_bad ("%s\n", errmsg);
                              break;
                            }
                        }
                      else
                        {
                          if (!hexagon_extract_operand
                                (operand, binsn->insn, 0, binsn->opcode->enc, &reg_num, &errmsg))
                            {
                              if (errmsg)
                                as_bad ("%s\n", errmsg);
                              break;
                            }

                          if (operand->flags & HEXAGON_OPERAND_IS_CONTROL)
                            {
                              if (reg_num == HEXAGON_P30)
                                {
                                  // Set pArray completely
                                  size_t j;

                                  for (j = 0; j < HEXAGON_NUM_PREDICATE_REGS; j++)
                                    if (pArray [j].used)
                                      {
                                        //as_bad (("register `C%d' modified more than once."), reg_num);
                                        cArray [reg_num].used++;
                                        break;
                                      }
                                }
                              arrayPtr = cArray;
                            }
                          else if (operand->flags & HEXAGON_OPERAND_IS_SYSTEM)
                            {
                              arrayPtr = sArray;
                              /* Record which register is changed. */
                              ainsn->oreg = reg_num;
                              ainsn->opair = (operand->flags & HEXAGON_OPERAND_IS_PAIR);
                            }
                          else if (operand->flags & HEXAGON_OPERAND_IS_GUEST)
                            arrayPtr = guArray;
                          else
                            {
                              arrayPtr = gArray;

                              if (!(ainsn->flags & HEXAGON_INSN_OUT_RNEW)
                                  && !(operand->flags & HEXAGON_OPERAND_IS_MODIFIED))
                                {
                                  /* Record which register is changed. */
                                  ainsn->flags |= HEXAGON_INSN_OUT_RNEW;
                                  ainsn->oreg = reg_num;
                                  ainsn->opair = (operand->flags & HEXAGON_OPERAND_IS_PAIR);
                                }
                            }
                        }

                      hexagon_check_register
                        (arrayPtr, reg_num, NULL, pred_reg_rd_mask, operand, binsn, i);
                      if (operand->flags & HEXAGON_OPERAND_IS_PAIR)
                        hexagon_check_register
                          (arrayPtr, reg_num + 1, NULL, pred_reg_rd_mask, operand, binsn, i);
                    }
                }
              if (!*cp)
                break;
            }

          /* Check for implicit operand conflicts. */
          if (binsn->opcode->implicit)
            {
              if (binsn->opcode->implicit & IMPLICIT_PC)
                {
                  /* Look into multiple implicit references to the PC in order
                     to allow slots with two branches. */
                  cArray [HEXAGON_PC].used++;
                  if (cArray [HEXAGON_PC].used > 1
                      && cArray [HEXAGON_PC].used > numOfBranchRelax)
                    hexagon_check_implicit
                      (binsn->opcode, IMPLICIT_PC, HEXAGON_PC, cArray, "c9/pc");
                }

              hexagon_check_implicit
                (binsn->opcode, IMPLICIT_LC0, HEXAGON_LC0, cArray, "c1/lc0");
              hexagon_check_implicit
                (binsn->opcode, IMPLICIT_SA0, HEXAGON_SA0, cArray, "c0/sa0");
              hexagon_check_implicit
                (binsn->opcode, IMPLICIT_LC1, HEXAGON_LC1, cArray, "c3/lc1");
              hexagon_check_implicit
                (binsn->opcode, IMPLICIT_SA1, HEXAGON_SA1, cArray, "c2/sa1");

              hexagon_check_implicit_predicate
                (binsn->opcode, IMPLICIT_P3,  3);
              hexagon_check_implicit_predicate
                (binsn->opcode, IMPLICIT_P0,  0); /* V3 */
              hexagon_check_implicit_predicate
                (binsn->opcode, IMPLICIT_P1,  1); /* V4 */

              if (binsn->opcode->implicit & IMPLICIT_SP)
                hexagon_check_register
                  (gArray, HEXAGON_SP, "r29/sp", pred_reg_rd_mask, NULL, binsn, i);
              if (binsn->opcode->implicit & IMPLICIT_FP)
                hexagon_check_register
                  (gArray, HEXAGON_FP, "r30/fp", pred_reg_rd_mask, NULL, binsn, i);
              if (binsn->opcode->implicit & IMPLICIT_LR)
                hexagon_check_register
                  (gArray, HEXAGON_LR, "r31/lr", pred_reg_rd_mask, NULL, binsn, i);
            }

          /* If a pair, move to the righthand insn. */
          if ((ainsn->opcode->flags & HEXAGON_CODE_IS_DUPLEX)
              && binsn != &apacket->pairs [i].right.insn)
            binsn = &apacket->pairs [i].right.insn;
          else
            break;
        }
      while (TRUE);
    }
}

/** Return the number of slots.

@param apacket A packet reference.
*/
size_t
hexagon_packet_slots
(const hexagon_packet *apacket)
{
  return (apacket->size + apacket->duplex);
}

/** Return the number of words.

@param apacket A packet reference.
*/
size_t
hexagon_packet_length
(const hexagon_packet *apacket)
{
  return (apacket->size + apacket->prefix + apacket->relax);
}

/** Return the size taken up by a packet.

@param apacket A packet reference.
*/
size_t
hexagon_packet_size
(const hexagon_packet *apacket)
{
  return (apacket->size + apacket->duplex + apacket->prefix + apacket->relax);
}

/** Return the number of insns.

@param apacket A packet reference.
*/
size_t
hexagon_packet_count
(const hexagon_packet *apacket)
{
  return (apacket->size);
}

/** Return the number of effective insns.

@param apacket A packet reference.
*/
size_t
hexagon_packet_insns
(const hexagon_packet *apacket)
{
  size_t i, n;

  for (i = n = 0; i < apacket->size; i++)
    if (!apacket->insns [i].pad)
      n++;

  return (n);
}

/* Used to handle packet begin/end syntax */

void
hexagon_packet_begin
(hexagon_packet *apacket)
{
  if (hexagon_in_packet)
    {
      hexagon_packet_form (apacket, &hexagon_aqueue);
      hexagon_packet_end (apacket);
      hexagon_packet_write (apacket);
    }

  hexagon_in_packet = TRUE;
  hexagon_packet_init (apacket);
}

/* Function to write packet header, encode instructions in a packet,
   and perform various assembler restriction checks */

void
hexagon_packet_end
(hexagon_packet *apacket)
{
  int n;

  /* Checking for multiple restrictions in packet. */
  hexagon_packet_check (apacket);

  hexagon_in_packet = FALSE;

  // check for multiple writes to SR (implicit not allowed if explicit writes are present)
  if (numOfOvf && cArray [HEXAGON_SR].used)
    {
      as_bad_where (NULL, apacket->lineno,
                    _("`OVF' bit in `SR' register cannot be set (implicitly or explicitly) more than once in packet."));
      return;
    }

  if (!hexagon_check_new_predicate ())
    return;

  if (apacket->is_inner)
    {
      /* Check whether registers updated by :endloop0 are updated in packet. */
      if (cArray [HEXAGON_P30].used | cArray [HEXAGON_SR].used
          | cArray [HEXAGON_SA0].used | cArray [HEXAGON_LC0].used | cArray [HEXAGON_PC].used)
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
      if (hexagon_has_solo (apacket))
        as_bad (_("packet marked with `:endloop0' cannot contain a solo instruction."));
    }

  if (apacket->is_outer)
    {
      /* Check whether registers updated by :endloop1 are updated in packet. */
      if (cArray [HEXAGON_SA1].used | cArray [HEXAGON_LC1].used | cArray [HEXAGON_PC].used)
        as_bad (_("packet marked with `:endloop1' cannot contain instructions that " \
                  "modify registers `SA1', `LC1' or `PC'."));

      /* Check for a solo instruction in a packet with :endloop1. */
      if (hexagon_has_solo (apacket))
        as_bad (_("packet marked with `:endloop1' cannot contain a solo instruction."));
    }

  if (numOfBranchAddrMax1 && numOfBranchAddr > 1)
    {
      as_bad_where (NULL, apacket->lineno,
                    _("loop setup and direct branch instructions cannot be in same packet."));
      return;
    }

  if ((numOfBranchStiff > 1)
      || (numOfBranchStiff && numOfBranchMax1 > 1)
      || (numOfBranchMax1 > 1 && numOfBranchMax1 > numOfBranchRelax)
      || (numOfBranchAddr > 1 && numOfBranchAddr > numOfBranchAddrRelax))
    {
      as_bad_where (NULL, apacket->lineno,
                    _("too many branches in packet."));
      return;
    }

  if (numOfBranchRelax2nd && numOfBranchRelax > numOfBranchRelax2nd)
    {
      as_bad_where (NULL, apacket->lineno,
                    _("unconditional branch cannot precede another branch in packet."));
      return;
    }

  if (!hexagon_has_but_ax (apacket))
    as_bad_where (NULL, apacket->lineno,
                  _("instruction cannot appear in packet with other than A-type or X-type instructions."));

  n = hexagon_has_mem (apacket);
  if ((hexagon_no_dual_memory && n > 1) || n > 2)
    as_bad_where (NULL, apacket->lineno,
                  _("multiple memory operations in packet."));

  if (hexagon_pairs_info && hexagon_has_pair (apacket))
    as_bad_where (NULL, apacket->lineno,
                  _("instructions paired."));

  hexagon_shuffle_do (apacket);
}

int
hexagon_has_solo
(const hexagon_packet *apacket)
{
  int solo;
  size_t i;

  /* Solo insns can not exist in a packet. */
  for (i = 0, solo = FALSE; i < apacket->size && !solo; i++)
    if (apacket->insns [i].opcode->attributes & A_RESTRICT_NOPACKET)
      solo = TRUE;

  return (solo);
}

/* Called by the assembler parser when it can't recognize a line ...

   At this point, "ch" is the next character to be processed and
   "input_line_pointer" has already been moved past "ch".
 */

int
hexagon_unrecognized_line
(int ch ATTRIBUTE_UNUSED)
{
  char *str = input_line_pointer - 1;

  if (*str == PACKET_BEGIN)
    {
      str++;
      if (hexagon_in_packet)
        as_warn (_("found `%c' inside a packet."), PACKET_BEGIN);

      hexagon_packet_begin (&hexagon_apacket);
      return TRUE;
    }
  else if (*str == PACKET_END)
    {
      str++;
      if (!hexagon_in_packet)
        as_warn (_("found `%c' before opening a packet."), PACKET_END);

      while (ISBLANK (*str))
        str++;

      if (!strncasecmp (str, PACKET_END_INNER, strlen (PACKET_END_INNER)))
        {
          hexagon_aqueue.is_inner = TRUE;
          input_line_pointer = str += strlen (PACKET_END_INNER);
        }

      while (ISBLANK (*str))
        str++;

      if (!strncasecmp (str, PACKET_END_OUTER, strlen (PACKET_END_OUTER)))
        {
          hexagon_aqueue.is_outer = TRUE;
          input_line_pointer = str += strlen (PACKET_END_OUTER);
        }

      while (ISBLANK (*str))
        str++;

      if (!strncasecmp (str, PACKET_END_INNER, strlen (PACKET_END_INNER)))
        {
          hexagon_aqueue.is_inner = TRUE;
          input_line_pointer = str += strlen (PACKET_END_INNER);
        }

      if (hexagon_packet_form (&hexagon_apacket, &hexagon_aqueue))
        {
          hexagon_packet_end (&hexagon_apacket);
          hexagon_packet_write (&hexagon_apacket);
        }
      else
        hexagon_in_packet = FALSE;

      return TRUE;
    }
  else if (!strncasecmp (str, PACKET_END_INNER, strlen (PACKET_END_INNER)))
    {
      as_warn (_("found `%s' when not closing a packet."), PACKET_END_INNER);

      input_line_pointer += strlen (PACKET_END_INNER) - 1;
      return TRUE;
    }
  else if (!strncasecmp (str, PACKET_END_OUTER, strlen (PACKET_END_OUTER)))
    {
      as_warn (_("found `%s' when not closing a packet."), PACKET_END_OUTER);

      input_line_pointer += strlen (PACKET_END_OUTER) - 1;
      return TRUE;
    }

  /* Not a valid line */
  return FALSE;
}

void
hexagon_cleanup
(void)
{
  if (hexagon_in_packet)
    {
      as_warn (_("reached end of file before closing a packet."));

      hexagon_packet_form (&hexagon_apacket, &hexagon_aqueue);
      hexagon_packet_end (&hexagon_apacket);
      hexagon_packet_write (&hexagon_apacket );
    }
}

void
hexagon_statistics
(void)
{
  if (hexagon_falign_info && n_falign [HEXAGON_FALIGN_TOTAL])
    {
      as_warn (_("%u of %u \".falign\" (%u%%) inserted new `nop' instructions."),
                n_falign [HEXAGON_FALIGN_NEED], n_falign [HEXAGON_FALIGN_TOTAL],
                n_falign [HEXAGON_FALIGN_NEED] * 100 / n_falign [HEXAGON_FALIGN_TOTAL]);

      if (n_falign [HEXAGON_FALIGN_NEED])
        as_warn (_("%u of %u \".falign\" (%u%%) inserted new `nop' packets."),
                n_falign [HEXAGON_FALIGN_PACK], n_falign [HEXAGON_FALIGN_NEED],
                n_falign [HEXAGON_FALIGN_PACK] * 100 / n_falign [HEXAGON_FALIGN_NEED]);
    }

  if (hexagon_falign_more && n_falign [HEXAGON_FALIGN_PACK])
    {
      as_warn (_("reasons for \".falign\" inserting new `nop' packets:"));
      as_warn (_("  %u of %u (%u%%) reached a packet \".falign\"."),
               n_falign [HEXAGON_FALIGN_FALIGN], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_FALIGN] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
      as_warn (_("  %u of %u (%u%%) reached a single-instruction \".falign\"."),
               n_falign [HEXAGON_FALIGN_FALIGN1], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_FALIGN1] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
      as_warn (_("  %u of %u (%u%%) in different sections."),
               n_falign [HEXAGON_FALIGN_SECTION], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_SECTION] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
      as_warn (_("  %u of %u (%u%%) reached end of history."),
               n_falign [HEXAGON_FALIGN_END], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_END] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
      as_warn (_("  %u of %u (%u%%) exhausted history."),
               n_falign [HEXAGON_FALIGN_TOP], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_TOP] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
      as_warn (_("  %u of %u (%u%%) reached a label."),
               n_falign [HEXAGON_FALIGN_LABEL], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_LABEL] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
      as_warn (_("  %u of %u (%u%%) reached a \".align\"."),
               n_falign [HEXAGON_FALIGN_ALIGN], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_ALIGN] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
      as_warn (_("  %u of %u (%u%%) reached another `nop' packet."),
               n_falign [HEXAGON_FALIGN_NOP], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_NOP] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
      as_warn (_("  %u of %u (%u%%) failed inserting new `nop' instruction."),
               n_falign [HEXAGON_FALIGN_SHUF], n_falign [HEXAGON_FALIGN_PACK],
               n_falign [HEXAGON_FALIGN_SHUF] * 100 / n_falign [HEXAGON_FALIGN_PACK]);
    }

  if (hexagon_pairs_info && n_pairs [HEXAGON_PAIRS_TOTAL])
    {
      as_warn (_("%u instruction pairings."), n_pairs [HEXAGON_PAIRS_TOTAL]);
    }
}
