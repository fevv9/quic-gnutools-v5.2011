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
#include "language.h"
#include "gdb_string.h"
#include "inferior.h"
#include "gdbcore.h"
#include "arch-utils.h"
#include "regcache.h"
#include "frame.h"
#include "frame-unwind.h"
#include "frame-base.h"
#include "trad-frame.h"
#include "dis-asm.h"
#include "gdb_assert.h"
#include "sim-regno.h"
#include "symtab.h" 
#include "command.h"
#include "remote.h"
#include "gdbthread.h"
#include "gdbcmd.h"
#include "completer.h"
#include "reg_offsets.h"
#include "objfiles.h"
#include "cli/cli-decode.h"


/* following register values taken from architecture register definitions */
#define MAXREGNAMELEN     50
#define NUM_GEN_REGS      32
#define NUM_PER_THREAD_CR 40
#define NUM_GLOBAL_REGS   64
#define TOTAL_PER_THREAD_REGS (NUM_GEN_REGS+NUM_PER_THREAD_CR)
#define NUM_GLOBAL_REGS 64
#define MAX_INTERRUPT 32
#define END_OFFSET             0xFFFFFFFF

/*******************************************************************
 *   Global Variables                                              *
 *******************************************************************/   

/* captures the arguments to the target passed thru the 
 * set targetargs command */
char* q6targetargsInfo = NULL;
char *current_q6_target = NULL;
int Q6_tcl_fe_state = 0;

/*******************************************************************
 *   Extern Variables                                              *
 *******************************************************************/   
extern void _initialize_qdsp6_tdep (void);
extern char *current_q6_target;

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
  char* sName; // reg name
  int   Index; // Offset in reg file
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

/* static pointer which points to the v1/v2 thread registers */
static regtype_t * threadRegSetInfo;
/* static pointer which points to the v1/v2 thread registers */
static regtype_t * globalRegSetInfo;

enum 
{
  first_gpr_regnum          = 0,
  last_gpr_regnum           = NUM_GEN_REGS -1,   // 31
  qdsp6_num_regs            = REG_TID + 1,    // 70 + 1(32 GPR + 32 CR + 6 SCR) 
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
}Qdsp6_Version;

int q6Version = 0;
int q6ArgRegMax = 0;
int qdsp6_version = 0;


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

#define CURRENT_VARIANT (gdbarch_tdep (current_gdbarch))


/* Allocate a new variant structure, and set up default values for all
   the fields.  */
static struct gdbarch_tdep *
new_variant (void)
{
  struct gdbarch_tdep *var;
  int r;
  char buf[20];

  var = xmalloc (sizeof (*var));
  memset (var, 0, sizeof (*var));
  
  var->num_gprs = NUM_GEN_REGS;
  var->num_fprs = 0;
  var->num_hw_watchpoints = 0;
  var->num_hw_breakpoints = 0;

  /* By default, don't supply any general-purpose or floating-point
     register names.  */
  var->register_names 
    = (char **) xmalloc ((TOTAL_PER_THREAD_REGS)* sizeof (char *));

  for (r = 0; r < TOTAL_PER_THREAD_REGS; r++)
    var->register_names[r] = "";

  /* Do, however, supply default names for the known special-purpose
     registers.  */
  var->register_names[REG_SA0]     = "sa0";
  var->register_names[REG_LC0]     = "lc0";
  var->register_names[REG_SA1]     = "sa1";
  var->register_names[REG_LC1]     = "lc1";
  var->register_names[REG_P3_0]    = "p3:0";
  var->register_names[REG_M0]      = "m0";
  var->register_names[REG_M1]      = "m1";
    var->register_names[REG_SR]      = "usr";
  /* else if(q6Version == Q6_V3)  XXX_SM anything? */


  var->register_names[REG_PC]      = "pc";
  var->register_names[REG_UGP]     = "ugp";
  
    var->register_names[REG_GP]     = "gp";
  /* else if(q6Version == Q6_V3)  XXX_SM anything? */


  /* per thread Supervisor Control Registers (SCR )   */
  var->register_names[REG_SGP]     = "sgp";
  var->register_names[REG_SSR]     = "ssr"  ;
  var->register_names[REG_IMASK]   = "imask";
  var->register_names[REG_BADVA]   = "badva";
  var->register_names[REG_ELR]     = "elr";
  var->register_names[REG_TID]     = "tid"; 

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
  if (reg < 0)
    return "?toosmall?";
  if (reg >=  qdsp6_num_regs)
    return "?toolarge?";

  return CURRENT_VARIANT->register_names[reg];
}


