
#ifndef __TGIF_H
#define __TGIF_H
#include "tcl.h"

#include <sys/types.h>
#include <stdarg.h>
/*
 * tcl_status_buffer: this is a global buffer we use so that
 *                    output from GDB commands can be redirected
 *                    into tcl commands.  It is manipulated in
 *                    the functions fputs_unfiltered in main.c.
 *                    inset_into_buffer is so we can reuse the
 *                    same buffer.
 */
extern void fix_error_level (void);

extern int was_proceed_command;

extern Tcl_Interp *interp;
/*
 * Flag that indicates whether to use a window interface or not
 */
extern int use_windows;

/*
 * was_gdb_command: This is a global varaible that will tell us
 *                  if we just executed  a GDB command or not.
 *                  This is so we can suppress tcl output if GDB
 *                  has already printed it.
 */
extern int was_gdb_command;

/*
 * need_concatenation: some commands need their output to be
 *                     built up (i.e. thread apply) across many
 *                     calls to execute.  If so, they set this
 *                     flag.
 */
extern int need_concatenation;



/* These are used in 'catch' command processing. */
#define GDB_PROCEED	    5
#define GDB_PROCEED_STRING "5"
#define GDB_PROCEED_ERROR  "command returned bad code: 5"

#define	INIT_FILTER_SIZE (1024)

/*
 * top_level_from_tty: This is a global variable to tell us if we
 *                     are supposed to be printing.  It is saved and
 *                     restored as Tcl moves through various layers of
 *                     recursive command parsing.  This is all done in
 *                     Tgif_execute_command.
 *
 */
extern int top_level_from_tty;
#define isInteractive()	(top_level_from_tty)
#define	setInteractive(status)	top_level_from_tty = (status)

/*
 * Function: Tgif_Init
 *
 * Purpose:  This function it called from Gdb to initialize a Tcl
 *           interpreter for the application.  It also registers some
 *           application specific commands and starts Tk.
 *
 *
 */
int Tgif_Init (void);

void Execute(char *cmd);

/*
 * Tgif_execute_command - The front-end to Tcl-Gdb interface. Processes
 * any command thats passed on to Gdb, for Tcl-specific constructs.
 */
int Tgif_execute_command (char *cmd, int from_tty);
static int tgif_AppInit (Tcl_Interp *interp);
//extern int Print_If_Needed(char *, int);
extern void Print_If_Needed(char *, int);


/*
 * Tgif_Concat - Given a list of command line arguments,
 *               this forms a string containing the elements
 *               of the list.
 */
char *Tgif_Concat (int argc, char **argv);

/*
 * The following commands implement the new tcl commands that
 * are added for scripting ability for gdb
 */
int Tgif_SetprocCmd(ClientData clientdata, Tcl_Interp *interp,
		    int argc, char *argv[]);

#endif
