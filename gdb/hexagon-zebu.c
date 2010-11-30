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

#include "defs.h"
#include "gdbcore.h"
#include "serial.h"
#include "gdbthread.h"
#include "target.h"
#include "remote.h"
#include "inferior.h"
#include "exceptions.h"

#define TARGET_NAME "hexagon-zebu"
#include "hexagon-sim.h"

#include <unistd.h>
#include <time.h>
#include <string.h>

#ifdef USE_WIN32API
#include "../libiberty/pex-common.h"
#include <windows.h>
#else
#include <netdb.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif
#include <signal.h>
#include <sys/types.h>

static int qzebu_debug = 0;
static struct serial scb;

static pid_t hexagon_sim_pid = (pid_t) 0;
static void hexagonzebu_close (int quitting);
static void hexagon_mourn_inferior (void);
struct target_ops *hexagonzebu_target = (struct target_ops *)NULL;
static int portid = 0;
static struct hostent *hexagon_hostent = (struct hostent *)NULL;
static int alarmed = 0;
static int sim_died = 0;

/*
 * function: hexagon_random_port
 * description:
 *        - Get a random port number that is not currently in use.
 */
static int hexagon_random_port(void)
{
     int port;
     srand( getpid() *
            (unsigned)time(0));
     port     =  (int)((((float)rand()/RAND_MAX) * 10000) + 10000); 
     return port;
}


	

/*
 * function: hexagonzebu_can_run
 * description:
 * 	- Tries to assure that when create_inferior is called it will be successful.
 *	- The bulk of the code here is geared to reliably finding a port number that 
 *        will work.
 */
static int
hexagonzebu_can_run(void)
{
    extern char *current_hexagon_target;
    extern struct bfd *exec_bfd;
    char *default_host = "localhost";
    struct sockaddr_in sockaddr;
    int ntries = 10;
    int portvalid = -1;
    char target_port[8];

    if (qzebu_debug)
    {
        printf_filtered ("%s\n", __func__);
    }

    if (!THIS_TARGET())
	return 0;
	

    if (!exec_bfd)
    {
        printf_filtered ("No executable file specified.\n");
        printf_filtered ("Use the \"file\" or \"exec-file\" command.\n");
        return 0;
    }

    hexagon_hostent = gethostbyname (default_host);
    if (!hexagon_hostent)
    {
        printf_filtered ("bailing from %s.\n", __func__);
        perror ("gethostbyname");
        printf_filtered ("hostname = %s.\n", default_host);
        return 0;
    }
/*
 * When we are done we should have found a port number that we can bind to.
 * All this loop will do is validate that the selected port is going to work
 * so we can be reasonable sure this will work for our simulation connection.
 * 
 * This looks like overkill until one considers that the debugger isn't always
 * run interactively.  Qualcomm's testsuite runs thousands of sessions and we have
 * to be sure that the selected port is going to work or some tests will timeout.
 */
    scb.fd = 0;
    do
    {
        int yes = 1;

        scb.fd = socket (PF_INET, SOCK_STREAM, 0);
        if (scb.fd < 0)
        {
            printf_filtered ("bailing from %s.\n", __func__);
            perror ("socket");
            exit(errno); /* execlp failed so bail */
        }
	/* see "man select_tut" */
        setsockopt (scb.fd, SOL_SOCKET, SO_REUSEADDR, (char *) &yes, sizeof (yes));

 	portid = hexagon_random_port();
        sockaddr.sin_family = PF_INET;
        sockaddr.sin_port = htons (portid);
        memcpy (&sockaddr.sin_addr.s_addr, hexagon_hostent->h_addr,
		sizeof (struct in_addr));

	portvalid = bind (scb.fd, (struct sockaddr *) &sockaddr, sizeof (sockaddr)); 
#if USE_WIN32API
	closesocket (scb.fd);
#else
	close (scb.fd);		/* Will always reopen below */
#endif

    } while ((portvalid != 0) && (ntries-- > 0));

    if (qzebu_debug)
	printf_filtered ("ntries == %d, portvalid = %d, portid = %d\n",
			 ntries, portvalid, portid);
    if (ntries < 0)
    {
        printf_filtered ("bailing from %s. could not find an open port\n", __func__);
	return 0;
    }

    return 1;
}

/*
 * function: hexagonzebu_exec_simulation
 * description:
 * 	Call fork/execvp to start the simulator
 */