static struct type *
qdsp6_register_type (struct gdbarch *gdbarch, int reg)
{
  return builtin_type_int32;

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
  static unsigned char breakpoint_v2[] = {0x0c, 0xdb, 0x00, 0x54};
  static unsigned char* breakpoint;

  if(q6Version == Q6_V2) 
  {
    breakpoint=breakpoint_v2;
    *lenp = sizeof (breakpoint_v2);
  } 
/* XXX_SM : reserve for V3 update 
 * else if (q6Version == Q6_V3) 
 * {
 *   breakpoint=breakpoint_v3;
 *   *lenp = sizeof (breakpoint_v3);
 * }
 */
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

        /* for ignoring 11 immediate bits */
static ULONGEST ALLOCFRAME_OPCODE_MASK;
        /* allocframe opcode ignoring immediate bits */
static ULONGEST ALLOCFRAME_OPCODE_BITS;
static ULONGEST ALLOCFRAME_SIZE_BITS;
static ULONGEST ALLOCFRAME_SIZE_SHIFT;
#define ALLOCFRAME_SIZE(opcode) (((opcode) & ALLOCFRAME_SIZE_BITS) << ALLOCFRAME_SIZE_SHIFT)
        /* for ignoring immediate bits */
static ULONGEST MORE_SP_UPDATE_OPCODE_MASK;
        /* sp update opcode ignoring immediate bits 
          (uses Rd=add(Rs,#s16) where Rd and Rs = r29 */
static ULONGEST MORE_SP_UPDATE_OPCODE_BITS;
        /* for ignoring immediate and Rt bits for 
            byte/half/single/double word */
static ULONGEST CALLEE_SAVE_OPCODE_MASK;
        /* callee save opcode ignoring immediate and Rt
           bits (uses memb(Rs+#11:0)=Rt where Rs = r29
           and Rt = callee save register */
static ULONGEST CALLEE_SAVE_OPCODE_BITS_B;
        /* callee save opcode ignoring immediate and Rt
           bits (uses memh(Rs+#11:1)=Rt where Rs = r29
           and Rt = callee save register */
static ULONGEST CALLEE_SAVE_OPCODE_BITS_H;
        /* callee save opcode ignoring immediate and Rt
           bits (uses memw(Rs+#11:2)=Rt where Rs = r29
           and Rt = callee save register */
static ULONGEST CALLEE_SAVE_OPCODE_BITS_W;
        /* callee save opcode ignoring immediate and Rt
           bits (uses memd(Rs+#11:3)=Rt where Rs = r29
           and Rt = callee save register */
static ULONGEST CALLEE_SAVE_OPCODE_BITS_D;
        /* for ignoring immediate and Rt bits */
static ULONGEST FUNC_ARG_SAVE_OPCODE_MASK;
        /* callee save opcode ignoring immediate and Rt
           bits (uses memw(Rs+#s11:2)=Rt where Rs = r30
           and Rt = callee save register */
static ULONGEST FUNC_ARG_SAVE_OPCODE_BITS;
static ULONGEST FUNC_ARG_SAVE_REG_SHIFT;
static ULONGEST FUNC_ARG_SAVE_REG_BITS;
#define FUNC_ARG_SAVE_REG(opcode) (((opcode) >> FUNC_ARG_SAVE_REG_SHIFT) & FUNC_ARG_SAVE_REG_BITS)

static CORE_ADDR
qdsp6_analyze_prologue (CORE_ADDR pc, struct frame_info *next_frame,
                      struct qdsp6_unwind_cache *info)
{
  ULONGEST op;

  //printf("\n ******* Calling qdsp6_analyze_prologue \n");

  /* Non-zero iff we've seen the instruction that initializes the
     frame pointer for this function's frame.  */
  int fp_set = 0;

  /* If fp_set is non_zero, then this is the distance from
     the stack pointer to frame pointer: fp = sp + fp_offset.  */
  int fp_offset = 0;

  /* Total size of frame prior to any alloca operations. */
  int framesize = 0;

  /* Flag indicating if lr has been saved on the stack.  */
  int lr_saved_on_stack = 0;

  /* The number of the general-purpose register we saved the return
     address ("link register") in, or -1 if we haven't moved it yet.  */
  int lr_save_reg = -1;

  /* Offset (from sp) at which lr has been saved on the stack.  */

  int lr_sp_offset = 0;

  int sp_mod_val = 0; /* allocframe argument value + other sp updates if
                      the change is too big for the allocframe encoding */

  /* If gr_saved[i] is non-zero, then we've noticed that general
     register i has been saved at gr_sp_offset[i] from the stack
     pointer.  */
  char gr_saved[64];
  int gr_sp_offset[64];

  /* The address of the most recently scanned prologue instruction.  */
  CORE_ADDR last_prologue_pc;

  /* The upper bound to of the pc values to scan.  */
  CORE_ADDR lim_pc;


  memset (gr_saved, 0, sizeof (gr_saved));

  last_prologue_pc = pc;

  /* Try to compute an upper limit (on how far to scan) based on the
     line number info.  */
  lim_pc = skip_prologue_using_sal (pc);
  /* If there's no line number info, lim_pc will be 0.  In that case,
     set the limit to be 100 instructions away from pc.  Hopefully, this
     will be far enough away to account for the entire prologue.  Don't
     worry about overshooting the end of the function.  The scan loop
     below contains some checks to avoid scanning unreasonably far.  */
  if (lim_pc == 0)
    lim_pc = pc + 400;

  /* If we have a frame, we don't want to scan past the frame's pc.  This
     will catch those cases where the pc is in the prologue.  */
  if (next_frame)
    {
      CORE_ADDR frame_pc = frame_pc_unwind (next_frame);
      if (frame_pc < lim_pc)
	lim_pc = frame_pc;
    }

//printf("  *** HERE\n");

        /* Scan the prologue.  */
    /* for ignoring 11 immediate bits */
    ALLOCFRAME_OPCODE_MASK = 0xFFFFF800UL;
    /* allocframe opcode ignoring immediate bits */
    ALLOCFRAME_OPCODE_BITS = 0xA09DC000UL;
    ALLOCFRAME_SIZE_BITS = 0x000007FFUL;
    ALLOCFRAME_SIZE_SHIFT = 3UL;
    /* for ignoring immediate bits */
    MORE_SP_UPDATE_OPCODE_MASK = 0xF01FC01FUL;
    /* sp update opcode ignoring immediate bits 
      (uses Rd=add(Rs,#s16) where Rd and Rs = r29 */
    MORE_SP_UPDATE_OPCODE_BITS = 0xB01DC01DUL;
    /* for ignoring immediate and Rt bits for 
        byte/half/single/double word */
    CALLEE_SAVE_OPCODE_MASK = 0xF1DFC000UL;
    /* callee save opcode ignoring immediate and Rt
       bits (uses memb(Rs+#11:0)=Rt where Rs = r29
       and Rt = callee save register */
    CALLEE_SAVE_OPCODE_BITS_B = 0xA11DC000UL;
    /* callee save opcode ignoring immediate and Rt
       bits (uses memh(Rs+#11:1)=Rt where Rs = r29
       and Rt = callee save register */
    CALLEE_SAVE_OPCODE_BITS_H = 0xA15DC000UL;
    /* callee save opcode ignoring immediate and Rt
       bits (uses memw(Rs+#11:2)=Rt where Rs = r29
       and Rt = callee save register */
    CALLEE_SAVE_OPCODE_BITS_W = 0xA19DC000UL;
    /* callee save opcode ignoring immediate and Rt
       bits (uses memd(Rs+#11:3)=Rt where Rs = r29
       and Rt = callee save register */
    CALLEE_SAVE_OPCODE_BITS_D = 0xA1DDC000UL;
    /* for ignoring immediate and Rt bits */
    FUNC_ARG_SAVE_OPCODE_MASK = 0xA19FC000L;
    /* callee save opcode ignoring immediate and Rt
       bits (uses memw(Rs+#s11:2)=Rt where Rs = r30
       and Rt = callee save register */
    FUNC_ARG_SAVE_OPCODE_BITS = 0xA19EC000UL;
    FUNC_ARG_SAVE_REG_BITS = 0x1FUL;
    FUNC_ARG_SAVE_REG_SHIFT = 8UL;
    


#define ALLOCFRAME_MATCH(opcode) \
        (ALLOCFRAME_OPCODE_BITS == (ALLOCFRAME_OPCODE_MASK & (opcode)))
#define MORE_SP_UPDATE_MATCH(opcode) \
        (MORE_SP_UPDATE_OPCODE_BITS == (MORE_SP_UPDATE_OPCODE_MASK & (opcode)))
#define MORE_SP_UPDATE_SIZE(opcode) \
        ((signed short) ((((unsigned short) ((op >> 21) & 0x7F)) << 9) | \
                          ((unsigned short) ((op >> 5) & 0x1FF))))
// for byte accesses - check if it is a store byte insn 
#define CALLEE_SAVE_MATCH_B(opcode) \
        (CALLEE_SAVE_OPCODE_BITS_B == (CALLEE_SAVE_OPCODE_MASK & (opcode)) && \
             is_callee_saves_reg(CALLEE_SAVE_REG(opcode))) 
// get if Rt if a calle saved register             
#define CALLEE_SAVE_REG(opcode) (((opcode) >> 8) & 0x1F)


// for byte accesses  - skip ignore bits
#define CALLEE_SAVE_OFFSET_B(opcode) \
        ((signed short) (((unsigned short) ((((opcode) >> 25) & 0x3) <<  11)) | \
                         ((unsigned short) ((((opcode) >> 13) & 0x01) << 10)) | \
                         ((unsigned short) (( (opcode)        & 0xFF) <<  0))))


// for half word  accesses - check if it is a store hword insn 
#define CALLEE_SAVE_MATCH_H(opcode) \
        (CALLEE_SAVE_OPCODE_BITS_H == (CALLEE_SAVE_OPCODE_MASK & (opcode)) && \
             is_callee_saves_reg(CALLEE_SAVE_REG(opcode))) 

// for half wordaccesses  - skip ignore bits
#define CALLEE_SAVE_OFFSET_H(opcode) \
        ((signed short) (((unsigned short) ((((opcode) >> 25) & 0x3) <<  11)) | \
                         ((unsigned short) ((((opcode) >> 13) & 0x01) << 10)) | \
                         ((unsigned short) (( (opcode)        & 0xFF) <<  1))))


// for single word accesses
#define CALLEE_SAVE_MATCH_W(opcode) \
        (CALLEE_SAVE_OPCODE_BITS_W == (CALLEE_SAVE_OPCODE_MASK & (opcode)) && \
             is_callee_saves_reg(CALLEE_SAVE_REG(opcode))) 

// for single word accesses
#define CALLEE_SAVE_OFFSET_W(opcode) \
        ((signed short) (((unsigned short) ((((opcode) >> 25) & 0x3) <<  11)) | \
                         ((unsigned short) ((((opcode) >> 13) & 0x01) << 10)) | \
                         ((unsigned short) (( (opcode)        & 0xFF) <<  2))))




// for double word accesses
#define CALLEE_SAVE_MATCH_D(opcode) \
        (CALLEE_SAVE_OPCODE_BITS_D == (CALLEE_SAVE_OPCODE_MASK & (opcode)) && \
             is_callee_saves_reg(CALLEE_SAVE_REG(opcode))) 


// for double word accesses
#define CALLEE_SAVE_OFFSET_D(opcode) \
        ((signed short) (((unsigned short) ((((opcode) >> 25) & 0x3) <<  11)) | \
                         ((unsigned short) ((((opcode) >> 13) & 0x01) << 10)) | \
                         ((unsigned short) (( (opcode)        & 0xFF) <<  3))))


#define FUNC_ARG_SAVE_MATCH(opcode) \
        (FUNC_ARG_SAVE_OPCODE_BITS == (FUNC_ARG_SAVE_OPCODE_MASK & (opcode)) && \
             is_argument_reg(FUNC_ARG_SAVE_REG(opcode)))

      op = read_memory_unsigned_integer (pc, 4);
      if (ALLOCFRAME_MATCH(op)) 
      {
	//printf("  *** ALLOCFRAME_MATCH(op)\n");
	int offset = - ALLOCFRAME_SIZE(op);

       /* process optional additional sp updates 
          (when allocframe immediate was not big enough) */
      sp_mod_val = 0;
      do {
        sp_mod_val += offset;
	pc += 4;
        op = read_memory_unsigned_integer (pc, 4);
        offset = MORE_SP_UPDATE_SIZE(op); 
        /* offset should be negative since stack grows downward */
      } while ((MORE_SP_UPDATE_MATCH(op) && offset < 0) &&
	       (pc < lim_pc));

      //printf(" ** sp_mod_val = %d\n", sp_mod_val);


      /* record stores of callee save registers
	 (offsets for these should be non-negative) for memb(Rs+#s11:0)=Rt */
      offset = CALLEE_SAVE_OFFSET_B(op);
      //printf("  * CALLEE_SAVE_MATCH_B(0x%08x) = 0x%08x, offset = 0x%08x\n", op, CALLEE_SAVE_MATCH_B(op), offset);
      while ((CALLEE_SAVE_MATCH_B(op) && offset >= 0) &&
	     (pc < lim_pc)) 
      {
        int callee_save_reg = CALLEE_SAVE_REG(op);
        gr_saved[callee_save_reg] = 1;
        gr_sp_offset[callee_save_reg] = offset;
	pc += 4;
        op = read_memory_unsigned_integer (pc, 4);
        offset = CALLEE_SAVE_OFFSET_B(op);
      }

      //printf("  * offset = 0x%08x\n", offset);

      /* record stores of callee save registers 
	 (offsets for these should be non-negative) for memh(Rs+#s11:1)=Rt */
      offset = CALLEE_SAVE_OFFSET_H(op);
      while ((CALLEE_SAVE_MATCH_H(op) && offset >= 0) &&
	     (pc < lim_pc))
      {
	int callee_save_reg = CALLEE_SAVE_REG(op);
	gr_saved[callee_save_reg] = 1;
        gr_sp_offset[callee_save_reg] = offset;
	pc += 4;
        op = read_memory_unsigned_integer (pc, 4);
        offset = CALLEE_SAVE_OFFSET_H(op);
      }

      /* record stores of callee save registers 
         (offsets for these should be non-negative) for memw(Rs+#s11:2)=Rt */
      offset = CALLEE_SAVE_OFFSET_W(op);
      while ((CALLEE_SAVE_MATCH_W(op) && offset >= 0) &&
	     (pc < lim_pc))
      {
        int callee_save_reg = CALLEE_SAVE_REG(op);
        gr_saved[callee_save_reg] = 1;
        gr_sp_offset[callee_save_reg] = offset;
	pc += 4;
        op = read_memory_unsigned_integer (pc, 4);
        offset = CALLEE_SAVE_OFFSET_W(op);
      }

      /* record stores of callee save registers
	 (offsets for these should be non-negative) for memd(Rs+#s11:3)=Rt */
      offset = CALLEE_SAVE_OFFSET_D(op);
      while ((CALLEE_SAVE_MATCH_D(op) && offset >= 0) &&
	     (pc < lim_pc))
      {
        int callee_save_reg = CALLEE_SAVE_REG(op);
        gr_saved[callee_save_reg] = 1;
        gr_sp_offset[callee_save_reg] = offset;
	pc += 4;
        op = read_memory_unsigned_integer (pc, 4);
        offset = CALLEE_SAVE_OFFSET_D(op);
      }



      /* record stores of function arguments registers  */
      while ((FUNC_ARG_SAVE_MATCH(op)) &&
	     (pc < lim_pc))
      {
	pc += 4;
        op = read_memory_unsigned_integer (pc, 4);
      }
    }

    //printf("  *** pc = 0x%08x\n", pc);

    last_prologue_pc = pc;



  if (next_frame && info)
    {
      int i;
      ULONGEST this_base;

      frame_unwind_register (next_frame, REG_FP, &this_base);

      for (i = 0; i < 64; i++)
	if (gr_saved[i])
	  info->saved_regs[i].addr = this_base + sp_mod_val + gr_sp_offset[i];

      info->saved_regs[REG_FP].addr = this_base;
      info->saved_regs[REG_LR].addr = this_base + 4;
      info->saved_regs[REG_PC] = info->saved_regs[REG_LR];

      info->prev_sp = this_base + 8;
      info->base = this_base;
      trad_frame_set_value (info->saved_regs, REG_SP, info->prev_sp);

    }

  return last_prologue_pc;
}


static CORE_ADDR
qdsp6_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR pc)
{
  CORE_ADDR func_addr, func_end, new_pc;
  //printf("\n ******* Calling qdsp6_skip_prologue \n");

  new_pc = pc;

  /* If the line table has entry for a line *within* the function
     (i.e., not in the prologue, and not past the end), then that's
     our location.  */
  //printf(" **** find_pc_partial_function(0x%08x, 0x0, 0x%08x, 0x%08x) = %d\n", pc, &func_addr, &func_end, find_pc_partial_function (pc, NULL, &func_addr, &func_end));
  if (find_pc_partial_function (pc, NULL, &func_addr, &func_end))
    {
      struct symtab_and_line sal;

      //printf(" * * func_addr = 0x%08x, func_end = 0x%08x\n", func_addr, func_end);

      sal = find_pc_line (func_addr, 0);

      //printf(" * * sal.line = 0x%08x, sal.end = 0x%08x\n", sal.line, sal.end);

      if (sal.line != 0 && sal.end < func_end)
	{
	  new_pc = sal.pc;
	}
    }

  /* qdsp6 prologue is at least 4 bytes (allocframe) */
  if (new_pc < pc + 4) 
    new_pc = qdsp6_analyze_prologue (pc, 0, 0);

  return new_pc;
}


