

/* 
 * Function Prototypes:
 */
static bfd_reloc_status_type hexagon_elf_reloc
  (bfd *, arelent *, asymbol *, PTR , asection *, bfd *, char **);

static bfd_reloc_status_type hexagon_elf_final_sda_base
  (bfd *, char **, bfd_vma *);

static int hexagon_reloc_operand
  (reloc_howto_type *howto, hexagon_insn *insn, bfd_vma offset, char **errmsg);

static bfd_reloc_code_real_type hexagon_elf_reloc_val_lookup
  (unsigned char elf_reloc_val);

static int hexagon_elf_section_processing
  (bfd *abfd __attribute__ ((unused)), Elf_Internal_Shdr *hdr);

static void hexagon_elf_symbol_processing
  (bfd *abfd, asymbol *asym);

static bfd_boolean hexagon_elf_section_from_bfd_section
  (bfd *abfd __attribute__ ((unused)), asection *sec, int *retval);

static bfd_boolean hexagon_elf_link_output_symbol_hook
  (bfd *abfd ATTRIBUTE_UNUSED,
   struct bfd_link_info *info ATTRIBUTE_UNUSED,
   const char *name ATTRIBUTE_UNUSED, Elf_Internal_Sym *sym,
   asection *input_sec);


/*
 * Macro Definitions:
 */
#define SDA_BASE "_SDA_BASE_"

/*
 * Early alias for _SDA_BASE_.
 */
#define DEFAULT_SDA_BASE "__default_sda_base__"


/*
 * XXX_SM: cooked is done, section->size should just work.
 */
#define get_section_size_now(abfd,sec) 					\
    bfd_section_size (abfd, sec)

#define HEXAGON_TRAMPOLINE_PREFIX     ".PAD"
#define HEXAGON_TRAMPOLINE_PREFIX_LEN (sizeof (HEXAGON_TRAMPOLINE_PREFIX))


/*
 * Data Structures:
 */
typedef struct
  {
    bfd_vma rtype, offset;
  } hexagon_trampoline_rel;

static const hexagon_trampoline_rel hexagon_trampoline_rels [] =
  {
    {R_HEXAGON_HL16, 2 * sizeof (hexagon_insn)},
  };

static const hexagon_insn hexagon_trampoline [] =
  {
    /* This trampoline requires 1 special
       relocation, but takes a cycle longer.   */
    0xbffd7f1d, /*  { sp = add (sp, #-8)       */
    0xa79dfcfe, /*    memw (sp + #-8) = r28 }  */
    0x723cc000, /*  r28.h = #HI (foo)          */
    0x713cc000, /*  r28.l = #LO (foo)          */
    0xb01d411d, /*  { sp = add (sp, #8)        */
    0x529c4000, /*    jumpr r28                */
    0x919dc01c, /*    r28 = memw (sp) }        */
  };

    /* This trampoline requires 1 relocation,
       but mixes code and data in a page.     */
    /* 0x6a09401c, { r28 = pc                 */
    /* 0xbffd7f1d,   r29 = add (r29, #-8)     */
    /* 0xa79dfcfe,   memw (r29 + #-8) = r28 } */
    /* 0x919cc0fc, r28 = memw (r28 + #28)     */
    /* 0xb01d411d, { r29 = add (r29, #8)      */
    /* 0x529c4000,   jumpr r28                */
    /* 0x919dc01c,   r28 = memw (r29) }       */
    /* 0x00000000, .word foo                  */

    /* This trampoline requires 2 relocations. */
    /* 0xbffd7f1d,  { sp = add (sp, #-8)       */
    /* 0x723c4000,    r28.h = #HI (foo)        */
    /* 0xa79dfcfe,    memw (sp + #-8) = r28 }  */
    /* 0x713cc000,  r28.l = #LO (foo)          */
    /* 0xb01d411d,  { sp = add (sp, #8)        */
    /* 0x529c4000,    jumpr r28                */
    /* 0x919dc01c,    r28 = memw (sp) }        */


