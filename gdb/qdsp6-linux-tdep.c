/* Hexagon GNU/Linux native support.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "defs.h"
#include "osabi.h"

#include "solib-svr4.h"
#include "symtab.h"

#include "glibc-tdep.h"
#include "gdbarch.h"
#include "frame.h"
#include "target.h"
#include "breakpoint.h"
#include "qdsp6-tdep.h"

static int qdsp6_linux_debug = 0;

/* These 2 bits are set when the packet needs to be parsed */
#define PARSE_BITS 0xC000

/*******************************************************************
 *   Branch Infomation                                             *
 *******************************************************************/   
typedef enum BR_TYPE
{
  NOT_BR = 0,
  PC_ABS = 1,
  PC_REL = 2 
} branch_type;
static const char *branch_type_name[3] = {"No branch", "Absolute", "Relative"};

struct branch_info
{
  branch_type  brtype; 
  unsigned int opc;  /* opcode */
  unsigned int imm;  /* address destination */
  unsigned int reg;  /* register holding address destination */
  unsigned int pred; /* predicate selector */
   
} branch_insn[] = 
{{PC_ABS, 0x50a00000, 0x00000000, 0x001f0000, 0},    /* callr Rs32 */
 {PC_ABS, 0x51000000, 0x00000000, 0x001f0000, 0x300},/* if Pu4 callr */
 {PC_ABS, 0x51200000, 0x00000000, 0x001f0000, 0x300},/* if !Pu4 callr */
 {PC_REL, 0x5a000000, 0x01ff3ffe, 0x00000000, 0},    /* call # */
 {PC_REL, 0x5d000000, 0x00df20fe, 0x00000000, 0x300},/* if Pu4 call # */
 {PC_REL, 0x5d200000, 0x00df20fe, 0x00000000, 0x300},/* if !Pu4 call # */
 {PC_ABS, 0x52800000, 0x00000000, 0x001f0000, 0},    /* jumpr */
 {PC_ABS, 0x53400000, 0x00000000, 0x001f0000, 0x300},/* if Pu4 jumpr */
 {PC_ABS, 0x53600000, 0x00000000, 0x001f0000, 0x300},/* if !Pu4 jumpr */
 {PC_REL, 0x58000000, 0x01ff3ffe, 0x00000000, 0},    /* jump */
 {PC_REL, 0x5c000000, 0x00df20fe, 0x00000000, 0x300},/* if Pu4 jump #*/
 {PC_REL, 0x5c000800, 0x00df20fe, 0x00000000, 0x300},/* if Pu4.new jump:nt # */
 {PC_REL, 0x5c001800, 0x00df20fe, 0x00000000, 0x300},/* if Pu4.new jump:t # */
 {PC_REL, 0x5c200000, 0x00df20fe, 0x00000000, 0x300},/* if !Pu4 jump # */
 {PC_REL, 0x5c200800, 0x00df20fe, 0x00000000, 0x300},/* if !Pu4.new jump:nt # */
 {PC_REL, 0x5c201800, 0x00df20fe, 0x00000000, 0x300},/* if !Pu4.new jump:t # */
 {0},
};

struct loop_info
{
  unsigned int opc; /* opcode */
  unsigned int top; /* pc-rel top of loop */
  unsigned int cnt; /* loop cnt, immediat */
  unsigned int reg; /* register holding loop cnt */
   
} loop_insn[] = 
{{0x60000000, 0x00001f18, 0x00000000, 0x001f0000},/* loop0 (#, r) */
 {0x60200000, 0x00001f18, 0x00000000, 0x001f0000},/* loop1 (#, r) */
 {0x69000000, 0x00001f18, 0x001f00e3, 0x0},/* loop0 (#, #i) */
 {0x69100000, 0x00001f18, 0x001f00e3, 0x0},/* loop1 (#, #i) */
 {0},
};

