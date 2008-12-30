#include "defs.h"
#include "frame.h"
#include "frame-unwind.h"
#include "frame-base.h"
#include "trad-frame.h"
#include "gdbcmd.h"
#include "gdbcore.h"
#include "gdbtypes.h"
#include "inferior.h"
#include "symfile.h"
#include "arch-utils.h"
#include "regcache.h"
#include "gdb_string.h"
#include "dis-asm.h"

struct gdbarch_tdep
{
    int foo;
};

static struct gdbarch *
qdsp6_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *tdep;

  /* Find a candidate among the list of pre-declared architectures. */
  arches = gdbarch_list_lookup_by_info (arches, &info);
  if (arches != NULL)
    return arches->gdbarch;

  /* None found, create a new architecture from the information provided. */
  tdep = XMALLOC (struct gdbarch_tdep);
  gdbarch = gdbarch_alloc (&info, tdep);

  /* If we ever need to differentiate the device types, do it here. */
  switch (info.bfd_arch_info->mach)
    {
    case bfd_mach_qdsp6_v2:
    case bfd_mach_qdsp6_v3:
    case bfd_mach_qdsp6_v4:
      break;
    }

  return gdbarch;
}

void
_initialize_qdsp6_tdep (void)
{
  register_gdbarch_init (bfd_arch_qdsp6, qdsp6_gdbarch_init);

}
