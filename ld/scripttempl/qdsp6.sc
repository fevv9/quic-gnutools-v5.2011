##################################################################
#
# Unusual variables checked by this code:
#	NOP - four byte opcode for no-op (defaults to 0)
#	NO_SMALL_DATA - no .sbss/.sbss2/.sdata/.sdata2 sections if not
#		empty.
#	SMALL_DATA_CTOR - .ctors contains small data.
#	SMALL_DATA_DTOR - .dtors contains small data.
#	DATA_ADDR - if end-of-text-plus-one-page isn't right for data start
#	INITIAL_READONLY_SECTIONS - at start of text segment
#	OTHER_READONLY_SECTIONS - other than .text .init .rodata ...
#		(e.g., .PARISC.milli)
#	OTHER_TEXT_SECTIONS - these get put in .text when relocating
#	OTHER_READWRITE_SECTIONS - other than .data .bss .ctors .sdata ...
#		(e.g., .PARISC.global)
#	OTHER_RELRO_SECTIONS - other than .data.rel.ro ...
#		(e.g. PPC32 .fixup, .got[12])
#	OTHER_BSS_SECTIONS - other than .bss .sbss ...
#	ATTRS_SECTIONS - at the end
#	OTHER_SECTIONS - at the end
#	EXECUTABLE_SYMBOLS - symbols that must be defined for an
#		executable (e.g., _DYNAMIC_LINK)
#       TEXT_START_ADDR - the first byte of the text segment, after any
#               headers.
#       TEXT_BASE_ADDRESS - the first byte of the text segment.
#	TEXT_START_SYMBOLS - symbols that appear at the start of the
#		.text section.
#	DATA_START_SYMBOLS - symbols that appear at the start of the
#		.data section.
#	DATA_END_SYMBOLS - symbols that appear at the end of the
#		writeable data sections.
#	OTHER_GOT_SYMBOLS - symbols defined just before .got.
#	OTHER_GOT_SECTIONS - sections just after .got.
#	OTHER_SDATA_SECTIONS - sections just after .sdata.
#	OTHER_BSS_SYMBOLS - symbols that appear at the start of the
#		.bss section besides __bss_start.
#	DATA_PLT - .plt should be in data segment, not text segment.
#	PLT_BEFORE_GOT - .plt just before .got when .plt is in data segement.
#	BSS_PLT - .plt should be in bss segment
#	NO_REL_RELOCS - Don't include .rel.* sections in script
#	NO_RELA_RELOCS - Don't include .rela.* sections in script
#	NON_ALLOC_DYN - Place dynamic sections after data segment.
#	TEXT_DYNAMIC - .dynamic in text segment, not data segment.
#	EMBEDDED - whether this is for an embedded system.
#	SHLIB_TEXT_START_ADDR - if set, add to SIZEOF_HEADERS to set
#		start address of shared library.
#	INPUT_FILES - INPUT command of files to always include
#	WRITABLE_RODATA - if set, the .rodata section should be writable
#	INIT_START, INIT_END -  statements just before and just after
# 	combination of .init sections.
#	FINI_START, FINI_END - statements just before and just after
# 	combination of .fini sections.
#	STACK_ADDR - start of a .stack section.
#	OTHER_SYMBOLS - symbols to place right at the end of the script.
#	ETEXT_NAME - name of a symbol for the end of the text section,
#		normally etext.
#	SEPARATE_GOTPLT - if set, .got.plt should be separate output section,
#		so that .got can be in the RELRO area.  It should be set to
#		the number of bytes in the beginning of .got.plt which can be
#		in the RELRO area as well.
#	USER_LABEL_PREFIX - prefix to add to user-visible symbols.
#
# When adding sections, do note that the names of some sections are used
# when specifying the start address of the next.
#

#  Many sections come in three flavours.  There is the 'real' section,
#  like ".data".  Then there are the per-procedure or per-variable
#  sections, generated by -ffunction-sections and -fdata-sections in GCC,
#  and useful for --gc-sections, which for a variable "foo" might be
#  ".data.foo".  Then there are the linkonce sections, for which the linker
#  eliminates duplicates, which are named like ".gnu.linkonce.d.foo".
#  The exact correspondences are:
#
#  Section	Linkonce section
#  .text	.gnu.linkonce.t.foo
#  .rodata	.gnu.linkonce.r.foo
#  .data	.gnu.linkonce.d.foo
#  .bss		.gnu.linkonce.b.foo
#  .sdata	.gnu.linkonce.s.foo
#  .sbss	.gnu.linkonce.sb.foo
#  .sdata2	.gnu.linkonce.s2.foo
#  .sbss2	.gnu.linkonce.sb2.foo
#  .debug_info	.gnu.linkonce.wi.foo
#  .tdata	.gnu.linkonce.td.foo
#  .tbss	.gnu.linkonce.tb.foo
#  .lrodata	.gnu.linkonce.lr.foo
#  .ldata	.gnu.linkonce.l.foo
#  .lbss	.gnu.linkonce.lb.foo
#
#  Each of these can also have corresponding .rel.* and .rela.* sections.

