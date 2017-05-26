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
    AC_CHECK_FUNCS([fputs],,
        AC_MSG_ERROR(not found, needed by --enable-out-stderr))
],
# else
[
    enable_out_stderr="no"
])


#
# --enable-timestamp
#


AC_ARG_ENABLE([timestamp],
    AS_HELP_STRING([--enable-timestamp], [Enable printing timestamp with log]))

AS_IF([test "x$enable_timestamp" = "xyes"],
[
    AC_DEFINE([ENABLE_TIMESTAMP], [1], [Enable printing timestamp with log])
    AC_CHECK_FUNCS([clock time],,
        AC_MSG_ERROR(not found, needed by --enable-timestamp))
],
#else
[
    enable_timestamp="no"
])


#
# --enable-realtime
#


AC_ARG_ENABLE([realtime],
    AS_HELP_STRING([--enable-realtime], [Enable using CLOCK_REALTIME in log]))

AS_IF([test "x$enable_realtime" = "xyes"],
[
    AC_DEFINE([ENABLE_REALTIME], [1], [Enable using CLOCK_REALTIME in log])
    AC_CHECK_FUNCS([clock_gettime],,
        AC_MSG_ERROR(not found, needed by --enable-realtime))
],
#else
[
    enable_realtime="no"
])


#
# --enable-monotonic
#


AC_ARG_ENABLE([monotonic],
    AS_HELP_STRING([--enable-monotonic], [Enable using CLOCK_MONOTONIC in log]))

AS_IF([test "x$enable_monotonic" = "xyes"],
[
    AC_DEFINE([ENABLE_MONOTONIC], [1], [Enable using CLOCK_MONOTONIC in log])
    AC_CHECK_FUNCS([clock_gettime],,
        AC_MSG_ERROR(not found, needed by --enable-monotonic))
],
#else
[
    enable_monotonic="no"
])


#
# --enable-finfo
#


AC_ARG_ENABLE([finfo],
    AS_HELP_STRING([--enable-finfo], [Enable printing file info with log]))

AS_IF([test "x$enable_finfo" = "xyes"],
[
    AC_DEFINE([ENABLE_FINFO], [1], [Enable printing file info with log])
    AC_CHECK_FUNCS([strncat strlen sprintf],,
        AC_MSG_ERROR(not found, needed by --enable-finfo))
],
#else
[
    enable_finfo="no"
])


#
# --enable-colors
#


AC_ARG_ENABLE([colors],
    AS_HELP_STRING([--enable-colors], [Enable output colorization]))

AS_IF([test "x$enable_colors" = "xyes"],
[
    AC_DEFINE([ENABLE_COLORS], [1], [Enable output colorization])
    AC_CHECK_FUNCS([strcpy strlen],,
        AC_MSG_ERROR(not found, needed by --enable-colors))
],
#else
[
    enable_colors="no"
])


#
# --enable-reentrant
#


AC_ARG_ENABLE([reentrant],
    AS_HELP_STRING([--enable-reentrant], [Enable reentrant functions]))

AS_IF([test "x$enable_reentrant" = "xyes"],
[
    AC_DEFINE([ENABLE_REENTRANT], [1], [Enable reentrant functions])
    AC_CHECK_FUNCS([gmtime_r],,
        AC_MSG_ERROR(not found, needed by --enable-reentrant))
],
#else
[
    enable_reentrant="no"
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


#
# EL_FLEN_MAX
#


AC_ARG_VAR([EL_FLEN_MAX], [Maximum size of file in message])
AS_IF([test "x$EL_FLEN_MAX" = x], [EL_FLEN_MAX="32"])
AC_DEFINE_UNQUOTED([EL_FLEN_MAX], [$EL_FLEN_MAX], [Maximum size of file in message])



AC_OUTPUT


echo
echo "embedlog compilation configuration summary"
echo
echo "print to stderr............. : $enable_out_stderr"
echo "print timestamp enabled..... : $enable_timestamp"
echo "clock_realtime timestamp.... : $enable_realtime"
echo "clock_monotonic timestamp... : $enable_monotonic"
echo "print file info with log.... : $enable_finfo"
echo "colorize output............. : $enable_colors"
echo "reentrant functions......... : $enable_reentrant"
echo "maimum file length.......... : $EL_FLEN_MAX"
echo "maximum log message......... : $EL_LOG_MAX"
