
#
# Makefile to build binutils for the QDSP6.
#

dirs = obj-win obj-lnx32 obj-lnx


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

MINGW_GCC=/pkg/qct/software/hexagon/windows-cross/gcc-3.4.5-cross/bin

all:
	@echo "Specify a target:"
	@echo "	\"obj-win\" for windows"
	@echo "	\"obj-lnx\" for linux" 
	@echo
	@echo "Be sure to set PREFIX to the desired install location"
	@echo
	@echo "example: make PREFIX=\$$top/install/gnu obj-lnx"

install: $(PREFIX)

obj-win:
	mkdir $@
	cd $@ && \
	PATH=$(MINGW_GCC):$(PATH) \
	CC=i386-pc-mingw32-gcc \
	CC_FOR_TARGET=i386-pc-mingw32-gcc \
	CC_FOR_BUILD=gcc \
	../configure \
  		--target=qdsp6 \
  		--host=i386-pc-mingw32 \
  		--build=i686-linux \
  		--prefix=$(PREFIX) \
  		--disable-tcl \
  		--enable-bfd-assembler \
  		--disable-multilib 		&& \
	PATH=$(MINGW_GCC):$(PATH) make -j 8 all | tee build.log && \
	PATH=$(MINGW_GCC):$(PATH) make install  | tee install.log

obj-lnx: 
	mkdir $@
	cd $@ && \
	CFLAGS="$(BUILD_CFLAGS)" \
	../configure \
  		--target=qdsp6 \
  		--prefix=$(PREFIX) \
  		--disable-tcl \
  		--disable-multilib    && \
	make -j 8 all | tee build.log && \
	make install  | tee install.log

obj-lnx32: 
	mkdir $@
	cd $@ && \
	CC=gcc \
	CC_FOR_TARGET=gcc \
	CC_FOR_BUILD=gcc \
	CFLAGS="-m32 $(BUILD_CFLAGS)" \
	../configure \
  		--target=qdsp6 \
  		--prefix=$(PREFIX) \
  		--disable-tcl \
  		--disable-multilib    && \
	make -j 8 all | tee build.log && \
	make install  | tee install.log

.PHONY: clean
clean:
	rm -rf $(dirs) $(PREFIX)