test -z "$ENTRY" && ENTRY=${USER_LABEL_PREFIX}_start
test -z "${BIG_OUTPUT_FORMAT}" && BIG_OUTPUT_FORMAT=${OUTPUT_FORMAT}
test -z "${LITTLE_OUTPUT_FORMAT}" && LITTLE_OUTPUT_FORMAT=${OUTPUT_FORMAT}
if [ -z "$MACHINE" ]; then OUTPUT_ARCH=${ARCH}; else OUTPUT_ARCH=${ARCH}:${MACHINE}; fi
test -z "${ELFSIZE}" && ELFSIZE=32
test -z "${ALIGNMENT}" && ALIGNMENT="${ELFSIZE}" / 8
test "$LD_FLAG" = "N" && DATA_ADDR=.
test -z "${ETEXT_NAME}" && ETEXT_NAME=${USER_LABEL_PREFIX}etext
test -n "$CREATE_SHLIB$CREATE_PIE" && test -n "$SHLIB_DATA_ADDR" && COMMONPAGESIZE=""
test -z "$CREATE_SHLIB$CREATE_PIE" && test -n "$DATA_ADDR" && COMMONPAGESIZE=""
test -n "$RELRO_NOW" && unset SEPARATE_GOTPLT
test -z "$ATTRS_SECTIONS" && ATTRS_SECTIONS=".gnu.attributes 0 : { KEEP (*(.gnu.attributes)) }"
DATA_SEGMENT_ALIGN="ALIGN(${SEGMENT_SIZE}) + (. & (${MAXPAGESIZE} - 1))"
DATA_SEGMENT_RELRO_END=""
DATA_SEGMENT_END=""
if test -n "${COMMONPAGESIZE}"; then
  DATA_SEGMENT_ALIGN="ALIGN (${SEGMENT_SIZE}) - ((${MAXPAGESIZE} - .) & (${MAXPAGESIZE} - 1)); . = DATA_SEGMENT_ALIGN (${MAXPAGESIZE}, ${COMMONPAGESIZE})"
  DATA_SEGMENT_END=". = DATA_SEGMENT_END (.);"
  DATA_SEGMENT_RELRO_END=". = DATA_SEGMENT_RELRO_END (${SEPARATE_GOTPLT-0}, .);"
fi
if test -z "${INITIAL_READONLY_SECTIONS}${CREATE_SHLIB}"; then
  INITIAL_READONLY_SECTIONS=".interp       ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.interp) - __ebi_va_start__)}
                             { *(.interp) }"
fi
if test -z "$PLT"; then
  IPLT=".iplt         ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.iplt) - __ebi_va_start__)} { *(.iplt) }"
  PLT=".plt          ${RELOCATING-0} :  ${TCM+AT (__ebi_pa_start__ + ADDR (.plt) - __ebi_va_start__)} { *(.plt)${IREL_IN_PLT+ *(.iplt)} }
  ${IREL_IN_PLT-$IPLT}"
fi
test -n "${DATA_PLT-${BSS_PLT-text}}" && TEXT_PLT=yes
if test -z "$GOT"; then
  if test -z "$SEPARATE_GOTPLT"; then
    GOT=".got          ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.got) - __ebi_va_start__)} { *(.got.plt) *(.igot.plt) *(.got) *(.igot) }"
  else
    GOT=".got          ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.got) - __ebi_va_start__)} { *(.got) *(.igot) }"
    GOTPLT=".got.plt      ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.got.plt) - __ebi_va_start__)} { *(.got.plt)  *(.igot.plt) }"
  fi
fi
RELA_IFUNC=".rela.ifunc   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.ifunc) - __ebi_va_start__)} { *(.rela.ifunc) }"
RELA_IPLT=".rela.iplt    ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.iplt) - __ebi_va_start__)}
    {
      ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__rela_iplt_start = .);}}
      *(.rela.iplt)
      ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__rela_iplt_end = .);}}
    }"
DYNAMIC=".dynamic      ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.dynamic) - __ebi_va_start__)} { *(.dynamic) }"
RODATA=".rodata       ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rodata) - __ebi_va_start__)}
        {
          *(.rodata.hot .rodata.hot.* .gnu.linkonce.r.hot.*)
          *(.rodata${RELOCATING+ .rodata.* .gnu.linkonce.r.*})
        }"
DATARELRO=".data.rel.ro : { *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro* .gnu.linkonce.d.rel.ro.*) }"
DISCARDED="/DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }"
if test -z "${NO_SMALL_DATA}"; then
  SBSS="
  ${RELOCATING+. = ALIGN (${ALIGNMENT});}
  .sbss         ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.sbss) - __ebi_va_start__)}
  {
    ${RELOCATING+${SBSS_START_SYMBOLS}}
    ${CREATE_SHLIB+*(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)}
    *(.dynsbss)
    *(.sbss.hot${RELOCATING+ .sbss.hot.* .gnu.linkonce.sb.hot.*})
    *(.sbss${RELOCATING+ .sbss.* .gnu.linkonce.sb.*})
    *(.scommon${RELOCATING+ .scommon.*})
    ${RELOCATING+. = ALIGN (. != 0 ? ${ALIGNMENT} : 1);}
    ${RELOCATING+${SBSS_END_SYMBOLS}}
  }"
  SBSS2="
  .sbss2        ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.sbss2) - __ebi_va_start__)}
  { *(.sbss2${RELOCATING+ .sbss2.* .gnu.linkonce.sb2.*}) }"
  SDATA="
  ${RELOCATING+. = ALIGN (${ALIGNMENT});}
  .sdata        ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.sdata) - __ebi_va_start__)}
  {
    ${RELOCATING+${SDATA_START_SYMBOLS}}
    ${CREATE_SHLIB+*(.sdata2 .sdata2.* .gnu.linkonce.s2.*)}
    *(.sdata.1${RELOCATING+ .sdata.1.* .gnu.linkonce.s.1.*})
    *(.sbss.1${RELOCATING+ .sbss.1.* .gnu.linkonce.sb.1.*})
    *(.scommon.1${RELOCATING+ .scommon.1.*})
    *(.sdata.2${RELOCATING+ .sdata.2.* .gnu.linkonce.s.2.*})
    *(.sbss.2${RELOCATING+ .sbss.2.* .gnu.linkonce.sb.2.*})
    *(.scommon.2${RELOCATING+ .scommon.2.*})
    *(.sdata.4${RELOCATING+ .sdata.4.* .gnu.linkonce.s.4.*})
    *(.sbss.4${RELOCATING+ .sbss.4.* .gnu.linkonce.sb.4.*})
    *(.scommon.4${RELOCATING+ .scommon.4.*})
    *(.lit[a4]${RELOCATING+ .lit[a4].* .gnu.linkonce.l[a4].*})
    *(.lit8${RELOCATING+ .lit8.* .gnu.linkonce.l8.*})
    *(.sdata.8${RELOCATING+ .sdata.8.* .gnu.linkonce.s.8.*})
    *(.sbss.8${RELOCATING+ .sbss.8.* .gnu.linkonce.sb.8.*})
    *(.scommon.8${RELOCATING+ .scommon.8.*})
    *(.sdata.hot${RELOCATING+ .sdata.hot.* .gnu.linkonce.s.hot.*})
    *(.sdata${RELOCATING+ .sdata.* .gnu.linkonce.s.*})
  }"
  OTHER_SDATA_SECTIONS=""
  SDATA2="
  .sdata2       ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.sdata2) - __ebi_va_start__)}
    {
      ${RELOCATING+${SDATA2_START_SYMBOLS}}
      { *(.sdata2${RELOCATING+ .sdata2.* .gnu.linkonce.s2.*})}
    }"
  REL_SDATA="
  .rela.sdata   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.sdata) - __ebi_va_start__)} \
{ *(.rela.sdata .rela.lit[a48]${RELOCATING+ .rela.sdata.* .rela.lit[a48].* .rela.gnu.linkonce.s.* .rela.gnu.linkonce.l[a48].*}) }"
  REL_SBSS="
  .rela.sbss    ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.sbss) - __ebi_va_start__)} \
{ *(.rela.sbss${RELOCATING+ .rela.sbss.* .rela.gnu.linkonce.sb.*}) }"
  REL_SDATA2="
  .rela.sdata2  ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.sdata2) - __ebi_va_start__)} \
{ *(.rela.sdata2${RELOCATING+ .rela.sdata2.* .rela.gnu.linkonce.s2.*}) }"
  REL_SBSS2="
  .rela.sbss2   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.sbss2) - __ebi_va_start__)} \
{ *(.rela.sbss2${RELOCATING+ .rela.sbss2.* .rela.gnu.linkonce.sb2.*}) }"
else
  NO_SMALL_DATA=" "
