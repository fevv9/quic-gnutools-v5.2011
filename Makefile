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

PYTHON_INSTALL=/pkg/qct/software/python/2.7

all: help

.PHONY: help build_lnx build_win
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
        $(EXP_FLAGS) \
		    --prefix=$(PREFIX) 

build_win:
	mkdir -p $@
	cd $@ && \
	HAVETCL=1 \
	PATH=$(MINGW_GCC):$(PATH) \
	CC=i386-pc-mingw32-gcc \
	CC_FOR_TARGET=i386-pc-mingw32-gcc \
	CC_FOR_BUILD=gcc \
	MAKEINFO=$(MAKEINFO) \
	ISS_DIR=$(ISS_DIR) \
	../configure \
		--with-python=/prj/dsp/qdsp6/austin/builds/hexbuild/python/Python26 \
		--x-includes=../win \
  		--host=i386-pc-mingw32 \
  		--build=i686-linux \
		$(CONFIGURE_OPTIONS)  && \
	PATH=$(MINGW_GCC):$(PATH) $(MAKE) -j $(JOBS) all

build_lnx: 
	mkdir -p $@
	cd $@ && \
	CFLAGS="$(BUILD_CFLAGS)" \
	HAVETCL=1 \
	MAKEINFO=$(MAKEINFO) \
	ISS_DIR=$(ISS_DIR) \
	../configure \
		--with-python=$(PYTHON_INSTALL) \
		$(CONFIGURE_OPTIONS)  && \
	$(MAKE) -j $(JOBS) all

build_native: 
	mkdir -p $@
	cd $@ &&  \
	CC=qdsp6-gcc \
	AS=qdsp6-as \
	AR=qdsp6-ar \
	LD=qdsp6-ld \
	CFLAGS="$(BUILD_CFLAGS) -fno-short-enums" \
	../configure \
		--disable-nls \
		--disable-werror \
		--build=i686-linux \
		--host=qdsp6-linux-uclibc \
		--target=qdsp6-linux-uclibc \
		--without-tcl --without-gas \
		--without-ld \
		--without-gprof \
		--with-curses && \
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
