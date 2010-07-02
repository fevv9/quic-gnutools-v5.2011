SRCS = tgifInit.c hooks.c setproc.c register.c GuiSupport.c
OBJS = tgifInit.o hooks.o setproc.o register.o GuiSupport.o
ARCH = tgif.a

gdb_src = ../..
INCLUDE_GDB = -I$(gdb_src)/include -I$(gdb_src)/config -I$(gdb_src)/gdb -I$(gdb_src)/bfd -I$(gdb_src)/gdb/config -I$(gdb_src)/mmalloc
TCL_INCLUDE = -I$(gdb_src)/tcl/generic
LOCAL_CFLAGS = -g $(CFLAGS) $(INCLUDE_GDB) $(TCL_INCLUDE) -DHAVETK -DHAVETCL -DNOINIT -DPTHREAD_DEBUG -DNEW_PROCEED_HANDLING
CC = gcc

all:	$(ARCH)

$(ARCH):	$(OBJS) tgifInit.o
	rm -f $(ARCH)
	ar -rc $(ARCH) $(OBJS)
	rm -f ../gdb
	rm -f ../tgifInit.o
#	cp tgifInit.c ../tgifInit.c

depend:	$(SRCS)
	makedepend -- $(LOCAL_CFLAGS) -- $(SRCS)

distclean realclean clean:
	rm -f core $(OBJS) tgif.a

.c.o:
	$(CC) -c $(LOCAL_CFLAGS) $<

# DO NOT DELETE THIS LINE -- make depend depends on it.

tgifBasic.o: ../../tcl/generic/tcl.h
tgifBasic.o: tgif.h
tgifInit.o:  ../../tcl/generic/tcl.h
tgifInit.o:  ../../tcl/generic/tclInt.h
tgifInit.o: ../../tcl/generic/tclRegexp.h
tgifInit.o: tgif.h
tgif.o: ../../tcl/generic/tcl.h
tgifCmdPass.o: ../../tcl/generic/tcl.h
tgifCmdPass.o: tgif.h
tgifCmdPass.o: tgifCmds.h
tgifCmdCare.o: ../../tcl/generic/tcl.h
tgifCmdCare.o: tgif.h
tgifCmdCare.o: tgifCmds.h
tgifAddCmds.o: ../../gdb/defs.h
tgifAddCmds.o: ../../include/ansidecl.h
tgifAddCmds.o: ../../include/libiberty.h ../../include/progress.h
tgifAddCmds.o: ../../mmalloc/mmalloc.h ../../include/fopen-same.h
tgifAddCmds.o: ../../include/dis-asm.h ../../gdb/symtab.h
tgifAddCmds.o: ../../include/obstack.h ../../gdb/bcache.h
tgifAddCmds.o: ../../gdb/gdbtypes.h ../../gdb/value.h ../../gdb/expression.h
tgifAddCmds.o: ../../gdb/gdbcore.h ../../gdb/frame.h ../../gdb/command.h
tgifAddCmds.o: ../../gdb/gdbcmd.h ../../gdb/target.h
tgifAddCmds.o: ../../tcl/generic/tcl.h tgif.h
tgifAddCmds.o:  tgifCmds.h