fi
if test -z "${DATA_GOT}"; then
  if test -n "${NO_SMALL_DATA}"; then
    DATA_GOT=" "
  fi
fi
if test -z "${SDATA_GOT}" && test -z "${DATA_GOT}"; then
  if test -z "${NO_SMALL_DATA}"; then
    SDATA_GOT=" "
  fi
fi
test -n "$SEPARATE_GOTPLT" && SEPARATE_GOTPLT=" "
test "${LARGE_SECTIONS}" = "yes" && REL_LARGE="
  .rela.ldata   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.ldata) - __ebi_va_start__)} { *(.rela.ldata${RELOCATING+ .rela.ldata.* .rela.gnu.linkonce.l.*}) }
  .rela.lbss    ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.lbss) - __ebi_va_start__)} { *(.rela.lbss${RELOCATING+ .rela.lbss.* .rela.gnu.linkonce.lb.*}) }
  .rela.lrodata ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.lrodata) - __ebi_va_start__)} { *(.rela.lrodata${RELOCATING+ .rela.lrodata.* .rela.gnu.linkonce.lr.*}) }"
test "${LARGE_SECTIONS}" = "yes" && OTHER_BSS_SECTIONS="
  ${OTHER_BSS_SECTIONS}
  .lbss ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.lbss) - __ebi_va_start__)}
  {
    *(.dynlbss)
    *(.lbss${RELOCATING+ .lbss.* .gnu.linkonce.lb.*})
    *(LARGE_COMMON)
  }"
test "${LARGE_SECTIONS}" = "yes" && LARGE_SECTIONS="
  .lrodata ${RELOCATING-0} ${RELOCATING+ALIGN(${MAXPAGESIZE}) + (. & (${MAXPAGESIZE} - 1))} : ${TCM+AT (__ebi_pa_start__ + ADDR (.lrodata) - __ebi_va_start__)}
  {
    *(.lrodata${RELOCATING+ .lrodata.* .gnu.linkonce.lr.*})
  }
  .ldata ${RELOCATING-0} ${RELOCATING+ALIGN(${MAXPAGESIZE}) + (. & (${MAXPAGESIZE} - 1))} : ${TCM+AT (__ebi_pa_start__ + ADDR (.ldata) - __ebi_va_start__)}
  {
    *(.ldata${RELOCATING+ .ldata.* .gnu.linkonce.l.*})
    ${RELOCATING+. = ALIGN(. != 0 ? ${ALIGNMENT} : 1);}
  }"
CTOR=".ctors        ${CONSTRUCTING-0} :  ${TCM+AT (__ebi_pa_start__ + ADDR (.ctors) - __ebi_va_start__)}
  {
    ${CONSTRUCTING+${CTOR_START}}
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */

    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))

    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */

    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o $OTHER_EXCLUDE_FILES) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    ${CONSTRUCTING+${CTOR_END}}
  }"
DTOR=".dtors        ${CONSTRUCTING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.dtors) - __ebi_va_start__)}
  {
    ${CONSTRUCTING+${DTOR_START}}
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o $OTHER_EXCLUDE_FILES) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    ${CONSTRUCTING+${DTOR_END}}
  }"
STACK="  .stack        ${RELOCATING-0}${RELOCATING+${STACK_ADDR}} : ${TCM+AT (__ebi_pa_start__ + ADDR (.stack) - __ebi_va_start__)}
  {
    ${RELOCATING+${USER_LABEL_PREFIX}_stack = .;}
    *(.stack)
  }"

TEXT_START_ADDR="SEGMENT_START(\"text-segment\", ${TEXT_START_ADDR})"
SHLIB_TEXT_START_ADDR="SEGMENT_START(\"text-segment\", ${SHLIB_TEXT_START_ADDR:-0})"

# if this is for an embedded system, don't add SIZEOF_HEADERS.
if [ -z "$EMBEDDED" ]; then
   test -z "${TEXT_BASE_ADDRESS}" && TEXT_BASE_ADDRESS="${TEXT_START_ADDR} + SIZEOF_HEADERS"
