AC_INIT([embedlog], [0.0.1], [michal.lyszczek@bofc.pl])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])


#
# --enable-feature options
#

#
# --enable-out-stderr
#


AC_ARG_ENABLE([out-stderr],
    AS_HELP_STRING([--enable-out-stderr], [Enable printing to stderr]))

AS_IF([test "x$enable_out_stderr" = "xyes"],
[
    AC_DEFINE([ENABLE_OUT_STDERR], [1], [Enable printing to stderr])
    AC_CHECK_FUNCS([fputs],, AC_MSG_ERROR(fputs() was not found))
],
# else
[
    enable_out_stderr="no"
])


#
# VARIABLES=value options
#


#
# EL_LOG_MAX
#


AC_ARG_VAR([EL_LOG_MAX], [Maximum size of log message])
AS_IF([test "x$EL_LOG_MAX" = x], [EL_LOG_MAX="2 * 1024"])
AC_DEFINE_UNQUOTED([EL_LOG_MAX], [$EL_LOG_MAX], [Maximum size of log message])


AC_OUTPUT


echo
echo "embedlog compilation configuration summary"
echo
echo "print to stderr............. : $enable_out_stderr"
echo "maximum log message......... : $EL_LOG_MAX"
