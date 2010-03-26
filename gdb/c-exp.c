/* A Bison parser, made by GNU Bison 1.875.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     FLOAT = 259,
     DECFLOAT = 260,
     STRING = 261,
     CHAR = 262,
     NAME = 263,
     COMPLETE = 264,
     TYPENAME = 265,
     NAME_OR_INT = 266,
     STRUCT = 267,
     CLASS = 268,
     UNION = 269,
     ENUM = 270,
     SIZEOF = 271,
     UNSIGNED = 272,
     COLONCOLON = 273,
     TEMPLATE = 274,
     ERROR = 275,
     SIGNED_KEYWORD = 276,
     LONG = 277,
     SHORT = 278,
     INT_KEYWORD = 279,
     CONST_KEYWORD = 280,
     VOLATILE_KEYWORD = 281,
     DOUBLE_KEYWORD = 282,
     VARIABLE = 283,
     ASSIGN_MODIFY = 284,
     TRUEKEYWORD = 285,
     FALSEKEYWORD = 286,
     ABOVE_COMMA = 287,
     OROR = 288,
     ANDAND = 289,
     NOTEQUAL = 290,
     EQUAL = 291,
     GEQ = 292,
     LEQ = 293,
     RSH = 294,
     LSH = 295,
     DECREMENT = 296,
     INCREMENT = 297,
     UNARY = 298,
     ARRAY_OPEN = 299,
     SEG = 300,
     DOT_STAR = 301,
     ARROW_STAR = 302,
     ARROW = 303,
     ARRAY_CLOSE = 304,
     BLOCKNAME = 305,
     FILENAME = 306
   };
#endif
#define INT 258
#define FLOAT 259
#define DECFLOAT 260
#define STRING 261
#define CHAR 262
#define NAME 263
#define COMPLETE 264
#define TYPENAME 265
#define NAME_OR_INT 266
#define STRUCT 267
#define CLASS 268
#define UNION 269
#define ENUM 270
#define SIZEOF 271
#define UNSIGNED 272
#define COLONCOLON 273
#define TEMPLATE 274
#define ERROR 275
#define SIGNED_KEYWORD 276
#define LONG 277
#define SHORT 278
#define INT_KEYWORD 279
#define CONST_KEYWORD 280
#define VOLATILE_KEYWORD 281
#define DOUBLE_KEYWORD 282
#define VARIABLE 283
#define ASSIGN_MODIFY 284
#define TRUEKEYWORD 285
#define FALSEKEYWORD 286
#define ABOVE_COMMA 287
#define OROR 288
#define ANDAND 289
#define NOTEQUAL 290
#define EQUAL 291
#define GEQ 292
#define LEQ 293
#define RSH 294
#define LSH 295
#define DECREMENT 296
#define INCREMENT 297
#define UNARY 298
#define ARRAY_OPEN 299
#define SEG 300
#define DOT_STAR 301
#define ARROW_STAR 302
#define ARROW 303
#define ARRAY_CLOSE 304
#define BLOCKNAME 305
#define FILENAME 306




/* Copy the first part of user declarations.  */
#line 38 "c-exp.y"


#include "defs.h"
#include "gdb_string.h"
#include <ctype.h>
#include "expression.h"
#include "value.h"
#include "parser-defs.h"
#include "language.h"
#include "c-lang.h"
#include "bfd.h" /* Required by objfiles.h.  */
#include "symfile.h" /* Required by objfiles.h.  */
#include "objfiles.h" /* For have_full_symbols and have_partial_symbols */
#include "charset.h"
#include "block.h"
#include "cp-support.h"
#include "dfp.h"
#include "gdb_assert.h"
#include "macroscope.h"

#define parse_type builtin_type (parse_gdbarch)

/* Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
   as well as gratuitiously global symbol names, so we can have multiple
   yacc generated parsers in gdb.  Note that these are only the variables
   produced by yacc.  If other parser generators (bison, byacc, etc) produce
   additional global names that conflict at link time, then those parser
   generators need to be fixed instead of adding those names to this list. */

#define	yymaxdepth c_maxdepth
#define	yyparse	c_parse_internal
#define	yylex	c_lex
#define	yyerror	c_error
#define	yylval	c_lval
#define	yychar	c_char
#define	yydebug	c_debug
#define	yypact	c_pact	
#define	yyr1	c_r1			
#define	yyr2	c_r2			
#define	yydef	c_def		
#define	yychk	c_chk		
#define	yypgo	c_pgo		
#define	yyact	c_act		
#define	yyexca	c_exca
#define yyerrflag c_errflag
#define yynerrs	c_nerrs
#define	yyps	c_ps
#define	yypv	c_pv
#define	yys	c_s
#define	yy_yys	c_yys
#define	yystate	c_state
#define	yytmp	c_tmp
#define	yyv	c_v
#define	yy_yyv	c_yyv
#define	yyval	c_val
#define	yylloc	c_lloc
#define yyreds	c_reds		/* With YYDEBUG defined */
#define yytoks	c_toks		/* With YYDEBUG defined */
#define yyname	c_name		/* With YYDEBUG defined */
#define yyrule	c_rule		/* With YYDEBUG defined */
#define yylhs	c_yylhs
#define yylen	c_yylen
#define yydefred c_yydefred
#define yydgoto	c_yydgoto
#define yysindex c_yysindex
#define yyrindex c_yyrindex
#define yygindex c_yygindex
#define yytable	 c_yytable
#define yycheck	 c_yycheck

#ifndef YYDEBUG
#define	YYDEBUG 1		/* Default to yydebug support */
#endif

#define YYFPRINTF parser_fprintf

int yyparse (void);

static int yylex (void);

void yyerror (char *);



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 127 "c-exp.y"
typedef union YYSTYPE {
    LONGEST lval;
    struct {
      LONGEST val;
      struct type *type;
    } typed_val_int;
    struct {
      DOUBLEST dval;
      struct type *type;
    } typed_val_float;
    struct {
      gdb_byte val[16];
      struct type *type;
    } typed_val_decfloat;
    struct symbol *sym;
    struct type *tval;
    struct stoken sval;
    struct typed_stoken tsval;
    struct ttype tsym;
    struct symtoken ssym;
    int voidval;
    struct block *bval;
    enum exp_opcode opcode;
    struct internalvar *ivar;

    struct stoken_vector svec;
    struct type **tvec;
    int *ivec;
  } YYSTYPE;
/* Line 191 of yacc.c.  */
#line 291 "../../gdb/c-exp.c.tmp"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 157 "c-exp.y"

/* YYSTYPE gets defined by %union */
static int parse_number (char *, int, int, YYSTYPE *);


