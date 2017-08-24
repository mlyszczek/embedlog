/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


#ifndef EMBEDLOG_H
#define EMBEDLOG_H 1

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

#if NOFINFO
#   define ELE EL_LEVEL_ERR, NULL, 0
#   define ELW EL_LEVEL_WRN, NULL, 0
#   define ELI EL_LEVEL_INF, NULL, 0
#   define ELD EL_LEVEL_DBG, NULL, 0
#else
#   define ELE EL_LEVEL_ERR, __FILE__, __LINE__
#   define ELW EL_LEVEL_WRN, __FILE__, __LINE__
#   define ELI EL_LEVEL_INF, __FILE__, __LINE__
#   define ELD EL_LEVEL_DBG, __FILE__, __LINE__
#endif

#if (__STDC_VERSION__ >= 199901L)
#   if NDEBUG
#       define EL_DEBUG(...) ((void)0)
#   else
#       define EL_DEBUG(...) el_print(ELD, __VA_ARGS__)
#   endif
#endif

enum el_output
{
    EL_OUT_STDERR = 0x01,
    EL_OUT_SYSLOG = 0x02,
    EL_OUT_FILE   = 0x04,
    EL_OUT_NET    = 0x08,
    EL_OUT_TTY    = 0x10,
    EL_OUT_ALL    = 0x1f
};

enum el_level
{
    EL_LEVEL_ERR,
    EL_LEVEL_WRN,
    EL_LEVEL_INF,
    EL_LEVEL_DBG
};

enum el_option
{
    EL_OPT_COLORS,
    EL_OPT_TS,
    EL_OPT_TS_TM,
    EL_OPT_PRINT_LEVEL,
    EL_OPT_FINFO,

    EL_OPT_FNAME,
    EL_OPT_FROTATE_NUMBER,
    EL_OPT_FROTATE_SIZE,

    EL_OPT_ERROR /* internal use only */
};

enum el_option_timestamp
{
    EL_OPT_TS_OFF,
    EL_OPT_TS_SHORT,
    EL_OPT_TS_LONG,

    EL_OPT_TS_ERROR /* internal use only */
};

enum el_option_timestamp_timer
{
    EL_OPT_TS_TM_CLOCK,
    EL_OPT_TS_TM_TIME,
    EL_OPT_TS_TM_REALTIME,
    EL_OPT_TS_TM_MONOTONIC,

    EL_OPT_TS_TM_ERROR /* internal use only */
};

struct el_options
{
    int          outputs;
    int          level;
    int          colors;
    int          timestamp;
    int          timestamp_timer;
    int          print_log_level;

    int          finfo;
    int          frotate_number;
    int          fcurrent_rotate;
    long         frotate_size;
    long         fpos;
    FILE        *file;
    const char  *fname;
};


int el_init(void);
int el_cleanup(void);
int el_level_set(enum el_level level);
int el_output_enable(enum el_output output);
int el_output_disable(enum el_output output);
int el_option(enum el_option option, ...);
int el_puts(const char *string);
int el_print(enum el_level level, const char *file, size_t line,
        const char *fmt, ...);
int el_vprint(enum el_level level, const char *file, size_t line,
        const char *fmt, va_list ap);
int el_pmemory(enum el_level level, const char *file, size_t line,
        const void *memory, size_t mlen);
int el_perror(enum el_level level, const char *file, size_t line,
        const char *fmt, ...);


int el_oinit(struct el_options *options);
int el_ocleanup(struct el_options *options);
int el_olevel_set(struct el_options *options, enum el_level level);
int el_ooutput_enable(struct el_options *options, enum el_output output);
int el_ooutput_disable(struct el_options *options, enum el_output output);
int el_ooption(struct el_options *options, enum el_option option, ...);
int el_oputs(struct el_options *options, const char *string);
int el_oprint(enum el_level level, const char *file, size_t line,
        struct el_options *options, const char *fnt, ...);
int el_ovprint(enum el_level level, const char *file, size_t line,
        struct el_options *options, const char *fmt, va_list ap);
int el_opmemory(enum el_level level, const char *file, size_t line,
        struct el_options *options, const void *memory, size_t mlen);
int el_operror(enum el_level level, const char *file, size_t line,
        struct el_options *options, const char *fmt, ...);


#endif
