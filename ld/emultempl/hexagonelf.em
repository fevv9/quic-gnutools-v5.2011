##################################################################

# This file is sourced from elf32.em, and defines extra
# Hexagon-specific routines.

case "${target_alias}" in
  hexagon*-linux*)
    HEXAGON_IS_LINUX=" "
    # The SDA is not supported in Linux.
    HEXAGON_LINUX_G_SWITCH=$'
      else
        {
          g_switch_value = 0;
          einfo (_("%P: small data size reset to zero\\n"));
        }
    '
    # The TCM is not supported in Linux.
    HEXAGON_LINUX_USE_TCM=$'
      else
        einfo (_("%P%F: `-tcm\\\' not supported\\n"));
    '
    ;;
  hexagon*)
    ;;
esac

cat >>e${EMULATION_NAME}.c <<EOF

#include "elf/internal.h"
#include "elf/hexagon.h"
#include "elf-bfd.h"

#define EMUL_HEXAGON

static char* hexagon_arch_name;
static int hexagon_cmdline_set_arch;

struct hexagon_march
{
  char *march_name_fe, *march_alt_fe, *march_short_fe;
  char *march_name_be;
};

static const struct hexagon_march hexagon_marchs [] =
{
  {"hexagonv2", "qdsp6v2", "v2", "hexagonv2"},
  {"hexagonv3", "qdsp6v3", "v3", "hexagonv3"},
  {"hexagonv4", "qdsp6v4", "v4", "hexagonv4"},
};

static size_t hexagon_marchs_size =
  sizeof (hexagon_marchs) / sizeof (hexagon_marchs [0]);

static void
hexagon_after_parse (void)
{
  /* Do not try to relax to add trampolines with -r. */
  if (link_info.hexagon_trampolines)
    link_info.hexagon_trampolines = command_line.relax = !link_info.relocatable;

  /* It likely does not make sense to have a DSO using the TCM. */
  if (config.use_tcm)
    {
      if (link_info.shared)
        einfo (_("%P%F: \`-tcm\' not supported with \`-shared\'\\n"));
      ${HEXAGON_LINUX_USE_TCM}
    }

  if (g_switch_value)
    {
      /* The compiler doesn't set up the GP register, which precludes the SDA
         in some situations. */
      if (link_info.shared)
        {
          g_switch_value = 0;
          einfo (_("%P: small data size set to zero with \`-shared\'\\n"));
        }
      ${HEXAGON_LINUX_G_SWITCH}
    }

  after_parse_default ();
}

EOF

test -n "$HEXAGON_IS_LINUX" && cat >>e${EMULATION_NAME}.c <<EOF

static void
hexagon_before_parse (void)
{
  gld${EMULATION_NAME}_before_parse ();

  /* Default to no SDA. */
  g_switch_value = 0;
}

EOF

# Put these extra routines in ld_${EMULATION_NAME}_emulation
LDEMUL_AFTER_PARSE=hexagon_after_parse
test -n "$HEXAGON_IS_LINUX" && LDEMUL_BEFORE_PARSE=hexagon_before_parse

# Hexagon option values (between 300 and 399).
PARSE_AND_LIST_PROLOGUE=$'
#define OPTION_MV2         (301)
#define OPTION_MV3         (302)
#define OPTION_MV4         (303)
#define OPTION_MARCH       (304)
#define OPTION_MCPU        (305)
#define OPTION_TCM         (306)
#define OPTION_TRAMPOLINES (307)
'

# Hexagon options.
# Because the option `-m' is overloaded here, proper code must be added to get_emulation ().
PARSE_AND_LIST_LONGOPTS=$'
    {"mv2",         no_argument,       NULL, OPTION_MV2},
    {"mv3",         no_argument,       NULL, OPTION_MV3},
    {"mv4",         no_argument,       NULL, OPTION_MV4},
    {"march",       required_argument, NULL, OPTION_MARCH},
    {"mcpu",        required_argument, NULL, OPTION_MCPU},
    {"tcm",         no_argument,       NULL, OPTION_TCM},
    {"trampolines", optional_argument, NULL, OPTION_TRAMPOLINES},