/* Line 214 of yacc.c.  */
#line 307 "../../gdb/c-exp.c.tmp"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or xmalloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC xmalloc
#  define YYSTACK_FREE xfree
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  99
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   874

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  76
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  32
/* YYNRULES -- Number of rules. */
#define YYNRULES  169
/* YYNRULES -- Number of states. */
#define YYNSTATES  254

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   306

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    69,     2,     2,     2,    54,    40,     2,
      60,    72,    52,    50,    32,    51,    58,    53,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    75,     2,
      43,    34,    44,    35,    49,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    59,     2,    71,    39,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    73,    38,    74,    70,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    33,    36,    37,
      41,    42,    45,    46,    47,    48,    55,    56,    57,    61,
      62,    63,    64,    65,    66,    67,    68
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned short yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    11,    15,    18,    21,
      24,    27,    30,    33,    36,    39,    42,    45,    48,    52,
      57,    61,    65,    69,    73,    78,    82,    86,    90,    95,
     100,   104,   105,   111,   113,   114,   116,   120,   122,   126,
     131,   136,   140,   144,   148,   152,   156,   160,   164,   168,
     172,   176,   180,   184,   188,   192,   196,   200,   204,   208,
     212,   216,   222,   226,   230,   232,   234,   236,   238,   240,
     242,   244,   249,   251,   254,   256,   258,   260,   262,   264,
     268,   272,   276,   281,   283,   286,   288,   291,   293,   294,
     298,   300,   302,   304,   305,   307,   310,   312,   315,   317,
     321,   324,   326,   329,   331,   334,   338,   341,   345,   347,
     349,   351,   353,   355,   358,   362,   365,   369,   373,   377,
     380,   383,   387,   392,   396,   400,   405,   409,   414,   418,
     423,   426,   430,   433,   437,   440,   444,   446,   449,   452,
     455,   458,   461,   464,   466,   469,   471,   477,   480,   483,
     485,   489,   491,   493,   495,   497,   499,   503,   505,   510,
     513,   516,   518,   520,   522,   524,   526,   528,   530,   532
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      77,     0,    -1,    79,    -1,    78,    -1,    98,    -1,    80,
      -1,    79,    32,    80,    -1,    52,    80,    -1,    40,    80,
      -1,    51,    80,    -1,    50,    80,    -1,    69,    80,    -1,
      70,    80,    -1,    56,    80,    -1,    55,    80,    -1,    80,
      56,    -1,    80,    55,    -1,    16,    80,    -1,    80,    65,
     106,    -1,    80,    65,   106,     9,    -1,    80,    65,     9,
      -1,    80,    65,    88,    -1,    80,    64,    80,    -1,    80,
      58,   106,    -1,    80,    58,   106,     9,    -1,    80,    58,
       9,    -1,    80,    58,    88,    -1,    80,    63,    80,    -1,
      80,    59,    79,    71,    -1,    80,    61,    79,    66,    -1,
      80,    62,    79,    -1,    -1,    80,    60,    81,    83,    72,
      -1,    73,    -1,    -1,    80,    -1,    83,    32,    80,    -1,
      74,    -1,    82,    83,    84,    -1,    82,    98,    84,    80,
      -1,    60,    98,    72,    80,    -1,    60,    79,    72,    -1,
      80,    49,    80,    -1,    80,    52,    80,    -1,    80,    53,
      80,    -1,    80,    54,    80,    -1,    80,    50,    80,    -1,
      80,    51,    80,    -1,    80,    48,    80,    -1,    80,    47,
      80,    -1,    80,    42,    80,    -1,    80,    41,    80,    -1,
      80,    46,    80,    -1,    80,    45,    80,    -1,    80,    43,
      80,    -1,    80,    44,    80,    -1,    80,    40,    80,    -1,
      80,    39,    80,    -1,    80,    38,    80,    -1,    80,    37,
      80,    -1,    80,    36,    80,    -1,    80,    35,    80,    75,
      80,    -1,    80,    34,    80,    -1,    80,    29,    80,    -1,
       3,    -1,     7,    -1,    11,    -1,     4,    -1,     5,    -1,
      87,    -1,    28,    -1,    16,    60,    98,    72,    -1,     6,
      -1,    85,     6,    -1,    85,    -1,    30,    -1,    31,    -1,
      67,    -1,    68,    -1,    86,    18,   106,    -1,    86,    18,
     106,    -1,    99,    18,   106,    -1,    99,    18,    70,   106,
      -1,    88,    -1,    18,   106,    -1,   107,    -1,    49,     8,
      -1,   105,    -1,    -1,    90,    89,    90,    -1,    91,    -1,
     105,    -1,    92,    -1,    -1,    52,    -1,    52,    94,    -1,
      40,    -1,    40,    94,    -1,    95,    -1,    60,    94,    72,
      -1,    95,    96,    -1,    96,    -1,    95,    97,    -1,    97,
      -1,    59,    71,    -1,    59,     3,    71,    -1,    60,    72,
      -1,    60,   102,    72,    -1,   103,    -1,    10,    -1,    24,
      -1,    22,    -1,    23,    -1,    22,    24,    -1,    22,    21,
      24,    -1,    22,    21,    -1,    21,    22,    24,    -1,    17,
      22,    24,    -1,    22,    17,    24,    -1,    22,    17,    -1,
      22,    22,    -1,    22,    22,    24,    -1,    22,    22,    21,
      24,    -1,    22,    22,    21,    -1,    21,    22,    22,    -1,
      21,    22,    22,    24,    -1,    17,    22,    22,    -1,    17,
      22,    22,    24,    -1,    22,    22,    17,    -1,    22,    22,
      17,    24,    -1,    23,    24,    -1,    23,    21,    24,    -1,
      23,    21,    -1,    17,    23,    24,    -1,    23,    17,    -1,
      23,    17,    24,    -1,    27,    -1,    22,    27,    -1,    12,
     106,    -1,    13,   106,    -1,    14,   106,    -1,    15,   106,
      -1,    17,   101,    -1,    17,    -1,    21,   101,    -1,    21,
      -1,    19,   106,    43,    98,    44,    -1,    92,    99,    -1,
      99,    92,    -1,   100,    -1,    99,    18,   106,    -1,    10,
      -1,    24,    -1,    22,    -1,    23,    -1,    98,    -1,   102,
      32,    98,    -1,    99,    -1,   103,    93,    94,    93,    -1,
      25,    26,    -1,    26,    25,    -1,   104,    -1,    25,    -1,
      26,    -1,     8,    -1,    67,    -1,    10,    -1,    11,    -1,
       8,    -1,    67,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,   246,   246,   247,   250,   257,   258,   263,   267,   271,
     275,   279,   283,   287,   291,   295,   299,   303,   307,   313,
     320,   330,   338,   342,   348,   355,   365,   373,   377,   381,
     385,   394,   391,   401,   405,   408,   412,   416,   419,   426,
     432,   438,   444,   448,   452,   456,   460,   464,   468,   472,
     476,   480,   484,   488,   492,   496,   500,   504,   508,   512,
     516,   520,   524,   528,   534,   541,   550,   561,   568,   575,
     578,   582,   591,   608,   626,   660,   667,   676,   684,   690,
     700,   715,   729,   754,   755,   783,   836,   842,   843,   846,
     849,   850,   854,   855,   858,   860,   862,   864,   866,   869,
     871,   876,   883,   885,   889,   891,   895,   897,   909,   913,
     915,   917,   919,   921,   923,   925,   927,   929,   931,   933,
     935,   937,   939,   941,   943,   945,   947,   949,   951,   953,
     955,   957,   959,   961,   963,   965,   967,   969,   971,   974,
     977,   980,   983,   987,   989,   993,   998,  1002,  1004,  1006,
    1054,  1079,  1080,  1086,  1092,  1101,  1106,  1113,  1114,  1118,
    1119,  1122,  1126,  1128,  1132,  1133,  1134,  1135,  1138,  1139
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "INT", "FLOAT", "DECFLOAT", "STRING", 
  "CHAR", "NAME", "COMPLETE", "TYPENAME", "NAME_OR_INT", "STRUCT", 
  "CLASS", "UNION", "ENUM", "SIZEOF", "UNSIGNED", "COLONCOLON", 
  "TEMPLATE", "ERROR", "SIGNED_KEYWORD", "LONG", "SHORT", "INT_KEYWORD", 
  "CONST_KEYWORD", "VOLATILE_KEYWORD", "DOUBLE_KEYWORD", "VARIABLE", 
  "ASSIGN_MODIFY", "TRUEKEYWORD", "FALSEKEYWORD", "','", "ABOVE_COMMA", 
  "'='", "'?'", "OROR", "ANDAND", "'|'", "'^'", "'&'", "NOTEQUAL", 
  "EQUAL", "'<'", "'>'", "GEQ", "LEQ", "RSH", "LSH", "'@'", "'+'", "'-'", 
  "'*'", "'/'", "'%'", "DECREMENT", "INCREMENT", "UNARY", "'.'", "'['", 
  "'('", "ARRAY_OPEN", "SEG", "DOT_STAR", "ARROW_STAR", "ARROW", 
  "ARRAY_CLOSE", "BLOCKNAME", "FILENAME", "'!'", "'~'", "']'", "')'", 
  "'{'", "'}'", "':'", "$accept", "start", "type_exp", "exp1", "exp", 
  "@1", "lcurly", "arglist", "rcurly", "string_exp", "block", "variable", 
  "qualified_name", "space_identifier", "const_or_volatile", 
  "cv_with_space_id", "const_or_volatile_or_space_identifier_noopt", 
  "const_or_volatile_or_space_identifier", "abs_decl", "direct_abs_decl", 
  "array_mod", "func_mod", "type", "typebase", "qualified_type", 
  "typename", "nonempty_typelist", "ptype", "const_and_volatile", 
  "const_or_volatile_noopt", "name", "name_not_typename", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,    44,   287,    61,    63,   288,   289,   124,    94,
      38,   290,   291,    60,    62,   292,   293,   294,   295,    64,
      43,    45,    42,    47,    37,   296,   297,   298,    46,    91,
      40,   299,   300,   301,   302,   303,   304,   305,   306,    33,
     126,    93,    41,   123,   125,    58
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    76,    77,    77,    78,    79,    79,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    81,    80,    82,    83,    83,    83,    84,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    85,    85,    80,    80,    80,    86,    86,    86,
      87,    88,    88,    87,    87,    87,    89,    90,    90,    91,
      92,    92,    93,    93,    94,    94,    94,    94,    94,    95,
      95,    95,    95,    95,    96,    96,    97,    97,    98,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,    99,
     100,   101,   101,   101,   101,   102,   102,   103,   103,   104,
     104,   105,   105,   105,   106,   106,   106,   106,   107,   107
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     1,     1,     1,     1,     3,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     3,     4,
       3,     3,     3,     3,     4,     3,     3,     3,     4,     4,
       3,     0,     5,     1,     0,     1,     3,     1,     3,     4,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     5,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     2,     1,     1,     1,     1,     1,     3,
       3,     3,     4,     1,     2,     1,     2,     1,     0,     3,
       1,     1,     1,     0,     1,     2,     1,     2,     1,     3,
       2,     1,     2,     1,     2,     3,     2,     3,     1,     1,
       1,     1,     1,     2,     3,     2,     3,     3,     3,     2,
       2,     3,     4,     3,     3,     4,     3,     4,     3,     4,
       2,     3,     2,     3,     2,     3,     1,     2,     2,     2,
       2,     2,     2,     1,     2,     1,     5,     2,     2,     1,
       3,     1,     1,     1,     1,     1,     3,     1,     4,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
      88,    64,    67,    68,    72,    65,   168,   109,    66,     0,
       0,     0,     0,    88,   143,     0,     0,   145,   111,   112,
     110,   162,   163,   136,    70,    75,    76,    88,    88,    88,
      88,    88,    88,    88,   169,    78,    88,    88,    33,     0,
       3,     2,     5,    34,    74,     0,    69,    83,     0,    90,
      88,     4,   157,   149,   108,   161,    91,    85,   164,   166,
     167,   165,   138,   139,   140,   141,    88,    17,    88,   151,
     153,   154,   152,   142,    84,     0,   153,   154,   144,   119,
     115,   120,   113,   137,   134,   132,   130,   159,   160,     8,
      10,     9,     7,    14,    13,     0,     0,    11,    12,     1,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    88,    88,    88,    88,    88,    88,    88,
      88,    88,    88,    16,    15,    88,    88,    31,    88,    88,
      88,    88,    88,    35,     0,     0,    73,     0,     0,    88,
     147,     0,   148,    92,     0,     0,   126,   117,   133,    88,
     124,   116,   118,   114,   128,   123,   121,   135,   131,    41,
      88,     6,    63,    62,     0,    60,    59,    58,    57,    56,
      51,    50,    54,    55,    53,    52,    49,    48,    42,    46,
      47,    43,    44,    45,    25,   166,    26,    23,     0,    34,
       0,    30,    27,    22,    20,    21,    18,    88,    37,    38,
      88,    80,    86,    89,    87,     0,     0,    81,    96,    94,
       0,    88,    93,    98,   101,   103,    71,   127,     0,   157,
     125,   129,   122,    40,    88,    24,    28,     0,    29,    19,
      36,    39,   150,    82,    97,    95,     0,   104,   106,     0,
     155,     0,   158,    88,   100,   102,   146,    61,    32,   105,
      99,    88,   107,   156
};

