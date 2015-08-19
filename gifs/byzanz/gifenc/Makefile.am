noinst_LTLIBRARIES = libgifenc.la

libgifenc_la_SOURCES = \
	gifenc.c \
	quantize.c

noinst_HEADERS = \
	gifenc.h

# we could use just glib instead of gtk here
# if we removed the gdk-pixbuf support routines

libgifenc_la_CFLAGS = $(BYZANZ_CFLAGS) 
libgifenc_la_LIBADD = $(BYZANZ_LIBS) 
