/*****************************************************************
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
*****************************************************************/
/*
 *
 * File: cmdlist.h
 *
 * Purpose: This is file contains the list of gdb commands
 *          which are registered as tcl commands.
 *
 */

#ifdef TYPE1
#define REGISTER_TYPE1(type,newname,pre,post) { type,"none",newname,pre,post },
#else
#define REGISTER_TYPE1(type,newname,pre,post)
#endif

#ifdef TYPE2
#define REGISTER_TYPE2(type,oldname,newname,pre,post) {type,oldname,newname,pre,post},
#else
#define REGISTER_TYPE2(type,oldname,newname,pre,post)
#endif

#ifdef TYPE3
#define REGISTER_TYPE3(type,newname,pre,post) {type,"none",newname,pre,post},
#else
#define REGISTER_TYPE3(type,newname,pre,post)
#endif


REGISTER_TYPE1(1,"h",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"q",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"help",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"quit",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"document",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"echo",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"cd",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"kdebug",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"dl",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"duel",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"whatis",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"pt",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"ptype",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"core-file",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"mt",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"maintenance",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"maint",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"remote <command>",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"sharedlibrary",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"target",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"section",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"exec-file",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"make",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"shell",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"stop",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"handle",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"path",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"tty",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"load",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"assf",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"add-shared-symbol-files",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"add-symbol-file",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"symbol-file",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"rbreak",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"inspect",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"p",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"print",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"call",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"output",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"printf",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"display",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"undisplay",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"disassemble",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"x",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"reverse-search",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"search",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"forward-search",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"dir",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"dire",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"direc",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"direct",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"directo",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"director",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"directory",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"t",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"thread",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"where",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"bt",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"ba",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"bac",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"backtrace",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"select-frame",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"frame",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"fr",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"f",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"delete",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"pwd",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"down-silently",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"dow",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"do",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"down",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"up-silently",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"up",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"awatch",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"rwatch",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"watch",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"clear", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"d", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"disa", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"dis", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"disable", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"enable", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"thbreak", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"hbreak", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"tbreak", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"condition", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"ignore", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"running", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"stack", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"data", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"breakpoints", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"files", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"status", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"support", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"user-defined", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"aliases", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"obscure", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"internals", (void*)0x0, (void*)0x0)

REGISTER_TYPE1(1,"file",(void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"catch",(void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"info", (void*)0x0, (void *)0x0)
REGISTER_TYPE1(1,"set",(void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"unset",(void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"break", (void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"continue", (void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"cont", (void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"show", (void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"list", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"search", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"complete", (void *)0x0, (void *)0x0)

REGISTER_TYPE1(1,"restore", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"silent", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"apropos", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"return",(void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"append", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"dump", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"overlay", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"source", (void *)0x0, (void *)0x0)



REGISTER_TYPE1(1,"commands", (void*)0x0, (void*)0x0)
REGISTER_TYPE1(1,"define", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"end", (void*)0x0, (void*)0x0)

/* trace point commands */

REGISTER_TYPE1(1,"actions", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"collect", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"while-stepping", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"passcount", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"save-tracepoints", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"trace", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"tdump", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"tfind", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"tload", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"tstart", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"tstatus", (void *)0x0, (void *)0x0)
REGISTER_TYPE1(1,"tstop", (void *)0x0, (void *)0x0)

REGISTER_TYPE1(1,"remap", (void*)0x0,(void*)0x0)
REGISTER_TYPE1(1,"reset-interrupt", (void*)0x0,(void*)0x0)
/*
  * type 2: no special processing, just pass the name
  * through.
  * [ams] Commands which are duplicated in Tcl
  */
REGISTER_TYPE2(2,"break", "brea",(void *)0x0, (void *)0x0)
REGISTER_TYPE2(2,"break", "bre",(void *)0x0, (void *)0x0)
REGISTER_TYPE2(2,"break", "br",(void *)0x0, (void *)0x0)
REGISTER_TYPE2(2,"break", "b",(void *)0x0, (void *)0x0)
REGISTER_TYPE2(2,"list", "l",(void *)0x0, (void *)0x0)
REGISTER_TYPE2(2,"i", "i", (void*)0x0, (void *)0x0)

/* gdb control flow commands */

REGISTER_TYPE2(2,"continue","c",(void*)0x0,(void*)0x0)
REGISTER_TYPE2(2,"unset environment", "unsetenv",(void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"set environment", "env",(void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"set variable","var",(void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"return","ret",(void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"delete", "d", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"p/x", "p/x", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"p/d", "p/d", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"p/u", "p/u", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"p/o", "p/o", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"p/t", "p/t", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"p/a", "p/a", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"p/c", "p/c", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"p/f", "p/f", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"print/x", "print/x", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"print/d", "print/d", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"print/u", "print/u", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"print/o", "print/o", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"print/t", "print/t", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"print/a", "print/a", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"print/c", "print/c", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"print/f", "print/f", (void*)0x0, (void*)0x0)
REGISTER_TYPE2(2,"quit", "exit",  (void *)0x0, (void *)0x0)

/*
 * Type 3: UpdateProceed commands.
 *
 */
REGISTER_TYPE3(3,"kill",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"r",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"run",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"fg",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"c",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"jump",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"u",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"until",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"s",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"step",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"n",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"next",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"finish",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"ni",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"nexti",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"si",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"stepi",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"signal",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"detach",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"attach",(void *)0x0, UpdateProceed)
REGISTER_TYPE3(3,"advance", (void*)0x0,UpdateProceed)
REGISTER_TYPE3(3,"disconnect", (void*)0x0,UpdateProceed)
REGISTER_TYPE3(3,"interrupt", (void*)0x0,UpdateProceed)

