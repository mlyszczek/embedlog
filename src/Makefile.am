embedlog_sources = el-options.c \
	el-perror.c \
	el-pmemory.c \
	el-print.c \
	el-puts.c \
	el-ts.c \
	el-flush.c \
	el-utils.c \
	snprintf.c

if ENABLE_OUT_FILE
embedlog_sources += el-file.c
endif

if ENABLE_OUT_TTY
embedlog_sources += el-tty.c
endif

if ENABLE_BINARY_LOGS
embedlog_sources += el-pbinary.c \
	el-decode-number.c \
	el-encode-number.c
endif

if ENABLE_PTHREAD
embedlog_sources += el-lock.c
endif

lib_LTLIBRARIES = libembedlog.la

libembedlog_la_SOURCES = $(embedlog_sources)
libembedlog_la_SOURCES += el-private.h \
	el-utils.h \
	valid.h
LIBRARY_VERSION=6:0:6
#           .------' | `---.
#        current:revision:age
#           |        |     `- increment if interfaces have been added
#           |        |        set to zero if interfaces have been removed
#           |        |        or changed
#           |        `- increment if source code has changed
#           |           set to zero if current is incremented
#           `- increment if interfaces have been added, removed or changed
libembedlog_la_LDFLAGS = $(COVERAGE_CFLAGS) -version-info $(LIBRARY_VERSION)

libembedlog_la_CFLAGS = $(COVERAGE_CFLAGS) -I$(top_builddir)/include

# static code analyzer

if ENABLE_ANALYZER

analyze_plists = $(embedlog_sources:%.c=%.plist)
MOSTLYCLEANFILES = $(analyze_plists)

$(analyze_plists): %.plist: %.c
	@echo "  CCSA  " $@
	@clang --analyze -I.. -I../include -DHAVE_CONFIG_H=1 $< -o $@

analyze: $(analyze_plists)

endif
