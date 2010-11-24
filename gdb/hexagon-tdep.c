/* Target-dependent code for QUALCOMM HEXAGON GDB, the GNU Debugger.
   Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.

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
#include "hexagon-tdep.h"
#include "cli/cli-decode.h"
#include "hexagon-sim.h"



/*******************************************************************
 *   Global Variables                                              *
 *******************************************************************/   
int hexagon_debug = 0;

/*******************************************************************
 *   Extern Variables                                              *
 *******************************************************************/   
extern void _initialize_hexagon_tdep (void);
extern char *current_hexagon_target;
extern int frame_debug;

static gdbarch_init_ftype hexagon_gdbarch_init;
static gdbarch_register_name_ftype hexagon_register_name;
static gdbarch_breakpoint_from_pc_ftype hexagon_breakpoint_from_pc;
static gdbarch_adjust_breakpoint_address_ftype hexagon_gdbarch_adjust_breakpoint_address;
static gdbarch_skip_prologue_ftype hexagon_skip_prologue;
static LONGEST hexagon_call_dummy_words[] ={0};


#define END_OFFSET             0xFFFFFFFF
// To automatically get all thread registers from arch
static hexagon_regtype_t threadRegSetInfo_v2[]={
#include "v2/thread_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static hexagon_regtype_t globalRegSetInfo_v2[]={
#include "v2/global_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static hexagon_regtype_t threadRegSetInfo_v3[]={
#include "v3/thread_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static hexagon_regtype_t globalRegSetInfo_v3[]={
#include "v3/global_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static hexagon_regtype_t threadRegSetInfo_v4[]={
#include "v4/thread_regs.h"
  {"", END_OFFSET}
};

// To automatically get all thread registers from arch
static hexagon_regtype_t globalRegSetInfo_v4[]={
#include "v4/global_regs.h"
  {"", END_OFFSET}
};

/* pointer to the architecture's thread registers */
hexagon_regtype_t * threadRegSetInfo;
/* pointer to the architecture's thread registers */
hexagon_regtype_t * globalRegSetInfo;

struct hexagon_system_register_offsets *hexagon_RegOffset = (struct hexagon_system_register_offsets *)0;


enum 
{
  first_gpr_regnum          = 0,
  last_gpr_regnum           = NUM_GEN_REGS -1,   // 31
  hexagon_num_pseudo_regs     = 0,                 // Pseudo registers 
  struct_return_regnum      = 0,                 // !!! DRP 
  first_fpr_regnum          = 64,
  last_fpr_regnum           = 63,
};


typedef enum 
{
  HEXAGON_INV_VER                   = 0, 
  HEXAGON_V1                        = 1,
  HEXAGON_V2                        = 2,   // 31
  HEXAGON_V3                        = 3,   // 31
  HEXAGON_V4                        = 4,   // 31
}Qdsp6_Version;

int hexagon_Version = 0;
int hexagon_ArgRegMax = 0;
int hexagon_version = 0;
int hexagon_reg_index_max = 0;
int hexagon_reg_count = 0;


struct hexagon_unwind_cache		/* was struct frame_extra_info */
{
    /* The previous frame's inner-most stack address.  Used as this
       frame ID's stack_addr.  */
    CORE_ADDR prev_sp;
    
    /* The frame's base, optionally used by the high-level debug info.  */
    CORE_ADDR base;
    
    /* Table indicating the location of each and every register.  */
    struct trad_frame_saved_reg *saved_regs;
};


