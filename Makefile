
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
		    --disable-tcl \
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
	../configure \
  		--host=i386-pc-mingw32 \
  		--build=i686-linux \
		$(CONFIGURE_OPTIONS)  && \
	PATH=$(MINGW_GCC):$(PATH) make -j $(JOBS) all 2>&1| tee build.log 

build_lnx: 
	mkdir $@
	cd $@ && \
	CFLAGS="$(BUILD_CFLAGS)" \
	../configure \
		$(CONFIGURE_OPTIONS)  && \
	make -j $(JOBS) all 2>&1| tee build.log


.PHONY: install_lnx
install_lnx: build_lnx
	$(MAKE) -C build_lnx install

.PHONY: install_win
install_win: build_win
	PATH=$(MINGW_GCC):$(PATH) $(MAKE) -C build_win install

.PHONY: clean
clean:
	rm -rf $(dirs) 