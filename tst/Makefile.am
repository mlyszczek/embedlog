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
	test-el-pbinary.c \
	mtest.h \
	test-group-list.h

test_CFLAGS = -I$(top_builddir)/include \
	-I$(top_srcdir)/src \
	-I/usr/local/include \
	-I/usr/include \
	-DRUN_TESTS=1 \
	$(COVERAGE_CFLAGS)

test_LDFLAGS = -L/usr/local/lib \
	-L/usr/lib \
	$(COVERAGE_LDFLAGS)
test_LDADD = -lrb

TESTS = $(check_PROGRAMS)
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/tap-driver.sh
