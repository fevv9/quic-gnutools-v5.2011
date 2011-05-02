/* Hexagon SOS simulator remote support.

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
#include "gdbcore.h"
#include "regcache.h"
#include "osabi.h"
#include <sys/procfs.h>

#include "solib-svr4.h"
#include "symtab.h"

#include "glibc-tdep.h"
#include "gdbarch.h"
#include "frame.h"
#include "target.h"
#include "breakpoint.h"


/* function: hexagon_sos_init_abi
   description:
        - Generic SOS ABI settings
*/
static void
hexagon_sos_init_abi (struct gdbarch_info info, struct gdbarch *gdbarch)
{

  /* Linux uses SVR4-style shared libraries.  */
  set_gdbarch_skip_trampoline_code (gdbarch, find_solib_trampoline_target);
  set_solib_svr4_fetch_link_map_offsets
    (gdbarch, svr4_ilp32_fetch_link_map_offsets);

  /* Linux uses the dynamic linker included in the GNU C Library.  */
  set_gdbarch_skip_solib_resolver (gdbarch, glibc_skip_solib_resolver);

}

/* function: hexagon_sos_init_abi_v2
   description:
        - Hexagon V2 specific ABI settings
*/
static void
hexagon_sos_init_abi_v2 (struct gdbarch_info info, struct gdbarch *gdbarch)
{
    hexagon_sos_init_abi(info, gdbarch); 
}

/* function: hexagon_sos_init_abi_v3
   description:
        - Hexagon V3 specific ABI settings
*/
static void
hexagon_sos_init_abi_v3 (struct gdbarch_info info, struct gdbarch *gdbarch)
{
    hexagon_sos_init_abi(info, gdbarch); 
}

/* function: hexagon_sos_init_abi_v4
   description:
        - Hexagon V3 specific ABI settings
*/
static void
hexagon_sos_init_abi_v4 (struct gdbarch_info info, struct gdbarch *gdbarch)
{
    hexagon_sos_init_abi(info, gdbarch); 
}


static enum gdb_osabi
hexagon_sos_elf_osabi_sniffer (bfd *abfd)
{
  return GDB_OSABI_HEXAGON_SOS;
}

void
_initialize_hexagon_sos_tdep (void)
{
  gdbarch_register_osabi_sniffer (bfd_arch_hexagon,
                                  bfd_target_elf_flavour,
                                  hexagon_sos_elf_osabi_sniffer);

  gdbarch_register_osabi (bfd_arch_hexagon,
                          bfd_mach_hexagon_v2, GDB_OSABI_HEXAGON_SOS,
                          hexagon_sos_init_abi_v2);

  gdbarch_register_osabi (bfd_arch_hexagon,
                          bfd_mach_hexagon_v3, GDB_OSABI_HEXAGON_SOS,
                          hexagon_sos_init_abi_v3);

  gdbarch_register_osabi (bfd_arch_hexagon,
                          bfd_mach_hexagon_v4, GDB_OSABI_HEXAGON_SOS,
                          hexagon_sos_init_abi_v4);
}
