#! /bin/sh

# Guess values for system-dependent variables and create Makefiles.
# Generated automatically using autoconf version 2.13 
# Copyright (C) 1992, 93, 94, 95, 96 Free Software Foundation, Inc.
#
# This configure script is free software; the Free Software Foundation
# gives unlimited permission to copy, distribute and modify it.

# Defaults:
ac_help=
ac_default_prefix=/usr/local
# Any additions from configure.in:
ac_help="$ac_help
  --enable-threads        build with threads"
ac_help="$ac_help
  --enable-shared         build and link with shared libraries [--enable-shared]"
ac_help="$ac_help
  --enable-64bit          enable 64bit support (where applicable = amd64|ia64)"
ac_help="$ac_help
  --enable-symbols        build with debugging symbols [--disable-symbols]"

# Initialize some variables set by options.
# The variables have the same names as the options, with
# dashes changed to underlines.
build=NONE
cache_file=./config.cache
exec_prefix=NONE
host=NONE
no_create=
nonopt=NONE
no_recursion=
prefix=NONE
program_prefix=NONE
program_suffix=NONE
program_transform_name=s,x,x,
silent=
site=
srcdir=
target=NONE
verbose=
x_includes=NONE
x_libraries=NONE
bindir='${exec_prefix}/bin'
sbindir='${exec_prefix}/sbin'
libexecdir='${exec_prefix}/libexec'
datadir='${prefix}/share'
sysconfdir='${prefix}/etc'
sharedstatedir='${prefix}/com'
localstatedir='${prefix}/var'
libdir='${exec_prefix}/lib'
includedir='${prefix}/include'
oldincludedir='/usr/include'
infodir='${prefix}/info'
mandir='${prefix}/man'

# Initialize some other variables.
subdirs=
MFLAGS= MAKEFLAGS=
SHELL=${CONFIG_SHELL-/bin/sh}
# Maximum number of lines to put in a shell here document.
ac_max_here_lines=12

ac_prev=
for ac_option
do

  # If the previous option needs an argument, assign it.
  if test -n "$ac_prev"; then
    eval "$ac_prev=\$ac_option"
    ac_prev=
    continue
  fi

  case "$ac_option" in
  -*=*) ac_optarg=`echo "$ac_option" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
  *) ac_optarg= ;;
  esac

  # Accept the important Cygnus configure options, so we can diagnose typos.

  case "$ac_option" in

  -bindir | --bindir | --bindi | --bind | --bin | --bi)
    ac_prev=bindir ;;
  -bindir=* | --bindir=* | --bindi=* | --bind=* | --bin=* | --bi=*)
    bindir="$ac_optarg" ;;

  -build | --build | --buil | --bui | --bu)
    ac_prev=build ;;
  -build=* | --build=* | --buil=* | --bui=* | --bu=*)
    build="$ac_optarg" ;;

  -cache-file | --cache-file | --cache-fil | --cache-fi \
  | --cache-f | --cache- | --cache | --cach | --cac | --ca | --c)
    ac_prev=cache_file ;;
  -cache-file=* | --cache-file=* | --cache-fil=* | --cache-fi=* \
  | --cache-f=* | --cache-=* | --cache=* | --cach=* | --cac=* | --ca=* | --c=*)
    cache_file="$ac_optarg" ;;

  -datadir | --datadir | --datadi | --datad | --data | --dat | --da)
    ac_prev=datadir ;;
  -datadir=* | --datadir=* | --datadi=* | --datad=* | --data=* | --dat=* \
  | --da=*)
    datadir="$ac_optarg" ;;

  -disable-* | --disable-*)
    ac_feature=`echo $ac_option|sed -e 's/-*disable-//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_feature| sed 's/[-a-zA-Z0-9_]//g'`"; then
      { echo "configure: error: $ac_feature: invalid feature name" 1>&2; exit 1; }
    fi
    ac_feature=`echo $ac_feature| sed 's/-/_/g'`
    eval "enable_${ac_feature}=no" ;;

  -enable-* | --enable-*)
    ac_feature=`echo $ac_option|sed -e 's/-*enable-//' -e 's/=.*//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_feature| sed 's/[-_a-zA-Z0-9]//g'`"; then
      { echo "configure: error: $ac_feature: invalid feature name" 1>&2; exit 1; }
    fi
    ac_feature=`echo $ac_feature| sed 's/-/_/g'`
    case "$ac_option" in
      *=*) ;;
      *) ac_optarg=yes ;;
    esac
    eval "enable_${ac_feature}='$ac_optarg'" ;;

  -exec-prefix | --exec_prefix | --exec-prefix | --exec-prefi \
  | --exec-pref | --exec-pre | --exec-pr | --exec-p | --exec- \
  | --exec | --exe | --ex)
    ac_prev=exec_prefix ;;
  -exec-prefix=* | --exec_prefix=* | --exec-prefix=* | --exec-prefi=* \
  | --exec-pref=* | --exec-pre=* | --exec-pr=* | --exec-p=* | --exec-=* \
  | --exec=* | --exe=* | --ex=*)
    exec_prefix="$ac_optarg" ;;

  -gas | --gas | --ga | --g)
    # Obsolete; use --with-gas.
    with_gas=yes ;;

  -help | --help | --hel | --he)
    # Omit some internal or obsolete options to make the list less imposing.
    # This message is too long to be a string in the A/UX 3.1 sh.
    cat << EOF
