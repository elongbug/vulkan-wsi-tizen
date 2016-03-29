noinst_LTLIBRARIES = null-driver.la

AM_CFLAGS = $(GCC_CFLAGS)

null_driver_la_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src/utils
null_driver_la_LDFLAGS = -module -avoid-version
null_driver_la_LIBADD = $(top_builddir)/src/utils/libutils.la

null_driver_la_SOURCES = null-driver.h	\
						 null-driver.c
