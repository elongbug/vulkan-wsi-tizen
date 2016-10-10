m4_define([tvw_major], 0)
m4_define([tvw_minor], 0)
m4_define([tvw_micro], 0)

m4_define([tvw_version], [tvw_major.tvw_minor.tvw_micro])

AC_PREREQ([2.64])
AC_INIT([tvw], [tvw_version], [tkq.kim@samsung.com])

AC_SUBST([TVW_VERSION_MAJOR], [tvw_major_version])
AC_SUBST([TVW_VERSION_MINOR], [tvw_minor_version])
AC_SUBST([TVW_VERSION_MICRO], [tvw_micro_version])
AC_SUBST([TVW_VERSION], [tvw_version])

AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

AC_PROG_CC
PKG_PROG_PKG_CONFIG

LT_PREREQ([2.2])
LT_INIT([disable-static])

if test "x$GCC" = "xyes"; then
GCC_CFLAGS="-Wall -Wextra -Wno-unused-parameter \
			-Wno-missing-field-initializers -g -fvisibility=hidden \
			-Wstrict-prototypes -Wmissing-prototypes -Wsign-compare"
fi
AC_SUBST(GCC_CFLAGS)

PKG_CHECK_MODULES(WAYLAND, [wayland-client])
AC_DEFINE([VK_USE_PLATFORM_WAYLAND_KHR], [1], [Enable wayland WSI functions])

PKG_CHECK_MODULES(TPL, [tpl-egl])
PKG_CHECK_MODULES(TBM, [libtbm])
PKG_CHECK_MODULES(TDM, [libtdm])

# Output files
AC_CONFIG_FILES([
Makefile
src/utils/Makefile
src/null-driver/Makefile
src/wsi/Makefile
samples/Makefile
])

AC_OUTPUT
