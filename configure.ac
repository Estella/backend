AC_PREREQ(2.61)
AC_INIT([repsheet], [1.4.0], [aaron@aaronbedra.com])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
#PKG_CHECK_MODULES([HIREDIS], [hiredis >= 0.10.0])
PKG_CHECK_MODULES([JSON_C], [json >= 0.9])
PKG_CHECK_MODULES([LIBCURL], [libcurl >= 7.14])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    test/Makefile
])

AC_OUTPUT