/* YYDEFGOTO[NTERM-NUM]. */
static const short yydefgoto[] =
{
      -1,    39,    40,    95,    42,   189,    43,   134,   199,    44,
      45,    46,    47,   139,    48,    49,    50,   144,   212,   213,
     214,   215,   240,    68,    53,    73,   241,    54,    55,    56,
      62,    57
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -78
static const short yypact[] =
{
     285,   -78,   -78,   -78,   -78,   -78,   -78,   -78,   -78,    48,
      48,    48,    48,   356,   116,    48,    48,   120,    57,    25,
     -78,    27,     7,   -78,   -78,   -78,   -78,   285,   285,   285,
     285,   285,   285,   285,     8,   -78,   285,   285,   -78,    73,
     -78,    44,   599,   214,    85,    98,   -78,   -78,    71,   -78,
     307,   -78,    13,   -78,    11,   -78,    87,   -78,   -78,   -78,
     -78,   -78,   -78,   -78,   -78,   -78,   285,   135,   128,   -78,
      23,   117,   -78,   -78,   -78,   102,    61,   -78,   -78,   127,
     131,    65,   -78,   -78,   136,   143,   -78,   -78,   -78,   135,
     135,   135,   135,   135,   135,   -18,    75,   135,   135,   -78,
     285,   285,   285,   285,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   285,   285,   285,   285,   285,   285,   285,
     285,   285,   285,   -78,   -78,   503,   285,   -78,   285,   285,
     285,   285,   523,   599,   -22,    97,   -78,    48,   172,    93,
      99,     5,   -78,   -78,    28,   132,   164,   -78,   -78,   307,
     179,   -78,   -78,   -78,   182,   183,   -78,   -78,   -78,   -78,
     285,   599,   599,   599,   557,   659,   513,   686,   712,   737,
     760,   760,   779,   779,   779,   779,   339,   339,   795,   809,
     809,   135,   135,   135,   -78,   109,   -78,   200,    -2,   214,
      -5,   -78,   197,   197,   -78,   -78,   201,   285,   -78,   -78,
     285,   196,   -78,   -78,   -78,    48,    48,   161,   153,   123,
       6,   449,    15,   125,   -78,   -78,   427,   -78,   171,    99,
     -78,   -78,   -78,   135,   285,   -78,   -78,   -15,   -78,   -78,
     599,   135,   -78,   -78,   -78,   -78,   163,   -78,   -78,   174,
     -78,    -7,   -78,   151,   -78,   -78,   -78,   630,   -78,   -78,
     -78,   307,   -78,   -78
};

/* YYPGOTO[NTERM-NUM].  */
static const short yypgoto[] =
{
     -78,   -78,   -78,     3,    -8,   -78,   -78,    58,   113,   -78,
     -78,   -78,   -77,   -78,   110,   -78,   -50,    38,   -52,   -78,
      40,    54,     1,     0,   -78,   251,   -78,   -78,   -78,   133,
      -4,   -78
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -151
static const short yytable[] =
{
      52,    51,   142,    41,   143,    67,    63,    64,    65,   236,
     197,    74,    75,    58,   100,    59,    60,   197,   142,    89,
      90,    91,    92,    93,    94,   251,   -77,   100,    97,    98,
     100,   141,    88,    52,    96,   133,    21,    22,    21,    22,
      21,    22,    84,    52,   135,   146,    85,   147,   186,    86,
     140,   -93,   198,    87,   159,   195,    58,   248,    59,    60,
     -88,   228,   -88,   -93,   -88,   252,    52,   145,   208,   226,
     -93,   -93,    61,    99,    79,   206,   100,   237,    80,    81,
     209,    82,   154,   150,    83,   151,   155,   210,   211,   156,
     142,   136,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,    61,   137,   205,    21,    22,
     138,   187,   192,   193,    21,    22,    69,  -109,   196,   188,
      69,   190,   191,   201,  -109,  -109,   -87,   207,    70,    71,
      72,   148,    76,    77,    72,   149,   141,   160,   -88,   219,
     218,   152,   223,    21,    22,   153,   234,   235,  -109,   239,
     157,     7,   143,     9,    10,    11,    12,   158,    14,   142,
      16,   198,    17,    18,    19,    20,    21,    22,    23,  -150,
     202,   133,   210,   211,   210,   243,  -150,  -150,   217,   230,
     123,   124,   231,   125,   126,   127,   128,   129,   130,   131,
     132,   232,   233,   220,   216,   209,   221,   222,   223,   225,
     229,   219,   210,   211,   -79,   246,   247,     1,     2,     3,
       4,     5,     6,   238,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,   249,    17,    18,    19,    20,    21,
      22,    23,    24,   219,    25,    26,   250,   227,   200,   203,
     242,   219,   253,   244,    27,   125,   126,   127,   128,   129,
     130,   131,   132,   -88,    28,    29,    30,   245,    78,    31,
      32,     0,   204,     0,    33,     0,     0,     0,     0,     0,
       0,    34,    35,    36,    37,     0,     0,    38,     1,     2,
       3,     4,     5,     6,     0,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,     0,    25,    26,     7,     0,     9,
      10,    11,    12,     0,    14,    27,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    28,    29,    30,     0,     0,
      31,    32,     0,     0,     0,    33,     0,     0,     0,     0,
       0,     0,    34,    35,    36,    37,     0,     0,    38,     1,
       2,     3,     4,     5,     6,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,     0,    25,    26,   117,   118,
     119,   120,   121,   122,   123,   124,    27,   125,   126,   127,
     128,   129,   130,   131,   132,     0,    28,    29,    30,     0,
       0,    31,    32,     0,     0,     0,    66,     0,     0,     0,
       0,     0,     0,    34,    35,    36,    37,     0,     0,    38,
       1,     2,     3,     4,     5,     6,     0,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,     0,    25,    26,     7,
       0,     9,    10,    11,    12,     0,    14,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,     0,     0,     0,
       0,     0,    31,    32,     0,     0,     0,    33,     0,   208,
       0,     0,     0,     0,    34,    35,    36,    37,     0,     0,
      38,   209,     0,     0,     0,     0,     0,     0,   210,   211,
       0,    58,   184,   185,    60,     9,    10,    11,    12,     0,
      14,   238,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    58,   194,   185,    60,     9,    10,    11,    12,     0,
      14,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
      61,   125,   126,   127,   128,   129,   130,   131,   132,     0,
       0,     0,     0,     0,     0,     0,   101,     0,     0,     0,
      61,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,     0,     0,     0,     0,     0,   101,     0,
       0,     0,   224,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   123,   124,     0,   125,   126,   127,   128,   129,   130,
     131,   132,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,     0,
     125,   126,   127,   128,   129,   130,   131,   132,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,     0,   125,   126,   127,   128,   129,
     130,   131,   132,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   123,   124,     0,   125,   126,
     127,   128,   129,   130,   131,   132,   115,   116,   117,   118,
     119,   120,   121,   122,   123,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132,   118,   119,   120,   121,   122,
     123,   124,     0,   125,   126,   127,   128,   129,   130,   131,
     132,   120,   121,   122,   123,   124,     0,   125,   126,   127,
     128,   129,   130,   131,   132
};

static const short yycheck[] =
{
       0,     0,    52,     0,    54,    13,    10,    11,    12,     3,
      32,    15,    16,     8,    32,    10,    11,    32,    68,    27,
      28,    29,    30,    31,    32,    32,    18,    32,    36,    37,
      32,    18,    25,    33,    33,    43,    25,    26,    25,    26,
      25,    26,    17,    43,    43,    22,    21,    24,   125,    24,
      50,    40,    74,    26,    72,   132,     8,    72,    10,    11,
      49,    66,    49,    52,    49,    72,    66,    66,    40,    71,
      59,    60,    67,     0,    17,    70,    32,    71,    21,    22,
      52,    24,    17,    22,    27,    24,    21,    59,    60,    24,
     140,     6,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,    67,    18,    18,    25,    26,
      49,   125,   130,   131,    25,    26,    10,    18,   132,   126,
      10,   128,   129,   137,    25,    26,    49,   141,    22,    23,
      24,    24,    22,    23,    24,    43,    18,    72,    49,   149,
     149,    24,   160,    25,    26,    24,   208,   209,    49,   211,
      24,    10,   212,    12,    13,    14,    15,    24,    17,   219,
      19,    74,    21,    22,    23,    24,    25,    26,    27,    18,
       8,   189,    59,    60,    59,    60,    25,    26,    24,   197,
      55,    56,   200,    58,    59,    60,    61,    62,    63,    64,
      65,   205,   206,    24,    72,    52,    24,    24,   216,     9,
       9,   211,    59,    60,    18,    44,   224,     3,     4,     5,
       6,     7,     8,    72,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    71,    21,    22,    23,    24,    25,
      26,    27,    28,   243,    30,    31,    72,   189,   135,   139,
     212,   251,   251,   213,    40,    58,    59,    60,    61,    62,
      63,    64,    65,    49,    50,    51,    52,   213,    17,    55,
      56,    -1,   139,    -1,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    67,    68,    69,    70,    -1,    -1,    73,     3,     4,
       5,     6,     7,     8,    -1,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    -1,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    10,    -1,    12,
      13,    14,    15,    -1,    17,    40,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    50,    51,    52,    -1,    -1,
      55,    56,    -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,
      -1,    -1,    67,    68,    69,    70,    -1,    -1,    73,     3,
       4,     5,     6,     7,     8,    -1,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    -1,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    49,    50,
      51,    52,    53,    54,    55,    56,    40,    58,    59,    60,
      61,    62,    63,    64,    65,    -1,    50,    51,    52,    -1,
      -1,    55,    56,    -1,    -1,    -1,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    -1,    -1,    73,
       3,     4,     5,     6,     7,     8,    -1,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    -1,    21,    22,
      23,    24,    25,    26,    27,    28,    -1,    30,    31,    10,
      -1,    12,    13,    14,    15,    -1,    17,    -1,    19,    -1,
      21,    22,    23,    24,    25,    26,    27,    -1,    -1,    -1,
      -1,    -1,    55,    56,    -1,    -1,    -1,    60,    -1,    40,
      -1,    -1,    -1,    -1,    67,    68,    69,    70,    -1,    -1,
      73,    52,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,
      -1,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
      17,    72,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
      17,    -1,    19,    -1,    21,    22,    23,    24,    25,    26,
      27,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      67,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    29,    -1,    -1,    -1,
      67,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    -1,    -1,    -1,    -1,    -1,    29,    -1,
      -1,    -1,    75,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    -1,    58,    59,    60,    61,    62,
      63,    64,    65,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    50,    51,    52,    53,    54,
      55,    56,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    52,    53,    54,    55,    56,    -1,    58,    59,    60,
      61,    62,    63,    64,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    21,    22,    23,
      24,    25,    26,    27,    28,    30,    31,    40,    50,    51,
      52,    55,    56,    60,    67,    68,    69,    70,    73,    77,
      78,    79,    80,    82,    85,    86,    87,    88,    90,    91,
      92,    98,    99,   100,   103,   104,   105,   107,     8,    10,
      11,    67,   106,   106,   106,   106,    60,    80,    99,    10,
      22,    23,    24,   101,   106,   106,    22,    23,   101,    17,
      21,    22,    24,    27,    17,    21,    24,    26,    25,    80,
      80,    80,    80,    80,    80,    79,    98,    80,    80,     0,
      32,    29,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    58,    59,    60,    61,    62,
      63,    64,    65,    80,    83,    98,     6,    18,    49,    89,
      99,    18,    92,    92,    93,    98,    22,    24,    24,    43,
      22,    24,    24,    24,    17,    21,    24,    24,    24,    72,
      72,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
      80,    80,    80,    80,     9,    10,    88,   106,    79,    81,
      79,    79,    80,    80,     9,    88,   106,    32,    74,    84,
      84,   106,     8,    90,   105,    18,    70,   106,    40,    52,
      59,    60,    94,    95,    96,    97,    72,    24,    98,    99,
      24,    24,    24,    80,    75,     9,    71,    83,    66,     9,
      80,    80,   106,   106,    94,    94,     3,    71,    72,    94,
      98,   102,    93,    60,    96,    97,    44,    80,    72,    71,
      72,    32,    72,    98
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrlab1

/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)         \
  Current.first_line   = Rhs[1].first_line;      \
  Current.first_column = Rhs[1].first_column;    \
  Current.last_line    = Rhs[N].last_line;       \
  Current.last_column  = Rhs[N].last_column;
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (cinluded).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylineno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylineno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to xreallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to xreallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 251 "c-exp.y"
    { write_exp_elt_opcode(OP_TYPE);
			  write_exp_elt_type(yyvsp[0].tval);
			  write_exp_elt_opcode(OP_TYPE);}
    break;

  case 6:
#line 259 "c-exp.y"
    { write_exp_elt_opcode (BINOP_COMMA); }
    break;

  case 7:
#line 264 "c-exp.y"
    { write_exp_elt_opcode (UNOP_IND); }
    break;

  case 8:
#line 268 "c-exp.y"
    { write_exp_elt_opcode (UNOP_ADDR); }
    break;

  case 9:
#line 272 "c-exp.y"
    { write_exp_elt_opcode (UNOP_NEG); }
    break;

  case 10:
#line 276 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PLUS); }
    break;

  case 11:
#line 280 "c-exp.y"
    { write_exp_elt_opcode (UNOP_LOGICAL_NOT); }
    break;

  case 12:
#line 284 "c-exp.y"
    { write_exp_elt_opcode (UNOP_COMPLEMENT); }
    break;

  case 13:
#line 288 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PREINCREMENT); }
    break;

  case 14:
#line 292 "c-exp.y"
    { write_exp_elt_opcode (UNOP_PREDECREMENT); }
    break;

  case 15:
#line 296 "c-exp.y"
    { write_exp_elt_opcode (UNOP_POSTINCREMENT); }
    break;

  case 16:
#line 300 "c-exp.y"
    { write_exp_elt_opcode (UNOP_POSTDECREMENT); }
    break;

  case 17:
#line 304 "c-exp.y"
    { write_exp_elt_opcode (UNOP_SIZEOF); }
    break;

  case 18:
#line 308 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_PTR);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_PTR); }
    break;

  case 19:
#line 314 "c-exp.y"
    { mark_struct_expression ();
			  write_exp_elt_opcode (STRUCTOP_PTR);
			  write_exp_string (yyvsp[-1].sval);
			  write_exp_elt_opcode (STRUCTOP_PTR); }
    break;

  case 20:
#line 321 "c-exp.y"
    { struct stoken s;
			  mark_struct_expression ();
			  write_exp_elt_opcode (STRUCTOP_PTR);
			  s.ptr = "";
			  s.length = 0;
			  write_exp_string (s);
			  write_exp_elt_opcode (STRUCTOP_PTR); }
    break;

  case 21:
#line 331 "c-exp.y"
    { /* exp->type::name becomes exp->*(&type::name) */
			  /* Note: this doesn't work if name is a
			     static member!  FIXME */
			  write_exp_elt_opcode (UNOP_ADDR);
			  write_exp_elt_opcode (STRUCTOP_MPTR); }
    break;

  case 22:
#line 339 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_MPTR); }
    break;

  case 23:
#line 343 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_STRUCT);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); }
    break;

  case 24:
#line 349 "c-exp.y"
    { mark_struct_expression ();
			  write_exp_elt_opcode (STRUCTOP_STRUCT);
			  write_exp_string (yyvsp[-1].sval);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); }
    break;

  case 25:
#line 356 "c-exp.y"
    { struct stoken s;
			  mark_struct_expression ();
			  write_exp_elt_opcode (STRUCTOP_STRUCT);
			  s.ptr = "";
			  s.length = 0;
			  write_exp_string (s);
			  write_exp_elt_opcode (STRUCTOP_STRUCT); }
    break;

  case 26:
#line 366 "c-exp.y"
    { /* exp.type::name becomes exp.*(&type::name) */
			  /* Note: this doesn't work if name is a
			     static member!  FIXME */
			  write_exp_elt_opcode (UNOP_ADDR);
			  write_exp_elt_opcode (STRUCTOP_MEMBER); }
    break;

  case 27:
#line 374 "c-exp.y"
    { write_exp_elt_opcode (STRUCTOP_MEMBER); }
    break;

  case 28:
#line 378 "c-exp.y"
    { write_exp_elt_opcode (BINOP_SUBSCRIPT); }
    break;

  case 29:
#line 382 "c-exp.y"
    { write_exp_elt_opcode (BINOP_SUBSCRIPT); }
    break;

  case 30:
#line 386 "c-exp.y"
    { write_exp_elt_opcode (BINOP_SUBSCRIPT); }
    break;

  case 31:
#line 394 "c-exp.y"
    { start_arglist (); }
    break;

  case 32:
#line 396 "c-exp.y"
    { write_exp_elt_opcode (OP_FUNCALL);
			  write_exp_elt_longcst ((LONGEST) end_arglist ());
			  write_exp_elt_opcode (OP_FUNCALL); }
    break;

  case 33:
#line 402 "c-exp.y"
    { start_arglist (); }
    break;

  case 35:
#line 409 "c-exp.y"
    { arglist_len = 1; }
    break;

  case 36:
#line 413 "c-exp.y"
    { arglist_len++; }
    break;

  case 37:
#line 417 "c-exp.y"
    { yyval.lval = end_arglist () - 1; }
    break;

  case 38:
#line 420 "c-exp.y"
    { write_exp_elt_opcode (OP_ARRAY);
			  write_exp_elt_longcst ((LONGEST) 0);
			  write_exp_elt_longcst ((LONGEST) yyvsp[0].lval);
			  write_exp_elt_opcode (OP_ARRAY); }
    break;

  case 39:
#line 427 "c-exp.y"
    { write_exp_elt_opcode (UNOP_MEMVAL);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_MEMVAL); }
    break;

  case 40:
