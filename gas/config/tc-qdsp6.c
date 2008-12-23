/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
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

#include <regex.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/param.h>
#include "as.h"
#include "dwarf2dbg.h"
#include "elf/qdsp6.h"
#include "libiberty.h"
#include "libbfd.h"
#include "opcode/qdsp6.h"
#include "struc-symbol.h"
#include "safe-ctype.h"
#include "subsegs.h"

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
#define MAX_FALIGN_LABELS     (1024)
/* Number of packets to look back at an .falign. */
#define MAX_FALIGN_PACKETS    (15) /* Must be at least 1. */
#if MAX_FALIGN_PACKETS < 1
#error MAX_FALIGN_PACKETS must be at least 1.
#endif
/* Include the current packet. */
#define MAX_PACKETS           (MAX_FALIGN_PACKETS + 1)
#define MAX_FIXUPS            (5)
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
#define MAX_MAPPED_INSN_LEN   (1024)
#define MAX_OPERANDS          (10) /* Safe limit. */
#define MAX_SMALL_SECTION     \
  (1 + 1 + 1 + \
   MAX (MAX (sizeof (SMALL_COM_SECTION), sizeof (SMALL_BSS_SECTION)), \
        sizeof (SMALL_DATA_SECTION)))

#define QDSP6_INSN_LEN (4) /* This is the instruction size. */

#define QDSP6_NOP         "nop"
#define QDSP6_NOP_LEN     (3)
#define QDSP6_DCFETCH     "dcfetch"
#define QDSP6_DCFETCH_LEN (7)

/* QDSP6 registers. */
#define QDSP6_NUM_GENERAL_PURPOSE_REGS 32
#define QDSP6_NUM_CONTROL_REGS         32
#define QDSP6_NUM_SYS_CTRL_REGS        64
#define QDSP6_NUM_PREDICATE_REGS       4

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

#define COND_EXEC_POS           20
#define COND_EXEC_INV_OFF       1  // Offset from previous bit
#define COND_EXEC_DOT_NEW_OFF   2  // Offset from previous bit
#define COND_EXEC_INFO_NUM_BITS 3
#define COND_EXEC_INFO_MASK     7
// P3 (31, 30, 29); P2 (28 27 26); P1 (25, 24, 23); P0 (22, 21, 20)
#define PRED_REG_INFO_MASK      0xFFF00000

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
    expressionS	    e;
    segT            sec;
    int             sub;
    symbolS        *sym;
    struct _qdsp6_literal
                   *next;
    size_t          size;
    char            name [MAX_LITERAL_NAME];
    char            secname [MAX_LITERAL_SECTION];
  } qdsp6_literal;

/* We need to keep a list of fixups.  We can't simply generate them as
   we go, because that would require us to first create the frag, and
   that would screw up references to ``.''.  */
typedef struct
  {
    const qdsp6_operand *operand;
    expressionS exp;
  } qdsp6_fixup;

typedef struct
  {
    char *ptr;
    fragS *frag;
    ptrdiff_t offset;
  } qdsp6_insn_loc;

/** Instruction packet. */
typedef struct
  {
    qdsp6_insn insn;
    const qdsp6_opcode *opcode;
    size_t lineno;
    char padded;
    char used;
    qdsp6_insn_loc loc;
    qdsp6_fixup fixups [MAX_FIXUPS];
    size_t fc;
    fixS *fix;
  } qdsp6_packet_insn;

/** Packet house keeping. */
typedef struct
  {
    size_t size; /* Number of insns. */
    char faligned; /* Packet is .falign'ed. */
    char is_inner; /* Packet has :endloop0. */
    char is_outer; /* Packet has :endloop1. */
    size_t offset;
    qdsp6_packet_insn insns [MAX_PACKET_INSNS]; /* ORIGINAL insns. */
    segT seg; /* Its section. */
    int  sub; /* Its sub-section. */
    int  reason; /* Reason for last history flush. */
  } qdsp6_packet;

/** Reason for flushing the packet history. */
enum _qdsp6_flush_reason
  {
    QDSP6_FLUSH_INIT   = 0x00, /* Initilization. */
    QDSP6_FLUSH_LABEL  = 0x01, /* A label was defined. */
    QDSP6_FLUSH_ALIGN  = 0x02, /* An alignment was performed. */
    QDSP6_FLUSH_NOP    = 0x04  /* A NOP packet was added. */
  };

/** .falign counter types. */
enum _qdsp_falign_counters
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
    QDSP6_FALIGN_COUNTERS
  };

extern int qdsp6_get_mach (char *);
extern void qdsp6_code_symbol (expressionS *);

void qdsp6_insert_operand (char *, const qdsp6_operand *, offsetT, fixS*);
void qdsp6_common (int);
void qdsp6_option (int);
void qdsp6_falign (int);
void init_opcode_tables (int);
int qdsp6_is_nop (qdsp6_insn);
qdsp6_insn qdsp6_find_nop (void);
void qdsp6_init_reg (void);
void qdsp6_check_insn (const qdsp6_opcode *, qdsp6_insn);
void qdsp6_check_single_insn (const qdsp6_opcode *, qdsp6_insn);
int qdsp6_check_new_predicate (void);
void qdsp6_check_implicit (const qdsp6_opcode *, unsigned int implicit, int reg, int *array, const char *name);
void qdsp6_packet_open (void);
void qdsp6_packet_close (void);
void qdsp6_packet_begin (void);
void qdsp6_packet_end (void);
void qdsp6_packet_end_inner (void);
void qdsp6_packet_end_outer (void);
void qdsp6_packet_end_lookahead (int *inner_p, int *);
void qdsp6_packet_write (void);
void qdsp6_packet_rewrite (qdsp6_packet *, size_t);
int qdsp6_packet_falign (size_t);
void qdsp6_history_flush (size_t, int);
void qdsp6_history_push (qdsp6_packet *);
void qdsp6_history_commit (size_t);
int qdsp6_history_coalesce (size_t, size_t);
size_t qdsp6_insert_nops (size_t);
char *qdsp6_insn_write
  (qdsp6_insn, size_t, qdsp6_fixup *, size_t, fixS **, int);
char *qdsp6_parse_immediate
  (const qdsp6_opcode *, const qdsp6_operand *,
   char *, qdsp6_insn *, int *);
int qdsp6_gp_const_lookup (char *str, char *);
segT qdsp6_create_sbss_section (const char *, flagword, unsigned int);
segT qdsp6_create_scom_section (const char *, flagword, unsigned int);
segT qdsp6_create_literal_section (const char *, flagword, unsigned int);
qdsp6_literal *qdsp6_add_to_lit_pool (expressionS *, size_t);
void qdsp6_shuffle_packet (qdsp6_packet *, size_t *);
void qdsp6_handle_shuffle (size_t *);
void qdsp6_do_shuffle (void);
int qdsp6_do_shuffle_helper (qdsp6_packet *, size_t, size_t *);
int qdsp6_discard_dcfetch (int);
int qdsp6_find_single_memop (qdsp6_packet *);
addressT qdsp6_frag_fix_addr (void);
int qdsp6_is_nop_should_keep (int);
int qdsp6_find_noslot1 (qdsp6_packet_insn *, int);
void qdsp6_check_register (int *, int, int, const qdsp6_operand *, qdsp6_insn, const qdsp6_opcode *);
int qdsp6_packet_check_solo (void);

static segT qdsp6_sdata_section, qdsp6_sbss_section;
static asection qdsp6_scom_section;
static asymbol  qdsp6_scom_symbol;

static qdsp6_literal *litpool;
static size_t litpoolcounter;

static qdsp6_packet_insn qdsp6_nop_insn;

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
#define OPTION_QDSP6_FALIGN_INFO (OPTION_MD_BASE + 2)
    { "falign_info", no_argument, NULL, OPTION_QDSP6_FALIGN_INFO },
#define OPTION_QDSP6_FALIGN_INFO_NEW (OPTION_MD_BASE + 3)
    { "mfalign-info", no_argument, NULL, OPTION_QDSP6_FALIGN_INFO_NEW },
#define OPTION_QDSP6_FALIGN_MORE_INFO (OPTION_MD_BASE + 4)
    { "mfalign-more-info", no_argument, NULL, OPTION_QDSP6_FALIGN_MORE_INFO },
/* Code in md_parse_option () assumes that the options -mv*, are sequential. */
#define OPTION_QDSP6_MQDSP6V2 (OPTION_MD_BASE + 5)
    { "mv2", no_argument, NULL, OPTION_QDSP6_MQDSP6V2 },
#define OPTION_QDSP6_MQDSP6V3 (OPTION_MD_BASE + 6)
    { "mv3", no_argument, NULL, OPTION_QDSP6_MQDSP6V3 },
#define OPTION_QDSP6_MARCH (OPTION_MD_BASE + 7)
    { "march", required_argument, NULL, OPTION_QDSP6_MARCH },
#define OPTION_QDSP6_MCPU (OPTION_MD_BASE + 8)
    { "mcpu", required_argument, NULL, OPTION_QDSP6_MCPU },
#define OPTION_QDSP6_MSORT_SDA (OPTION_MD_BASE + 9)
    { "msort-sda", no_argument, NULL, OPTION_QDSP6_MSORT_SDA },
#define OPTION_QDSP6_MNO_SORT_SDA (OPTION_MD_BASE + 10)
    { "mno-sort-sda", no_argument, NULL, OPTION_QDSP6_MNO_SORT_SDA },
  };
size_t md_longopts_size = sizeof (md_longopts);

#define IS_SYMBOL_OPERAND(o) \
 ((o) == 'b' || (o) == 'c' || (o) == 's' || (o) == 'o' || (o) == 'O')

static int qdsp6_sort_sda = TRUE;

static size_t qdsp6_gp_size = QDSP6_SMALL_GPSIZE;

static int qdsp6_packet_reverse = FALSE;
static int qdsp6_packet_shuffle = TRUE;
static int qdsp6_packet_native  = FALSE;

static int qdsp6_in_packet = FALSE;

qdsp6_packet qdsp6_packets [MAX_PACKETS]; /* Includes current packet. */

static int faligning; /* 1 => .falign next packet we see */
static int qdsp6_falign_info; /* 1 => report statistics about .falign usage */
static int qdsp6_falign_more; /* report more statistics about .falign. */
static char *falign_file;
static unsigned int falign_line;

static symbolS *falign_labels [MAX_FALIGN_LABELS]; /* labels seen between .falign and next pkt */
static size_t falign_label_count; /* # labels seen between .falign and next pkt */

static size_t n_falign [QDSP6_FALIGN_COUNTERS]; /* .falign statistics. */

// Arrays to keep track of register writes
static int gArray [QDSP6_NUM_GENERAL_PURPOSE_REGS],
           cArray [QDSP6_NUM_CONTROL_REGS],
           sArray [QDSP6_NUM_SYS_CTRL_REGS],
           pArray [QDSP6_NUM_PREDICATE_REGS],
// To keep track of register reads
           pNewArray [QDSP6_NUM_PREDICATE_REGS];

static int implicit_sr_ovf_bit_flag;  /* keeps track of the ovf bit in SR */
static int num_inst_in_packet_from_src;
static int numOfBranchAddr;
static int numOfBranchRelax;
static int numOfBranchAddrMax1;
static int numOfBranchMax1;
static int numOfLoopMax1;

// To support --march options
struct mach_arch_option
  {
	char *march_name_fe, *march_short_fe;
	unsigned int march_name_be;
  };

static struct mach_arch_option mach_arch_options [] =
  {
    {"qdsp6v2", "v2", bfd_mach_qdsp6_v2},
    {"qdsp6v3", "v3", bfd_mach_qdsp6_v3},
  };