else
   test -z "${TEXT_BASE_ADDRESS}" && TEXT_BASE_ADDRESS="${TEXT_START_ADDR}"
fi

TCM_ADDRESS=0xd8000000 # Physical address of the TCM.

# EBI is the default memory bus, so just define those sections that don't fit the default code and data sections.
EBI_CODE="
    . = ALIGN (DEFINED (EBI_CODE_CACHED_ALIGN)? EBI_CODE_CACHED_ALIGN : ${ALIGNMENT});

    *(.ebi_code_cached${RELOCATING+ .ebi_code_cached.*})
    *(.ebi_code_cached_wb${RELOCATING+ .ebi_code_cached_wb.*})
    *(.ebi_code_cached_wt${RELOCATING+ .ebi_code_cached_wt.*})
    *(.ebi_code_uncached${RELOCATING+ .ebi_code_uncached.*})"

EBI_DATA="
  . = ALIGN (DEFINED (EBI_DATA_CACHED_ALIGN)? EBI_DATA_CACHED_ALIGN : ${MAXPAGESIZE});

  .ebi_data_cached : AT (__ebi_pa_start__ + ADDR (.ebi_data_cached) - __ebi_va_start__)
  {
    *(.ebi_data_cached${RELOCATING+ .ebi_data_cached.*})
    *(.ebi_data_cached_wb${RELOCATING+ .ebi_data_cached_wb.*})
  } :DATA"

EBI_DATA_WT="
  . = ALIGN (DEFINED (EBI_DATA_CACHED_WT_ALIGN)? EBI_DATA_CACHED_WT_ALIGN : ${MAXPAGESIZE});

  .ebi_data_cached_wt : AT (__ebi_pa_start__ + ADDR (.ebi_data_cached_wt) - __ebi_va_start__)
  { *(.ebi_data_cached_wt${RELOCATING+ .ebi_data_cached_wt.*}) } :EBI_DATA_WT"

EBI_DATA_UN="
  . = ALIGN (DEFINED (EBI_DATA_UNCACHED_ALIGN)? EBI_DATA_UNCACHED_ALIGN : ${MAXPAGESIZE});

  .ebi_data_uncached : AT (__ebi_pa_start__ + ADDR (.ebi_data_uncached) - __ebi_va_start__)
  { *(.ebi_data_uncached${RELOCATING+ .ebi_data_uncached.*}) } :EBI_DATA_UN"

# SMI sections.
SMI_CODE="
  . = ALIGN (DEFINED (SMI_CODE_CACHED_ALIGN)? SMI_CODE_CACHED_ALIGN : ${MAXPAGESIZE});
  . = ALIGN (DEFINED (SMI_CODE_UNCACHED_ALIGN)? SMI_CODE_UNCACHED_ALIGN : ${MAXPAGESIZE});

  .smi_code_cached : AT (__smi_pa_start__ + ADDR (.smi_code_cached) - __smi_va_start__)
  {
    *(.smi_code_cached${RELOCATING+ .smi_code_cached.*})
    *(.smi_code_cached_wb${RELOCATING+ .smi_code_cached_wb.*})
    *(.smi_code_cached_wt${RELOCATING+ .smi_code_cached_wt.*})
    *(.smi_code_uncached${RELOCATING+ .smi_code_uncached.*})
  } :SMI_CODE =${NOP-0}"

SMI_DATA="
  . = ALIGN (DEFINED (SMI_DATA_CACHED_ALIGN)? SMI_DATA_CACHED_ALIGN : ${MAXPAGESIZE});
  . = ALIGN (DEFINED (SMI_DATA_CACHED_WB_ALIGN)? SMI_DATA_CACHED_WB_ALIGN : ${MAXPAGESIZE});

  .smi_data_cached : AT (__smi_pa_start__ + ADDR (.smi_data_cached) - __smi_va_start__)
  {
    *(.smi_data_cached${RELOCATING+ .smi_data_cached.*})
    *(.smi_data_cached_wb${RELOCATING+ .smi_data_cached_wb.*})
  } :SMI_DATA"

SMI_DATA_WT="
  . = ALIGN (DEFINED (SMI_DATA_CACHED_WT_ALIGN)? SMI_DATA_CACHED_WT_ALIGN : ${MAXPAGESIZE});

  .smi_data_cached_wt : AT (__smi_pa_start__ + ADDR (.smi_data_cached_wt) - __smi_va_start__)
  { *(.smi_data_cached_wt${RELOCATING+ .smi_data_cached_wt.*}) } :SMI_DATA_WT"

SMI_DATA_UN="
  . = ALIGN (DEFINED (SMI_DATA_UNCACHED_ALIGN)? SMI_DATA_UNCACHED_ALIGN : ${MAXPAGESIZE});

  .smi_data_uncached : AT (__smi_pa_start__ + ADDR (.smi_data_uncached) - __smi_va_start__)
  { *(.smi_data_uncached${RELOCATING+ .smi_data_uncached.*}) } :SMI_DATA_UN"

# TCM sections.
TCM_CODE="
  . = ALIGN (DEFINED (TCM_CODE_CACHED_ALIGN)? TCM_CODE_CACHED_ALIGN : ${MAXPAGESIZE});
  . = ALIGN (DEFINED (TCM_CODE_UNCACHED_ALIGN)? TCM_CODE_UNCACHED_ALIGN : ${MAXPAGESIZE});

  .tcm_code_cached : AT (__tcm_pa_start__ + ADDR (.tcm_code_cached) - __tcm_va_start__)
  {
    *(.tcm_code_cached${RELOCATING+ .tcm_code_cached.*})
    *(.tcm_code_cached_wb${RELOCATING+ .tcm_code_cached_wb.*})
    *(.tcm_code_cached_wt${RELOCATING+ .tcm_code_cached_wt.*})
    *(.tcm_code_uncached${RELOCATING+ .tcm_code_uncached.*})
  } :TCM_CODE =${NOP-0}"

