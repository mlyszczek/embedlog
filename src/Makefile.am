lib_LTLIBRARIES = libembedlog.la
libembedlog_la_SOURCES = el-print.c options.c
libembedlog_la_LDFLAGS = -version-info 0:0:0
libembedlog_la_CFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = test
test_SOURCES = el-print.c options.c main.c
test_CFLAGS = -I$(top_srcdir)/include -O0 -g3 -ggdb

TESTS = $(check_PROGRAMS)
LOG_DRIVER = $(top_srcdir)/tap-driver
