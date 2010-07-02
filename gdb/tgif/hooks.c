
/*
 *
 * File: hooks.c
 *
 * Purpose: This file contains hook functions which are
 *          used to invoke special processing inside gdb.
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
#include <unistd.h>
#include <fcntl.h>
/*
 * This is the return buffer so that the results of
 * gdb commands can be sent to tcl.
 */
size_t SIZEOFBUFFER = 131072;
char* tcl_status_buffer = NULL;
char* inset_into_buffer = NULL;


int
tgif_query_hook(char *ctlstr, va_list args) {

	int flags;
	register int answer;
	register int ans2;
	int retval;

	struct timeval timeout;
	fd_set readset;
	int sreturn;

	if (!isInteractive() || need_concatenation) {
		return 1;
	}
	if (!input_from_terminal_p ())
		return 1;

	while (1) {
		wrap_here ("");		/* Flush any buffered output */
		gdb_flush (gdb_stdout);

		if (annotation_level > 1)
			printf_filtered ("\n\032\032pre-query\n");

		vfprintf_filtered (gdb_stdout, ctlstr, args);
		printf_filtered ("(y or n) ");

		if (annotation_level > 1)
			printf_filtered ("\n\032\032query\n");

		gdb_flush (gdb_stdout);

		answer = -1;
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		while ( answer < 0 ) {
			FD_ZERO(&readset);
			FD_SET( fileno(stdin), &readset );
			Tcl_DoOneEvent(0);
			sreturn = select ((fileno(stdin)+ 1), &readset, NULL, NULL, &timeout);
			if ( sreturn <= 0 ) { continue; }

			answer = fgetc(stdin);
		}
		clearerr (stdin);		/* in case of C-d */
		if (answer == EOF) {	/* C-d */
			retval = 1;
			break;
		}
		if (answer != '\n')	/* Eat rest of input line, to EOF or newline */
			do {
				ans2 = -1;
				timeout.tv_sec = 0;
				timeout.tv_usec = 0;

				if ((flags = fcntl (fileno (stdin), F_GETFL, 0)) < 0)
					return (EOF);
				flags |= O_NONBLOCK;
				fcntl (fileno (stdin), F_SETFL, flags);

				ans2 = fgetc (stdin);

				while ( ans2 < 0 ) {
					FD_ZERO(&readset);
					FD_SET( fileno(stdin), &readset );
					Tcl_DoOneEvent(0);
					sreturn = select ((fileno(stdin)+ 1), &readset, NULL, NULL,
							  &timeout);
					if ( sreturn <= 0 ) {
						continue;
					}

					ans2 = fgetc(stdin);
				}
				clearerr (stdin);
			}
			while (ans2 != EOF && ans2 != '\n');
		if (answer >= 'a')
			answer -= 040;
		if (answer == 'Y') {
			retval = 1;
			break;
		}
		if (answer == 'N') {
			retval = 0;
			break;
		}
		printf_filtered ("Please answer y or n.\n");
	}

	if (annotation_level > 1)
		printf_filtered ("\n\032\032post-query\n");
	return retval;
}