TCM_DATA="
  . = ALIGN (DEFINED (TCM_DATA_CACHED_ALIGN)? TCM_DATA_CACHED_ALIGN : ${MAXPAGESIZE});
  . = ALIGN (DEFINED (TCM_DATA_CACHED_WB_ALIGN)? TCM_DATA_CACHED_WB_ALIGN : ${MAXPAGESIZE});

  .tcm_data_cached : AT (__tcm_pa_start__ + ADDR (.tcm_data_cached) - __tcm_va_start__)
  {
    *(.tcm_data_cached${RELOCATING+ .tcm_data_cached.*})
    *(.tcm_data_cached_wb${RELOCATING+ .tcm_data_cached_wb.*})
  } :TCM_DATA"

TCM_DATA_WT="
  . = ALIGN (DEFINED (TCM_DATA_CACHED_WT_ALIGN)? TCM_DATA_CACHED_WT_ALIGN : ${MAXPAGESIZE});

  .tcm_data_cached_wt : AT (__tcm_pa_start__ + ADDR (.tcm_data_cached_wt) - __tcm_va_start__)
  { *(.tcm_data_cached_wt${RELOCATING+ .tcm_data_cached_wt.*}) } :TCM_DATA_WT"

TCM_DATA_UN="
  . = ALIGN (DEFINED (TCM_DATA_UNCACHED_ALIGN)? TCM_DATA_UNCACHED_ALIGN : ${MAXPAGESIZE});

  .tcm_data_uncached : AT (__tcm_pa_start__ + ADDR (.tcm_data_uncached) - __tcm_va_start__)
  { *(.tcm_data_uncached${RELOCATING+ .tcm_data_uncached.*}) } :TCM_DATA_UN"

OTHER_TEXT_SECTIONS="${TCM+${EBI_CODE}}"
OTHER_READWRITE_SECTIONS="${TCM+${EBI_DATA}}"

cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}",
              "${BIG_OUTPUT_FORMAT}",
	      "${LITTLE_OUTPUT_FORMAT}")
OUTPUT_ARCH(${OUTPUT_ARCH})
${RELOCATING+ENTRY(${ENTRY})}
${RELOCATING+${LIB_SEARCH_DIRS}}

${TCM+PHDRS
"{"
  HEADERS     PT_PHDR FILEHDR PHDRS;
/* Dynamic segments */
  DYNAMIC     PT_DYNAMIC;
  ${CREATE_SHLIB+INTERP      PT_INTERP;}
/* EBI segments */
  BOOTUP      PT_LOAD;                    /* start-up code */
  CODE        PT_LOAD;                    /* code */
  CONST       PT_LOAD;                    /* read-only data */
  DATA        PT_LOAD;                    /* read-write data */
  SDATA       PT_LOAD;                    /* read-write small data */
/* More EBI segments */
  EBI_DATA_WT PT_LOAD FLAGS (0x40000000); /* cached write-thru data */
  EBI_DATA_UN PT_LOAD FLAGS (0x80000000); /* uncached data */
/* SMI segments */
  SMI_CODE    PT_LOAD;                    /* cached and uncached code */
  SMI_DATA    PT_LOAD;                    /* cached and cached write-back data */
  SMI_DATA_WT PT_LOAD FLAGS (0x40000000); /* cached write-thru data */
  SMI_DATA_UN PT_LOAD FLAGS (0x80000000); /* uncached data */
/* TCM segments */
  TCM_CODE    PT_LOAD FLAGS (0x20000000); /* cached and uncached code */
  TCM_DATA    PT_LOAD FLAGS (0x20000000); /* cached and cached write-back data */
  TCM_DATA_WT PT_LOAD FLAGS (0x60000000); /* cached write-thru data */
  TCM_DATA_UN PT_LOAD FLAGS (0xa0000000); /* uncached data */
"}"}

${RELOCATING+${EXECUTABLE_SYMBOLS}}
${RELOCATING+${INPUT_FILES}}
${RELOCATING- /* For some reason, the Solaris linker makes bad executables
  if gld -r is used and the intermediate file has sections starting
  at non-zero addresses.  Could be a Solaris ld bug, could be a GNU ld
  bug.  But for now assigning the zero vmas works.  */}

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  ${CREATE_SHLIB-${CREATE_PIE-${RELOCATING+PROVIDE (__executable_start = ${TEXT_START_ADDR}); . = ${TEXT_BASE_ADDRESS};}}}
  ${CREATE_SHLIB+${RELOCATING+. = ${SHLIB_TEXT_START_ADDR} + SIZEOF_HEADERS;}}
  ${CREATE_PIE+${RELOCATING+. = ${SHLIB_TEXT_START_ADDR} + SIZEOF_HEADERS;}}

/* Start EBI memory. */
  ${TCM+__ebi_va_start__ = ALIGN ((DEFINED (EBI_VA_START)? EBI_VA_START : .), ${MAXPAGESIZE});}
  ${TCM+__ebi_pa_start__ = ALIGN ((DEFINED (EBI_PA_START)? EBI_PA_START : .), ${MAXPAGESIZE});}

  ${TCM+. = __ebi_va_start__;}
  ${TCM+__ebi_start = __ebi_pa_start__;}

  ${INITIAL_READONLY_SECTIONS}
  .note.gnu.build-id : { *(.note.gnu.build-id) }
EOF

test -n "${RELOCATING+0}" || unset NON_ALLOC_DYN
test -z "${NON_ALLOC_DYN}" || TEXT_DYNAMIC=
cat > ldscripts/dyntmp.$$ <<EOF
  ${TEXT_DYNAMIC+${DYNAMIC}}
  .hash         ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.hash)     - __ebi_va_start__)} { *(.hash) }
  .gnu.hash     ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.gnu.hash) - __ebi_va_start__)} { *(.gnu.hash) }
  .dynsym       ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.dynsym)   - __ebi_va_start__)} { *(.dynsym) }
  .dynstr       ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.dynstr)   - __ebi_va_start__)} { *(.dynstr) }
  .gnu.version  ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.gnu.version)   - __ebi_va_start__)} { *(.gnu.version) }
  .gnu.version_d ${RELOCATING-0}: ${TCM+AT (__ebi_pa_start__ + ADDR (.gnu.version_d) - __ebi_va_start__)} { *(.gnu.version_d) }
  .gnu.version_r ${RELOCATING-0}: ${TCM+AT (__ebi_pa_start__ + ADDR (.gnu.version_r) - __ebi_va_start__)} { *(.gnu.version_r) }
