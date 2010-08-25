AC_DEFUN([MOONLIGHT_CHECK_LLVM],
[
	AC_ARG_ENABLE(llvm,[  --enable-llvm	Enable the experimental LLVM back-end], enable_llvm=$enableval, enable_llvm=no)

	LLVM_CFLAGS=
	LLVM_LIBS=

	if test "x$enable_llvm" = "xyes"; then
		AC_PATH_PROG(LLVM_CONFIG, llvm-config, no)
		if test "x$LLVM_CONFIG" = "xno"; then
			AC_MSG_ERROR([llvm-config not found.])
		fi

		LLVM_LIBS=`$LLVM_CONFIG --libfiles core bitwriter jit interpreter x86codegen nativecodegen`

		AC_DEFINE(USE_LLVM, 1, [Include support for LLVM])
	fi

	AC_SUBST(LLVM_CFLAGS)
	AC_SUBST(LLVM_LIBS)

	AM_CONDITIONAL(ENABLE_LLVM, [test x$enable_llvm = xyes])
])