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
#include "gdbtypes.h"
#include "gdbcore.h"
#include "gdbcmd.h"
#include "serial.h"
#include "gdbthread.h"
#include "target.h"
#include "remote.h"
#include "inferior.h"
#include "exceptions.h"
#include "completer.h"
#include "watchpoint_types.h"
#include "cli/cli-decode.h"
#include "hexagon-tdep.h"

#define TARGET_NAME "hexagon-sim"
#include "hexagon-sim.h"

#include <unistd.h>
#include <time.h>
#include <string.h>

#ifdef USE_WIN32API
#include "../libiberty/pex-common.h"
#include <windows.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif

#include <sys/types.h>

static int qsim_debug = 0;
static struct serial scb;

char *hexagon_targetargsInfo[SIM_ARG_MAX];
static pid_t hexagon_sim_pid = (pid_t) 0;
static void hexagon_sim_close (int quitting);
static void hexagon_mourn_inferior (void);
struct target_ops *hexagon_sim_target = (struct target_ops *)NULL;
static int portid = 0;
static struct hostent *hexagon_hostent = (struct hostent *)NULL;
static int alarmed = 0;
static int sim_died = 0;

/* captures the arguments to the target passed thru the 
 * set targetargs command */
char *current_hexagon_target = NULL;
int HEXAGON_tcl_fe_state = 0;

/* for set globalregs command - contains the user typed string*/
static char *hexagon_globalregsInfo = NULL;
/* for set interrupt command - contains the user typed string*/
static char *hexagon_Interrupt      = NULL;
/* for hwthrdbg command; 0 indicates s/w thread debug; non-zero indicated 
   h/w thread debug */
static int hexagon_hwthread_debug = 0;

/* The list of available "info hexagon " commands.  */
static struct cmd_list_element *hexagon_cmdlist = NULL;


/* maximum message size from RIL */
/* 1.0 MB = 512 * 2048 */
#define GDB_RSP_RIL_INFO_MAX_SIZE 256 * 2048
#define SET_CMD_BUFFER_SIZE 1024
static long gdb_rsp_ril_info_max_size=GDB_RSP_RIL_INFO_MAX_SIZE;

/* store for holding the response; this is raw-ascii coded */
static char response_buf[GDB_RSP_RIL_INFO_MAX_SIZE];
static char *response = response_buf;


/* pointer to the architecture's thread registers */
hexagon_regtype_t * threadRegSetInfo;
/* pointer to the architecture's thread registers */
hexagon_regtype_t * globalRegSetInfo;


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
 * function: hexagon_sim_can_run
 * description:
 * 	- Tries to assure that when create_inferior is called it will be successful.
 *	- The bulk of the code here is geared to reliably finding a port number that 
 *        will work.
 */