Usage: configure [options] [host]
Options: [defaults in brackets after descriptions]
Configuration:
  --cache-file=FILE       cache test results in FILE
  --help                  print this message
  --no-create             do not create output files
  --quiet, --silent       do not print \`checking...' messages
  --version               print the version of autoconf that created configure
Directory and file names:
  --prefix=PREFIX         install architecture-independent files in PREFIX
                          [$ac_default_prefix]
  --exec-prefix=EPREFIX   install architecture-dependent files in EPREFIX
                          [same as prefix]
  --bindir=DIR            user executables in DIR [EPREFIX/bin]
  --sbindir=DIR           system admin executables in DIR [EPREFIX/sbin]
  --libexecdir=DIR        program executables in DIR [EPREFIX/libexec]
  --datadir=DIR           read-only architecture-independent data in DIR
                          [PREFIX/share]
  --sysconfdir=DIR        read-only single-machine data in DIR [PREFIX/etc]
  --sharedstatedir=DIR    modifiable architecture-independent data in DIR
                          [PREFIX/com]
  --localstatedir=DIR     modifiable single-machine data in DIR [PREFIX/var]
  --libdir=DIR            object code libraries in DIR [EPREFIX/lib]
  --includedir=DIR        C header files in DIR [PREFIX/include]
  --oldincludedir=DIR     C header files for non-gcc in DIR [/usr/include]
  --infodir=DIR           info documentation in DIR [PREFIX/info]
  --mandir=DIR            man documentation in DIR [PREFIX/man]
  --srcdir=DIR            find the sources in DIR [configure dir or ..]
  --program-prefix=PREFIX prepend PREFIX to installed program names
  --program-suffix=SUFFIX append SUFFIX to installed program names
  --program-transform-name=PROGRAM
                          run sed PROGRAM on installed program names
EOF
    cat << EOF
Host type:
  --build=BUILD           configure for building on BUILD [BUILD=HOST]
  --host=HOST             configure for HOST [guessed]
  --target=TARGET         configure for TARGET [TARGET=HOST]
Features and packages:
  --disable-FEATURE       do not include FEATURE (same as --enable-FEATURE=no)
  --enable-FEATURE[=ARG]  include FEATURE [ARG=yes]
  --with-PACKAGE[=ARG]    use PACKAGE [ARG=yes]
  --without-PACKAGE       do not use PACKAGE (same as --with-PACKAGE=no)
  --x-includes=DIR        X include files are in DIR
  --x-libraries=DIR       X library files are in DIR
EOF
    if test -n "$ac_help"; then
      echo "--enable and --with options recognized:$ac_help"
    fi
    exit 0 ;;

  -host | --host | --hos | --ho)
    ac_prev=host ;;
  -host=* | --host=* | --hos=* | --ho=*)
    host="$ac_optarg" ;;

  -includedir | --includedir | --includedi | --included | --include \
  | --includ | --inclu | --incl | --inc)
    ac_prev=includedir ;;
  -includedir=* | --includedir=* | --includedi=* | --included=* | --include=* \
  | --includ=* | --inclu=* | --incl=* | --inc=*)
    includedir="$ac_optarg" ;;

  -infodir | --infodir | --infodi | --infod | --info | --inf)
    ac_prev=infodir ;;
  -infodir=* | --infodir=* | --infodi=* | --infod=* | --info=* | --inf=*)
    infodir="$ac_optarg" ;;

  -libdir | --libdir | --libdi | --libd)
    ac_prev=libdir ;;
  -libdir=* | --libdir=* | --libdi=* | --libd=*)
    libdir="$ac_optarg" ;;

  -libexecdir | --libexecdir | --libexecdi | --libexecd | --libexec \
  | --libexe | --libex | --libe)
    ac_prev=libexecdir ;;
  -libexecdir=* | --libexecdir=* | --libexecdi=* | --libexecd=* | --libexec=* \
  | --libexe=* | --libex=* | --libe=*)
    libexecdir="$ac_optarg" ;;

  -localstatedir | --localstatedir | --localstatedi | --localstated \
  | --localstate | --localstat | --localsta | --localst \
  | --locals | --local | --loca | --loc | --lo)
    ac_prev=localstatedir ;;
  -localstatedir=* | --localstatedir=* | --localstatedi=* | --localstated=* \
  | --localstate=* | --localstat=* | --localsta=* | --localst=* \
  | --locals=* | --local=* | --loca=* | --loc=* | --lo=*)
    localstatedir="$ac_optarg" ;;

  -mandir | --mandir | --mandi | --mand | --man | --ma | --m)
    ac_prev=mandir ;;
  -mandir=* | --mandir=* | --mandi=* | --mand=* | --man=* | --ma=* | --m=*)
    mandir="$ac_optarg" ;;

  -nfp | --nfp | --nf)
    # Obsolete; use --without-fp.
    with_fp=no ;;

  -no-create | --no-create | --no-creat | --no-crea | --no-cre \
  | --no-cr | --no-c)
    no_create=yes ;;

  -no-recursion | --no-recursion | --no-recursio | --no-recursi \
  | --no-recurs | --no-recur | --no-recu | --no-rec | --no-re | --no-r)
    no_recursion=yes ;;

  -oldincludedir | --oldincludedir | --oldincludedi | --oldincluded \
  | --oldinclude | --oldinclud | --oldinclu | --oldincl | --oldinc \
  | --oldin | --oldi | --old | --ol | --o)
    ac_prev=oldincludedir ;;
  -oldincludedir=* | --oldincludedir=* | --oldincludedi=* | --oldincluded=* \
  | --oldinclude=* | --oldinclud=* | --oldinclu=* | --oldincl=* | --oldinc=* \
  | --oldin=* | --oldi=* | --old=* | --ol=* | --o=*)
    oldincludedir="$ac_optarg" ;;

  -prefix | --prefix | --prefi | --pref | --pre | --pr | --p)
    ac_prev=prefix ;;
  -prefix=* | --prefix=* | --prefi=* | --pref=* | --pre=* | --pr=* | --p=*)
    prefix="$ac_optarg" ;;

  -program-prefix | --program-prefix | --program-prefi | --program-pref \
  | --program-pre | --program-pr | --program-p)
    ac_prev=program_prefix ;;
  -program-prefix=* | --program-prefix=* | --program-prefi=* \
  | --program-pref=* | --program-pre=* | --program-pr=* | --program-p=*)
    program_prefix="$ac_optarg" ;;

  -program-suffix | --program-suffix | --program-suffi | --program-suff \
  | --program-suf | --program-su | --program-s)
    ac_prev=program_suffix ;;
  -program-suffix=* | --program-suffix=* | --program-suffi=* \
  | --program-suff=* | --program-suf=* | --program-su=* | --program-s=*)
    program_suffix="$ac_optarg" ;;

  -program-transform-name | --program-transform-name \
  | --program-transform-nam | --program-transform-na \
  | --program-transform-n | --program-transform- \
  | --program-transform | --program-transfor \
  | --program-transfo | --program-transf \
  | --program-trans | --program-tran \
  | --progr-tra | --program-tr | --program-t)
    ac_prev=program_transform_name ;;
  -program-transform-name=* | --program-transform-name=* \
  | --program-transform-nam=* | --program-transform-na=* \
  | --program-transform-n=* | --program-transform-=* \
  | --program-transform=* | --program-transfor=* \
  | --program-transfo=* | --program-transf=* \
  | --program-trans=* | --program-tran=* \
  | --progr-tra=* | --program-tr=* | --program-t=*)
    program_transform_name="$ac_optarg" ;;

  -q | -quiet | --quiet | --quie | --qui | --qu | --q \
  | -silent | --silent | --silen | --sile | --sil)
    silent=yes ;;

  -sbindir | --sbindir | --sbindi | --sbind | --sbin | --sbi | --sb)
    ac_prev=sbindir ;;
  -sbindir=* | --sbindir=* | --sbindi=* | --sbind=* | --sbin=* \
  | --sbi=* | --sb=*)
    sbindir="$ac_optarg" ;;

  -sharedstatedir | --sharedstatedir | --sharedstatedi \
  | --sharedstated | --sharedstate | --sharedstat | --sharedsta \
  | --sharedst | --shareds | --shared | --share | --shar \
  | --sha | --sh)
    ac_prev=sharedstatedir ;;
  -sharedstatedir=* | --sharedstatedir=* | --sharedstatedi=* \
  | --sharedstated=* | --sharedstate=* | --sharedstat=* | --sharedsta=* \
  | --sharedst=* | --shareds=* | --shared=* | --share=* | --shar=* \
  | --sha=* | --sh=*)
    sharedstatedir="$ac_optarg" ;;

  -site | --site | --sit)
    ac_prev=site ;;
  -site=* | --site=* | --sit=*)
    site="$ac_optarg" ;;

  -srcdir | --srcdir | --srcdi | --srcd | --src | --sr)
    ac_prev=srcdir ;;
  -srcdir=* | --srcdir=* | --srcdi=* | --srcd=* | --src=* | --sr=*)
    srcdir="$ac_optarg" ;;

  -sysconfdir | --sysconfdir | --sysconfdi | --sysconfd | --sysconf \
  | --syscon | --sysco | --sysc | --sys | --sy)
    ac_prev=sysconfdir ;;
  -sysconfdir=* | --sysconfdir=* | --sysconfdi=* | --sysconfd=* | --sysconf=* \
  | --syscon=* | --sysco=* | --sysc=* | --sys=* | --sy=*)
    sysconfdir="$ac_optarg" ;;

  -target | --target | --targe | --targ | --tar | --ta | --t)
    ac_prev=target ;;
  -target=* | --target=* | --targe=* | --targ=* | --tar=* | --ta=* | --t=*)
    target="$ac_optarg" ;;

  -v | -verbose | --verbose | --verbos | --verbo | --verb)
    verbose=yes ;;

  -version | --version | --versio | --versi | --vers)
    echo "configure generated by autoconf version 2.13"
    exit 0 ;;

  -with-* | --with-*)
    ac_package=`echo $ac_option|sed -e 's/-*with-//' -e 's/=.*//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_package| sed 's/[-_a-zA-Z0-9]//g'`"; then
      { echo "configure: error: $ac_package: invalid package name" 1>&2; exit 1; }
    fi
    ac_package=`echo $ac_package| sed 's/-/_/g'`
    case "$ac_option" in
      *=*) ;;
      *) ac_optarg=yes ;;
    esac
    eval "with_${ac_package}='$ac_optarg'" ;;

  -without-* | --without-*)
    ac_package=`echo $ac_option|sed -e 's/-*without-//'`
    # Reject names that are not valid shell variable names.
    if test -n "`echo $ac_package| sed 's/[-a-zA-Z0-9_]//g'`"; then
      { echo "configure: error: $ac_package: invalid package name" 1>&2; exit 1; }
    fi
    ac_package=`echo $ac_package| sed 's/-/_/g'`
    eval "with_${ac_package}=no" ;;

  --x)
    # Obsolete; use --with-x.
    with_x=yes ;;

  -x-includes | --x-includes | --x-include | --x-includ | --x-inclu \
  | --x-incl | --x-inc | --x-in | --x-i)
    ac_prev=x_includes ;;
  -x-includes=* | --x-includes=* | --x-include=* | --x-includ=* | --x-inclu=* \
  | --x-incl=* | --x-inc=* | --x-in=* | --x-i=*)
    x_includes="$ac_optarg" ;;

  -x-libraries | --x-libraries | --x-librarie | --x-librari \
  | --x-librar | --x-libra | --x-libr | --x-lib | --x-li | --x-l)
    ac_prev=x_libraries ;;
  -x-libraries=* | --x-libraries=* | --x-librarie=* | --x-librari=* \
  | --x-librar=* | --x-libra=* | --x-libr=* | --x-lib=* | --x-li=* | --x-l=*)
    x_libraries="$ac_optarg" ;;

  -*) { echo "configure: error: $ac_option: invalid option; use --help to show usage" 1>&2; exit 1; }
    ;;

  *)
    if test -n "`echo $ac_option| sed 's/[-a-z0-9.]//g'`"; then
      echo "configure: warning: $ac_option: invalid host type" 1>&2
    fi
    if test "x$nonopt" != xNONE; then
      { echo "configure: error: can only configure for one host and one target at a time" 1>&2; exit 1; }
    fi
    nonopt="$ac_option"
    ;;

  esac
done

if test -n "$ac_prev"; then
  { echo "configure: error: missing argument to --`echo $ac_prev | sed 's/_/-/g'`" 1>&2; exit 1; }
fi

trap 'rm -fr conftest* confdefs* core core.* *.core $ac_clean_files; exit 1' 1 2 15

# File descriptor usage:
# 0 standard input
# 1 file creation
# 2 errors and warnings
# 3 some systems may open it to /dev/tty
# 4 used on the Kubota Titan
# 6 checking for... messages and results
# 5 compiler messages saved in config.log
if test "$silent" = yes; then
  exec 6>/dev/null
else
  exec 6>&1
fi
exec 5>./config.log

echo "\
This file contains any messages produced by compilers while
running configure, to aid debugging if configure makes a mistake.
" 1>&5

# Strip out --no-create and --no-recursion so they do not pile up.
# Also quote any args containing shell metacharacters.
ac_configure_args=
for ac_arg
do
  case "$ac_arg" in
  -no-create | --no-create | --no-creat | --no-crea | --no-cre \
  | --no-cr | --no-c) ;;
  -no-recursion | --no-recursion | --no-recursio | --no-recursi \
  | --no-recurs | --no-recur | --no-recu | --no-rec | --no-re | --no-r) ;;
  *" "*|*"	"*|*[\[\]\~\#\$\^\&\*\(\)\{\}\\\|\;\<\>\?]*)
  ac_configure_args="$ac_configure_args '$ac_arg'" ;;
  *) ac_configure_args="$ac_configure_args $ac_arg" ;;
  esac
done

# NLS nuisances.
# Only set these to C if already set.  These must not be set unconditionally
# because not all systems understand e.g. LANG=C (notably SCO).
# Fixing LC_MESSAGES prevents Solaris sh from translating var values in `set'!
# Non-C LC_CTYPE values break the ctype check.
if test "${LANG+set}"   = set; then LANG=C;   export LANG;   fi
if test "${LC_ALL+set}" = set; then LC_ALL=C; export LC_ALL; fi
if test "${LC_MESSAGES+set}" = set; then LC_MESSAGES=C; export LC_MESSAGES; fi
if test "${LC_CTYPE+set}"    = set; then LC_CTYPE=C;    export LC_CTYPE;    fi

# confdefs.h avoids OS command line length limits that DEFS can exceed.
rm -rf conftest* confdefs.h
# AIX cpp loses on an empty file, so make sure it contains at least a newline.
echo > confdefs.h

# A filename unique to this package, relative to the directory that
# configure is in, which we can look for to find out if srcdir is correct.
ac_unique_file=../generic/tcl.h

# Find the source files, if location was not specified.
if test -z "$srcdir"; then
  ac_srcdir_defaulted=yes
  # Try the directory containing this script, then its parent.
  ac_prog=$0
  ac_confdir=`echo $ac_prog|sed 's%/[^/][^/]*$%%'`
  test "x$ac_confdir" = "x$ac_prog" && ac_confdir=.
  srcdir=$ac_confdir
  if test ! -r $srcdir/$ac_unique_file; then
    srcdir=..
  fi
else
  ac_srcdir_defaulted=no
fi
if test ! -r $srcdir/$ac_unique_file; then
  if test "$ac_srcdir_defaulted" = yes; then
    { echo "configure: error: can not find sources in $ac_confdir or .." 1>&2; exit 1; }
  else
    { echo "configure: error: can not find sources in $srcdir" 1>&2; exit 1; }
  fi
fi
srcdir=`echo "${srcdir}" | sed 's%\([^/]\)/*$%\1%'`

# Prefer explicitly selected file to automatically selected ones.
if test -z "$CONFIG_SITE"; then
  if test "x$prefix" != xNONE; then
    CONFIG_SITE="$prefix/share/config.site $prefix/etc/config.site"
  else
    CONFIG_SITE="$ac_default_prefix/share/config.site $ac_default_prefix/etc/config.site"
  fi
