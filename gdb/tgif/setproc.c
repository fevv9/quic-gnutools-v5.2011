/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/*
 * Steps to add a tcl command here ::
 * 1. Add the command line "command" and the tcl function that implements
 *    the command in the struct record {}AddCmds;
 * 2. Declare the implemented function before struct record - to placate
 *    the compiler.
 * 3. Implement the tcl command....
 *
 * Sudhir Halbhavi - started coding Sept 04, 1995, on a dark stormy night...
 */

#include "defs.h"
#include "symtab.h"
#include "gdbtypes.h"
#include "value.h"
#include "gdbcore.h"
#include "frame.h"
#include "command.h"
#include "gdbcmd.h"
#include "target.h"
#include "expression.h"

#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include <errno.h>
#include "tgif.h"

#ifdef HAVETCL
int setproc_command_from_tcl (int bnum, char *proc, long procpoint);
int get_number (char **pp);
#endif
/*
 * These are the new Tcl commands to be added at TgifInit time.
 * These commands add for the Tcl scripting ability and adds functionality
 * to debug programs.
 */

/*
 * Sets a Tcl procedure at a breakpoint - basically does the same as
 * what "commands_command" does, except that the "command" is the name
 * of the Tcl script.
 * Script Usage : setproc <bpnum> <tcl-script-name>
 *		  Will set tcl-script-name to be executed at
 *		  <bpnum> everytime breakpoint <bpnum> is hit.
 */
int
Tgif_SetprocCmd(ClientData clientdata, Tcl_Interp *interp, int argc,
		char *argv[]) {
  int bpnum;
  char *errmsg;
  char *proc;

  if (argc < 3) {
    interp->result = "wrong #args, Usage: setproc <bpnum> <procedure>";
    return TCL_ERROR;
  }

  /*
   * Concatenate args for tcl procedure.
   */
  proc = Tcl_Concat ((argc - 2), &argv[2]);

  errmsg = (char *)malloc(10000 * sizeof(char));
  
  if(*argv[1] == '$')
      bpnum = get_number (&argv[1]); /* parses convenience var to get bpnum */
  else      
      bpnum = atoi (argv[1]);
  /*
   * call setproc command in breakpoint.c - will set proc to be
   * the "command" at that breakpoint.
   */
{
long procpoint;
  procpoint = (long) proc;

  if ( setproc_command_from_tcl (bpnum, proc, procpoint) ) {
    sprintf (errmsg, "No breakpoint number %d", bpnum);
    interp->result = errmsg;
    free (proc);
    return TCL_ERROR;
  }
  else
    return TCL_OK;
 }

}