#line 433 "c-exp.y"
    { write_exp_elt_opcode (UNOP_CAST);
			  write_exp_elt_type (yyvsp[-2].tval);
			  write_exp_elt_opcode (UNOP_CAST); }
    break;

  case 41:
#line 439 "c-exp.y"
    { }
    break;

  case 42:
#line 445 "c-exp.y"
    { write_exp_elt_opcode (BINOP_REPEAT); }
    break;

  case 43:
#line 449 "c-exp.y"
    { write_exp_elt_opcode (BINOP_MUL); }
    break;

  case 44:
#line 453 "c-exp.y"
    { write_exp_elt_opcode (BINOP_DIV); }
    break;

  case 45:
#line 457 "c-exp.y"
    { write_exp_elt_opcode (BINOP_REM); }
    break;

  case 46:
#line 461 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ADD); }
    break;

  case 47:
#line 465 "c-exp.y"
    { write_exp_elt_opcode (BINOP_SUB); }
    break;

  case 48:
#line 469 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LSH); }
    break;

  case 49:
#line 473 "c-exp.y"
    { write_exp_elt_opcode (BINOP_RSH); }
    break;

  case 50:
#line 477 "c-exp.y"
    { write_exp_elt_opcode (BINOP_EQUAL); }
    break;

  case 51:
#line 481 "c-exp.y"
    { write_exp_elt_opcode (BINOP_NOTEQUAL); }
    break;

  case 52:
#line 485 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LEQ); }
    break;

  case 53:
#line 489 "c-exp.y"
    { write_exp_elt_opcode (BINOP_GEQ); }
    break;

  case 54:
#line 493 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LESS); }
    break;

  case 55:
#line 497 "c-exp.y"
    { write_exp_elt_opcode (BINOP_GTR); }
    break;

  case 56:
#line 501 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_AND); }
    break;

  case 57:
#line 505 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_XOR); }
    break;

  case 58:
#line 509 "c-exp.y"
    { write_exp_elt_opcode (BINOP_BITWISE_IOR); }
    break;

  case 59:
#line 513 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LOGICAL_AND); }
    break;

  case 60:
#line 517 "c-exp.y"
    { write_exp_elt_opcode (BINOP_LOGICAL_OR); }
    break;

  case 61:
#line 521 "c-exp.y"
    { write_exp_elt_opcode (TERNOP_COND); }
    break;

  case 62:
#line 525 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ASSIGN); }
    break;

  case 63:
#line 529 "c-exp.y"
    { write_exp_elt_opcode (BINOP_ASSIGN_MODIFY);
			  write_exp_elt_opcode (yyvsp[-1].opcode);
			  write_exp_elt_opcode (BINOP_ASSIGN_MODIFY); }
    break;

  case 64:
#line 535 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (yyvsp[0].typed_val_int.type);
			  write_exp_elt_longcst ((LONGEST)(yyvsp[0].typed_val_int.val));
			  write_exp_elt_opcode (OP_LONG); }
    break;

  case 65:
#line 542 "c-exp.y"
    {
			  struct stoken_vector vec;
			  vec.len = 1;
			  vec.tokens = &yyvsp[0].tsval;
			  write_exp_string_vector (yyvsp[0].tsval.type, &vec);
			}
    break;

  case 66:
#line 551 "c-exp.y"
    { YYSTYPE val;
			  parse_number (yyvsp[0].ssym.stoken.ptr, yyvsp[0].ssym.stoken.length, 0, &val);
			  write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (val.typed_val_int.type);
			  write_exp_elt_longcst ((LONGEST)val.typed_val_int.val);
			  write_exp_elt_opcode (OP_LONG);
			}
    break;

  case 67:
#line 562 "c-exp.y"
    { write_exp_elt_opcode (OP_DOUBLE);
			  write_exp_elt_type (yyvsp[0].typed_val_float.type);
			  write_exp_elt_dblcst (yyvsp[0].typed_val_float.dval);
			  write_exp_elt_opcode (OP_DOUBLE); }
    break;

  case 68:
#line 569 "c-exp.y"
    { write_exp_elt_opcode (OP_DECFLOAT);
			  write_exp_elt_type (yyvsp[0].typed_val_decfloat.type);
			  write_exp_elt_decfloatcst (yyvsp[0].typed_val_decfloat.val);
			  write_exp_elt_opcode (OP_DECFLOAT); }
    break;

  case 71:
#line 583 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
			  write_exp_elt_type (parse_type->builtin_int);
			  CHECK_TYPEDEF (yyvsp[-1].tval);
			  write_exp_elt_longcst ((LONGEST) TYPE_LENGTH (yyvsp[-1].tval));
			  write_exp_elt_opcode (OP_LONG); }
    break;

  case 72:
#line 592 "c-exp.y"
    {
			  /* We copy the string here, and not in the
			     lexer, to guarantee that we do not leak a
			     string.  Note that we follow the
			     NUL-termination convention of the
			     lexer.  */
			  struct typed_stoken *vec = XNEW (struct typed_stoken);
			  yyval.svec.len = 1;
			  yyval.svec.tokens = vec;

			  vec->type = yyvsp[0].tsval.type;
			  vec->length = yyvsp[0].tsval.length;
			  vec->ptr = xmalloc (yyvsp[0].tsval.length + 1);
			  memcpy (vec->ptr, yyvsp[0].tsval.ptr, yyvsp[0].tsval.length + 1);
			}
    break;

  case 73:
#line 609 "c-exp.y"
    {
			  /* Note that we NUL-terminate here, but just
			     for convenience.  */
			  char *p;
			  ++yyval.svec.len;
			  yyval.svec.tokens = xrealloc (yyval.svec.tokens,
					       yyval.svec.len * sizeof (struct typed_stoken));

			  p = xmalloc (yyvsp[0].tsval.length + 1);
			  memcpy (p, yyvsp[0].tsval.ptr, yyvsp[0].tsval.length + 1);

			  yyval.svec.tokens[yyval.svec.len - 1].type = yyvsp[0].tsval.type;
			  yyval.svec.tokens[yyval.svec.len - 1].length = yyvsp[0].tsval.length;
			  yyval.svec.tokens[yyval.svec.len - 1].ptr = p;
			}
    break;

  case 74:
#line 627 "c-exp.y"
    {
			  int i;
			  enum c_string_type type = C_STRING;

			  for (i = 0; i < yyvsp[0].svec.len; ++i)
			    {
			      switch (yyvsp[0].svec.tokens[i].type)
				{
				case C_STRING:
				  break;
				case C_WIDE_STRING:
				case C_STRING_16:
				case C_STRING_32:
				  if (type != C_STRING
				      && type != yyvsp[0].svec.tokens[i].type)
				    error ("Undefined string concatenation.");
				  type = yyvsp[0].svec.tokens[i].type;
				  break;
				default:
				  /* internal error */
				  internal_error (__FILE__, __LINE__,
						  "unrecognized type in string concatenation");
				}
			    }

			  write_exp_string_vector (type, &yyvsp[0].svec);
			  for (i = 0; i < yyvsp[0].svec.len; ++i)
			    xfree (yyvsp[0].svec.tokens[i].ptr);
			  xfree (yyvsp[0].svec.tokens);
			}
    break;

  case 75:
#line 661 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
                          write_exp_elt_type (parse_type->builtin_bool);
                          write_exp_elt_longcst ((LONGEST) 1);
                          write_exp_elt_opcode (OP_LONG); }
    break;

  case 76:
#line 668 "c-exp.y"
    { write_exp_elt_opcode (OP_LONG);
                          write_exp_elt_type (parse_type->builtin_bool);
                          write_exp_elt_longcst ((LONGEST) 0);
                          write_exp_elt_opcode (OP_LONG); }
    break;

  case 77:
#line 677 "c-exp.y"
    {
			  if (yyvsp[0].ssym.sym)
			    yyval.bval = SYMBOL_BLOCK_VALUE (yyvsp[0].ssym.sym);
			  else
			    error ("No file or function \"%s\".",
				   copy_name (yyvsp[0].ssym.stoken));
			}
    break;

  case 78:
#line 685 "c-exp.y"
    {
			  yyval.bval = yyvsp[0].bval;
			}
    break;

  case 79:
#line 691 "c-exp.y"
    { struct symbol *tem
			    = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					     VAR_DOMAIN, (int *) NULL);
			  if (!tem || SYMBOL_CLASS (tem) != LOC_BLOCK)
			    error ("No function \"%s\" in specified context.",
				   copy_name (yyvsp[0].sval));
			  yyval.bval = SYMBOL_BLOCK_VALUE (tem); }
    break;

  case 80:
#line 701 "c-exp.y"
    { struct symbol *sym;
			  sym = lookup_symbol (copy_name (yyvsp[0].sval), yyvsp[-2].bval,
					       VAR_DOMAIN, (int *) NULL);
			  if (sym == 0)
			    error ("No symbol \"%s\" in specified context.",
				   copy_name (yyvsp[0].sval));

			  write_exp_elt_opcode (OP_VAR_VALUE);
			  /* block_found is set by lookup_symbol.  */
			  write_exp_elt_block (block_found);
			  write_exp_elt_sym (sym);
			  write_exp_elt_opcode (OP_VAR_VALUE); }
    break;

  case 81:
