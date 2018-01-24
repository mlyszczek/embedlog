embedlog_sources = el-options.c \
	el-perror.c \
	el-pmemory.c \
	el-print.c \
	el-puts.c

if ENABLE_OUT_FILE
embedlog_sources += el-file.c
endif

if CUSTOM_SNPRINTF
embedlog_sources += snprintf.c
endif
