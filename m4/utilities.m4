AC_DEFUN([MOON_ARG_ENABLED_DISABLED_BY_DEFAULT],
[
	case "$1" in
		enable) enableval=no ;;
		disable) enableval=yes ;;
		*) AC_ERROR(Invalid arguments to m4 macro) ;;
	esac

	AC_ARG_ENABLE([$2], AC_HELP_STRING([--$1-$2], [$3]), [])	
])

AC_DEFUN([MOON_ARG_ENABLED_BY_DEFAULT], [MOON_ARG_ENABLED_DISABLED_BY_DEFAULT([disable], [$1], [$2])])
AC_DEFUN([MOON_ARG_DISABLED_BY_DEFAULT], [MOON_ARG_ENABLED_DISABLED_BY_DEFAULT([enable], [$1], [$2])])

dnl add conf options for the subdirectory only
AC_DEFUN([CUSTOM_SUBDIR_OPTION],
[
AC_CONFIG_SUBDIRS([$1])

m4_ifblank([$2], [rm -f $1/configure.gnu],
    [AX_PRINT_TO_FILE([$1/configure.gnu],
[#!/bin/sh
./configure $2
])
])

dnl If this macro or the invocation of it is changed, we want configure
dnl to be re-run in subdirs. We cannot touch configure because that
dnl would make it younger than its dependencies (e.g. configure.ac) and
dnl we would potentially miss to regenerate configure. Instead we let
dnl autoconf touch configure.ac, meaning when autoconf is run in topdir
dnl it will trigger autotools to be run in subdirs.
m4_syscmd([
test -f $1/configure.ac && touch $1/configure.ac
test -f $1/configure.in && touch $1/configure.in
])

])


dnl If needed, define the m4_ifblank and m4_ifnblank macros from autoconf 2.64
dnl This allows us to run with earlier Autoconfs as well.
ifdef([m4_ifblank],[],[
m4_define([m4_ifblank],
[m4_if(m4_translit([[$1]],  [ ][	][
]), [], [$2], [$3])])])
dnl
ifdef([m4_ifnblank],[],[
m4_define([m4_ifnblank],
[m4_if(m4_translit([[$1]],  [ ][	][
]), [], [$3], [$2])])])
