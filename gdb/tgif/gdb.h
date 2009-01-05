/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
#ifndef __GDB_H
#define __GDB_H
/*
 *
 * File: gdb.h
 *
 * Purpose: This file contains headers to allow us to 
 *          use gdb functions.
 *
 */
extern int execute_command( char * buffer, int from_tty);

void non_tcl_execute_command (char *p, int from_tty);
#endif
