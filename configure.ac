AC_PREREQ(2.61)
AC_INIT([repsheet], [2.0.2], [aaron@aaronbedra.com])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([check], [check >= 0.9.4])
PKG_CHECK_MODULES([libhiredis], [libhiredis >= 0.10.0], , [PKG_CHECK_MODULES([hiredis], [hiredis >= 0.10.0])])

AC_CONFIG_FILES([
    Makefile
    src/Makefile
    test/Makefile
])

AC_OUTPUT
