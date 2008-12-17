

/* 
 * Function Prototypes:
 */
static bfd_reloc_status_type qdsp6_elf_reloc
  (bfd *, arelent *, asymbol *, PTR , asection *, bfd *, char **);

static bfd_reloc_status_type qdsp6_elf_final_sda_base
  (bfd *, char **, bfd_vma *);

static int qdsp6_reloc_operand
  (reloc_howto_type *howto, qdsp6_insn *insn, bfd_vma offset, char **errmsg);

static bfd_reloc_code_real_type qdsp6_elf_reloc_val_lookup
  (unsigned char elf_reloc_val);




/*
 * Macro Definitions:
 */
#define SDA_BASE "_SDA_BASE_"

/*
 * Early alias for _SDA_BASE_.
 */
#define DEFAULT_SDA_BASE "__default_sda_base__"


#define get_section_size_now(abfd,sec) 					\
  (sec->reloc_done 							\
    ? bfd_get_section_size_after_reloc (sec) 				\
    : bfd_get_section_size_before_reloc (sec))

#define QDSP6_TRAMPOLINE_PREFIX     ".PAD"
#define QDSP6_TRAMPOLINE_PREFIX_LEN (sizeof (QDSP6_TRAMPOLINE_PREFIX))

