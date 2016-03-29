module_LTLIBRARIES = vulkan-wsi-tizen.la
moduledir = $(libdir)/vulkan

AM_CFLAGS = $(GCC_CFLAGS)

vulkan_wsi_tizen_la_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/src/utils
vulkan_wsi_tizen_la_LDFLAGS = -module -avoid-version
vulkan_wsi_tizen_la_LIBADD = $(top_builddir)/src/utils/libutils.la

vulkan_wsi_tizen_la_SOURCES = wsi.h	\
							  wsi.c

manifestdir = /etc/vulkan/icd.d
manifest_DATA = vulkan-wsi-tizen.json
