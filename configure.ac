#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([tinybasic], [0.1.0], [endeavor2wako@gmail.com])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src/basic.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
#~ IT_PROG_INTLTOOL
#~ GETTEXT_PACKAGE=$PACKAGE
#~ AC_SUBST(GETTEXT_PACKAGE)

# Checks for libraries.
#~ PKG_CHECK_MODULES(GTK, gtk+-3.0)
#~ PKG_CHECK_MODULES(NOTIFY, libnotify)
#~ PKG_CHECK_MODULES(JSONGLIB, json-glib-1.0)
#~ PKG_CHECK_MODULES(XAPP, xapp)
#~ PKG_CHECK_MODULES(TAGLIB, taglib_c)

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h locale.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([setlocale])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

                 #~ po/Makefile.in
                 #~ data/Makefile])

AC_OUTPUT