/* function: immed_r22
   description:
        - call/jump 22
*/
#define IM24_16 0x01ff0000
#define IM13_1  0x00003fff
#define M22 1U << (22 - 1)
static inline CORE_ADDR
immed_r22 (unsigned int val)
{
  CORE_ADDR ret;
  unsigned int a, b;
  a = (val & IM24_16) >> 3;
  b = (val & IM13_1) >> 1;
  ret = (a | b) << 2;

  if (ret & M22)
    {
      ret = (ret |  ~0x7fffff);
    }

  return ret;
}

/* function: immed_r15
   description:
	- jump with a 15bit immediate
	- the immediate 15 bits are sprinkled around the insn.
*/
#define IM23_22 0x00c00000
#define IM20_16 0x001f0000
#define IM13    0x00002000
#define IM7_1   0x000000fe
#define M15   1U << (15 - 1)
static inline CORE_ADDR
immed_r15 (unsigned int val)
{
  CORE_ADDR ret;
  unsigned int a, b, c, d;
  a = (val & IM23_22) >> 9;
  b = (val & IM20_16) >> 8;
  c = (val & IM13) >> 6;
  d = (val & IM7_1) >> 1;

  ret = (a | b | c | d) << 2;

  if (ret & M15)
    {
      ret = (ret | ~0xffff);
    }

  return ret;
}

/* function: is_loop
   description:
	- Determine if the given instruction is a loop.
	- Return the address of the top of the loop and the count.
	- XXX_SM this function currently isn't called but keeping around
	  just in case.
 */
static int
is_loop (unsigned int insn, CORE_ADDR *top,
	 unsigned int *immediate, int *regnum)
{
  unsigned int inopc;
  int i = 0;

  inopc = insn;
  inopc &= ~PARSE_BITS;		/* Strip the parse bits if present */

  *immediate = -1;
  *regnum    = -1;

  while (loop_insn[i].opc)
    {
      unsigned int wopc = inopc & ~(loop_insn[i].top |
				    loop_insn[i].cnt |
				    loop_insn[i].reg);

      if ((loop_insn[i].opc ^ wopc) == 0x0)
	{
	  if (inopc & 1<<27)  /* Immediate version */
	    {
		*immediate = (inopc & 0x3);
		*immediate |= (inopc & 0xe0) >>3;
		*immediate |= (inopc & 0x1f0000) >>16;
	    }
	  else /* Register version */
	    {
		*regnum = (inopc & 0x1f0000) >> 16;
	    }

	  *top = (inopc & 0x18) >> 3;
printf ("*top=0x%x\n", *top);
	  *top |= (inopc & 0x1f00) >> 6;
printf ("*top=0x%x\n", *top);

	  return 1;
	}
      i++;
    }

    return 0;
}


/* function: is_end_loop
   description:
	- return 0 if end of loop bit P=10b (bit #15 only) is not set
	- read lc0/1 sa0/1 to determine the top of the loop.
	- This code assumes the documented convention, that loop0 is always
	  the inner loop and loop1 is always the output loop as documented
	  in the Hexagon Programmers Handbook is followed.
 */
static int
is_end_loop(struct gdbarch *gdbarch, struct frame_info *frame,
	    unsigned int insn, CORE_ADDR *target)
{
  int rc = 0;

  if ((insn & PARSE_BITS) == 0x8000)
    {
      gdb_byte buf[4];
      unsigned int sa0,sa1;
      unsigned int lc0,lc1;
      enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);

      get_frame_register_bytes (frame, REG_SA0, 0, 4, buf);
      sa0 = extract_unsigned_integer (buf, 4, byte_order) & -4;
      get_frame_register_bytes (frame, REG_LC0, 0, 4, buf);
      lc0 = extract_unsigned_integer (buf, 4, byte_order);

      get_frame_register_bytes (frame, REG_SA1, 0, 4, buf);
      sa1 = extract_unsigned_integer (buf, 4, byte_order) & -4;
      get_frame_register_bytes (frame, REG_LC1, 0, 4, buf);
      lc1 = extract_unsigned_integer (buf, 4, byte_order);

      if (qdsp6_linux_debug)
	{
	  printf_filtered ("end loop: sa0=0x%x, lc0=0x%x\n", sa0, lc0);
	  printf_filtered ("end loop: sa1=0x%x, lc1=0x%x\n", sa1, lc1);
	}

      if (lc0 > 1)
	{
	  *target = sa0;
	  rc = 1;
	}

      else if (lc1 > 1)
	{
	  *target = sa1;
	  rc = 1;
	}
    }
  return rc;
}