/* A structure describing the HEXAGON architecture.
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
 * function: hexagon_index_max
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
hexagon_index_max (hexagon_regtype_t *rt)
{
  int max_index = 0;
  while (rt->index != END_OFFSET)
    {
      max_index = max (hexagon_reg_index_max, rt->index);
      hexagon_reg_count++;
      rt++;
    }

  if (hexagon_debug == 1)
  {
    printf_filtered ("HEXAGON_DEB: hexagon_Version = %d, max reg index = %d, thread reg count = %d\n", hexagon_Version, hexagon_reg_index_max, hexagon_reg_count);
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
  extern int hexagon_Version;

  var = xmalloc (sizeof (*var));
  memset (var, 0, sizeof (*var));
  
  var->num_gprs = NUM_GEN_REGS;
  var->num_fprs = 0;
  var->num_hw_watchpoints = 0;
  var->num_hw_breakpoints = 0;

  gdb_assert (hexagon_reg_index_max != 0);

  reg_count = hexagon_reg_index_max+1;

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
  if(hexagon_Version < HEXAGON_V4)
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
      var->register_names[REG_G0]      = "g0"; 
      var->register_names[REG_G1]      = "g1"; 
      var->register_names[REG_G2]      = "g2"; 
      var->register_names[REG_G3]      = "g3"; 
  }

  return var;
}


/* Indicate that HEXAGON  has NUM_GPRS general-purpose
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


/* Indicate that  HEXAGON has has NUM_FPRS floating-point
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
hexagon_register_name (struct gdbarch *gdbarch, int reg)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  if (reg < 0)
    return "?toosmall?";
  if (reg >  hexagon_reg_index_max)
    return "?toolarge?";

  return tdep->register_names[reg];
}


static struct type *
hexagon_register_type (struct gdbarch *gdbarch, int reg)
{
    return builtin_type (gdbarch)->builtin_uint32;
}

static const unsigned char *
hexagon_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *pcptr, int *lenp)
{
  /* breakpoint instruction is trap0(#0xDB) which encodes as  0c db 00 54 for v2*/
  static unsigned char breakpoint_insn[] = {0x0c, 0xdb, 0x00, 0x54};
  static unsigned char *breakpoint;

  if ((hexagon_Version == HEXAGON_V4) || 
      (hexagon_Version == HEXAGON_V3) ||
      (hexagon_Version == HEXAGON_V2))
  {
    breakpoint=breakpoint_insn;
    *lenp = sizeof (breakpoint_insn);
  } 
  else
  {
    printf_filtered ("%s, invalid setting for arch level %d\n",__func__, hexagon_Version);
    gdb_assert(0);
  }
  return breakpoint;
}

/* Define the maximum number of instructions which may be packed into a
   packet (VLIW instruction).  */
static const int max_instrs_per_packet = 4;

/* Define the size (in bytes) of an hexagon instruction.  */
static const int hexagon_instr_size = 4;

/* Adjust a breakpoint's address to account for the hexagon architecture's
   constraint that a break instruction must not appear as any but the
   first instruction in the packet.  */
static CORE_ADDR
hexagon_gdbarch_adjust_breakpoint_address (struct gdbarch *gdbarch, 
					 CORE_ADDR bpaddr)
{

  int count =max_instrs_per_packet;
  CORE_ADDR addr = bpaddr - hexagon_instr_size;
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
      char instr[hexagon_instr_size];
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

      addr -= hexagon_instr_size;
    }

  if (count >= 0)
    bpaddr = addr + hexagon_instr_size;

  return bpaddr;
}



/* Return true if REG is a callee-saves register, false otherwise.  */
static int
is_callee_saves_reg (int reg)
{
  //printf("\n In s_callee_saves_reg ..................\n");
  if(hexagon_Version == HEXAGON_V2) 
      return (reg >= 24 && reg <= 27);
  else
      return (reg >= 16 && reg <= 27);
}


