AC_DEFUN([MOONLIGHT_CHECK_LARGE_FILE],
[
	AC_SYS_LARGEFILE
	AC_CACHE_CHECK([whether _LARGEFILE64_SOURCE needs to be defined for large files], moon_cv_largefile64_source, [
		AC_TRY_COMPILE([
			#include <sys/types.h>
			#include <sys/stat.h>
			#include <fcntl.h>
		],[
			int fd = open ("__o_largefile", O_CREAT | O_RDWR | O_LARGEFILE, 0644);
		],[
			moon_cv_largefile64_source="no"
		],[
			AC_TRY_COMPILE([
 				#define _LARGEFILE64_SOURCE 1
 				#include <sys/types.h>
 				#include <sys/stat.h>
 				#include <fcntl.h>
			],[
 				int fd = open ("__o_largefile", O_CREAT | O_RDWR | O_LARGEFILE, 0644);
			],[
 				moon_cv_largefile64_source="yes"
			],[
 				moon_cv_largefile64_source="unknown (large files may not be supported)"
			])
		])
	])

	if test "x$largefile64_source" = "xyes"; then
		LARGEFILE_CFLAGS="-D_LARGEFILE64_SOURCE=1"
	elif test "x$largefile64_source" = "xundefined"; then
		AC_DEFINE(O_LARGEFILE, 0, [Define to 0 if your system does not have the O_LARGEFILE flag])
	fi

	if test -n "$ac_cv_sys_large_files" -a "x$ac_cv_sys_large_files" != "xno"; then
		LARGEFILE_CFLAGS="$LARGEFILE_CFLAGS -D_LARGE_FILES=1"
	fi

	if test "x$ac_cv_sys_file_offset_bits" != "xno"; then
		LARGEFILE_CFLAGS="$LARGEFILE_CFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits"
	fi
])