#line 716 "c-exp.y"
    {
			  struct type *type = yyvsp[-2].tval;
			  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
			      && TYPE_CODE (type) != TYPE_CODE_UNION
			      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
			    error ("`%s' is not defined as an aggregate type.",
				   TYPE_NAME (type));

			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_type (type);
			  write_exp_string (yyvsp[0].sval);
			  write_exp_elt_opcode (OP_SCOPE);
			}
    break;

  case 82:
#line 730 "c-exp.y"
    {
			  struct type *type = yyvsp[-3].tval;
			  struct stoken tmp_token;
			  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
			      && TYPE_CODE (type) != TYPE_CODE_UNION
			      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
			    error ("`%s' is not defined as an aggregate type.",
				   TYPE_NAME (type));

			  tmp_token.ptr = (char*) alloca (yyvsp[0].sval.length + 2);
			  tmp_token.length = yyvsp[0].sval.length + 1;
			  tmp_token.ptr[0] = '~';
			  memcpy (tmp_token.ptr+1, yyvsp[0].sval.ptr, yyvsp[0].sval.length);
			  tmp_token.ptr[tmp_token.length] = 0;

			  /* Check for valid destructor name.  */
			  destructor_name_p (tmp_token.ptr, type);
			  write_exp_elt_opcode (OP_SCOPE);
			  write_exp_elt_type (type);
			  write_exp_string (tmp_token);
			  write_exp_elt_opcode (OP_SCOPE);
			}
    break;

  case 84:
#line 756 "c-exp.y"
    {
			  char *name = copy_name (yyvsp[0].sval);
			  struct symbol *sym;
			  struct minimal_symbol *msymbol;

			  sym =
			    lookup_symbol (name, (const struct block *) NULL,
					   VAR_DOMAIN, (int *) NULL);
			  if (sym)
			    {
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      write_exp_elt_block (NULL);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			      break;
			    }

			  msymbol = lookup_minimal_symbol (name, NULL, NULL);
			  if (msymbol != NULL)
			    write_exp_msymbol (msymbol);
			  else if (!have_full_symbols () && !have_partial_symbols ())
			    error ("No symbol table is loaded.  Use the \"file\" command.");
			  else
			    error ("No symbol \"%s\" in current context.", name);
			}
    break;

  case 85:
#line 784 "c-exp.y"
    { struct symbol *sym = yyvsp[0].ssym.sym;

			  if (sym)
			    {
			      if (symbol_read_needs_frame (sym))
				{
				  if (innermost_block == 0 ||
				      contained_in (block_found, 
						    innermost_block))
				    innermost_block = block_found;
				}

			      write_exp_elt_opcode (OP_VAR_VALUE);
			      /* We want to use the selected frame, not
				 another more inner frame which happens to
				 be in the same block.  */
			      write_exp_elt_block (NULL);
			      write_exp_elt_sym (sym);
			      write_exp_elt_opcode (OP_VAR_VALUE);
			    }
			  else if (yyvsp[0].ssym.is_a_field_of_this)
			    {
			      /* C++: it hangs off of `this'.  Must
			         not inadvertently convert from a method call
				 to data ref.  */
			      if (innermost_block == 0 || 
				  contained_in (block_found, innermost_block))
				innermost_block = block_found;
			      write_exp_elt_opcode (OP_THIS);
			      write_exp_elt_opcode (OP_THIS);
			      write_exp_elt_opcode (STRUCTOP_PTR);
			      write_exp_string (yyvsp[0].ssym.stoken);
			      write_exp_elt_opcode (STRUCTOP_PTR);
			    }
			  else
			    {
			      struct minimal_symbol *msymbol;
			      char *arg = copy_name (yyvsp[0].ssym.stoken);

			      msymbol =
				lookup_minimal_symbol (arg, NULL, NULL);
			      if (msymbol != NULL)
				write_exp_msymbol (msymbol);
			      else if (!have_full_symbols () && !have_partial_symbols ())
				error ("No symbol table is loaded.  Use the \"file\" command.");
			      else
				error ("No symbol \"%s\" in current context.",
				       copy_name (yyvsp[0].ssym.stoken));
			    }
			}
    break;

  case 86:
#line 837 "c-exp.y"
    { push_type_address_space (copy_name (yyvsp[0].ssym.stoken));
		  push_type (tp_space_identifier);
		}
    break;

  case 94:
#line 859 "c-exp.y"
    { push_type (tp_pointer); yyval.voidval = 0; }
    break;

  case 95:
#line 861 "c-exp.y"
    { push_type (tp_pointer); yyval.voidval = yyvsp[0].voidval; }
    break;

  case 96:
#line 863 "c-exp.y"
    { push_type (tp_reference); yyval.voidval = 0; }
    break;

  case 97:
#line 865 "c-exp.y"
    { push_type (tp_reference); yyval.voidval = yyvsp[0].voidval; }
    break;

  case 99:
#line 870 "c-exp.y"
    { yyval.voidval = yyvsp[-1].voidval; }
    break;

  case 100:
#line 872 "c-exp.y"
    {
			  push_type_int (yyvsp[0].lval);
			  push_type (tp_array);
			}
    break;

  case 101:
#line 877 "c-exp.y"
    {
			  push_type_int (yyvsp[0].lval);
			  push_type (tp_array);
			  yyval.voidval = 0;
			}
    break;

  case 102:
#line 884 "c-exp.y"
    { push_type (tp_function); }
    break;

  case 103:
#line 886 "c-exp.y"
    { push_type (tp_function); }
    break;

  case 104:
#line 890 "c-exp.y"
    { yyval.lval = -1; }
    break;

  case 105:
#line 892 "c-exp.y"
    { yyval.lval = yyvsp[-1].typed_val_int.val; }
    break;

  case 106:
#line 896 "c-exp.y"
    { yyval.voidval = 0; }
    break;

  case 107:
#line 898 "c-exp.y"
    { xfree (yyvsp[-1].tvec); yyval.voidval = 0; }
    break;

  case 109:
#line 914 "c-exp.y"
    { yyval.tval = yyvsp[0].tsym.type; }
    break;

  case 110:
#line 916 "c-exp.y"
    { yyval.tval = parse_type->builtin_int; }
    break;

  case 111:
#line 918 "c-exp.y"
    { yyval.tval = parse_type->builtin_long; }
    break;

  case 112:
#line 920 "c-exp.y"
    { yyval.tval = parse_type->builtin_short; }
    break;

  case 113:
#line 922 "c-exp.y"
    { yyval.tval = parse_type->builtin_long; }
    break;

  case 114:
#line 924 "c-exp.y"
    { yyval.tval = parse_type->builtin_long; }
    break;

  case 115:
#line 926 "c-exp.y"
    { yyval.tval = parse_type->builtin_long; }
    break;

  case 116:
#line 928 "c-exp.y"
    { yyval.tval = parse_type->builtin_long; }
    break;

  case 117:
#line 930 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_long; }
    break;

  case 118:
#line 932 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_long; }
    break;

  case 119:
#line 934 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_long; }
    break;

  case 120:
#line 936 "c-exp.y"
    { yyval.tval = parse_type->builtin_long_long; }
    break;

  case 121:
#line 938 "c-exp.y"
    { yyval.tval = parse_type->builtin_long_long; }
    break;

  case 122:
#line 940 "c-exp.y"
    { yyval.tval = parse_type->builtin_long_long; }
    break;

  case 123:
#line 942 "c-exp.y"
    { yyval.tval = parse_type->builtin_long_long; }
    break;

  case 124:
#line 944 "c-exp.y"
    { yyval.tval = parse_type->builtin_long_long; }
    break;

  case 125:
#line 946 "c-exp.y"
    { yyval.tval = parse_type->builtin_long_long; }
    break;

  case 126:
#line 948 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_long_long; }
    break;

  case 127:
#line 950 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_long_long; }
    break;

  case 128:
#line 952 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_long_long; }
    break;

  case 129:
#line 954 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_long_long; }
    break;

  case 130:
#line 956 "c-exp.y"
    { yyval.tval = parse_type->builtin_short; }
    break;

  case 131:
#line 958 "c-exp.y"
    { yyval.tval = parse_type->builtin_short; }
    break;

  case 132:
#line 960 "c-exp.y"
    { yyval.tval = parse_type->builtin_short; }
    break;

  case 133:
#line 962 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_short; }
    break;

  case 134:
#line 964 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_short; }
    break;

  case 135:
#line 966 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_short; }
    break;

  case 136:
#line 968 "c-exp.y"
    { yyval.tval = parse_type->builtin_double; }
    break;

  case 137:
#line 970 "c-exp.y"
    { yyval.tval = parse_type->builtin_long_double; }
    break;

  case 138:
#line 972 "c-exp.y"
    { yyval.tval = lookup_struct (copy_name (yyvsp[0].sval),
					      expression_context_block); }
    break;

  case 139:
#line 975 "c-exp.y"
    { yyval.tval = lookup_struct (copy_name (yyvsp[0].sval),
					      expression_context_block); }
    break;

  case 140:
#line 978 "c-exp.y"
    { yyval.tval = lookup_union (copy_name (yyvsp[0].sval),
					     expression_context_block); }
    break;

  case 141:
#line 981 "c-exp.y"
    { yyval.tval = lookup_enum (copy_name (yyvsp[0].sval),
					    expression_context_block); }
    break;

  case 142:
#line 984 "c-exp.y"
    { yyval.tval = lookup_unsigned_typename (parse_language,
							 parse_gdbarch,
							 TYPE_NAME(yyvsp[0].tsym.type)); }
    break;

  case 143:
#line 988 "c-exp.y"
    { yyval.tval = parse_type->builtin_unsigned_int; }
    break;

  case 144:
#line 990 "c-exp.y"
    { yyval.tval = lookup_signed_typename (parse_language,
						       parse_gdbarch,
						       TYPE_NAME(yyvsp[0].tsym.type)); }
    break;

  case 145:
#line 994 "c-exp.y"
    { yyval.tval = parse_type->builtin_int; }
    break;

  case 146:
#line 999 "c-exp.y"
    { yyval.tval = lookup_template_type(copy_name(yyvsp[-3].sval), yyvsp[-1].tval,
						    expression_context_block);
			}
    break;

  case 147:
#line 1003 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[0].tval); }
    break;

  case 148:
#line 1005 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[-1].tval); }
    break;

  case 150:
#line 1055 "c-exp.y"
    {
		  struct type *type = yyvsp[-2].tval;
		  struct type *new_type;
		  char *ncopy = alloca (yyvsp[0].sval.length + 1);

		  memcpy (ncopy, yyvsp[0].sval.ptr, yyvsp[0].sval.length);
		  ncopy[yyvsp[0].sval.length] = '\0';

		  if (TYPE_CODE (type) != TYPE_CODE_STRUCT
		      && TYPE_CODE (type) != TYPE_CODE_UNION
		      && TYPE_CODE (type) != TYPE_CODE_NAMESPACE)
		    error ("`%s' is not defined as an aggregate type.",
			   TYPE_NAME (type));

		  new_type = cp_lookup_nested_type (type, ncopy,
						    expression_context_block);
		  if (new_type == NULL)
		    error ("No type \"%s\" within class or namespace \"%s\".",
			   ncopy, TYPE_NAME (type));
		  
		  yyval.tval = new_type;
		}
    break;

  case 152:
#line 1081 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "int";
		  yyval.tsym.stoken.length = 3;
		  yyval.tsym.type = parse_type->builtin_int;
		}
    break;

  case 153:
#line 1087 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "long";
		  yyval.tsym.stoken.length = 4;
		  yyval.tsym.type = parse_type->builtin_long;
		}
    break;

  case 154:
#line 1093 "c-exp.y"
    {
		  yyval.tsym.stoken.ptr = "short";
		  yyval.tsym.stoken.length = 5;
		  yyval.tsym.type = parse_type->builtin_short;
		}
    break;

  case 155:
