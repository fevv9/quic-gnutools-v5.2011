/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.  All Rights
# Reserved.  Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/* Target-dependent code for QUALCOMM QDSP6 GDB, the GNU Debugger.
   Copyright 2002, 2003, 2004 Free Software Foundation, Inc.

   This file is part of GDB.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "defs.h"
#include "arch-utils.h"
#include "gdbtypes.h"
#include "gdbcmd.h"
#include "gdbcore.h"
#include "gdb_string.h"
#include "gdb_assert.h"
#include "frame.h"
#include "frame-unwind.h"
#include "frame-base.h"
#include "trad-frame.h"
#include "symtab.h"
#include "symfile.h"
#include "value.h"
#include "inferior.h"
#include "dis-asm.h"
#include "objfiles.h"
#include "language.h"
#include "regcache.h"
#include "reggroups.h"
#include "floatformat.h"
#include "block.h"
#include "observer.h"
#include "infcall.h"
#include "remote.h"
#include "gdbthread.h"
#include "completer.h"
#include "qdsp6-tdep.h"
#include "cli/cli-decode.h"
#include "qdsp6-sim.h"


/* following register values taken from architecture register definitions */
#define MAXREGNAMELEN     50
#define NUM_GEN_REGS      32
#define NUM_PER_THREAD_CR 40
#define NUM_GLOBAL_REGS   64
#define TOTAL_PER_THREAD_REGS (NUM_GEN_REGS+NUM_PER_THREAD_CR)
#define NUM_GLOBAL_REGS 64
#define MAX_INTERRUPT 32
#define END_OFFSET             0xFFFFFFFF
#define SIM_ARG_MAX 256

/*******************************************************************
 *   Global Variables                                              *
 *******************************************************************/   

/* captures the arguments to the target passed thru the 
 * set targetargs command */
char *q6targetargsInfo[SIM_ARG_MAX];
char *current_q6_target = NULL;
int Q6_tcl_fe_state = 0;
int qdsp6_debug = 0;

/*******************************************************************
 *   Extern Variables                                              *
 *******************************************************************/   
extern void _initialize_qdsp6_tdep (void);
extern char *current_q6_target;
extern int frame_debug;

/*******************************************************************
 *   Static Variables                                              *
 *******************************************************************/   
/* for set globalregs command - contains the user typed string*/
static char *q6globalregsInfo = NULL;
/* for set interrupt command - contains the user typed string*/
static char *q6Interrupt      = NULL;
/* for hwthrdbg command; 0 indicates s/w thread debug; non-zero indicated 
   h/w thread debug */
static int q6hwthread_debug = 0;

static gdbarch_init_ftype qdsp6_gdbarch_init;
static gdbarch_register_name_ftype qdsp6_register_name;
static gdbarch_breakpoint_from_pc_ftype qdsp6_breakpoint_from_pc;
static gdbarch_adjust_breakpoint_address_ftype qdsp6_gdbarch_adjust_breakpoint_address;
static gdbarch_skip_prologue_ftype qdsp6_skip_prologue;
static LONGEST qdsp6_call_dummy_words[] ={0};

/* The list of available "info q6 " commands.  */
static struct cmd_list_element *q6cmdlist = NULL;

// To enable printing of register names 
typedef struct 
{
  char* reg_name; // reg name
  int   index; // Offset in reg file
} regtype_t;


// To automatically get all thread registers from arch
static regtype_t threadRegSetInfo_v2[]={
#include "v2/thread_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static regtype_t globalRegSetInfo_v2[]={
#include "v2/global_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static regtype_t threadRegSetInfo_v3[]={
#include "v3/thread_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static regtype_t globalRegSetInfo_v3[]={
#include "v3/global_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static regtype_t threadRegSetInfo_v4[]={
#include "v4/thread_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static regtype_t globalRegSetInfo_v4[]={
#include "v4/global_regs.h"
  {"", END_OFFSET}
};

/* static pointer which points to the v1/v2 thread registers */
static regtype_t * threadRegSetInfo;
/* static pointer which points to the v1/v2 thread registers */
static regtype_t * globalRegSetInfo;
static struct qdsp6_system_register_offsets *q6RegOffset = (struct qdsp6_system_register_offsets *)0;

enum 
{
  first_gpr_regnum          = 0,
  last_gpr_regnum           = NUM_GEN_REGS -1,   // 31
  qdsp6_num_pseudo_regs     = 0,                 // Pseudo registers 
  struct_return_regnum      = 0,                 // !!! DRP 
  first_fpr_regnum          = 64,
  last_fpr_regnum           = 63,
};


typedef enum 
{
  Q6_INV_VER                   = 0, 
  Q6_V1                        = 1,
  Q6_V2                        = 2,   // 31
  Q6_V3                        = 3,   // 31
  Q6_V4                        = 4,   // 31
}Qdsp6_Version;

int q6Version = 0;
int q6ArgRegMax = 0;
int qdsp6_version = 0;
int qdsp6_reg_index_max = 0;
int qdsp6_reg_count = 0;


struct qdsp6_unwind_cache		/* was struct frame_extra_info */
{
    /* The previous frame's inner-most stack address.  Used as this
       frame ID's stack_addr.  */
    CORE_ADDR prev_sp;
    
    /* The frame's base, optionally used by the high-level debug info.  */
    CORE_ADDR base;
    
    /* Table indicating the location of each and every register.  */
    struct trad_frame_saved_reg *saved_regs;
};


/* A structure describing the QDSP6 architecture.
   We allocate and initialize one of these structures when we create
   the gdbarch object for a variant.

   The portable code of GDB knows that registers whose names are the 
   empty string don't exist, so the  `register_names' array captures 
   all the per-variant information we   need.

   In the future, if we need to have per-variant maps for raw size,
   virtual type, etc., we should replace register_names with an array
   of structures, each of which gives all the necessary info for one
   register.  Don't stick parallel arrays in here --- that's so
   Fortran.  */
struct gdbarch_tdep
{
  /* Total number of  general-purpose registers  */
  int num_gprs;

  /* Total number of  floating-point registers   */
  int num_fprs;

  /* Total number of hardware watchpoints supported  */
  int num_hw_watchpoints;

  /* Total number of hardware breakpoints supported  */
  int num_hw_breakpoints;

  /* Register names.  */
  char **register_names;
};

/*
 * function: qdsp6_index_max
 * description:
 *	- Traverse the architecture's register file and return
 *	  the largest index.
 *      - This could be hard coded if the values didn't change
 *	- NOTE:
 *		- This is really geared for the simulator, different
 *		  layers use different index offsets.  In the ideal
 *	          world we would just use the indexes found in the
 *		  spec.
 *		  
 */
static int
qdsp6_index_max (regtype_t *rt)
{
  int max_index = 0;
  while (rt->index != END_OFFSET)
    {
      max_index = max (qdsp6_reg_index_max, rt->index);
      qdsp6_reg_count++;
      rt++;
    }

  if (qdsp6_debug == 1)
  {
    printf_filtered ("QDSP6_DEB: q6Version = %d, max reg index = %d, thread reg count = %d\n", q6Version, qdsp6_reg_index_max, qdsp6_reg_count);
  }

  return max_index;

}



/* Allocate a new variant structure, and set up default values for all
   the fields.  */
static struct gdbarch_tdep *
new_variant (void)
{
  struct gdbarch_tdep *var;
  int regnum;
  int reg_count;
  char buf[20];
  extern int q6Version;

  var = xmalloc (sizeof (*var));
  memset (var, 0, sizeof (*var));
  
  var->num_gprs = NUM_GEN_REGS;
  var->num_fprs = 0;
  var->num_hw_watchpoints = 0;
  var->num_hw_breakpoints = 20;

  gdb_assert (qdsp6_reg_index_max != 0);

  reg_count = qdsp6_reg_index_max+1;

  /* By default, don't supply any general-purpose or floating-point
     register names.  */
  var->register_names 
    = (char **) xmalloc ((reg_count)* sizeof (char *));

  for (regnum = 0; regnum < reg_count; regnum++)
    var->register_names[regnum] = "";

  /* Do, however, supply default names for the known special-purpose
     registers.  */
#if 0
printf ("REG_SSR = %d\n", REG_SSR);
printf ("REG_SA0 = %d\n", REG_SA0);
printf ("REG_SA1 = %d\n", REG_SA1);
printf ("REG_M0 = %d\n", REG_M0);
printf ("REG_M1 = %d\n", REG_M1);
printf ("REG_TID = %d\n", REG_TID);
printf ("REG_ELR = %d\n", REG_ELR);
printf ("REG_HTID = %d\n", REG_HTID);
printf ("REG_CCR = %d\n", REG_CCR);
#endif

  var->register_names[REG_SA0]     = "sa0";
  var->register_names[REG_SA1]     = "sa1";
  var->register_names[REG_LC0]     = "lc0";
  var->register_names[REG_LC1]     = "lc1";
  var->register_names[REG_P3_0]    = "p3:0";
  var->register_names[REG_M0]      = "m0";
  var->register_names[REG_M1]      = "m1";
  var->register_names[REG_USR]     = "usr";
  var->register_names[REG_PC]      = "pc";
  var->register_names[REG_UGP]     = "ugp";
  var->register_names[REG_GP]      = "gp";
  /* per thread Supervisor Control Registers (SCR )   */
  var->register_names[REG_SSR]     = "ssr"  ;
  var->register_names[REG_IMASK]   = "imask";
  var->register_names[REG_ELR]     = "elr";
  if(q6Version < Q6_V4)
  {
      var->register_names[REG_SGP]     = "sgp";
      var->register_names[REG_BADVA]   = "badva";
      var->register_names[REG_TID]     = "tid"; 
  }
  else
  {
      var->register_names[REG_SGP0]     = "sgp0";
      var->register_names[REG_BADVA0]   = "badva0";
      var->register_names[REG_BADVA1]   = "badva1";
      var->register_names[REG_STID]     = "stid"; 
      var->register_names[REG_HTID]     = "htid"; 
      var->register_names[REG_CCR]      = "ccr"; 
  }

  return var;
}


/* Indicate that Q6  has NUM_GPRS general-purpose
   registers, and fill in the names array appropriately.  */
static void
set_variant_num_gprs (struct gdbarch_tdep *var, int num_gprs)
{
  int r;

  var->num_gprs = num_gprs;

  for (r = 0; r < num_gprs; ++r)
    {
      char buf[20];
      sprintf (buf, "r%d", r);
      var->register_names[first_gpr_regnum + r] = xstrdup (buf);
    }
}


/* Indicate that  Q6 has has NUM_FPRS floating-point
   registers, and fill in the names array appropriately.  */