static struct qdsp6_unwind_cache *
qdsp6_frame_unwind_cache (struct frame_info *next_frame,
			 void **this_prologue_cache)
{
  struct gdbarch *gdbarch = get_frame_arch (next_frame);
  CORE_ADDR pc;
  ULONGEST prev_sp;
  ULONGEST this_base;
  struct qdsp6_unwind_cache *info;

  //printf("\n ******* Calling qdsp6_frame_unwind_cache \n");
  if ((*this_prologue_cache))
    return (*this_prologue_cache);

  info = FRAME_OBSTACK_ZALLOC (struct qdsp6_unwind_cache);
  (*this_prologue_cache) = info;
  info->saved_regs = trad_frame_alloc_saved_regs (next_frame);

  /* Prologue analysis does the rest...  */
  qdsp6_analyze_prologue (frame_func_unwind (next_frame, NORMAL_FRAME), next_frame, info);

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
      store_unsigned_integer (valbuf, len, gpr0_val);
    }
  else if (len <= 8)
    {
      ULONGEST regval;
      regcache_cooked_read_unsigned (regcache, 0, &regval);
      store_unsigned_integer (valbuf, 4, regval);
      regcache_cooked_read_unsigned (regcache, 1, &regval);
      store_unsigned_integer ((bfd_byte *) valbuf + 4, 4, regval);
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
qdsp6_return_value (struct gdbarch *gdbarch, struct type *type,
                   struct regcache *regcache, void *readbuf,
                   const void *writebuf)
{
  //printf("\nCalling qdsp6_return_value .........................");

  // anything less than 8 bytes go in registers
  if (((TYPE_CODE (type) == TYPE_CODE_STRUCT
       || TYPE_CODE (type) == TYPE_CODE_UNION) &&
       TYPE_LENGTH (type) > 8) || (TYPE_LENGTH (type) > 8))
      /* Structs, unions, and anything larger than 8 bytes (2 registers)
       *        goes on the stack.  */
      return RETURN_VALUE_STRUCT_CONVENTION;

   if (readbuf)
       qdsp6_extract_return_value (type, regcache, readbuf);
   if (writebuf)
       qdsp6_store_return_value (type, regcache, writebuf);

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
qdsp6_push_dummy_call (struct gdbarch *gdbarch, CORE_ADDR func_addr,
                     struct regcache *regcache, CORE_ADDR bp_addr,
                     int nargs, struct value **args, CORE_ADDR sp,
		     int struct_return, CORE_ADDR struct_addr)
{
  int argreg;
  int argnum;
  int len;
  int stack_space = 0, stack_offset;
  int numOfFixedTypeFunctionArgs = 0;
  char *val;
  char valbuf[32];
  char *sDemangledFunctionName    = NULL;
  CORE_ADDR regval;
  enum type_code typecode;
  struct value *arg;
  struct type *arg_type;
  struct type *ftype;
  struct symbol  *pFuncSymbol    =  NULL;
  struct minimal_symbol *msymbol =  NULL;

  
  
#if 0
  printf("\n In qdsp6_push_dummy_call ........................\n");
  printf("\nFunction Address = %d\n", func_addr);
  printf("Push %d args at sp = %x, struct_return=%d (%x)\n",
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
	  store_unsigned_integer (valbuf, 4, VALUE_ADDRESS (arg));
	  typecode = TYPE_CODE_PTR;
	  val = valbuf;
	  write_memory (sp+stack_offset, value_contents (arg), len);
	  stack_offset += align_up (len, 4);
	  // everything written to memory, nothing to write in regs
	  len = 0;
	}
      else
	{
	  val = (char *) value_contents (arg);
	}


      while (len > 0)
	{
	  int partial_len = (len < 4 ? len : 4);

	  /* Pass function params into regs R0-R5 for only fixed
	     type of arguments */
	  if ((argreg < q6ArgRegMax) &&
              (argnum < numOfFixedTypeFunctionArgs))
	    {
	      regval = extract_unsigned_integer (val, partial_len);
#if 0
	      printf("  Argnum %d data %x -> reg %d\n",
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
	    regval = extract_unsigned_integer (val, partial_len);
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
	      printf("  Argnum %d data %x -> offset %d (%x)\n",
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
  //printf("\n In qdsp6_store_return_value .....................\n");
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

#if 0
/* Hardware watchpoint / breakpoint support   */

int
qdsp6_check_watch_resources (int type, int cnt, int ot)
{
  struct gdbarch_tdep *var = CURRENT_VARIANT;

  /* Watchpoints not supported on simulator.  */
  if (strcmp (target_shortname, "sim") == 0)
    return 0;

  if (type == bp_hardware_breakpoint)
    {
      if (var->num_hw_breakpoints == 0)
	return 0;
      else if (cnt <= var->num_hw_breakpoints)
	return 1;
    }
  else
    {
      if (var->num_hw_watchpoints == 0)
	return 0;
      else if (ot)
	return -1;
      else if (cnt <= var->num_hw_watchpoints)
	return 1;
    }
  return -1;
}


CORE_ADDR
qdsp6_stopped_data_address (void)
{
    return 0;
}
#endif

static CORE_ADDR
qdsp6_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  return frame_unwind_register_unsigned (next_frame, REG_PC);
}

/* Given a GDB frame, determine the address of the calling function's
   frame.  This will be used to create a new GDB frame struct.  */

static void
qdsp6_frame_this_id (struct frame_info *next_frame,
		    void **this_prologue_cache, struct frame_id *this_id)
{
  struct qdsp6_unwind_cache *info
    = qdsp6_frame_unwind_cache (next_frame, this_prologue_cache);
  CORE_ADDR base;
  CORE_ADDR func;
  struct minimal_symbol *msym_stack;
  struct frame_id id;

  /* The FUNC is easy.  */
  func = frame_func_unwind (next_frame, NORMAL_FRAME);

  /* Check if the stack is empty.  */
  msym_stack = lookup_minimal_symbol ("_stack", NULL, NULL);
  if (msym_stack && info->base == SYMBOL_VALUE_ADDRESS (msym_stack))
    return;

  /* Hopefully the prologue analysis either correctly determined the
     frame's base (which is the SP from the previous frame), or set
     that base to "NULL".  */
  base = info->prev_sp;
  if (base == 0)
    return;

  id = frame_id_build (base, func);

  /* Check that we're not going round in circles with the same frame
     ID (but avoid applying the test to sentinel frames which do go
     round in circles).  Can't use frame_id_eq() as that doesn't yet
     compare the frame's PC value.  */
  if (frame_relative_level (next_frame) >= 0
      && get_frame_type (next_frame) != DUMMY_FRAME
      && frame_id_eq (get_frame_id (next_frame), id))
    return;

  (*this_id) = id;
}

static void
qdsp6_frame_prev_register (struct frame_info *next_frame,
			  void **this_prologue_cache,
			  int regnum, int *optimizedp,
			  enum lval_type *lvalp, CORE_ADDR *addrp,
			  int *realnump, void *bufferp)
{
  struct qdsp6_unwind_cache *info
    = qdsp6_frame_unwind_cache (next_frame, this_prologue_cache);
    trad_frame_get_prev_register (next_frame, info->saved_regs, regnum,
                                  optimizedp, lvalp, addrp, realnump, bufferp);

}

static const struct frame_unwind qdsp6_frame_unwind = {
  NORMAL_FRAME,
  qdsp6_frame_this_id,
  qdsp6_frame_prev_register
};

static const struct frame_unwind *
qdsp6_frame_sniffer (struct frame_info *next_frame)
{
  return &qdsp6_frame_unwind;
}

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
qdsp6_unwind_dummy_id (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  return frame_id_build (qdsp6_unwind_sp (gdbarch, next_frame),
			 frame_pc_unwind (next_frame));
}


static struct gdbarch *
qdsp6_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *var;

  /* Check to see if we've already built an appropriate architecture
     object for this executable.  */
  arches = gdbarch_list_lookup_by_info (arches, &info);
  if (arches)
    return arches->gdbarch;
 
 /* V1/V2 registers set by new_variant, hence move v1/v2 specific info
  * out of the switch stmt to here before call to new_variant */
  switch (info.bfd_arch_info->mach)
    {
    case bfd_mach_qdsp6_v2:
      q6Version = Q6_V2;
      q6ArgRegMax = 6;
      threadRegSetInfo = &threadRegSetInfo_v2[0];
      globalRegSetInfo = &globalRegSetInfo_v2[0];
    break;
    
    case bfd_mach_qdsp6_v3:
      q6Version = Q6_V3;
      q6ArgRegMax = 6;
      threadRegSetInfo = &threadRegSetInfo_v3[0];
      globalRegSetInfo = &globalRegSetInfo_v3[0];
    break;
    
    default:
      /* Never heard of this variant.  */
      return 0;
    }

  /* Select the right tdep structure for this variant (qdsp6).  */
  var = new_variant ();
  switch (info.bfd_arch_info->mach)
    {
    case bfd_mach_qdsp6:
    case bfd_mach_qdsp6_v2:
    case bfd_mach_qdsp6_v3:
      set_variant_num_gprs (var, 32);
      set_variant_num_fprs (var, 0);
      break;

    default:
      /* Never heard of this variant.  */
      return 0;
    }
  
  gdbarch = gdbarch_alloc (&info, var);

  set_gdbarch_short_bit (gdbarch, 16);
  set_gdbarch_int_bit (gdbarch, 32);
  set_gdbarch_long_bit (gdbarch, 32);
  set_gdbarch_long_long_bit (gdbarch, 64);
#if 0
  set_gdbarch_float_bit (gdbarch, 32);
  set_gdbarch_double_bit (gdbarch, 64);
  set_gdbarch_long_double_bit (gdbarch, 64);
#endif
  set_gdbarch_ptr_bit (gdbarch, 32);

  set_gdbarch_num_regs (gdbarch, qdsp6_num_regs);
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
  frame_unwind_append_sniffer (gdbarch, qdsp6_frame_sniffer);
  frame_base_set_default (gdbarch, &qdsp6_frame_base);

  /* Settings for calling functions in the inferior.  */
  set_gdbarch_push_dummy_call (gdbarch, qdsp6_push_dummy_call);
  set_gdbarch_unwind_dummy_id (gdbarch, qdsp6_unwind_dummy_id);

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
  set_gdbarch_call_dummy_location (gdbarch, ON_STACK);

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

  if (q6Version == Q6_V3)
    numGblRegs = sizeof(globalRegSetInfo_v3)/sizeof(regtype_t);
  else
    numGblRegs = sizeof(globalRegSetInfo_v2)/sizeof(regtype_t);

  for (i = 0; i < numGblRegs; i++)
  {

    if(globalRegSetInfo[i].sName == NULL)
        break;

    if ((regname != NULL)                    && 
	(len == strlen (globalRegSetInfo[i].sName)) && 
	(strncmp (globalRegSetInfo[i].sName,regname,len) == 0))
    {
        regnum =  globalRegSetInfo[i].Index ;
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


static int
fromhex (int a)
{
  if (a >= '0' && a <= '9')
    return a - '0';
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else
    error ("Reply contains invalid hex digit %d", a);
}

static int
hex2bin (const char *hex, char *bin, int count)
{
  int i;

  for (i = 0; i < count; i++)
    {
      if (hex[0] == 0 || hex[1] == 0)
	{
	  /* Hex string is short, or of uneven length.
	     Return the count that has been converted so far. */
	  return i;
	}
      *bin = fromhex (hex[0]) * 16 + fromhex (hex[1]);
      bin++;
      hex += 2;
    }
  return i;
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
  ULONGEST pSize = 0;

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
  getpkt (pageBuf, &gdb_rsp_ril_info_max_size , 0);

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
      regValues[i] = extract_signed_integer(&sDisplayBuf, lenValue/2);
    
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
         qdsp6_print_reg_info(globalRegSetInfo[i].sName, regValues[n] );
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
      qdsp6_print_reg_info(globalRegSetInfo[i].sName, regValues[n]);
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
      regValues[i] = extract_signed_integer(&sDisplayBuf, lenValue/2);
    
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
      qdsp6_print_reg_info(globalRegSetInfo[i].sName, regValues[n] );
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
      qdsp6_print_reg_info(globalRegSetInfo[i].sName, regValues[n]);
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

void  
setQ6targetargs (char * args, int tty, struct cmd_list_element *cmdlist)
{
/* empty function */
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

   /* allocate memory for control variable for setshow command */
   q6targetargsInfo  = (char*)malloc(sizeof(char) *  SET_CMD_BUFFER_SIZE);

   if(q6targetargsInfo ==  NULL)
     error ("Memory allocation for target args failed!");
   else 
    memset ( q6targetargsInfo, 0, sizeof(char) *  SET_CMD_BUFFER_SIZE);
  
   /* set global regs */
   add_setshow_string_noescape_cmd ("targetargs",         /* name */
		            no_class,      /* class category for help list */
		            &q6targetargsInfo,    /* control var address */
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
  remote_timeout = 10;

 }

/* Free resources allocated by qdsp6-tdep.c */
 void 
 free_q6alloc_mem(void)
 {
   if(q6globalregsInfo != NULL)
    free (q6globalregsInfo);
        
   if(q6Interrupt != NULL)
    free (q6Interrupt);

   if(q6targetargsInfo != NULL)
    free (q6targetargsInfo);
   
   if(current_q6_target != NULL)
    xfree(current_q6_target);

 }

 
