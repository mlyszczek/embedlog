include Makefile.am.coverage
check_PROGRAMS = test

test_SOURCES = main.c \
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
	$(COVERAGE_CFLAGS)

test_LDFLAGS = \
	$(COVERAGE_LDFLAGS) \
	-Wl,-Bstatic $(top_builddir)/src/libembedlog.la -Wl,-Bdynamic
test_LDADD = -lrb

TESTS = $(check_PROGRAMS)
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/tap-driver.sh