static void
set_variant_num_fprs (struct gdbarch_tdep *var, int num_fprs)
{
  int r;

  var->num_fprs = num_fprs;

  for (r = 0; r < num_fprs; ++r)
    {
      char buf[20];
      sprintf (buf, "fr%d", r);
      var->register_names[first_fpr_regnum + r] = xstrdup (buf);
    }
}


static const char *
qdsp6_register_name (struct gdbarch *gdbarch, int reg)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  if (reg < 0)
    return "?toosmall?";
  if (reg >  qdsp6_reg_index_max)
    return "?toolarge?";

  return tdep->register_names[reg];
}


static struct type *
qdsp6_register_type (struct gdbarch *gdbarch, int reg)
{
    return builtin_type (gdbarch)->builtin_uint32;
}

static void
qdsp6_pseudo_register_read (struct gdbarch *gdbarch, struct regcache *regcache,
                          int reg, void *buffer)
{
  return;
}

static void
qdsp6_pseudo_register_write (struct gdbarch *gdbarch, struct regcache *regcache,
                          int reg, const void *buffer)
{
  return;

}

static const unsigned char *
qdsp6_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *pcptr, int *lenp)
{
  /* breakpoint instruction is trap0(#0xDB) which encodes as  0c db 00 54 for v2*/
  static unsigned char breakpoint_insn[] = {0x0c, 0xdb, 0x00, 0x54};
  static unsigned char *breakpoint;

  if ((q6Version == Q6_V4) || 
      (q6Version == Q6_V3) ||
      (q6Version == Q6_V2))
  {
    breakpoint=breakpoint_insn;
    *lenp = sizeof (breakpoint_insn);
  } 
  else
  {
    printf_filtered ("%s, invalid setting for arch level %d\n",__func__, q6Version);
    gdb_assert(0);
  }
  return breakpoint;
}

/* Define the maximum number of instructions which may be packed into a
   packet (VLIW instruction).  */
static const int max_instrs_per_packet = 4;

/* Define the size (in bytes) of an qdsp6 instruction.  */
static const int qdsp6_instr_size = 4;

/* Adjust a breakpoint's address to account for the qdsp6 architecture's
   constraint that a break instruction must not appear as any but the
   first instruction in the packet.  */
static CORE_ADDR
qdsp6_gdbarch_adjust_breakpoint_address (struct gdbarch *gdbarch, 
					 CORE_ADDR bpaddr)
{

  int count =max_instrs_per_packet;
  CORE_ADDR addr = bpaddr - qdsp6_instr_size;
  CORE_ADDR func_start = get_pc_function_start (bpaddr);

  /* the dummy call is AT_ENTRY by default;
     don't adjust the breakpoint for dummy calls */
  if ((bpaddr == entry_point_address()) || (bpaddr == 0))
    return bpaddr;

  /* Find the end of the previous packing sequence.
     This will be indicated by finding an instruction word 
     whose packing bits (bit 14 & 15) is set to one.\ or attempting to access
     invalid memory*/
  while (count-- > 0 && addr >= func_start)
    {
      /* stores the instruction encoding as 4 bytes */
      char instr[qdsp6_instr_size];
      int status;

      /* read the memory content at this address to get 
	 the instruction encoding */
      status = target_read_memory (addr, instr, sizeof instr); 
      /*status = read_memory_nobpt (addr, instr, sizeof instr);*/
      if (status != 0)
	break;

      /* instruction encoding byte layout
	 xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx 
	 byte 0   byte 1   byte 2   byte 3   */
      /* This is a little endian architecture, so byte 1 will have bits 14
	 and  15. bit 14/15 set to 1 indicates 	 end of packet  */
      if((instr[1] & 0x80) && (instr[1] & 0x40)) 
	break;

      addr -= qdsp6_instr_size;
    }

  if (count >= 0)
    bpaddr = addr + qdsp6_instr_size;

  return bpaddr;
}



/* Return true if REG is a callee-saves register, false otherwise.  */
static int
is_callee_saves_reg (int reg)
{
  //printf("\n In s_callee_saves_reg ..................\n");
  if(q6Version == Q6_V2) 
      return (reg >= 24 && reg <= 27);
  else
      return (reg >= 16 && reg <= 27);
}


/* Return true if REG is an argument register, false otherwise.  */
static int
is_argument_reg (int reg)
{
  //printf("\n In is_argument_reg ...........................\n");
  return (reg < q6ArgRegMax);
}

/*
 * macro: is_rs29
 * description:
 *	- Checks insn bits, 16:20 to see if the register source
 *	  is 29 (0x1d).
 *	- WARNING verify encoding pattern when using!
 *		- OK with: (add, memb, memh, memw, memd)
 */
#define is_rs29(insn) ((insn & 0x001f0000) == 0x1d0000)

/*
 * macro: is_rd29
 * description:
 *	- Checks insn bits, 0:4 to see if the register destination
 *	  is 29 (0x1d).
 */
#define is_rd29(insn) ((insn & 0x1f) == 0x1d)

/*
 * function: is_allocframe
 * description:
 *	- given an instruction determine if it is an allocframe, if so
 *        return 1.
 */
static int
is_allocframe (unsigned int insn, CORE_ADDR pc, unsigned int *immediate)
{
  if (ALLOCFRAME_MATCH (insn))
    {
      *immediate = ALLOCFRAME_SIZE (insn);
      return 1;
    }
  else
    return 0;
}

/*
 * function: is_more_stack
 * description:
 *	- Are we asking for more stack, r29 = add (r29, ###)?  If so then
 *        return 1.
 */
static int
is_more_stack (unsigned int insn, CORE_ADDR pc, int *immediate)
{
  int rc = 0;
  unsigned int mask;
  enum
  {
    addi = 0xb0000000,
  };
  mask = addi;

  if (((insn & mask) == addi) && is_rs29 (insn) && is_rd29 (insn))
    {
      *immediate = (((insn & 0x0fe00000) >> 12) | ((insn & 0x3fe0) >> 5));
      rc = 1;
    }
  if (rc && *immediate & 0x8000)
    *immediate |= ~0x7fff;	/* Sign extend */

  return rc;
}

/*
 * function: is_memX_r29
 * description:
 *	- look at an insn and see if it is storing an insn
 *        onto the stack
 *	- memd insns will set 2 registers (reg2) otherwise reg2 == -1.
 */
static int
is_memX_r29 (unsigned int insn, CORE_ADDR pc,
	     int *stack_offset, unsigned int *reg1, unsigned int *reg2)
{
  int rc = 0;
  unsigned int mask;
  enum
  {
    memb = 0xa1000000,
    memh = 0xa1400000,
    memw = 0xa1800000,
    memd = 0xa1c00000
  };
  mask = memb | memh | memw | memd;
  *reg2 = -1;


  if (((insn & mask) == memw) && is_rs29 (insn))
    {
      *reg1 = (insn & 0x1f00) >> 8;
      rc = 1;
    }
  else if (((insn & mask) == memd) && is_rs29 (insn))
    {
      *reg1 = (insn & 0x1f00) >> 8;
      *reg2 = *reg1 + 1;
      *stack_offset = CALLEE_SAVE_OFFSET_D (insn);
      rc = 1;
    }
  else if (((insn & mask) == memh) && is_rs29 (insn))
    {
      *reg1 = (insn & 0x1f00) >> 8;
      *stack_offset = CALLEE_SAVE_OFFSET_H (insn);
      rc = 1;
    }
  else if (((insn & mask) == memb) && is_rs29 (insn))
    {
      *reg1 = (insn & 0x1f00) >> 8;
      *stack_offset = CALLEE_SAVE_OFFSET_B (insn);
      rc = 1;
    }

  if (rc && (*stack_offset & 0x1000))
    *stack_offset |= ~0x1fff;	/* Sign extend */

  if (frame_debug && rc == 1)
    {
      printf_filtered ("QDSP6:%s: PC:0x%lx saving reg %x:%x, offset = %d\n",
		       __func__, pc, *reg1, *reg2, *stack_offset);
    }

  return rc;
}

/*
 * function: is_immed
 * description:
 * 	- looks for an immext, a payload extender for a large
 *	  immediate to follow.
 */
static int
is_immed (unsigned int insn, CORE_ADDR pc)
{
  int rc = 0;
  if (IMMEXT_MATCH (insn))
    rc = 1;

  return rc;
}

/*
 * function: is_branch
 * description:
 * 	- looks for a call or jump, return 1 if found.
 *	- All branches are in 1 of 2 classes, class codes reside in bits
 *	  31:28.
 */
static int
is_branch (unsigned int insn, CORE_ADDR pc)
{
  int rc = 0;
  enum
  {
    branch1 = 0x50000000,
    branch2 = 0x60000000
  };
  unsigned int mask = branch1 | branch2;

  if (((insn & mask) == branch1) || ((insn & mask) == branch2))
    rc = 1;

  return rc;
}

/*
 * function: get_packet
 * description:
 * 	- fill in the provided buffer with 1-4 insn's that represent the
 *	  packet at the given pc.
 *	- return the number of insn's in the packet.
 *	- a return of 0 signifies failure of some kind
 */
static int
get_packet (CORE_ADDR pc, gdb_byte * buffer, enum bfd_endian byte_order)
{
  int i;
  int packet_cnt = 1;
  unsigned int insn;
  enum
  {
    endofpacket = 0xc000,
  };
  unsigned int mask = endofpacket;


  for (i = 0; i < 16; i += 4, pc += 4, packet_cnt++)
    {
      if (target_read_memory (pc, &buffer[i], 4))
	gdb_assert (0);

      insn = extract_unsigned_integer (&buffer[i], 4, byte_order);
      if ((insn & mask) == endofpacket)
	break;
    }

  return packet_cnt;
}

/* Scan the qdsp6 prologue, starting at PC, until frame->PC.
   If FRAME is non-zero, fill in its saved_regs with appropriate addresses.
   We assume FRAME's saved_regs array has already been allocated and cleared.
   Return the first PC value after the prologue.

   Note that, for unoptimized code, we almost don't need this function
   at all; all arguments and locals live on the stack, so we just need
   the FP to find everything.  The catch: structures passed by value
   have their addresses living in registers; they're never spilled to
   the stack.  So if you ever want to be able to get to these
   arguments in any frame but the top, you'll need to do this serious
   prologue analysis.  */

