/* Support Cygwin paths under MinGW.
   Copyright (C) 2006 Free Software Foundation, Inc.
   Written by CodeSourcery.

This file is part of the libiberty library.
Libiberty is free software; you can redistribute it and/or modify it
under the terms of the GNU Library General Public License as published
by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Libiberty is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with libiberty; see the file COPYING.LIB.  If not, write
to the Free Software Foundation, Inc., 51 Franklin Street - Fifth
Floor, Boston, MA 02110-1301, USA.  */

/*****************************************************************
* Copyright (c) $Date$ Qualcomm Innovation Center, Inc.
* All Rights Reserved.
* Modified by Qualcomm Innovation Center, Inc. on $Date$
*****************************************************************/

#include <windows.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <io.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "libiberty.h"
#include "win_realpath.h"

/* Returns the handle for the MVCRT DLL, or NULL if it is not
   available.  */
static HANDLE
msvcrt_dll (void)
{
  static HANDLE dll = INVALID_HANDLE_VALUE;

  /* After we call LoadLibrary, DLL will be either a valid handle or
     NULL, so this check ensures that we only try to load the library
     once.  */
  if (dll == INVALID_HANDLE_VALUE)
    dll = LoadLibrary ("msvcrt.dll");

  return dll;
}


/* Call the underlying MSVCRT fopen with PATH and MODE, and return
   what it returns.  */
static FILE *
msvcrt_fopen (const char *path, const char *mode)
{
  typedef FILE *(fopen_type)(const char *path,  
                             const char *mode); 

  static fopen_type *f = NULL; 

  /* Get the address of "fopen".  */
  if (!f) 
    {
      HANDLE dll = msvcrt_dll ();
      if (!dll)
        {
          errno = ENOSYS; 
          return NULL;
        }
      f = (fopen_type *) GetProcAddress (dll, "fopen");
      if (!f) 
        {
          errno = ENOSYS; 
          return NULL;
        }
    }

  /* Call fopen.  */
  return (*f)(path, mode);
}

FILE *
fopen (const char *path, const char *mode)
{
  FILE *f;
  char win32_path[MAX_PATH + 1];
  char *p = path;

  if (win_realpath (path, win32_path, MAX_PATH+1))
    p = win32_path;

  f = msvcrt_fopen (p, mode);
  return f;
}

int 
open (const char *path, int oflag, ...)
{
  int fd;
  char win32_path[MAX_PATH + 1];
  char *p = path;
  int pmode = 0;

  if ((oflag & _O_CREAT))
    {
      va_list ap;
      va_start (ap, oflag); 
      pmode = va_arg (ap, int); 
      va_end (ap);
    }

  if (win_realpath(path, win32_path, MAX_PATH+1)) 
        p = win32_path;

  return _open(p, oflag, pmode);
}


int
stat (const char *path, struct stat *buffer)
{
  int r;
  char win32_path[MAX_PATH + 1];
  char *p = path;

  if (win_realpath(path, win32_path, MAX_PATH+1))
        p = win32_path;
  r = _stat (p, (struct _stat *) buffer);
  return r;
}

