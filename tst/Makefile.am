include Makefile.am.coverage
check_PROGRAMS = test test_wrapper

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

test_LDFLAGS = $(COVERAGE_LDFLAGS)
test_LDADD = -lrb $(top_builddir)/src/.libs/libembedlog.a

test_wrapper: test
	echo 'LD_PRELOAD=/usr/lib64/libfaketime.so FAKETIME_NO_CACHE=1 TZ=UTC ./test' > $@
	chmod +x $@

TESTS = test_wrapper
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/tap-driver.sh
