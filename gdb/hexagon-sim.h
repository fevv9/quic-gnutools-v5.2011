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

#if !defined (HEXAGON_SIM_H)
#define HEXAGON_SIM_H 1

static __inline__ int
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

static __inline__ int
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
        *bin++ = fromhex (hex[0]) * 16 + fromhex (hex[1]);
        hex += 2;
    }
    return i;
}

#define THIS_TARGET() \
	(current_hexagon_target ? 					\
		(strcmp(current_hexagon_target, TARGET_NAME) == 0)	\
		:						\
		0)

#define SIM_ARG_MAX 256

#endif /* HEXAGON_SIM_H */