/* function: is_branch
   description:
	- Determine if the given instruction is a branch.
*/
static branch_type
is_branch (unsigned int insn, CORE_ADDR *offset, int *reg)
{
  unsigned int inopc;
  int i = 0;

  *reg = -1;
  inopc = insn;

  inopc &= ~PARSE_BITS;		/* Strip the parse bits if present */
  while (branch_insn[i].opc)
    {
      unsigned int wopc = inopc & ~(branch_insn[i].imm |
				    branch_insn[i].reg |
				    branch_insn[i].pred);

      if ((branch_insn[i].opc ^ wopc) == 0x0)
	{
	  if (branch_insn[i].reg == 0x0)
	    {
	      unsigned int immed = inopc & branch_insn[i].imm;
	      *reg = -1;

	      if (branch_insn[i].imm & 0x01000000)
		  *offset = immed_r22 (immed);
	      else
		  *offset = immed_r15 (immed);

	      if (qdsp6_linux_debug)
		{
		  printf_filtered ("IMMED match 0x%x\n", branch_insn[i].opc);
		  printf_filtered ("IMMEDIATE value 0x%x\n", immed);
		  printf_filtered ("offset 0x%x\n", *offset);
		}
	    }
	  else
	    {
	      int regnum = (inopc & 0x1f0000) >> 16;
	      *reg = regnum;
	      if (qdsp6_linux_debug)
		{
		  printf_filtered ("REG match 0x%x, register = %d\n",
			  branch_insn[i].opc, regnum);
		}
	    }
	  return branch_insn[i].brtype;
	}

      i++;
    }

  return 0;
}

/* function: handle_branches
   description:
	- Check for branches and return destinations for each branch
 */
static void
handle_branches(struct gdbarch *gdbarch,
                struct frame_info *frame,
                struct packet *packet)
{
  int i;
  branch_type brtype;
  int reg;
  gdb_byte buf[4];
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  CORE_ADDR next_pc;
  CORE_ADDR offset = 0ull;
  CORE_ADDR target = 0ull;
  CORE_ADDR top;
  unsigned int count;

  next_pc = packet->addr[packet->count-1];
  for (i=0; i<packet->count; i++)
    {
      if (qdsp6_linux_debug == 1)
	printf_filtered ("packet[%d] = 0x%llx:0x%08x\n",
			  i, packet->addr[i], packet->insn[i]);

        if ((brtype = is_branch (packet->insn[i], &offset, &reg)) != NOT_BR)
        {

	  if (qdsp6_linux_debug == 1)
	    {
	      printf_filtered ("Branch found: %s\n", branch_type_name[brtype]);
	      if (brtype == PC_REL)
	        printf_filtered ("Branch dest = 0x%llx\n",
				  packet->addr[0]+offset);
	      else
	        printf_filtered ("Branch dest = 0x%llx\n", offset);
	    }
  
	  if (reg == -1) /* immediate insn */
            target = offset;
	  else
	    {
              get_frame_register_bytes (frame, reg, 0, 4, buf);
              target = extract_unsigned_integer (buf, 4, byte_order) & -4;
	    }
  
	  if (brtype == PC_REL)
            target = (packet->addr[0] + target);
  
          if (target != next_pc)
            insert_single_step_breakpoint (gdbarch, target);
        }

      else if (is_end_loop(gdbarch, frame, packet->insn[i], &target))
	{
	  if (qdsp6_linux_debug == 1)
	      printf_filtered ("End loop found: top of loop = 0x%x\n",
				target);

/*
   This check keeps us from splitting up packets, if the address lc[0/1] is
   in the packet we are currently processing then we must avoid setting
   a bp because it would break up the packet.
 */
	  if (target != packet->addr[0])
            insert_single_step_breakpoint (gdbarch, target);
        }

    }
}


