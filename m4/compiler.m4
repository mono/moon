AC_DEFUN([MOONLIGHT_CHECK_C_COMPILER],
[
	dnl add -fstack-protector-all when compiling with gcc
	if test "x$GCC" = "xyes" ; then
		CFLAGS="-fstack-protector-all $CFLAGS"
		CXXFLAGS="-fstack-protector-all $CXXFLAGS"
	fi

	dnl check for MMX support
	AC_COMPILE_IFELSE([
		int main () {
			int i = 0;
			int j = -1;
			__asm__ __volatile__ (
				"movd (%0), %%mm1;"
				"movd %%mm1, (%1);"
				: : "r" (&i), "r" (&j)
			);
			return j;
		}
	], AC_DEFINE(HAVE_MMX, [1], [MMX support]))

	dnl check for SSE2 support
	AC_COMPILE_IFELSE([
		#include <stdio.h>
		#include <string.h>
		int main () {
			char buffer[[128]];
			int j = 1;

			memset(buffer, 0x0, 128);
			__asm__ __volatile__ (
				"movdqu (%0), %%xmm1;"
				"movd %%xmm1, (%1);"
				: : "r" (buffer), "r" (&j)
			);
			return (int)j;
		}
	], AC_DEFINE(HAVE_SSE2, [1], [SSE2 support]))
])
