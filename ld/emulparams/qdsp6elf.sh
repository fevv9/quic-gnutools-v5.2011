
SCRIPT_NAME=qdsp6
ARCH=qdsp6
MACHINE=
TEMPLATE_NAME=elf32
EXTRA_EM_FILE=qdsp6elf
OUTPUT_FORMAT="elf32-littleqdsp6"
LITTLE_OUTPUT_FORMAT="elf32-littleqdsp6"
BIG_OUTPUT_FORMAT="elf32-bigqdsp6"
# EMBEDDED should be yanked for a real OS, e.g., Linux.
EMBEDDED=" "
MAXPAGESIZE=4096
# TEXT_START_ADDR should be 0 for embedded.
TEXT_START_ADDR=${EMBEDDED+0}${EMBEDDED-${MAXPAGESIZE}}
NONPAGED_TEXT_START_ADDR=0
ENTRY=start
TEXT_DYNAMIC=" "
DATA_GOT=" "
GENERATE_SHLIB_SCRIPT=yes
GENERATE_COMBRELOC_SCRIPT=yes
# QDSP6 V2 NOP
NOP=0x00c0007f
# Largest cache-line size
ALIGNMENT=64 # L2
OTHER_EXCLUDE_FILES=fini.o
SBSS_START_SYMBOLS="PROVIDE (${USER_LABEL_PREFIX}__sbss_start = .);
                    PROVIDE (${USER_LABEL_PREFIX}___sbss_start = .);"
SBSS_END_SYMBOLS="PROVIDE (${USER_LABEL_PREFIX}__sbss_end = .);
                  PROVIDE (${USER_LABEL_PREFIX}___sbss_end = .);"
SDATA_START_SYMBOLS="__default_sda_base__ = .;
                     PROVIDE (_SDA_BASE_ = .);"
