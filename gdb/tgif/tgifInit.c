
/* -------------------------------------------------------------------- 
 *
 * File: tgifInit.c 
 *
 * Purpose: This file contains the initialization routines for Tcl-Gdb 
 *          interface, along with constructs to assist in message 
 *          passing between the two.
 *
 * --------------------------------------------------------------------
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

#include "tclInt.h"
#include "GuiSupport.h"
#include "gdb.h"
#include "hooks.h"
#include "register.h"

#if USE_WIN32API
#include <windows.h>
#endif



struct ui_file
  {
    int *magic;
    ui_file_flush_ftype *to_flush;
    ui_file_write_ftype *to_write;
    ui_file_fputs_ftype *to_fputs;
    ui_file_read_ftype *to_read;
    ui_file_delete_ftype *to_delete;
    ui_file_isatty_ftype *to_isatty;
    ui_file_rewind_ftype *to_rewind;
    ui_file_put_ftype *to_put;
    void *to_data;
  };

#define MAXPATHSTRING 2048

/* Global Variable Section */
int was_gdb_command;
int top_level_from_tty = 1;
int tclsh_executable = 0;
int need_concatenation = 0;
int was_proceed_command = 0;
int is_breakpoint_callbk = 0;
int cmd_result_flag = TCL_OK;


/* Extern Section */
extern int  is_breakpoint_callbk;
extern int  orig_from_tty;
extern int  tcl_getInteractive();
extern void tcl_setInteractive(int status);
/*
 * Interpreter for the application
 */
Tcl_Interp *interp;


/*
 * This variable (was_gdb_error) and fix_error_level are used
 * to decide when and if to print error output.
 */
static int was_gdb_error = 0;
void fix_error_level (void) {
	was_gdb_error = 1;
}


void 
Execute(char *cmd) {
  Tcl_Eval(interp, cmd);
}

/*
 * These variables are used to buffer long commands.
 */
static Tcl_DString tcl_command;
static int  longCommand = 0;

//int [ams]
void Print_If_Needed(void) {
    if (isInteractive()) {
		printf("%s\n", interp->result);
	}
}

/* -------------------------------------------------------------
 *
 * Function: tgif_puts_hook TEST
 *
 * Purpose: This is a hook for the standard gdb printing routines.
 *          It is called from fputs_unfiltered (the lowest level
 *          gdb routine) to manage the results from GDB.
 *
 * -------------------------------------------------------------*/
void Tgif_puts_hook (const char * linebuffer, 
                     struct ui_file * stream)
{    
	char * temp;
    int pVal = 0; 
	if ( tcl_status_buffer != NULL ) {
		int length = strlen(linebuffer);
		if ( inset_into_buffer+length+1 > tcl_status_buffer+SIZEOFBUFFER ) {
			SIZEOFBUFFER += 4096;
			temp = (char * )realloc( tcl_status_buffer, SIZEOFBUFFER );
			if ( temp == NULL ) {
				SIZEOFBUFFER -= 4096;
				printf("Realloc failed.  Sorry.\n" );
                return ;
			} else { 
				inset_into_buffer = ( inset_into_buffer - tcl_status_buffer ) + temp;
				tcl_status_buffer = temp;
			}
		}
		strcpy( inset_into_buffer, linebuffer);
		inset_into_buffer += length; 
		*inset_into_buffer = '\0';
	}

    /* print output criteria 
     * 1st  : tcl command */
    if (was_gdb_command == 0) 
         stream->to_fputs(linebuffer, stream);
    else if (was_gdb_command == 1)
    {   
       /* 1st  : non-nested gdb command @ command line + mi
        * 3rd  : gdb_command in callback, 'puts' alone enabled */
#ifdef USE_WIN32API
       if (is_breakpoint_callbk != 1)
         stream->to_fputs(linebuffer, stream);
#else
       if ((tcl_getInteractive() == 0) && (is_breakpoint_callbk != 1))
         stream->to_fputs(linebuffer, stream);
#endif
    }
  return; 
 
}


/* ---------------------------------------------------------------------
 *
 * Function: Tgif_Init 
 *
 * Purpose: This function is called from Gdb to initialize a Tcl 
 *          interpreter.  It initializes a Tcl/Tk interpreter, registers
 *          a few commands with it .
 *
 * ---------------------------------------------------------------------
 */
int Tgif_Init ( void ) { 

  char PathString[MAXPATHSTRING] = {'0'};
  char temp[MAXPATHSTRING]       = {'0'};
  char *pathstring = NULL;
  
    //printf("\nIn  Tgif_Init.\n");
    /* Create Tcl Interpreter.*/
	interp = Tcl_CreateInterp ();
	
    /*Setup interactvity.*/
    Tcl_SetVar (interp, "tcl_interactive", isatty(0)?"1":"0", TCL_GLOBAL_ONLY);

#ifdef USE_WIN32API
    GetModuleFileName(NULL,temp,MAXPATHSTRING-1);
    /* Get the absolute path of tcl8.4 libraries */
    strcat(temp, "/../../lib/tcl84");
    strcpy(PathString, lrealpath(temp));
#else
    /* Get the path where qdsp6-gdb executable resides */ 
    snprintf(temp, sizeof(temp),"/proc/self/exe");
    pathstring = realpath(temp, NULL);
    if (pathstring && (strlen (pathstring) < MAXPATHSTRING)) {
        strcpy (PathString, pathstring);
        free (pathstring);
    }
    else {
        fprintf(stderr, "Unable to find path to init.tcl\n");
        free (pathstring);
        return TCL_ERROR;
    }

    /* Get the absolute path of tcl8.4 libraries */
    strcat(PathString, "/../../lib/tcl84");
#endif
    
    Tcl_SetVar(interp, "tcl_library", PathString, TCL_GLOBAL_ONLY);

	/* Initialize interpreter.*/
	if (Tcl_Init(interp) == TCL_ERROR) {
		fprintf(stderr, "Unable to locate init.tcl.\n");
		return TCL_ERROR;
	}
	

	/*Setup the hooks. TEST */
	deprecated_query_hook = tgif_query_hook;

	
	/* Do Command Ini  */
	if ( tgif_AppInit(interp) != TCL_OK ) {
		fprintf( stderr, "Tcl_AppInit failed: %s\n", interp->result );
	}  
	
   // printf("\nOut Tgif_Init.\n");
    return 1;
}