static CORE_ADDR
qdsp6_analyze_prologue (struct gdbarch *gdbarch,
			CORE_ADDR start_pc, CORE_ADDR end_pc,
		        struct frame_info *next_frame,
                        struct qdsp6_unwind_cache *info)
{
  ULONGEST op;
  gdb_byte *op_addr=(gdb_byte *)&op;
  gdb_byte buf[16];
  int end_of_prologue = 0;
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);

  int allocframe = 0;

  /* Non-zero iff we've seen the instruction that initializes the
     frame pointer for this function's frame.  */
  int fp_set = 0;

  int sp_mod_val = 0; /* allocframe argument value + other sp updates if
                      the change is too big for the allocframe encoding */

  /* If gr_saved[i] is non-zero, then we've noticed that general
     register i has been saved at gr_sp_offset[i] from the stack
     pointer.  */
  char gr_saved[32] = {0};
  int  gr_sp_offset[32];

  /* The address of the most recently scanned prologue instruction.  */
  CORE_ADDR prolog_pc;

  /* The upper bound to of the pc values to scan.  */
  CORE_ADDR lim_pc;
  CORE_ADDR pc;

  if (qdsp6_debug == 1)
      printf_filtered ("QDSP6_DEB: %s, pc = 0x%lx\n",__func__, start_pc);


  prolog_pc = start_pc;

  /* Try to compute an upper limit (on how far to scan) based on the
     line number info.  */
  lim_pc = skip_prologue_using_sal (gdbarch, start_pc);
  /* If there's no line number info, lim_pc will be 0.  In that case,
     set the limit to be 100 instructions away from pc.  Hopefully, this
     will be far enough away to account for the entire prologue.  Don't
     worry about overshooting the end of the function.  The scan loop
     below contains some checks to avoid scanning unreasonably far.  */
  if (lim_pc == 0)
    lim_pc = start_pc + 0x64;

  /* If we have a frame, we don't want to scan past the frame's pc.  This
     will catch those cases where the pc is in the prologue.  */
  if (next_frame)
    {
      CORE_ADDR frame_pc = get_frame_pc (next_frame);
      if (frame_pc < lim_pc)
	lim_pc = frame_pc;
    }

/*
 * XXX_SM:
 * Note: There is something to consider and that is when pc == end_pc but
 * the current pc is at the start of the packet.  I think we need to
 * determine if this pc is part of a packet and if it is then examine the
 * whole packet not just the first insn.
 *
 * WHEN end_pc == start_pc WE ARE AT THE BEGINNING OF A FUNCTION.
 *
 * A target_read_packet function that will advance the pc by the number of
 * insn's that are contained in the packet anywhere from 4 to 16 bytes in
 * 4 byte increments looking for the PP (parse bits) for execute 
 * 15:14 = 1:1 to signify the end of packet
 */
  pc = start_pc;
  while (pc <= end_pc && !end_of_prologue)
    {
      unsigned int insn;
      unsigned int operand;
      unsigned int offset;
      int reg1;
      int reg2;
      int packet_total;
      int pcnt;
 
      packet_total = get_packet (pc, buf, byte_order);

      for (pcnt = 0; pcnt<packet_total; pcnt++)
        {
          insn = extract_unsigned_integer (&buf[pcnt*4], 4, byte_order);

          if (is_allocframe(insn, pc, &operand))
            {
	      prolog_pc+=4;
	      allocframe = -operand;	/* Stack grows downward */

	      if (start_pc == end_pc)
	          fp_set = 1;
            }
          else if (is_more_stack (insn, pc, &operand))
	    {
	      prolog_pc+=4;
	      allocframe += operand;
	    }
    
          else if (is_memX_r29 (insn, pc, &offset, &reg1, &reg2))
            {
	      prolog_pc+=4;
              gr_saved[reg1] = 1;
              gr_sp_offset[reg1] = offset;
	      if (reg2 > 0)
	        {
                  gr_saved[reg2] = 1;
                  gr_sp_offset[reg2] = offset+4; /* TODO check this is + not - */
	        }
            }
          else if (is_immed (insn, pc))
            {
	      prolog_pc+=4;
            }
          else if (is_branch(insn, pc)) /* no branches in prologue */
            {
		end_of_prologue = 1;
	        break;
            }
          else 
            {
            }
        }

	pc += packet_total * 4;
    }

  if (next_frame && info)
    {
      int i;
      ULONGEST this_base;
      ULONGEST this_sp;
      ULONGEST this_pc;
      ULONGEST this_lr;
      ULONGEST this_fp;
      gdb_byte buf[4];

      this_sp = get_frame_register_unsigned (next_frame, REG_SP);
      this_pc = get_frame_register_unsigned (next_frame, REG_PC);
      this_lr = get_frame_register_unsigned (next_frame, REG_LR);

      if (fp_set)
      {
          this_base = get_frame_register_unsigned (next_frame, REG_SP);
	  this_base -= 8;
/*
 * Store the back chain
 */
          this_fp = get_frame_register_unsigned (next_frame, REG_FP);
	  target_write_memory (this_base, &this_fp, 4);
	  target_write_memory (this_base+4, &this_lr, 4);
      }
      else
          this_base = get_frame_register_unsigned (next_frame, REG_FP);


      for (i = 0; i < 32; i++)
	if (gr_saved[i])
	  info->saved_regs[i].addr = this_base + sp_mod_val + gr_sp_offset[i];

      info->saved_regs[REG_FP].addr = this_base;
      info->saved_regs[REG_LR].addr = this_base + 4;

      info->saved_regs[REG_PC] = info->saved_regs[REG_LR];

      info->prev_sp = this_base + 8; /* +4 for saved R30, +4 for saved R31 */
      info->base = this_base;
      trad_frame_set_value (info->saved_regs, REG_SP, info->prev_sp);

      if (qdsp6_debug == 1)
      {
	printf_filtered ("\t QDSP6_DEB: allocframe = %d\n", allocframe);
	printf_filtered ("\t QDSP6_DEB: info.base = 0x%p\n", info->base);
	printf_filtered ("\t QDSP6_DEB: reg_sp = 0x%p\n", this_sp);
	printf_filtered ("\t QDSP6_DEB: this_lr = 0x%p\n", this_lr);
	printf_filtered ("\t QDSP6_DEB: info.REG_FP  = 0x%x\n", info->saved_regs[REG_FP].addr);
	printf_filtered ("\t QDSP6_DEB: info.REG_LR  = 0x%x\n", info->saved_regs[REG_LR].addr);
	printf_filtered ("\t QDSP6_DEB: info.REG_PC  = 0x%x\n", info->saved_regs[REG_PC].addr);
	printf_filtered ("\t QDSP6_DEB: info.prev_sp = 0x%x\n", info->saved_regs[REG_PC].addr);
      }


    }

  return prolog_pc;
}


static CORE_ADDR
qdsp6_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR pc)
{
  CORE_ADDR func_addr, func_end, new_pc;

  new_pc = pc;

  /* If the line table has entry for a line *within* the function
     (i.e., not in the prologue, and not past the end), then that's
     our location.  */

  if (qdsp6_debug == 1)
  {
      printf_filtered ("QDSP6_DEB: %s\n",__func__);
      printf_filtered ("\tQDSP6_DEB: pc = 0x%lx\n", pc);
  }

  if (find_pc_partial_function (pc, NULL, &func_addr, &func_end))
    {
      struct symtab_and_line sal;

      sal = find_pc_line (func_addr, 0);

      /* printf(" * * sal.line = 0x%08x, sal.end = 0x%08x\n", sal.line, sal.end); */

      if (sal.line != 0 && sal.end < func_end)
	{
	  new_pc = sal.pc;
	}
    }

  /* qdsp6 prologue is at least 4 bytes (allocframe) */
  if (new_pc < pc + 4) 
    new_pc = qdsp6_analyze_prologue (gdbarch, pc, pc, 0, 0);

  if (qdsp6_debug == 1)
  {
      printf_filtered ("\tQDSP6_DEB: returns new_pc = 0x%lx\n", new_pc);
  }
  return new_pc;
}


static struct qdsp6_unwind_cache *
qdsp6_frame_unwind_cache (struct frame_info *this_frame,
			 void **this_prologue_cache)
{
  struct gdbarch *gdbarch = get_frame_arch (this_frame);
  CORE_ADDR pc;
  ULONGEST prev_sp;
  ULONGEST this_base;
  struct qdsp6_unwind_cache *info;

  if (qdsp6_debug == 1)
  {
      printf_filtered("QDSP6_DEB: %s\n",__func__);
      printf_filtered("\tQDSP6_DEB: *this_prologue_cache = %p\n",
	     *this_prologue_cache);
  }

  if ((*this_prologue_cache))
    return (*this_prologue_cache);

  info = FRAME_OBSTACK_ZALLOC (struct qdsp6_unwind_cache);
  (*this_prologue_cache) = info;
  info->saved_regs = trad_frame_alloc_saved_regs (this_frame);

  /* Prologue analysis does the rest...  */
  pc = get_frame_pc(this_frame);
  if (qdsp6_debug == 1)
      printf_filtered("\tpre ana: QDSP6_DEB: pc = 0x%x\n",pc);
  pc = qdsp6_analyze_prologue (gdbarch,
			get_frame_func (this_frame), get_frame_pc(this_frame),
			this_frame, info);

  if (qdsp6_debug == 1)
  {
      printf_filtered("\tQDSP6_DEB: pc = 0x%lx\n", pc);
  }
  
  return info;
}

static void
qdsp6_extract_return_value (struct type *type, struct regcache *regcache,
                          void *valbuf)
{
  int len = TYPE_LENGTH (type);
  //printf("\n ******* Calling extract return value \n");

  if (len <= 4)
    {
      ULONGEST gpr0_val;
      regcache_cooked_read_unsigned (regcache, 0, &gpr0_val);
      store_unsigned_integer ((gdb_byte *)valbuf, len, BFD_ENDIAN_LITTLE, gpr0_val);
    }
  else if (len <= 8)
    {
      ULONGEST regval;
      regcache_cooked_read_unsigned (regcache, 0, &regval);
      store_unsigned_integer ((gdb_byte *)valbuf, 4, BFD_ENDIAN_LITTLE, regval);
      regcache_cooked_read_unsigned (regcache, 1, &regval);
      store_unsigned_integer ((bfd_byte *) valbuf + 4, 4, BFD_ENDIAN_LITTLE, regval);
    }
  else
    internal_error (__FILE__, __LINE__, "Illegal return value length: %d", len);
}

static CORE_ADDR
qdsp6_extract_struct_value_address (struct regcache *regcache)
{
  ULONGEST addr;
  //printf("\n In qdsp6_extract_struct_value_address ..... ");
  regcache_cooked_read_unsigned (regcache, struct_return_regnum, &addr);
  return addr;
}

static CORE_ADDR
qdsp6_frame_align (struct gdbarch *gdbarch, CORE_ADDR sp)
{
  /* Require dword alignment.  */
  return align_down (sp, 8);
}

static void
qdsp6_store_return_value (struct type *type, struct regcache *regcache,
                        const void *valbuf);

