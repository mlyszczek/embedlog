lib_LTLIBRARIES = libembedlog.la
libembedlog_la_SOURCES = el-print.c el-puts.c el-perror.c el-pmemory.c options.c

if ENABLE_OUT_FILE
libembedlog_la_SOURCES += el-file.c
endif

libembedlog_la_LDFLAGS = -version-info 0:0:0
libembedlog_la_CFLAGS = -I$(top_srcdir)/include

bin_PROGRAMS = test
test_SOURCES = el-print.c el-puts.c el-perror.c el-pmemory.c options.c main.c

if ENABLE_OUT_FILE
test_SOURCES += el-file.c
endif

test_CFLAGS = -I$(top_srcdir)/include -O0 -g3 -ggdb

TESTS = $(check_PROGRAMS)
LOG_DRIVER = $(top_srcdir)/tap-driver