EOF

if [ "x$COMBRELOC" = x ]; then
  COMBRELOCCAT="cat >> ldscripts/dyntmp.$$"
else
  COMBRELOCCAT="cat > $COMBRELOC"
fi
eval $COMBRELOCCAT <<EOF
  .rela.init    ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.init) - __ebi_va_start__)} { *(.rela.init) }
  .rela.text    ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.text) - __ebi_va_start__)} { *(.rela.text${RELOCATING+ .rela.text.* .rela.gnu.linkonce.t.*}) }
  .rela.fini    ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.fini) - __ebi_va_start__)} { *(.rela.fini) }
  .rela.rodata  ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.rodata) - __ebi_va_start__)} { *(.rela.rodata${RELOCATING+ .rela.rodata.* .rela.gnu.linkonce.r.*}) }
  ${OTHER_READONLY_RELOC_SECTIONS}
  .rela.data.rel.ro ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.data.rel.ro) - __ebi_va_start__)} { *(.rela.data.rel.ro${RELOCATING+* .rela.gnu.linkonce.d.rel.ro.*}) }
  .rela.data    ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.data) - __ebi_va_start__)} { *(.rela.data${RELOCATING+ .rela.data.* .rela.gnu.linkonce.d.*}) }
  .rela.tdata	${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.tdata) - __ebi_va_start__)} { *(.rela.tdata${RELOCATING+ .rela.tdata.* .rela.gnu.linkonce.td.*}) }
  .rela.tbss	${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.tbss) - __ebi_va_start__)} { *(.rela.tbss${RELOCATING+ .rela.tbss.* .rela.gnu.linkonce.tb.*}) }
  .rela.ctors   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.ctors) - __ebi_va_start__)} { *(.rela.ctors) }
  .rela.dtors   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.dtors) - __ebi_va_start__)} { *(.rela.dtors) }
  .rela.got     ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.got) - __ebi_va_start__)} { *(.rela.got) }
  ${OTHER_GOT_RELOC_SECTIONS}
  ${REL_SDATA}
  ${REL_SBSS}
  ${REL_SDATA2}
  ${REL_SBSS2}
  .rela.bss     ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.bss) - __ebi_va_start__)} { *(.rela.bss${RELOCATING+ .rela.bss.* .rela.gnu.linkonce.b.*}) }
  ${REL_LARGE}
  ${IREL_IN_PLT+$REL_IFUNC}
  ${IREL_IN_PLT+$RELA_IFUNC}
  ${IREL_IN_PLT-$REL_IPLT}
  ${IREL_IN_PLT-$RELA_IPLT}
EOF

if [ -n "$COMBRELOC" ]; then
cat >> ldscripts/dyntmp.$$ <<EOF
  .rela.dyn     ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.dyn) - __ebi_va_start__)}
    {
EOF
sed -e '/^[ 	]*[{}][ 	]*$/d;/:[ 	]*$/d;/\.rel\./d;s/^.*: .*:*.*{ *\(.*\)}$/      \1/' $COMBRELOC >> ldscripts/dyntmp.$$
cat >> ldscripts/dyntmp.$$ <<EOF
    }
EOF
fi

cat >> ldscripts/dyntmp.$$ <<EOF
  .rela.plt     ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rela.plt) - __ebi_va_start__)}
    {
      *(.rela.plt)
      ${IREL_IN_PLT+${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__rela_iplt_start = .);}}}
      ${IREL_IN_PLT+${RELOCATING+*(.rela.iplt)}}
      ${IREL_IN_PLT+${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__rela_iplt_end = .);}}}
    }
  ${OTHER_PLT_RELOC_SECTIONS}
EOF

if test -z "${NON_ALLOC_DYN}"; then
  if test -z "${NO_REL_RELOCS}${NO_RELA_RELOCS}"; then
    cat ldscripts/dyntmp.$$
  else
    if test -z "${NO_REL_RELOCS}"; then
      sed -e '/^[ 	]*\.rela\.[^}]*$/,/}/d' -e '/^[ 	]*\.rela\./d' ldscripts/dyntmp.$$
    fi
    if test -z "${NO_RELA_RELOCS}"; then
      sed -e '/^[ 	]*\.rel\.[^}]*$/,/}/d' -e '/^[ 	]*\.rel\./d' ldscripts/dyntmp.$$
    fi
  fi
  rm -f ldscripts/dyntmp.$$
fi

cat <<EOF
/* Code starts. */
  ${RELOCATING+. = ALIGN (DEFINED (TEXTALIGN)? (TEXTALIGN * 1K) : ${MAXPAGESIZE});}
  ${TCM+. = ALIGN (DEFINED (EBI_CODE_CACHED_ALIGN)? EBI_CODE_CACHED_ALIGN : ${MAXPAGESIZE});}

  ${TCM+.BOOTUP : "{}" :BOOTUP}

  .start        ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.start) - __ebi_va_start__)}
  {
    ${RELOCATING+${INIT_START}}
    KEEP (*(.start))
    ${RELOCATING+${INIT_END}}
  } =${NOP-0}

  ${TCM+.CODE : "{}" :CODE}

  .init         ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.init) - __ebi_va_start__)}
  {
    ${RELOCATING+${INIT_START}}
    KEEP (*(.init))
    ${RELOCATING+${INIT_END}}
  } =${NOP-0}

  ${TEXT_PLT+${PLT}}
  ${TINY_READONLY_SECTION}
  .text         ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.text) - __ebi_va_start__)}
  {
    ${RELOCATING+${TEXT_START_SYMBOLS}}
    ${RELOCATING+*(.text.unlikely .text.*_unlikely)}
    *(.text.hot .text.hot.* .gnu.linkonce.t.hot.*)
    *(.text .stub${RELOCATING+ .text.* .gnu.linkonce.t.*})
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
    ${RELOCATING+${OTHER_TEXT_SECTIONS}}
  } =${NOP-0}
  .fini         ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.fini) - __ebi_va_start__)}
  {
    ${RELOCATING+${FINI_START}}
    KEEP (*(.fini))
    ${RELOCATING+${FINI_END}}
  } =${NOP-0}
  ${RELOCATING+PROVIDE (__${ETEXT_NAME} = .);}
  ${RELOCATING+PROVIDE (_${ETEXT_NAME} = .);}
  ${RELOCATING+PROVIDE (${ETEXT_NAME} = .);}