/* function: qdsp6_software_single_step
   description:
	- Hexagon V2-3 do not support hardware single step this must be dealt
   	  with here
*/
int
qdsp6_software_single_step (struct frame_info *frame)
{
  CORE_ADDR pc, next_pc, lim_pc; 
  unsigned int insn;
  unsigned int offset, reg;
  gdb_byte buf[4];
  branch_type brtype;
  struct gdbarch *gdbarch = get_frame_arch (frame);
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  int insn_count = 0;
  struct packet pkt = { count:0 };


  pc = get_frame_pc (frame);
  lim_pc = pc + 0x10;

/* This will examine the insn at the current PC and increment the PC until
   it finds and insn with the PARSE_BITS set.  This keeps packets atomic.
 */
  do
    {
      if (target_read_memory (pc, buf, 4))
	{
	  internal_warning (__FILE__, __LINE__,
			    _("\nread_memory failed to read address: 0x%llx"),
                            pc);
	  return 0;
	}

      insn = extract_unsigned_integer (buf, 4, byte_order);


      pkt.insn[insn_count] = insn;
      pkt.addr[insn_count] = pc;

      insn_count++;
      pkt.count++;

      if ((insn & PARSE_BITS) != PARSE_BITS)
	pc+=4;
    }
  while (((insn & PARSE_BITS) != PARSE_BITS) && (pc < lim_pc));

  next_pc = pc+4;
  insert_single_step_breakpoint (gdbarch, next_pc);

/* This will look at the packet and may put an additional breakpoint at the
   destination point if a branch is present.
 */
  handle_branches(gdbarch, frame, &pkt);

  return 1;
}

/* function: qdsp6_linux_init_abi
   description:
	- Generic GNU/Linux ABI settings
*/
extern CORE_ADDR
qdsp6_push_dummy_call (struct gdbarch *gdbarch, struct value *function,
                     struct regcache *regcache, CORE_ADDR bp_addr,
                     int nargs, struct value **args, CORE_ADDR sp,
		     int struct_return, CORE_ADDR struct_addr);
static void
qdsp6_linux_init_abi (struct gdbarch_info info, struct gdbarch *gdbarch)
{
  set_gdbarch_software_single_step (gdbarch, qdsp6_software_single_step);
  set_gdbarch_push_dummy_call (gdbarch, qdsp6_push_dummy_call);
  set_gdbarch_decr_pc_after_break (gdbarch, 4);
}

/* function: qdsp6_linux_init_abi_v2
   description:
	- Hexagon V2 specific ABI settings
*/
static void
qdsp6_linux_init_abi_v2 (struct gdbarch_info info, struct gdbarch *gdbarch)
{
    qdsp6_linux_init_abi(info, gdbarch); 
}

/* function: qdsp6_linux_init_abi_v3
   description:
	- Hexagon V3 specific ABI settings
*/
static void
qdsp6_linux_init_abi_v3 (struct gdbarch_info info, struct gdbarch *gdbarch)
{
    qdsp6_linux_init_abi(info, gdbarch); 
}


static enum gdb_osabi
qdsp6_linux_elf_osabi_sniffer (bfd *abfd)
{
  return GDB_OSABI_LINUX;
}


/* function: _initialize_qdsp6_linux_tdep
   description:
	Hexagon V2 and V3 (and beyond) have different ABI's.
	    - V2    : Callee Saved r24-r27 and r29, r30, r31
	    - V3/V4 : Callee Saved r16-r27 and r29, r30, r31
	    - V4    : Support hardware single step, yay!
	Registers 0-5 still are used for function arguments
*/
void
_initialize_qdsp6_linux_tdep (void)
{
  gdbarch_register_osabi_sniffer (bfd_arch_qdsp6,
				  bfd_target_elf_flavour,
				  qdsp6_linux_elf_osabi_sniffer);

  gdbarch_register_osabi (bfd_arch_qdsp6, bfd_mach_qdsp6_v2, GDB_OSABI_LINUX,
			  qdsp6_linux_init_abi_v2);

  gdbarch_register_osabi (bfd_arch_qdsp6, bfd_mach_qdsp6_v3, GDB_OSABI_LINUX,
			  qdsp6_linux_init_abi_v3);
}
