##################################################################
# Copyright (c) $Date$ QUALCOMM INCORPORATED.
# All Rights Reserved.
# Modified by QUALCOMM INCORPORATED on $Date$
##################################################################

SCRIPT_NAME=qdsp6
OUTPUT_FORMAT="elf32-littleqdsp6"
LITTLE_OUTPUT_FORMAT="elf32-littleqdsp6"
BIG_OUTPUT_FORMAT="elf32-bigqdsp6"
TEXT_START_ADDR=0
MAXPAGESIZE=4096
NONPAGED_TEXT_START_ADDR=0
ARCH=qdsp6
MACHINE=
ENTRY=start
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=qdsp6elf
TEXT_DYNAMIC=
GENERATE_SHLIB_SCRIPT=yes
GENERATE_COMBRELOC_SCRIPT=yes
EMBEDDED=yes
# QDSP6 V2 NOP
NOP=0x00c0007f
# Largest cache-line size
ALIGNMENT=64 # L2
OTHER_EXCLUDE_FILES=fini.o