/* Constants start. */
  ${RELOCATING+. = ALIGN (DEFINED (RODATAALIGN)? (RODATAALIGN * 1K) : ${MAXPAGESIZE});}
  ${TCM+. = ALIGN (DEFINED (EBI_DATA_CACHED_ALIGN)? EBI_DATA_CACHED_ALIGN : ${MAXPAGESIZE});}

  ${TCM+.CONST : "{}" :CONST}

  ${WRITABLE_RODATA-${RODATA}}
  .rodata1      ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.rodata1) - __ebi_va_start__)} { *(.rodata1) }
  .eh_frame_hdr : ${TCM+AT (__ebi_pa_start__ + ADDR (.eh_frame_hdr) - __ebi_va_start__)} { *(.eh_frame_hdr) }
  .eh_frame     ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.eh_frame) - __ebi_va_start__)} ONLY_IF_RO { KEEP (*(.eh_frame)) }
  .gcc_except_table ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.gcc_except_table) - __ebi_va_start__)} ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }
  ${OTHER_READONLY_SECTIONS}

/* Data start. */
  /* Adjust the address for the data segment.  We want to adjust up to
     the same address within the page on the next page up.  */
  ${CREATE_SHLIB-${CREATE_PIE-${RELOCATING+. = ${DATA_ADDR-${DATA_SEGMENT_ALIGN}};}}}
  ${CREATE_SHLIB+${RELOCATING+. = ${SHLIB_DATA_ADDR-${DATA_SEGMENT_ALIGN}};}}
  ${CREATE_PIE+${RELOCATING+. = ${SHLIB_DATA_ADDR-${DATA_SEGMENT_ALIGN}};}}

  ${RELOCATING+. = ALIGN (DEFINED (DATAALIGN)? (DATAALIGN * 1K) : ${MAXPAGESIZE});}
  ${TCM+. = ALIGN (DEFINED (EBI_DATA_CACHED_ALIGN)? EBI_DATA_CACHED_ALIGN : ${MAXPAGESIZE});}

  ${TCM+.DATA : "{}" :DATA}

  /* Exception handling  */
  .eh_frame     ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.eh_frame) - __ebi_va_start__)} ONLY_IF_RW { KEEP (*(.eh_frame)) }
  .gcc_except_table ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.gcc_except_table) - __ebi_va_start__)} ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }

  /* Thread Local Storage sections  */
  .tdata	${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.tdata) - __ebi_va_start__)} { *(.tdata${RELOCATING+ .tdata.* .gnu.linkonce.td.*}) }
  .tbss		${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.tbss) - __ebi_va_start__)} { *(.tbss${RELOCATING+ .tbss.* .gnu.linkonce.tb.*})${RELOCATING+ *(.tcommon)} }

  .preinit_array   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.preinit_array) - __ebi_va_start__)}
  {
    ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__preinit_array_start = .);}}
    KEEP (*(.preinit_array))
    ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__preinit_array_end = .);}}
  }
  .init_array   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.init_array) - __ebi_va_start__)}
  {
     ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__init_array_start = .);}}
     KEEP (*(SORT(.init_array.*)))
     KEEP (*(.init_array))
     ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__init_array_end = .);}}
  }
  .fini_array   ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.fini_array) - __ebi_va_start__)}
  {
    ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__fini_array_start = .);}}
    KEEP (*(.fini_array))
    KEEP (*(SORT(.fini_array.*)))
    ${RELOCATING+${CREATE_SHLIB-PROVIDE_HIDDEN (${USER_LABEL_PREFIX}__fini_array_end = .);}}
  }
  ${SMALL_DATA_CTOR-${RELOCATING+${CTOR}}}
  ${SMALL_DATA_DTOR-${RELOCATING+${DTOR}}}
  .jcr          ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.jcr) - __ebi_va_start__)} { KEEP (*(.jcr)) }

  ${RELOCATING+${DATARELRO}}
  ${OTHER_RELRO_SECTIONS}
  ${TEXT_DYNAMIC-${DYNAMIC}}
  ${DATA_GOT+${RELRO_NOW+${GOT}}}
  ${DATA_GOT+${RELRO_NOW+${GOTPLT}}}
  ${DATA_GOT+${RELRO_NOW-${SEPARATE_GOTPLT+${GOT}}}}
  ${RELOCATING+${DATA_SEGMENT_RELRO_END}}
  ${DATA_GOT+${RELRO_NOW-${SEPARATE_GOTPLT-${GOT}}}}
  ${DATA_GOT+${RELRO_NOW-${GOTPLT}}}

  ${DATA_PLT+${PLT_BEFORE_GOT-${PLT}}}

  .data         ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.data) - __ebi_va_start__)}
  {
    ${RELOCATING+${DATA_START_SYMBOLS}}
    *(.data.hot${RELOCATING+ .data.hot.* .gnu.linkonce.d.hot.*})
    *(.data${RELOCATING+ .data.* .gnu.linkonce.d.*})
    ${CONSTRUCTING+SORT(CONSTRUCTORS)}
  }
  .data1        ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.data1) - __ebi_va_start__)} { *(.data1) }
  ${WRITABLE_RODATA+${RODATA}}
  ${OTHER_READWRITE_SECTIONS}
  ${DATA_PLT+${PLT_BEFORE_GOT+${PLT}}}
  ${RELOCATING+${DATA_END_SYMBOLS-${USER_LABEL_PREFIX}_edata = .; PROVIDE (${USER_LABEL_PREFIX}edata = .);}}

  ${RELOCATING+. = ALIGN (${ALIGNMENT});}
  ${RELOCATING+${USER_LABEL_PREFIX}__bss_start = .;}
  ${RELOCATING+${OTHER_BSS_SYMBOLS}}
  ${BSS_PLT+${PLT}}
  .bss          ${RELOCATING-0} : ${TCM+AT (__ebi_pa_start__ + ADDR (.bss) - __ebi_va_start__)}
  {
   *(.dynbss)
   *(.bss.hot${RELOCATING+ .bss.hot.* .gnu.linkonce.b.hot.*})
   *(.bss${RELOCATING+ .bss.* .gnu.linkonce.b.*})
   *(COMMON)
   /* Align here to ensure that the .bss section occupies space up to
      _end.  Align after .bss to ensure correct alignment even if the
      .bss section disappears because there are no input sections. */
   ${RELOCATING+. = ALIGN(. != 0 ? ${ALIGNMENT} : 1);}
  }
  ${OTHER_BSS_SECTIONS}
  ${RELOCATING+. = ALIGN(${ALIGNMENT});}
  ${RELOCATING+${OTHER_BSS_END_SYMBOLS}}
  ${RELOCATING+${END_SYMBOLS-${USER_LABEL_PREFIX}_end = .;}}
  ${LARGE_SECTIONS}
  ${TCM+${EBI_DATA_WT}
  ${EBI_DATA_UN}}

