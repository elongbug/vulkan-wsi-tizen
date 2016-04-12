noinst_PROGRAMS =

noinst_PROGRAMS += tri			\
				   vulkaninfo

tri_CFLAGS = $(WAYLAND_CFLAGS) -I$(top_srcdir)/include
tri_LDADD = $(WAYLAND_LIBS) -lvulkan
tri_SOURCES = tri.c

vulkaninfo_CFLAGS = -I$(top_srcdir)/include
vulkaninfo_LDADD =  -lvulkan
vulkaninfo_SOURCES = vulkaninfo.c
