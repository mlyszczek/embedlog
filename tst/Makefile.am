include Makefile.am.coverage
include $(top_srcdir)/embedlog-sources.mk
check_PROGRAMS = test

test_SOURCES = $(embedlog_sources)
test_SOURCES += main.c \
	test-el-options.c \
	test-el-print.c \
	test-el-file.c \
	test-el-perror.c \
	test-el-pmemory.c \
	mtest.h \
	test-group-list.h

test_CFLAGS = -I$(top_srcdir)/include \
	-I$(top_srcdir)/src \
	-I/usr/local/include \
	-O0 -g3 -ggdb \
	$(COVERAGE_CFLAGS)

test_LDFLAGS = -L/usr/local/lib \
	$(COVERAGE_LDFLAGS)
test_LDADD = -lrb

TESTS = $(check_PROGRAMS)
LOG_DRIVER = $(top_srcdir)/tap-driver