static int
hexagonzebu_exec_simulation(char *sim_name, char *exec_name,
		      int portid, char *args, char **env)
{
    char port[11];
    int index = 0;
#ifdef USE_WIN32API
    struct pex_obj *p = pex_init(PEX_RECORD_TIMES,"hexagon sim process",NULL);
    const char *errmsg;
    int err;
    extern char *current_hexagon_target;
#endif

#ifdef USE_WIN32API
    hexagon_sim_pid = 0;
#else
    if ((hexagon_sim_pid = fork()) == -1)
    {
        printf_filtered("failed fork for simulator, exiting.\n");
        exit (errno);
    }
#endif
    if (hexagon_sim_pid == 0) /* we are the child so exec the sim */
    {
        char *sim_args[256];
        int argc = 0;
	sprintf (port, "%d", portid);
        sim_args[index++] = strdupa (sim_name);
        sim_args[index++] = strdupa (exec_name);
        sim_args[index++] = strdupa ("--gdbserv");
        sim_args[index++] = strdupa (port);

	while (hexagontargetargsInfo[argc])
            sim_args[index++] = strdupa (hexagontargetargsInfo[argc++]);

	/*
	 * This case the user has passed arguments to the program that
	 * is going to be run on the simulator.
	 * NOTE: Always use a NULL (0) to signal the end of the list.
	 */
	if (args)
	{
            sim_args[index++] = strdupa ("--");
	    sim_args[index++] = strdupa (args);
	    sim_args[index++] = 0;
	}
	else
	{
	    sim_args[index++] = 0;
	}

        if (qzebu_debug)
        {
	    int i = 0;
	    while (sim_args[i])
		printf_filtered ("%s ", sim_args[i++]);
	    printf_filtered ("\n");
        }

#ifdef USE_WIN32API
/*
 * Windows does the fork and exec in one step.
 */
	hexagon_sim_pid = (p->funcs->exec_child)(p, PEX_SEARCH,
			 current_hexagon_target, sim_args, NULL, 
			 0, 1, 2, 0, &errmsg, &err);
	if (-1 == hexagon_sim_pid) 
	    error ("hexagonsim_exec_simulation: Unable to execute hexagon-sim!");

#else
        execvp(sim_args[0], sim_args); 
        printf_filtered ("bailing from child.\n");
        perror ("execvp");
        exit(errno); /* execlp failed so bail */
#endif
    }

/*
 * now verify that the launched simulator is ready to accept
 * new connections.
 */
    //hexagon_sim_accept_timeout (...); // TODO
    sleep(1);

    return portid;
}


/*
 * function: hexagonzebu_create_inferior
 * description:
 *        - Primary function of this target.
 *        - Starts the simulator and forms a connection.
 */
static void hexagonzebu_create_inferior (char *exec_file, char *args, char **env)
{
    extern struct serial *remote_desc;
    extern struct serial_ops *serial_interface_lookup (char *);
    extern char *current_hexagon_target;

    struct sockaddr_in sockaddr;

    char target_port[8];
    int rc = 0;
    int nagle_off = 1;
    int reuse = 1;

    if (!THIS_TARGET())
	return 0;

    hexagonzebu_exec_simulation("hexagon-zebu", exec_bfd->filename, portid, NULL, NULL);
    sprintf (target_port, ":%d", portid);
    push_remote_target(target_port, 1);


    return;
}

static void
hexagonzebu_close (int quitting)
{
    if (qzebu_debug)
    {
        printf_filtered ("%s, quitting = %d\n", __func__, quitting);
    }

    if ((quitting) && (hexagon_sim_pid != 0))
    {
        catch_errors ((catch_errors_ftype *) putpkt, "k", "", RETURN_MASK_ERROR);
    }
}

/*
 * function: hexagon_threads_extra_info
 * description: (from remote.c)
 *     - Simple routine that queries the simulator to print more detailed thread
 *       info.
 */
static char *
hexagon_threads_extra_info (struct thread_info *tp)
{
    int result;
    static char display_buf[256];	/* arbitrary... */
    char *bufp = alloca (256);
    int n = 0;                    /* position in display_buf */

    sprintf (bufp, "qThreadExtraInfo,%x", PIDGET (tp->ptid));
    putpkt (bufp);
    getpkt (bufp, 256, 0);

    if (bufp[0] != 0)
    {
        n = min (strlen (bufp) / 2, sizeof (display_buf));
        result = hex2bin (bufp, display_buf, n);
        display_buf [result] = '\0';
        return display_buf;
    }
    return (char *)0;
}

/*
 * function: hexagon_find_new_threads
 * description: (from remote.c)
 *     - Simple routine that queries the simulator to find out how many threads
 *	 there are.
 */
static void
hexagon_find_new_threads (void)
{
  char *buf = alloca (256);
  char *bufp;
  int tid;

    putpkt ("qfThreadInfo");
    bufp = buf;
    getpkt (bufp, 256, 0);

    if (bufp[0] != '\0')		/* q packet recognized */
    {
	while (*bufp++ == 'm')	/* reply contains one or more TID */
	{
	    do
	    {
		tid = strtol (bufp, &bufp, 16);
		if (tid != 0 && !in_thread_list (pid_to_ptid (tid)))
		    add_thread (pid_to_ptid (tid));
	    }
	    while (*bufp++ == ',');	/* comma-separated list */
	}
	return;	/* done */
    }
    return;
}

/*
 * function: hexagonzebu_open
 * description:
 *      - Called when the user selects the hexagon-zebu target architecture.
 */
static void
hexagonzebu_open (char *args, int from_tty)
{
    extern char *current_hexagon_target;
    if (qzebu_debug)
    {
        printf_filtered ("%s\n", __func__);
    }
    target_preopen (1);
    current_hexagon_target = "hexagon-zebu";
}


void
_initialize_hexagon_zebu (void)
{
  struct target_ops *t;

  t = XZALLOC (struct target_ops);

  t->to_shortname = "hexagon-zebu";
  t->to_longname = "Qualcomm Hexagon ZEBU interface";
  t->to_doc = "Qualcomm Hexagon Zebu interface, used for debugging software \n\
running on the Hexagon (hexagon-zebu) target";

  t->to_open = hexagonzebu_open;
  t->to_close = hexagonzebu_close;
  t->to_can_run = hexagonzebu_can_run;
  t->to_create_inferior = hexagonzebu_create_inferior;

  /* Register target.  */
  add_target (t);
  hexagonzebu_target = t;

}

