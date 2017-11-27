AC_INIT([embedlog], [0.1.0], [michal.lyszczek@bofc.pl])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_C89
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_FILES([Makefile \
                 src/Makefile \
                 include/Makefile \
                 tst/Makefile \
                 man/Makefile \
                 examples/Makefile])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])


###
# compile own snprintf if system doesn't provide one
#


AC_CHECK_FUNCS([snprintf])
AM_CONDITIONAL([CUSTOM_SNPRINTF], [test "x$ac_cv_func_snprintf" != "xyes"])


###
# gcov coverage reporting
#


m4_include([m4/gcov.m4])
AC_TDD_GCOV
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_CXXFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)


###
# --enable-feature options
#

###
# --enable-out-stderr
#


AC_ARG_ENABLE([out-stderr],
    AS_HELP_STRING([--enable-out-stderr], [Enable printing to stderr]),
    [], [enable_out_stderr="yes"])

AS_IF([test "x$enable_out_stderr" = "xyes"],
[
    AC_DEFINE([ENABLE_OUT_STDERR], [1], [Enable printing to stderr])
    AC_CHECK_FUNCS(accesss)
    AC_CHECK_FUNCS([fputs],,
        AC_MSG_ERROR(not found, needed by --enable-out-stderr))
],
# else
[
    enable_out_stderr="no"
])

###
# --enable-out-file
#


AC_ARG_ENABLE([out-file],
    AS_HELP_STRING([--enable-out-file], [Enable printing to file]),
    [], [enable_out_file="yes"])

AM_CONDITIONAL([ENABLE_OUT_FILE], [test "x$enable_out_file" = "xyes"])
AS_IF([test "x$enable_out_file" = "xyes"],
[
    AC_DEFINE([ENABLE_OUT_FILE], [1], [Enable printing to file])
    AC_CHECK_FUNCS([fopen fclose fwrite remove rename],,
        AC_MSG_ERROR(not found, needed by --enable-out-file))
],
# else
[
    enable_out_file="no"
])


###
# --enable-out-custom
#


AC_ARG_ENABLE([out-custom],
    AS_HELP_STRING([--enable-out-custom], [Enable printing to custom]),
    [], [enable_out_custom="yes"])

AM_CONDITIONAL([ENABLE_OUT_CUSTOM], [test "x$enable_out_custom" = "xyes"])
AS_IF([test "x$enable_out_custom" = "xyes"],
[
    AC_DEFINE([ENABLE_OUT_CUSTOM], [1], [Enable printing to custom])
    AC_CHECK_FUNCS([fopen fclose fwrite remove rename],,
        AC_MSG_ERROR(not found, needed by --enable-out-custom))
],
# else
[
    enable_out_custom="no"
])


###
# --enable-timestamp
#


AC_ARG_ENABLE([timestamp],
    AS_HELP_STRING([--enable-timestamp], [Enable printing timestamp with log]),
    [], [enable_timestamp="yes"])

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


###
# --enable-realtime
#


AC_ARG_ENABLE([realtime],
    AS_HELP_STRING([--enable-realtime], [Enable using CLOCK_REALTIME in log]),
    [], [enable_realtime="yes"])

AS_IF([test "x$enable_realtime" = "xyes"],
[
    AS_IF([test "x$enable_timestamp" != "xyes"],
    [
        AC_MSG_WARN(--enable-realtime without --enable-timestamp has no effect)
    ])

    AC_DEFINE([ENABLE_REALTIME], [1], [Enable using CLOCK_REALTIME in log])
    AC_CHECK_FUNCS([clock_gettime],,
        AC_MSG_ERROR(not found, needed by --enable-realtime))
],
#else
[
    enable_realtime="no"
])


###
# --enable-monotonic
#


AC_ARG_ENABLE([monotonic],
    AS_HELP_STRING([--enable-monotonic], [Enable using CLOCK_MONOTONIC in log]),
    [], [enable_monotonic="yes"])

AS_IF([test "x$enable_monotonic" = "xyes"],
[
    AS_IF([test "x$enable_timestamp" != "xyes"],
    [
        AC_MSG_WARN(--enable-monotonic without --enable-timestamp has no effect)
    ])

    AC_DEFINE([ENABLE_MONOTONIC], [1], [Enable using CLOCK_MONOTONIC in log])
    AC_CHECK_FUNCS([clock_gettime],,
        AC_MSG_ERROR(not found, needed by --enable-monotonic))
],
#else
[
    enable_monotonic="no"
])


###
# --enable-finfo
#


AC_ARG_ENABLE([finfo],
    AS_HELP_STRING([--enable-finfo], [Enable printing file info with log]),
    [], [enable_finfo="yes"])

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


###
# --enable-colors
#


AC_ARG_ENABLE([colors],
    AS_HELP_STRING([--enable-colors], [Enable output colorization]),
    [], [enable_colors="yes"])

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


###
# --enable-reentrant
#


AC_ARG_ENABLE([reentrant],
    AS_HELP_STRING([--enable-reentrant], [Enable reentrant functions]),
    [], [enable_reentrant="yes"])

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


###
# VARIABLES=value options
#


###
# EL_LOG_MAX
#


AC_ARG_VAR([EL_LOG_MAX], [Maximum size of log message])
AS_IF([test "x$EL_LOG_MAX" = x], [EL_LOG_MAX="8192"])
AC_DEFINE_UNQUOTED([EL_LOG_MAX], [$EL_LOG_MAX], [Maximum size of log message])


###
# EL_FLEN_MAX
#


AC_ARG_VAR([EL_FLEN_MAX], [Maximum size of file in message])
AS_IF([test "x$EL_FLEN_MAX" = x], [EL_FLEN_MAX="32"])
AC_DEFINE_UNQUOTED([EL_FLEN_MAX], [$EL_FLEN_MAX], [Maximum size of file in message])

###
# EL_MEM_LINE_SIZE
#


AC_ARG_VAR([EL_MEM_LINE_SIZE], [Maximum size of file in message])
AS_IF([test "x$EL_MEM_LINE_SIZE" = x], [EL_MEM_LINE_SIZE="16"])
AC_DEFINE_UNQUOTED([EL_MEM_LINE_SIZE], [$EL_MEM_LINE_SIZE],
    [Number of bytes to print in one line in el_pmemory function])


###
# MAX_PATH
#

AC_ARG_VAR([MAX_PATH], [Maximum size of path to log to file])
AS_IF([test "x$MAX_PATH" = x], [MAX_PATH="4096"])
AC_DEFINE_UNQUOTED([MAX_PATH], [$MAX_PATH],
    [Maximum length of path to open when printing to file])


AC_OUTPUT


echo
echo "embedlog compilation configuration summary"
echo
echo "print to stderr............. : $enable_out_stderr"
echo "print to file............... : $enable_out_file"
echo "print to custom routine..... : $enable_out_custom"
echo "print timestamp enabled..... : $enable_timestamp"
echo "clock_realtime timestamp.... : $enable_realtime"
echo "clock_monotonic timestamp... : $enable_monotonic"
echo "print file info with log.... : $enable_finfo"
echo "colorize output............. : $enable_colors"
echo "reentrant functions......... : $enable_reentrant"
echo "maximum file length......... : $EL_FLEN_MAX"
echo "maximum log message......... : $EL_LOG_MAX"
echo "maximum path length......... : $MAX_PATH"
echo "pmemory line size........... : $EL_MEM_LINE_SIZE"
