module_LTLIBRARIES = vulkan-wsi-tizen.la
moduledir = $(libdir)/vulkan

AM_CFLAGS = $(GCC_CFLAGS)

vulkan_wsi_tizen_includedir = $(includedir)/vulkan
vulkan_wsi_tizen_include_HEADERS = vulkan-wsi-tizen.h

vulkan_wsi_tizen_la_CFLAGS = $(AM_CFLAGS) -I$(top_srcdir)/include	\
							 -I$(top_srcdir)/src/utils				\
							 -fvisibility=hidden					\
							 $(TPL_CFLAGS)

vulkan_wsi_tizen_la_LDFLAGS = -module -avoid-version
vulkan_wsi_tizen_la_LIBADD = $(top_builddir)/src/utils/libutils.la	\
							 $(TPL_LIBS)

vulkan_wsi_tizen_la_SOURCES = wsi.h				\
							  entry-points.c	\
							  surface.c			\
							  swapchain.c		\
							  display.c			\
							  allocator.c		\
							  icd.c				\
							  extensions.c

manifestdir = /etc/vulkan/icd.d
manifest_DATA = vulkan-wsi-tizen.json