static enum return_value_convention
qdsp6_return_value (struct gdbarch *gdbarch, struct type *func_type,
		   struct type *valtype, struct regcache *regcache,
		   gdb_byte *readbuf, const gdb_byte *writebuf)
{
  //printf("\nCalling qdsp6_return_value .........................");

  /* Structs, unions, and anything larger than 8 bytes (2 registers)
     goes on the stack.  */
  int struct_return = (((TYPE_CODE (valtype) == TYPE_CODE_STRUCT
       			 || TYPE_CODE (valtype) == TYPE_CODE_UNION)
			 && TYPE_LENGTH (valtype) > 8) 
			 || (TYPE_LENGTH (valtype) > 8));

   if (readbuf)
     {
       gdb_assert (!struct_return);
       qdsp6_extract_return_value (valtype, regcache, readbuf);
     }
   if (writebuf)
     {
       gdb_assert (!struct_return);
       qdsp6_store_return_value (valtype, regcache, writebuf);
     }

   if (struct_return)
     return RETURN_VALUE_STRUCT_CONVENTION;
   else
     return RETURN_VALUE_REGISTER_CONVENTION;
}

static CORE_ADDR
qdsp6_push_dummy_code (struct gdbarch *gdbarch,
                      CORE_ADDR sp, CORE_ADDR funaddr,
                      struct value **args, int nargs,
                      struct type *value_type,
                      CORE_ADDR *real_pc, CORE_ADDR *bp_addr,
                      struct regcache *regcache)
{
  sp = (sp - 4) & ~3;
  (*real_pc) = funaddr;
  regcache_cooked_read(regcache, REG_LR, (gdb_byte *)bp_addr);
       
  return sp;
}

static CORE_ADDR
qdsp6_push_dummy_call (struct gdbarch *gdbarch, struct value *function,
                     struct regcache *regcache, CORE_ADDR bp_addr,
                     int nargs, struct value **args, CORE_ADDR sp,
		     int struct_return, CORE_ADDR struct_addr)
{
  int argreg;
  int argnum;
  int len;
  int stack_space = 0, stack_offset;
  int numOfFixedTypeFunctionArgs = 0;
  gdb_byte *val;
  unsigned char valbuf[32];
  char *sDemangledFunctionName    = NULL;
  CORE_ADDR regval;
  enum type_code typecode;
  struct value *arg;
  struct type *arg_type;
  struct type *ftype;
  struct symbol  *pFuncSymbol    =  NULL;
  struct minimal_symbol *msymbol =  NULL;
  CORE_ADDR func_addr = find_function_addr (function, NULL);

  
  
#if 0
  printf("\n In qdsp6_push_dummy_call ........................\n");
  printf("\nFunction Address = 0x%x\n", func_addr);
  printf("Push %d args at sp = 0x%x, struct_return=%d (%x)\n",
	 nargs, (int) sp, struct_return, struct_addr);
#endif

  
  /* get the function symbol for this function address */
   pFuncSymbol = find_pc_function (func_addr);

  if(pFuncSymbol)
  {
    /* Get the type of the current symbol (function) being passed in */
    ftype = check_typedef (SYMBOL_TYPE (pFuncSymbol));
    
    /* Get the number of fixed type args for the function symbol */
    numOfFixedTypeFunctionArgs = TYPE_NFIELDS(ftype);
  }
  else
  {
    /* Try the minimal symbols.*/
    msymbol = lookup_minimal_symbol_by_pc (func_addr);

    /*This check is for C++ tests alone. */
    if(msymbol)
      sDemangledFunctionName =  SYMBOL_DEMANGLED_NAME(msymbol);
    
    numOfFixedTypeFunctionArgs = -1;
  }
 
  for (argnum = 0; argnum < nargs; ++argnum)
    stack_space += align_up (TYPE_LENGTH (value_type (args[argnum])), 4);

  if (stack_space > 0)
    sp -= stack_space;

  /* Make sure stack is dword aligned. */
  sp = align_down (sp, 8);

  stack_offset = 0;

  argreg = 0;

  if (struct_return)
    regcache_cooked_write_unsigned (regcache, struct_return_regnum,
                                    struct_addr);

  for (argnum = 0; argnum < nargs; ++argnum)
    {
      arg = args[argnum];
      arg_type = check_typedef (value_type (arg));
      len = TYPE_LENGTH (arg_type);
      typecode = TYPE_CODE (arg_type);
 
      if ((typecode == TYPE_CODE_STRUCT || 
	   typecode == TYPE_CODE_UNION) &&
	  (len > 8))
	{
	  store_unsigned_integer (valbuf, 4, 
				  BFD_ENDIAN_LITTLE, value_address (arg));
	  typecode = TYPE_CODE_PTR;
	  val = valbuf;
	  write_memory (sp+stack_offset, value_contents (arg), len);
	  stack_offset += align_up (len, 4);
	  // everything written to memory, nothing to write in regs
	  len = 0;
	}
      else
	{
	  val = (unsigned char *)value_contents (arg);
	}


      while (len > 0)
	{
	  int partial_len = (len < 4 ? len : 4);

	  /* Pass function params into regs R0-R5 for only fixed
	     type of arguments */
	  if ((argreg < q6ArgRegMax) &&
              (argnum < numOfFixedTypeFunctionArgs))
	    {
	      regval = extract_unsigned_integer (val, partial_len,
						 BFD_ENDIAN_LITTLE);
#if 0
	      printf("  Argnum %d data 0x%x -> reg 0x%d\n",
		     argnum, (int) regval, argreg);
#endif

	      typecode = TYPE_CODE (arg_type);
 
	      /*Candidates larger than 32 bits are passed in even-odd reg pair.*/
	      if((len > 4) && ((argreg%2) != 0))
		  ++argreg;

	      regcache_cooked_write_unsigned (regcache, argreg, regval);
	      ++argreg;
	    }
	  else if((argreg < q6ArgRegMax) && 
                  (sDemangledFunctionName != NULL))
	  {
	    /* This code is a temp. hack for C++ gdb test virtfunc.cc 
	       which does not return a valid symbol pointer for "thunk" 
	       function which calls the actual virtual function implementation */
	    regval = extract_unsigned_integer (val, partial_len,
					       BFD_ENDIAN_LITTLE);
	    typecode = TYPE_CODE (arg_type);
	    
	    /*Candidates larger than 32 bits are passed in even-odd reg pair.*/
	    if((len > 4) && ((argreg%2) != 0))
	      ++argreg;
	    
	    regcache_cooked_write_unsigned (regcache, argreg, regval);
	    ++argreg;

	  }
	  else
	    {
#if 0
	      printf("  Argnum %d data 0x%x -> offset %d (0x%x)\n",
		     argnum, *((int *)val), stack_offset, (int) (sp + stack_offset));
#endif
	      write_memory (sp + stack_offset, val, partial_len);
	      stack_offset += align_up (partial_len, 4);
	    }
	  len -= partial_len;
	  val += partial_len;
	}
    }

  /* Set the return address.  For qdsp6, the return breakpoint is
     always at BP_ADDR.  */
  regcache_cooked_write_unsigned (regcache, REG_LR, bp_addr);

  /* Finally, update the SP register.  */
  regcache_cooked_write_unsigned (regcache, REG_SP, sp);

  return sp;
}

static void
qdsp6_store_return_value (struct type *type, struct regcache *regcache,
                        const void *valbuf)
{
  printf("\n In qdsp6_store_return_value .....................\n");
  int len = TYPE_LENGTH (type);

  if (len <= 4)
    {
      bfd_byte val[4];
      memset (val, 0, sizeof (val));
      memcpy (val, valbuf, len);
      regcache_cooked_write (regcache, 0, val);
    }
  else if (len <= 8)
    {
      regcache_cooked_write (regcache, 0, valbuf);
      regcache_cooked_write (regcache, 1, (bfd_byte *) valbuf + 4);
    }
  else
    internal_error (__FILE__, __LINE__,
                    "Don't know how to return a %d-byte value.", len);
}

static CORE_ADDR
qdsp6_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  CORE_ADDR pc;
  if (qdsp6_debug == 1)
  {
      printf_filtered ("QDSP6_DEB: %s\n",__func__);
  }

  pc = frame_unwind_register_unsigned (next_frame, REG_PC);
  if (qdsp6_debug == 1)
  {
      printf_filtered ("\tQDSP6_DEB: pc = 0x%x\n",pc);
  }
  return pc;
}

/* Given a GDB frame, determine the address of the calling function's
   frame.  This will be used to create a new GDB frame struct.  */

static void
qdsp6_frame_this_id (struct frame_info *this_frame,
		    void **this_prologue_cache, struct frame_id *this_id)
{
  struct qdsp6_unwind_cache *info
    = qdsp6_frame_unwind_cache (this_frame, this_prologue_cache);
  CORE_ADDR base;
  CORE_ADDR func;
  struct frame_id id;

  /* The FUNC is easy.  */
  func = get_frame_func (this_frame);

  /* Hopefully the prologue analysis either correctly determined the
     frame's base (which is the SP from the previous frame), or set
     that base to "NULL".  */
  base = info->prev_sp;
  if (base == 0)
    return;

  id = frame_id_build (base, func);
  (*this_id) = id;

}

static struct value *
qdsp6_frame_prev_register (struct frame_info *this_frame,
			  void **this_prologue_cache,
			  int regnum)
{
  struct qdsp6_unwind_cache *info
    = qdsp6_frame_unwind_cache (this_frame, this_prologue_cache);

  return trad_frame_get_prev_register (this_frame, info->saved_regs, regnum);

}

static const struct frame_unwind qdsp6_frame_unwind = {
  NORMAL_FRAME,
  qdsp6_frame_this_id,
  qdsp6_frame_prev_register,
  NULL,
  default_frame_sniffer
};


static CORE_ADDR
qdsp6_frame_base_address (struct frame_info *next_frame, void **this_cache)
{
  struct qdsp6_unwind_cache *info
    = qdsp6_frame_unwind_cache (next_frame, this_cache);
  return info->base;
}

static const struct frame_base qdsp6_frame_base = {
  &qdsp6_frame_unwind,
  qdsp6_frame_base_address,
  qdsp6_frame_base_address,
  qdsp6_frame_base_address
};

static CORE_ADDR
qdsp6_unwind_sp (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  return frame_unwind_register_unsigned (next_frame, REG_SP);
}

/* Assuming NEXT_FRAME->prev is a dummy, return the frame ID of that
   dummy frame.  The frame ID's base needs to match the TOS value
   saved by save_dummy_frame_tos(), and the PC match the dummy frame's
   breakpoint.  */

static struct frame_id
qdsp6_dummy_id (struct gdbarch *gdbarch, struct frame_info *this_frame)
{
  CORE_ADDR pc = get_frame_register_unsigned (this_frame, REG_PC);
  CORE_ADDR sp = get_frame_register_unsigned (this_frame, REG_SP);
  return frame_id_build (sp, pc);
#if 0
  return frame_id_build (qdsp6_unwind_sp (gdbarch, this_frame),
			 get_frame_pc (this_frame));
#endif
}


