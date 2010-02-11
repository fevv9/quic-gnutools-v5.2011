##################################################################
# Copyright (c) $Date$ Qualcomm Innovation Center, Inc.
# All Rights Reserved.
# Modified by Qualcomm Innovation Center, Inc. on $Date$
##################################################################
#
# Makefile to build binutils for the QDSP6.
#

dirs = build_win build_lnx 


BS=$(shell uname)

ifeq "$(BS)" "Linux"
DESTDIR=/usr/local/qdsp6/gnu
else
DESTDIR=/usr/local/qdsp6-mingw/gnu
endif

ifdef DEBUG
BUILD_CFLAGS=-g -O0
else
BUILD_CFLAGS=-O4
endif

ifndef PREFIX
PREFIX = $(DESTDIR)
endif

ifndef JOBS
JOBS = 1
endif

MINGW_GCC=/pkg/qct/software/hexagon/windows-cross/gcc-3.4.5-cross/bin

# makeinfo must be version 4.7 or greater

ifneq ("$(shell $(MAKEINFO) --version | egrep 'texinfo[^0-9]*(4\.([0-6]))')","")
  MAKEINFO := /pkg/qct/software/gnu/texinfo/4.13a/bin/makeinfo
endif

all: help

.PHONY: help
help:
	@echo "Specify a target:"
	@echo "	\"build_win\" for windows"
	@echo "	\"build_lnx\" for linux" 
	@echo
	@echo "Be sure to set PREFIX to the desired install location"
	@echo
	@echo "example: make PREFIX=\$$top/install/gnu build_lnx"


install: $(PREFIX)

CONFIGURE_OPTIONS = --target=qdsp6 \
		    --disable-nls \
		    --enable-bfd-assembler \
		    --disable-multilib \
		    --prefix=$(PREFIX)

build_win:
	mkdir $@
	cd $@ && \
	PATH=$(MINGW_GCC):$(PATH) \
	CC=i386-pc-mingw32-gcc \
	CC_FOR_TARGET=i386-pc-mingw32-gcc \
	CC_FOR_BUILD=gcc \
	MAKEINFO=$(MAKEINFO) \
	../configure \
  		--host=i386-pc-mingw32 \
  		--build=i686-linux \
		$(CONFIGURE_OPTIONS)  && \
	PATH=$(MINGW_GCC):$(PATH) $(MAKE) -j $(JOBS) all

build_lnx: 
	mkdir $@
	cd $@ && \
	CFLAGS="$(BUILD_CFLAGS)" \
	MAKEINFO=$(MAKEINFO) \
	../configure \
		$(CONFIGURE_OPTIONS)  && \
	$(MAKE) -j $(JOBS) all


.PHONY: install_lnx
install_lnx: build_lnx
	$(MAKE) -C build_lnx install

.PHONY: install_win
install_win: build_win
	PATH=$(MINGW_GCC):$(PATH) $(MAKE) -C build_win install

.PHONY: clean
clean:
	rm -rf $(dirs) 
