
/* BFD support for the QDSP6 processor
   Copyright 1994, 1995, 1997, 2001, 2002 Free Software Foundation, Inc.
   Contributed by Doug Evans (dje@cygnus.com).

This file is part of BFD, the Binary File Descriptor library.

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"

static const bfd_arch_info_type *qdsp6_bfd_compatible
  (const bfd_arch_info_type *a, const bfd_arch_info_type *b);

#define QDSP6(mach, print_name, default_p, next) \
  {					 \
    32,	/* 32 bits in a word  */	 \
    32,	/* 32 bits in an address  */	 \
    8,	/* 8 bits in a byte  */		 \
    bfd_arch_qdsp6,			 \
    mach,				 \
    "qdsp6",				 \
    print_name,				 \
    4, /* section alignment power  */	 \
    default_p,				 \
    qdsp6_bfd_compatible,		 \
    bfd_default_scan,			 \
    next,				 \
  }

static const bfd_arch_info_type qdsp6_arch_info [] =
{
  QDSP6 (bfd_mach_qdsp6_v3, "qdsp6v3", FALSE, qdsp6_arch_info+1 ),
  QDSP6 (bfd_mach_qdsp6_v4, "qdsp6v4", FALSE, qdsp6_arch_info+2 ),
  QDSP6 (bfd_mach_qdsp6,    "qdsp6",   FALSE, NULL)
};

/* This is the default ISA. */
const bfd_arch_info_type bfd_qdsp6_arch =
  QDSP6 (bfd_mach_qdsp6_v2, "qdsp6v2", TRUE, qdsp6_arch_info+0 );

/* Utility routines.  */
int qdsp6_get_mach PARAMS ((char *));

/** Given CPU type, return its bfd_mach_qdsp6* value.

@param name CPU type.
@return bfd_mach_qdsp6* value if successful or -1 otherwise.
*/
int
qdsp6_get_mach
(char *name)
{
  const bfd_arch_info_type *p;

  for (p = &bfd_qdsp6_arch; p != NULL; p = p->next)
    if (!strcmp (name, p->printable_name))
      return p->mach;

  return -1;
}

static const bfd_arch_info_type *
qdsp6_bfd_compatible
(const bfd_arch_info_type *a, const bfd_arch_info_type *b)
{
  if (a->arch != bfd_arch_qdsp6 || b->arch != bfd_arch_qdsp6)
    return NULL;

  /* V1 is not compatible with anything else. */
  if (   (a->mach == bfd_mach_qdsp6 || b->mach == bfd_mach_qdsp6)
      && a->mach != b->mach)
    return NULL;

  /* V3 uses a new ABI that is not compatible with the V2 ABI. */
  if (   (a->mach == bfd_mach_qdsp6_v2 || b->mach == bfd_mach_qdsp6_v2)
      && a->mach != b->mach)
    return NULL;

  /* Return the most recent one. */
  if (a->mach > b->mach)
    return a;
  else
    return b;
}