'

# Hexagon target help.
PARSE_AND_LIST_OPTIONS=$'
  fprintf (file, _("  --march={v2|v3|v4}          Link for the specified Hexagon architecture\\n"));
  fprintf (file, _("  --mcpu={v2|v3|v4}           Equivalent to `--march\'\\n"));
  fprintf (file, _("  -m{v2|v3|v4}                Equivalent to `--march\'\\n"));
'
test -z "$HEXAGON_IS_LINUX" && PARSE_AND_LIST_OPTIONS=$PARSE_AND_LIST_OPTIONS$'
  fprintf (file, _("  --tcm                       Use the TCM\\n"));
'
PARSE_AND_LIST_OPTIONS=$PARSE_AND_LIST_OPTIONS$'
  fprintf (file, _("  --trampolines[={yes|no}]    Add trampolines when necessary (default)\\n"));
'

# Hexagon option processing.
PARSE_AND_LIST_ARGS_CASES=$'
    case 1: /* File name. */
      /* Unless the output architecture is specified, then it is determined by
         the input files according to hexagon_bfd_compatible (). */
      if (!hexagon_cmdline_set_arch)
        {
          static const struct bfd_arch_info *arch_info = NULL;
          bfd *in_bfd;
          char *in_bfd_name;
          char **matching;
          size_t i;

          in_bfd = bfd_openr (optarg, NULL);
          if (in_bfd == NULL)
            {
              einfo (_("%P%F: Cannot read file `%s\'\\n"), optarg);
              xexit (1);
            }

          if (bfd_check_format_matches (in_bfd, bfd_object, &matching))
            {
              in_bfd_name = (char*) bfd_printable_name (in_bfd);
              if (!in_bfd_name)
                {
                  einfo (_("%P%F: Does not recognize the architecture of file `%s\'.\\n"), optarg);
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

              for (i = 0; i < hexagon_marchs_size; i++)
                if (!strcmp (in_bfd_name, hexagon_marchs [i].march_name_be))
                  {
                    /* Only set if the cmdline has not set the type yet. */
                    ldfile_set_output_arch (hexagon_marchs [i].march_name_be, bfd_arch_hexagon);

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
      link_info.hexagon_trampolines = optarg
                                    ? !strncasecmp (optarg, N_("y"), 1)
                                    : TRUE;
      break;

    case OPTION_MV2:
    case OPTION_MV3:
    case OPTION_MV4:
    case OPTION_MARCH:
    case OPTION_MCPU:
      /* The output architecture is specified. */
        {
          char *temp_hexagon_arch_name = NULL;
          size_t i;

          switch (optc)
            {
              case OPTION_MV2:
              case OPTION_MV3:
              case OPTION_MV4:
                /* -mv* options. */
                temp_hexagon_arch_name
                  = hexagon_marchs [optc - OPTION_MV2].march_name_be;
                break;

              default:
                /* -march and- mcpu options. */
                for (i = 0; i < hexagon_marchs_size; i++)
                  if (!strcmp (optarg, hexagon_marchs [i].march_name_fe)
                      || !strcmp (optarg, hexagon_marchs [i].march_alt_fe)
                      || !strcmp (optarg, hexagon_marchs [i].march_short_fe))
                    {
                      temp_hexagon_arch_name = hexagon_marchs [i].march_name_be;
                      break;
                    }

                if (i == hexagon_marchs_size)
                  {
                    einfo (_("invalid architecture `%s\'.\\n"));
                    xexit (1);
                  }

                break;
            }

          if (hexagon_cmdline_set_arch)
            if (strcmp (temp_hexagon_arch_name, hexagon_arch_name) != 0)
              {
                einfo (_("conflicting architectures specified.\\n"));
                xexit (1);
              }

          hexagon_arch_name = temp_hexagon_arch_name;
          hexagon_cmdline_set_arch = TRUE;
          ldfile_set_output_arch (hexagon_arch_name, bfd_arch_hexagon);
        }
      break;
'

source_em ${srcdir}/emultempl/needrelax.em
