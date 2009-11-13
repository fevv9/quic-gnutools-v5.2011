
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

all: obj-win obj-lnx obj-lnx32

install: $(PREFIX)

obj-win:
	mkdir $@
	cd $@ && \
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
	make -j 20 all install | tee build.log

obj-lnx: 
	mkdir $@
	cd $@ && \
	CFLAGS="$(BUILD_CFLAGS)" \
	../configure \
  		--target=qdsp6 \
  		--prefix=$(PREFIX) \
  		--disable-tcl && \
	make -j 20 all install | tee build.log

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
  		--enable-bfd-assembler \
  		--disable-multilib 		&& \
	make -j 20 all install | tee build.log

.PHONY: clean
clean:
	rm -rf $(dirs) $(PREFIX)