static size_t mach_arch_options_size =
  sizeof (mach_arch_options) / sizeof (struct mach_arch_option);

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

    case OPTION_QDSP6_FALIGN_MORE_INFO:
      qdsp6_falign_more = TRUE;
      /* Fall through. */

    case OPTION_QDSP6_FALIGN_INFO:
    case OPTION_QDSP6_FALIGN_INFO_NEW:
      if (c == OPTION_QDSP6_FALIGN_INFO)
        as_tsktsk (_("Option \"-%s\" is deprecated; use \"-%s\" instead."),
                  md_longopts [OPTION_QDSP6_FALIGN_INFO     - OPTION_MD_BASE].name,
                  md_longopts [OPTION_QDSP6_FALIGN_INFO_NEW - OPTION_MD_BASE].name);
      qdsp6_falign_info = TRUE;
      break;

    case OPTION_QDSP6_MQDSP6V2:
    case OPTION_QDSP6_MQDSP6V3:
    case OPTION_QDSP6_MARCH:
    case OPTION_QDSP6_MCPU:
      switch (c)
        {
          case OPTION_QDSP6_MQDSP6V2:
          case OPTION_QDSP6_MQDSP6V3:
            /* -mv* options. */
            temp_qdsp6_mach_type
              = mach_arch_options [c - OPTION_QDSP6_MQDSP6V2].march_name_be;
            break;

          default:
            /* -march and- mcpu options. */
            for (i = 0; i < mach_arch_options_size; i++)
              if (   !strcmp (arg, mach_arch_options [i].march_name_fe)
                  || !strcmp (arg, mach_arch_options [i].march_short_fe))
                {
                  temp_qdsp6_mach_type = mach_arch_options [i].march_name_be;
                  break;
                }

            if (i == mach_arch_options_size)
              as_fatal (_("invalid architecture."));
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
  -msort-sda               enable sorting the small-data area (default)\n\
  -mfalign-info            report \".falign\" statistics\n\
  -mv2                     assemble code for the QDSP6 V2 architecture (default)\n\
  -mv3                     assemble code for the QDSP6 V3 architecture\n\
  -march={qdsp6v2|qdsp6v3} assemble code for the specified QDSP6 architecture\n\
  -mcpu={qdsp6v2|qdsp6v3}  equivalent to \"-march\"\n",
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
    as_warn (_("could not set architecture and machine."));

  /* This call is necessary because we need to initialize `qdsp6_operand_map'
     which may be needed before we see the first insn.  */
  qdsp6_opcode_init_tables
    (qdsp6_get_opcode_mach (qdsp6_mach_type, target_big_endian));

  /* Initialize global NOP. */
  qdsp6_nop_insn.insn   = qdsp6_find_nop ();
  qdsp6_nop_insn.opcode = qdsp6_lookup_insn (qdsp6_nop_insn.insn);

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


/* We need to distinguish a register pair name (e.g., r1:0 or p3:0)
   from a label. */
int qdsp6_start_label
(char c, char *before, char *after)
{
  static int re_ok;
  static const char ex_before [] =
    REGEX_LEFT ("((r((0*[12]?[13579])|31))|sp|lr|p3)$");
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
      /* Compile RE for GPR or predicate pairs. */
      assert (!regcomp (&re_before, ex_before, REG_EXTENDED | REG_ICASE | REG_NOSUB));
      assert (!regcomp (&re_after,  ex_after,  REG_EXTENDED | REG_ICASE | REG_NOSUB));

      re_ok = TRUE;
    }

  /* Register pairs are not labels. */
  if (    !(er_before = regexec (&re_before, before, 0, NULL, 0))
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
          if (   !fragP->fr_subtype
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
init_opcode_tables(
     int mach
)
{
  if (!bfd_set_arch_mach (stdoutput, bfd_arch_qdsp6, mach))
    as_warn (_("could not set architecture and machine."));

  /* This initializes a few things in qdsp6-opc.c that we need.
     This must be called before the various qdsp6_xxx_supported fns.  */
  qdsp6_opcode_init_tables (qdsp6_get_opcode_mach (mach, target_big_endian));

  /* Tell `.option' it's too late.  */
  cpu_tables_init_p = 1;
}

/* Insert an operand value into an instruction. */

void
qdsp6_insert_operand
(char *where, const qdsp6_operand *operand, offsetT val, fixS *fixP)
{
  qdsp6_insn insn;
  const qdsp6_opcode *opcode;
  char *errmsg = NULL;

  // Decide if it's a 16 or 32-bit instruction
  if (target_big_endian) {
    insn = bfd_getb16 ((unsigned char *) where);
  }
  else {
    insn = bfd_getl16 ((unsigned char *) where);
  }
  if (!QDSP6_IS16INSN (insn)) {
    if (target_big_endian) {
      insn = bfd_getb32 ((unsigned char *) where);
    }
    else {
      insn = bfd_getl32 ((unsigned char *) where);
    }
  }

  opcode = qdsp6_lookup_insn(insn);
  if (!opcode) {
    as_bad("could not find opcode");
  }

  if (!qdsp6_encode_operand(operand, &insn, opcode->enc, val, &errmsg)) {
    if (errmsg) {
      if(fixP!=NULL && fixP->fx_file!=NULL)
	{
	  char tmpError[200];
	  sprintf(tmpError, " when resolving symbol in file %s at line %d", fixP->fx_file, fixP->fx_line);
	  strcat(errmsg, tmpError);
	}

      as_bad(errmsg);
    }
  }

  if (QDSP6_IS16INSN (insn)) {
    if (target_big_endian) {
      bfd_putb16 ((bfd_vma) insn, (unsigned char *) where);
    }
    else {
      bfd_putl16 ((bfd_vma) insn, (unsigned char *) where);
    }
  }
  else {
    if (target_big_endian) {
      bfd_putb32 ((bfd_vma) insn, (unsigned char *) where);
    }
    else {
      bfd_putl32 ((bfd_vma) insn, (unsigned char *) where);
    }
  }
}

char *
qdsp6_parse_immediate
(const qdsp6_opcode *opcode, const qdsp6_operand *operand,
 char *str, qdsp6_insn *insn, int *val)
{
  char *hold;
  expressionS exp;
  long value = 0;
  int is_lo16 = 0;
  int is_hi16 = 0;

  /* We only have the '#' for immediates that are NOT pc relative */
  if (!(operand->flags & QDSP6_OPERAND_PC_RELATIVE))
    {
      if (*str == '#')
        /* Skip over the '#' */
        str++;
      else
        return NULL;
    }

  /* QDSP6 TODO: Allow white space between lo/hi and the paren */
  if (TOLOWER(str[0]) == 'l' && TOLOWER(str[1]) == 'o' && str[2] == '(')
    {
      /* Make sure we have a corresponding lo16 operand */
      operand = qdsp6_operand_find_lo16(operand);
      if (!operand)
        return NULL;

      is_lo16 = 1;
      str += 3;
    }
  else if (TOLOWER(str[0]) == 'h' && TOLOWER(str[1]) == 'i' && str[2] == '(')
    {
      /* Make sure we have a corresponding hi16 operand */
      operand = qdsp6_operand_find_hi16(operand);
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
          as_bad("missing )");
          return NULL;
        }
      else
        str++;
    }

  if (exp.X_op == O_illegal)
    {
      as_bad("illegal operand");
      return NULL;
    }
  else if (exp.X_op == O_absent)
    {
      as_bad("missing operand");
      return NULL;
    }
  else if (exp.X_op == O_register)
    {
      as_bad("unexpected register");
      return NULL;
    }
  else if (exp.X_op == O_constant)
    {
      char *errmsg = NULL;
      value = exp.X_add_number;

      if (!qdsp6_encode_operand (operand, insn, opcode->enc, value, &errmsg))
        {
          if (errmsg)
            as_bad (errmsg);
          return NULL;
        }
    }
  else
    {
      /* This expression involves one or more symbols.
        Record a fixup to process later */
      if (qdsp6_packets [0].insns [qdsp6_packets [0].size].fc >= MAX_FIXUPS)
        as_fatal (_("too many fixups."));

      if (operand->reloc_type == BFD_RELOC_NONE)
        {
          as_bad (_("unexpected non-constant expression."));
          return NULL;
        }

      qdsp6_packets [0].insns [qdsp6_packets [0].size].fixups [qdsp6_packets [0].insns [qdsp6_packets [0].size].fc].exp = exp;
      qdsp6_packets [0].insns [qdsp6_packets [0].size].fixups [qdsp6_packets [0].insns [qdsp6_packets [0].size].fc].operand = operand;
      qdsp6_packets [0].insns [qdsp6_packets [0].size].fc++;
    }

  *val = value;
  return str;
}

char*
qdsp6_insn_write
(qdsp6_insn insn, size_t fc, qdsp6_fixup *fixups,
 size_t offset, fixS **fixSP, int lineno)
{
  char *f;
  size_t i;

  /* Write out the instruction.
     It is important to fetch enough space in one call to `frag_more'.
     We use (f - frag_now->fr_literal) to compute where we are and we
     don't want frag_now to change between calls.  */
  if (QDSP6_IS16INSN (insn))
    {
      // 16-bit instruction
      f = frag_more (2);
      if ((frag_now->fr_address + frag_now_fix ()) % 2)
        as_warn (_("current location is not %d-byte aligned."), 2);

      md_number_to_chars (f, insn, 2);
/* 
 * XXX_SM: this was modified in 2.14 so that loc.line is set to
 * lineno as seen here.  My sense is that because packets are 
 * broken up across lines this is needed and that as_where()
 * isn't smart enough to figure out the correct line number.
 * These calculations have change so I'm leaving reverting to
 * the baseline for now.
 * Old Q6 wants: 
 *	dwarf2_emit_insn (2, lineno);
 */
      dwarf2_emit_insn (2); /* XXX_SM */
    }
  else
    {
      // 32-bit instruction
      f = frag_more (QDSP6_INSN_LEN);
      if ((frag_now->fr_address + frag_now_fix ()) % QDSP6_INSN_LEN)
        as_warn (_("current location is not %d-byte aligned."), QDSP6_INSN_LEN);

      md_number_to_chars (f, insn, QDSP6_INSN_LEN);
      dwarf2_emit_insn (QDSP6_INSN_LEN); /* XXX_SM */
    }

  /* Create any fixups */
  if (fixSP)
    *fixSP = 0;

  for (i = 0; i < fc; i++)
    {
      const qdsp6_operand *operand = fixups [i].operand;

      fixS *fixP =
        fix_new_exp (frag_now, f - frag_now->fr_literal,
                     QDSP6_IS16INSN (insn)? 2: QDSP6_INSN_LEN,
                     &fixups [i].exp,
                     (operand->flags & QDSP6_OPERAND_PC_RELATIVE) != 0,
                     operand->reloc_type);

      fixP->tc_fix_data = (qdsp6_operand *) operand;

      if (operand->flags & (QDSP6_OPERAND_IS_LO16 | QDSP6_OPERAND_IS_HI16))
        fixP->fx_no_overflow = TRUE;

    /* Any PC relative operands within a packet must
      be relative to the address of the first instruction
      We'll adjust the offset here to account for that. */
      if (operand->flags & QDSP6_OPERAND_PC_RELATIVE)
        fixP->fx_offset += offset;

      /* QDSP6 insns will never have more than 1 fixup? */
      if (fixSP)
        *fixSP = fixP;
    }

  return f;
}

/** Mark the next packet for .falign.

@see qdsp6_packet_falign ().
*/
void
qdsp6_falign
(int ignore ATTRIBUTE_UNUSED)
{
  if (!faligning)
    {
      faligning = TRUE;
      falign_label_count = 0;
      n_falign [QDSP6_FALIGN_TOTAL]++;
    }

  as_where (&falign_file, &falign_line);

  demand_empty_rest_of_line ();

  /* Just in case out of .text. */
  record_alignment (now_seg, DEFAULT_CODE_FALIGN);
}

void
qdsp6_frob_label
(symbolS *label)
{
  /* Do not frob literal symbols. */
  if (   !strncmp (now_seg->name, LITERAL_SECTION, LITERAL_LEN)
      || !strncmp (now_seg->name, LITERAL_SECTION_ONCE, LITERAL_ONCE_LEN))
    return;

  if (faligning)
    {
      /* Catch labels between .falign and the instruction packet. */
      if (falign_label_count >= MAX_FALIGN_LABELS)
        as_bad (_("too many labels between \".falign\" and the next packet."));

      falign_labels [falign_label_count++] = label;
    }
  else if (!qdsp6_in_packet)
    /* Catch labels outside a packet. */
    qdsp6_history_flush (0, QDSP6_FLUSH_LABEL);

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
        bzero (fragP->fr_literal + fragP->fr_fix, skip);
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

  /* Once aligned, it's out of bounds for .falign. */
  qdsp6_history_flush (0, QDSP6_FLUSH_ALIGN);
}

/** Rewrite the specified packet.

@param pkt reference to packet.
@param n number of new instructions added.
*/
void
qdsp6_packet_rewrite
(qdsp6_packet *pkt, size_t n)
{
  size_t req_insns = pkt->size, num_insns;
  size_t old_size = pkt->size - n;
  size_t max_skip = MAX_PACKET_INSNS - req_insns;
  size_t i;

  assert (pkt->size);

  /* Rewrite existing packet with (possibly) changed instructions. */
  for (num_insns = 0, i = 0; num_insns < old_size; i++)
    {
      if (   max_skip > 0
	  && pkt->insns [i].padded
	  && qdsp6_is_nop (pkt->insns [i].insn))
	{
	  pkt->insns [i].padded = FALSE;
      max_skip--;
      continue;
    }

      if (num_insns == 0 && pkt->is_inner)
	pkt->insns [i].insn
	  = QDSP6_PACKET_BIT_SET (pkt->insns [i].insn, QDSP6_END_LOOP);
      else if (num_insns == 1 && pkt->is_outer)
	pkt->insns [i].insn
	  = QDSP6_PACKET_BIT_SET (pkt->insns [i].insn, QDSP6_END_LOOP);
      else
	pkt->insns [i].insn
	  = QDSP6_PACKET_BIT_SET (pkt->insns [i].insn, QDSP6_END_NOT);

      /* Rewrite instruction at this location in original packet. */
      md_number_to_chars (pkt->insns [num_insns].loc.ptr, pkt->insns [i].insn, QDSP6_INSN_LEN);

      /* Modify fixup for this instruction to refer to new location. */
      if (pkt->insns [i].fix)
	{
	  pkt->insns [i].fix->fx_frag = pkt->insns [num_insns].loc.frag;
	  pkt->insns [i].fix->fx_where = pkt->insns [num_insns].loc.offset;
	  if (pkt->insns [i].fix->fx_pcrel)
	    pkt->insns [i].fix->fx_offset += num_insns * QDSP6_INSN_LEN;
    }

      /* Count insn as legit. */
      num_insns++;
    }

  /* Output additional instructions/ */
  for (i = i; num_insns < req_insns; num_insns++)
    {
      if (num_insns >= req_insns - 1)
	pkt->insns [i].insn
	  = QDSP6_PACKET_BIT_SET (pkt->insns [i].insn, QDSP6_END_PACKET);
      else
	pkt->insns [i].insn
	  = QDSP6_PACKET_BIT_SET (pkt->insns [i].insn, QDSP6_END_NOT);

      pkt->insns [i].loc.ptr
        = qdsp6_insn_write (pkt->insns [i].insn,
	                    0, NULL, 1, NULL,
			    pkt->insns [i].lineno);

    // modify fixup for this instruction to refer to new location
      if (pkt->insns [i].fix)
	{
	  pkt->insns [i].fix->fx_frag = frag_now;
	  pkt->insns [i].fix->fx_where = pkt->insns [i].loc.ptr
	                               - frag_now->fr_literal;
	  if (pkt->insns [i].fix->fx_pcrel)
	    pkt->insns [i].fix->fx_offset += num_insns * QDSP6_INSN_LEN;
	}
    }
  }

/** Insert NOP instructions in previous packet(s).

This requires rebuilding the original packet,
including reshuffling into a valid ordering and
reinserting end-loop and end-packet bits.
@param nops number of NOP instruction to insert.
@return number of NOP instructions not inserted.
@see qdsp6_packet_falign ()
*/
size_t
qdsp6_insert_nops
(size_t nops)
{
  size_t left, still;
  size_t last;
  size_t i, j;
  int go;
  int incorp = FALSE, insert = FALSE;

  for (j = 1, left = nops, last = 0, go = TRUE;
       j < MAX_PACKETS && left && go; /* and sub-section. */
       j++)
    {
      /* Scratch packet. */
      qdsp6_packet packet;

      /* Possible reasons to stop walking the history early. */
      if (!qdsp6_packets [j].size) /* Empty packet. */
        {
          if (qdsp6_packets [j].reason & QDSP6_FLUSH_LABEL)
            n_falign [QDSP6_FALIGN_LABEL]++;
          if (qdsp6_packets [j].reason & QDSP6_FLUSH_ALIGN)
            n_falign [QDSP6_FALIGN_ALIGN]++;
          if (qdsp6_packets [j].reason & QDSP6_FLUSH_NOP)
            n_falign [QDSP6_FALIGN_NOP]++;

          if (!qdsp6_packets [j].reason)
            n_falign [QDSP6_FALIGN_TOP]++;

          break;
        }
      if (qdsp6_packets [j].faligned) /* An .falign'ed packet. */
        {
          if (qdsp6_packets [j].size == 1)
            n_falign [QDSP6_FALIGN_FALIGN1]++;
          else
            n_falign [QDSP6_FALIGN_FALIGN]++;
          break;
        }
      if (   qdsp6_packets [j].seg != now_seg     /* Not in the same section... */
          || qdsp6_packets [j].sub != now_subseg) /* nor sub-section. */
        {
          n_falign [QDSP6_FALIGN_SECTION]++;
          break;
        }

      /* Skip a full packet. */
      if (qdsp6_packets [j].size >= MAX_PACKET_INSNS)
	continue;

      /* Prepare the packet for reshuffling and collect some restrictions. */
      for (i = 0, packet = qdsp6_packets [j];
           i < MAX_PACKET_INSNS && go;
           i++)
	if (i < packet.size)
	  {
	    packet.insns [i].used = FALSE;

	    /* Fix ups don't allow to move an insn. */
	    // go &= !packet.insns [i].fix && !packet.insns [i].fc;
          }
	else
	  bzero (packet.insns + i, sizeof (packet.insns [i]));

      /* Stop at a packet with restrictions. */
      if (!go)
	break;

      /* Fill up scratch packet with NOP. */
      for (still = left;
	   still && packet.size < MAX_PACKET_INSNS;
	   still--, packet.size++)
	{
	  packet.insns [packet.size].insn = qdsp6_nop_insn.insn;
	  packet.insns [packet.size].opcode = qdsp6_nop_insn.opcode;
	  packet.insns [packet.size].lineno
	    = packet.insns [packet.size - 1].lineno;
	  packet.offset += QDSP6_INSN_LEN;
        }

      /* Try to reshuffle packet. */
      if (!qdsp6_do_shuffle_helper (&packet, 0, NULL))
	continue;
      else
        {
	  /* Undo tweaks to PC-relative fixup offsets when rewriting. */
/*
	  for (i = 0; i < MAX_PACKET_INSNS; i++)
	    if (   packet.insns [i].fix
		&& packet.insns [i].fix->fx_pcrel)
	      packet.insns [i].fix->fx_offset -= i * QDSP6_INSN_LEN;
*/
          /* Update the packet. */
/*
          qdsp6_packet_rewrite (&packet, left - still);
*/

	  /* Commit scratch packet back to the packet history. */
	  qdsp6_packets [j] = packet;

	  left = still;
	  last = j;

	  incorp = TRUE;
        }
    }

  if (j >= MAX_PACKETS) /* End of history reached. */
    n_falign [QDSP6_FALIGN_END]++;

  /* Commit changes. */
  if (last)
    qdsp6_history_commit (last);

  /* Add remaining NOP in a separate packet. */
  if (left)
    {
      /* No need to add the NOP packet to the history,
          so flush it, but for the .falign'ed packet,
          so that it's not stale. */
      qdsp6_history_flush (1, QDSP6_FLUSH_NOP);

      if (qdsp6_falign_info)
        as_warn_where
          (falign_file, falign_line,
            _("`.falign' inserted a new %lu-`nop' packet."), left);

      while (--left)
        qdsp6_insn_write (qdsp6_nop_insn.insn | QDSP6_END_NOT, 0, NULL, 0, NULL, falign_line);
      qdsp6_insn_write (qdsp6_nop_insn.insn | QDSP6_END_PACKET, 0, NULL, 0, NULL, falign_line);

      insert = TRUE;
    }

    /* Collect statistics.
       A .falign may resulted in both NOPs incorporated and new packets inserted.
       When both happen at the same time, count the .falign as inserted. */
  if (insert)
    n_falign [QDSP6_FALIGN_INS]++;
  else if (incorp)
    n_falign [QDSP6_FALIGN_INC]++;

  return (left);
}

/** .falign the current packet if possible.

@param new_pkt_size the desired packet size, in insns.
@return true if packet had to be .falign'ed.
@see qdsp6_insert_nops ().
*/
int
qdsp6_packet_falign
(size_t new_pkt_size)
{
  addressT new_pkt_addr = qdsp6_frag_fix_addr ();
  addressT new_pkt_off, new_pkt_over;
  size_t i;

  new_pkt_off  = new_pkt_addr % (1 << DEFAULT_CODE_FALIGN);
  new_pkt_over = new_pkt_off + (new_pkt_size * QDSP6_INSN_LEN);
  new_pkt_over = new_pkt_over > (1 << DEFAULT_CODE_FALIGN)
               ? new_pkt_over % (1 << DEFAULT_CODE_FALIGN)
	       : 0;

  /* Check if new packet will cross a decode-window. */
  if (new_pkt_over)
    {
      int nops_add, nops_left;

      nops_add =  ((1 << DEFAULT_CODE_FALIGN) - new_pkt_off) / QDSP6_INSN_LEN;

      /* Add NOP to .falign the current packet. */
      nops_left = qdsp6_insert_nops (nops_add);

      /* Update any labels that refer to the original packet address. */
      if (nops_left < nops_add)
        for (i = 0; i < falign_label_count; i++)
          symbol_adjust_value (falign_labels [i], nops_add * QDSP6_INSN_LEN);
      falign_label_count = 0;

      faligning = FALSE;
      return TRUE;
    }
  else
    {
      falign_label_count = 0;

      faligning = FALSE;
      return FALSE;
    }
}

/** Check if packet has single memory operation.

Bug #832: if there is only one memory op in a packet,
it should go in slot 0.
@param packet Packet to examine.
@return True if so.
*/
int
qdsp6_find_single_memop
(qdsp6_packet *packet)
{
  int i;
  int count;

  /* Count number of memory ops in this packet. */
  for (i = count = 0; i < MAX_PACKET_INSNS; i++)
      if (   packet->insns [i].opcode
	  && (  packet->insns [i].opcode->attributes
              & A_RESTRICT_SINGLE_MEM_FIRST))
      count++;

  return (count == 1);
}

int
qdsp6_is_nop_should_keep
(int current)
{
  int foundPrevNoneNop = FALSE;

  int prev = current-1;
  int next = current+1;

  if (next < MAX_PACKET_INSNS)
    {
  while (prev >= 0)
    {
	  if (!qdsp6_is_nop (qdsp6_packets [0].insns [prev].insn))
	{
	      foundPrevNoneNop = TRUE;
	  break;
	}
      prev--;
    }

  // A_RESTRICT_NOSLOT1: the next instruction cannot be in slot 1
  // For arch_v1, the current nop should be kept (not sure why)
  // For arch_v2, only when the next instruction can be put in slot 1
  // that should the current nop be kept
      if (   foundPrevNoneNop
	  && (qdsp6_packets [0].insns [next].opcode->attributes & A_RESTRICT_NOSLOT1))
    {
	  if (   qdsp6_packets [0].insns [next].opcode->slot_mask & 2
	      || qdsp6_if_arch_v1 ())
	    return TRUE;
	}
    }

  return FALSE;
}

/** Flush all or part of the packet history.

The packet history should be flushed after some events which make it useless,
such as .align and .falign.
@param number of packets to leave intact (typically 0 or 1).
@param reason for flushing the history.
@see qdsp6_history_push ().
*/
    void
qdsp6_history_flush
(size_t number, int reason)
{
  size_t flushed;

  number = MIN (number, MAX_PACKETS);
  flushed = MIN (number? number: 1, MAX_PACKETS);

  bzero (qdsp6_packets + number,
         (MAX_PACKETS - number) * sizeof (qdsp6_packets [0]));

  if (flushed < MAX_PACKETS)
    qdsp6_packets [flushed].reason |= reason;
}

/** Save a packet into packet history.

Save a packet into packet history used to perform .falign back in
the previous few packets.  The current packet is initialized.
The current packet should be pushed once it's commited.
@param packet reference to packet to be saved (typically the current packet).
@see qdsp6_history_flush ().
*/
void
qdsp6_history_push
(qdsp6_packet *packet)
{
  if (MAX_PACKETS > 2)
    memmove (qdsp6_packets + 2, qdsp6_packets + 1,
             (MAX_PACKETS - 3) * sizeof (qdsp6_packets [0]));

  if (MAX_PACKETS > 1)
    {
      packet->seg = now_seg;
      packet->sub = now_subseg;

      qdsp6_packets [1] = *packet;
    }

  bzero (qdsp6_packets + 0, sizeof (qdsp6_packets [0]));
}

/** Coalesce history locations starting at specified insn.

@param ip Index to earliest packet in history (must not be 0, or the current one).
@param ii Index to insn in packet without a location yet.
@return True if specified insn has got a location.
*/
int
qdsp6_history_coalesce (size_t ip, size_t ii)
{
  size_t jp, ji;
  size_t i, j;

  if (!ip)
    return FALSE;

  i = ip;
  j = ii;

  if (qdsp6_packets [i].insns [j].loc.ptr)
    return TRUE;

  jp = ii + 1 < qdsp6_packets [ip].size? ip: ip - 1;
  ji = ii + 1 < qdsp6_packets [ip].size? ii + 1: 0;

  while (ip > 0 && jp > 0)
    {
      while (!qdsp6_packets [jp].insns [ji].loc.ptr)
	{
	  if (++ji >= qdsp6_packets [jp].size)
	    {
	      ji = 0;
	      if (!--jp)
                break;
	    }
        }

      if (jp)
        {
          qdsp6_packets [ip].insns [ii].loc
            = qdsp6_packets [jp].insns [ji].loc;
          bzero (&qdsp6_packets [jp].insns [ji].loc,
                sizeof (qdsp6_packets [jp].insns [ji].loc));

          if (++ji >= qdsp6_packets [jp].size)
            {
              ji = 0;
              --jp;
            }
          if (++ii >= qdsp6_packets [ip].size)
            {
              ii = 0;
              --ip;
            }
        }
    }

  return (qdsp6_packets [i].insns [j].loc.ptr? TRUE: FALSE);
}

/** Commit possibly updated packet history.

If packets in the packet history were changed, then the instruction stream is
modified to reflect these changes.
@param n Index of the earliest modified packet in the history
         (must not be 0, or the current one).
@todo Handle 16-bit insns.
*/
void
qdsp6_history_commit (size_t n)
{
  size_t i, j;
  int on;

  /* Rewrite history packets with (possibly) changed instructions. */
  for (on = TRUE, i = n; on && i > 0; i--)
    {
      for (j = 0; on && j < qdsp6_packets [i].size; j++)
	{
	  /* Mark packet bits. */
	  if (j == 0 && qdsp6_packets [i].is_inner)
	    qdsp6_packets [i].insns [j].insn
	      = QDSP6_PACKET_BIT_SET (qdsp6_packets [i].insns [j].insn,
	                              QDSP6_END_LOOP);
	  else if (j == 1 && qdsp6_packets [i].is_outer)
	    qdsp6_packets [i].insns [j].insn
	      = QDSP6_PACKET_BIT_SET (qdsp6_packets [i].insns [j].insn,
	                              QDSP6_END_LOOP);
	  else if (j >= qdsp6_packets [i].size - 1)
	    qdsp6_packets [i].insns [j].insn
	      = QDSP6_PACKET_BIT_SET (qdsp6_packets [i].insns [j].insn,
	                              QDSP6_END_PACKET);
	  else
	    qdsp6_packets [i].insns [j].insn
	      = QDSP6_PACKET_BIT_SET (qdsp6_packets [i].insns [j].insn,
	                              QDSP6_END_NOT);

	  if (!qdsp6_packets [i].insns [j].loc.ptr)
	    {
	      qdsp6_packets [i].insns [j].padded = FALSE;
	      on = qdsp6_history_coalesce (i, j);
	    }

	  /* Rewrite instruction at this location. */
	  if (qdsp6_packets [i].insns [j].loc.ptr)
	    {
	      md_number_to_chars (qdsp6_packets [i].insns [j].loc.ptr,
	                          qdsp6_packets [i].insns [j].insn,
		  		  QDSP6_INSN_LEN);

	      /* Modify fixup for this instruction to refer to new location. */
	      if (qdsp6_packets [i].insns [j].fix)
		{
		  qdsp6_packets [i].insns [j].fix->fx_frag
		    = qdsp6_packets [i].insns [j].loc.frag;
		  qdsp6_packets [i].insns [j].fix->fx_where
		    = qdsp6_packets [i].insns [j].loc.offset;
/*
		  if (qdsp6_packets [i].insns [j].fix->fx_pcrel)
		    qdsp6_packets [i].insns [j].fix->fx_offset
		      += j * QDSP6_INSN_LEN;
*/
                }
	    }
	  else
	    {
	      on = FALSE;
	      break;
	    }
	}
      if (!on)
	break;
    }

  /* Append (possibly) modified packets. */
  for (; i > 0; i--, j = 0)
    {
      for (; j < qdsp6_packets [i].size; j++)
	{
	  if (j == 0 && qdsp6_packets [i].is_inner)
	    qdsp6_packets [i].insns [j].insn
	      = QDSP6_PACKET_BIT_SET (qdsp6_packets [i].insns [j].insn,
	                              QDSP6_END_LOOP);
	  else if (j == 1 && qdsp6_packets [i].is_outer)
	    qdsp6_packets [i].insns [j].insn
	      = QDSP6_PACKET_BIT_SET (qdsp6_packets [i].insns [j].insn,
	                              QDSP6_END_LOOP);
	  else if (j >= qdsp6_packets [i].size - 1)
	    qdsp6_packets [i].insns [j].insn
	      = QDSP6_PACKET_BIT_SET (qdsp6_packets [i].insns [j].insn,
	                              QDSP6_END_PACKET);
	  else
	    qdsp6_packets [i].insns [j].insn
	      = QDSP6_PACKET_BIT_SET (qdsp6_packets [i].insns [j].insn,
	                              QDSP6_END_NOT);

	  qdsp6_packets [i].insns [j].loc.ptr
	    = qdsp6_insn_write (qdsp6_packets [i].insns [j].insn,
				0 /*qdsp6_packets [i].insns [j].fc*/,
				0 /*qdsp6_packets [i].insns [j].fixups*/,
				j * QDSP6_INSN_LEN,
				0 /*&qdsp6_packets [i].insns [j].fix*/,
				qdsp6_packets [i].insns [j].lineno);
	  qdsp6_packets [i].insns [j].loc.frag = frag_now;
	  qdsp6_packets [i].insns [j].loc.offset
	    = qdsp6_packets [i].insns [j].loc.ptr - frag_now->fr_literal;

	  /* Modify fixup for this instruction to refer to new location. */
	  if (qdsp6_packets [i].insns [j].fix)
	    {
	      qdsp6_packets [i].insns [j].fix->fx_frag
		= qdsp6_packets [i].insns [j].loc.frag;
	      qdsp6_packets [i].insns [j].fix->fx_where
		= qdsp6_packets [i].insns [j].loc.offset;
/*
	      if (qdsp6_packets [i].insns [j].fix->fx_pcrel)
		qdsp6_packets [i].insns [j].fix->fx_offset
		  += j * QDSP6_INSN_LEN;
*/
            }
        }
    }
}

/** Write the current packet out.

@todo Must handle 16-bit insns eventually.
*/
void
qdsp6_packet_write
(void)
{
  qdsp6_packet packet;
  size_t max_skip, num_nops, num_padded_nops;
  size_t req_insns;
  size_t i;

  /* Determine if we can skip any NOP, for
     at least 2 instructions are needed for :endloop0 and
     at least 3 instructions for :endloop1; otherwise
     all can be skipped and nothing is emitted. */
  if (qdsp6_packets [0].is_inner || qdsp6_packets [0].is_outer)
    max_skip = qdsp6_packets [0].size - (qdsp6_packets [0].is_outer? 3: 2);
  else
    max_skip = qdsp6_packets [0].size;

  /* Calculate the number of NOP needed. */
  for (i = 0, num_nops = 0, num_padded_nops = 0;
       i < qdsp6_packets [0].size;
       i++)
    {
      qdsp6_insn insn = qdsp6_packets [0].insns [i].insn;

      if (qdsp6_is_nop (insn) && !qdsp6_is_nop_should_keep (i))
	{
	  num_nops++;

	  if (qdsp6_packets [0].insns [i].padded)
	    num_padded_nops++;
	}
    }
  max_skip = MIN (max_skip, MIN (num_nops, num_padded_nops));

  /* Keep track of the number of emitted instructions to
     determine which packet bits to set. */
  req_insns = qdsp6_packets [0].size - max_skip;

  if ((qdsp6_packets [0].faligned = faligning))
    qdsp6_packet_falign (req_insns);

  /* Initialize scratch packet. */
  qdsp6_packets [0].offset = 0;
  packet = qdsp6_packets [0];
  packet.size = 0;

  for (i = 0; i < qdsp6_packets [0].size; i++)
    {
      /* Make sure that the packet bits are clear. */
      if (qdsp6_packets [0].insns [i].insn & QDSP6_PACKET_BIT_MASK)
	as_fatal (_("unexpected value in instruction packet bits."));

      /* Skip the padded NOP, not every NOP. */
      if (   max_skip
	  && qdsp6_packets [0].insns [i].padded
	  && qdsp6_is_nop (qdsp6_packets [0].insns [i].insn)
	  && !qdsp6_is_nop_should_keep (i))
	{
	  max_skip--;
	  continue;
    }

      /* Set proper packet bits. */
      if (packet.size == 0 && qdsp6_packets [0].is_inner)
	qdsp6_packets [0].insns [i].insn |= QDSP6_END_LOOP;
      else if (packet.size == 1 && qdsp6_packets [0].is_outer)
	qdsp6_packets [0].insns [i].insn |= QDSP6_END_LOOP;
      else if (packet.size >= req_insns - 1)
	qdsp6_packets [0].insns [i].insn |= QDSP6_END_PACKET;
      else
	qdsp6_packets [0].insns [i].insn |= QDSP6_END_NOT;

      packet.insns [packet.size] = qdsp6_packets [0].insns [i];
      packet.insns [packet.size].loc.ptr
        = qdsp6_insn_write (packet.insns [packet.size].insn,
	                    packet.insns [packet.size].fc,
	                    packet.insns [packet.size].fixups,
			    packet.offset,
			    &packet.insns [packet.size].fix,
			    packet.insns [packet.size].lineno);
      packet.insns [packet.size].loc.frag = frag_now;
      packet.insns [packet.size].loc.offset
        = packet.insns [packet.size].loc.ptr - frag_now->fr_literal;

      packet.offset += QDSP6_IS16INSN (packet.insns [packet.size].insn)
		     ? 2
		     : QDSP6_INSN_LEN;

      /* Count insn as legit. */
      packet.size++;
    }

  qdsp6_history_push (&packet);
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
    if (   (size_t) snprintf (small_name, sizeof (small_name), "%s.%d", name, access)
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
    if (   (size_t) snprintf (small_name, sizeof (small_name), "%s.%d", name, access)
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
  qdsp6_literal *litptr, *lastptr;
  segT current_section = now_seg;
  int current_subsec = now_subseg;
  offsetT mask = (size == 8)? 0xffffffffffffffffULL: 0xffffffffULL;

  /* Sanity check for illegal constant literals. */
  if (size != 8 && size != 4)
    as_fatal (_("bad literal constant."));

  /* try to find the existing one for the expression "exp"
   * to reuse the literal
   */
  for (litptr = litpool; litptr != NULL; litptr = litptr->next)
    {
      /* save the last node */
      lastptr = litptr;

      if (   (litptr->e.X_op == exp->X_op)
          && (litptr->e.X_op == O_constant)
          && ((litptr->e.X_add_number & mask) == (exp->X_add_number & mask))
          && (litptr->size == size))
        break;

      if (   (litptr->e.X_op == exp->X_op)
          && (litptr->e.X_op == O_symbol)
          && (litptr->e.X_op_symbol == exp->X_op_symbol)
          && (litptr->e.X_add_symbol == exp->X_add_symbol)
          && (litptr->e.X_add_number == exp->X_add_number)
          && (litptr->size == size))
        break;
    }

  /* do we need to create a new entry? */
  if (!litptr)
    {
      if (litpoolcounter >= MAX_LITERAL_POOL)
        {
          as_bad (_("too many literal constants."));
          return NULL;
        }

      litptr = xmalloc (sizeof (*litptr));
      litptr->next = NULL;
      litptr->size = size;

      /* save the head pointer to "litpool" */
      if (!litpool)
        litpool = lastptr = litptr;

      /* get the expression */
      litptr->e = *exp;

      if (litptr->e.X_op == O_constant)
        /* Create a constant symbol (with its value as the suffix). */
        {
          if (size == 8)
            {
              sprintf (litptr->name, "%s_%016llx",
                     LITERAL_PREFIX, (long long) litptr->e.X_add_number);
              sprintf (litptr->secname, "%s%s", LITERAL_SECTION_ONCE_8, litptr->name);
            }
          else
            {
              sprintf (litptr->name, "%s_%08x",
                     LITERAL_PREFIX, (int) litptr->e.X_add_number);
              sprintf (litptr->secname, "%s%s", LITERAL_SECTION_ONCE_4, litptr->name);
            }

          litptr->sec =
            qdsp6_create_literal_section (litptr->secname, SEC_LINK_ONCE, bfd_log2 (size));
          litptr->sub = LITERAL_SUBSECTION + (size == 8? 2: 1);
        }
      else /* if (litptr->e.X_op == O_symbol) */
        /* Create a label symbol (with the literal order as the suffix). */
        {
          sprintf (litptr->name, "%s_%04lx", LITERAL_PREFIX, litpoolcounter);
          strcpy (litptr->secname, LITERAL_SECTION_A);

          litptr->sec = qdsp6_create_literal_section (litptr->secname, 0, bfd_log2 (size));
          litptr->sub = LITERAL_SUBSECTION;
        }

      /* Group the literals per size and type in separate subsections
         in order to minimize wasted space due to alignment. */
      subseg_set (litptr->sec, litptr->sub);

      /* Set the alignment. */
      frag_align (bfd_log2 (size), 0, 0);

      /* Define the symbol. */
      litptr->sym =
        symbol_new (litptr->name, now_seg, (valueT) frag_now_fix (), frag_now);
      symbol_table_insert (litptr->sym);

      /* Set the size and type. */
      S_SET_SIZE (litptr->sym, size);
      symbol_get_bfdsym (litptr->sym)->flags |= BSF_OBJECT;

      /* Set the scope. */
      if (litptr->e.X_op == O_constant)
        S_SET_EXTERNAL (litptr->sym);

      /* Emit the data definition. */
      emit_expr (&(litptr->e), size);

      /* Restore the previous section. */
      subseg_set (current_section, current_subsec);

      litpoolcounter++;

      /* Add literal to the literal pool list. */
      if (litpool != litptr)
        lastptr->next = litptr;
    }

  return (litptr);
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
      if (   rm_right [1].rm_so < 0 || rm_right [1].rm_eo < 0
          || (er_re = regexec (&re_r32, str, 1, rm_left, 0)))
        return FALSE;
      else
        size = 4;
    }
  else if (!(er_re = regexec (&re_c64, str, 2, rm_right, 0)))
    {
      if (   rm_right [1].rm_so < 0 || rm_right [1].rm_eo < 0
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
    snprintf (new_str, MAX_MAPPED_INSN_LEN, "%.*s = mem%c (#%s)",
              rm_left [0].rm_eo - rm_left [0].rm_so, str,
              size == 4? 'w' : 'd', litcurptr->name);
  else
    return FALSE;

  return TRUE;
}

 /* This routine is called for each instruction to be assembled.  */
static void
qdsp6_md_assemble(char *str, int padded)
{
  const qdsp6_opcode *std_opcode;
  char *start;
  const char *last_errmsg = 0;
  static int init_tables_p = 0;
  int is_id;
  /* to store the new instruction string for following instructions:
   * Rd32 = CONST32(#imm), Rd32 = CONST32(label)
   * Rdd32 = CONST64(#imm), Rdd32 = CONST64(label)
   */
  char new_str [MAX_MAPPED_INSN_LEN];
  /* For instruction mapping */
  char mapped_insn_str [MAX_MAPPED_INSN_LEN];
  int operand_val;
  int operand_vals [MAX_OPERANDS];
  size_t operand_idx;

  /* Opcode table initialization is deferred until here because we have to
     wait for a possible .option command.  */
  if (!init_tables_p)
    {
      init_opcode_tables (qdsp6_mach_type);
      init_tables_p = 1;
    }

  if (qdsp6_packets [0].size >= MAX_PACKET_INSNS)
    {
      as_bad (_("too many instructions in packet (maximum is %d)."),
              MAX_PACKET_INSNS);
      return;
    }


  /* Skip leading white space.  */
  while (ISSPACE (*str))
    str++;

  /* special handling of GP related syntax:
   * Rd32 = CONST32(#imm) or Rd32 = CONST32(label)
   * Rdd32 = CONST64(#imm) or Rdd32 = CONST64(label)
   */
  if (qdsp6_gp_const_lookup (str, new_str) == 1)
    str = new_str;

  /* The instructions are stored in lists hashed by the first letter (though
     we needn't care how they're hashed).  Get the first in the list.  */
  std_opcode = qdsp6_opcode_lookup_asm (str);

  /* Keep looking until we find a match.  */
  start = str;
  for (qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode = std_opcode;
       qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode != NULL;
       qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode
         = QDSP6_OPCODE_NEXT_ASM
	     (qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode))
    {
      char *syn;

      /* Is this opcode supported by the selected cpu?  */
      if (!qdsp6_opcode_supported (qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode))
	continue;

      /* Scan the syntax string.  If it doesn't match, try the next one.  */
      qdsp6_packets [0].insns [qdsp6_packets [0].size].insn
        = qdsp6_encode_opcode
	    (qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode->enc);
      qdsp6_packets [0].insns [qdsp6_packets [0].size].fc = 0;
      is_id = 0;
      operand_idx = 0;

      for (str = start,
	   syn = qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode->syntax;
           *syn != '\0' && *str != '\0'; )
        {
          if ((ISUPPER (*syn) && (*syn != 'I')) || *syn == '#')
	    {
	      /* We have an operand */
	      const qdsp6_operand *operand = qdsp6_lookup_operand (syn);

	      if (operand)
	        {
		  size_t len = strlen(operand->fmt);
		  char *errmsg = NULL;

		  if (operand->flags & QDSP6_OPERAND_IS_IMMEDIATE)
		    {
		      str
		        = qdsp6_parse_immediate
			    (qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode,
			     operand, str,
			     &qdsp6_packets [0].insns [qdsp6_packets [0].size].insn,
			     &operand_val);

		      if (!str)
			goto NEXT_OPCODE;
                    }
                  else
                    {
                      str
		        = operand->parse
			    (operand,
			     &qdsp6_packets [0].insns [qdsp6_packets [0].size].insn,
			     qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode->enc,
			     str, &errmsg, &operand_val);
                      if (!str)
                        {
                          /* This instruction doesn't match the syntax */
                          if (errmsg)
                            as_bad (errmsg);

                          goto NEXT_OPCODE;
                        }
                      //An operand just parsed
                    }

                  // RK. Store the operand value
                  operand_vals[operand_idx++] = operand_val;

                  /* Move past the opcode specifier */
                  syn += len;
                  is_id = 0;
                }
              else
		as_bad (_("operand not found for `%s'."), syn);
            }
          else
            {
              /* Non operand chars must match exactly.  */
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
              if (   is_id
                  && (is_name_beginner (*syn) || *syn == '#')
                  && !ISSPACE (*str))
                goto NEXT_OPCODE;

              while (ISSPACE (*str))
                str++;
            }
        }

      /* If we're at the end of the syntax string, we're done.  */
      if (*syn == '\0')
        {
          char *cur_file;
          unsigned int cur_line;

          /* RK: Do remap */
          if (qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode->map_func != NULL)
            {
              qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode->map_func (mapped_insn_str, operand_vals);

              // Concatenate the remaining chars in "str"
              strcat (mapped_insn_str, str);
              qdsp6_md_assemble (mapped_insn_str, padded);

              return;
            }

          /* QDSP6 TODO: Perform various error and warning tests.  */

          /* Find where we are, will be used to dump to the dwarf2 debug info*/
          as_where (&cur_file, &cur_line);

          //	printf("cur_file=%s, cur_line=%u\n", cur_file, cur_line);

          if (qdsp6_in_packet)
            {
              if (qdsp6_packet_reverse)
                {
                  /*
                   * Check the issue slot
                   * This might seem backward, but the bits in the opcode's
                   * issue slot mask are based on an old ordering which is
                   * the reverse of the current correct ordering.
                   *
                   * Note that the default case (below) uses the reverse
                   * of this ordering.
                  */
                  unsigned int slot = qdsp6_packets [0].size;
                  unsigned int slot_mask = 1 << slot;

                  if ((qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode->slot_mask & slot_mask) == 0)
                    as_bad (_("instruction not valid in this issue slot "
                              "(ordering reversed)"));
                }
              else if (qdsp6_packet_shuffle)
                {
                  /* Nothing to do - we'll rearrange them later */
                }
              else if (qdsp6_packet_native)
                {
                  /* Check the issue slot */
                  unsigned int slot = MAX_PACKET_INSNS
                                    - qdsp6_packets [0].size - 1;
	          unsigned int slot_mask = 1 << slot;

                  if ((qdsp6_packets [0].insns [qdsp6_packets [0].size].opcode->slot_mask & slot_mask) == 0)
                    as_bad (_("instruction not valid in this issue slot."));
                }
              else
                {
                  as_fatal (_("unknown packet ordering."));
                }

	      qdsp6_packets [0].insns [qdsp6_packets [0].size].used = 0;
	      qdsp6_packets [0].insns [qdsp6_packets [0].size].padded = padded;
	      qdsp6_packets [0].insns [qdsp6_packets [0].size].lineno = cur_line;

	      qdsp6_packets [0].size++;
            }
	  else
            {
              qdsp6_check_single_insn
	        (qdsp6_packets [0].insns [0].opcode,
		 qdsp6_packets [0].insns [0].insn);

              qdsp6_packets [0].faligned = faligning;
              qdsp6_packets [0].is_inner = FALSE;
              qdsp6_packets [0].is_outer = FALSE;

              qdsp6_packets [0].insns [0].used = 0;
              qdsp6_packets [0].insns [0].padded = 0;
              qdsp6_packets [0].insns [0].lineno = cur_line;

              qdsp6_packets [0].size = 1;

              // Set the packet end bits for a single instructions
              qdsp6_packets [0].insns [0].loc.ptr
                = qdsp6_insn_write
		    (qdsp6_packets [0].insns [0].insn | QDSP6_END_PACKET,
		     qdsp6_packets [0].insns [0].fc,
                     qdsp6_packets [0].insns [0].fixups, 0,
		     &qdsp6_packets [0].insns [0].fix, cur_line);
	      qdsp6_packets [0].insns [0].loc.frag = frag_now;
              qdsp6_packets [0].insns [0].loc.offset
	        = qdsp6_packets [0].insns [0].loc.ptr - frag_now->fr_literal;

              qdsp6_history_push (qdsp6_packets + 0);

              faligning = 0;
            }

          while (ISSPACE (*str))
            ++str;

          /* Check for the packet end string */
          if (!strncmp (str, PACKET_END, strlen (PACKET_END)))
            {
              qdsp6_packet_end ();
              str += strlen (PACKET_END);

              while (ISSPACE (*str))
                ++str;

              /* Check for the end inner/outer modifiers.
                 Note that they can appear in any order. */
              while (1)
                {
                  if (!strncasecmp (str, PACKET_END_INNER,
                                    strlen (PACKET_END_INNER)))
                    {
                      qdsp6_packet_end_inner ();
                      str += strlen (PACKET_END_INNER);

                      while (ISSPACE (*str))
                        ++str;
                    }
                  else if (!strncasecmp (str, PACKET_END_OUTER,
			                  strlen (PACKET_END_OUTER)))
                    {
                      qdsp6_packet_end_outer ();
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
                  as_bad (_("junk at end of line: `%s'"), str);
                }
              else if (!qdsp6_packets [0].is_inner || !qdsp6_packets [0].is_outer)
                {
                  /* May need to lookahead in the input stream for
                     (more) inner/outer modifiers. */
                  int inner, outer;

                  qdsp6_packet_end_lookahead (&inner, &outer); /* may make 'str' invalid */
                  --input_line_pointer;

                  if (inner)
                    qdsp6_packet_end_inner ();

                  if (outer)
                    qdsp6_packet_end_outer ();
                }

              qdsp6_packet_write ();
            }
          else if (!is_end_of_line [(unsigned char) *str])
            {
              as_bad (_("junk at end of line: `%s'"), str);
            }

          /* All done.  */
          return;
        }

      /* Try the next entry.  */
      NEXT_OPCODE:
        continue;
    }

  if (NULL == last_errmsg)
    {
      int big = strlen (start) > MAX_MESSAGE - 3? TRUE: FALSE;

      as_bad (_("bad instruction `%.*s%s'"),
              big? MAX_MESSAGE - 3: MAX_MESSAGE, start, big? "...": "");
    }
  else
    as_bad (last_errmsg);
}

/*
 * XXX_SM this is slight change to the Q6 base to save having to
 * change the baseline's definition of md_assemble.
 */
void
md_assemble (char *str)
{
    qdsp6_md_assemble (str, 0);
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
      as_bad (_("negative symbol length."));
      ignore_rest_of_line ();
      return;
    }

  *p = 0;
  symbolP = symbol_find_or_make (name);
  *p = c;

  if (S_IS_DEFINED (symbolP) && !S_IS_COMMON (symbolP))
    {
      as_bad (_("attempt to re-define symbol `%s'."), S_GET_NAME (symbolP));
      ignore_rest_of_line ();
      return;
    }

  if (   (S_GET_VALUE (symbolP) != 0)
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
        as_warn (_("negative alignment, defaulting to %ld."),
                 (long) (align = 0));
    }

  /* Convert alignment to a power of 2,
     if not provided, use the natural size alignment. */
  temp = align? align: MIN (size, MAX_DATA_ALIGNMENT);

  align2 = bfd_log2 (temp);
  alignb = 1 << align2;

  if (align && alignb != temp)
    as_warn (_("alignment not a power of 2, defaulting to %ld."),
             (long) (alignb = 1 << ++align2));

  /* Start assuming that the access is 0, i.e.
     use the natural access for the symbol size and alignment. */
  access = 0;
  /* Now parse the optional alignment field. */
  if (*input_line_pointer == ',')
    {
      input_line_pointer++;
      access = get_absolute_expression ();

      if (access < 0)
        as_warn (_("negative access, defaulting to %ld."),
                 (long) (access = 0));

      if (access != (1 << bfd_log2 (access)))
        as_warn (_("access not a power of 2, defaulting to %ld."),
                 (long) (access = 0));
    }

  if (!access)
    access = 1 << MIN (bfd_log2 (size), align2);

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
      as_bad ("\".option\" must appear before any instructions");
      ignore_rest_of_line ();
      return;
    }

  if (mach < 0)
    {
      as_bad (_("invalid identifier for \".option\"."));
      ignore_rest_of_line ();
      return;
    }
  else if (mach_type_specified_p && mach != qdsp6_mach_type)
    {
      as_bad ("\".option\" conflicts with initial definition");
      ignore_rest_of_line ();
      return;
    }
  else
    {
      /* The cpu may have been selected on the command line.  */
      if (mach != qdsp6_mach_type)
	as_warn (_("\".option\" overrides the command-line or the default value."));
      qdsp6_mach_type = mach;

      if (!bfd_set_arch_mach (stdoutput, bfd_arch_qdsp6, mach))
	as_fatal (_("could not set architecture and machine."));

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
md_atof(
     int type,
     char *litP,
     int *sizeP
)
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

/** Write a value out to the object file using the appropriate endianness.
*/
void
md_number_to_chars
(char *buf, valueT val, int n)
{
  if (target_big_endian)
    number_to_chars_bigendian (buf, val, n);
  else
    number_to_chars_littleendian (buf, val, n);
}

/* Round up a section size to the appropriate boundary.  */
valueT
md_section_align
(segT segment, valueT size)
{
  int align = bfd_get_section_alignment (stdoutput, segment);

  return ((size + (1 << align) - 1) & (-1 << align));
}

/* We don't have any form of relaxing.  */
int
md_estimate_size_before_relax(
     fragS *fragp ATTRIBUTE_UNUSED,
     asection *seg ATTRIBUTE_UNUSED
)
{
  as_fatal (_("md_estimate_size_before_relax."));
  return 1;
}

/* Convert a machine dependent frag.  We never generate these.  */

void
md_convert_frag(
     bfd *abfd ATTRIBUTE_UNUSED,
     asection *sec ATTRIBUTE_UNUSED,
     fragS *fragp ATTRIBUTE_UNUSED
)
{
  as_fatal (_("md_convert_frag."));
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
      as_bad (_("expression too complex code symbol."));
      return;
    }
}

/* Parse an operand that is machine-specific. */

void
md_operand(
    expressionS *expressionP ATTRIBUTE_UNUSED
)
{
#if 0
  char *str = input_line_pointer;
  char ch;
  int base = 10;
  unsigned int u_value = 0;

  if (str[0] == '#' && ISDIGIT(str[1])) {
    /* Read the number */
    ch = *++str;
    while (   ISDIGIT(ch)
           || (base == 16 && ch >= 'a' && ch <= 'f')) {
      if (ISDIGIT(ch)) {
        u_value = base*u_value + (ch - '0');
      }
      else {
        u_value = base*u_value + (ch - 'a' + 10);
      }

      ch = *++str;
    }

    expressionP->X_op = O_constant;
    expressionP->X_add_number = u_value;

    input_line_pointer = str;
  }
  else {
    printf("md_operand called: %s\n", input_line_pointer);
    abort();
  }
#else
    printf("md_operand called: %s\n", input_line_pointer);
    abort();
#endif
}

/* We have no need to default values of symbols. */

symbolS *
md_undefined_symbol(
     char *name ATTRIBUTE_UNUSED
)
{
#if 0
  printf("md_undefined_symbol called: %s\n", name);
#endif
  return 0;
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
/* 
 * XXX:SM change to use the #define version in expr.h
 * Was expr(0,exp) if we don't want to use the macro then then
 * 2.19 version would be: "expr (0, exp, expr_normal)"
 */
  expression (exp);
  if (code_symbol_fix)
    {
      qdsp6_code_symbol (exp);
      input_line_pointer = p;
    }
}

/* Record a fixup for a cons expression.  */

#if 0
void
qdsp6_cons_fix_new(
     fragS *frag,
     int where,
     int nbytes,
     expressionS *exp
)
{
  if (nbytes == 4)
    {
      int reloc_type;
      expressionS exptmp;

      /* This may be a special QDSP6 reloc (eg: %st()).  */
      reloc_type = get_qdsp6_exp_reloc_type (1, BFD_RELOC_32, exp, &exptmp);
      fix_new_exp (frag, where, nbytes, &exptmp, 0, reloc_type);
    }
  else
    {
      fix_new_exp (frag, where, nbytes, exp, 0,
		   nbytes == 2 ? BFD_RELOC_16
		   : nbytes == 8 ? BFD_RELOC_64
		   : BFD_RELOC_32);
    }
}
#endif

/* Functions concerning relocs.  */

/* The location from which a PC relative jump should be calculated,
   given a PC relative reloc.  */

long
md_pcrel_from (fixS *fixP)
{
  return (fixP->fx_frag->fr_address + fixP->fx_where);
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
    as_bad_where (fixP->fx_file, fixP->fx_line, _("expression too complex"));

  operand = (qdsp6_operand *) fixP->tc_fix_data;
  if (operand)
    {
      /* Fetch the instruction, insert the fully resolved operand
        value, and stuff the instruction back again.  */
      if (fixP->fx_done)
        qdsp6_insert_operand (fixP->fx_frag->fr_literal + fixP->fx_where,
                              operand, (offsetT) value, fixP);
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
tc_gen_reloc(
     asection *section ATTRIBUTE_UNUSED,
     fixS *fixP
)
{
  arelent *reloc;

  reloc = (arelent *) xmalloc (sizeof (arelent));

  reloc->sym_ptr_ptr = (asymbol **) xmalloc (sizeof (asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixP->fx_addsy);
  reloc->address = fixP->fx_frag->fr_address + fixP->fx_where;
  reloc->howto = bfd_reloc_type_lookup (stdoutput, fixP->fx_r_type);
  if (reloc->howto == (reloc_howto_type *) NULL) {
    as_bad_where (fixP->fx_file, fixP->fx_line,
                  "internal error: can't export reloc type %d (\"%s\")",
                  fixP->fx_r_type,
                  bfd_get_reloc_code_name (fixP->fx_r_type));
    return NULL;
  }

  assert (!fixP->fx_pcrel == !reloc->howto->pc_relative);

  /* fixP->fx_offset was not handled in md_apply_fix() so we
     need to propagate it into the (RELA) relocation */
  reloc->addend = fixP->fx_offset;

  return reloc;
}

//Look for unused noslot1 instruction, if found, return its index,
//otherwise return -1.
int
qdsp6_find_noslot1
(qdsp6_packet_insn input [], int current)
{
  int i;

  for (i = 0; i < MAX_PACKET_INSNS; i++)
      if( i != current && !input[i].used )
      if (input [i].opcode->attributes & A_RESTRICT_NOSLOT1)
	return (i);

  return -1;
}

/** Shuffle packet accoding to from-to mapping.

@param packet reference.
@param fromto mapping array.
*/
void
qdsp6_shuffle_packet
(qdsp6_packet *packet, size_t *fromto)
	{
  qdsp6_packet_insn insns [MAX_PACKET_INSNS];
  size_t to;
  size_t i;

  for (i = 0; i < MAX_PACKET_INSNS; i++)
    {
      to = fromto [i];
      if (to >= MAX_PACKET_INSNS)
	to = i;

      /* Shuffle insns, fix-ups... */
      insns [i]     = packet->insns [to];
      /* But not frags. */
      insns [i].loc = packet->insns [i].loc;
    }

  memcpy (packet->insns, insns, sizeof (packet->insns));
}

/** Shuffle a packet according to architectural restrictions.

@param packet Packet reference.
@param slot_num Slot number (initially called for slot 0).
@param shuffle Optional pre-initialized from-to helper array (typically NULL).
@return True if packet was shuffled succesfully.
*/
int
qdsp6_do_shuffle_helper
(qdsp6_packet *packet, size_t slot_num, size_t *shuffle)
{
  size_t slot = MAX_PACKET_INSNS - slot_num - 1;
  size_t slot_mask = 1 << slot;
  size_t tmp_slot_mask;
  size_t *fromto, aux [MAX_PACKET_INSNS];
  int single_memop = qdsp6_find_single_memop (packet);
  int changed;
  size_t i;

  if (slot_num >= packet->size)
    return TRUE;

  /* Choose from-to map. */
  if (shuffle)
    fromto = shuffle;
  else
    {
      for (i = 0; i < MAX_PACKET_INSNS; i++)
	aux [i] = MAX_PACKET_INSNS;

      fromto = aux;
    }

  for (i = 0, changed = FALSE;
       i < packet->size && !changed;
       i++)
    {
          /* check if there is only one memory op
           * (A_RESTRICT_SINGLE_MEM_FIRST). If so, it requires
           * the slot 0 (slot_mask is 1) */
      if (   single_memop
	  && (packet->insns [i].opcode->attributes & A_RESTRICT_SINGLE_MEM_FIRST))
	tmp_slot_mask = packet->insns [i].opcode->slot_mask & 1;
      else
	tmp_slot_mask = packet->insns [i].opcode->slot_mask;

      if (!packet->insns [i].used && (tmp_slot_mask & slot_mask))
	{
	  /* Check for NOSLOT1 restriction. */
	  if (slot == 1 && !qdsp6_is_nop (packet->insns [i].insn))
	    if (qdsp6_find_noslot1 (packet->insns, i) >= 0)
	      /* If so, then skip this slot. */
	      continue;

          /* Allocate this slot. */
	  fromto [slot_num] = i;
	  packet->insns [i].used = TRUE;

	  if ((changed = qdsp6_do_shuffle_helper (packet, slot_num + 1, fromto)))
	    break;
          else
	    packet->insns [i].used = FALSE;
        }
      }

  /* Shuffle packet if successful and at the top-most call. */
  if (changed && !shuffle)
    qdsp6_shuffle_packet (packet, fromto);

  return (changed);
}


/** For V2, discard specified number of DCFETCH.

It must not be called after packet has been written out.
@param number # DCFETCH to discard, with "0" meaning all but one.
@return # replacements performed.
 */
int
qdsp6_discard_dcfetch
(int number)
{
  int count = 0, found = 0;
  size_t i;

    // dcfetch fix:
    // for V2, there cannot be 2 or more dcfetch instructions
    // and a dcfetch cannot go into slot 0
  if (!qdsp6_if_arch_v1 () && qdsp6_packets [0].size > 1)
    {
      if (!number)
        /* Replace all but one DCFETCH instruction. */
        {
          for (i = 0; i < qdsp6_packets [0].size; i++)
            {
              if (!strncasecmp (qdsp6_packets [0].insns [i].opcode->syntax,
                                QDSP6_DCFETCH, QDSP6_DCFETCH_LEN))
                {
                  found++;

                  if (found > MAX_DCFETCH)
                    as_warn (_("too many `dcfetch' in packet."));

                  if (found > 1)
                    /* Delete extra DCFETCH. */
                    {
                      as_warn_where (NULL, qdsp6_packets [0].insns [i].lineno,
                                    _("extra `dcfetch' removed."));

                      qdsp6_packets [0].insns [i]        = qdsp6_nop_insn;
                      qdsp6_packets [0].insns [i].padded = TRUE;

                      count++;
                    }
                }
            }
        }
      else
        /* Replace specified number of DCFETCH. */
        {
          for (i = 0; number && i < qdsp6_packets [0].size; i++)
            {
              if (!strncasecmp (qdsp6_packets [0].insns [i].opcode->syntax,
                                QDSP6_DCFETCH, QDSP6_DCFETCH_LEN))
                {
                  found++;

                  as_warn_where (NULL, qdsp6_packets [0].insns [i].lineno,
                                _("`dcfetch' removed."));

                  qdsp6_packets [0].insns [i]        = qdsp6_nop_insn;
                  qdsp6_packets [0].insns [i].padded = TRUE;

                  number--;
                  count++;
                }
            }
        }
    }

  return (count);
}

void
qdsp6_handle_shuffle
(size_t fromto [])
{
  if (qdsp6_do_shuffle_helper (qdsp6_packets + 0, 0, fromto))
    qdsp6_shuffle_packet (qdsp6_packets + 0, fromto);
  else
    {
      /* Try to discard a DCFETCH before trying again. */
      if (qdsp6_discard_dcfetch (1))
        qdsp6_handle_shuffle (fromto);
      else
	as_bad (_("unable to shuffle instructions in packet."));
    }
}

/* Top level function for packet shuffling
     1) Set up the recursive call
     2) Rearrange the instructions
     3) Reports an error if necessary
 */

    void
qdsp6_do_shuffle
(void)
{
  qdsp6_packet_insn insn;
  size_t fromto [MAX_PACKET_INSNS];
  char *file;
  char found, has_prefer_slot0;
  size_t i, j, k;

  if (!qdsp6_if_arch_v1 ())
{
      /* Get rid of extra insns. */
      qdsp6_discard_dcfetch (0);

      /* Initialize shuffle map. */
      for (i = 0; i < MAX_PACKET_INSNS; i++)
        fromto [i] = MAX_PACKET_INSNS;

      // to reorder the instructions in the packet so it
      // starts with slot 3 instead of slot 0 when shuffle
      // 1. first, go through all non-padded instructions, find
      // the highest slot each instruction can go to, and put
      // it in the corresponding position (slot 3 goes to index 0)
      // if an index is already assigned, then go to the next one
      // if all the positions after the wanted one are taken, then
      // go back to the beginning of the array
      // 2. for all padded instructions, put them one by one starting
      // from the first available position (index 0 -> 3)

      // step 1
      for (i = 0; i < MAX_PACKET_INSNS; i++)
	{
          if (!qdsp6_packets [0].insns [i].padded)
	    {
              /* Find the highest slot this instruction can go to. */
              for (j = 0, found = FALSE;
	           j < MAX_PACKET_INSNS && !found;
	           j++)
		{
                  if (qdsp6_packets [0].insns [i].opcode->slot_mask & (1 << (MAX_PACKET_INSNS - j - 1)))
		    {
                      for (k = j;
                           k < MAX_PACKET_INSNS && !found;
		           k++)
			if (fromto [k] >= MAX_PACKET_INSNS)
			  {
			    fromto [k] = i;
			    found = TRUE;
                      }
                      break;
                  }
		}

              // if all the positions after the wanted one are taken
              // go back to the beginning of the array
	      for (j = 0;
		   j < MAX_PACKET_INSNS && !found;
		   j++)
		if (fromto [j] >= MAX_PACKET_INSNS)
		  {
		    fromto [j] = i;
		    found = TRUE;
              }
          } // end if (!...padded)
      } // end for (i=0 ...)

      // step 2
      for (i = 0; i < MAX_PACKET_INSNS; i++)
	{
          if (qdsp6_packets [0].insns [i].padded)
	    {
              // find the first unused index and put this padded
              // instruction there
              for (j = 0; j < MAX_PACKET_INSNS; j++)
		{
                  if (fromto [j] >= MAX_PACKET_INSNS)
		    {
                      fromto [j] = i;
                      break;
                  }
              }
          }
      } // end for (i=0; ...)

      qdsp6_shuffle_packet (qdsp6_packets + 0, fromto);

      for (i = 0, has_prefer_slot0 = FALSE;
           i < MAX_PACKET_INSNS && !has_prefer_slot0;
           i++)
	{
          if (qdsp6_packets [0].insns [i].opcode->attributes & A_RESTRICT_PREFERSLOT0)
	    {
              has_prefer_slot0 = TRUE;
              // Place the prefer_slot0 insn at index (MAX_PACKET_INSNS - 1), by swapping i
			  // and (MAX_PACKET_INSNS - 1)
              insn = qdsp6_packets [0].insns [i];
              qdsp6_packets [0].insns [i] = qdsp6_packets [0].insns [MAX_PACKET_INSNS - 1];
              qdsp6_packets [0].insns [MAX_PACKET_INSNS - 1] = insn;
          }
      }
  }

  qdsp6_handle_shuffle (fromto);

  if (has_prefer_slot0)
    {
      as_where (&file, NULL);

      for (i = 0; i < MAX_PACKET_INSNS - 1; i++)
	  if (qdsp6_packets [0].insns [i].opcode->attributes & A_RESTRICT_PREFERSLOT0)
	      as_warn_where (file, qdsp6_packets [0].insns [i].lineno,
	                     _("instruction `%s' prefers slot #0, but has been assigned to slot #%lu."),
			     qdsp6_packets [0].insns [i].opcode->syntax, i);
    }
}

/* Determine if an instruction is a nop */

int
qdsp6_is_nop
(qdsp6_insn insn)
{
  const qdsp6_opcode *opcode = qdsp6_lookup_insn(insn);

  return (!strcasecmp (opcode->syntax, QDSP6_NOP));
}

qdsp6_insn
qdsp6_find_nop
(void)
{
  const char nop [] = QDSP6_NOP;
  const qdsp6_opcode *opcode = qdsp6_opcode_lookup_asm (nop);

  /* Keep looking until we find a match.  */
  for (; opcode != NULL; opcode = QDSP6_OPCODE_NEXT_ASM (opcode))
    if (!strcasecmp (opcode->syntax, nop))
      break;

  assert(opcode != NULL);
  return (qdsp6_encode_opcode (opcode->enc));
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
    if (pNewArray [i] && !pArray [i])
      {
        if (cArray [QDSP6_P30])
          as_warn (_("modifying `c4/p3:0' does not apply to `p%lu.new'."),
                   (unsigned long) i);

        as_bad
          (_("register `p%lu' used with `.new' but not modified in the same packet."),
           (unsigned long) i);

        return FALSE;
      }

  return TRUE;
}

void
qdsp6_check_implicit
(const qdsp6_opcode *opcode, unsigned int implicit, int reg, int *array, const char *name)
{
  if (opcode->implicit_reg_def & implicit)
    {
      if (array [reg])
	as_bad (_("register `%s' modified multiple times."), name);
      else
	array [reg]++;
    }
}

void
qdsp6_check_register(
    int array[],
    int reg_num,
	int pred_reg_rd_mask,
    const qdsp6_operand *operand,
    qdsp6_insn insn,
    const qdsp6_opcode *opcode
)
{
  char *errmsg = NULL;
  char buff[100];
  char *reg_name;
  int prev_pred_reg_rd_mask;
  int mult_wr_err = 0;
  int i;
  int a_pred_mask;
  int prev_a_pred_mask;

  if(array[reg_num]){
	prev_pred_reg_rd_mask = array[reg_num] & PRED_REG_INFO_MASK;

	// If any mask is zero. Note: This handles V1 since masks are zero
	// for V1
	if ((pred_reg_rd_mask == 0) || (prev_pred_reg_rd_mask == 0))
		mult_wr_err = 1;
	else {
		// Or same non-zero mask for same predicate
		for (i = 0; i < QDSP6_NUM_PREDICATE_REGS; i++) {
			a_pred_mask = (pred_reg_rd_mask >> (COND_EXEC_POS + (i * COND_EXEC_INFO_NUM_BITS))) & COND_EXEC_INFO_MASK;
			prev_a_pred_mask = (prev_pred_reg_rd_mask >> (COND_EXEC_POS + (i * COND_EXEC_INFO_NUM_BITS))) & COND_EXEC_INFO_MASK;
			if ((a_pred_mask == prev_a_pred_mask) && (a_pred_mask != 0)) {
				mult_wr_err = 1;
				break;
			}
		}
	}

	if (mult_wr_err)
	  {
	    reg_name
	      = qdsp6_dis_operand (operand, insn, 0, opcode->enc, buff, &errmsg);

	    if (reg_name)
	      as_bad (_("register `%s' modified multiple times."), buff);
	    else if (errmsg)
	      as_bad (errmsg);
	  }
	else
	  array [reg_num] |= pred_reg_rd_mask | 1;
  }
  else
    array [reg_num] = pred_reg_rd_mask | 1;
}

void
qdsp6_check_insn
(const qdsp6_opcode *opcode, qdsp6_insn insn)
{
  char *errmsg = NULL;
  int reg_num;
  int *arrayPtr;
  char *cp;
  int pred_reg_rd_mask = 0;
  int pred_reg_offset;
  int is_arch_v1 = qdsp6_if_arch_v1 ();

  // Check whether the instruction is legal inside the packet
  // But allow single instruction in packet
  if(   (opcode->attributes & A_RESTRICT_NOPACKET)
     && num_inst_in_packet_from_src > 1)
    {
      as_bad (_("illegal instruction in packet."));
      qdsp6_in_packet = 0;
      return;
    }

  //Check loop can not exist in the same packet as branch label instructions
  //they are using the same adder
  if ((opcode->attributes & A_RESTRICT_BRANCHADDER_MAX1))
    numOfBranchAddrMax1++;

  if ((opcode->attributes & A_BRANCHADDER))
    numOfBranchAddr++;

  if ((opcode->attributes & A_RESTRICT_COF_MAX1))
    numOfBranchMax1++;

  if (   (opcode->attributes & A_RELAX_COF_1ST)
      || (opcode->attributes & A_RELAX_COF_2ND))
    numOfBranchRelax++;

  if ((opcode->attributes & A_RESTRICT_LOOP_LA))
    numOfLoopMax1++;

  // check for implicit register definitions
  if (opcode->implicit_reg_def)
    {
      if (opcode->implicit_reg_def & IMPLICIT_PC)
        {
          /* Look into multiple implicit references to the PC in order to allow
             slots with two branches in V3. */
          cArray [QDSP6_PC]++;
          if (cArray [QDSP6_PC] > 1 && cArray [QDSP6_PC] > numOfBranchRelax)
            qdsp6_check_implicit (opcode, IMPLICIT_PC, QDSP6_PC, cArray, "c9/pc");
        }

      qdsp6_check_implicit (opcode, IMPLICIT_LR,  QDSP6_LR,  gArray, "r31/lr");
      qdsp6_check_implicit (opcode, IMPLICIT_SP,  QDSP6_SP,  gArray, "r29/sp");
      qdsp6_check_implicit (opcode, IMPLICIT_FP,  QDSP6_FP,  gArray, "r30/fp");
      qdsp6_check_implicit (opcode, IMPLICIT_LC0, QDSP6_LC0, cArray, "c1/lc0");
      qdsp6_check_implicit (opcode, IMPLICIT_SA0, QDSP6_SA0, cArray, "c0/sa0");
      qdsp6_check_implicit (opcode, IMPLICIT_LC1, QDSP6_LC1, cArray, "c3/lc1");
      qdsp6_check_implicit (opcode, IMPLICIT_SA1, QDSP6_SA1, cArray, "c2/sa1");
      qdsp6_check_implicit (opcode, IMPLICIT_P3,  3,         pArray, "p3");
      qdsp6_check_implicit (opcode, IMPLICIT_P0,  0,         pArray, "p0"); /* V3 */
    }

  // check for attributes (currently checking for only OVF bit
  if ((opcode->attributes) && (opcode->attributes & A_RESTRICT_NOSRMOVE))
    implicit_sr_ovf_bit_flag = 1;

  for (cp = opcode->syntax; *cp != '\0'; cp++)
    {
      // Walk the syntax string for the opcode
      if (ISUPPER (*cp))
        {
          // Check for register operand
          // Get the operand from operand lookup table
          const qdsp6_operand *operand = qdsp6_lookup_operand (cp);
          if(operand == NULL)
            break;

          cp += strlen (operand->fmt) - 1; // Move the pointer to the end of the operand

          if (   is_arch_v1 == 0
              && (operand->flags & QDSP6_OPERAND_IS_READ)
              && (operand->flags & QDSP6_OPERAND_IS_PREDICATE_REG))
            {
              if (!qdsp6_extract_predicate_operand (operand, insn, opcode->enc, &reg_num, &errmsg))
                {
                  if (errmsg)
                    as_bad (errmsg);
                  break;
                }

              pred_reg_offset = COND_EXEC_POS + (reg_num * COND_EXEC_INFO_NUM_BITS);
              pred_reg_rd_mask = 1 << pred_reg_offset;

              if (opcode->attributes & CONDITION_SENSE_INVERTED)
                pred_reg_rd_mask |= 1 << (pred_reg_offset + COND_EXEC_INV_OFF);

              if (opcode->attributes & CONDITION_DOTNEW)
                {
                  pred_reg_rd_mask |= 1 << (pred_reg_offset + COND_EXEC_DOT_NEW_OFF);
                  pNewArray [reg_num] = 1;
                }
            }

          if (operand->flags & QDSP6_OPERAND_IS_WRITE)
            {
              // check whether it is a predicate register
              if (operand->flags & QDSP6_OPERAND_IS_PREDICATE_REG)
                {
                  arrayPtr = pArray;

                  if (!qdsp6_extract_predicate_operand (operand, insn, opcode->enc, &reg_num, &errmsg))
                    {
                      if (errmsg)
                        as_bad (errmsg);
                      break;
                    }

                  if (cArray [QDSP6_P30])
                    {
                      as_bad (_("register `p%d' modified multiple times."), reg_num);
                      break;
                    }
                }
              // check whether it is a modifier register
              else if (operand->flags & QDSP6_OPERAND_IS_MODIFIER_REG)
                {
                  arrayPtr = cArray;

                  if (!qdsp6_extract_modifier_operand (operand,insn,opcode->enc,&reg_num,&errmsg))
                    {
                      if (errmsg)
                        as_bad (errmsg);
                      break;
                    }
                }
              else
                {
                  // Call qdsp6_extract_operand to get the operand
                  if (!qdsp6_extract_operand(operand, insn, 0, opcode->enc, &reg_num, &errmsg))
                    {
                      if (errmsg)
                        as_bad (errmsg);
                      break;
                    }

                  // check if it is a control register
                  if (operand->flags & QDSP6_OPERAND_IS_CONTROL_REG)
                    {
                      if (reg_num==QDSP6_P30)
                        {
                          // Set pArray completely
                          size_t j;
                          for (j = 0; j < QDSP6_NUM_PREDICATE_REGS; j++)
                            if (pArray [j])
                              {
                                //as_bad (("register `c%d' modified multiple times."), reg_num);
                                cArray[reg_num] = 1;
                                break;
                              }
                        }
                      arrayPtr = cArray;
                    }
                  // check whether it is a system control register
                  else if (operand->flags & QDSP6_OPERAND_IS_SYSCONTROL_REG)
                    arrayPtr = sArray;
                  // If nothing is flagged, it is a general purpose register
                  else
                    arrayPtr = gArray;
                }

              qdsp6_check_register (arrayPtr, reg_num, pred_reg_rd_mask, operand, insn, opcode);
              if (operand->flags & QDSP6_OPERAND_IS_REGISTER_PAIR)  // For register pairs
                qdsp6_check_register (arrayPtr, reg_num+1, pred_reg_rd_mask, operand, insn, opcode);
            }
        } // end if for CAPITAL letters indicating it is a register
    } // end for loop for walking the syntax string
}

void
qdsp6_init_reg
(void)
{
  bzero (gArray,    sizeof (gArray));
  bzero (cArray,    sizeof (cArray));
  bzero (sArray,    sizeof (sArray));
  bzero (pArray,    sizeof (pArray));
  bzero (pNewArray, sizeof (pNewArray));
}

void
qdsp6_check_single_insn
(const qdsp6_opcode *opcode, qdsp6_insn insn)
{
  qdsp6_init_reg ();

  implicit_sr_ovf_bit_flag = 0;
  num_inst_in_packet_from_src = 0;
  numOfBranchAddr = 0;
  numOfBranchRelax = 0;
  numOfBranchAddrMax1 = 0;
  numOfBranchMax1 = 0;
  numOfLoopMax1 = 0;

  qdsp6_check_insn (opcode, insn);

  qdsp6_check_new_predicate ();
}

void
qdsp6_packet_open
(void)
{
  qdsp6_packet_begin ();
}

void
qdsp6_packet_close
(void)
{
  int inner, outer;

  qdsp6_packet_end ();

  qdsp6_packet_end_lookahead (&inner, &outer);
  if (inner)
    qdsp6_packet_end_inner ();
  if (outer)
    qdsp6_packet_end_outer ();
}

  /* Used to handle packet begin/end syntax */

void
qdsp6_packet_begin
(void)
{
  if (qdsp6_in_packet)
    {
      qdsp6_packet_close ();
      qdsp6_packet_write ();

      qdsp6_packet_open ();

      as_warn (_("found `%s' inside a packet."), PACKET_BEGIN);
      return;
    }

  qdsp6_in_packet = 1;
  qdsp6_packets [0].size = 0;
}

/* Function to write packet header, encode instructions in a packet,
   and perform various assembler restriction checks */

void
qdsp6_packet_end
(void)
{
  size_t i;

  if (!qdsp6_in_packet)
    {
      qdsp6_packets [0].size = 0;

      as_warn (_("found `%s' before opening a packet."), PACKET_END);
      return;
    }

  qdsp6_init_reg ();

  implicit_sr_ovf_bit_flag = 0;
  num_inst_in_packet_from_src = qdsp6_packets [0].size;
  numOfBranchAddr = 0;
  numOfBranchRelax = 0;
  numOfBranchAddrMax1 = 0;
  numOfBranchMax1 = 0;
  numOfLoopMax1 = 0;

  // check for number of instructions in the packet
  if (!qdsp6_packet_shuffle && qdsp6_packets [0].size != MAX_PACKET_INSNS) {
    as_bad (_("too many instructions in packet (maximum is %d).\n"),
            MAX_PACKET_INSNS);
  }

  // pad packet with NOPs to MAX_PACKET_INSNS
  for (i = qdsp6_packets [0].size; i < MAX_PACKET_INSNS; i++) {
    qdsp6_md_assemble (QDSP6_NOP, 1); //1 means padded by as
  }

  // Checking for multiple writes to the same register in a packet
  for(i = 0; i < qdsp6_packets [0].size; i++) {
	const qdsp6_opcode *opcode = qdsp6_packets [0].insns [i].opcode; // Get the opcode
    qdsp6_insn insn = qdsp6_packets [0].insns [i].insn; // get the encoded instruction

    qdsp6_check_insn(opcode, insn);
  } // end for loop for instructions in packet

  // check for multiple writes to SR (implicit not allowed if explicit writes are present)
  if (implicit_sr_ovf_bit_flag && cArray [QDSP6_SR])
    {
      as_bad (_("implicit definition of OVF bit in SR register not allowed in packet with explicit SR register definition."));
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
      as_bad (_("branch immediate and loop setup instructions cannot exist in same packet."));
      qdsp6_in_packet = FALSE;
      return;
    }

  if (   (numOfBranchMax1 > 1 && numOfBranchMax1 > numOfBranchRelax)
      || (numOfBranchAddr > 1 && numOfBranchAddr > numOfBranchRelax))
    {
      as_bad (_("illegal number of branches in packet."));
      qdsp6_in_packet = FALSE;
      return;
    }

  /* See if we need to reorder the instructions */
  if (qdsp6_packet_reverse) {
    for (i = 0; i < MAX_PACKET_INSNS / 2; i++) {
      qdsp6_packet_insn tmp = qdsp6_packets [0].insns [i];
      qdsp6_packets [0].insns [i] =
        qdsp6_packets [0].insns [MAX_PACKET_INSNS - i - 1];
      qdsp6_packets [0].insns [MAX_PACKET_INSNS - i - 1] = tmp;
      }
  }
  else if (qdsp6_packet_shuffle) {
    qdsp6_do_shuffle ();
}

  qdsp6_in_packet = FALSE;
}

int
qdsp6_packet_check_solo
(void)
{
  int solo;
  size_t i;

  /* Solo insns can not exist in a packet. */
  for (i = 0, solo = FALSE; i < qdsp6_packets [0].size && !solo; i++)
    if (qdsp6_packets [0].insns [i].opcode->attributes & A_RESTRICT_NOPACKET)
      solo = TRUE;

  return (solo);
}

/** Validate end of inner loop packet.
*/
void
qdsp6_packet_end_inner
(void)
{
  /* Check whether registers updated by :endloop0 are updated in packet. */
  if (  cArray [QDSP6_P30] | cArray [QDSP6_SR]
      | cArray [QDSP6_SA0] | cArray [QDSP6_LC0] | cArray [QDSP6_PC])
    as_bad (_("packet marked with `:endloop0' cannot contain instructions that " \
              "modify registers `c4/p3:0', `c8/usr', `sa0', `lc0' or `pc'."));

  /* Although it may be dangerous to modify P3 then, legacy code is full of this. */
/*
  if (pArray [3])
    as_warn (_("packet marked with `:endloop0' that contain instructions that " \
               "modify register `p3' may have undefined results if "
               "ending a pipelined loop."));
*/

  /* Check for a solo instruction in a packet with :endloop0. */
  if (qdsp6_packet_check_solo ())
    as_bad (_("packet marked with `:endloop0' cannot contain a solo instruction."));

  qdsp6_packets [0].is_inner = TRUE;
}

/** Validate end of outer loop packet.
*/
void
qdsp6_packet_end_outer
(void)
{
  /* Check whether registers updated by :endloop1 are updated in packet. */
  if (cArray [QDSP6_SA1] | cArray [QDSP6_LC1] | cArray [QDSP6_PC])
    as_bad (_("packet marked with `:endloop1' cannot contain instructions that " \
              "modify registers `sa1', `lc1' or `pc'."));

  /* Check for a solo instruction in a packet with :endloop1. */
  if (qdsp6_packet_check_solo ())
    as_bad (_("packet marked with `:endloop1' cannot contain a solo instruction."));

  qdsp6_packets [0].is_outer = TRUE;
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
  extern char *get_buffer_limit(); /* exposes read.c static buffer limit*/
  extern void put_buffer_limit(char *);

  char *buffer_limit = get_buffer_limit(); /* read.c */
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
      else if (   !inner
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
      else if (   !outer
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

  if (!strncmp (str, PACKET_BEGIN, strlen (PACKET_BEGIN)))
    {
      qdsp6_packet_open ();

      input_line_pointer += strlen (PACKET_BEGIN) - 1;

      return 1;
    }
  else if (!strncmp (str, PACKET_END, strlen (PACKET_END)))
    {
      qdsp6_packet_close ();
      qdsp6_packet_write ();

      input_line_pointer += strlen (PACKET_END) - 1;

      return 1;
    }
  else if (!strncmp (str, PACKET_END_INNER, strlen (PACKET_END_INNER)))
    {
      as_warn (_("found `%s' when not closing a packet."), PACKET_END_INNER);

      input_line_pointer += strlen (PACKET_END_INNER) - 1;

      return 1;
    }
  else if (!strncmp (str, PACKET_END_OUTER, strlen (PACKET_END_OUTER)))
    {
      as_warn (_("found `%s' when not closing a packet."), PACKET_END_OUTER);

      input_line_pointer += strlen (PACKET_END_OUTER) - 1;

      return 1;
    }

  /* Not a valid line */
  return 0;
}

void
qdsp6_cleanup
(void)
{
  size_t n_faligned;

  if (qdsp6_in_packet)
    {
      as_warn (_("reached end of file before closing a packet."));

      qdsp6_packet_close ();
      qdsp6_packet_write ();
    }

  /* Wipe out packet history. */
  qdsp6_history_flush (0, QDSP6_FLUSH_INIT);

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
      as_warn (_("reasons for `.falign' inserting new `nop' packets:"));
      as_warn (_("  %lu of %lu (%lu%%) reached another `.falign'."),
               n_falign [QDSP6_FALIGN_FALIGN], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_FALIGN] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) reached a single-instruction `.falign'."),
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
      as_warn (_("  %lu of %lu (%lu%%) reached a `.align'."),
               n_falign [QDSP6_FALIGN_ALIGN], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_ALIGN] * 100 / n_falign [QDSP6_FALIGN_INS]);
      as_warn (_("  %lu of %lu (%lu%%) reached another `nop' packet."),
               n_falign [QDSP6_FALIGN_NOP], n_falign [QDSP6_FALIGN_INS],
               n_falign [QDSP6_FALIGN_NOP] * 100 / n_falign [QDSP6_FALIGN_INS]);
    }

  bzero (n_falign, sizeof (n_falign));
}
