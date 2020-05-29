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
libembedlog_la_LDFLAGS = -version-info 9999:0:0
libembedlog_la_CFLAGS = -I$(top_builddir)/include

# static code analyzer

if ENABLE_ANALYZER

analyze_plists = $(embedlog_sources:%.c=%.plist)
MOSTLYCLEANFILES = $(analyze_plists)

$(analyze_plists): %.plist: %.c
	@echo "  CCSA  " $@
	@clang --analyze -I.. -I../include -DHAVE_CONFIG_H=1 $< -o $@

analyze: $(analyze_plists)

endif
