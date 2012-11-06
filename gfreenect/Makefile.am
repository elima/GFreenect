MAINTAINERCLEANFILES = \
	Makefile.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = @PRJ_API_NAME@.pc

library_includedir=$(includedir)/@PRJ_API_NAME@
library_include_HEADERS = gfreenect.h

CLEANFILES = *~ @PRJ_API_NAME@.pc

AM_CFLAGS = $(GLIB_CFLAGS) -Wall \
	-DPKGDATADIR="\"$(pkgdatadir)\"" \
	-DENABLE_TESTS="\"$(enable_tests)\""

if ENABLE_DEBUG
AM_CFLAGS += -Werror -g3 -O0 -ggdb
else
AM_CFLAGS += -DG_DISABLE_ASSERT -DG_DISABLE_CHECKS
endif

gfreenect-marshal.h: gfreenect-marshal.list
	glib-genmarshal --header \
		--prefix=gfreenect_marshal gfreenect-marshal.list > gfreenect-marshal.h

gfreenect-marshal.c: gfreenect-marshal.list
	glib-genmarshal --body \
		--prefix=gfreenect_marshal gfreenect-marshal.list > gfreenect-marshal.c

BUILT_SOURCES = gfreenect-marshal.c gfreenect-marshal.h

# libraries
lib_LTLIBRARIES = lib@PRJ_API_NAME@.la

# libgfreenect
source_c = \
	gfreenect-frame-mode.c \
	gfreenect-device.c

source_h = \
	gfreenect.h \
	gfreenect-decls.h \
	gfreenect-frame-mode.h \
	gfreenect-device.h

source_h_priv =

lib@PRJ_API_NAME@_la_LIBADD = \
	$(GLIB_LIBS) \
	$(FREENECT_LIBS) \
	-lm

lib@PRJ_API_NAME@_la_CFLAGS  = \
	$(AM_CFLAGS) \
	$(FREENECT_CFLAGS)

lib@PRJ_API_NAME@_la_LDFLAGS = \
	-version-info 0:1:0 \
	-no-undefined

lib@PRJ_API_NAME@_la_SOURCES = \
	$(source_c) \
	$(source_h) \
	$(source_h_priv)

nodist_lib@PRJ_API_NAME@_la_SOURCES = \
	gfreenect-marshal.c \
	gfreenect-marshal.h

gfreenectdir = $(includedir)/@PRJ_API_NAME@
gfreenect_HEADERS = \
	$(source_h) \
	$(source_h_priv)

# introspection support
if HAVE_INTROSPECTION

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --warn-all --symbol-prefix=gfreenect
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

introspection_sources = $(gfreenect_HEADERS) $(source_c)

@PRJ_GIR_API_NAME@.gir: lib@PRJ_API_NAME@.la Makefile
@PRJ_GIR_TARGET_NAME@_INCLUDES = GObject-2.0 Gio-2.0
@PRJ_GIR_TARGET_NAME@_CFLAGS = $(INCLUDES) $(FREENECT_CFLAGS)
@PRJ_GIR_TARGET_NAME@_LIBS = @PRJ_API_NAME@ m freenect
@PRJ_GIR_TARGET_NAME@_LDFLAGS = $(FREENECT_LIBS)
@PRJ_GIR_TARGET_NAME@_NAMESPACE = @PRJ_GIR_NAMESPACE@
@PRJ_GIR_TARGET_NAME@_FILES = $(addprefix $(srcdir)/,$(introspection_sources))
@PRJ_GIR_TARGET_NAME@_SCANNERFLAGS = \
	--warn-all \
	--c-include='gfreenect.h' \
	--pkg-export=@PRJ_API_NAME@

INTROSPECTION_GIRS += @PRJ_GIR_API_NAME@.gir

girdir = @INTROSPECTION_GIRDIR@
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = @INTROSPECTION_TYPELIBDIR@
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)
endif

maintainer-clean-local:
	rm -rf tmp-introspect*

EXTRA_DIST = \
	gfreenect-marshal.list
