
noinst_LTLIBRARIES = libbyzanz.la
bin_PROGRAMS = byzanz-record byzanz-playback
man_MANS = byzanz.1 byzanz-record.1 byzanz-playback.1

noinst_HEADERS = \
	byzanzencoder.h \
	byzanzencoderbyzanz.h \
	byzanzencoderflv.h \
	byzanzencodergif.h \
	byzanzencodergstreamer.h \
	byzanzencoderogv.h \
	byzanzencoderwebm.h \
	byzanzlayer.h \
	byzanzlayercursor.h \
	byzanzlayerwindow.h \
	byzanzqueue.h \
	byzanzqueueinputstream.h \
	byzanzqueueoutputstream.h \
	byzanzrecorder.h \
	byzanzsession.h \
	byzanzselect.h \
	byzanzserialize.h \
	paneltogglebutton.h \
	screenshot-utils.h

libbyzanz_la_SOURCES = \
	byzanzencoder.c \
	byzanzencoderbyzanz.c \
	byzanzencoderflv.c \
	byzanzencodergif.c \
	byzanzencodergstreamer.c \
	byzanzencoderogv.c \
	byzanzencoderwebm.c \
	byzanzlayer.c \
	byzanzlayercursor.c \
	byzanzlayerwindow.c \
	byzanzqueue.c \
	byzanzqueueinputstream.c \
	byzanzqueueoutputstream.c \
	byzanzrecorder.c \
	byzanzsession.c \
	byzanzselect.c \
	byzanzserialize.c

libbyzanz_la_CFLAGS = $(BYZANZ_CFLAGS) -I$(top_srcdir)/gifenc
libbyzanz_la_LIBADD = $(BYZANZ_LIBS) $(top_builddir)/gifenc/libgifenc.la

byzanz_playback_SOURCES = \
	playback.c

byzanz_playback_CFLAGS = $(BYZANZ_CFLAGS)
byzanz_playback_LDADD = $(BYZANZ_LIBS) ./libbyzanz.la


byzanz_record_SOURCES = \
	record.c

byzanz_record_CFLAGS = $(BYZANZ_CFLAGS)
byzanz_record_LDADD = $(BYZANZ_LIBS) ./libbyzanz.la

if HAVE_APPLET
libexec_PROGRAMS = byzanz-applet

byzanz_applet_SOURCES = \
	byzanzapplet.c \
	paneltogglebutton.c \
	screenshot-utils.c

byzanz_applet_CFLAGS = -DBYZANZ_MENU_UI_DIR=\""$(uidir)"\" $(APPLET_CFLAGS)
byzanz_applet_LDADD = $(APPLET_LIBS) ./libbyzanz.la

gsettings_schemas_in_in = org.gnome.byzanz.applet.gschema.xml.in.in

@INTLTOOL_XML_NOMERGE_RULE@

gsettings_schemas_in = $(gsettings_schemas_in_in:.xml.in.in=.xml.in)
gsettings_SCHEMAS = $(gsettings_schemas_in:.xml.in=.xml)

%.gschema.xml.in: %.gschema.xml.in.in Makefile
	$(AM_V_GEN) $(SED) -e 's^\@datadir\@^$(datadir)^g' -e 's^\@GETTEXT_PACKAGE\@^$(GETTEXT_PACKAGE)^g' < $< > $@

@GSETTINGS_RULES@

applet_in_files = org.gnome.ByzanzApplet.panel-applet.in
applet_DATA     = $(applet_in_files:.panel-applet.in=.panel-applet)

$(applet_in_files): $(applet_in_files).in Makefile
	sed \
            -e "s|\@LIBEXECDIR\@|$(libexecdir)|" \
            -e "s|\@VERSION\@|$(PACKAGE_VERSION)|" \
            $< > $@

%.panel-applet: %.panel-applet.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

uidir = $(datadir)/byzanz
ui_DATA = byzanzapplet.xml

CLEANFILES = \
	$(applet_DATA) \
	$(applet_DATA).in \
	$(service_DATA) \
	$(schemas_DATA) \
	$(gsettings_SCHEMAS_in) \
	$(gsettings_SCHEMAS) \
	*.gschema.valid

EXTRA_DIST = \
	org.gnome.ByzanzApplet.panel-applet.in.in \
	$(gsettings_schemas_in_in)	\
	$(man_MANS) \
	$(ui_DATA)
endif