fi
for ac_site_file in $CONFIG_SITE; do
  if test -r "$ac_site_file"; then
    echo "loading site script $ac_site_file"
    . "$ac_site_file"
  fi
done

if test -r "$cache_file"; then
  echo "loading cache $cache_file"
  . $cache_file
else
  echo "creating cache $cache_file"
  > $cache_file
fi

ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$CPP $CPPFLAGS'
ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS conftest.$ac_ext 1>&5'
ac_link='${CC-cc} -o conftest${ac_exeext} $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&5'
cross_compiling=$ac_cv_prog_cc_cross

ac_exeext=
ac_objext=o
if (echo "testing\c"; echo 1,2,3) | grep c >/dev/null; then
  # Stardent Vistra SVR4 grep lacks -e, says ghazi@caip.rutgers.edu.
  if (echo -n testing; echo 1,2,3) | sed s/-n/xn/ | grep xn >/dev/null; then
    ac_n= ac_c='
' ac_t='	'
  else
    ac_n=-n ac_c= ac_t=
  fi
else
  ac_n= ac_c='\c' ac_t=
fi




TCL_VERSION=8.4
TCL_MAJOR_VERSION=8
TCL_MINOR_VERSION=4
TCL_PATCH_LEVEL=".16"
VER=$TCL_MAJOR_VERSION$TCL_MINOR_VERSION

TCL_DDE_VERSION=1.2
TCL_DDE_MAJOR_VERSION=1
TCL_DDE_MINOR_VERSION=2
DDEVER=$TCL_DDE_MAJOR_VERSION$TCL_DDE_MINOR_VERSION

TCL_REG_VERSION=1.1
TCL_REG_MAJOR_VERSION=1
TCL_REG_MINOR_VERSION=1
REGVER=$TCL_REG_MAJOR_VERSION$TCL_REG_MINOR_VERSION

#------------------------------------------------------------------------
# Handle the --prefix=... option
#------------------------------------------------------------------------

if test "${prefix}" = "NONE"; then
    prefix=/usr/local
fi
if test "${exec_prefix}" = "NONE"; then
    exec_prefix=$prefix
fi
# libdir must be a fully qualified path (not ${exec_prefix}/lib)
eval libdir="$libdir"

#------------------------------------------------------------------------
# Standard compiler checks
#------------------------------------------------------------------------

# If the user did not set CFLAGS, set it now to keep
# the AC_PROG_CC macro from adding "-g -O2".
if test "${CFLAGS+set}" != "set" ; then
    CFLAGS=""
fi

# Extract the first word of "gcc", so it can be a program name with args.
set dummy gcc; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:577: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_CC'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$CC"; then
  ac_cv_prog_CC="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_CC="gcc"
      break
    fi
  done
  IFS="$ac_save_ifs"
fi
fi
CC="$ac_cv_prog_CC"
if test -n "$CC"; then
  echo "$ac_t""$CC" 1>&6
else
  echo "$ac_t""no" 1>&6
fi

if test -z "$CC"; then
  # Extract the first word of "cc", so it can be a program name with args.
set dummy cc; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:607: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_CC'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$CC"; then
  ac_cv_prog_CC="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_prog_rejected=no
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      if test "$ac_dir/$ac_word" = "/usr/ucb/cc"; then
        ac_prog_rejected=yes
	continue
      fi
      ac_cv_prog_CC="cc"
      break
    fi
  done
  IFS="$ac_save_ifs"
if test $ac_prog_rejected = yes; then
  # We found a bogon in the path, so make sure we never use it.
  set dummy $ac_cv_prog_CC
  shift
  if test $# -gt 0; then
    # We chose a different compiler from the bogus one.
    # However, it has the same basename, so the bogon will be chosen
    # first if we set CC to just the basename; use the full file name.
    shift
    set dummy "$ac_dir/$ac_word" "$@"
    shift
    ac_cv_prog_CC="$@"
  fi
fi
fi
fi
CC="$ac_cv_prog_CC"
if test -n "$CC"; then
  echo "$ac_t""$CC" 1>&6
else
  echo "$ac_t""no" 1>&6
fi

  if test -z "$CC"; then
    case "`uname -s`" in
    *win32* | *WIN32*)
      # Extract the first word of "cl", so it can be a program name with args.
set dummy cl; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:658: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_CC'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$CC"; then
  ac_cv_prog_CC="$CC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_CC="cl"
      break
    fi
  done
  IFS="$ac_save_ifs"
fi
fi
CC="$ac_cv_prog_CC"
if test -n "$CC"; then
  echo "$ac_t""$CC" 1>&6
else
  echo "$ac_t""no" 1>&6
fi
 ;;
    esac
  fi
  test -z "$CC" && { echo "configure: error: no acceptable cc found in \$PATH" 1>&2; exit 1; }
fi

echo $ac_n "checking whether the C compiler ($CC $CFLAGS $LDFLAGS) works""... $ac_c" 1>&6
echo "configure:690: checking whether the C compiler ($CC $CFLAGS $LDFLAGS) works" >&5

ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$CPP $CPPFLAGS'
ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS conftest.$ac_ext 1>&5'
ac_link='${CC-cc} -o conftest${ac_exeext} $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&5'
cross_compiling=$ac_cv_prog_cc_cross

cat > conftest.$ac_ext << EOF

#line 701 "configure"
#include "confdefs.h"

