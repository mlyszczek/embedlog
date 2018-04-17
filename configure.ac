AC_INIT([embedlog], [0.2.2], [michal.lyszczek@bofc.pl])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AC_LANG([C])
AC_PROG_CC
AC_PROG_CC_C89
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_FILES([Makefile \
                 src/Makefile \
                 include/Makefile \
                 tst/Makefile \
                 man/Makefile \
                 examples/Makefile \
                 www/Makefile])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([config.h])


###
# compile own snprintf if system doesn't provide one
#


AC_CHECK_FUNCS([snprintf])
AS_IF([test "x$ac_cv_func_snprintf" != "xyes"],
[
    ###
    # system is missing snprintf, provide one from the library
    #

    AC_DEFINE([NEED_SNPRINTF_ONLY], [1], [Library uses only snprintf])
])


###
# force portable snprintf (from library source tree) instead of provided one
# by the system
#
# --enable-portable-snprintf
#


AC_ARG_ENABLE([portable-snprintf],
    AS_HELP_STRING([--enable-portable-snprintf], [Use snprintf provided by library]),
    [], [enable_portable_snprintf="no"])

AS_IF([test "x$enable_portable_snprintf" = "xyes"],
[
    AC_DEFINE([PREFER_PORTABLE_SNPRINTF], [1], [Use snprintf provided by library])
])


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
    AC_CHECK_FUNCS([fputs],,
        AC_MSG_ERROR(not found, needed by --enable-out-stderr))
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
    AC_CHECK_FUNCS(access stat fsync fileno)
    AC_CHECK_FUNCS([fopen fclose fwrite remove rename],,
        AC_MSG_ERROR(not found, needed by --enable-out-file))
])


###
# --enable-out-tty
#


AC_ARG_ENABLE([out-tty],
    AS_HELP_STRING([--enable-out-tty], [Enable printing to tty]),
    [], [enable_out_tty="yes"])

AM_CONDITIONAL([ENABLE_OUT_TTY], [test "x$enable_out_tty" = "xyes"])
AS_IF([test "x$enable_out_tty" = "xyes"],
[
    AC_DEFINE([ENABLE_OUT_TTY], [1], [Enable printing to tty])
    AC_CHECK_HEADERS(termios.h)
    AC_CHECK_FUNCS([open tcgetattr cfsetispeed tcsetattr close],,
        AC_MSG_ERROR(not found, needed by --enable-out-tty))
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
])


###
# --enable-clock
#


AC_ARG_ENABLE([clock],
    AS_HELP_STRING([--enable-clock], [Enable using clock() as tme source in log]),
    [], [enable_clock="yes"])

AS_IF([test "x$enable_clock" = "xyes"],
[
    AS_IF([test "x$enable_timestamp" != "xyes"],
    [
        AC_MSG_WARN(--enable-clock without --enable-timestamp has no effect)
    ])

    AC_DEFINE([ENABLE_CLOCK], [1], [Enable using clock() as time source in log])
    AC_CHECK_FUNCS([clock],,
        AC_MSG_ERROR(not found, needed by --enable-clock))
])


###
# --enable-binary
#


AC_ARG_ENABLE([binary-logs],
    AS_HELP_STRING([--enable-binary-logs], [Enable printing binary logs]),
    [], [enable_binary_logs="no"])

AM_CONDITIONAL([ENABLE_BINARY_LOGS], [test "x$enable_binary_logs" = "xyes"])
AS_IF([test "x$enable_binary_logs" = "xyes"],
[
    AC_DEFINE([ENABLE_PREFIX], [1], [Enable printing binary logs])
    AC_CHECK_FUNCS([memcpy],,
        AC_MSG_ERROR(not found, needed by --enable-binary-logs))
])


###
# --enable-prefix
#


AC_ARG_ENABLE([prefix],
    AS_HELP_STRING([--enable-prefix], [Enable printing prefix to each message]),
    [], [enable_prefix="yes"])

AS_IF([test "x$enable_prefix" = "xyes"],
[
    AC_DEFINE([ENABLE_PREFIX], [1], [Enable printing prefix to each message])
    AC_CHECK_FUNCS([strncat strlen],,
        AC_MSG_ERROR(not found, needed by --enable-prefix))
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
])