static struct gdbarch *
qdsp6_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *var;
  extern struct qdsp6_system_register_offsets reg_offset_v4;
  extern struct qdsp6_system_register_offsets reg_offset;

  /* Check to see if we've already built an appropriate architecture
     object for this executable.  */
  bfd_get_flavour(info.abfd);
  arches = gdbarch_list_lookup_by_info (arches, &info);
  if (arches)
    return arches->gdbarch;
 
 /* V2/V3/V4 registers set by new_variant
    Note: ABI says registers r0-r5 can be used as arguments to
          a function */
  switch (info.bfd_arch_info->mach)
    {
    case bfd_mach_qdsp6_v2:
      q6Version = Q6_V2;
      q6ArgRegMax = 6;
      threadRegSetInfo = &threadRegSetInfo_v2[0];
      globalRegSetInfo = &globalRegSetInfo_v2[0];
      q6RegOffset = &reg_offset;      
    break;
    
    case bfd_mach_qdsp6_v3:
      q6Version = Q6_V3;
      q6ArgRegMax = 6;
      threadRegSetInfo = &threadRegSetInfo_v3[0];
      globalRegSetInfo = &globalRegSetInfo_v3[0];
      q6RegOffset = &reg_offset;      
    break;

    case bfd_mach_qdsp6_v4:
      q6Version = Q6_V4;
      q6ArgRegMax = 6;
      threadRegSetInfo = &threadRegSetInfo_v4[0];
      globalRegSetInfo = &globalRegSetInfo_v4[0];
      q6RegOffset = &reg_offset_v4;      
    break;
    
    default:
      /* Never heard of this variant.  */
      return 0;
    }


  qdsp6_reg_index_max = qdsp6_index_max (threadRegSetInfo);
  
  /* Select the right tdep structure for this variant (qdsp6).  */
  var = new_variant ();
  switch (info.bfd_arch_info->mach)
    {
    case bfd_mach_qdsp6:
    case bfd_mach_qdsp6_v2:
    case bfd_mach_qdsp6_v3:
    case bfd_mach_qdsp6_v4:
      set_variant_num_gprs (var, 32);
      set_variant_num_fprs (var, 0);
      break;

    default:
      /* Never heard of this variant.  */
      return 0;
    }
  
  gdbarch = gdbarch_alloc (&info, var);


  /* Data types */
  set_gdbarch_short_bit (gdbarch, 16);
  set_gdbarch_int_bit (gdbarch, 32);
  set_gdbarch_long_bit (gdbarch, 32);
  set_gdbarch_long_long_bit (gdbarch, 64);
  set_gdbarch_float_bit (gdbarch, 32);
  set_gdbarch_double_bit (gdbarch, 64);
  set_gdbarch_long_double_bit (gdbarch, 64);
  set_gdbarch_ptr_bit (gdbarch, 32);


  set_gdbarch_num_regs (gdbarch, qdsp6_reg_index_max+1);
  set_gdbarch_num_pseudo_regs (gdbarch, qdsp6_num_pseudo_regs);

  set_gdbarch_sp_regnum (gdbarch, REG_SP);
  set_gdbarch_deprecated_fp_regnum (gdbarch,  REG_FP);
  set_gdbarch_pc_regnum (gdbarch, REG_PC);

  set_gdbarch_register_name (gdbarch, qdsp6_register_name);
  set_gdbarch_register_type (gdbarch, qdsp6_register_type);
#if 0
  set_gdbarch_register_sim_regno (gdbarch, qdsp6_register_sim_regno);

  set_gdbarch_pseudo_register_read (gdbarch, qdsp6_pseudo_register_read);
  set_gdbarch_pseudo_register_write (gdbarch, qdsp6_pseudo_register_write);
#endif

  set_gdbarch_skip_prologue (gdbarch, qdsp6_skip_prologue);
  set_gdbarch_breakpoint_from_pc (gdbarch, qdsp6_breakpoint_from_pc);

  set_gdbarch_adjust_breakpoint_address (gdbarch, qdsp6_gdbarch_adjust_breakpoint_address);

  set_gdbarch_push_dummy_code (gdbarch, qdsp6_push_dummy_code);

  set_gdbarch_return_value (gdbarch, qdsp6_return_value);

  /* Frame stuff.  */
  set_gdbarch_unwind_pc (gdbarch, qdsp6_unwind_pc);
  set_gdbarch_unwind_sp (gdbarch, qdsp6_unwind_sp);
  set_gdbarch_frame_align (gdbarch, qdsp6_frame_align);

  frame_unwind_append_unwinder (gdbarch, &qdsp6_frame_unwind);

  frame_base_set_default (gdbarch, &qdsp6_frame_base);

  /* Settings for calling functions in the inferior.  */
  set_gdbarch_push_dummy_call (gdbarch, qdsp6_push_dummy_call);
  set_gdbarch_dummy_id (gdbarch, qdsp6_dummy_id);

  /* Settings that should be unnecessary.  */
  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);

  /* Hardware watchpoint / breakpoint support.  */
  var->num_hw_watchpoints = 0;
  var->num_hw_breakpoints = 0;

  
  /* set the disassembler */
  set_gdbarch_print_insn (gdbarch, qdsp6_get_disassembler_from_mach
		                  (info.bfd_arch_info->mach,
                                   (info.byte_order == BFD_ENDIAN_BIG)));

  /* Bug Fix for bug1316
     Changed CALL_DUMMY_LOCATION from default AT_ENTRY to ON_STACK
   */  
  //set_gdbarch_call_dummy_location (gdbarch, ON_STACK);

  return gdbarch;
}

/* Map global register name to its register number */
static int 
qdsp6_global_regnum_from_name(char *regname, int *index)
{
  int i, len= strlen(regname)  ;
  int regnum = -1 ;
  int numGblRegs;
  if(regname == NULL)
    return -1;

  switch (q6Version)
  {
    case Q6_V4:
	numGblRegs = sizeof(globalRegSetInfo_v4)/sizeof(regtype_t);
	break;
    
    case Q6_V3:
	numGblRegs = sizeof(globalRegSetInfo_v3)/sizeof(regtype_t);
	break;

    case Q6_V2:
	numGblRegs = sizeof(globalRegSetInfo_v2)/sizeof(regtype_t);
	break;

    default:
	printf_filtered ("%s, invalid setting for arch level %d\n",__func__, q6Version);
	gdb_assert(0);
	break;
  }

  for (i = 0; i < numGblRegs; i++)
  {

    if(globalRegSetInfo[i].reg_name == NULL)
        break;

    if ((regname != NULL)                    && 
	(len == strlen (globalRegSetInfo[i].reg_name)) && 
	(strncmp (globalRegSetInfo[i].reg_name,regname,len) == 0))
    {
        regnum =  globalRegSetInfo[i].index ;
        break;
    }
  }

  *index = i ;
  return regnum;
}

/* Print reg info in the following format
   register name  0xff 255 */ 
static void  
qdsp6_print_reg_info(char  *regname,  LONGEST regvalue)
{

  if(regname == NULL)
    error("Invalid Register Name\n");

  if(strlen(regname) <=7)
     fprintf_filtered(gdb_stdout,"%s\t\t", regname);
  else
     fprintf_filtered(gdb_stdout,"%s\t", regname);
  
  fprintf_filtered(gdb_stdout, "0x%x", (unsigned int) regvalue);
  fprintf_filtered(gdb_stdout, "\t");  
  
  // print the value of the register in decimal
  fprintf_filtered(gdb_stdout, "%u", (unsigned int) regvalue);

  fprintf_filtered(gdb_stdout, "\n");
  
  gdb_flush (gdb_stdout);
}



/* maximum message size from RIL */
/* 1.0 MB = 512 * 2048 */
#define GDB_RSP_RIL_INFO_MAX_SIZE 256 * 2048
#define SET_CMD_BUFFER_SIZE 1024
static long gdb_rsp_ril_info_max_size=GDB_RSP_RIL_INFO_MAX_SIZE;

/* store for holding the response; this is raw-ascii coded */
static char response_buf[GDB_RSP_RIL_INFO_MAX_SIZE];
static char *response = response_buf;

/* storage to hold the display buffer */
static char display_buf[GDB_RSP_RIL_INFO_MAX_SIZE/2 +1];

/* print out if no timeout occured and got non null string */
#define PRINT_MSG \
   if ((response[0] != 0) && ((strstr(response, "timeout") == NULL)))\
    {\
      n = min (strlen (response) / 2, sizeof (display_buf));\
      result = hex2bin (response, display_buf, n);\
      display_buf [result] = '\0';\
      printf_filtered("%s\n", display_buf);\
    }


char* gettargetresponse ()
{
 int n;
 int result;
  
 if ((response[0] != 0) && ((strstr(response, "timeout") == NULL)))
    {
      n = min (strlen (response) / 2, sizeof (display_buf));
      result = hex2bin (response, display_buf, n);
      display_buf [result] = '\0';
    }
    return display_buf;
}