static int
hexagon_sim_can_run(void)
{
    extern struct bfd *exec_bfd;
    char *default_host = "localhost";
    struct sockaddr_in sockaddr;
    int ntries = 10;
    int portvalid = -1;

    if (qsim_debug)
    {
        printf_filtered ("%s\n", __func__);
    }
    if (!THIS_TARGET())
	return 0;

    if (!exec_bfd)
    {
	if (qsim_debug == 1)
	{
            printf_filtered ("No executable file specified.\n");
            printf_filtered ("Use the \"file\" or \"exec-file\" command.\n");
	}
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

    if (qsim_debug)
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
 * function: hexagon_sim_exec_simulation
 * description:
 * 	Call fork/execvp to start the simulator
 */
static int
hexagon_sim_exec_simulation(char *sim_name, char *exec_name,
		         char *args, char **env, int portid)
{
    char port[11];
    int index = 0;
#ifdef USE_WIN32API
    struct pex_obj *p = pex_init(PEX_RECORD_TIMES,"hexagon sim process",NULL);
    const char *errmsg;
    int err;
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

/* XXX_SM: When the user passes explicit arguments to the simulator
           he should not include the matching executable name, gdb
           will furnish that to the simulator.
 */
	if (hexagon_targetargsInfo[0]==(char *)0)
	    sim_args[index++] = strdupa (exec_name);

        sim_args[index++] = strdupa ("--gdbserv");
        sim_args[index++] = strdupa (port);

	while (hexagon_targetargsInfo[argc])
            sim_args[index++] = strdupa (hexagon_targetargsInfo[argc++]);

	/*
	 * This case the user has passed arguments to the program that
	 * is going to be run on the simulator.
	 * NOTE: Always use a NULL (0) to signal the end of the list.
	 */
	if (args)
	{
            sim_args[index++] = strdupa ("--");

            /*
             * When simulator targetargs have been specified place the
             * exec name prior to the argument list being passed to
             * the executable running on the simulator.
             */
	    if (hexagon_targetargsInfo[0]!=(char *)0)
	        sim_args[index++] = strdupa (exec_name);

	    sim_args[index++] = strdupa (args);
	    sim_args[index++] = 0;
	}
	else
	{
	    sim_args[index++] = 0;
	}

        if (qsim_debug)
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
	    error ("hexagon_sim_exec_simulation: Unable to execute hexagon-sim!");

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

void
hexagon_catch_alarm (int sig)
{
    alarmed = 1;
}
#if 0
void
hexagon_catch_sigchld (int signum)
{
    int pid, status;
    pid = waitpid (-1, &status, WNOHANG);
    if (pid < 0)
	perror ("waitpid");

    sim_died = 1;
}
#endif

/*
 * function: hexagon_create_inferior
 * description:
 *        - Primary function of this target.
 *        - Starts the simulator and forms a connection.
 */
static void hexagon_create_inferior (struct target_ops *ops, char *exec_file,
				   char *args, char **env, int tty)
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
	return;

    hexagon_sim_exec_simulation("hexagon-sim", exec_file, args, env, portid);
    sprintf (target_port, ":%d", portid);
    push_remote_target(target_port, 1);

    return;
}

static void
hexagon_sim_close (int quitting)
{
    if (qsim_debug)
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
 * function: hexagon_sim_open
 * description:
 *      - Called when the user selects the hexagon-sim target architecture.
 */
static void
hexagon_sim_open (char *args, int from_tty)
{
    if (qsim_debug)
    {
        printf_filtered ("%s\n", __func__);
    }
    target_preopen (1);
    current_hexagon_target = TARGET_NAME;
	
}

static int
hexagon_can_use_hw_breakpoint (int type, int cnt, int othertype)
{
  return 1;
}

static void
hexagon_command (char *args, int from_tty)
{
  printf_unfiltered (_("\"hexagon_\" prefix must be followed by the name of an HEXAGON command.\n"));
  help_list (hexagon_cmdlist, "hexagon ", -1, gdb_stdout);
}

static void
hexagon_watch_command (char *args, int from_tty)
{
    char *substr;
    long cycle = 0;
    long interval = 0;
    long address = 0;
    long pagesize = 0;
    char msg [80]  = {'0'};

    substr = strtok(args, " \t\r\n");
    if (substr != NULL)
    {
	if (!(strcmp (substr, "cycle")))
	{
	    if ((substr = strtok(NULL, " \t\r\n"))!=NULL)
	    {
		/* if not base 10 check for base 16 */
		if ((cycle = strtol(substr, NULL, 10))==NULL)
		  cycle = strtol(substr, NULL, 16);
	    }
	    if ((substr = strtok(NULL, " \t\r\n"))!=NULL)
	    {
		if ((interval = strtol(substr, NULL, 10))==NULL)
		  interval = strtol(substr, NULL, 16);
	    }

	    sprintf(msg, "qRegWatch,%d,%x,%x", WATCHPOINT_PCYCLE,
		    cycle, interval);
	    putpkt (msg);
  	    getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
	    
	}
	else if (!(strcmp (substr, "tlbmiss")))
	{
	    if ((substr = strtok(NULL, " \t\r\n"))!=NULL)
	    {
		if ((address = strtol(substr, NULL, 10))==NULL)
		  address = strtol(substr, NULL, 16);
	    }
	    if ((substr = strtok(NULL, " \t\r\n"))!=NULL)
	    {
		if ((pagesize = strtol(substr, NULL, 10))==NULL)
		  pagesize = strtol(substr, NULL, 16);
	    }
	    sprintf(msg, "qRegWatch,%d,%x,%x", WATCHPOINT_TLBMISS,
		    address, pagesize);
	    putpkt (msg);
  	    getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
	}
    }
}




/* Map global register name to its register number */
static int 
hexagon_global_regnum_from_name(char *regname, int *index)
{
  int i=0;
  int len= strlen(regname);
  int regnum = -1;
  int numGblRegs;
  if(regname == NULL)
    return -1;

  while (globalRegSetInfo[i].reg_name != NULL)
  {
    if ((regname != NULL)                    && 
	(len == strlen (globalRegSetInfo[i].reg_name)) && 
	(strncmp (globalRegSetInfo[i].reg_name,regname,len) == 0))
    {
        regnum =  globalRegSetInfo[i].index ;
        break;
    }
    i++;
  }

  *index = i ;
  return regnum;
}

/* Print reg info in the following format
   register name  0xff 255 */ 
static void  
hexagon_print_reg_info(char  *regname,  LONGEST regvalue)
{

  if(regname == NULL)
    warning("Invalid Register Name\n");

  if(strlen(regname) <=7)
     fprintf_filtered(gdb_stdout,"%s\t\t", regname);
  else
     fprintf_filtered(gdb_stdout,"%s\t", regname);
  
  fprintf_filtered(gdb_stdout, "0x%x", (unsigned int) regvalue);
  fprintf_filtered(gdb_stdout, "\t");  
  
  // print the value of the register in decimal
  fprintf_filtered(gdb_stdout, "%u", (unsigned int) regvalue);

  fprintf_filtered(gdb_stdout, "\n");
  
  gdb_flush (gdb_stdout);
}



/* storage to hold the display buffer */
static char display_buf[GDB_RSP_RIL_INFO_MAX_SIZE/2 +1];

/* print out if no timeout occured and got non null string */
#define PRINT_MSG \
   if ((response[0] != 0) && ((strstr(response, "timeout") == NULL)))\
    {\
      n = min (strlen (response) / 2, sizeof (display_buf));\
      result = hex2bin (response, display_buf, n);\
      display_buf [result] = '\0';\
      printf_filtered("%s\n", display_buf);\
    }


char* gettargetresponse ()
{
 int n;
 int result;
  
 if ((response[0] != 0) && ((strstr(response, "timeout") == NULL)))
    {
      n = min (strlen (response) / 2, sizeof (display_buf));
      result = hex2bin (response, display_buf, n);
      display_buf [result] = '\0';
    }
    return display_buf;
}

static void
hexagon_rtos_info_command (char *args, int from_tty)
{
  int result;
  int n = 0;                    /* position in display_buf */
  
  if(current_hexagon_target == NULL)
    error ("Program has no rtos info. Load program first");
    
  putpkt ("qhexagon_RtosInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  PRINT_MSG;
}

static ULONGEST hexagon_get_pagetable_size ()
{
  char *pResponse;
  long long unsigned pSize = 0;

  putpkt ("qPageTableSize");
  getpkt ( &response, &gdb_rsp_ril_info_max_size , 0);
 
  if(response == NULL)
    error ("Invalid Page Table size string returned from DRIL.");

  pResponse = gettargetresponse();

  if(pResponse == NULL)
    error ("Invalid Page Table size string returned from DRIL.");

  if(strcmp(pResponse,"PageTable command supported only for rtos apps") == 0)
    error ("PageTable command supported only for rtos apps");

  /* Check for valid page size */
  sscanf(pResponse,"%llu",&pSize);

  if (pSize <= 0)
     error ("Invalid Page Table size returned from DRIL.");

  return pSize;

}
static void
hexagon_pagetable_info_command (char *args, int from_tty)
{
  int result;
  int n              = 0;  /* position in display_buf */
  ULONGEST pSize     = 0;
  char *pageBuf      = NULL;
  char * displayBuf  = NULL;

  if(current_hexagon_target == NULL)
    error ("Program has no pagetable info. Load program first");

  /* Get the size of page table data */  
  pSize = hexagon_get_pagetable_size() ;

  /* Allocate memory chunk for page table 
   * We allocate double the size as the msg is hex encoded*/
  pageBuf    = (char*) malloc (pSize*2+1);
   /* Allocate memory chunk for page table display */ 
  displayBuf = (char*) malloc (pSize+1);

  if ((pageBuf == NULL) || (displayBuf == NULL))
    error ("Page Table memory allocation failed");
  
  putpkt ("qPageTableInfo");
  getpkt (&pageBuf, &gdb_rsp_ril_info_max_size , 0);

  /* Display page table info */
  if ((pageBuf[0] != 0) && ((strstr(pageBuf, "timeout") == NULL)))
  {
    n = min(strlen(pageBuf)/2, pSize+1);
    result = hex2bin (pageBuf, displayBuf, n);
    displayBuf [result] = '\0';
    printf_filtered("%s\n", displayBuf);
  }
 
  /* Free resources allocated locally */
  if (pageBuf != NULL)
  {  
    free (pageBuf);
    pageBuf = NULL;
  }

  if (displayBuf != NULL)
  {  
    free (displayBuf);
    displayBuf = NULL;
  }

}


static void
hexagon_tlb_info_command (char *args, int from_tty)
{
  int result;
  int n = 0;                    /* position in display_buf */

  if(current_hexagon_target == NULL)
    error ("Program has no tlb info. Load program first");
    
  putpkt ("qTLBInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size, 0);
  PRINT_MSG;
}



static LONGEST
get_globalregs_buffer (char* args, int printInfo)
{
  int  result, namelen, numValidGlobalRegs=0;
  int  n = 0,  i =0 ;                  
  int  lenDelimiter=2 , lenValue=8 , lenData, lenResponse;
    
  char sDisplayBuf[SET_CMD_BUFFER_SIZE];
  char * sDelimiter; 
  char * sRegData = response;
  char sRegNames[NUM_GLOBAL_REGS][MAXREGNAMELEN]; 
  LONGEST  regValues[NUM_GLOBAL_REGS];

  putpkt ("qGlobalRegsInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  
  lenResponse  = strlen(response); 

 /*RSP for each register = register name:value */
  /* value for each reigster is represented as 8 nibbles */
  for(n =0, lenData =0; 
     (n<NUM_GLOBAL_REGS) && (lenData< lenResponse); 
     n++ )
  {
    if ((response[0] != 0) && 
	((strstr(response, "timeout") == NULL)))
    {
      /*read in the register name till : is seen */
      for(i=0, namelen = 0;;i+=2, namelen+=2)
         if((sRegData[i] == '3') && (sRegData[i+1] == 'a'))
          break;
      
      result  = hex2bin(sRegData, sRegNames[n], namelen/2);
      sRegNames[n][result] = '\0';

      /* read reg value after strlen(reg name) + strlen("3a"). 
         0x3a = ':' the delimiter between register name and its value*/
      result = hex2bin (sRegData+namelen+lenDelimiter,
                        sDisplayBuf, lenValue/2);
      sDisplayBuf [result] = '\0';
      

      /* get the register offset to store the register value */
      i  =  hexagon_global_regnum_from_name(sRegNames[n], &result);

      if(i == -1)
      {
        hexagon_print_reg_info("Invalid Register Name", 0);
        return -1;
      }

      /* Get the value of the register */
      regValues[i] = extract_signed_integer((const gdb_byte *)&sDisplayBuf,
					    lenValue/2, BFD_ENDIAN_LITTLE);
    
      /* get the length of the string read so far */  
      lenData += namelen+lenDelimiter+lenValue;

      /* advance the data to read in the next register set info */
      /* advance pointer by reg name length + : + 8 bytes of reg value */ 
      sRegData+=namelen;
      sRegData+=lenDelimiter;
      sRegData+=lenValue;

      /* increase the valid reg count */
      numValidGlobalRegs++;
    }
  }
  /* get particular register value , get the index into the array*/
  /* passing 0 as 2nd arg will return register offset in arch defn */
  if((printInfo != 1) && (args != NULL))
  {
     n =  hexagon_global_regnum_from_name(args, &i);
     return regValues[n];
  }
  else if(printInfo == 1)
  {

    if(args != NULL)
    {
       /* print particular register , get the index into the array*/
       /* passing 0 as 2nd arg will return register offset in arch defn */
       n =  hexagon_global_regnum_from_name(args, &i);
       if(n != -1)
         hexagon_print_reg_info(globalRegSetInfo[i].reg_name, regValues[n] );
       else
         hexagon_print_reg_info("Invalid Register Name", 0);

    }
   else
   {
   
    /* Print all the registers */
    for(result =0; result<numValidGlobalRegs ; result++)
    {
      /* get the register offset to store the register value */
      n  =  hexagon_global_regnum_from_name(sRegNames[result], &i);
      hexagon_print_reg_info(globalRegSetInfo[i].reg_name, regValues[n]);
    }
   }
  }

  return 0;  
}


static void
hexagon_globalregs_info_command (char *args, int from_tty)
{
  int  result, namelen, numValidGlobalRegs=0;
  int  n = 0,  i =0 ;                  
  int  lenDelimiter=2 , lenValue=8 , lenData, lenResponse;
  
  char sDisplayBuf[SET_CMD_BUFFER_SIZE];
  char * sDelimiter; 
  char * sRegData = response;
  char sRegNames[NUM_GLOBAL_REGS][MAXREGNAMELEN]; 
  LONGEST  regValues[NUM_GLOBAL_REGS];

  if(current_hexagon_target == NULL)
    error ("Program has no global registers. Load program first");

  putpkt ("qGlobalRegsInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  if(strncmp(response,"E00", strlen("E00"))==0)
     error ("Error reading register values from simulator!"); 
     
  lenResponse  = strlen(response);
  
  /*RSP for each register = register name:value */
  /* value for each reigster is represented as 8 nibbles */
  for(n =0, lenData =0; 
     (n<NUM_GLOBAL_REGS) && (lenData< lenResponse); 
      n++)
  {
    if ((response[0] != 0) && 
	((strstr(response, "timeout") == NULL)))
    {
      /*read in the register name till : is seen */
      for(i=0, namelen = 0;;i+=2, namelen+=2)
         if((sRegData[i] == '3') && (sRegData[i+1] == 'a'))
          break;
      
      result  = hex2bin(sRegData, sRegNames[n], namelen/2);
      sRegNames[n][result] = '\0';

      /* read reg value after strlen(reg name) + strlen("3a"). 
         0x3a = ':' the delimiter between register name and its value*/
      result = hex2bin (sRegData+namelen+lenDelimiter,
                        sDisplayBuf, lenValue/2);
      sDisplayBuf [result] = '\0';
      

      /* get the register offset to store the register value */
      i  =  hexagon_global_regnum_from_name(sRegNames[n], &result);

      if(i == -1)
      {
        hexagon_print_reg_info("Invalid Register Name", 0);
        return;
      }

      /* Get the value of the register */
      regValues[i] = extract_signed_integer((const gdb_byte *)&sDisplayBuf,
					    lenValue/2, BFD_ENDIAN_LITTLE);
    
      /* get the length of the string read so far */  
      lenData += namelen+lenDelimiter+lenValue;

      /* advance the data to read in the next register set info */
      /* advance pointer by reg name length + : + 8 bytes of reg value */ 
      sRegData+=namelen;
      sRegData+=lenDelimiter;
      sRegData+=lenValue;

      /* increase the valid reg count */
      numValidGlobalRegs++;
    }
  }
  
  /* Print register Info */

  if(args != NULL)
  {
    /* print particular register , get the index into the array*/
    /* passing 0 as 2nd arg will return register offset in arch defn */
    n =  hexagon_global_regnum_from_name(args, &i);
    if(n != -1)
      hexagon_print_reg_info(globalRegSetInfo[i].reg_name, regValues[n] );
    else
      hexagon_print_reg_info("Invalid Register Name", 0);
  }
  else
  {
    /* Print all the registers */
    for(result =0; result<numValidGlobalRegs ; result++)
    {
      /* get the register offset to store the register value */
      n  =  hexagon_global_regnum_from_name(sRegNames[result], &i);
      hexagon_print_reg_info(globalRegSetInfo[i].reg_name, regValues[n]);
    }
  }
  
}

/* Get the value of a particular register */
int get_hexagon_globalregs_value (char *args)
{

  LONGEST regVal;

  if(current_hexagon_target == NULL)
    error ("Program has no global registers. Load program first.");

  if(args == NULL)
   error ("Empty register name !.\n");
 
  return get_globalregs_buffer (args,0);
}

static void
hexagon_thread_details_info_command (char *args, int from_tty)
{
  int result;
  int num;
  ptid_t ptid;
  int n = 0;                    /* position in display_buf */
  char message[30];

  if (!args)
    {
      error ("Thread ID not specified");
      return;
    }

  sscanf(args, "%d", &num);

  /* convert into tid */
  ptid = thread_id_to_pid (num);

  if (ptid_equal(ptid, pid_to_ptid(-1)))
    {
      error ("Thread ID %d not known.", num);
      return;
    }


  sprintf(message, "qThreadDetailInfo,%d", PIDGET(ptid));
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  PRINT_MSG;
}


void 
setglobalregistervalue(char *args, int from_tty, 
                       struct cmd_list_element *cmdlist)
{
  char *regName, *regValue, *substr,*hexagon_regbuf = NULL;
  char tmpbuf[9], message[256];
  int regNum, len=0, result;
  int newValue;   //LONGEST newValue;
  struct expression *expr;
 
  if(current_hexagon_target == NULL)
    error("Program has no global registers. Load program first");
  else if(hexagon_globalregsInfo == NULL)
    error("Invalid register name/value.");
					
  /*Check for blanks in the user entered string */ 
   substr = strtok((char*)hexagon_globalregsInfo, " ");
   if(substr != NULL)
   {		
     hexagon_regbuf = (char*)malloc(sizeof(char) * SET_CMD_BUFFER_SIZE);

     if(hexagon_regbuf == NULL)
        error("Memory allocation for register buffer failed !");

     /*clear the temporary buffer */
     memset(hexagon_regbuf,0,SET_CMD_BUFFER_SIZE);

     /*extract user string with spaces removed */
     strcat(hexagon_regbuf, substr);
     while ( (substr=strtok(NULL, " ")) != NULL)
       strcat(hexagon_regbuf, substr);
   }
   else
     hexagon_regbuf=hexagon_globalregsInfo; 

  /* Extract the register name whose value is to be set  */
  regName = strtok(hexagon_regbuf, "=");
  if(regName == NULL)  
    error("Invalid register name/value.");
  
  /* Get the global register index for this register */
  regNum =  hexagon_global_regnum_from_name(regName, &result);
  if(regNum == -1)  
    error("Invalid register name : %s.",regName );
  
  /* get the register value to be set  */
  regValue =hexagon_regbuf+strlen(regName)+1;

  /* parse the new register value and get the decimal value
     expr->elts[2].longconst stores the converted values */
  expr =  parse_expression (regValue);

  if(!expr)
    return;
  
  /* get the decimal value to be set */
  newValue = expr->elts[2].longconst;

  /* get the reg num and reg value to be set into string format */
  sprintf(message, "qSetGlobalRegsInfo,%x,", regNum);

  sprintf(tmpbuf,"%02x", newValue & 0x0ff);
  strcat(message,tmpbuf);
  sprintf(tmpbuf,"%02x",(newValue>>8) & 0x0ff);
  strcat(message,tmpbuf);
  sprintf(tmpbuf,"%02x",(newValue>>16) & 0x0ff);
  strcat(message,tmpbuf);
  sprintf(tmpbuf,"%02x",(newValue>>24) & 0x0ff);
  strcat(message,tmpbuf);

  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  if(strcmp(response,"E02") == 0)
    error ("'%s' is a read-only register.",regName);

  gdb_flush (gdb_stdout);

  /* free resources allocated to hexagon_globalregsinfo */
  if(hexagon_regbuf != NULL)
    free (hexagon_regbuf); 

}  

void init_globalregs()
{
   /* Chain containing all defined show subcommands.  */
   struct cmd_list_element *hexagon_showlist = NULL;

   /* allocate memory for control variable for setshow command */
   hexagon_globalregsInfo  = (char*)malloc(sizeof(char) * SET_CMD_BUFFER_SIZE);
  
 
    if(hexagon_globalregsInfo ==  NULL)
        error ("Memory allocation for register buffer failed!");
     /* set global regs */
      add_setshow_string_noescape_cmd ("globalregs",         /* name */
		   no_class,             /* class category for help list */
		   &hexagon_globalregsInfo,    /* address of the variable being controlled by this */
           "set HEXAGON global register to the specified value", /* documentation for this cmd */
           "set HEXAGON global register to the specified value", /* documentation for this cmd */
           "set HEXAGON global register to the specified value", /* documentation for this cmd */
		   &setglobalregistervalue,  /* set cmd func ptr */
		   NULL,                     /* show cmd fn  ptr */
		   &setlist,
		   &hexagon_showlist);


}

/*
 * function: setHEXAGON_targetargs
 * description:
 * 	- takes a single input string and breaks it up into component
 * 	  parts suitable execvp
 */
void  
setHEXAGON_targetargs (char * args, int tty, struct cmd_list_element *c)
{
    ULONGEST addr = *(unsigned long *)c->var;
    char *simargs = (char *) addr;
    char **targs = hexagon_targetargsInfo;
    int index = 0; 

    simargs = strtok (simargs, " \t\n");
    while (simargs)
    {
	targs[index++] = simargs;
	simargs = strtok(0, " \t\n");

	gdb_assert (index < SIM_ARG_MAX);
    }
}

static void
hexagon_tclfe_set (char *args, int from_tty,
		 struct cmd_list_element *c)
{
/* Note: the var_type is var_boolean so this will only be 1 or 0 */
   HEXAGON_tcl_fe_state = *(int *)(c->var);
}

static void
hexagon_tclfe_show (char *args, int from_tty,
		 struct cmd_list_element *c)
{
   return;
}

void init_targetargs()
{
   /* Chain containing all defined show subcommands.  */
   struct cmd_list_element *targetargsshowlist = NULL;

   /* set global regs */
   add_setshow_string_noescape_cmd ("targetargs",         /* name */
		            no_class,      /* class category for help list */
		            &hexagon_targetargsInfo[0],    /* control var address */
                    "set HEXAGON target args", /* doc for this cmd */
                    "set HEXAGON target args", /* doc for this cmd */
                    "set HEXAGON target args", /* doc for this cmd */
		            &setHEXAGON_targetargs, /* set cmd func ptr */
		            NULL,                 /* show cmd fn  ptr */
		            &setlist,
		            &targetargsshowlist);

   add_setshow_boolean_cmd ("tclfe",             /* name */
		    no_class,            /* class category for help list */
		    &HEXAGON_tcl_fe_state,	 /* control var address */
                    "Turn on or off HEXAGON GDB's TCL interpreter", 
                    "Turn on or off HEXAGON GDB's TCL interpreter", 
                    "Turn on or off HEXAGON GDB's TCL interpreter", 
		    hexagon_tclfe_set,	 /* Add function to set */
		    NULL,	 /* Add function to print show */
		    &setlist,
		    &showlist);

}


void
set_HEXAGON_hwthread_debug(char *args, int from_tty, struct cmd_list_element *cmdlist)
{

  char message[SET_CMD_BUFFER_SIZE] = {'\0'};
  
  if (current_hexagon_target == NULL)
    error ("Cannot set interrupts. Load program first.");

  if (hexagon_hwthread_debug)
    {
      sprintf(message, "hexagon_hwThreadDebug,%d", 1);
    }
  else
    {
      sprintf(message, "hexagon_hwThreadDebug,%d", 0);
      
    }
    
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  
  gdb_flush (gdb_stdout);
}


void 
set_HEXAGON_interrupt(char *args, int from_tty, struct cmd_list_element *cmdlist)
{
  char message[SET_CMD_BUFFER_SIZE] = {'0'};
  char buf[256] = {'0'};
  char *substr;
  int interruptNum; //LONGEST interruptNum;
  struct expression* expr;
 
  if (current_hexagon_target == NULL)
    error ("Cannot set interrupts. Load program first.");
 
  if (hexagon_Interrupt == NULL)
     error ("Usage: set interrupt <interrupt no> <delay> <period>.");
  
  /*Extract watch point type*/ 
  substr = strtok(hexagon_Interrupt, " ");
  if(substr != NULL)
  {
    /* parse user input and get decimal values, This
       function does error checking on the value entered */
    expr =  parse_expression (substr);

    if(!expr) return;
  
    /* get the decimal value to be set */
    interruptNum = expr->elts[2].longconst;

    /* Check if interrupt num specified lies in the valid range */ 
    if((interruptNum <0)) /* || (interruptNum > MAX_INTERRUPT)) */
       error( "Invalid Interrupt Number %d, set interrupts 0 - 32.",
        interruptNum);

    /* get the reg num and reg value to be set into string format */
    sprintf(message, "qGenInterrupt,%x", interruptNum);

    /* get the period and delay if any */
    while ((substr=strtok(NULL, " ")) != NULL)
    {

      /* parse user input and get decimal values, This
      function does error checking on the value entered */
      expr =  parse_expression (substr);

      if(!expr) return;
  
      /* get the decimal value to be set */
      interruptNum = expr->elts[2].longconst;
    
      /* get the delay/period to be set into string format */
      sprintf(buf, ",%x", interruptNum);
      strcat(message,buf);

     }
   }
   else
    error ("Usage: set interrupt <interrupt no> <delay> <period>.");

  
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  gdb_flush (gdb_stdout);

}


void init_set_hwthread_debug_only_mode()
{
  /* Chain containing all defined show subcommands.  */
   struct cmd_list_element *hexagon_showlist = NULL;


   /* set hwthrdbg */
   add_setshow_zinteger_cmd ("hwthrdbg",        /* name */
                    no_class,          /* class category for help list */
                    &hexagon_hwthread_debug, /*address of the variable being controlled by this */
                    "switch to hw thread debug more", /* documentation for this cmd */
                    "switch to hw thread debug more", /* documentation for this cmd */
                    "switch to hw thread debug more", /* documentation for this cmd */
                    &set_HEXAGON_hwthread_debug,          /* set cmd func ptr */
                    NULL,                     /* show cmd fn  ptr */
                    &setlist,
                    &hexagon_showlist);
   
   
}


void init_set_interrupt()
{
   /* Chain containing all defined show subcommands.  */
   struct cmd_list_element *hexagon_showlist = NULL;
   struct cmd_list_element *c  ;

   /* for set interrupt command - contains the user typed string*/
   hexagon_Interrupt = (char*)malloc(sizeof(char) * SET_CMD_BUFFER_SIZE);
 
   if(hexagon_Interrupt == NULL)
        error ("Memory allocation for interrupt buffer failed !");

   /* set interrupt */
   add_setshow_string_noescape_cmd ("interrupt",               /* name */
		             no_class,                 /* class category for help list */
		             &hexagon_Interrupt,             /*address of the variable being controlled by this */
       	             "generate interrupt in HEXAGON core", /* documentation for this cmd */
       	             "generate interrupt in HEXAGON core", /* documentation for this cmd */
       	             "generate interrupt in HEXAGON core", /* documentation for this cmd */
		             &set_HEXAGON_interrupt,          /* set cmd func ptr */
		             NULL,                     /* show cmd fn  ptr */
		             &setlist,
		             &hexagon_showlist);





/*c = (add_set_cmd ("interrupt", no_class, var_string,
		   (char *) &hexagon_Interrupt,
		   "Set interrupt in HEXAGON core", 
           &sethistlist));
  set_cmd_sfunc (c, set_HEXAGON_interrupt);      
  add_show_from_set (c, &showlist);
  set_cmd_completer (c, command_completer);
  */ 
}

void
hexagon_proc_cycles_info_command(char *args, int from_tty)
{
  unsigned long long cycle_count = 0;
  int i;
  if(current_hexagon_target == NULL)
    error ("Program has no proc cycle info. Load program first");
    
  putpkt ("qProcCyclesInfo");
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  if (strlen(response) == 16) {
    // got 16 nibbles get the cycle count and print
    for (i=0; i<8; i++) {
      cycle_count |= (((fromhex(response[i*2]) << 4) |
		       (fromhex(response[i*2 + 1]))) << (i*8));  
    }
    printf_filtered("%lld\n", cycle_count);
  }
  else {
    error ("Processor cycles not available");
  }
}


void
hexagon_thread_cycles_info_command(char *args, int from_tty)
{
  unsigned long long cycle_count = 0;
  int i, num;
  ptid_t ptid;
  char message[30];
  
  if(current_hexagon_target == NULL)
    error ("Program has no thread cycles info. Load program first");
    
  if (!args) {
    error ("Thread ID not specified");
    return;
  }

  sscanf(args, "%d", &num);

  /* convert into tid */
  ptid = thread_id_to_pid (num);
  
  if (ptid_equal(ptid, pid_to_ptid(-1))) {
    error ("Thread ID %d not known.", num);
    return;
  }

  /* send the message out */
  sprintf(message, "qThreadCyclesInfo,%d", PIDGET(ptid));
  putpkt (message);

  /* get the response */
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

  if (strlen(response) == 16) {
    // got 16 nibbles get the cycle count and print
    for (i=0; i<8; i++) {
      cycle_count |= (((fromhex(response[i*2]) << 4) |
		       (fromhex(response[i*2 + 1]))) << (i*8));  
    }
    printf_filtered("0x%llx\n", cycle_count);
  }
  else {
    error ("Thread cycles not available");
  }
}


void
hexagon_msg_channels_info_command (char *args, int from_tty)
{
 int result;
 int n = 0;                    /* position in display_buf */
 
 if(current_hexagon_target == NULL)
   error ("Program has no message channels info. Load program first");
    
 putpkt ("hexagon_MsgChannelInfo");
 getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
 if(strncmp(response,"E00", strlen("E00"))==0)
    warning ("No message channel information.");
 else   
    PRINT_MSG;

}

void
hexagon_msg_queues_info_command (char *args, int from_tty)
{
 int result;
 int n = 0;                    /* position in display_buf */
 char msgStr[256]={0};
 char pktStr[256]={0};
 
 
 if(current_hexagon_target == NULL)
   error ("Program has no message queues info. Load program first");

  while(args && (*args != '\0') && ((*args == ' ') || (*args == '\t'))) args++;
  if (!args || (*args == '\0')) {
    error ("Message Channel name not specified.");
    return;
  }

  // ASG - bug 1436.  Need to pass whole string
  //sscanf(args, "%s", &msgStr[0]);
    /* send the message out */
  //sprintf(pktStr, "hexagon_MsgQueueInfo,%s", msgStr);
  sprintf(pktStr, "hexagon_MsgQueueInfo,%s", args);
  putpkt (pktStr);

  /* get the response */
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  
  if(strncmp(response,"E00", strlen("E00"))==0)
    warning ("Error reading message queue information. Check message queue name!");
  else   
    PRINT_MSG;

}

void
hexagon_timers_info_command (char *args, int from_tty)
{
 int result;
 int n = 0;                    /* position in display_buf */
 
 if(current_hexagon_target == NULL)
   error ("Program has no timer info. Load program first");
    
 putpkt ("hexagon_TimerInfo");
 getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
 if(strncmp(response,"E00", strlen("E00"))==0)
    warning ("No timer information.");
 else   
    PRINT_MSG;

}


void
hexagon_mutex_info_command (char *args, int from_tty)
{
 int result;
 int n = 0;                    /* position in display_buf */
 int mutexID;
 char pktStr[256]={0};
 struct expression * expr;
 
 if(current_hexagon_target == NULL)
   error ("Program has no mutex info. Load program first");

  if (!args) {
    error ("Mutex ID not specified.");
    return;
  }

  /* parse the mutex ID and get the decimal value
     expr->elts[2].longconst stores the converted values */
  expr =  parse_expression (args);

  if(!expr)
   warning("Invalid Mutex ID specified !");;
  
  /* get the decimal value to be set */
  mutexID = expr->elts[2].longconst;
    
    /* send the message out */
  sprintf(pktStr, "hexagon_MutexInfo,%x", mutexID);
  putpkt (pktStr);

  /* get the response */
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
  
  if(strncmp(response,"E00", strlen("E00"))==0)
    warning ("Error reading mutex information. Check mutex ID!");
  else   
    PRINT_MSG;

}



void  
hexagon_interrupt_map_info_command(char *args, int from_tty)
{
 
 int result;
 int n = 0;                    /* position in display_buf */
 
 if(current_hexagon_target == NULL)
   error ("Program has no interrupt map info. Load program first");
    
 putpkt ("qInterruptMap");
 getpkt (&response, &gdb_rsp_ril_info_max_size , 0);
 PRINT_MSG;

}


void 
hexagon_interrupt_remap(char *args, int from_tty)
{
  char message[256] = {'0'};
  char tmpbuf [25]  = {'0'};
  int interruptNum, tokValue = 0; 
  char* substr, *pTmp;
  struct expression* expr;

  if(current_hexagon_target == NULL)
    error ("Cannot remap interrupts. Load program first.");


  if (args == NULL)
  {
    error ("Interrupt ID not specified.");
    return;
  }

  substr = strtok(args, " ");
  if(substr != NULL)
  {
     sprintf(message, "qInterruptRemap,%s,", substr );

     while ((substr=strtok(NULL, " ")) != NULL)
     {

        tokValue +=1;

       /* parse user input and get decimal values, This
        function does error checking on the value entered */


      expr =  parse_expression (substr);

      if(!expr) return;
  
      /* get the decimal value to be set */
      interruptNum = expr->elts[2].longconst;
      
       /* Check if interrupt num specified lies in the valid range */ 
       if((interruptNum <0)) /* || (interruptNum > MAX_INTERRUPT)) */
         error ("Invalid Interrupt Number %d, set interrupts 0 - 32.",
                interruptNum);
       else if(tokValue >=2)
         error("Invalid Interrupt remap command\nUsage:  remap <name> <value>");
        
        /* Construct the RSP */
        sprintf(tmpbuf,"%x", interruptNum);
        strcat(message,tmpbuf);         

     }
  }
 
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

}
void 
hexagon_interrupt_reset (char *args, int from_tty)
{
  char message[256] = {'0'};
  char tmpbuf [25]  = {'0'};
  char *pintType[3] = {"both", "periodic", "pin"};
  char *substr;
  int interruptNum, nargs = 0; 
  struct expression * expr;
  
  if(current_hexagon_target == NULL)
    error ("Cannot reset interrupts. Load program first.");

  if (args == NULL)
    error ("Interrupt ID not specified");

  substr = strtok(args, " ");
  if(substr != NULL)
  {
      nargs +=1;
      //interruptNum = strtol(substr,pTmp ,10);

      if(nargs == 1)
       { 
          expr =  parse_expression (substr);
          if(!expr)
              return;
          /* get the decimal value to be set */
          interruptNum = expr->elts[2].longconst;

          /* Check if interrupt num specified lies in the valid range */ 
          if((interruptNum <0)) /* || (interruptNum > MAX_INTERRUPT)) */
              error ("Invalid Interrupt Number %d, set interrupts 0 - 32.\n",
                  interruptNum);
          /* Construct the RSP */
          sprintf(tmpbuf, "qInterruptReset,%x", interruptNum);
          memset(message,0,strlen(message));
          strcat (message,tmpbuf);
       }
        
       /* get the type of interrupt to be cleared
        * default = both */
       while ((substr=strtok(NULL, " ")) != NULL)
       {
         nargs +=1;
          
         if(nargs > 2)
           error ("Usage: reset-interrupt <number> [both] <periodic> <pin>.\n");
           
         /* Search for interrupt type to be cleared */
         for(interruptNum=0; interruptNum < 3; interruptNum++)
         {
           if(strcmp(substr, pintType[interruptNum])==0)
             break;
         }
         switch (interruptNum)
         {
           case 0:
           case 1:
           case 2:
            sprintf(tmpbuf, ",%x", interruptNum);
            strcat (message,tmpbuf);
           break;

           default:
             error ("Invalid type '%s' specified for reset interrupt.\n",substr);
             break;
          }
      }// while substr-strtok ..
  } //if(substr != NULL)
  else
     error ("Usage: reset-interrupt <number> [both] <periodic> <pin>.\n");
  
  putpkt (message);
  getpkt (&response, &gdb_rsp_ril_info_max_size , 0);

}

void 
init_set_interruptmap(void)
{

  struct cmd_list_element *c;
  c = add_com ("remap", no_class, hexagon_interrupt_remap,
	       "Remap an interrupt.");
  set_cmd_completer (c, location_completer);
}
void 
init_reset_interrupt(void)
{

  struct cmd_list_element *c;

 /* add_prefix_cmd ("reset", no_class, hexagon_interrupt_reset,
                  concat("Use 'reset interrupt' "\
                  "to reset periodic/pin interrupts.", NULL),
                  &resetlist, "reset ", 1, &cmdlist);
  c = add_cmd ("interrupt", no_class, hexagon_interrupt_reset,
           "Usage: reset interrupt <number> [both] <periodic> <pin>.",
           &resetlist);*/
  
  c = add_com ("reset-interrupt", no_class, hexagon_interrupt_reset,
	       "Reset an interrupt.\n"\
           "Usage: reset-interrupt <number> [both] <periodic> <pin>."); 
  set_cmd_completer (c, location_completer);
}

void 
init_hexagon_commands (void)
{
  /* initialize the global registers */
  init_globalregs();    
 
  /* initialize target args */
  init_targetargs();

  init_set_interrupt();
  init_reset_interrupt();
  init_set_interruptmap();

  /* set mode for h/w debug only */
  init_set_hwthread_debug_only_mode();
}


void
_initialize_hexagon_sim (void)
{
  struct target_ops *t;

  t = XZALLOC (struct target_ops);

  add_prefix_cmd ("hexagon", class_support, hexagon_command,
		  _("Various HEXAGON specific commands."),
		  &hexagon_cmdlist, "hexagon ", 0, &cmdlist);

  add_cmd ("watch", class_breakpoint, hexagon_watch_command,
_("set either a cycle count or tlbmiss breakpoint.\n\
	hexagon watch cycle <cycle count>\n\
	hexagon watch tlbmiss <32 bit addr> <page size in bits>\n"),
	   &hexagon_cmdlist);


  t->to_shortname = "hexagon-sim";
  t->to_longname = "Qualcomm Hexagon Simulation interface";
  t->to_doc = "Qualcomm Hexagon Simulation interface, used for debugging software \n\
running on the Hexagon (hexagon-sim) simulator";

  t->to_open = hexagon_sim_open;
  t->to_close = hexagon_sim_close;
  t->to_can_run = hexagon_sim_can_run;
  t->to_create_inferior = hexagon_create_inferior;
  t->to_can_use_hw_breakpoint = hexagon_can_use_hw_breakpoint;

  /* Register target.  */
  add_target (t);
  hexagon_sim_target = t;
  current_hexagon_target = TARGET_NAME;

  /* info hexagon-rtos */
  add_info ("hexagon-rtos", hexagon_rtos_info_command,
	    "HEXAGON RTOS information");

  /* info pagetable */
  add_info ("pagetable", hexagon_pagetable_info_command,
	    "Pagetable information");

  /* info TLB */
  add_info ("tlb", hexagon_tlb_info_command,
	    "TLB information");

  /* info globalregs */
  add_info ("globalregs", hexagon_globalregs_info_command,
	    "List of global registers and their contents");

  /* thread details informatino */
  add_info ("thread-details", hexagon_thread_details_info_command,
	    "Details information for a thread-id");

  /* processor cycle count information */
  add_info ("proc-cycles", hexagon_proc_cycles_info_command,
            "Processor cycle count");

  /* processor cycle count information */
  add_info ("thread-cycles", hexagon_thread_cycles_info_command,
            "Thread cycle count");
      
  /*Interrupt map  information */
  add_info ("interrupt-map", hexagon_interrupt_map_info_command,
            "Interrupt map");
              /*Qube msg channel information */
  add_info ("msg-channels", hexagon_msg_channels_info_command,
            "RTOS Message Channels Info");
            
  /*Qube msg queue information */
  add_info ("msg-queue", hexagon_msg_queues_info_command,
            "RTOS Message Queues Info");
             
   /*Qube timers information */
  add_info ("timers", hexagon_timers_info_command,
            "RTOS Timers Info");
            
  /*Qube msg queue information */
  add_info ("mutex", hexagon_mutex_info_command,
            "RTOS Mutex Info");
            
  init_hexagon_commands(); 
}
