
#ifndef __GUI_SUPP_H
#define __GUI_SUPP_H
/*
 *
 * File: GuiSupport.h
 *
 * Purpose: This file contains the headers for gui support
 *          routines.
 *
 */
int wrap_execute(ClientData clientData, Tcl_Interp * interp,
	      int argc, char* argv[]);

int get_number_levels(ClientData clientData, Tcl_Interp * interp,
		   int argc, char* argv[]);

int
is_complete(ClientData clientData, Tcl_Interp *interp,
	     int argc, char* argv []);

#endif
