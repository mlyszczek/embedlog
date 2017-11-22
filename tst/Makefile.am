check_PROGRAMS = test
test_SOURCES  = main.c \
				el-options.c \
				el-print.c \
				el-file.c

test_SOURCES += ../src/el-print.c \
				../src/el-puts.c \
				../src/el-perror.c \
				../src/el-pmemory.c \
				../src/el-options.c

if ENABLE_OUT_FILE
test_SOURCES += ../src/el-file.c
endif

test_CFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src -O0 -g3 -ggdb -DNDEBUG
test_LDADD = -lrb

TESTS = $(check_PROGRAMS)
LOG_DRIVER = $(top_srcdir)/tap-driver