main(){return(0);}
EOF
if { (eval echo configure:706: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext}; then
  ac_cv_prog_cc_works=yes
  # If we can't run a trivial program, we are probably using a cross compiler.
  if (./conftest; exit) 2>/dev/null; then
    ac_cv_prog_cc_cross=no
  else
    ac_cv_prog_cc_cross=yes
  fi
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  ac_cv_prog_cc_works=no
fi
rm -fr conftest*
ac_ext=c
# CFLAGS is not in ac_cpp because -g, -O, etc. are not valid cpp options.
ac_cpp='$CPP $CPPFLAGS'
ac_compile='${CC-cc} -c $CFLAGS $CPPFLAGS conftest.$ac_ext 1>&5'
ac_link='${CC-cc} -o conftest${ac_exeext} $CFLAGS $CPPFLAGS $LDFLAGS conftest.$ac_ext $LIBS 1>&5'
cross_compiling=$ac_cv_prog_cc_cross

echo "$ac_t""$ac_cv_prog_cc_works" 1>&6
if test $ac_cv_prog_cc_works = no; then
  { echo "configure: error: installation or configuration problem: C compiler cannot create executables." 1>&2; exit 1; }
fi
echo $ac_n "checking whether the C compiler ($CC $CFLAGS $LDFLAGS) is a cross-compiler""... $ac_c" 1>&6
echo "configure:732: checking whether the C compiler ($CC $CFLAGS $LDFLAGS) is a cross-compiler" >&5
echo "$ac_t""$ac_cv_prog_cc_cross" 1>&6
cross_compiling=$ac_cv_prog_cc_cross

echo $ac_n "checking whether we are using GNU C""... $ac_c" 1>&6
echo "configure:737: checking whether we are using GNU C" >&5
if eval "test \"`echo '$''{'ac_cv_prog_gcc'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.c <<EOF
#ifdef __GNUC__
  yes;
#endif
EOF
if { ac_try='${CC-cc} -E conftest.c'; { (eval echo configure:746: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }; } | egrep yes >/dev/null 2>&1; then
  ac_cv_prog_gcc=yes
else
  ac_cv_prog_gcc=no
fi
fi

echo "$ac_t""$ac_cv_prog_gcc" 1>&6

if test $ac_cv_prog_gcc = yes; then
  GCC=yes
else
  GCC=
fi

ac_test_CFLAGS="${CFLAGS+set}"
ac_save_CFLAGS="$CFLAGS"
CFLAGS=
echo $ac_n "checking whether ${CC-cc} accepts -g""... $ac_c" 1>&6
echo "configure:765: checking whether ${CC-cc} accepts -g" >&5
if eval "test \"`echo '$''{'ac_cv_prog_cc_g'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  echo 'void f(){}' > conftest.c
if test -z "`${CC-cc} -g -c conftest.c 2>&1`"; then
  ac_cv_prog_cc_g=yes
else
  ac_cv_prog_cc_g=no
fi
rm -f conftest*

fi

echo "$ac_t""$ac_cv_prog_cc_g" 1>&6
if test "$ac_test_CFLAGS" = set; then
  CFLAGS="$ac_save_CFLAGS"
elif test $ac_cv_prog_cc_g = yes; then
  if test "$GCC" = yes; then
    CFLAGS="-g -O2"
  else
    CFLAGS="-g"
  fi
else
  if test "$GCC" = yes; then
    CFLAGS="-O2"
  else
    CFLAGS=
  fi
fi


# To properly support cross-compilation, one would
# need to use these tool checks instead of
# the ones below and reconfigure with
# autoconf 2.50. You can also just set
# the CC, AR, RANLIB, and RC environment
# variables if you want to cross compile.

if test "${GCC}" = "yes" ; then
    # Extract the first word of "ar", so it can be a program name with args.
set dummy ar; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:808: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_AR'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$AR"; then
  ac_cv_prog_AR="$AR" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_AR="ar"
      break
    fi
  done
  IFS="$ac_save_ifs"
fi
fi
AR="$ac_cv_prog_AR"
if test -n "$AR"; then
  echo "$ac_t""$AR" 1>&6
else
  echo "$ac_t""no" 1>&6
fi

    # Extract the first word of "ranlib", so it can be a program name with args.
set dummy ranlib; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:837: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_RANLIB'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$RANLIB"; then
  ac_cv_prog_RANLIB="$RANLIB" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_RANLIB="ranlib"
      break
    fi
  done
  IFS="$ac_save_ifs"
fi
fi
RANLIB="$ac_cv_prog_RANLIB"
if test -n "$RANLIB"; then
  echo "$ac_t""$RANLIB" 1>&6
else
  echo "$ac_t""no" 1>&6
fi

    # Extract the first word of "windres", so it can be a program name with args.
set dummy windres; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:866: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_RC'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$RC"; then
  ac_cv_prog_RC="$RC" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_RC="windres"
      break
    fi
  done
  IFS="$ac_save_ifs"
fi
fi
RC="$ac_cv_prog_RC"
if test -n "$RC"; then
  echo "$ac_t""$RC" 1>&6
else
  echo "$ac_t""no" 1>&6
fi


    if test "${AR}" = "" ; then
	{ echo "configure: error: Required archive tool 'ar' not found on PATH." 1>&2; exit 1; }
    fi
    if test "${RANLIB}" = "" ; then
	{ echo "configure: error: Required archive index tool 'ranlib' not found on PATH." 1>&2; exit 1; }
    fi
    if test "${RC}" = "" ; then
	{ echo "configure: error: Required resource tool 'windres' not found on PATH." 1>&2; exit 1; }
    fi
fi

#--------------------------------------------------------------------
# Checks to see if the make progeam sets the $MAKE variable.
#--------------------------------------------------------------------

echo $ac_n "checking whether ${MAKE-make} sets \${MAKE}""... $ac_c" 1>&6
echo "configure:909: checking whether ${MAKE-make} sets \${MAKE}" >&5
set dummy ${MAKE-make}; ac_make=`echo "$2" | sed 'y%./+-%__p_%'`
if eval "test \"`echo '$''{'ac_cv_prog_make_${ac_make}_set'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftestmake <<\EOF
all:
	@echo 'ac_maketemp="${MAKE}"'
EOF
# GNU make sometimes prints "make[1]: Entering...", which would confuse us.
eval `${MAKE-make} -f conftestmake 2>/dev/null | grep temp=`
if test -n "$ac_maketemp"; then
  eval ac_cv_prog_make_${ac_make}_set=yes
else
  eval ac_cv_prog_make_${ac_make}_set=no
fi
rm -f conftestmake
fi
if eval "test \"`echo '$ac_cv_prog_make_'${ac_make}_set`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  SET_MAKE=
else
  echo "$ac_t""no" 1>&6
  SET_MAKE="MAKE=${MAKE-make}"
fi


#--------------------------------------------------------------------
# Perform additinal compiler tests.
#--------------------------------------------------------------------

echo $ac_n "checking for Cygwin environment""... $ac_c" 1>&6
echo "configure:941: checking for Cygwin environment" >&5
if eval "test \"`echo '$''{'ac_cv_cygwin'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 946 "configure"
#include "confdefs.h"

int main() {

#ifndef __CYGWIN__
#define __CYGWIN__ __CYGWIN32__
#endif
return __CYGWIN__;
; return 0; }
EOF
if { (eval echo configure:957: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  ac_cv_cygwin=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  ac_cv_cygwin=no
fi
rm -f conftest*
rm -f conftest*
fi

echo "$ac_t""$ac_cv_cygwin" 1>&6
CYGWIN=
test "$ac_cv_cygwin" = yes && CYGWIN=yes

if test "$ac_cv_cygwin" = "yes" ; then
    { echo "configure: error: Compiling under Cygwin is not currently supported.
A maintainer for the Cygwin port of Tcl/Tk is needed. See the README
file for information about building with Mingw." 1>&2; exit 1; }
fi


echo $ac_n "checking for SEH support in compiler""... $ac_c" 1>&6
echo "configure:982: checking for SEH support in compiler" >&5
if eval "test \"`echo '$''{'tcl_cv_seh'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$cross_compiling" = yes; then
  tcl_cv_seh=no
else
  cat > conftest.$ac_ext <<EOF
#line 990 "configure"
#include "confdefs.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

int main(int argc, char** argv) {
    int a, b = 0;
    __try {
        a = 666 / b;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
    return 1;
}

EOF
if { (eval echo configure:1009: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; } && test -s conftest${ac_exeext} && (./conftest; exit) 2>/dev/null
then
  tcl_cv_seh=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -fr conftest*
  tcl_cv_seh=no
fi
rm -fr conftest*
fi


fi

echo "$ac_t""$tcl_cv_seh" 1>&6
if test "$tcl_cv_seh" = "no" ; then
    cat >> confdefs.h <<\EOF
#define HAVE_NO_SEH 1
EOF

fi

#
# Check to see if the excpt.h include file provided contains the
# definition for EXCEPTION_DISPOSITION; if not, which is the case
# with Cygwin's version as of 2002-04-10, define it to be int, 
# sufficient for getting the current code to work.
#
echo $ac_n "checking for EXCEPTION_DISPOSITION support in include files""... $ac_c" 1>&6
echo "configure:1039: checking for EXCEPTION_DISPOSITION support in include files" >&5
if eval "test \"`echo '$''{'tcl_cv_eh_disposition'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1044 "configure"
#include "confdefs.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

int main() {

  EXCEPTION_DISPOSITION x;

; return 0; }
EOF
if { (eval echo configure:1057: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  tcl_cv_eh_disposition=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  tcl_cv_eh_disposition=no
fi
rm -f conftest*

fi

echo "$ac_t""$tcl_cv_eh_disposition" 1>&6
if test "$tcl_cv_eh_disposition" = "no" ; then
    cat >> confdefs.h <<\EOF
#define EXCEPTION_DISPOSITION int
EOF

fi


# Check to see if the winsock2.h include file provided contains
# typedefs like LPFN_ACCEPT and friends.
#
echo $ac_n "checking for LPFN_ACCEPT support in winsock2.h""... $ac_c" 1>&6
echo "configure:1083: checking for LPFN_ACCEPT support in winsock2.h" >&5
if eval "test \"`echo '$''{'tcl_cv_lpfn_decls'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1088 "configure"
#include "confdefs.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <winsock2.h>

int main() {

  LPFN_ACCEPT accept;

; return 0; }
EOF
if { (eval echo configure:1102: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  tcl_cv_lpfn_decls=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  tcl_cv_lpfn_decls=no
fi
rm -f conftest*

fi

echo "$ac_t""$tcl_cv_lpfn_decls" 1>&6
if test "$tcl_cv_lpfn_decls" = "no" ; then
    cat >> confdefs.h <<\EOF
#define HAVE_NO_LPFN_DECLS 1
EOF

fi

# Check to see if winnt.h defines CHAR, SHORT, and LONG
# even if VOID has already been #defined. The win32api
# used by mingw and cygwin is known to do this.

echo $ac_n "checking for winnt.h that ignores VOID define""... $ac_c" 1>&6
echo "configure:1128: checking for winnt.h that ignores VOID define" >&5
if eval "test \"`echo '$''{'tcl_cv_winnt_ignore_void'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1133 "configure"
#include "confdefs.h"

#define VOID void
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

int main() {

  CHAR c;
  SHORT s;
  LONG l;

; return 0; }
EOF
if { (eval echo configure:1149: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  tcl_cv_winnt_ignore_void=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  tcl_cv_winnt_ignore_void=no
fi
rm -f conftest*

fi

echo "$ac_t""$tcl_cv_winnt_ignore_void" 1>&6
if test "$tcl_cv_winnt_ignore_void" = "yes" ; then
    cat >> confdefs.h <<\EOF
#define HAVE_WINNT_IGNORE_VOID 1
EOF

fi

# Check to see if malloc.h is missing the alloca function
# declaration. This is known to be a problem with Mingw.
# If we compiled without the function declaration, it
# would work but we would get a warning message from gcc.
# If we add the function declaration ourselves, it
# would not compile correctly because the _alloca
# function expects the argument to be passed in a
# register and not on the stack. Instead, we just
# call it from inline asm code.

echo $ac_n "checking for alloca declaration in malloc.h""... $ac_c" 1>&6
echo "configure:1181: checking for alloca declaration in malloc.h" >&5
if eval "test \"`echo '$''{'tcl_cv_malloc_decl_alloca'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1186 "configure"
#include "confdefs.h"

#include <malloc.h>

int main() {

  size_t arg = 0;
  void* ptr;
  ptr = alloca;
  ptr = alloca(arg);

; return 0; }
EOF
if { (eval echo configure:1200: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  tcl_cv_malloc_decl_alloca=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  tcl_cv_malloc_decl_alloca=no
fi
rm -f conftest*

fi

echo "$ac_t""$tcl_cv_malloc_decl_alloca" 1>&6
if test "$tcl_cv_malloc_decl_alloca" = "no" &&
   test "${GCC}" = "yes" ; then
    cat >> confdefs.h <<\EOF
#define HAVE_ALLOCA_GCC_INLINE 1
EOF

fi

# See if the compiler supports casting to a union type.
# This is used to stop gcc from printing a compiler
# warning when initializing a union member.

echo $ac_n "checking for cast to union support""... $ac_c" 1>&6
echo "configure:1227: checking for cast to union support" >&5
if eval "test \"`echo '$''{'tcl_cv_cast_to_union'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1232 "configure"
#include "confdefs.h"

int main() {

  union foo { int i; double d; };
  union foo f = (union foo) (int) 0;

; return 0; }
EOF
if { (eval echo configure:1242: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  tcl_cv_cast_to_union=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  tcl_cv_cast_to_union=no
fi
rm -f conftest*

fi

echo "$ac_t""$tcl_cv_cast_to_union" 1>&6
if test "$tcl_cv_cast_to_union" = "yes"; then
    cat >> confdefs.h <<\EOF
#define HAVE_CAST_TO_UNION 1
EOF

fi


#--------------------------------------------------------------------
# Determines the correct binary file extension (.o, .obj, .exe etc.)
#--------------------------------------------------------------------

echo $ac_n "checking for object suffix""... $ac_c" 1>&6
echo "configure:1269: checking for object suffix" >&5
if eval "test \"`echo '$''{'ac_cv_objext'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  rm -f conftest*
echo 'int i = 1;' > conftest.$ac_ext
if { (eval echo configure:1275: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  for ac_file in conftest.*; do
    case $ac_file in
    *.c) ;;
    *) ac_cv_objext=`echo $ac_file | sed -e s/conftest.//` ;;
    esac
  done
else
  { echo "configure: error: installation or configuration problem; compiler does not work" 1>&2; exit 1; }
fi
rm -f conftest*
fi

echo "$ac_t""$ac_cv_objext" 1>&6
OBJEXT=$ac_cv_objext
ac_objext=$ac_cv_objext

echo $ac_n "checking for mingw32 environment""... $ac_c" 1>&6
echo "configure:1293: checking for mingw32 environment" >&5
if eval "test \"`echo '$''{'ac_cv_mingw32'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1298 "configure"
#include "confdefs.h"

int main() {
return __MINGW32__;
; return 0; }
EOF
if { (eval echo configure:1305: \"$ac_compile\") 1>&5; (eval $ac_compile) 2>&5; }; then
  rm -rf conftest*
  ac_cv_mingw32=yes
else
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  ac_cv_mingw32=no
fi
rm -f conftest*
rm -f conftest*
fi

echo "$ac_t""$ac_cv_mingw32" 1>&6
MINGW32=
test "$ac_cv_mingw32" = yes && MINGW32=yes


echo $ac_n "checking for executable suffix""... $ac_c" 1>&6
echo "configure:1324: checking for executable suffix" >&5
if eval "test \"`echo '$''{'ac_cv_exeext'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test "$CYGWIN" = yes || test "$MINGW32" = yes; then
  ac_cv_exeext=.exe
else
  rm -f conftest*
  echo 'int main () { return 0; }' > conftest.$ac_ext
  ac_cv_exeext=
  if { (eval echo configure:1334: \"$ac_link\") 1>&5; (eval $ac_link) 2>&5; }; then
    for file in conftest.*; do
      case $file in
      *.c | *.o | *.obj) ;;
      *) ac_cv_exeext=`echo $file | sed -e s/conftest//` ;;
      esac
    done
  else
    { echo "configure: error: installation or configuration problem: compiler cannot create executables." 1>&2; exit 1; }
  fi
  rm -f conftest*
  test x"${ac_cv_exeext}" = x && ac_cv_exeext=no
fi
fi

EXEEXT=""
test x"${ac_cv_exeext}" != xno && EXEEXT=${ac_cv_exeext}
echo "$ac_t""${ac_cv_exeext}" 1>&6
ac_exeext=$EXEEXT


#--------------------------------------------------------------------
# Check whether --enable-threads or --disable-threads was given.
#--------------------------------------------------------------------


    echo $ac_n "checking for building with threads""... $ac_c" 1>&6
echo "configure:1361: checking for building with threads" >&5
    # Check whether --enable-threads or --disable-threads was given.
if test "${enable_threads+set}" = set; then
  enableval="$enable_threads"
  tcl_ok=$enableval
else
  tcl_ok=no
fi


    if test "$tcl_ok" = "yes"; then
	echo "$ac_t""yes" 1>&6
	TCL_THREADS=1
	cat >> confdefs.h <<\EOF
#define TCL_THREADS 1
EOF

	# USE_THREAD_ALLOC tells us to try the special thread-based
	# allocator that significantly reduces lock contention
	cat >> confdefs.h <<\EOF
#define USE_THREAD_ALLOC 1
EOF

    else
	TCL_THREADS=0
	echo "$ac_t""no (default)" 1>&6
    fi
    


#--------------------------------------------------------------------
# The statements below define a collection of symbols related to
# building libtcl as a shared library instead of a static library.
#--------------------------------------------------------------------


    echo $ac_n "checking how to build libraries""... $ac_c" 1>&6
echo "configure:1398: checking how to build libraries" >&5
    # Check whether --enable-shared or --disable-shared was given.
if test "${enable_shared+set}" = set; then
  enableval="$enable_shared"
  tcl_ok=$enableval
else
  tcl_ok=yes
fi


    if test "${enable_shared+set}" = set; then
	enableval="$enable_shared"
	tcl_ok=$enableval
    else
	tcl_ok=yes
    fi

    if test "$tcl_ok" = "yes" ; then
	echo "$ac_t""shared" 1>&6
	SHARED_BUILD=1
    else
	echo "$ac_t""static" 1>&6
	SHARED_BUILD=0
	cat >> confdefs.h <<\EOF
#define STATIC_BUILD 1
EOF

    fi


#--------------------------------------------------------------------
# The statements below define a collection of compile flags.  This 
# macro depends on the value of SHARED_BUILD, and should be called
# after SC_ENABLE_SHARED checks the configure switches.
#--------------------------------------------------------------------



    # Step 0: Enable 64 bit support?

    echo $ac_n "checking if 64bit support is requested""... $ac_c" 1>&6
echo "configure:1439: checking if 64bit support is requested" >&5
    # Check whether --enable-64bit or --disable-64bit was given.
if test "${enable_64bit+set}" = set; then
  enableval="$enable_64bit"
  do64bit=$enableval
else
  do64bit=no
fi

    echo "$ac_t""$do64bit" 1>&6

    # Set some defaults (may get changed below)
    EXTRA_CFLAGS=""

    # Extract the first word of "cygpath", so it can be a program name with args.
set dummy cygpath; ac_word=$2
echo $ac_n "checking for $ac_word""... $ac_c" 1>&6
echo "configure:1456: checking for $ac_word" >&5
if eval "test \"`echo '$''{'ac_cv_prog_CYGPATH'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  if test -n "$CYGPATH"; then
  ac_cv_prog_CYGPATH="$CYGPATH" # Let the user override the test.
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS=":"
  ac_dummy="$PATH"
  for ac_dir in $ac_dummy; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/$ac_word; then
      ac_cv_prog_CYGPATH="cygpath -w"
      break
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_prog_CYGPATH" && ac_cv_prog_CYGPATH="echo"
fi
fi
CYGPATH="$ac_cv_prog_CYGPATH"
if test -n "$CYGPATH"; then
  echo "$ac_t""$CYGPATH" 1>&6
else
  echo "$ac_t""no" 1>&6
fi


    SHLIB_SUFFIX=".dll"

    # MACHINE is IX86 for LINK, but this is used by the manifest,
    # which requires x86|amd64|ia64.
    MACHINE="X86"

    # Check for a bug in gcc's windres that causes the
    # compile to fail when a Windows native path is
    # passed into windres. The mingw toolchain requires
    # Windows native paths while Cygwin should work
    # with both. Avoid the bug by passing a POSIX
    # path when using the Cygwin toolchain.

    if test "$GCC" = "yes" && test "$CYGPATH" != "echo" ; then
	conftest=/tmp/conftest.rc
	echo "STRINGTABLE BEGIN" > $conftest
	echo "101 \"name\"" >> $conftest
	echo "END" >> $conftest

	echo $ac_n "checking for Windows native path bug in windres""... $ac_c" 1>&6
echo "configure:1504: checking for Windows native path bug in windres" >&5
	cyg_conftest=`$CYGPATH $conftest`
	if { ac_try='$RC -o conftest.res.o $cyg_conftest'; { (eval echo configure:1506: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }; } ; then
	    echo "$ac_t""no" 1>&6
	else
	    echo "$ac_t""yes" 1>&6
	    CYGPATH=echo
	fi
	conftest=
	cyg_conftest=
    fi

    if test "$CYGPATH" = "echo" || test "$ac_cv_cygwin" = "yes"; then
        DEPARG='"$<"'
    else
        DEPARG='"$(shell $(CYGPATH) $<)"'
    fi

    # set various compiler flags depending on whether we are using gcc or cl

    echo $ac_n "checking compiler flags""... $ac_c" 1>&6
echo "configure:1525: checking compiler flags" >&5
    if test "${GCC}" = "yes" ; then
	if test "$do64bit" != "no" ; then
	    echo "configure: warning: "64bit mode not supported with GCC on Windows"" 1>&2
	fi
	SHLIB_LD=""
	SHLIB_LD_LIBS=""
	LIBS=""
	LIBS_GUI="-lgdi32 -lcomdlg32 -limm32 -lcomctl32 -lshell32"
	STLIB_LD='${AR} cr'
	RC_OUT=-o
	RC_TYPE=
	RC_INCLUDE=--include
	RC_DEFINE=--define
	RES=res.o
	MAKE_LIB="\${STLIB_LD} \$@"
	POST_MAKE_LIB="\${RANLIB} \$@"
	MAKE_EXE="\${CC} -o \$@"
	LIBPREFIX="lib"

	#if test "$ac_cv_cygwin" = "yes"; then
	#    extra_cflags="-mno-cygwin"
	#    extra_ldflags="-mno-cygwin"
	#else
	#    extra_cflags=""
	#    extra_ldflags=""
	#fi

	if test "$ac_cv_cygwin" = "yes"; then
	  touch ac$$.c
	  if ${CC} -c -mwin32 ac$$.c >/dev/null 2>&1; then
	    case "$extra_cflags" in
	      *-mwin32*) ;;
	      *) extra_cflags="-mwin32 $extra_cflags" ;;
	    esac
	    case "$extra_ldflags" in
	      *-mwin32*) ;;
	      *) extra_ldflags="-mwin32 $extra_ldflags" ;;
	    esac
	  fi
	  rm -f ac$$.o ac$$.c
	else
	  extra_cflags=''
	  extra_ldflags=''
	fi

	if test "${SHARED_BUILD}" = "0" ; then
	    # static
            echo "$ac_t""using static flags" 1>&6
	    runtime=
	    MAKE_DLL="echo "
	    LIBSUFFIX="s\${DBGX}.a"
	    LIBFLAGSUFFIX="s\${DBGX}"
	    LIBRARIES="\${STATIC_LIBRARIES}"
	    EXESUFFIX="s\${DBGX}.exe"
	else
	    # dynamic
            echo "$ac_t""using shared flags" 1>&6

	    # ad-hoc check to see if CC supports -shared.
	    if "${CC}" -shared 2>&1 | egrep ': -shared not supported' >/dev/null; then
		{ echo "configure: error: ${CC} does not support the -shared option.
                You will need to upgrade to a newer version of the toolchain." 1>&2; exit 1; }
	    fi

	    runtime=
	    # Link with gcc since ld does not link to default libs like
	    # -luser32 and -lmsvcrt by default. Make sure CFLAGS is
	    # included so -mno-cygwin passed the correct libs to the linker.
	    SHLIB_LD='${CC} -shared ${CFLAGS}'
	    SHLIB_LD_LIBS='${LIBS}'
	    # Add SHLIB_LD_LIBS to the Make rule, not here.
	    MAKE_DLL="\${SHLIB_LD} \$(LDFLAGS) -o \$@ ${extra_ldflags} \
	        -Wl,--out-implib,\$(patsubst %.dll,lib%.a,\$@)"

	    LIBSUFFIX="\${DBGX}.a"
	    LIBFLAGSUFFIX="\${DBGX}"
	    EXESUFFIX="\${DBGX}.exe"
	    LIBRARIES="\${SHARED_LIBRARIES}"
	fi
	# DLLSUFFIX is separate because it is the building block for
	# users of tclConfig.sh that may build shared or static.
	DLLSUFFIX="\${DBGX}.dll"
	SHLIB_SUFFIX=.dll

	EXTRA_CFLAGS="${extra_cflags}"

	CFLAGS_DEBUG=-g
	CFLAGS_OPTIMIZE="-O2 -fomit-frame-pointer"
	CFLAGS_WARNING="-Wall -Wconversion"
	LDFLAGS_DEBUG=
	LDFLAGS_OPTIMIZE=

	# Specify the CC output file names based on the target name
	CC_OBJNAME="-o \$@"
	CC_EXENAME="-o \$@"

	# Specify linker flags depending on the type of app being 
	# built -- Console vs. Window.
	#
	# ORIGINAL COMMENT:
	# We need to pass -e _WinMain@16 so that ld will use
	# WinMain() instead of main() as the entry point. We can't
	# use autoconf to check for this case since it would need
	# to run an executable and that does not work when
	# cross compiling. Remove this -e workaround once we
	# require a gcc that does not have this bug.
	#
	# MK NOTE: Tk should use a different mechanism. This causes 
	# interesting problems, such as wish dying at startup.
	#LDFLAGS_WINDOW="-mwindows -e _WinMain@16 ${extra_ldflags}"
	LDFLAGS_CONSOLE="-mconsole ${extra_ldflags}"
	LDFLAGS_WINDOW="-mwindows ${extra_ldflags}"
    else
	if test "${SHARED_BUILD}" = "0" ; then
	    # static
            echo "$ac_t""using static flags" 1>&6
	    runtime=-MT
	    MAKE_DLL="echo "
	    LIBSUFFIX="s\${DBGX}.lib"
	    LIBFLAGSUFFIX="s\${DBGX}"
	    LIBRARIES="\${STATIC_LIBRARIES}"
	    EXESUFFIX="s\${DBGX}.exe"
	    SHLIB_LD_LIBS=""
	else
	    # dynamic
            echo "$ac_t""using shared flags" 1>&6
	    runtime=-MD
	    # Add SHLIB_LD_LIBS to the Make rule, not here.
	    MAKE_DLL="\${SHLIB_LD} \$(LDFLAGS) -out:\$@"
	    LIBSUFFIX="\${DBGX}.lib"
	    LIBFLAGSUFFIX="\${DBGX}"
	    EXESUFFIX="\${DBGX}.exe"
	    LIBRARIES="\${SHARED_LIBRARIES}"
	    SHLIB_LD_LIBS='${LIBS}'
	fi
	# DLLSUFFIX is separate because it is the building block for
	# users of tclConfig.sh that may build shared or static.
	DLLSUFFIX="\${DBGX}.dll"

	# This is a 2-stage check to make sure we have the 64-bit SDK
	# We have to know where the SDK is installed.
	# This magic is based on MS Platform SDK for Win2003 SP1 - hobbs
	if test "$do64bit" != "no" ; then
	    if test "x${MSSDK}x" = "xx" ; then
		MSSDK="C:/Progra~1/Microsoft Platform SDK"
	    fi
	    MSSDK=`echo "$MSSDK" | sed -e 's!\\\!/!g'`
	    PATH64=""
	    case "$do64bit" in
		amd64|x64|yes)
		    MACHINE="AMD64" ; # default to AMD64 64-bit build
		    PATH64="${MSSDK}/Bin/Win64/x86/AMD64"
		    ;;
		ia64)
		    MACHINE="IA64"
		    PATH64="${MSSDK}/Bin/Win64"
		    ;;
	    esac
	    if test ! -d "${PATH64}" ; then
		echo "configure: warning: Could not find 64-bit $MACHINE SDK to enable 64bit mode" 1>&2
		echo "configure: warning: Ensure latest Platform SDK is installed" 1>&2
		do64bit="no"
	    else
		echo "$ac_t""   Using 64-bit $MACHINE mode" 1>&6
	    fi
	fi

	if test "$do64bit" != "no" ; then
	    # The space-based-path will work for the Makefile, but will
	    # not work if AC_TRY_COMPILE is called.
	    CC="\"${PATH64}/cl.exe\" -I\"${MSSDK}/Include\" \
		-I\"${MSSDK}/Include/crt\" -I\"${MSSDK}/Include/crt/sys\""
	    RC="\"${MSSDK}/bin/rc.exe\""
	    CFLAGS_DEBUG="-nologo -Zi -Od ${runtime}d"
	    # Do not use -O2 for Win64 - this has proved buggy in code gen.
	    CFLAGS_OPTIMIZE="-nologo -O1 ${runtime}"
	    lflags="-nologo -MACHINE:${MACHINE} -LIBPATH:\"${MSSDK}/Lib/${MACHINE}\""
	    LINKBIN="\"${PATH64}/link.exe\""
	    # Avoid 'unresolved external symbol __security_cookie' errors.
	    # c.f. http://support.microsoft.com/?id=894573
	    LIBS="user32.lib advapi32.lib bufferoverflowU.lib"
	else
	    RC="rc"
	    # -Od - no optimization
	    # -WX - warnings as errors
	    CFLAGS_DEBUG="-nologo -Z7 -Od -WX ${runtime}d"
	    # -O2 - create fast code (/Og /Oi /Ot /Oy /Ob2 /Gs /GF /Gy)
	    CFLAGS_OPTIMIZE="-nologo -O2 ${runtime}"
	    lflags="-nologo"
	    LINKBIN="link"
	    LIBS="user32.lib advapi32.lib"
	fi

	LIBS_GUI="gdi32.lib comdlg32.lib imm32.lib comctl32.lib shell32.lib"
	SHLIB_LD="${LINKBIN} -dll -incremental:no ${lflags}"
	# link -lib only works when -lib is the first arg
	STLIB_LD="${LINKBIN} -lib ${lflags}"
	RC_OUT=-fo
	RC_TYPE=-r
	RC_INCLUDE=-i
	RC_DEFINE=-d
	RES=res
	MAKE_LIB="\${STLIB_LD} -out:\$@"
	POST_MAKE_LIB=
	MAKE_EXE="\${CC} -Fe\$@"
	LIBPREFIX=""

	EXTRA_CFLAGS=""
	CFLAGS_WARNING="-W3"
	LDFLAGS_DEBUG="-debug:full"
	LDFLAGS_OPTIMIZE="-release"
	
	# Specify the CC output file names based on the target name
	CC_OBJNAME="-Fo\$@"
	CC_EXENAME="-Fe\"\$(shell \$(CYGPATH) '\$@')\""

	# Specify linker flags depending on the type of app being 
	# built -- Console vs. Window.
	LDFLAGS_CONSOLE="-link -subsystem:console ${lflags}"
	LDFLAGS_WINDOW="-link -subsystem:windows ${lflags}"
    fi

    # DL_LIBS is empty, but then we match the Unix version
    
    
    
    


#--------------------------------------------------------------------
# Set the default compiler switches based on the --enable-symbols 
# option.  This macro depends on C flags, and should be called
# after SC_CONFIG_CFLAGS macro is called.
#--------------------------------------------------------------------


    echo $ac_n "checking for build with symbols""... $ac_c" 1>&6
echo "configure:1763: checking for build with symbols" >&5
    # Check whether --enable-symbols or --disable-symbols was given.
if test "${enable_symbols+set}" = set; then
  enableval="$enable_symbols"
  tcl_ok=$enableval
else
  tcl_ok=no
fi

# FIXME: Currently, LDFLAGS_DEFAULT is not used, it should work like CFLAGS_DEFAULT.
    if test "$tcl_ok" = "no"; then
	CFLAGS_DEFAULT='$(CFLAGS_OPTIMIZE)'
	LDFLAGS_DEFAULT='$(LDFLAGS_OPTIMIZE)'
	DBGX=""
	echo "$ac_t""no" 1>&6
    else
	CFLAGS_DEFAULT='$(CFLAGS_DEBUG)'
	LDFLAGS_DEFAULT='$(LDFLAGS_DEBUG)'
	DBGX=g
	if test "$tcl_ok" = "yes"; then
	    echo "$ac_t""yes (standard debugging)" 1>&6
	fi
    fi
    
    

    if test "$tcl_ok" = "mem" -o "$tcl_ok" = "all"; then
	cat >> confdefs.h <<\EOF
#define TCL_MEM_DEBUG 1
EOF

    fi

    if test "$tcl_ok" = "compile" -o "$tcl_ok" = "all"; then
	cat >> confdefs.h <<\EOF
#define TCL_COMPILE_DEBUG 1
EOF

	cat >> confdefs.h <<\EOF
#define TCL_COMPILE_STATS 1
EOF

    fi

    if test "$tcl_ok" != "yes" -a "$tcl_ok" != "no"; then
	if test "$tcl_ok" = "all"; then
	    echo "$ac_t""enabled symbols mem compile debugging" 1>&6
	else
	    echo "$ac_t""enabled $tcl_ok debugging" 1>&6
	fi
    fi


TCL_DBGX=${DBGX}

#--------------------------------------------------------------------
# man2tcl needs this so that it can use errno.h
#--------------------------------------------------------------------

echo $ac_n "checking how to run the C preprocessor""... $ac_c" 1>&6
echo "configure:1823: checking how to run the C preprocessor" >&5
# On Suns, sometimes $CPP names a directory.
if test -n "$CPP" && test -d "$CPP"; then
  CPP=
fi
if test -z "$CPP"; then
if eval "test \"`echo '$''{'ac_cv_prog_CPP'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
    # This must be in double quotes, not single quotes, because CPP may get
  # substituted into the Makefile and "${CC-cc}" will confuse make.
  CPP="${CC-cc} -E"
  # On the NeXT, cc -E runs the code through the compiler's parser,
  # not just through cpp.
  cat > conftest.$ac_ext <<EOF
#line 1838 "configure"
#include "confdefs.h"
#include <assert.h>
Syntax Error
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1844: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  :
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  CPP="${CC-cc} -E -traditional-cpp"
  cat > conftest.$ac_ext <<EOF
#line 1855 "configure"
#include "confdefs.h"
#include <assert.h>
Syntax Error
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1861: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  :
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  CPP="${CC-cc} -nologo -E"
  cat > conftest.$ac_ext <<EOF
#line 1872 "configure"
#include "confdefs.h"
#include <assert.h>
Syntax Error
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1878: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  :
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  CPP=/lib/cpp
fi
rm -f conftest*
fi
rm -f conftest*
fi
rm -f conftest*
  ac_cv_prog_CPP="$CPP"
fi
  CPP="$ac_cv_prog_CPP"
else
  ac_cv_prog_CPP="$CPP"
fi
echo "$ac_t""$CPP" 1>&6

ac_safe=`echo "errno.h" | sed 'y%./+-%__p_%'`
echo $ac_n "checking for errno.h""... $ac_c" 1>&6
echo "configure:1904: checking for errno.h" >&5
if eval "test \"`echo '$''{'ac_cv_header_$ac_safe'+set}'`\" = set"; then
  echo $ac_n "(cached) $ac_c" 1>&6
else
  cat > conftest.$ac_ext <<EOF
#line 1909 "configure"
#include "confdefs.h"
#include <errno.h>
EOF
ac_try="$ac_cpp conftest.$ac_ext >/dev/null 2>conftest.out"
{ (eval echo configure:1914: \"$ac_try\") 1>&5; (eval $ac_try) 2>&5; }
ac_err=`grep -v '^ *+' conftest.out | grep -v "^conftest.${ac_ext}\$"`
if test -z "$ac_err"; then
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=yes"
else
  echo "$ac_err" >&5
  echo "configure: failed program was:" >&5
  cat conftest.$ac_ext >&5
  rm -rf conftest*
  eval "ac_cv_header_$ac_safe=no"
fi
rm -f conftest*
fi
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  echo "$ac_t""yes" 1>&6
  :
else
  echo "$ac_t""no" 1>&6
MAN2TCLFLAGS="-DNO_ERRNO_H"
fi



#------------------------------------------------------------------------
# tclConfig.sh refers to this by a different name
#------------------------------------------------------------------------

TCL_SHARED_BUILD=${SHARED_BUILD}

#--------------------------------------------------------------------
# Perform final evaluations of variables with possible substitutions.
#--------------------------------------------------------------------

TCL_SHARED_LIB_SUFFIX="\${NODOT_VERSION}${DLLSUFFIX}"
TCL_UNSHARED_LIB_SUFFIX="\${NODOT_VERSION}${LIBSUFFIX}"
TCL_EXPORT_FILE_SUFFIX="\${NODOT_VERSION}${LIBSUFFIX}"

eval "TCL_SRC_DIR=\"`cd $srcdir/..; pwd`\""

eval "TCL_DLL_FILE=tcl${VER}${DLLSUFFIX}"

eval "TCL_LIB_FILE=${LIBPREFIX}tcl$VER${LIBSUFFIX}"

eval "TCL_LIB_FLAG=\"-ltcl${VER}${LIBFLAGSUFFIX}\""
eval "TCL_BUILD_LIB_SPEC=\"-L`pwd` ${TCL_LIB_FLAG}\""
eval "TCL_LIB_SPEC=\"-L${libdir} ${TCL_LIB_FLAG}\""

eval "TCL_STUB_LIB_FILE=\"${LIBPREFIX}tclstub${VER}${LIBSUFFIX}\""
eval "TCL_STUB_LIB_FLAG=\"-ltclstub${VER}${LIBFLAGSUFFIX}\""
eval "TCL_BUILD_STUB_LIB_SPEC=\"-L`pwd` ${TCL_STUB_LIB_FLAG}\""
eval "TCL_STUB_LIB_SPEC=\"-L${libdir} ${TCL_STUB_LIB_FLAG}\""
eval "TCL_BUILD_STUB_LIB_PATH=\"`pwd`/${TCL_STUB_LIB_FILE}\""
eval "TCL_STUB_LIB_PATH=\"${libdir}/${TCL_STUB_LIB_FILE}\""

# Install time header dir can be set via --includedir
eval "TCL_INCLUDE_SPEC=\"-I${includedir}\""


eval "DLLSUFFIX=${DLLSUFFIX}"
eval "LIBPREFIX=${LIBPREFIX}"
eval "LIBSUFFIX=${LIBSUFFIX}"
eval "EXESUFFIX=${EXESUFFIX}"

CFG_TCL_SHARED_LIB_SUFFIX=${TCL_SHARED_LIB_SUFFIX}
CFG_TCL_UNSHARED_LIB_SUFFIX=${TCL_UNSHARED_LIB_SUFFIX}
CFG_TCL_EXPORT_FILE_SUFFIX=${TCL_EXPORT_FILE_SUFFIX}

#--------------------------------------------------------------------
# Adjust the defines for how the resources are built depending
# on symbols and static vs. shared.
#--------------------------------------------------------------------

if test ${SHARED_BUILD} = 0 ; then
    if test "${DBGX}" = "g"; then
        RC_DEFINES="${RC_DEFINE} STATIC_BUILD ${RC_DEFINE} DEBUG"
    else
        RC_DEFINES="${RC_DEFINE} STATIC_BUILD"
    fi
else
    if test "${DBGX}" = "g"; then
        RC_DEFINES="${RC_DEFINE} DEBUG"
    else
        RC_DEFINES=""
    fi
fi

#--------------------------------------------------------------------
#	The statements below define the symbol TCL_PACKAGE_PATH, which
#	gives a list of directories that may contain packages.  The list
#	consists of one directory for machine-dependent binaries and
#	another for platform-independent scripts.
#--------------------------------------------------------------------

if test "$prefix" != "$exec_prefix"; then
    TCL_PACKAGE_PATH="${libdir} ${prefix}/lib"
else
    TCL_PACKAGE_PATH="${prefix}/lib"
fi








# empty on win

















# win/tcl.m4 doesn't set (CFLAGS)







# win/tcl.m4 doesn't set (LDFLAGS)



























# empty on win, but needs sub'ing










# win only















trap '' 1 2 15
cat > confcache <<\EOF
# This file is a shell script that caches the results of configure
# tests run on this system so they can be shared between configure
# scripts and configure runs.  It is not useful on other systems.
# If it contains results you don't want to keep, you may remove or edit it.
#
# By default, configure uses ./config.cache as the cache file,
# creating it if it does not exist already.  You can give configure
# the --cache-file=FILE option to use a different cache file; that is
# what configure does when it calls configure scripts in
# subdirectories, so they share the cache.
# Giving --cache-file=/dev/null disables caching, for debugging configure.
# config.status only pays attention to the cache file if you give it the
# --recheck option to rerun configure.
#
EOF
# The following way of writing the cache mishandles newlines in values,
# but we know of no workaround that is simple, portable, and efficient.
# So, don't put newlines in cache variables' values.
# Ultrix sh set writes to stderr and can't be redirected directly,
# and sets the high bit in the cache file unless we assign to the vars.
(set) 2>&1 |
  case `(ac_space=' '; set | grep ac_space) 2>&1` in
  *ac_space=\ *)
    # `set' does not quote correctly, so add quotes (double-quote substitution
    # turns \\\\ into \\, and sed turns \\ into \).
    sed -n \
      -e "s/'/'\\\\''/g" \
      -e "s/^\\([a-zA-Z0-9_]*_cv_[a-zA-Z0-9_]*\\)=\\(.*\\)/\\1=\${\\1='\\2'}/p"
    ;;
  *)
    # `set' quotes correctly as required by POSIX, so do not add quotes.
    sed -n -e 's/^\([a-zA-Z0-9_]*_cv_[a-zA-Z0-9_]*\)=\(.*\)/\1=${\1=\2}/p'
    ;;
  esac >> confcache
if cmp -s $cache_file confcache; then
  :
else
  if test -w $cache_file; then
    echo "updating cache $cache_file"
    cat confcache > $cache_file
  else
    echo "not updating unwritable cache $cache_file"
  fi
fi
rm -f confcache

trap 'rm -fr conftest* confdefs* core core.* *.core $ac_clean_files; exit 1' 1 2 15

test "x$prefix" = xNONE && prefix=$ac_default_prefix
# Let make expand exec_prefix.
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

# Any assignment to VPATH causes Sun make to only execute
# the first set of double-colon rules, so remove it if not needed.
# If there is a colon in the path, we need to keep it.
if test "x$srcdir" = x.; then
  ac_vpsub='/^[ 	]*VPATH[ 	]*=[^:]*$/d'
fi

trap 'rm -f $CONFIG_STATUS conftest*; exit 1' 1 2 15

# Transform confdefs.h into DEFS.
# Protect against shell expansion while executing Makefile rules.
# Protect against Makefile macro expansion.
#
# If the first sed substitution is executed (which looks for macros that
# take arguments), then we branch to the quote section.  Otherwise,
# look for a macro that doesn't take arguments.
cat >confdef2opt.sed <<\_ACEOF
t clear
: clear
s,^[ 	]*#[ 	]*define[ 	][ 	]*\([^ 	(][^ 	(]*([^)]*)\)[ 	]*\(.*\),-D\1=\2,g
t quote
s,^[ 	]*#[ 	]*define[ 	][ 	]*\([^ 	][^ 	]*\)[ 	]*\(.*\),-D\1=\2,g
t quote
d
: quote
s,[ 	`~#$^&*(){}\\|;'"<>?],\\&,g
s,\[,\\&,g
s,\],\\&,g
s,\$,$$,g
p
_ACEOF
# We use echo to avoid assuming a particular line-breaking character.
# The extra dot is to prevent the shell from consuming trailing
# line-breaks from the sub-command output.  A line-break within
# single-quotes doesn't work because, if this script is created in a
# platform that uses two characters for line-breaks (e.g., DOS), tr
# would break.
ac_LF_and_DOT=`echo; echo .`
DEFS=`sed -n -f confdef2opt.sed confdefs.h | tr "$ac_LF_and_DOT" ' .'`
rm -f confdef2opt.sed


# Without the "./", some shells look in PATH for config.status.
: ${CONFIG_STATUS=./config.status}

echo creating $CONFIG_STATUS
rm -f $CONFIG_STATUS
cat > $CONFIG_STATUS <<EOF
#! /bin/sh
# Generated automatically by configure.
# Run this file to recreate the current configuration.
# This directory was configured as follows,
# on host `(hostname || uname -n) 2>/dev/null | sed 1q`:
#
# $0 $ac_configure_args
#
# Compiler output produced by configure, useful for debugging
# configure, is in ./config.log if it exists.

ac_cs_usage="Usage: $CONFIG_STATUS [--recheck] [--version] [--help]"
for ac_option
do
  case "\$ac_option" in
  -recheck | --recheck | --rechec | --reche | --rech | --rec | --re | --r)
    echo "running \${CONFIG_SHELL-/bin/sh} $0 $ac_configure_args --no-create --no-recursion"
    exec \${CONFIG_SHELL-/bin/sh} $0 $ac_configure_args --no-create --no-recursion ;;
  -version | --version | --versio | --versi | --vers | --ver | --ve | --v)
    echo "$CONFIG_STATUS generated by autoconf version 2.13"
    exit 0 ;;
  -help | --help | --hel | --he | --h)
    echo "\$ac_cs_usage"; exit 0 ;;
  *) echo "\$ac_cs_usage"; exit 1 ;;
  esac
done

ac_given_srcdir=$srcdir

trap 'rm -fr `echo "Makefile tclConfig.sh tcl.hpj" | sed "s/:[^ ]*//g"` conftest*; exit 1' 1 2 15
EOF
cat >> $CONFIG_STATUS <<EOF

# Protect against being on the right side of a sed subst in config.status.
sed 's/%@/@@/; s/@%/@@/; s/%g\$/@g/; /@g\$/s/[\\\\&%]/\\\\&/g;
 s/@@/%@/; s/@@/@%/; s/@g\$/%g/' > conftest.subs <<\\CEOF
$ac_vpsub
$extrasub
s%@SHELL@%$SHELL%g
s%@CFLAGS@%$CFLAGS%g
s%@CPPFLAGS@%$CPPFLAGS%g
s%@CXXFLAGS@%$CXXFLAGS%g
s%@FFLAGS@%$FFLAGS%g
s%@DEFS@%$DEFS%g
s%@LDFLAGS@%$LDFLAGS%g
s%@LIBS@%$LIBS%g
s%@exec_prefix@%$exec_prefix%g
s%@prefix@%$prefix%g
s%@program_transform_name@%$program_transform_name%g
s%@bindir@%$bindir%g
s%@sbindir@%$sbindir%g
s%@libexecdir@%$libexecdir%g
s%@datadir@%$datadir%g
s%@sysconfdir@%$sysconfdir%g
s%@sharedstatedir@%$sharedstatedir%g
s%@localstatedir@%$localstatedir%g
s%@libdir@%$libdir%g
s%@includedir@%$includedir%g
s%@oldincludedir@%$oldincludedir%g
s%@infodir@%$infodir%g
s%@mandir@%$mandir%g
s%@CC@%$CC%g
s%@AR@%$AR%g
s%@RANLIB@%$RANLIB%g
s%@RC@%$RC%g
s%@SET_MAKE@%$SET_MAKE%g
s%@OBJEXT@%$OBJEXT%g
s%@EXEEXT@%$EXEEXT%g
s%@TCL_THREADS@%$TCL_THREADS%g
s%@CYGPATH@%$CYGPATH%g
s%@DL_LIBS@%$DL_LIBS%g
s%@CFLAGS_DEBUG@%$CFLAGS_DEBUG%g
s%@CFLAGS_OPTIMIZE@%$CFLAGS_OPTIMIZE%g
s%@CFLAGS_WARNING@%$CFLAGS_WARNING%g
s%@CFLAGS_DEFAULT@%$CFLAGS_DEFAULT%g
s%@LDFLAGS_DEFAULT@%$LDFLAGS_DEFAULT%g
s%@CPP@%$CPP%g
s%@MAN2TCLFLAGS@%$MAN2TCLFLAGS%g
s%@TCL_VERSION@%$TCL_VERSION%g
s%@TCL_MAJOR_VERSION@%$TCL_MAJOR_VERSION%g
s%@TCL_MINOR_VERSION@%$TCL_MINOR_VERSION%g
s%@TCL_PATCH_LEVEL@%$TCL_PATCH_LEVEL%g
s%@TCL_LIB_FILE@%$TCL_LIB_FILE%g
s%@TCL_LIB_FLAG@%$TCL_LIB_FLAG%g
s%@TCL_LIB_SPEC@%$TCL_LIB_SPEC%g
s%@TCL_STUB_LIB_FILE@%$TCL_STUB_LIB_FILE%g
s%@TCL_STUB_LIB_FLAG@%$TCL_STUB_LIB_FLAG%g
s%@TCL_STUB_LIB_SPEC@%$TCL_STUB_LIB_SPEC%g
s%@TCL_STUB_LIB_PATH@%$TCL_STUB_LIB_PATH%g
s%@TCL_INCLUDE_SPEC@%$TCL_INCLUDE_SPEC%g
s%@TCL_BUILD_STUB_LIB_SPEC@%$TCL_BUILD_STUB_LIB_SPEC%g
s%@TCL_BUILD_STUB_LIB_PATH@%$TCL_BUILD_STUB_LIB_PATH%g
s%@TCL_DLL_FILE@%$TCL_DLL_FILE%g
s%@TCL_SRC_DIR@%$TCL_SRC_DIR%g
s%@TCL_BIN_DIR@%$TCL_BIN_DIR%g
s%@TCL_DBGX@%$TCL_DBGX%g
s%@CFG_TCL_SHARED_LIB_SUFFIX@%$CFG_TCL_SHARED_LIB_SUFFIX%g
s%@CFG_TCL_UNSHARED_LIB_SUFFIX@%$CFG_TCL_UNSHARED_LIB_SUFFIX%g
s%@CFG_TCL_EXPORT_FILE_SUFFIX@%$CFG_TCL_EXPORT_FILE_SUFFIX%g
s%@EXTRA_CFLAGS@%$EXTRA_CFLAGS%g
s%@DEPARG@%$DEPARG%g
s%@CC_OBJNAME@%$CC_OBJNAME%g
s%@CC_EXENAME@%$CC_EXENAME%g
s%@LDFLAGS_DEBUG@%$LDFLAGS_DEBUG%g
s%@LDFLAGS_OPTIMIZE@%$LDFLAGS_OPTIMIZE%g
s%@LDFLAGS_CONSOLE@%$LDFLAGS_CONSOLE%g
s%@LDFLAGS_WINDOW@%$LDFLAGS_WINDOW%g
s%@STLIB_LD@%$STLIB_LD%g
s%@SHLIB_LD@%$SHLIB_LD%g
s%@SHLIB_LD_LIBS@%$SHLIB_LD_LIBS%g
s%@SHLIB_CFLAGS@%$SHLIB_CFLAGS%g
s%@SHLIB_SUFFIX@%$SHLIB_SUFFIX%g
s%@TCL_SHARED_BUILD@%$TCL_SHARED_BUILD%g
s%@LIBS_GUI@%$LIBS_GUI%g
s%@DLLSUFFIX@%$DLLSUFFIX%g
s%@LIBPREFIX@%$LIBPREFIX%g
s%@LIBSUFFIX@%$LIBSUFFIX%g
s%@EXESUFFIX@%$EXESUFFIX%g
s%@LIBRARIES@%$LIBRARIES%g
s%@MAKE_LIB@%$MAKE_LIB%g
s%@POST_MAKE_LIB@%$POST_MAKE_LIB%g
s%@MAKE_DLL@%$MAKE_DLL%g
s%@MAKE_EXE@%$MAKE_EXE%g
s%@TCL_BUILD_LIB_SPEC@%$TCL_BUILD_LIB_SPEC%g
s%@TCL_LD_SEARCH_FLAGS@%$TCL_LD_SEARCH_FLAGS%g
s%@TCL_NEEDS_EXP_FILE@%$TCL_NEEDS_EXP_FILE%g
s%@TCL_BUILD_EXP_FILE@%$TCL_BUILD_EXP_FILE%g
s%@TCL_EXP_FILE@%$TCL_EXP_FILE%g
s%@LIBOBJS@%$LIBOBJS%g
s%@TCL_LIB_VERSIONS_OK@%$TCL_LIB_VERSIONS_OK%g
s%@TCL_PACKAGE_PATH@%$TCL_PACKAGE_PATH%g
s%@TCL_DDE_VERSION@%$TCL_DDE_VERSION%g
s%@TCL_DDE_MAJOR_VERSION@%$TCL_DDE_MAJOR_VERSION%g
s%@TCL_DDE_MINOR_VERSION@%$TCL_DDE_MINOR_VERSION%g
s%@TCL_REG_VERSION@%$TCL_REG_VERSION%g
s%@TCL_REG_MAJOR_VERSION@%$TCL_REG_MAJOR_VERSION%g
s%@TCL_REG_MINOR_VERSION@%$TCL_REG_MINOR_VERSION%g
s%@RC_OUT@%$RC_OUT%g
s%@RC_TYPE@%$RC_TYPE%g
s%@RC_INCLUDE@%$RC_INCLUDE%g
s%@RC_DEFINE@%$RC_DEFINE%g
s%@RC_DEFINES@%$RC_DEFINES%g
s%@RES@%$RES%g

CEOF
EOF

cat >> $CONFIG_STATUS <<\EOF

# Split the substitutions into bite-sized pieces for seds with
# small command number limits, like on Digital OSF/1 and HP-UX.
ac_max_sed_cmds=90 # Maximum number of lines to put in a sed script.
ac_file=1 # Number of current file.
ac_beg=1 # First line for current file.
ac_end=$ac_max_sed_cmds # Line after last line for current file.
ac_more_lines=:
ac_sed_cmds=""
while $ac_more_lines; do
  if test $ac_beg -gt 1; then
    sed "1,${ac_beg}d; ${ac_end}q" conftest.subs > conftest.s$ac_file
  else
    sed "${ac_end}q" conftest.subs > conftest.s$ac_file
  fi
  if test ! -s conftest.s$ac_file; then
    ac_more_lines=false
    rm -f conftest.s$ac_file
  else
    if test -z "$ac_sed_cmds"; then
      ac_sed_cmds="sed -f conftest.s$ac_file"
    else
      ac_sed_cmds="$ac_sed_cmds | sed -f conftest.s$ac_file"
    fi
    ac_file=`expr $ac_file + 1`
    ac_beg=$ac_end
    ac_end=`expr $ac_end + $ac_max_sed_cmds`
  fi
done
if test -z "$ac_sed_cmds"; then
  ac_sed_cmds=cat
fi
EOF

cat >> $CONFIG_STATUS <<EOF

CONFIG_FILES=\${CONFIG_FILES-"Makefile tclConfig.sh tcl.hpj"}
EOF
cat >> $CONFIG_STATUS <<\EOF
for ac_file in .. $CONFIG_FILES; do if test "x$ac_file" != x..; then
  # Support "outfile[:infile[:infile...]]", defaulting infile="outfile.in".
  case "$ac_file" in
  *:*) ac_file_in=`echo "$ac_file"|sed 's%[^:]*:%%'`
       ac_file=`echo "$ac_file"|sed 's%:.*%%'` ;;
  *) ac_file_in="${ac_file}.in" ;;
  esac

  # Adjust a relative srcdir, top_srcdir, and INSTALL for subdirectories.

  # Remove last slash and all that follows it.  Not all systems have dirname.
  ac_dir=`echo $ac_file|sed 's%/[^/][^/]*$%%'`
  if test "$ac_dir" != "$ac_file" && test "$ac_dir" != .; then
    # The file is in a subdirectory.
    test ! -d "$ac_dir" && mkdir "$ac_dir"
    ac_dir_suffix="/`echo $ac_dir|sed 's%^\./%%'`"
    # A "../" for each directory in $ac_dir_suffix.
    ac_dots=`echo $ac_dir_suffix|sed 's%/[^/]*%../%g'`
  else
    ac_dir_suffix= ac_dots=
  fi

  case "$ac_given_srcdir" in
  .)  srcdir=.
      if test -z "$ac_dots"; then top_srcdir=.
      else top_srcdir=`echo $ac_dots|sed 's%/$%%'`; fi ;;
  /*) srcdir="$ac_given_srcdir$ac_dir_suffix"; top_srcdir="$ac_given_srcdir" ;;
  *) # Relative path.
    srcdir="$ac_dots$ac_given_srcdir$ac_dir_suffix"
    top_srcdir="$ac_dots$ac_given_srcdir" ;;
  esac


  echo creating "$ac_file"
  rm -f "$ac_file"
  configure_input="Generated automatically from `echo $ac_file_in|sed 's%.*/%%'` by configure."
  case "$ac_file" in
  *Makefile*) ac_comsub="1i\\
# $configure_input" ;;
  *) ac_comsub= ;;
  esac

  ac_file_inputs=`echo $ac_file_in|sed -e "s%^%$ac_given_srcdir/%" -e "s%:% $ac_given_srcdir/%g"`
  sed -e "$ac_comsub
s%@configure_input@%$configure_input%g
s%@srcdir@%$srcdir%g
s%@top_srcdir@%$top_srcdir%g
" $ac_file_inputs | (eval "$ac_sed_cmds") > $ac_file
fi; done
rm -f conftest.s*

EOF
cat >> $CONFIG_STATUS <<EOF

EOF
cat >> $CONFIG_STATUS <<\EOF

exit 0
EOF
chmod +x $CONFIG_STATUS
rm -fr confdefs* $ac_clean_files
test "$no_create" = yes || ${CONFIG_SHELL-/bin/sh} $CONFIG_STATUS || exit 1

