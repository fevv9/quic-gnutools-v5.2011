##################################################################

# This file is sourced from elf32.em, and defines extra
# QDSP6-specific routines.

cat >>e${EMULATION_NAME}.c <<EOF

#include "elf/internal.h"
#include "elf/qdsp6.h"
#include "elf-bfd.h"

#define EMUL_QDSP6

static void qdsp6_after_parse PARAMS ((void));

static char* qdsp6_arch_name;
static int qdsp6_cmdline_set_arch;

struct qdsp6_march
{
  char *march_name_fe, *march_short_fe;
  char *march_name_be;
};

static const struct qdsp6_march qdsp6_marchs [] =
{
  {"qdsp6v2", "v2", "qdsp6v2"},
  {"qdsp6v3", "v3", "qdsp6v3"},
  {"qdsp6v4", "v4", "qdsp6v4"},
};

static size_t qdsp6_marchs_size =
  sizeof (qdsp6_marchs) / sizeof (qdsp6_marchs [0]);

static void
qdsp6_after_parse (void)
{
  /* Do not try to relax to add trampolines with -r. */
  if (link_info.qdsp6_trampolines)
    link_info.qdsp6_trampolines = command_line.relax = !link_info.relocatable;

  /* It likely does not make sense to have a DSO using the TCM. */
  if (config.use_tcm && link_info.shared)
    einfo (_("%P%F: \"-tcm\" not supported with \"-shared\"\n"));

  after_parse_default();
}

EOF

# Put these extra routines in ld_${EMULATION_NAME}_emulation

LDEMUL_AFTER_PARSE=qdsp6_after_parse

# Define some shell vars to insert bits of code into the standard elf
# parse_args and list_options functions.

# QDSP6 option values (between 300 and 399).
PARSE_AND_LIST_PROLOGUE=$'
#define OPTION_MQDSP6V1    (301)
#define OPTION_MQDSP6V2    (302)
#define OPTION_MQDSP6V3    (303)
#define OPTION_MQDSP6V4    (304)
#define OPTION_MARCH       (305)
#define OPTION_MCPU        (306)
#define OPTION_TCM         (307)
#define OPTION_TRAMPOLINES (308)
'

# QDSP6 options.
# Because the option `-m' is overloaded here, proper code must be added to get_emulation ().
PARSE_AND_LIST_LONGOPTS=$'
    {"mv2",         no_argument,       NULL, OPTION_MQDSP6V2},
    {"mv3",         no_argument,       NULL, OPTION_MQDSP6V3},
    {"mv4",         no_argument,       NULL, OPTION_MQDSP6V4},
    {"march",       required_argument, NULL, OPTION_MARCH},
    {"mcpu",        required_argument, NULL, OPTION_MCPU},
    {"tcm",         no_argument,       NULL, OPTION_TCM},
    {"trampolines", optional_argument, NULL, OPTION_TRAMPOLINES},
'

# QDSP6 target help.
PARSE_AND_LIST_OPTIONS=$'
  fprintf (file, _("  --march={v2|v3|v4}          Link for the specified QDSP6 architecture\\n"));
  fprintf (file, _("  --mcpu={v2|v3|v4}           Equivalent to `--march\'\\n"));
  fprintf (file, _("  -m{v2|v3|v4}                Equivalent to `--march\'\\n"));
  fprintf (file, _("  --tcm                       Use the TCM\\n"));
  fprintf (file, _("  --trampolines[={yes|no}]    Add trampolines when necessary (default)\\n"));
'

# QDSP6 option processing.
PARSE_AND_LIST_ARGS_CASES=$'
    case 1: /* File name. */
      /* Unless the output architecture is specified, then it is determined by
         the input files according to qdsp6_bfd_compatible (). */
      if (!qdsp6_cmdline_set_arch)
        {
          static const struct bfd_arch_info *arch_info = NULL;
          bfd *in_bfd;
          char *in_bfd_name;
          char **matching;
          size_t i;

          in_bfd = bfd_openr (optarg, NULL);
          if (in_bfd == NULL)
            {
              einfo(_("%P%F: Cannot read file \'%s\'\\n"), optarg);
              xexit(1);
            }

          if (bfd_check_format_matches (in_bfd, bfd_object, &matching))
            {
              in_bfd_name = (char*) bfd_printable_name (in_bfd);
              if (!in_bfd_name)
                {
                  einfo (_("%P%F: Does not recognize the architecture of file \'%s\'.\\n"), optarg);
                  xexit (1);
                }

              arch_info = bfd_lookup_arch
                (ldfile_output_architecture, ldfile_output_machine);
              arch_info = (bfd_get_arch_info (in_bfd))->compatible
                (bfd_get_arch_info (in_bfd), arch_info);
              if (arch_info)
                {
                  ldfile_output_machine      = arch_info->mach;
                  ldfile_output_machine_name = arch_info->printable_name;
                }

              for (i = 0; i < qdsp6_marchs_size; i++)
                if (!strcmp (in_bfd_name, qdsp6_marchs [i].march_name_be))
                  {
                    /* Only set if the cmdline has not set the type yet. */
                    ldfile_set_output_arch (qdsp6_marchs [i].march_name_be, bfd_arch_qdsp6);

                    break;
                  }

              bfd_close (in_bfd);
            }
        }
      return FALSE;

    case OPTION_TCM:
      config.use_tcm = TRUE;
      break;

    case OPTION_TRAMPOLINES:
      link_info.qdsp6_trampolines = optarg
                                    ? !strncasecmp (optarg, N_("y"), 1)
                                    : TRUE;
      break;

    case OPTION_MQDSP6V1:
    case OPTION_MQDSP6V2:
    case OPTION_MQDSP6V3:
    case OPTION_MQDSP6V4:
    case OPTION_MARCH:
    case OPTION_MCPU:
      /* The output architecture is specified. */
        {
          char *temp_qdsp6_arch_name = NULL;
          size_t i;

          switch (optc)
            {
              case OPTION_MQDSP6V2:
              case OPTION_MQDSP6V3:
              case OPTION_MQDSP6V4:
                /* -mv* options. */
                temp_qdsp6_arch_name
                  = qdsp6_marchs [optc - OPTION_MQDSP6V2].march_name_be;
                break;

              default:
                /* -march and- mcpu options. */
                for (i = 0; i < qdsp6_marchs_size; i++)
                  if (   !strcmp (optarg, qdsp6_marchs [i].march_name_fe)
                      || !strcmp (optarg, qdsp6_marchs [i].march_short_fe))
                    {
                      temp_qdsp6_arch_name = qdsp6_marchs [i].march_name_be;
                      break;
                    }

                if (i == qdsp6_marchs_size)
                  {
                    einfo (_("invalid architecture `%s\'.\\n"));
                    xexit (1);
                  }

                break;
            }

          if (qdsp6_cmdline_set_arch)
            if (strcmp (temp_qdsp6_arch_name, qdsp6_arch_name) != 0)
              {
                einfo (_("conflicting architectures specified.\\n"));
                xexit (1);
              }

          qdsp6_arch_name = temp_qdsp6_arch_name;
          qdsp6_cmdline_set_arch = TRUE;
          ldfile_set_output_arch (qdsp6_arch_name, bfd_arch_qdsp6);
        }
      break;
'
source_em ${srcdir}/emultempl/needrelax.em
