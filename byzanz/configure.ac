AC_INIT
AC_CANONICAL_TARGET([])

dnl need to do this here so intltool-update doesn't die a slow but awful death
PACKAGE=byzanz
dnl when going to/from release please set the nano (fourth number) right !
dnl releases only do Wall, cvs and prerelease does Werror too
AS_VERSION($PACKAGE, TOOLS_VERSION, 0, 3, 0, 1, BYZANZ_CVS="no", BYZANZ_CVS="yes")

dnl AM_MAINTAINER_MODE only provides the option to configure to enable it
AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE($PACKAGE,$VERSION)
AC_PROG_LIBTOOL

AC_CONFIG_SRCDIR([gifenc/gifenc.c])
AM_CONFIG_HEADER(config.h)

dnl Add parameters for aclocal
AC_CONFIG_MACRO_DIR(macros)
AC_SUBST(ACLOCAL_AMFLAGS, "-I macros")
AC_PROG_INTLTOOL

dnl the gettext stuff needed
ALL_LINGUAS="ar bs ca cs da de dz el en_GB es fi fr gl hu id it ja lv nb oc pl pt pt_BR ro ru sl sr sr@latin sv tr vi uk zh_CN"
GETTEXT_PACKAGE=$PACKAGE
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], "$GETTEXT_PACKAGE",
                   [gettext package name])
AM_GLIB_GNU_GETTEXT
AM_GLIB_DEFINE_LOCALEDIR(GNOMELOCALEDIR)


AC_PROG_CC
AM_PROG_CC_C_O
dnl For interactive UNIX (a Sun thing)
AC_ISC_POSIX

dnl decide on error flags
AS_COMPILER_FLAGS(ERROR_CFLAGS, "-Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wold-style-definition -Wdeclaration-after-statement -Wmissing-declarations -Wmissing-prototypes -Wredundant-decls -Wmissing-noreturn -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Winline -Wformat-nonliteral -Wformat-security -Wswitch-enum -Wswitch-default -Winit-self -Wmissing-include-dirs -Wundef -Waggregate-return -Wmissing-format-attribute -Wnested-externs -Wunsafe-loop-optimizations -Wpacked -Winvalid-pch -Wsync-nand")
if test "x$BYZANZ_CVS" = "xyes"
then
  AS_COMPILER_FLAGS(ERROR_CFLAGS, "-Werror")
fi

AC_HEADER_STDC([])
AC_C_INLINE

dnl ##############################
dnl # Do automated configuration #
dnl ##############################

dnl Check for tools:
dnl ================

dnl modify pkg-config path
AC_ARG_WITH(pkg-config-path, 
   AC_HELP_STRING([--with-pkg-config-path],
                  [colon-separated list of pkg-config(1) dirs]),
   [export PKG_CONFIG_PATH=${withval}])

AC_DEFINE_DIR(DATADIR, "${datadir}", [datadir])

dnl Check for essential libraries first:
dnl ====================================

GLIB_GSETTINGS

CAIRO_REQ="1.10"
GTK_REQ="3.0.0"
APPLET_REQ="2.91.91"
XDAMAGE_REQ="1.0"
GIO_REQ="2.35"

PKG_CHECK_MODULES(GTK, cairo >= $CAIRO_REQ gtk+-3.0 >= $GTK_REQ x11 gio-2.0 >= $GIO_REQ)

PKG_CHECK_MODULES(XDAMAGE, xdamage >= $XDAMAGE_REQ)

LIBPANEL_APPLET="libpanelapplet-4.0"
PKG_CHECK_MODULES(APPLET, $LIBPANEL_APPLET >= $APPLET_REQ,
                  have_applet=yes, have_applet=no)
AM_CONDITIONAL(HAVE_APPLET, [test x$have_applet = xyes])
AS_IF([test x$have_applet = xyes],
      [appletdir=`$PKG_CONFIG --variable=libpanel_applet_dir $LIBPANEL_APPLET`
       AC_SUBST(appletdir)],[])

PKG_CHECK_MODULES(GST, gstreamer-app-1.0 gstreamer-1.0)

GIFENC_CFLAGS="$GTK_CFLAGS $ERROR_CFLAGS"
GIFENC_LIBS="$GTK_LIBS"
AC_SUBST(GIFENC_CFLAGS)
AC_SUBST(GIFENC_LIBS)

BYZANZ_CFLAGS="$GTK_CFLAGS $XDAMAGE_CFLAGS $GST_CFLAGS $ERROR_CFLAGS"
BYZANZ_LIBS="$GTK_LIBS $XDAMAGE_LIBS $GST_LIBS"
AC_SUBST(BYZANZ_CFLAGS)
AC_SUBST(BYZANZ_LIBS)

APPLET_CFLAGS="$APPLET_CFLAGS $BYZANZ_CFLAGS"
APPLET_LIBS="$APPLET_LIBS $BYZANZ_LIBS"

dnl #########################
dnl # Make the output files #
dnl #########################

AC_OUTPUT(
Makefile
data/Makefile
gifenc/Makefile
macros/Makefile
po/Makefile.in
src/Makefile
)