###
# --enable-colors-extended
#


AC_ARG_ENABLE([colors-extended],
    AS_HELP_STRING([--enable-colors-extended], [Enable more colors in logs]),
    [], [enable_colors_extended="no"])

AS_IF([test "x$enable_colors_extended" = "xyes"],
[
    AC_DEFINE([ENABLE_COLORS_EXTENDED], [1], [Enable more colors in logs])
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
])


###
# VARIABLES=value options
#

###
# EL_PREFIX_MAX
#


AC_ARG_VAR([EL_PREFIX_MAX], [Maximum size of prefix])
AS_IF([test "x$EL_PREFIX_MAX" = x], [EL_PREFIX_MAX="128"])
AC_DEFINE_UNQUOTED([EL_PREFIX_MAX], [$EL_PREFIX_MAX], [Maximum size of prefix])


###
# EL_LOG_MAX
#


AC_ARG_VAR([EL_LOG_MAX], [Maximum size of single log message])
AS_IF([test "x$EL_LOG_MAX" = x], [EL_LOG_MAX="8192"])
AC_DEFINE_UNQUOTED([EL_LOG_MAX], [$EL_LOG_MAX], [Maximum size of single log message])


###
# EL_FLEN_MAX
#


AC_ARG_VAR([EL_FLEN_MAX], [Maximum size of file info in message])
AS_IF([test "x$EL_FLEN_MAX" = x], [EL_FLEN_MAX="32"])
AC_DEFINE_UNQUOTED([EL_FLEN_MAX], [$EL_FLEN_MAX], [Maximum size of file info in message])

###
# EL_MEM_LINE_SIZE
#


AC_ARG_VAR([EL_MEM_LINE_SIZE], [Number of bytes to print in single lin in pmemory function])
AS_IF([test "x$EL_MEM_LINE_SIZE" = x], [EL_MEM_LINE_SIZE="16"])
AC_DEFINE_UNQUOTED([EL_MEM_LINE_SIZE], [$EL_MEM_LINE_SIZE],
    [Number of bytes to print in one line in el_pmemory function])


###
# MAX_PATH
#

AC_ARG_VAR([MAX_PATH], [Maximum length of path to log file (used when PATH_MAX is not available)])
AS_IF([test "x$MAX_PATH" = x], [MAX_PATH="4096"])
AC_DEFINE_UNQUOTED([MAX_PATH], [$MAX_PATH],
    [Maximum length of path to open when printing to file])


###
# --enable-analyzer
#


AC_ARG_ENABLE([analyzer],
    AS_HELP_STRING([--enable-analyzer], [Enable static code analyzer]),
    [enable_analyzer="yes"], [enable_analyzer="no"])
AM_CONDITIONAL([ENABLE_ANALYZER], [test "x$enable_analyzer" = "xyes"])


AC_OUTPUT


echo
echo "embedlog compilation configuration summary"
echo
echo "print to stderr............. : $enable_out_stderr"
echo "print to file............... : $enable_out_file"
echo "print to tty................ : $enable_out_tty"
echo "print to custom routine..... : $enable_out_custom"
echo "print timestamp enabled..... : $enable_timestamp"
echo "clock_realtime timestamp.... : $enable_realtime"
echo "clock_monotonic timestamp... : $enable_monotonic"
echo "clock() timestamp........... : $enable_clock"
echo "print prefix with each msg.. : $enable_prefix"
echo "print file info with log.... : $enable_finfo"
echo "force portable snprintf......: $enable_portable_snprintf"
echo "binary logs..................: $enable_binary_logs"
echo "colorize output............. : $enable_colors"
echo "extended colors............. : $enable_colors_extended"
echo "reentrant functions......... : $enable_reentrant"
echo "maximum prefix length....... : $EL_PREFIX_MAX"
echo "maximum file length......... : $EL_FLEN_MAX"
echo "maximum log message......... : $EL_LOG_MAX"
echo "maximum path length......... : $MAX_PATH"
echo "pmemory line size........... : $EL_MEM_LINE_SIZE"