static void
qdsp6_rtos_info_command (char *args, int from_tty)
{
  int result;
  int n = 0;                    /* position in display_buf */
  
  if(current_q6_target == NULL)
    error ("Program has no rtos info. Load program first");
    
  putpkt ("qq6RtosInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  PRINT_MSG;
}

static ULONGEST qdsp6_get_pagetable_size ()
{
  char *pResponse;
  long long unsigned pSize = 0;

  putpkt ("qPageTableSize");
  getpkt ( &response, &gdb_rsp_ril_info_max_size , 0);
 
  if(response == NULL)
    error ("Invalid Page Table size string returned from DRIL.");

  pResponse = gettargetresponse();

  if(pResponse == NULL)
    error ("Invalid Page Table size string returned from DRIL.");

  if(strcmp(pResponse,"PageTable command supported only for rtos apps") == 0)
    error ("PageTable command supported only for rtos apps");

  /* Check for valid page size */
  sscanf(pResponse,"%llu",&pSize);

  if (pSize <= 0)
     error ("Invalid Page Table size returned from DRIL.");

  return pSize;

}
static void
qdsp6_pagetable_info_command (char *args, int from_tty)
{
  int result;
  int n              = 0;  /* position in display_buf */
  ULONGEST pSize     = 0;
  char *pageBuf      = NULL;
  char * displayBuf  = NULL;

  if(current_q6_target == NULL)
    error ("Program has no pagetable info. Load program first");

  /* Get the size of page table data */  
  pSize = qdsp6_get_pagetable_size() ;

  /* Allocate memory chunk for page table 
   * We allocate double the size as the msg is hex encoded*/
  pageBuf    = (char*) malloc (pSize*2+1);
   /* Allocate memory chunk for page table display */ 
  displayBuf = (char*) malloc (pSize+1);

  if ((pageBuf == NULL) || (displayBuf == NULL))
    error ("Page Table memory allocation failed");
  
  putpkt ("qPageTableInfo");
  getpkt (&pageBuf, &gdb_rsp_ril_info_max_size , 0);

  /* Display page table info */
  if ((pageBuf[0] != 0) && ((strstr(pageBuf, "timeout") == NULL)))
  {
    n = min(strlen(pageBuf)/2, pSize+1);
    result = hex2bin (pageBuf, displayBuf, n);
    displayBuf [result] = '\0';
    printf_filtered("%s\n", displayBuf);
  }
 
  /* Free resources allocated locally */
  if (pageBuf != NULL)
  {  
    free (pageBuf);
    pageBuf = NULL;
  }

  if (displayBuf != NULL)
  {  
    free (displayBuf);
    displayBuf = NULL;
  }

}


static void
qdsp6_tlb_info_command (char *args, int from_tty)
{
  int result;
  int n = 0;                    /* position in display_buf */

  if(current_q6_target == NULL)
    error ("Program has no tlb info. Load program first");
    
  putpkt ("qTLBInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size, 0);
  PRINT_MSG;
}



static LONGEST
get_globalregs_buffer (char* args, int printInfo)
{
  int  result, namelen, numValidGlobalRegs=0;
  int  n = 0,  i =0 ;                  
  int  lenDelimiter=2 , lenValue=8 , lenData, lenResponse;
    
  char sDisplayBuf[SET_CMD_BUFFER_SIZE];
  char * sDelimiter; 
  char * sRegData = response;
  char sRegNames[NUM_GLOBAL_REGS][MAXREGNAMELEN]; 
  LONGEST  regValues[NUM_GLOBAL_REGS];

  putpkt ("qGlobalRegsInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  
  lenResponse  = strlen(response); 

 /*RSP for each register = register name:value */
  /* value for each reigster is represented as 8 nibbles */
  for(n =0, lenData =0; 
     (n<NUM_GLOBAL_REGS) && (lenData< lenResponse); 
     n++ )
  {
    if ((response[0] != 0) && 
	((strstr(response, "timeout") == NULL)))
    {
      /*read in the register name till : is seen */
      for(i=0, namelen = 0;;i+=2, namelen+=2)
         if((sRegData[i] == '3') && (sRegData[i+1] == 'a'))
          break;
      
      result  = hex2bin(sRegData, sRegNames[n], namelen/2);
      sRegNames[n][result] = '\0';

      /* read reg value after strlen(reg name) + strlen("3a"). 
         0x3a = ':' the delimiter between register name and its value*/
      result = hex2bin (sRegData+namelen+lenDelimiter,
                        sDisplayBuf, lenValue/2);
      sDisplayBuf [result] = '\0';
      

      /* get the register offset to store the register value */
      i  =  qdsp6_global_regnum_from_name(sRegNames[n], &result);

      if(i == -1)
      {
        qdsp6_print_reg_info("Invalid Register Name", 0);
        return -1;
      }

      /* Get the value of the register */
      regValues[i] = extract_signed_integer((const gdb_byte *)&sDisplayBuf,
					    lenValue/2, BFD_ENDIAN_LITTLE);
    
      /* get the length of the string read so far */  
      lenData += namelen+lenDelimiter+lenValue;

      /* advance the data to read in the next register set info */
      /* advance pointer by reg name length + : + 8 bytes of reg value */ 
      sRegData+=namelen;
      sRegData+=lenDelimiter;
      sRegData+=lenValue;

      /* increase the valid reg count */
      numValidGlobalRegs++;
    }
  }
  /* get particular register value , get the index into the array*/
  /* passing 0 as 2nd arg will return register offset in arch defn */
  if((printInfo != 1) && (args != NULL))
  {
     n =  qdsp6_global_regnum_from_name(args, &i);
     return regValues[n];
  }
  else if(printInfo == 1)
  {

    if(args != NULL)
    {
       /* print particular register , get the index into the array*/
       /* passing 0 as 2nd arg will return register offset in arch defn */
       n =  qdsp6_global_regnum_from_name(args, &i);
       if(n != -1)
         qdsp6_print_reg_info(globalRegSetInfo[i].reg_name, regValues[n] );
       else
         qdsp6_print_reg_info("Invalid Register Name", 0);

    }
   else
   {
   
    /* Print all the registers */
    for(result =0; result<numValidGlobalRegs ; result++)
    {
      /* get the register offset to store the register value */
      n  =  qdsp6_global_regnum_from_name(sRegNames[result], &i);
      qdsp6_print_reg_info(globalRegSetInfo[i].reg_name, regValues[n]);
    }
   }
  }

  return 0;  
}


static void
qdsp6_globalregs_info_command (char *args, int from_tty)
{
  int  result, namelen, numValidGlobalRegs=0;
  int  n = 0,  i =0 ;                  
  int  lenDelimiter=2 , lenValue=8 , lenData, lenResponse;
  
  char sDisplayBuf[SET_CMD_BUFFER_SIZE];
  char * sDelimiter; 
  char * sRegData = response;
  char sRegNames[NUM_GLOBAL_REGS][MAXREGNAMELEN]; 
  LONGEST  regValues[NUM_GLOBAL_REGS];

  if(current_q6_target == NULL)
    error ("Program has no global registers. Load program first");

  putpkt ("qGlobalRegsInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  if(strncmp(response,"E00", strlen("E00"))==0)
     error ("Error reading register values from simulator!"); 
     
  lenResponse  = strlen(response);
  
  /*RSP for each register = register name:value */
  /* value for each reigster is represented as 8 nibbles */
  for(n =0, lenData =0; 
     (n<NUM_GLOBAL_REGS) && (lenData< lenResponse); 
      n++)
  {
    if ((response[0] != 0) && 
	((strstr(response, "timeout") == NULL)))
    {
      /*read in the register name till : is seen */
      for(i=0, namelen = 0;;i+=2, namelen+=2)
         if((sRegData[i] == '3') && (sRegData[i+1] == 'a'))
          break;
      
      result  = hex2bin(sRegData, sRegNames[n], namelen/2);
      sRegNames[n][result] = '\0';

      /* read reg value after strlen(reg name) + strlen("3a"). 
         0x3a = ':' the delimiter between register name and its value*/
      result = hex2bin (sRegData+namelen+lenDelimiter,
                        sDisplayBuf, lenValue/2);
      sDisplayBuf [result] = '\0';
      

      /* get the register offset to store the register value */
      i  =  qdsp6_global_regnum_from_name(sRegNames[n], &result);

      if(i == -1)
      {
        qdsp6_print_reg_info("Invalid Register Name", 0);
        return;
      }

      /* Get the value of the register */
      regValues[i] = extract_signed_integer((const gdb_byte *)&sDisplayBuf,
					    lenValue/2, BFD_ENDIAN_LITTLE);
    
      /* get the length of the string read so far */  
      lenData += namelen+lenDelimiter+lenValue;

      /* advance the data to read in the next register set info */
      /* advance pointer by reg name length + : + 8 bytes of reg value */ 
      sRegData+=namelen;
      sRegData+=lenDelimiter;
      sRegData+=lenValue;

      /* increase the valid reg count */
      numValidGlobalRegs++;
    }
  }
  
  /* Print register Info */

  if(args != NULL)
  {
    /* print particular register , get the index into the array*/
    /* passing 0 as 2nd arg will return register offset in arch defn */
    n =  qdsp6_global_regnum_from_name(args, &i);
    if(n != -1)
      qdsp6_print_reg_info(globalRegSetInfo[i].reg_name, regValues[n] );
    else
      qdsp6_print_reg_info("Invalid Register Name", 0);
  }
  else
  {
    /* Print all the registers */
    for(result =0; result<numValidGlobalRegs ; result++)
    {
      /* get the register offset to store the register value */
      n  =  qdsp6_global_regnum_from_name(sRegNames[result], &i);
      qdsp6_print_reg_info(globalRegSetInfo[i].reg_name, regValues[n]);
    }
  }
  
}

/* Get the value of a particular register */
int get_q6globalregs_value (char *args)
{

  LONGEST regVal;

  if(current_q6_target == NULL)
    error ("Program has no global registers. Load program first.");

  if(args == NULL)
   error ("Empty register name !.\n");
 
  return get_globalregs_buffer (args,0);
}

static void
qdsp6_thread_details_info_command (char *args, int from_tty)
{
  int result;
  int num;
  ptid_t ptid;
  int n = 0;                    /* position in display_buf */
  char message[30];

  if (!args)
    {
      error ("Thread ID not specified");
      return;
    }

  sscanf(args, "%d", &num);

  /* convert into tid */
  ptid = thread_id_to_pid (num);

  if (ptid_equal(ptid, pid_to_ptid(-1)))
    {
      error ("Thread ID %d not known.", num);
      return;
    }


  sprintf(message, "qThreadDetailInfo,%d", PIDGET(ptid));
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  PRINT_MSG;
}


void 
setglobalregistervalue(char *args, int from_tty, 
                       struct cmd_list_element *cmdlist)
{
  char *regName, *regValue, *substr,*q6regbuf = NULL;
  char tmpbuf[9], message[256];
  int regNum, len=0, result;
  int newValue;   //LONGEST newValue;
  struct expression *expr;
 
  if(current_q6_target == NULL)
    error("Program has no global registers. Load program first");
  else if(q6globalregsInfo == NULL)
    error("Invalid register name/value.");
					
  /*Check for blanks in the user entered string */ 
   substr = strtok((char*)q6globalregsInfo, " ");
   if(substr != NULL)
   {		
     q6regbuf = (char*)malloc(sizeof(char) * SET_CMD_BUFFER_SIZE);

     if(q6regbuf == NULL)
        error("Memory allocation for register buffer failed !");

     /*clear the temporary buffer */
     memset(q6regbuf,0,SET_CMD_BUFFER_SIZE);

     /*extract user string with spaces removed */
     strcat(q6regbuf, substr);
     while ( (substr=strtok(NULL, " ")) != NULL)
       strcat(q6regbuf, substr);
   }
   else
     q6regbuf=q6globalregsInfo; 

  /* Extract the register name whose value is to be set  */
  regName = strtok(q6regbuf, "=");
  if(regName == NULL)  
    error("Invalid register name/value.");
  
  /* Get the global register index for this register */
  regNum =  qdsp6_global_regnum_from_name(regName, &result);
  if(regNum == -1)  
    error("Invalid register name : %s.",regName );
  
  /* get the register value to be set  */
  regValue =q6regbuf+strlen(regName)+1;

  /* parse the new register value and get the decimal value
     expr->elts[2].longconst stores the converted values */
  expr =  parse_expression (regValue);

  if(!expr)
    return;
  
  /* get the decimal value to be set */
  newValue = expr->elts[2].longconst;

  /* get the reg num and reg value to be set into string format */
  sprintf(message, "qSetGlobalRegsInfo,%x,", regNum);

  sprintf(tmpbuf,"%02x", newValue & 0x0ff);
  strcat(message,tmpbuf);
  sprintf(tmpbuf,"%02x",(newValue>>8) & 0x0ff);
  strcat(message,tmpbuf);
  sprintf(tmpbuf,"%02x",(newValue>>16) & 0x0ff);
  strcat(message,tmpbuf);
  sprintf(tmpbuf,"%02x",(newValue>>24) & 0x0ff);
  strcat(message,tmpbuf);

  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  if(strcmp(response,"E02") == 0)
    error ("'%s' is a read-only register.",regName);

  gdb_flush (gdb_stdout);

  /* free resources allocated to q6globalregsinfo */
  if(q6regbuf != NULL)
    free (q6regbuf); 

}  

void init_globalregs()
{
   /* Chain containing all defined show subcommands.  */
   struct cmd_list_element *q6showlist = NULL;

   /* allocate memory for control variable for setshow command */
   q6globalregsInfo  = (char*)malloc(sizeof(char) * SET_CMD_BUFFER_SIZE);
  
 
    if(q6globalregsInfo ==  NULL)
        error ("Memory allocation for register buffer failed!");
     /* set global regs */
      add_setshow_string_noescape_cmd ("globalregs",         /* name */
		   no_class,             /* class category for help list */
		   &q6globalregsInfo,    /* address of the variable being controlled by this */
           "set Q6 global register to the specified value", /* documentation for this cmd */
           "set Q6 global register to the specified value", /* documentation for this cmd */
           "set Q6 global register to the specified value", /* documentation for this cmd */
		   &setglobalregistervalue,  /* set cmd func ptr */
		   NULL,                     /* show cmd fn  ptr */
		   &setlist,
		   &q6showlist);


}

/*
 * function: setQ6targetargs
 * description:
 * 	- takes a single input string and breaks it up into component
 * 	  parts suitable execvp
 */
void  
setQ6targetargs (char * args, int tty, struct cmd_list_element *c)
{
    ULONGEST addr = *(unsigned long *)c->var;
    char *simargs = (char *) addr;
    char **targs = q6targetargsInfo;
    int index = 0; 

    simargs = strtok (simargs, " \t\n");
    while (simargs)
    {
	targs[index++] = simargs;
	simargs = strtok(0, " \t\n");

	gdb_assert (index < SIM_ARG_MAX);
    }
}

static void
qdsp6_tclfe_set (char *args, int from_tty,
		 struct cmd_list_element *c)
{
/* Note: the var_type is var_boolean so this will only be 1 or 0 */
   Q6_tcl_fe_state = *(int *)(c->var);
}

static void
qdsp6_tclfe_show (char *args, int from_tty,
		 struct cmd_list_element *c)
{
   return;
}

void init_targetargs()
{
   /* Chain containing all defined show subcommands.  */
   struct cmd_list_element *targetargsshowlist = NULL;

   /* set global regs */
   add_setshow_string_noescape_cmd ("targetargs",         /* name */
		            no_class,      /* class category for help list */
		            &q6targetargsInfo[0],    /* control var address */
                    "set Q6 target args", /* doc for this cmd */
                    "set Q6 target args", /* doc for this cmd */
                    "set Q6 target args", /* doc for this cmd */
		            &setQ6targetargs, /* set cmd func ptr */
		            NULL,                 /* show cmd fn  ptr */
		            &setlist,
		            &targetargsshowlist);

   add_setshow_boolean_cmd ("tclfe",             /* name */
		    no_class,            /* class category for help list */
		    &Q6_tcl_fe_state,	 /* control var address */
                    "Turn on or off Q6 GDB's TCL interpreter", 
                    "Turn on or off Q6 GDB's TCL interpreter", 
                    "Turn on or off Q6 GDB's TCL interpreter", 
		    qdsp6_tclfe_set,	 /* Add function to set */
		    NULL,	 /* Add function to print show */
		    &setlist,
		    &showlist);

}


void
set_Q6_hwthread_debug(char *args, int from_tty, struct cmd_list_element *cmdlist)
{

  char message[SET_CMD_BUFFER_SIZE] = {'\0'};
  
  if (current_q6_target == NULL)
    error ("Cannot set interrupts. Load program first.");

  if (q6hwthread_debug)
    {
      sprintf(message, "q6hwThreadDebug,%d", 1);
    }
  else
    {
      sprintf(message, "q6hwThreadDebug,%d", 0);
      
    }
    
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  
  gdb_flush (gdb_stdout);
}


void 
set_Q6_interrupt(char *args, int from_tty, struct cmd_list_element *cmdlist)
{
  char message[SET_CMD_BUFFER_SIZE] = {'0'};
  char buf[256] = {'0'};
  char *substr;
  int interruptNum; //LONGEST interruptNum;
  struct expression* expr;
 
  if (current_q6_target == NULL)
    error ("Cannot set interrupts. Load program first.");
 
  if (q6Interrupt == NULL)
     error ("Usage: set interrupt <interrupt no> <delay> <period>.");
  
  /*Extract watch point type*/ 
  substr = strtok(q6Interrupt, " ");
  if(substr != NULL)
  {
    /* parse user input and get decimal values, This
       function does error checking on the value entered */
    expr =  parse_expression (substr);

    if(!expr) return;
  
    /* get the decimal value to be set */
    interruptNum = expr->elts[2].longconst;

    /* Check if interrupt num specified lies in the valid range */ 
    if((interruptNum <0)) /* || (interruptNum > MAX_INTERRUPT)) */
       error( "Invalid Interrupt Number %d, set interrupts 0 - 32.",
        interruptNum);

    /* get the reg num and reg value to be set into string format */
    sprintf(message, "qGenInterrupt,%x", interruptNum);

    /* get the period and delay if any */
    while ((substr=strtok(NULL, " ")) != NULL)
    {

      /* parse user input and get decimal values, This
      function does error checking on the value entered */
      expr =  parse_expression (substr);

      if(!expr) return;
  
      /* get the decimal value to be set */
      interruptNum = expr->elts[2].longconst;
    
      /* get the delay/period to be set into string format */
      sprintf(buf, ",%x", interruptNum);
      strcat(message,buf);

     }
   }
   else
    error ("Usage: set interrupt <interrupt no> <delay> <period>.");

  
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  gdb_flush (gdb_stdout);

}


void init_set_hwthread_debug_only_mode()
{
  /* Chain containing all defined show subcommands.  */
   struct cmd_list_element *q6showlist = NULL;


   /* set hwthrdbg */
   add_setshow_zinteger_cmd ("hwthrdbg",        /* name */
                    no_class,          /* class category for help list */
                    &q6hwthread_debug, /*address of the variable being controlled by this */
                    "switch to hw thread debug more", /* documentation for this cmd */
                    "switch to hw thread debug more", /* documentation for this cmd */
                    "switch to hw thread debug more", /* documentation for this cmd */
                    &set_Q6_hwthread_debug,          /* set cmd func ptr */
                    NULL,                     /* show cmd fn  ptr */
                    &setlist,
                    &q6showlist);
   
   
}


void init_set_interrupt()
{
   /* Chain containing all defined show subcommands.  */
   struct cmd_list_element *q6showlist = NULL;
   struct cmd_list_element *c  ;

   /* for set interrupt command - contains the user typed string*/
   q6Interrupt = (char*)malloc(sizeof(char) * SET_CMD_BUFFER_SIZE);
 
   if(q6Interrupt == NULL)
        error ("Memory allocation for interrupt buffer failed !");

   /* set interrupt */
   add_setshow_string_noescape_cmd ("interrupt",               /* name */
		             no_class,                 /* class category for help list */
		             &q6Interrupt,             /*address of the variable being controlled by this */
       	             "generate interrupt in Q6 core", /* documentation for this cmd */
       	             "generate interrupt in Q6 core", /* documentation for this cmd */
       	             "generate interrupt in Q6 core", /* documentation for this cmd */
		             &set_Q6_interrupt,          /* set cmd func ptr */
		             NULL,                     /* show cmd fn  ptr */
		             &setlist,
		             &q6showlist);





/*c = (add_set_cmd ("interrupt", no_class, var_string,
		   (char *) &q6Interrupt,
		   "Set interrupt in Q6 core", 
           &sethistlist));
  set_cmd_sfunc (c, set_Q6_interrupt);      
  add_show_from_set (c, &showlist);
  set_cmd_completer (c, command_completer);
  */ 
}

void
qdsp6_proc_cycles_info_command(char *args, int from_tty)
{
  unsigned long long cycle_count = 0;
  int i;
  if(current_q6_target == NULL)
    error ("Program has no proc cycle info. Load program first");
    
  putpkt ("qProcCyclesInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  if (strlen(response) == 16) {
    // got 16 nibbles get the cycle count and print
    for (i=0; i<8; i++) {
      cycle_count |= (((fromhex(response[i*2]) << 4) |
		       (fromhex(response[i*2 + 1]))) << (i*8));  
    }
    printf_filtered("%lld\n", cycle_count);
  }
  else {
    error ("Processor cycles not available");
  }
}


void
qdsp6_thread_cycles_info_command(char *args, int from_tty)
{
  unsigned long long cycle_count = 0;
  int i, num;
  ptid_t ptid;
  char message[30];
  
  if(current_q6_target == NULL)
    error ("Program has no thread cycles info. Load program first");
    
  if (!args) {
    error ("Thread ID not specified");
    return;
  }

  sscanf(args, "%d", &num);

  /* convert into tid */
  ptid = thread_id_to_pid (num);
  
  if (ptid_equal(ptid, pid_to_ptid(-1))) {
    error ("Thread ID %d not known.", num);
    return;
  }

  /* send the message out */
  sprintf(message, "qThreadCyclesInfo,%d", PIDGET(ptid));
  putpkt (message);

  /* get the response */
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  if (strlen(response) == 16) {
    // got 16 nibbles get the cycle count and print
    for (i=0; i<8; i++) {
      cycle_count |= (((fromhex(response[i*2]) << 4) |
		       (fromhex(response[i*2 + 1]))) << (i*8));  
    }
    printf_filtered("0x%llx\n", cycle_count);
  }
  else {
    error ("Thread cycles not available");
  }
}


void
qdsp6_msg_channels_info_command (char *args, int from_tty)
{
 int result;
 int n = 0;                    /* position in display_buf */
 
 if(current_q6_target == NULL)
   error ("Program has no message channels info. Load program first");
    
 putpkt ("q6MsgChannelInfo");
 getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
 if(strncmp(response,"E00", strlen("E00"))==0)
    warning ("No message channel information.");
 else   
    PRINT_MSG;

}

void
qdsp6_msg_queues_info_command (char *args, int from_tty)
{
 int result;
 int n = 0;                    /* position in display_buf */
 char msgStr[256]={0};
 char pktStr[256]={0};
 
 
 if(current_q6_target == NULL)
   error ("Program has no message queues info. Load program first");

  while(args && (*args != '\0') && ((*args == ' ') || (*args == '\t'))) args++;
  if (!args || (*args == '\0')) {
    error ("Message Channel name not specified.");
    return;
  }

  // ASG - bug 1436.  Need to pass whole string
  //sscanf(args, "%s", &msgStr[0]);
    /* send the message out */
  //sprintf(pktStr, "q6MsgQueueInfo,%s", msgStr);
  sprintf(pktStr, "q6MsgQueueInfo,%s", args);
  putpkt (pktStr);

  /* get the response */
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  
  if(strncmp(response,"E00", strlen("E00"))==0)
    warning ("Error reading message queue information. Check message queue name!");
  else   
    PRINT_MSG;

}

void
qdsp6_timers_info_command (char *args, int from_tty)
{
 int result;
 int n = 0;                    /* position in display_buf */
 
 if(current_q6_target == NULL)
   error ("Program has no timer info. Load program first");
    
 putpkt ("q6TimerInfo");
 getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
 if(strncmp(response,"E00", strlen("E00"))==0)
    warning ("No timer information.");
 else   
    PRINT_MSG;

}


void
qdsp6_mutex_info_command (char *args, int from_tty)
{
 int result;
 int n = 0;                    /* position in display_buf */
 int mutexID;
 char pktStr[256]={0};
 struct expression * expr;
 
 if(current_q6_target == NULL)
   error ("Program has no mutex info. Load program first");

  if (!args) {
    error ("Mutex ID not specified.");
    return;
  }

  /* parse the mutex ID and get the decimal value
     expr->elts[2].longconst stores the converted values */
  expr =  parse_expression (args);

  if(!expr)
   warning("Invalid Mutex ID specified !");;
  
  /* get the decimal value to be set */
  mutexID = expr->elts[2].longconst;
    
    /* send the message out */
  sprintf(pktStr, "q6MutexInfo,%x", mutexID);
  putpkt (pktStr);

  /* get the response */
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  
  if(strncmp(response,"E00", strlen("E00"))==0)
    warning ("Error reading mutex information. Check mutex ID!");
  else   
    PRINT_MSG;

}



void  
qdsp6_interrupt_map_info_command(char *args, int from_tty)
{
 
 int result;
 int n = 0;                    /* position in display_buf */
 
 if(current_q6_target == NULL)
   error ("Program has no interrupt map info. Load program first");
    
 putpkt ("qInterruptMap");
 getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
 PRINT_MSG;

}


void 
q6interrupt_remap(char *args, int from_tty)
{
  char message[256] = {'0'};
  char tmpbuf [25]  = {'0'};
  int interruptNum, tokValue = 0; 
  char* substr, *pTmp;
  struct expression* expr;

  if(current_q6_target == NULL)
    error ("Cannot remap interrupts. Load program first.");


  if (args == NULL)
  {
    error ("Interrupt ID not specified.");
    return;
  }

  substr = strtok(args, " ");
  if(substr != NULL)
  {
     sprintf(message, "qInterruptRemap,%s,", substr );

     while ((substr=strtok(NULL, " ")) != NULL)
     {

        tokValue +=1;

       /* parse user input and get decimal values, This
        function does error checking on the value entered */


      expr =  parse_expression (substr);

      if(!expr) return;
  
      /* get the decimal value to be set */
      interruptNum = expr->elts[2].longconst;
      
       /* Check if interrupt num specified lies in the valid range */ 
       if((interruptNum <0)) /* || (interruptNum > MAX_INTERRUPT)) */
         error ("Invalid Interrupt Number %d, set interrupts 0 - 32.",
                interruptNum);
       else if(tokValue >=2)
         error("Invalid Interrupt remap command\nUsage:  remap <name> <value>");
        
        /* Construct the RSP */
        sprintf(tmpbuf,"%x", interruptNum);
        strcat(message,tmpbuf);         

     }
  }
 
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

}
void 
q6interrupt_reset (char *args, int from_tty)
{
  char message[256] = {'0'};
  char tmpbuf [25]  = {'0'};
  char *pintType[3] = {"both", "periodic", "pin"};
  char *substr;
  int interruptNum, nargs = 0; 
  struct expression * expr;
  
  if(current_q6_target == NULL)
    error ("Cannot reset interrupts. Load program first.");

  if (args == NULL)
    error ("Interrupt ID not specified");

  substr = strtok(args, " ");
  if(substr != NULL)
  {
      nargs +=1;
      //interruptNum = strtol(substr,pTmp ,10);

      if(nargs == 1)
       { 
          expr =  parse_expression (substr);
          if(!expr)
              return;
          /* get the decimal value to be set */
          interruptNum = expr->elts[2].longconst;

          /* Check if interrupt num specified lies in the valid range */ 
          if((interruptNum <0)) /* || (interruptNum > MAX_INTERRUPT)) */
              error ("Invalid Interrupt Number %d, set interrupts 0 - 32.\n",
                  interruptNum);
          /* Construct the RSP */
          sprintf(tmpbuf, "qInterruptReset,%x", interruptNum);
          memset(message,0,strlen(message));
          strcat (message,tmpbuf);
       }
        
       /* get the type of interrupt to be cleared
        * default = both */
       while ((substr=strtok(NULL, " ")) != NULL)
       {
         nargs +=1;
          
         if(nargs > 2)
           error ("Usage: reset-interrupt <number> [both] <periodic> <pin>.\n");
           
         /* Search for interrupt type to be cleared */
         for(interruptNum=0; interruptNum < 3; interruptNum++)
         {
           if(strcmp(substr, pintType[interruptNum])==0)
             break;
         }
         switch (interruptNum)
         {
           case 0:
           case 1:
           case 2:
            sprintf(tmpbuf, ",%x", interruptNum);
            strcat (message,tmpbuf);
           break;

           default:
             error ("Invalid type '%s' specified for reset interrupt.\n",substr);
             break;
          }
      }// while substr-strtok ..
  } //if(substr != NULL)
  else
     error ("Usage: reset-interrupt <number> [both] <periodic> <pin>.\n");
  
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

}

void 
init_set_interruptmap(void)
{

  struct cmd_list_element *c;
  c = add_com ("remap", no_class, q6interrupt_remap,
	       "Remap an interrupt.");
  set_cmd_completer (c, location_completer);
}
void 
init_reset_interrupt(void)
{

  struct cmd_list_element *c;

 /* add_prefix_cmd ("reset", no_class, q6interrupt_reset,
                  concat("Use 'reset interrupt' "\
                  "to reset periodic/pin interrupts.", NULL),
                  &resetlist, "reset ", 1, &cmdlist);
  c = add_cmd ("interrupt", no_class, q6interrupt_reset,
           "Usage: reset interrupt <number> [both] <periodic> <pin>.",
           &resetlist);*/
  
  c = add_com ("reset-interrupt", no_class, q6interrupt_reset,
	       "Reset an interrupt.\n"\
           "Usage: reset-interrupt <number> [both] <periodic> <pin>."); 
  set_cmd_completer (c, location_completer);
}

static void
qdsp6_command (char *args, int from_tty)
{
  printf_unfiltered (_("\"q6\" prefix must be followed by the name of an QDSP6 command.\n"));
  help_list (q6cmdlist, "q6 ", -1, gdb_stdout);
}
static void
qdsp6_watch_command (char *args, int from_tty)
{
  printf_unfiltered (_("QDSP6 watch command.\n"));

}
 
void 
init_q6_commands (void)
{
  /* initialize the global registers */
  init_globalregs();    
 
  /* initialize target args */
  init_targetargs();

  init_set_interrupt();
  init_reset_interrupt();
  init_set_interruptmap();

  /* set mode for h/w debug only */
  init_set_hwthread_debug_only_mode();

}

void
_initialize_qdsp6_tdep (void)
{

  extern int remote_timeout;

  /*Chain containing all defined set subcommands */
  struct cmd_list_element *q6setlist = NULL;

  register_gdbarch_init (bfd_arch_qdsp6, qdsp6_gdbarch_init);

  add_prefix_cmd ("q6", class_support, qdsp6_command,
		  _("Various QDSP6 specific commands."),
		  &q6cmdlist, "q6 ", 0, &cmdlist);

  add_cmd ("watch", class_breakpoint, qdsp6_watch_command,
_("set either a cycle count or tlbmiss breakpoint.\n\
	q6 watch cycle <cycle count>\n\
	q6 watch tlbmiss <32 bit addr> <page size in bits>\n"),
	   &q6cmdlist);

  /* info qdsp6-rtos */
  add_info ("qdsp6-rtos", qdsp6_rtos_info_command,
	    "QDSP6 RTOS information");

  /* info pagetable */
  add_info ("pagetable", qdsp6_pagetable_info_command,
	    "Pagetable information");

  /* info TLB */
  add_info ("tlb", qdsp6_tlb_info_command,
	    "TLB information");

  /* info globalregs */
  add_info ("globalregs", qdsp6_globalregs_info_command,
	    "List of global registers and their contents");

  /* thread details informatino */
  add_info ("thread-details", qdsp6_thread_details_info_command,
	    "Details information for a thread-id");

  /* processor cycle count information */
  add_info ("proc-cycles", qdsp6_proc_cycles_info_command,
            "Processor cycle count");

  /* processor cycle count information */
  add_info ("thread-cycles", qdsp6_thread_cycles_info_command,
            "Thread cycle count");
      
  /*Interrupt map  information */
  add_info ("interrupt-map", qdsp6_interrupt_map_info_command,
            "Interrupt map");
              /*Qube msg channel information */
  add_info ("msg-channels", qdsp6_msg_channels_info_command,
            "RTOS Message Channels Info");
            
  /*Qube msg queue information */
  add_info ("msg-queue", qdsp6_msg_queues_info_command,
            "RTOS Message Queues Info");
             
   /*Qube timers information */
  add_info ("timers", qdsp6_timers_info_command,
            "RTOS Timers Info");
            
  /*Qube msg queue information */
  add_info ("mutex", qdsp6_mutex_info_command,
            "RTOS Mutex Info");
            
  init_q6_commands(); 

/* Specific to our implementation, rumi and zebu can be slow. */
  // XXX_SM: remote_timeout = 10; This should be part of the rumi target when done

 }

/* Free resources allocated by qdsp6-tdep.c */
 void 
 free_q6alloc_mem(void)
 {
   if(q6globalregsInfo != NULL)
    free (q6globalregsInfo);
        
   if(q6Interrupt != NULL)
    free (q6Interrupt);

   if(current_q6_target != NULL)
    xfree(current_q6_target);

 }

