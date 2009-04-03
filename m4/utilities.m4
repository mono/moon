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

