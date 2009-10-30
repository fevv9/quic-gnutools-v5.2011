/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/*
 *
 * File: register.c
 *
 * Purpose: This file contains methods to register all
 *          the gdb commands with the tcl interpreter.
 *
 */

#include <string.h>
#include <stdio.h>
#include "tcl.h"
#include <tclInt.h>
#include <errno.h>
#include <stdlib.h>
#include "tgif.h"
#include "../defs.h" /* TEST */
#include <sys/types.h>
#include <sys/time.h>

#include "register.h"
#include "hooks.h"

#define HARDSTATUS 1

/* Externs Section */
extern int cmd_result_flag;
extern int tcl_getInteractive();

/* Global Variable Section */
/* Concat function is specified here. */
char *catBuffer    = 0;
int  catBufferSize = 0;
int  orig_from_tty = HARDSTATUS;

/*
 *
 * Function: Tgif_ProcessGDB
 *
 * Purpose: This function is called by the tcl interpreter
 *          to process a gdb command.
 *
 */
int
Tgif_ProcessGDB (ClientData clientdata, Tcl_Interp *interp,
		 int argc, char * argv[]) {

	char *gdbCmd;
	cmd_struct * tmp = (cmd_struct *)clientdata;

    // printf("\nIn Tgif_ProcessGDB.\n");
	if (tmp->type == 2) {
		argv [0] = tmp->old_name;
	} else {
		argv[0] = tmp->new_name;
	}

	gdbCmd   = Tgif_Concat (argc, argv);

	if (tmp->pre_processing) {
		(*tmp->pre_processing)();
	}

	if (!need_concatenation)
       inset_into_buffer = tcl_status_buffer;

    /* Check if we need to suppress gdb's display */
    //if(tcl_getInteractive() != 0)
    //    setInteractive(0);

	non_tcl_execute_command (gdbCmd, orig_from_tty);
    if ( inset_into_buffer != tcl_status_buffer )
       Tcl_SetResult( interp, tcl_status_buffer, TCL_VOLATILE );


	if (tmp->post_processing) {
		(*tmp->post_processing)();
	}
    //printf("\nOut Tgif_ProcessGDB.\n");
	return cmd_result_flag;
}

void UpdateProceed (void) {
	was_proceed_command = 1;
}


#define TYPE1
cmd_struct type1_cmds[] = {
	#include "cmdlist.h"
	{0,"foo","bar",(void *)0x0, (void *)0x0}
};
#undef TYPE1

#define TYPE2
cmd_struct type2_cmds[] = {
	#include "cmdlist.h"
	{0,"foo","foo",(void *)0x0, (void *)0x0}
};
#undef TYPE2

#define TYPE3
cmd_struct type3_cmds[] = {
	#include "cmdlist.h"
	{0,"foo","foo",(void*)0x0, (void*)0x0}
};
#undef TYPE3

void RenameDuplicateTclCmds (Tcl_Interp * interp)
{

    Tcl_Eval(interp, "rename append t_append");
    Tcl_Eval(interp, "rename break t_break");
    Tcl_Eval(interp, "rename catch t_catch");
    Tcl_Eval(interp, "rename continue t_continue");
    // Bug 1695 fix - remove t_exit
    //Tcl_Eval(interp, "rename exit t_exit");
    Tcl_Eval(interp, "rename file t_file");
    Tcl_Eval(interp, "rename info t_info");
    Tcl_Eval(interp, "rename list t_list");
    Tcl_Eval(interp, "rename load t_load");
    Tcl_Eval(interp, "rename return t_return");
    Tcl_Eval(interp, "rename set t_set");
    Tcl_Eval(interp, "rename source t_source");
    Tcl_Eval(interp, "rename trace t_trace");
    Tcl_Eval(interp, "rename trace t_unset");
}

void RegisterAllCmds (Tcl_Interp * interp) {
	int i = 0;

    RenameDuplicateTclCmds (interp);

	while (type1_cmds[i].type) {
		Tcl_CreateCommand (interp, type1_cmds[i].new_name, Tgif_ProcessGDB,
                                   (ClientData *)& type1_cmds[i],
                                   (Tcl_CmdDeleteProc *) NULL);
		i++;
	}

	i = 0;
	while (type2_cmds[i].type) {
				Tcl_CreateCommand (interp, type2_cmds[i].new_name, Tgif_ProcessGDB,
                                   (ClientData *) &type2_cmds[i],
                                   (Tcl_CmdDeleteProc *) NULL);
		i++;
	}
	i = 0;
	while (type3_cmds[i].type) {
				Tcl_CreateCommand (interp, type3_cmds[i].new_name, Tgif_ProcessGDB,
                                   (ClientData *) &type3_cmds[i],
                                   (Tcl_CmdDeleteProc *) NULL);
		i++;
	}
}


char *Tgif_Concat (int argc, char **argv) {
	int  i, catLen;
	char *dest;
	if (!catBuffer) {
		catBuffer     = (char *) xmalloc (INIT_FILTER_SIZE);
		catBufferSize = INIT_FILTER_SIZE;
	}

	for (i = 0, catLen = argc*2; i < argc; i++)
		catLen += strlen (argv[i]);

	if (catLen > catBufferSize) {
		free (catBuffer);
		catBuffer     = (char *) xmalloc (catLen);
		catBufferSize = catLen;
	}
	dest = catBuffer;

	for (i =0; i < argc; i++) {
		char *src = argv[i];

		while (*src) {
			*dest = *src;
			src ++; dest ++;
		}
		*dest = ' '; dest++;
	}

	if (argc) dest--;
	*dest = 0;
	return (catBuffer);
}
