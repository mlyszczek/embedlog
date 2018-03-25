/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


#ifndef EMBEDLOG_H
#define EMBEDLOG_H 1

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef NOFINFO
#   define ELF NULL, 0, EL_FATAL
#   define ELA NULL, 0, EL_ALERT
#   define ELC NULL, 0, EL_CRIT
#   define ELE NULL, 0, EL_ERROR
#   define ELW NULL, 0, EL_WARN
#   define ELN NULL, 0, EL_NOTICE
#   define ELI NULL, 0, EL_INFO
#   define ELD NULL, 0, EL_DBG
#else
#   define ELF __FILE__, __LINE__, EL_FATAL
#   define ELA __FILE__, __LINE__, EL_ALERT
#   define ELC __FILE__, __LINE__, EL_CRIT
#   define ELE __FILE__, __LINE__, EL_ERROR
#   define ELW __FILE__, __LINE__, EL_WARN
#   define ELN __FILE__, __LINE__, EL_NOTICE
#   define ELI __FILE__, __LINE__, EL_INFO
#   define ELD __FILE__, __LINE__, EL_DBG
#endif

#if (__STDC_VERSION__ >= 199901L)
#   ifdef NDEBUG
#       define EL_DEBUG(...) ((void)0)
#   else
#       define EL_DEBUG(...) el_print(ELD, __VA_ARGS__)
#   endif
#endif

enum el_output
{
    EL_OUT_NONE   = 0x0000,
    EL_OUT_STDERR = 0x0001,
    EL_OUT_SYSLOG = 0x0002,
    EL_OUT_FILE   = 0x0004,
    EL_OUT_NET    = 0x0008,
    EL_OUT_TTY    = 0x0010,
    EL_OUT_CUSTOM = 0x0020,
    EL_OUT_ALL    = 0x003f,
};

enum el_level
{
    EL_FATAL,
    EL_ALERT,
    EL_CRIT,
    EL_ERROR,
    EL_WARN,
    EL_NOTICE,
    EL_INFO,
    EL_DBG
};

enum el_option
{
    EL_LEVEL,
    EL_OUT,
    EL_COLORS,
    EL_TS,
    EL_TS_TM,
    EL_PRINT_LEVEL,
    EL_FINFO,
    EL_CUSTOM_PUTS,
    EL_TTY_DEV,

    EL_FNAME,
    EL_FROTATE_NUMBER,
    EL_FROTATE_SIZE,

    EL_OPT_ERROR /* internal use only */
};

enum el_option_timestamp
{
    EL_TS_OFF,
    EL_TS_SHORT,
    EL_TS_LONG,

    EL_TS_ERROR /* internal use only */
};

enum el_option_timestamp_timer
{
    EL_TS_TM_CLOCK,
    EL_TS_TM_TIME,
    EL_TS_TM_REALTIME,
    EL_TS_TM_MONOTONIC,

    EL_TS_TM_ERROR /* internal use only */
};

typedef int (*el_custom_puts)(const char *s);

struct el_options
{
    int             outputs;
    int             level;
    int             colors;
    int             timestamp;
    int             timestamp_timer;
    int             print_log_level;

    int             serial_fd;
    int             finfo;
    int             frotate_number;
    int             fcurrent_rotate;
    long            frotate_size;
    long            fpos;
    FILE           *file;
    const char     *fname;
    el_custom_puts  custom_puts;
};

int el_init(void);
int el_cleanup(void);
int el_option(int option, ...);
int el_puts(const char *string);
int el_print(const char *file, size_t line, enum el_level level,
        const char *fmt, ...);
int el_vprint(const char *file, size_t line, enum el_level level,
        const char *fmt, va_list ap);
int el_pmemory(const char *file, size_t line, enum el_level level,
        const void *memory, size_t mlen);
int el_perror(const char *file, size_t line, enum el_level level,
        const char *fmt, ...);


int el_oinit(struct el_options *options);
int el_ocleanup(struct el_options *options);
int el_ooption(struct el_options *options, int option, ...);
int el_oputs(struct el_options *options, const char *string);
int el_oprint(const char *file, size_t line, enum el_level level,
        struct el_options *options, const char *fnt, ...);
int el_ovprint(const char *file, size_t line, enum el_level level,
        struct el_options *options, const char *fmt, va_list ap);
int el_opmemory(const char *file, size_t line, enum el_level level,
        struct el_options *options, const void *memory, size_t mlen);
int el_operror(const char *file, size_t line, enum el_level level,
        struct el_options *options, const char *fmt, ...);


#endif
