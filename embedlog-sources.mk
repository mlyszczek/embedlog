embedlog_sources = el-options.c \
	el-perror.c \
	el-pmemory.c \
	el-print.c \
	el-puts.c \
	el-ts.c \
	el-decode-number.c \
	el-encode-number.c \
	snprintf.c

if ENABLE_OUT_FILE
embedlog_sources += el-file.c
endif

if ENABLE_OUT_TTY
embedlog_sources += el-tty.c
endif

if ENABLE_BINARY_LOGS
embedlog_sources += el-pbinary.c
endif