#line 1102 "c-exp.y"
    { yyval.tvec = (struct type **) xmalloc (sizeof (struct type *) * 2);
		  yyval.ivec[0] = 1;	/* Number of types in vector */
		  yyval.tvec[1] = yyvsp[0].tval;
		}
    break;

  case 156:
#line 1107 "c-exp.y"
    { int len = sizeof (struct type *) * (++(yyvsp[-2].ivec[0]) + 1);
		  yyval.tvec = (struct type **) xrealloc ((char *) yyvsp[-2].tvec, len);
		  yyval.tvec[yyval.ivec[0]] = yyvsp[0].tval;
		}
    break;

  case 158:
#line 1115 "c-exp.y"
    { yyval.tval = follow_types (yyvsp[-3].tval); }
    break;

  case 161:
#line 1123 "c-exp.y"
    { push_type (tp_const);
			  push_type (tp_volatile); 
			}
    break;

  case 162:
#line 1127 "c-exp.y"
    { push_type (tp_const); }
    break;

  case 163:
#line 1129 "c-exp.y"
    { push_type (tp_volatile); }
    break;

  case 164:
#line 1132 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;

  case 165:
#line 1133 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;

  case 166:
#line 1134 "c-exp.y"
    { yyval.sval = yyvsp[0].tsym.stoken; }
    break;

  case 167:
#line 1135 "c-exp.y"
    { yyval.sval = yyvsp[0].ssym.stoken; }
    break;


    }

/* Line 991 of yacc.c.  */
#line 2614 "../../gdb/c-exp.c.tmp"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("syntax error, unexpected ") + 1;
	  yysize += yystrlen (yytname[yytype]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror ("syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* Return failure if at end of input.  */
      if (yychar == YYEOF)
        {
	  /* Pop the error token.  */
          YYPOPSTACK;
	  /* Pop the rest of the stack.  */
	  while (yyss < yyssp)
	    {
	      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
	      yydestruct (yystos[*yyssp], yyvsp);
	      YYPOPSTACK;
	    }
	  YYABORT;
        }

      YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
      yydestruct (yytoken, &yylval);
      yychar = YYEMPTY;

    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab2;


/*----------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action.  |
`----------------------------------------------------*/
yyerrlab1:

  /* Suppress GCC warning that yyerrlab1 is unused when no action
     invokes YYERROR.  */
#if defined (__GNUC_MINOR__) && 2093 <= (__GNUC__ * 1000 + __GNUC_MINOR__) \
    && !defined __cplusplus
  __attribute__ ((__unused__))
#endif


  goto yyerrlab2;


/*---------------------------------------------------------------.
| yyerrlab2 -- pop states until the error token can be shifted.  |
`---------------------------------------------------------------*/
yyerrlab2:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      yyvsp--;
      yystate = *--yyssp;

      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 1149 "c-exp.y"


/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/*** Needs some error checking for the float case ***/

static int
parse_number (char *p, int len, int parsed_float, YYSTYPE *putithere)
{
  /* FIXME: Shouldn't these be unsigned?  We don't deal with negative values
     here, and we do kind of silly things like cast to unsigned.  */
  LONGEST n = 0;
  LONGEST prevn = 0;
  ULONGEST un;

  int i = 0;
  int c;
  int base = input_radix;
  int unsigned_p = 0;

  /* Number of "L" suffixes encountered.  */
  int long_p = 0;

  /* We have found a "L" or "U" suffix.  */
  int found_suffix = 0;

  ULONGEST high_bit;
  struct type *signed_type;
  struct type *unsigned_type;

  if (parsed_float)
    {
      /* It's a float since it contains a point or an exponent.  */
      char *s;
      int num;	/* number of tokens scanned by scanf */
      char saved_char;

      /* If it ends at "df", "dd" or "dl", take it as type of decimal floating
         point.  Return DECFLOAT.  */

      if (len >= 2 && p[len - 2] == 'd' && p[len - 1] == 'f')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = parse_type->builtin_decfloat;
	  decimal_from_string (putithere->typed_val_decfloat.val, 4,
			       gdbarch_byte_order (parse_gdbarch), p);
	  p[len - 2] = 'd';
	  return DECFLOAT;
	}

      if (len >= 2 && p[len - 2] == 'd' && p[len - 1] == 'd')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = parse_type->builtin_decdouble;
	  decimal_from_string (putithere->typed_val_decfloat.val, 8,
			       gdbarch_byte_order (parse_gdbarch), p);
	  p[len - 2] = 'd';
	  return DECFLOAT;
	}

      if (len >= 2 && p[len - 2] == 'd' && p[len - 1] == 'l')
	{
	  p[len - 2] = '\0';
	  putithere->typed_val_decfloat.type
	    = parse_type->builtin_declong;
	  decimal_from_string (putithere->typed_val_decfloat.val, 16,
			       gdbarch_byte_order (parse_gdbarch), p);
	  p[len - 2] = 'd';
	  return DECFLOAT;
	}

      s = xmalloc (len);
      saved_char = p[len];
      p[len] = 0;	/* null-terminate the token */
      num = sscanf (p, "%" DOUBLEST_SCAN_FORMAT "%s",
		    &putithere->typed_val_float.dval, s);
      p[len] = saved_char;	/* restore the input stream */

      if (num == 1)
	putithere->typed_val_float.type = 
	  parse_type->builtin_double;

      if (num == 2 )
	{
	  /* See if it has any float suffix: 'f' for float, 'l' for long 
	     double.  */
	  if (!strcasecmp (s, "f"))
	    putithere->typed_val_float.type = 
	      parse_type->builtin_float;
	  else if (!strcasecmp (s, "l"))
	    putithere->typed_val_float.type = 
	      parse_type->builtin_long_double;
	  else
	    {
	      xfree (s);
	      return ERROR;
	    }
	}

      xfree (s);
      return FLOAT;
    }

  /* Handle base-switching prefixes 0x, 0t, 0d, 0 */
  if (p[0] == '0')
    switch (p[1])
      {
      case 'x':
      case 'X':
	if (len >= 3)
	  {
	    p += 2;
	    base = 16;
	    len -= 2;
	  }
	break;

      case 't':
      case 'T':
      case 'd':
      case 'D':
	if (len >= 3)
	  {
	    p += 2;
	    base = 10;
	    len -= 2;
	  }
	break;

      default:
	base = 8;
	break;
      }

  while (len-- > 0)
    {
      c = *p++;
      if (c >= 'A' && c <= 'Z')
	c += 'a' - 'A';
      if (c != 'l' && c != 'u')
	n *= base;
      if (c >= '0' && c <= '9')
	{
	  if (found_suffix)
	    return ERROR;
	  n += i = c - '0';
	}
      else
	{
	  if (base > 10 && c >= 'a' && c <= 'f')
	    {
	      if (found_suffix)
		return ERROR;
	      n += i = c - 'a' + 10;
	    }
	  else if (c == 'l')
	    {
	      ++long_p;
	      found_suffix = 1;
	    }
	  else if (c == 'u')
	    {
	      unsigned_p = 1;
	      found_suffix = 1;
	    }
	  else
	    return ERROR;	/* Char not a digit */
	}
      if (i >= base)
	return ERROR;		/* Invalid digit in this base */

      /* Portably test for overflow (only works for nonzero values, so make
	 a second check for zero).  FIXME: Can't we just make n and prevn
	 unsigned and avoid this?  */
      if (c != 'l' && c != 'u' && (prevn >= n) && n != 0)
	unsigned_p = 1;		/* Try something unsigned */

      /* Portably test for unsigned overflow.
	 FIXME: This check is wrong; for example it doesn't find overflow
	 on 0x123456789 when LONGEST is 32 bits.  */
      if (c != 'l' && c != 'u' && n != 0)
	{	
	  if ((unsigned_p && (ULONGEST) prevn >= (ULONGEST) n))
	    error ("Numeric constant too large.");
	}
      prevn = n;
    }

  /* An integer constant is an int, a long, or a long long.  An L
     suffix forces it to be long; an LL suffix forces it to be long
     long.  If not forced to a larger size, it gets the first type of
     the above that it fits in.  To figure out whether it fits, we
     shift it right and see whether anything remains.  Note that we
     can't shift sizeof (LONGEST) * HOST_CHAR_BIT bits or more in one
     operation, because many compilers will warn about such a shift
     (which always produces a zero result).  Sometimes gdbarch_int_bit
     or gdbarch_long_bit will be that big, sometimes not.  To deal with
     the case where it is we just always shift the value more than
     once, with fewer bits each time.  */

  un = (ULONGEST)n >> 2;
  if (long_p == 0
      && (un >> (gdbarch_int_bit (parse_gdbarch) - 2)) == 0)
    {
      high_bit = ((ULONGEST)1) << (gdbarch_int_bit (parse_gdbarch) - 1);

      /* A large decimal (not hex or octal) constant (between INT_MAX
	 and UINT_MAX) is a long or unsigned long, according to ANSI,
	 never an unsigned int, but this code treats it as unsigned
	 int.  This probably should be fixed.  GCC gives a warning on
	 such constants.  */

      unsigned_type = parse_type->builtin_unsigned_int;
      signed_type = parse_type->builtin_int;
    }
  else if (long_p <= 1
	   && (un >> (gdbarch_long_bit (parse_gdbarch) - 2)) == 0)
    {
      high_bit = ((ULONGEST)1) << (gdbarch_long_bit (parse_gdbarch) - 1);
      unsigned_type = parse_type->builtin_unsigned_long;
      signed_type = parse_type->builtin_long;
    }
  else
    {
      int shift;
      if (sizeof (ULONGEST) * HOST_CHAR_BIT 
	  < gdbarch_long_long_bit (parse_gdbarch))
	/* A long long does not fit in a LONGEST.  */
	shift = (sizeof (ULONGEST) * HOST_CHAR_BIT - 1);
      else
	shift = (gdbarch_long_long_bit (parse_gdbarch) - 1);
      high_bit = (ULONGEST) 1 << shift;
      unsigned_type = parse_type->builtin_unsigned_long_long;
      signed_type = parse_type->builtin_long_long;
    }

   putithere->typed_val_int.val = n;

   /* If the high bit of the worked out type is set then this number
      has to be unsigned. */

   if (unsigned_p || (n & high_bit)) 
     {
       putithere->typed_val_int.type = unsigned_type;
     }
   else 
     {
       putithere->typed_val_int.type = signed_type;
     }

   return INT;
}

/* Temporary obstack used for holding strings.  */
static struct obstack tempbuf;
static int tempbuf_init;

/* Parse a C escape sequence.  The initial backslash of the sequence
   is at (*PTR)[-1].  *PTR will be updated to point to just after the
   last character of the sequence.  If OUTPUT is not NULL, the
   translated form of the escape sequence will be written there.  If
   OUTPUT is NULL, no output is written and the call will only affect
   *PTR.  If an escape sequence is expressed in target bytes, then the
   entire sequence will simply be copied to OUTPUT.  Return 1 if any
   character was emitted, 0 otherwise.  */