/* Small data start. */
  ${RELOCATING+. = ALIGN (DEFINED (DATAALIGN)? (DATAALIGN * 1K) : 512K);}
  ${TCM+. = ALIGN (DEFINED (EBI_DATA_CACHED_ALIGN)? EBI_DATA_CACHED_ALIGN : 512K);}

  ${TCM+.SDATA : "{}" :SDATA}

  ${SDATA}
  ${SMALL_DATA_CTOR+${RELOCATING+${CTOR}}}
  ${SMALL_DATA_DTOR+${RELOCATING+${DTOR}}}
  ${SDATA_GOT+${RELOCATING+${OTHER_GOT_SYMBOLS}}}
  ${SDATA_GOT+${GOT}}
  ${SDATA_GOT+${OTHER_GOT_SECTIONS}}
  ${OTHER_SDATA_SECTIONS}
  ${SBSS}
  ${RELOCATING+. = ALIGN(${ALIGNMENT});}
  ${RELOCATING+${OTHER_END_SYMBOLS}}
  ${RELOCATING+${END_SYMBOLS-PROVIDE (${USER_LABEL_PREFIX}end = .);}}
  ${RELOCATING+${DATA_SEGMENT_END}}
EOF

if test -n "${NON_ALLOC_DYN}"; then
  if test -z "${NO_REL_RELOCS}${NO_RELA_RELOCS}"; then
    cat ldscripts/dyntmp.$$
  else
    if test -z "${NO_REL_RELOCS}"; then
      sed -e '/^[ 	]*\.rela\.[^}]*$/,/}/d' -e '/^[ 	]*\.rela\./d' ldscripts/dyntmp.$$
    fi
    if test -z "${NO_RELA_RELOCS}"; then
      sed -e '/^[ 	]*\.rel\.[^}]*$/,/}/d' -e '/^[ 	]*\.rel\./d' ldscripts/dyntmp.$$
    fi
  fi
  rm -f ldscripts/dyntmp.$$
fi

cat <<EOF

  ${TCM+__ebi_end = __ebi_pa_start__ + . - __ebi_va_start__;}

${TCM+/* Start SMI memory. */}
  ${TCM+__smi_va_start__ = ALIGN ((DEFINED (SMI_VA_START)? SMI_VA_START : .), ${MAXPAGESIZE});}
  ${TCM+__smi_pa_start__ = ALIGN ((DEFINED (SMI_PA_START)? SMI_PA_START : .), ${MAXPAGESIZE});}

  ${TCM+. = __smi_va_start__;}
  ${TCM+__smi_start = __smi_pa_start__;}

  ${TCM+${SMI_CODE}}
  ${TCM+${SMI_DATA}}
  ${TCM+${SMI_DATA_WT}}
  ${TCM+${SMI_DATA_UN}}

  ${TCM+__smi_end = __smi_pa_start__ + . - __smi_va_start__;}

${TCM+/* Start TCM memory. */}
  ${TCM+__tcm_va_start__ = ALIGN ((DEFINED (TCM_VA_START)? TCM_VA_START : ${TCM_ADDRESS-0xd8000000}), ${MAXPAGESIZE});}
  ${TCM+__tcm_pa_start__ = ALIGN ((DEFINED (TCM_PA_START)? TCM_PA_START : ${TCM_ADDRESS-0xd8000000}), ${MAXPAGESIZE});}

  ${TCM+. = __tcm_va_start__;}
  ${TCM+__tcm_start = __tcm_pa_start__;}

  ${TCM+${TCM_CODE}}
  ${TCM+${TCM_DATA}}
  ${TCM+${TCM_DATA_WT}}
  ${TCM+${TCM_DATA_UN}}

  ${TCM+__tcm_end = __tcm_pa_start__ + . - __tcm_va_start__;}

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }

  .comment       0 : { *(.comment) }

  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */

  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }

  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }

  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }

  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info${RELOCATING+ .gnu.linkonce.wi.*}) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }

  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }

  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }

  ${TINY_DATA_SECTION}
  ${TINY_BSS_SECTION}

  ${STACK_ADDR+${STACK}}
  ${ATTRS_SECTIONS}
  ${OTHER_SECTIONS}
  ${RELOCATING+${OTHER_SYMBOLS}}
  ${RELOCATING+${DISCARDED}}
}
EOF