/* 
 * ------------------------------------------------------------------
 * 
 * Function: tgif_AppInit 
 *
 * Purpose: This function calls various sub-functions to do command
 *          registration.  GDB commands are registered with the Tcl
 *          interpreter.
 *
 * ------------------------------------------------------------------
 */
static int
tgif_AppInit (Tcl_Interp *interp) {
  //  printf("\nIn  tgif_AppInit\n");
	Tcl_CreateCommand( interp, "setproc", Tgif_SetprocCmd,
			   (ClientData) NULL, (Tcl_CmdDeleteProc*) NULL);
	RegisterAllCmds(interp);
	Tcl_DStringInit(&tcl_command);
  // printf("\nOut tgif_AppInit\n");
	return TCL_OK;
}

/*
 * --------------------------------------------------------------------
 * 
 * Function: eval_command
 *
 * Purpose: This is the main entry point from GDB into Tcl.  A 
 *          Tcl command executed by SmartGDB will pass through here.
 *          Note that all the output switching (i.e. whether to print
 *          output or not) is done in here.
 *
 * Notes:   from_tty --> This indicates if the command was executed
 *                       inside a script or not.
 *
 *          isInteractive --> This is a window into top_level_from_tty
 *                            which tells us if we're in a procedure
 *                            or not.
 *
 * --------------------------------------------------------------------
 */
static void
eval_command (char *cmd, int from_tty) {
	int  code, old_from_status;
  //  printf("\nIn  eval command.\n");
	was_gdb_command = 0;
    orig_from_tty   = from_tty;
    
	if ( from_tty==0  ) {
	  old_from_status = isInteractive();	 
	  setInteractive (0);
	}
	/*
	 * Actual Evaluation Here.
	 */

	code = Tcl_Eval(interp, cmd);
	if ( from_tty == 0 ) {
	  setInteractive (old_from_status);
	}
	if (code != TCL_OK) {
	  if ( ( !was_gdb_error ) && !was_proceed_command ) {
	    fprintf(stderr, "%s\n", interp->result);
          } else {
            was_gdb_error = 0;
          }
	} else {
	  if ( !was_gdb_command && from_tty && (*interp->result != 0)) {
         fprintf_filtered(gdb_stdout, "%s\n", interp->result);
         gdb_flush(gdb_stdout);
   	  }
	}
	was_proceed_command = 0;
	Tcl_ResetResult(interp);
  //  printf("\nOut eval command.\n");
}

/* 
 * ----------------------------------------------------------------------
 *
 * Function: Tgif_execute_command 
 *
 * Purpose: This is the front end to command processing.  It is called from
 *          the command loop.  It builds up a string of commands until they 
 *          are complete (checked by tcl) and then passes them on to be 
 *          executed.
 *
 * ----------------------------------------------------------------------
 */
int 
Tgif_execute_command (char *cmd, int from_tty) {
	int  code, old_from_status;
	char *newCmd;
    
    /* change to tcl prompt */
    set_prompt (TCL_PROMPT);
    /* reset tcl gdb display flag */
    tcl_setInteractive(0);

    if (!longCommand) {
		if (!Tcl_CommandComplete(cmd)) {
			longCommand = 1;
			Tcl_DStringFree(&tcl_command);
			Tcl_DStringAppend(&tcl_command, cmd, -1);
            return (0);
		}
	}
	else {
		Tcl_DStringAppend(&tcl_command, "\n", -1);
		cmd = Tcl_DStringAppend(&tcl_command, cmd, -1);
		if (!Tcl_CommandComplete(cmd)) {
            return (0);
        }
        longCommand = 0;
	}
   
    /* Process tcl+gdb command */
    eval_command (cmd, from_tty);
   
    /* reset tcl gdb display flag */
    tcl_setInteractive(0);

    /* change back to qdsp6-gdb prompt */
    set_prompt (DEFAULT_PROMPT);

	return (1);
}


/* 
 * -------------------------------------------------------------------------
 *
 * Function: Tgif_Cleanup 
 * 
 * Purpose: This function is registered in the cleanup chain to be called
 *          to clean up after our tgif stuff in case of a major error.
 *
 * -------------------------------------------------------------------------
 */
void
Tgif_Cleanup (void) {
	longCommand = 0;
	setInteractive (1);
#ifdef HAVETCL
	((Interp*)interp)->numLevels = 0;
#endif
}



/* -------------------------------------------------------------------------
 *
 * Function: _initialize_tgif
 *
 * Purpose: This function is called from the gdb intialization routines
 *          in top.c.  It sets up the buffer to pass between Tcl and Gdb
 *          results, and then returns.
 *
 * -------------------------------------------------------------------------
 */
void
_initialize_tgif (void) {
  tcl_status_buffer = (char *)malloc(SIZEOFBUFFER);
  inset_into_buffer = tcl_status_buffer;
  if ( tcl_status_buffer != NULL ) {
    Tgif_Init ();   
  } else { 
    fprintf(stderr, "No memory for buffer\n"); 
    exit(-1); 
  }
}