int
c_parse_escape (char **ptr, struct obstack *output)
{
  char *tokptr = *ptr;
  int result = 1;

  /* Some escape sequences undergo character set conversion.  Those we
     translate here.  */
  switch (*tokptr)
    {
      /* Hex escapes do not undergo character set conversion, so keep
	 the escape sequence for later.  */
    case 'x':
      if (output)
	obstack_grow_str (output, "\\x");
      ++tokptr;
      if (!isxdigit (*tokptr))
	error (_("\\x escape without a following hex digit"));
      while (isxdigit (*tokptr))
	{
	  if (output)
	    obstack_1grow (output, *tokptr);
	  ++tokptr;
	}
      break;

      /* Octal escapes do not undergo character set conversion, so
	 keep the escape sequence for later.  */
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      {
	int i;
	if (output)
	  obstack_grow_str (output, "\\");
	for (i = 0;
	     i < 3 && isdigit (*tokptr) && *tokptr != '8' && *tokptr != '9';
	     ++i)
	  {
	    if (output)
	      obstack_1grow (output, *tokptr);
	    ++tokptr;
	  }
      }
      break;

      /* We handle UCNs later.  We could handle them here, but that
	 would mean a spurious error in the case where the UCN could
	 be converted to the target charset but not the host
	 charset.  */
    case 'u':
    case 'U':
      {
	char c = *tokptr;
	int i, len = c == 'U' ? 8 : 4;
	if (output)
	  {
	    obstack_1grow (output, '\\');
	    obstack_1grow (output, *tokptr);
	  }
	++tokptr;
	if (!isxdigit (*tokptr))
	  error (_("\\%c escape without a following hex digit"), c);
	for (i = 0; i < len && isxdigit (*tokptr); ++i)
	  {
	    if (output)
	      obstack_1grow (output, *tokptr);
	    ++tokptr;
	  }
      }
      break;

      /* We must pass backslash through so that it does not
	 cause quoting during the second expansion.  */
    case '\\':
      if (output)
	obstack_grow_str (output, "\\\\");
      ++tokptr;
      break;

      /* Escapes which undergo conversion.  */
    case 'a':
      if (output)
	obstack_1grow (output, '\a');
      ++tokptr;
      break;
    case 'b':
      if (output)
	obstack_1grow (output, '\b');
      ++tokptr;
      break;
    case 'f':
      if (output)
	obstack_1grow (output, '\f');
      ++tokptr;
      break;
    case 'n':
      if (output)
	obstack_1grow (output, '\n');
      ++tokptr;
      break;
    case 'r':
      if (output)
	obstack_1grow (output, '\r');
      ++tokptr;
      break;
    case 't':
      if (output)
	obstack_1grow (output, '\t');
      ++tokptr;
      break;
    case 'v':
      if (output)
	obstack_1grow (output, '\v');
      ++tokptr;
      break;

      /* GCC extension.  */
    case 'e':
      if (output)
	obstack_1grow (output, HOST_ESCAPE_CHAR);
      ++tokptr;
      break;

      /* Backslash-newline expands to nothing at all.  */
    case '\n':
      ++tokptr;
      result = 0;
      break;

      /* A few escapes just expand to the character itself.  */
    case '\'':
    case '\"':
    case '?':
      /* GCC extensions.  */
    case '(':
    case '{':
    case '[':
    case '%':
      /* Unrecognized escapes turn into the character itself.  */
    default:
      if (output)
	obstack_1grow (output, *tokptr);
      ++tokptr;
      break;
    }
  *ptr = tokptr;
  return result;
}

/* Parse a string or character literal from TOKPTR.  The string or
   character may be wide or unicode.  *OUTPTR is set to just after the
   end of the literal in the input string.  The resulting token is
   stored in VALUE.  This returns a token value, either STRING or
   CHAR, depending on what was parsed.  *HOST_CHARS is set to the
   number of host characters in the literal.  */
static int
parse_string_or_char (char *tokptr, char **outptr, struct typed_stoken *value,
		      int *host_chars)
{
  int quote, i;
  enum c_string_type type;

  /* Build the gdb internal form of the input string in tempbuf.  Note
     that the buffer is null byte terminated *only* for the
     convenience of debugging gdb itself and printing the buffer
     contents when the buffer contains no embedded nulls.  Gdb does
     not depend upon the buffer being null byte terminated, it uses
     the length string instead.  This allows gdb to handle C strings
     (as well as strings in other languages) with embedded null
     bytes */

  if (!tempbuf_init)
    tempbuf_init = 1;
  else
    obstack_free (&tempbuf, NULL);
  obstack_init (&tempbuf);

  /* Record the string type.  */
  if (*tokptr == 'L')
    {
      type = C_WIDE_STRING;
      ++tokptr;
    }
  else if (*tokptr == 'u')
    {
      type = C_STRING_16;
      ++tokptr;
    }
  else if (*tokptr == 'U')
    {
      type = C_STRING_32;
      ++tokptr;
    }
  else
    type = C_STRING;

  /* Skip the quote.  */
  quote = *tokptr;
  if (quote == '\'')
    type |= C_CHAR;
  ++tokptr;

  *host_chars = 0;

  while (*tokptr)
    {
      char c = *tokptr;
      if (c == '\\')
	{
	  ++tokptr;
	  *host_chars += c_parse_escape (&tokptr, &tempbuf);
	}
      else if (c == quote)
	break;
      else
	{
	  obstack_1grow (&tempbuf, c);
	  ++tokptr;
	  /* FIXME: this does the wrong thing with multi-byte host
	     characters.  We could use mbrlen here, but that would
	     make "set host-charset" a bit less useful.  */
	  ++*host_chars;
	}
    }

  if (*tokptr != quote)
    {
      if (quote == '"')
	error ("Unterminated string in expression.");
      else
	error ("Unmatched single quote.");
    }
  ++tokptr;

  value->type = type;
  value->ptr = obstack_base (&tempbuf);
  value->length = obstack_object_size (&tempbuf);

  *outptr = tokptr;

  return quote == '"' ? STRING : CHAR;
}

struct token
{
  char *operator;
  int token;
  enum exp_opcode opcode;
  int cxx_only;
};

static const struct token tokentab3[] =
  {
    {">>=", ASSIGN_MODIFY, BINOP_RSH, 0},
    {"<<=", ASSIGN_MODIFY, BINOP_LSH, 0},
    {"->*", ARROW_STAR, BINOP_END, 1}
  };

static const struct token tokentab2[] =
  {
    {"+=", ASSIGN_MODIFY, BINOP_ADD, 0},
    {"-=", ASSIGN_MODIFY, BINOP_SUB, 0},
    {"*=", ASSIGN_MODIFY, BINOP_MUL, 0},
    {"/=", ASSIGN_MODIFY, BINOP_DIV, 0},
    {"%=", ASSIGN_MODIFY, BINOP_REM, 0},
    {"|=", ASSIGN_MODIFY, BINOP_BITWISE_IOR, 0},
    {"&=", ASSIGN_MODIFY, BINOP_BITWISE_AND, 0},
    {"^=", ASSIGN_MODIFY, BINOP_BITWISE_XOR, 0},
    {"++", INCREMENT, BINOP_END, 0},
    {"--", DECREMENT, BINOP_END, 0},
    {"->", ARROW, BINOP_END, 0},
    {"&&", ANDAND, BINOP_END, 0},
    {"||", OROR, BINOP_END, 0},
    /* "::" is *not* only C++: gdb overrides its meaning in several
       different ways, e.g., 'filename'::func, function::variable.  */
    {"::", COLONCOLON, BINOP_END, 0},
    {"<<", LSH, BINOP_END, 0},
    {">>", RSH, BINOP_END, 0},
    {"==", EQUAL, BINOP_END, 0},
    {"!=", NOTEQUAL, BINOP_END, 0},
    {"<=", LEQ, BINOP_END, 0},
    {">=", GEQ, BINOP_END, 0},
#ifdef HAVE_TCL
    {".*", DOT_STAR, BINOP_END, 1},
    {"@@", SEG, BINOP_SUBSCRIPT, 0},	/* SANTOSH Modification here. */
    {"<:", ARRAY_OPEN, BINOP_SUBSCRIPT, 0},
    {":>", ARRAY_CLOSE, BINOP_SUBSCRIPT, 0}
#else
    {".*", DOT_STAR, BINOP_END, 1}
#endif
  };

/* Identifier-like tokens.  */
static const struct token ident_tokens[] =
  {
    {"unsigned", UNSIGNED, OP_NULL, 0},
    {"template", TEMPLATE, OP_NULL, 1},
    {"volatile", VOLATILE_KEYWORD, OP_NULL, 0},
    {"struct", STRUCT, OP_NULL, 0},
    {"signed", SIGNED_KEYWORD, OP_NULL, 0},
    {"sizeof", SIZEOF, OP_NULL, 0},
    {"double", DOUBLE_KEYWORD, OP_NULL, 0},
    {"false", FALSEKEYWORD, OP_NULL, 1},
    {"class", CLASS, OP_NULL, 1},
    {"union", UNION, OP_NULL, 0},
    {"short", SHORT, OP_NULL, 0},
    {"const", CONST_KEYWORD, OP_NULL, 0},
    {"enum", ENUM, OP_NULL, 0},
    {"long", LONG, OP_NULL, 0},
    {"true", TRUEKEYWORD, OP_NULL, 1},
    {"int", INT_KEYWORD, OP_NULL, 0},

    {"and", ANDAND, BINOP_END, 1},
    {"and_eq", ASSIGN_MODIFY, BINOP_BITWISE_AND, 1},
    {"bitand", '&', OP_NULL, 1},
    {"bitor", '|', OP_NULL, 1},
    {"compl", '~', OP_NULL, 1},
    {"not", '!', OP_NULL, 1},
    {"not_eq", NOTEQUAL, BINOP_END, 1},
    {"or", OROR, BINOP_END, 1},
    {"or_eq", ASSIGN_MODIFY, BINOP_BITWISE_IOR, 1},
    {"xor", '^', OP_NULL, 1},
    {"xor_eq", ASSIGN_MODIFY, BINOP_BITWISE_XOR, 1}
  };

/* When we find that lexptr (the global var defined in parse.c) is
   pointing at a macro invocation, we expand the invocation, and call
   scan_macro_expansion to save the old lexptr here and point lexptr
   into the expanded text.  When we reach the end of that, we call
   end_macro_expansion to pop back to the value we saved here.  The
   macro expansion code promises to return only fully-expanded text,
   so we don't need to "push" more than one level.

   This is disgusting, of course.  It would be cleaner to do all macro
   expansion beforehand, and then hand that to lexptr.  But we don't
   really know where the expression ends.  Remember, in a command like

     (gdb) break *ADDRESS if CONDITION

   we evaluate ADDRESS in the scope of the current frame, but we
   evaluate CONDITION in the scope of the breakpoint's location.  So
   it's simply wrong to try to macro-expand the whole thing at once.  */
static char *macro_original_text;

/* We save all intermediate macro expansions on this obstack for the
   duration of a single parse.  The expansion text may sometimes have
   to live past the end of the expansion, due to yacc lookahead.
   Rather than try to be clever about saving the data for a single
   token, we simply keep it all and delete it after parsing has
   completed.  */
static struct obstack expansion_obstack;

static void
scan_macro_expansion (char *expansion)
{
  char *copy;

  /* We'd better not be trying to push the stack twice.  */
  gdb_assert (! macro_original_text);

  /* Copy to the obstack, and then free the intermediate
     expansion.  */
  copy = obstack_copy0 (&expansion_obstack, expansion, strlen (expansion));
  xfree (expansion);

  /* Save the old lexptr value, so we can return to it when we're done
     parsing the expanded text.  */
  macro_original_text = lexptr;
  lexptr = copy;
}


static int
scanning_macro_expansion (void)
{
  return macro_original_text != 0;
}


static void 
finished_macro_expansion (void)
{
  /* There'd better be something to pop back to.  */
  gdb_assert (macro_original_text);

  /* Pop back to the original text.  */
  lexptr = macro_original_text;
  macro_original_text = 0;
}


static void
scan_macro_cleanup (void *dummy)
{
  if (macro_original_text)
    finished_macro_expansion ();

  obstack_free (&expansion_obstack, NULL);
}


/* The scope used for macro expansion.  */
static struct macro_scope *expression_macro_scope;

/* This is set if a NAME token appeared at the very end of the input
   string, with no whitespace separating the name from the EOF.  This
   is used only when parsing to do field name completion.  */
static int saw_name_at_eof;

