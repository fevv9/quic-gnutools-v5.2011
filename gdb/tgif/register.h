/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
#ifndef __REGISTER_H
#define __REGISTER_H
/*
 *
 * File: register.h
 *
 * Purpose: This file contains definitions for the registration
 *          of gdb commands with the tcl interpreter.
 *
 */
#define NAMESIZE 100
typedef struct {
	int type;
	char old_name[NAMESIZE];
	char new_name[NAMESIZE];
	void (*pre_processing) (void);
	void (*post_processing)(void);
} cmd_struct;


void RegisterAllCmds (Tcl_Interp * interp);

char *Tgif_Concat (int argc, char **argv);
#endif