/* Return true if REG is an argument register, false otherwise.  */
static int
is_argument_reg (int reg)
{
  //printf("\n In is_argument_reg ...........................\n");
  return (reg < hexagon_ArgRegMax);
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
  enum
  {
	memd_allocframe = 0xeb001c00,
  };
  unsigned int mask = memd_allocframe;

  if (ALLOCFRAME_MATCH (insn))
    {
      *immediate = ALLOCFRAME_SIZE (insn);
      return 1;
    }
  else if (hexagon_Version == HEXAGON_V4)
    {
      if ((insn & mask) == memd_allocframe)
	{
	    *immediate = 8 * ((insn & 0x1f0)>>4);
            return 1;
	}
    }

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
      printf_filtered ("HEXAGON_:%s: PC:0x%lx saving reg %x:%x, offset = %d\n",
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
 * function: is_bogus
 * description:
 * 	- looks for a non-insn, when this happens we have read too
 *        far or received some bogus pc.
 */
static int
is_bogus (unsigned int insn, CORE_ADDR pc)
{
  int rc = 0;
  if (insn == 0x0)
    rc = 1;
  else if (insn == 0x1f1f1f1f)
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
	{
           internal_warning (__FILE__, __LINE__,
                    _("\ntarget_read_memory failed to read address: 0x%lx"),
                    pc);
	   return 0;
	}

      insn = extract_unsigned_integer (&buffer[i], 4, byte_order);
      if ((insn & mask) == endofpacket)
	break;
      if ((insn & mask) == 0) /* An endof packet and a duplex insn */
	break;
    }

  return packet_cnt;
}

/* Scan the hexagon prologue, starting at PC, until frame->PC.
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
hexagon_analyze_prologue (struct gdbarch *gdbarch,
			CORE_ADDR start_pc, CORE_ADDR end_pc,
		        struct frame_info *next_frame,
                        struct hexagon_unwind_cache *info)
{
  ULONGEST op;
  gdb_byte *op_addr=(gdb_byte *)&op;
  gdb_byte buf[16];
  int end_of_prologue = 0;
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);

  int allocframe = 0;
  int could_be_frameless = 1;

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

  if (hexagon_debug == 1)
      printf_filtered ("HEXAGON_DEB: %s, pc = 0x%lx\n",__func__, start_pc);


#if 1
  if (start_pc == 0)
	start_pc = end_pc;
#endif

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
 
      if (!(packet_total = get_packet (pc, buf, byte_order)))
	return 0;

      for (pcnt = 0; pcnt<packet_total; pcnt++)
        {
          insn = extract_unsigned_integer (&buf[pcnt*4], 4, byte_order);

          if (is_allocframe(insn, pc, &operand))
            {
	      prolog_pc+=4;
	      allocframe = -operand;	/* Stack grows downward */
	      could_be_frameless = 0;

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
          else if (is_bogus (insn, pc))
            {
		end_of_prologue = 1;
	        break;
            }
          else 
            {
	      prolog_pc+=4;
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

      if (fp_set || could_be_frameless)
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

      if (hexagon_debug == 1)
      {
	printf_filtered ("\t HEXAGON_DEB: allocframe = %d\n", allocframe);
	printf_filtered ("\t HEXAGON_DEB: info.base = 0x%p\n", info->base);
	printf_filtered ("\t HEXAGON_DEB: reg_sp = 0x%p\n", this_sp);
	printf_filtered ("\t HEXAGON_DEB: this_lr = 0x%p\n", this_lr);
	printf_filtered ("\t HEXAGON_DEB: info.REG_FP  = 0x%x\n", info->saved_regs[REG_FP].addr);
	printf_filtered ("\t HEXAGON_DEB: info.REG_LR  = 0x%x\n", info->saved_regs[REG_LR].addr);
	printf_filtered ("\t HEXAGON_DEB: info.REG_PC  = 0x%x\n", info->saved_regs[REG_PC].addr);
	printf_filtered ("\t HEXAGON_DEB: info.prev_sp = 0x%x\n", info->saved_regs[REG_PC].addr);
      }


    }

  return prolog_pc;
}


