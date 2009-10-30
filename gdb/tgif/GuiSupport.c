/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/*
 *
 * File: GuiSupport.c
 *
 * Purpose: This file contains methods to support a
 *          tk gui.
 *
 */
#include "tcl.h"
#include "tclInt.h"
#include <stdlib.h>
#include "GuiSupport.h"
#include "tgif.h"


/* -----------------------------------------------------------------------
 *
 * Function: wrap_execute:
 *
 * Purpose: This function is a test command that was written to support a
 *          fake command prompt.
 *
 * -----------------------------------------------------------------------
 */
int
wrap_execute( ClientData clientData, Tcl_Interp * interp,
	      int argc, char* argv[])
{
  int i, res;
  char * buffer;
  buffer = (char*)malloc(1000);
  if ( buffer == NULL ) {
     return TCL_ERROR;
  }
  strcpy( buffer, "");
  for ( i = 1; i < argc; i++ ) {
    strcat( buffer, argv[i] );
    strcat( buffer, " ");
  }
  printf("|%s|\n",buffer);

   res = execute_command( buffer, 1);
   free(buffer);
  return TCL_OK;
}

/*
 * --------------------------------------------------------------------------
 *
 * Function: get_number_levels
 *
 * Purpose: This procedure will be registered in gdb.  It will return the
 *          number of levels in the Tcl interpreter we have gone through
 *          to parse a command.  It is primarily for debugging purposes.
 *
 * --------------------------------------------------------------------------
 */
int
get_number_levels( ClientData clientData, Tcl_Interp * interp,
		   int argc, char* argv[] )
{
  char * buffer;
  buffer = (char*)malloc( 1000 );
  if ( buffer == NULL ) {
    return TCL_ERROR;
      }

  if (  ( interp != NULL ) && ( ((Interp*)interp)->framePtr != NULL ) ) {
    sprintf( buffer,"%d", ( ( Interp* ) interp )->framePtr->level );
    Tcl_SetResult( interp, buffer, TCL_VOLATILE );
  } else {
    Tcl_SetResult( interp, "NULL", TCL_STATIC );
  }
  free ( buffer );
  return TCL_OK;
}


/*
 * -------------------------------------------------------
 *
 * Function: is_complete
 *
 * Purpose:  This procedure will just call Tcl_CommandComplete.
 *           It will be registered in GDB so that we can determine
 *           the readiness of a command to be processed from the
 *           GUI we are writing.
 *
 * -------------------------------------------------------
 */
int
is_complete( ClientData clientData, Tcl_Interp *interp,
	     int argc, char* argv [] )
{
  int i;
  char * buffer;
  buffer = ( char* )malloc( 1000 );
  if ( buffer == NULL ) {
    return TCL_ERROR;
  }
  strcpy( buffer, " " );
  for ( i = 1; i < argc; i++ ) {
    strcat( buffer, argv[i] );
      strcat( buffer, " ");
  }
  sprintf( buffer, "%d", Tcl_CommandComplete(buffer) );
  Tcl_SetResult( interp, buffer, TCL_VOLATILE );
  free( buffer );
  return TCL_OK;
}


