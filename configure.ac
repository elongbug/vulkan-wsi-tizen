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

LT_PREREQ([2.2])
LT_INIT([disable-static])

if test "x$GCC" = "xyes"; then
GCC_CFLAGS="-Wall -Wextra -Wno-unused-parameter \
			-Wno-missing-field-initializers -g -fvisibility=hidden \
			-Wstrict-prototypes -Wmissing-prototypes -Wsign-compare"
fi
AC_SUBST(GCC_CFLAGS)

# Vulkan platform Wayland
AC_ARG_ENABLE(wayland,
			  AC_HELP_STRING([--enable-wayland], [Enable wayland WSI functions]),
			  enable_wayland=$enableval,
			  enable_wayland=no)

if test x$enable_wayland = xyes; then
	PKG_CHECK_MODULES(WAYLAND, [wayland-client])
	AC_DEFINE([VK_USE_PLATFORM_WAYLAND_KHR], [1], [Enable wayland WSI functions])
fi

AM_CONDITIONAL(ENABLE_WAYLAND, test x$enable_wayland = xyes)

# Vulkan platform XLIB
AC_ARG_ENABLE(x11,
			  AC_HELP_STRING([--enable-x11], [Enable X11 WSI functions]),
			  enable_x11=$enableval,
			  enable_x11=no)

if test x$enable_x11 = xyes; then
	PKG_CHECK_MODULES(X11, [x11])
	AC_DEFINE([VK_USE_PLATFORM_XLIB_KHR], [1], [Enable X11 WSI functions])
fi

AM_CONDITIONAL(ENABLE_X11, test x$enable_x11 = xyes)

# Vulkan platform XCB
AC_ARG_ENABLE(xcb,
			  AC_HELP_STRING([--enable-xcb], [Enable XCB WSI functions]),
			  enable_xcb=$enableval,
			  enable_xcb=no)

if test x$enable_xcb = xyes; then
	PKG_CHECK_MODULES(XCB, [x11-xcb])
	AC_DEFINE([VK_USE_PLATFORM_XCB_KHR], [1], [Enable XCB WSI functions])
fi

AM_CONDITIONAL(ENABLE_XCB, test x$enable_xcb = xyes)

# Output files
AC_CONFIG_FILES([
Makefile
src/utils/Makefile
src/null-driver/Makefile
src/wsi/Makefile
])

AC_OUTPUT
