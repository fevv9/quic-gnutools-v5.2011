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
#include "inferior.h"
#include "gdbcore.h"
#include "regcache.h"
#include "linux-nat.h"
#include "target.h"
#include "hexagon-tdep.h"

#include "gdb_assert.h"
#include "gdb_string.h"
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/procfs.h>

/* Prototypes for supply_gregset etc.  */
#include "gregset.h"

#include "hexagon-tdep.h"

/* Transfering floating-point registers between GDB, inferiors and cores.  
   Since HEXAGON has no floating-point registers, these functions do nothing.  */

void
supply_fpregset (struct regcache *regcache, const gdb_fpregset_t *fpregs)
{
  internal_error (__FILE__, __LINE__,
		  "Looking for non-existent floating point registers");
}
void
fill_fpregset (const struct regcache *regcache,
	       gdb_fpregset_t *fpregs, int regno)
{
  internal_error (__FILE__, __LINE__,
		  "Cannot fill non-existent floating point registers");
}


/* Fill register REGNO (if it is a general-purpose register) in
   *GREGSETPS with the value in GDB's register array.  If REGNO is -1,
   do this for all registers.  */

void
fill_gregset (const struct regcache *regcache,
	      elf_gregset_t * gregsetp, int regno)
{
  elf_greg_t *regp = (elf_greg_t *) gregsetp;
  int i;
  unsigned long psw, bbpsw, tmp;


}

void
supply_gregset (struct regcache *regcache, const gdb_gregset_t *gregsetp)
{
    int i;
    uint8_t *ptr = (uint8_t *)gregsetp;
    struct user_regs *reg = gregsetp;

/* Registers 0-31 occupy the first 32 entries, followed by special
   purpose registers */
    for (i=0; i<=31; i++)
	regcache_raw_supply (regcache, i, ptr+i*4);


    regcache_raw_supply (regcache, REG_GP, &reg->gp);
    regcache_raw_supply (regcache, REG_UGP, &reg->ugp);
    regcache_raw_supply (regcache, REG_SA0, &reg->sa0);
    regcache_raw_supply (regcache, REG_LC0, &reg->lc0);
    regcache_raw_supply (regcache, REG_SA1, &reg->sa1);
    regcache_raw_supply (regcache, REG_LC1, &reg->lc1);
    regcache_raw_supply (regcache, REG_M0, &reg->m0);
    regcache_raw_supply (regcache, REG_M1, &reg->m1);
    regcache_raw_supply (regcache, REG_P3_0, &reg->p3_0);
    regcache_raw_supply (regcache, REG_PC, &reg->pc);
    regcache_raw_supply (regcache, REG_BADVA, &reg->badva);
}



/* function: hexagon_linux_fetch_inferior_registers
   description:
	- Transferring registers between GDB and inferior.
	- Fetch register REGNO from the child process.  If REGNO is -1, do
   	  this for all registers (including the floating point and SSE
   	  registers).
*/
static void
hexagon_linux_fetch_inferior_registers (struct target_ops *ops,
				      struct regcache *regcache, int regno)
{
  /* Overload thread id onto process id */
  int tid;
  struct user_regs gregset;
  uint8_t *ptr=&gregset;
  CORE_ADDR reg;
  long rc;
  unsigned long tmp[1];
  struct pt_regs ptreg;
  int i;

  /* No thread id, just use process id */
  tid = TIDGET (inferior_ptid);
  if (tid == 0)
    tid = PIDGET (inferior_ptid);

  if (ptrace (PTRACE_GETREGS, tid, 0, (void *) &gregset) < 0)
    {
      perror_with_name (_("Couldn't get general-purpose registers."));
    }

  supply_gregset (regcache, (const gdb_gregset_t *) &gregset);
  return;
}



/* function: hexagon_store_register
	- store a given register
*/
static void
hexagon_store_register (const struct regcache *regcache, int tid, int regno)
{

  struct gdbarch *gdbarch = get_regcache_arch (regcache);
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  int i;
  long rc;
  size_t bytes_to_transfer;
  char buf[4];

  regcache_raw_collect (regcache, regno, buf);

  errno = 0;
  rc = ptrace (PTRACE_POKEUSER, tid, (PTRACE_TYPE_ARG3) regno,
	  *(long *) buf);

  if (errno != 0)
    {
      char message[128];
      sprintf (message, "ptrace error writing register %s (#%d), rc=0x%x",
	       gdbarch_register_name (gdbarch, regno), regno, rc);
      perror_with_name (message);
    }
}


/* function: hexagon_linux_store_inferior_registers
   description:
	- Transferring registers between GDB and inferior.
	- Store register REGNO from the child process.  If REGNO is -1, do
   	  this for all registers (including the floating point and SSE
   	  registers).
*/
static void
hexagon_linux_store_inferior_registers (struct target_ops *ops,
				      struct regcache *regcache, int regno)
{
  /* Overload thread id onto process id */
  int tid;

  /* No thread id, just use process id */
  tid = TIDGET (inferior_ptid);
  if (tid == 0)
    tid = PIDGET (inferior_ptid);

  if (regno >= 0)
    hexagon_store_register (regcache, tid, regno);
}

void 
_initialize_hexagon_linux_nat (void)
{
  struct target_ops *t;

  /* Fill in the generic GNU/Linux methods.  */
  t = linux_target ();

  t->to_fetch_registers = hexagon_linux_fetch_inferior_registers;
  t->to_store_registers = hexagon_linux_store_inferior_registers;

  /* Register the target.  */
  linux_nat_add_target (t);
}

