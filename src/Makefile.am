lib_LTLIBRARIES = libembedlog.la
libembedlog_la_SOURCES = el-print.c \
						 el-puts.c \
						 el-perror.c \
						 el-pmemory.c \
						 el-options.c

libembedlog_la_SOURCES += config-priv.h \
						  el-file.h \
						  el-options.h \
						  valid.h

if ENABLE_OUT_FILE
libembedlog_la_SOURCES += el-file.c
endif

if CUSTOM_SNPRINTF
libembedlog_la_SOURCES += snprintf.c
endif

libembedlog_la_LDFLAGS = -version-info 1:1:1
libembedlog_la_CFLAGS = -I$(top_srcdir)/include
