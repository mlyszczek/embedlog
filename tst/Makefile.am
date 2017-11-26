include Makefile.am.coverage

check_PROGRAMS = test
test_SOURCES  = main.c \
				el-options.c \
				el-print.c \
				el-file.c \
				el-perror.c \
				el-pmemory.c

test_SOURCES += ../src/el-print.c \
				../src/el-puts.c \
				../src/el-perror.c \
				../src/el-pmemory.c \
				../src/el-options.c

if ENABLE_OUT_FILE
test_SOURCES += ../src/el-file.c
endif

if CUSTOM_SNPRINTF
test_SOURCES += ../src/snprintf.c
endif

test_CFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src -O0 -g3 -ggdb \
			  -DNDEBUG $(COVERAGE_CFLAGS)
test_LDFLAGS = $(COVERAGE_LDFLAGS)
test_LDADD = -lrb

TESTS = $(check_PROGRAMS)
LOG_DRIVER = $(top_srcdir)/tap-driver
