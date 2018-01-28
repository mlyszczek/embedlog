include $(top_srcdir)/embedlog-sources.mk

lib_LTLIBRARIES = libembedlog.la

libembedlog_la_SOURCES = $(embedlog_sources)
libembedlog_la_SOURCES += config-priv.h \
	el-file.h \
	el-options.h \
	valid.h
libembedlog_la_LDFLAGS = -version-info 2:1:2
libembedlog_la_CFLAGS = -I$(top_srcdir)/include

# static code analyzer

if ENABLE_ANALYZER

analyze_plists = $(embedlog_sources:%.c=%.plist)
MOSTLYCLEANFILES = $(analyze_plists)

$(analyze_plists): %.plist: %.c
	@echo "  CCSA  " $@
	@clang --analyze -I.. $< -o $@

analyze: $(analyze_plists)

endif
