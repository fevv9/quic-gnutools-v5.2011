/* Target-dependent code for QUALCOMM QDSP6 GDB, the GNU Debugger.
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

#define TARGET_NAME "qdsp6-zebu"
#include "qdsp6-sim.h"

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

static pid_t qdsp6_sim_pid = (pid_t) 0;
static void qdsp6zebu_close (int quitting);
static void qdsp6_mourn_inferior (void);
struct target_ops *qdsp6zebu_target = (struct target_ops *)NULL;
static int portid = 0;
static struct hostent *qdsp6_hostent = (struct hostent *)NULL;
static int alarmed = 0;
static int sim_died = 0;

/*
 * function: qdsp6_random_port
 * description:
 *        - Get a random port number that is not currently in use.
 */
static int qdsp6_random_port(void)
{
     int port;
     srand( getpid() *
            (unsigned)time(0));
     port     =  (int)((((float)rand()/RAND_MAX) * 10000) + 10000); 
     return port;
}


	

/*
 * function: qdsp6zebu_can_run
 * description:
 * 	- Tries to assure that when create_inferior is called it will be successful.
 *	- The bulk of the code here is geared to reliably finding a port number that 
 *        will work.
 */
static int
qdsp6zebu_can_run(void)
{
    extern char *current_q6_target;
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

    qdsp6_hostent = gethostbyname (default_host);
    if (!qdsp6_hostent)
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

 	portid = qdsp6_random_port();
        sockaddr.sin_family = PF_INET;
        sockaddr.sin_port = htons (portid);
        memcpy (&sockaddr.sin_addr.s_addr, qdsp6_hostent->h_addr,
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
 * function: qdsp6zebu_exec_simulation
 * description:
 * 	Call fork/execvp to start the simulator
 */
static int
qdsp6zebu_exec_simulation(char *sim_name, char *exec_name,
		      int portid, char *args, char **env)
{
    char port[11];
    int index = 0;
#ifdef USE_WIN32API
    struct pex_obj *p = pex_init(PEX_RECORD_TIMES,"qdsp6 sim process",NULL);
    const char *errmsg;
    int err;
    extern char *current_q6_target;
#endif

#ifdef USE_WIN32API
    qdsp6_sim_pid = 0;
#else
    if ((qdsp6_sim_pid = fork()) == -1)
    {
        printf_filtered("failed fork for simulator, exiting.\n");
        exit (errno);
    }
#endif
    if (qdsp6_sim_pid == 0) /* we are the child so exec the sim */
    {
        char *sim_args[256];
        int argc = 0;
	sprintf (port, "%d", portid);
        sim_args[index++] = strdupa (sim_name);
        sim_args[index++] = strdupa (exec_name);
        sim_args[index++] = strdupa ("--gdbserv");
        sim_args[index++] = strdupa (port);

	while (q6targetargsInfo[argc])
            sim_args[index++] = strdupa (q6targetargsInfo[argc++]);

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
	qdsp6_sim_pid = (p->funcs->exec_child)(p, PEX_SEARCH,
			 current_q6_target, sim_args, NULL, 
			 0, 1, 2, 0, &errmsg, &err);
	if (-1 == qdsp6_sim_pid) 
	    error ("qdsp6sim_exec_simulation: Unable to execute qdsp6-sim!");

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
    //qdsp6_sim_accept_timeout (...); // TODO
    sleep(1);

    return portid;
}


/*
 * function: qdsp6zebu_create_inferior
 * description:
 *        - Primary function of this target.
 *        - Starts the simulator and forms a connection.
 */
static void qdsp6zebu_create_inferior (char *exec_file, char *args, char **env)
{
    extern struct serial *remote_desc;
    extern struct serial_ops *serial_interface_lookup (char *);
    extern char *current_q6_target;

    struct sockaddr_in sockaddr;

    char target_port[8];
    int rc = 0;
    int nagle_off = 1;
    int reuse = 1;

    if (!THIS_TARGET())
	return 0;

    qdsp6zebu_exec_simulation("qdsp6-zebu", exec_bfd->filename, portid, NULL, NULL);
    sprintf (target_port, ":%d", portid);
    push_remote_target(target_port, 1);


    return;
}

static void
qdsp6zebu_close (int quitting)
{
    if (qzebu_debug)
    {
        printf_filtered ("%s, quitting = %d\n", __func__, quitting);
    }

    if ((quitting) && (qdsp6_sim_pid != 0))
    {
        catch_errors ((catch_errors_ftype *) putpkt, "k", "", RETURN_MASK_ERROR);
    }
}

/*
 * function: qdsp6_threads_extra_info
 * description: (from remote.c)
 *     - Simple routine that queries the simulator to print more detailed thread
 *       info.
 */
static char *
qdsp6_threads_extra_info (struct thread_info *tp)
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
 * function: qdsp6_find_new_threads
 * description: (from remote.c)
 *     - Simple routine that queries the simulator to find out how many threads
 *	 there are.
 */
static void
qdsp6_find_new_threads (void)
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
 * function: qdsp6zebu_open
 * description:
 *      - Called when the user selects the qdsp6-zebu target architecture.
 */
static void
qdsp6zebu_open (char *args, int from_tty)
{
    extern char *current_q6_target;
    if (qzebu_debug)
    {
        printf_filtered ("%s\n", __func__);
    }
    target_preopen (1);
    current_q6_target = "qdsp6-zebu";
}


void
_initialize_qdsp6_zebu (void)
{
  struct target_ops *t;

  t = XZALLOC (struct target_ops);

  t->to_shortname = "qdsp6-zebu";
  t->to_longname = "Qualcomm Hexagon ZEBU interface";
  t->to_doc = "Qualcomm Hexagon Zebu interface, used for debugging software \n\
running on the Hexagon (qdsp6-zebu) target";

  t->to_open = qdsp6zebu_open;
  t->to_close = qdsp6zebu_close;
  t->to_can_run = qdsp6zebu_can_run;
  t->to_create_inferior = qdsp6zebu_create_inferior;

  /* Register target.  */
  add_target (t);
  qdsp6zebu_target = t;

}