/* This is set if the previously-returned token was a structure
   operator -- either '.' or ARROW.  This is used only when parsing to
   do field name completion.  */
static int last_was_structop;

/* Read one token, getting characters through lexptr.  */

static int
yylex (void)
{
  int c;
  int namelen;
  unsigned int i;
  char *tokstart;
  int saw_structop = last_was_structop;
  char *copy;

  last_was_structop = 0;

 retry:

  /* Check if this is a macro invocation that we need to expand.  */
  if (! scanning_macro_expansion ())
    {
      char *expanded = macro_expand_next (&lexptr,
                                          standard_macro_lookup,
                                          expression_macro_scope);

      if (expanded)
        scan_macro_expansion (expanded);
    }

  prev_lexptr = lexptr;

  tokstart = lexptr;
  /* See if it is a special token of length 3.  */
  for (i = 0; i < sizeof tokentab3 / sizeof tokentab3[0]; i++)
    if (strncmp (tokstart, tokentab3[i].operator, 3) == 0)
      {
	if (tokentab3[i].cxx_only
	    && parse_language->la_language != language_cplus)
	  break;

	lexptr += 3;
	yylval.opcode = tokentab3[i].opcode;
	return tokentab3[i].token;
      }

  /* See if it is a special token of length 2.  */
  for (i = 0; i < sizeof tokentab2 / sizeof tokentab2[0]; i++)
    if (strncmp (tokstart, tokentab2[i].operator, 2) == 0)
      {
	if (tokentab2[i].cxx_only
	    && parse_language->la_language != language_cplus)
	  break;

	lexptr += 2;
	yylval.opcode = tokentab2[i].opcode;
	if (in_parse_field && tokentab2[i].token == ARROW)
	  last_was_structop = 1;
	return tokentab2[i].token;
      }

  switch (c = *tokstart)
    {
    case 0:
      /* If we were just scanning the result of a macro expansion,
         then we need to resume scanning the original text.
	 If we're parsing for field name completion, and the previous
	 token allows such completion, return a COMPLETE token.
         Otherwise, we were already scanning the original text, and
         we're really done.  */
      if (scanning_macro_expansion ())
        {
          finished_macro_expansion ();
          goto retry;
        }
      else if (saw_name_at_eof)
	{
	  saw_name_at_eof = 0;
	  return COMPLETE;
	}
      else if (saw_structop)
	return COMPLETE;
      else
        return 0;

    case ' ':
    case '\t':
    case '\n':
      lexptr++;
      goto retry;

    case '[':
    case '(':
      paren_depth++;
      lexptr++;
      return c;

    case ']':
    case ')':
      if (paren_depth == 0)
	return 0;
      paren_depth--;
      lexptr++;
      return c;

    case ',':
      if (comma_terminates
          && paren_depth == 0
          && ! scanning_macro_expansion ())
	return 0;
      lexptr++;
      return c;

    case '.':
      /* Might be a floating point number.  */
      if (lexptr[1] < '0' || lexptr[1] > '9')
	{
	  if (in_parse_field)
	    last_was_structop = 1;
	  goto symbol;		/* Nope, must be a symbol. */
	}
      /* FALL THRU into number case.  */

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      {
	/* It's a number.  */
	int got_dot = 0, got_e = 0, toktype;
	char *p = tokstart;
	int hex = input_radix > 10;

	if (c == '0' && (p[1] == 'x' || p[1] == 'X'))
	  {
	    p += 2;
	    hex = 1;
	  }
	else if (c == '0' && (p[1]=='t' || p[1]=='T' || p[1]=='d' || p[1]=='D'))
	  {
	    p += 2;
	    hex = 0;
	  }

	for (;; ++p)
	  {
	    /* This test includes !hex because 'e' is a valid hex digit
	       and thus does not indicate a floating point number when
	       the radix is hex.  */
	    if (!hex && !got_e && (*p == 'e' || *p == 'E'))
	      got_dot = got_e = 1;
	    /* This test does not include !hex, because a '.' always indicates
	       a decimal floating point number regardless of the radix.  */
	    else if (!got_dot && *p == '.')
	      got_dot = 1;
	    else if (got_e && (p[-1] == 'e' || p[-1] == 'E')
		     && (*p == '-' || *p == '+'))
	      /* This is the sign of the exponent, not the end of the
		 number.  */
	      continue;
	    /* We will take any letters or digits.  parse_number will
	       complain if past the radix, or if L or U are not final.  */
	    else if ((*p < '0' || *p > '9')
		     && ((*p < 'a' || *p > 'z')
				  && (*p < 'A' || *p > 'Z')))
	      break;
	  }
	toktype = parse_number (tokstart, p - tokstart, got_dot|got_e, &yylval);
        if (toktype == ERROR)
	  {
	    char *err_copy = (char *) alloca (p - tokstart + 1);

	    memcpy (err_copy, tokstart, p - tokstart);
	    err_copy[p - tokstart] = 0;
	    error ("Invalid number \"%s\".", err_copy);
	  }
	lexptr = p;
	return toktype;
      }

    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '|':
    case '&':
    case '^':
    case '~':
    case '!':
    case '@':
    case '<':
    case '>':
    case '?':
    case ':':
    case '=':
    case '{':
    case '}':
    symbol:
      lexptr++;
      return c;

    case 'L':
    case 'u':
    case 'U':
      if (tokstart[1] != '"' && tokstart[1] != '\'')
	break;
      /* Fall through.  */
    case '\'':
    case '"':
      {
	int host_len;
	int result = parse_string_or_char (tokstart, &lexptr, &yylval.tsval,
					   &host_len);
	if (result == CHAR)
	  {
	    if (host_len == 0)
	      error ("Empty character constant.");
	    else if (host_len > 2 && c == '\'')
	      {
		++tokstart;
		namelen = lexptr - tokstart - 1;
		goto tryname;
	      }
	    else if (host_len > 1)
	      error ("Invalid character constant.");
	  }
	return result;
      }
    }

  if (!(c == '_' || c == '$'
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
    /* We must have come across a bad character (e.g. ';').  */
    error ("Invalid character '%c' in expression.", c);

  /* It's a name.  See how long it is.  */
  namelen = 0;
  for (c = tokstart[namelen];
       (c == '_' || c == '$' || (c >= '0' && c <= '9')
	|| (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '<');)
    {
      /* Template parameter lists are part of the name.
	 FIXME: This mishandles `print $a<4&&$a>3'.  */

      if (c == '<')
	{ 
               /* Scan ahead to get rest of the template specification.  Note
                  that we look ahead only when the '<' adjoins non-whitespace
                  characters; for comparison expressions, e.g. "a < b > c",
                  there must be spaces before the '<', etc. */
               
               char * p = find_template_name_end (tokstart + namelen);
               if (p)
                 namelen = p - tokstart;
               break;
	}
      c = tokstart[++namelen];
    }

  /* The token "if" terminates the expression and is NOT removed from
     the input stream.  It doesn't count if it appears in the
     expansion of a macro.  */
  if (namelen == 2
      && tokstart[0] == 'i'
      && tokstart[1] == 'f'
      && ! scanning_macro_expansion ())
    {
      return 0;
    }

  lexptr += namelen;

  tryname:

  yylval.sval.ptr = tokstart;
  yylval.sval.length = namelen;

  /* Catch specific keywords.  */
  copy = copy_name (yylval.sval);
  for (i = 0; i < sizeof ident_tokens / sizeof ident_tokens[0]; i++)
    if (strcmp (copy, ident_tokens[i].operator) == 0)
      {
	if (ident_tokens[i].cxx_only
	    && parse_language->la_language != language_cplus)
	  break;

	/* It is ok to always set this, even though we don't always
	   strictly need to.  */
	yylval.opcode = ident_tokens[i].opcode;
	return ident_tokens[i].token;
      }

  if (*tokstart == '$')
    {
      write_dollar_variable (yylval.sval);
      return VARIABLE;
    }
  
  /* Use token-type BLOCKNAME for symbols that happen to be defined as
     functions or symtabs.  If this is not so, then ...
     Use token-type TYPENAME for symbols that happen to be defined
     currently as names of types; NAME for other symbols.
     The caller is not constrained to care about the distinction.  */
  {
    struct symbol *sym;
    int is_a_field_of_this = 0;
    int hextype;

    sym = lookup_symbol (copy, expression_context_block,
			 VAR_DOMAIN,
			 parse_language->la_language == language_cplus
			 ? &is_a_field_of_this : (int *) NULL);
    /* Call lookup_symtab, not lookup_partial_symtab, in case there are
       no psymtabs (coff, xcoff, or some future change to blow away the
       psymtabs once once symbols are read).  */
    if (sym && SYMBOL_CLASS (sym) == LOC_BLOCK)
      {
	yylval.ssym.sym = sym;
	yylval.ssym.is_a_field_of_this = is_a_field_of_this;
	return BLOCKNAME;
      }
    else if (!sym)
      {				/* See if it's a file name. */
	struct symtab *symtab;

	symtab = lookup_symtab (copy);

	if (symtab)
	  {
	    yylval.bval = BLOCKVECTOR_BLOCK (BLOCKVECTOR (symtab), STATIC_BLOCK);
	    return FILENAME;
	  }
      }

    if (sym && SYMBOL_CLASS (sym) == LOC_TYPEDEF)
        {
	  /* NOTE: carlton/2003-09-25: There used to be code here to
	     handle nested types.  It didn't work very well.  See the
	     comment before qualified_type for more info.  */
	  yylval.tsym.type = SYMBOL_TYPE (sym);
	  return TYPENAME;
        }
    yylval.tsym.type
      = language_lookup_primitive_type_by_name (parse_language,
						parse_gdbarch, copy);
    if (yylval.tsym.type != NULL)
      return TYPENAME;

    /* Input names that aren't symbols but ARE valid hex numbers,
       when the input radix permits them, can be names or numbers
       depending on the parse.  Note we support radixes > 16 here.  */
    if (!sym && 
        ((tokstart[0] >= 'a' && tokstart[0] < 'a' + input_radix - 10) ||
         (tokstart[0] >= 'A' && tokstart[0] < 'A' + input_radix - 10)))
      {
 	YYSTYPE newlval;	/* Its value is ignored.  */
	hextype = parse_number (tokstart, namelen, 0, &newlval);
	if (hextype == INT)
	  {
	    yylval.ssym.sym = sym;
	    yylval.ssym.is_a_field_of_this = is_a_field_of_this;
	    return NAME_OR_INT;
	  }
      }

    /* Any other kind of symbol */
    yylval.ssym.sym = sym;
    yylval.ssym.is_a_field_of_this = is_a_field_of_this;
    if (in_parse_field && *lexptr == '\0')
      saw_name_at_eof = 1;
    return NAME;
  }
}

int
c_parse (void)
{
  int result;
  struct cleanup *back_to = make_cleanup (free_current_contents,
					  &expression_macro_scope);

  /* Set up the scope for macro expansion.  */
  expression_macro_scope = NULL;

  if (expression_context_block)
    expression_macro_scope
      = sal_macro_scope (find_pc_line (expression_context_pc, 0));
  else
    expression_macro_scope = default_macro_scope ();
  if (! expression_macro_scope)
    expression_macro_scope = user_macro_scope ();

  /* Initialize macro expansion code.  */
  obstack_init (&expansion_obstack);
  gdb_assert (! macro_original_text);
  make_cleanup (scan_macro_cleanup, 0);

  /* Initialize some state used by the lexer.  */
  last_was_structop = 0;
  saw_name_at_eof = 0;

  result = yyparse ();
  do_cleanups (back_to);
  return result;
}


void
yyerror (char *msg)
{
  if (prev_lexptr)
    lexptr = prev_lexptr;

  error ("A %s in expression, near `%s'.", (msg ? msg : "error"), lexptr);
}


