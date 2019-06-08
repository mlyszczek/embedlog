include $(top_srcdir)/embedlog-sources.mk

lib_LTLIBRARIES = libembedlog.la

libembedlog_la_SOURCES = $(embedlog_sources)
libembedlog_la_SOURCES += el-private.h \
	valid.h
libembedlog_la_LDFLAGS = -version-info 4:1:4
libembedlog_la_CFLAGS = -I$(top_srcdir)/include

# static code analyzer

if ENABLE_ANALYZER

analyze_plists = $(embedlog_sources:%.c=%.plist)
MOSTLYCLEANFILES = $(analyze_plists)

$(analyze_plists): %.plist: %.c
	@echo "  CCSA  " $@
	@clang --analyze -I.. -DHAVE_CONFIG_H=1 $< -o $@

analyze: $(analyze_plists)

endif
