AC_DEFUN([MOONLIGHT_CHECK_GALLIUM],
[
	AC_ARG_WITH(gallium_path, AC_HELP_STRING([--with-gallium-path=<path>], []),
		[], [with_gallium_path=$srcdir/../mesa])

	if test "x$with_gallium_path" = "xno"; then
		AC_MSG_WARN([You need to set the path to gallium to include pixel shaders in your Moonlight install])
		with_gallium_path="no"
	else
		if test ! -d "$with_gallium_path"; then
			AC_MSG_WARN([The path to gallium does not exist, you need to set it to an existing directory to include pixel shaders in your Moonlight install])
			with_gallium_path="no"
		fi
	fi

	if test "x$with_gallium_path" = "xno"; then
		AM_CONDITIONAL([HAVE_GALLIUM], false)
		GALLIUM_CFLAGS=
		GALLIUM_LIBS=
	else
		GALLIUM_PATH=$(cd "$with_gallium_path" && pwd)
		AC_SUBST(GALLIUM_PATH)
		AM_CONDITIONAL([HAVE_GALLIUM], true)
		GALLIUM_CFLAGS="-DUSE_GALLIUM=1 -I\$(GALLIUM_PATH)/src/gallium/include -I\$(GALLIUM_PATH)/src/gallium/auxiliary -I\$(GALLIUM_PATH)/src/gallium/drivers"
		GALLIUM_LIBS="\$(GALLIUM_PATH)/src/gallium/drivers/softpipe/libsoftpipe.a \$(GALLIUM_PATH)/src/gallium/auxiliary/libgallium.a"
	fi

	AC_SUBST(GALLIUM_CFLAGS)
	AC_SUBST(GALLIUM_LIBS)
])