static CORE_ADDR
hexagon_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR pc)
{
  CORE_ADDR func_addr, func_end, new_pc;

  new_pc = pc;

  /* If the line table has entry for a line *within* the function
     (i.e., not in the prologue, and not past the end), then that's
     our location.  */

  if (hexagon_debug == 1)
  {
      printf_filtered ("HEXAGON_DEB: %s\n",__func__);
      printf_filtered ("\tHEXAGON_DEB: pc = 0x%lx\n", pc);
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

  /* hexagon prologue is at least 4 bytes (allocframe) */
  if (new_pc < pc + 4) 
    new_pc = hexagon_analyze_prologue (gdbarch, pc, pc, 0, 0);

  if (hexagon_debug == 1)
  {
      printf_filtered ("\tHEXAGON_DEB: returns new_pc = 0x%lx\n", new_pc);
  }
  return new_pc;
}


static struct hexagon_unwind_cache *
hexagon_frame_unwind_cache (struct frame_info *this_frame,
			 void **this_prologue_cache)
{
  struct gdbarch *gdbarch = get_frame_arch (this_frame);
  CORE_ADDR pc;
  ULONGEST prev_sp;
  ULONGEST this_base;
  struct hexagon_unwind_cache *info;

  if (hexagon_debug == 1)
  {
      printf_filtered("HEXAGON_DEB: %s\n",__func__);
      printf_filtered("\tHEXAGON_DEB: *this_prologue_cache = %p\n",
	     *this_prologue_cache);
  }

  if ((*this_prologue_cache))
    return (*this_prologue_cache);

  info = FRAME_OBSTACK_ZALLOC (struct hexagon_unwind_cache);
  (*this_prologue_cache) = info;
  info->saved_regs = trad_frame_alloc_saved_regs (this_frame);

  /* Prologue analysis does the rest...  */
  pc = get_frame_pc(this_frame);
  if (hexagon_debug == 1)
      printf_filtered("\tpre ana: HEXAGON_DEB: pc = 0x%x\n",pc);
  pc = hexagon_analyze_prologue (gdbarch,
			get_frame_func (this_frame), get_frame_pc(this_frame),
			this_frame, info);

  if (hexagon_debug == 1)
  {
      printf_filtered("\tHEXAGON_DEB: pc = 0x%lx\n", pc);
  }
  
  return info;
}

static void
hexagon_extract_return_value (struct type *type, struct regcache *regcache,
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
hexagon_extract_struct_value_address (struct regcache *regcache)
{
  ULONGEST addr;
  //printf("\n In hexagon_extract_struct_value_address ..... ");
  regcache_cooked_read_unsigned (regcache, struct_return_regnum, &addr);
  return addr;
}

static CORE_ADDR
hexagon_frame_align (struct gdbarch *gdbarch, CORE_ADDR sp)
{
  /* Require dword alignment.  */
  return align_down (sp, 8);
}

static void
hexagon_store_return_value (struct type *type, struct regcache *regcache,
                        const void *valbuf);

static enum return_value_convention
hexagon_return_value (struct gdbarch *gdbarch, struct type *func_type,
		   struct type *valtype, struct regcache *regcache,
		   gdb_byte *readbuf, const gdb_byte *writebuf)
{
  //printf("\nCalling hexagon_return_value .........................");

  /* Structs, unions, and anything larger than 8 bytes (2 registers)
     goes on the stack.  */
  int struct_return = (((TYPE_CODE (valtype) == TYPE_CODE_STRUCT
       			 || TYPE_CODE (valtype) == TYPE_CODE_UNION)
			 && TYPE_LENGTH (valtype) > 8) 
			 || (TYPE_LENGTH (valtype) > 8));

   if (readbuf)
     {
       gdb_assert (!struct_return);
       hexagon_extract_return_value (valtype, regcache, readbuf);
     }
   if (writebuf)
     {
       gdb_assert (!struct_return);
       hexagon_store_return_value (valtype, regcache, writebuf);
     }

   if (struct_return)
     return RETURN_VALUE_STRUCT_CONVENTION;
   else
     return RETURN_VALUE_REGISTER_CONVENTION;
}

static CORE_ADDR
hexagon_push_dummy_code (struct gdbarch *gdbarch,
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
hexagon_push_dummy_call (struct gdbarch *gdbarch, struct value *function,
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
  printf("\n In hexagon_push_dummy_call ........................\n");
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
	  if ((argreg < hexagon_ArgRegMax) &&
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
	  else if((argreg < hexagon_ArgRegMax) && 
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

  /* Set the return address.  For hexagon, the return breakpoint is
     always at BP_ADDR.  */
  regcache_cooked_write_unsigned (regcache, REG_LR, bp_addr);

  /* Finally, update the SP register.  */
  regcache_cooked_write_unsigned (regcache, REG_SP, sp);

  return sp;
}

static void
hexagon_store_return_value (struct type *type, struct regcache *regcache,
                        const void *valbuf)
{
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
hexagon_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  CORE_ADDR pc;
  if (hexagon_debug == 1)
  {
      printf_filtered ("HEXAGON_DEB: %s\n",__func__);
  }

  pc = frame_unwind_register_unsigned (next_frame, REG_PC);
  if (hexagon_debug == 1)
  {
      printf_filtered ("\tHEXAGON_DEB: pc = 0x%x\n",pc);
  }
  return pc;
}

/* Given a GDB frame, determine the address of the calling function's
   frame.  This will be used to create a new GDB frame struct.  */

static void
hexagon_frame_this_id (struct frame_info *this_frame,
		    void **this_prologue_cache, struct frame_id *this_id)
{
  struct hexagon_unwind_cache *info
    = hexagon_frame_unwind_cache (this_frame, this_prologue_cache);
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
hexagon_frame_prev_register (struct frame_info *this_frame,
			  void **this_prologue_cache,
			  int regnum)
{
  struct hexagon_unwind_cache *info
    = hexagon_frame_unwind_cache (this_frame, this_prologue_cache);

  return trad_frame_get_prev_register (this_frame, info->saved_regs, regnum);

}

static const struct frame_unwind hexagon_frame_unwind = {
  NORMAL_FRAME,
  hexagon_frame_this_id,
  hexagon_frame_prev_register,
  NULL,
  default_frame_sniffer
};


static CORE_ADDR
hexagon_frame_base_address (struct frame_info *next_frame, void **this_cache)
{
  struct hexagon_unwind_cache *info
    = hexagon_frame_unwind_cache (next_frame, this_cache);
  return info->base;
}

static const struct frame_base hexagon_frame_base = {
  &hexagon_frame_unwind,
  hexagon_frame_base_address,
  hexagon_frame_base_address,
  hexagon_frame_base_address
};

static CORE_ADDR
hexagon_unwind_sp (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  return frame_unwind_register_unsigned (next_frame, REG_SP);
}

/* Assuming NEXT_FRAME->prev is a dummy, return the frame ID of that
   dummy frame.  The frame ID's base needs to match the TOS value
   saved by save_dummy_frame_tos(), and the PC match the dummy frame's
   breakpoint.  */

static struct frame_id
hexagon_dummy_id (struct gdbarch *gdbarch, struct frame_info *this_frame)
{
  CORE_ADDR pc = get_frame_register_unsigned (this_frame, REG_PC);
  CORE_ADDR sp = get_frame_register_unsigned (this_frame, REG_SP);
  return frame_id_build (sp, pc);
#if 0
  return frame_id_build (hexagon_unwind_sp (gdbarch, this_frame),
			 get_frame_pc (this_frame));
#endif
}


static struct gdbarch *
hexagon_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *var;

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
    case bfd_mach_hexagon_v2:
      hexagon_Version = HEXAGON_V2;
      threadRegSetInfo = &threadRegSetInfo_v2[0];
      globalRegSetInfo = &globalRegSetInfo_v2[0];
      hexagon_RegOffset = &hexagon_reg_offset;
    break;
    
    case bfd_mach_hexagon_v3:
      hexagon_Version = HEXAGON_V3;
      threadRegSetInfo = &threadRegSetInfo_v3[0];
      globalRegSetInfo = &globalRegSetInfo_v3[0];
      hexagon_RegOffset = &hexagon_reg_offset;
    break;

    case bfd_mach_hexagon_v4:
      hexagon_Version = HEXAGON_V4;
      threadRegSetInfo = &threadRegSetInfo_v4[0];
      globalRegSetInfo = &globalRegSetInfo_v4[0];
      hexagon_RegOffset = &hexagon_reg_offset_v4;
    break;
    
    default:
      /* Never heard of this variant.  */
      return 0;
    }


  hexagon_ArgRegMax = 6;
  hexagon_reg_index_max = hexagon_index_max (threadRegSetInfo);
  
  /* Select the right tdep structure for this variant (hexagon_).  */
  var = new_variant ();
  switch (info.bfd_arch_info->mach)
    {
    case bfd_mach_hexagon:
    case bfd_mach_hexagon_v2:
    case bfd_mach_hexagon_v3:
    case bfd_mach_hexagon_v4:
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


  set_gdbarch_num_regs (gdbarch, hexagon_reg_index_max+1);
  set_gdbarch_num_pseudo_regs (gdbarch, hexagon_num_pseudo_regs);

  set_gdbarch_sp_regnum (gdbarch, REG_SP);
  set_gdbarch_deprecated_fp_regnum (gdbarch,  REG_FP);
  set_gdbarch_pc_regnum (gdbarch, REG_PC);

  set_gdbarch_register_name (gdbarch, hexagon_register_name);
  set_gdbarch_register_type (gdbarch, hexagon_register_type);
  //set_gdbarch_register_sim_regno (gdbarch, hexagon_register_sim_regno);

  set_gdbarch_skip_prologue (gdbarch, hexagon_skip_prologue);
  set_gdbarch_breakpoint_from_pc (gdbarch, hexagon_breakpoint_from_pc);

  set_gdbarch_adjust_breakpoint_address (gdbarch, hexagon_gdbarch_adjust_breakpoint_address);

  set_gdbarch_push_dummy_code (gdbarch, hexagon_push_dummy_code);

  set_gdbarch_return_value (gdbarch, hexagon_return_value);

  /* Frame stuff.  */
  set_gdbarch_unwind_pc (gdbarch, hexagon_unwind_pc);
  set_gdbarch_unwind_sp (gdbarch, hexagon_unwind_sp);
  set_gdbarch_frame_align (gdbarch, hexagon_frame_align);

  frame_unwind_append_unwinder (gdbarch, &hexagon_frame_unwind);

  frame_base_set_default (gdbarch, &hexagon_frame_base);

  /* Settings for calling functions in the inferior.  */
  set_gdbarch_push_dummy_call (gdbarch, hexagon_push_dummy_call);
  set_gdbarch_dummy_id (gdbarch, hexagon_dummy_id);

  /* Settings that should be unnecessary.  */
  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);

  var->num_hw_watchpoints = 0;
  var->num_hw_breakpoints = 0;

  
  /* set the disassembler */
  set_gdbarch_print_insn (gdbarch, hexagon_get_disassembler_from_mach
		                  (info.bfd_arch_info->mach,
                                   (info.byte_order == BFD_ENDIAN_BIG)));

  /* Hook in ABI-specific overrides, if they have been registered.  */
  gdbarch_init_osabi (info, gdbarch);

  /* Bug Fix for bug1316
     Changed CALL_DUMMY_LOCATION from default AT_ENTRY to ON_STACK
   */  
  //set_gdbarch_call_dummy_location (gdbarch, ON_STACK);

  return gdbarch;
}

void
_initialize_hexagon_tdep (void)
{
  register_gdbarch_init (bfd_arch_hexagon, hexagon_gdbarch_init);
}
