/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


/* ==== include files ======================================================= */


#include "embedlog.h"
#include "config.h"
#include "config-priv.h"
#include "options.h"
#include "valid.h"

#if ENABLE_TIMESTAMP
#include <time.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


/* ==== private variables =================================================== */


static const char char_level[4] = { 'e', 'w', 'i', 'd' };


#if ENABLE_COLORS

/*
 * index to remove all formatting
 */


#define COLOR_RESET 4


/*
 * colors indexes are synced with log level
 */


static const char *color[] =
{
    "\e[31m",  /* red */
    "\e[33m",  /* yellow */
    "\e[32m",  /* green */
    "\e[34m",  /* blue */
    "\e[0m"    /* remove all formats */
};

#endif


/* ==== private functions =================================================== */


/* ==========================================================================
    adds color information to 'buf' based on  'level'.   Returns  number  of
    bytes stored in buf.  If colors are disabled, function will return 0 and
    nothing will be stored int 'buf'.

    color can be one of log levels passed directly, or COLOR_RESET macro, which
    will reset colors.
   ========================================================================== */


static size_t el_color
(
    char *buf,   /* buffer where to store color info */
    int   level  /* log level or COLOR_RESET */
)
{
#if ENABLE_COLORS
    if (g_options.colors == 0)
    {
        /*
         * no colors, you got it!
         */

        return 0;
    }

    strcpy(buf, color[level]);
    return strlen(color[level]);

#else
    return 0;
#endif
}


/* ==========================================================================
    returns seconds and microseconds calculated from clock() function
   ========================================================================== */


#if ENABLE_TIMESTAMP

static void el_ts_clock
(
    long     *s,   /* seconds will be stored here */
    long     *us   /* microseconds will be stored here */
)
{
    clock_t  clk;  /* clock value */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    clk = clock();

    *s   = clk / CLOCKS_PER_SEC;
    *us  = clk % CLOCKS_PER_SEC;
    *us *= 1000000 / CLOCKS_PER_SEC;
}


/* ==========================================================================
    returns seconds and microseconds calculated from time() function.
   ========================================================================== */


static void el_ts_time
(
    long *s,  /* seconds will be stored here */
    long *us  /* microseconds will be stored here */
)
{
    *s = (long)time(NULL);
    *us = 0;
}


/* ==========================================================================
    returns seconds and microseconds calculated from clock_gettime function
   ========================================================================== */


#if ENABLE_REALTIME || ENABLE_MONOTONIC

static void el_ts_clock_gettime
(
    long           *s,     /* seconds will be stored here */
    long           *us,    /* microseconds will be stored here */
    clockid_t       clkid  /* clock id */
)
{
    struct timespec tp;    /* clock value */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    clock_gettime(clkid, &tp);

    *s  = tp.tv_sec;
    *us = tp.tv_nsec / 1000;
}

#endif

#endif /* ENABLE_TIMESTAMP */


/* ==========================================================================
    returns current  timestamp  in  'buf'  location.   Depending  on  global
    settings timestamp can be in long or short format.  Function will return
    number of bytes copied to 'buf'.  If timestamp has been disabled  during
    compilation time or in runtime during settings, function will return  0.
   ========================================================================== */


static size_t el_timestamp
(
    char       *buf  /* buffer where timestamp will be stored */
)
{
#if ENABLE_TIMESTAMP
    long        s;   /* timestamp seconds */
    long        us;  /* timestamp microseconds */
    size_t      tl;  /* timestamp length */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if (g_options.timestamp == EL_OPT_TS_OFF)
    {
        /*
         * user doesn't want us to print timestamp, that's fine
         */

        return 0;
    }

    /*
     * first we get seconds and microseconds from proper timer
     */

    switch (g_options.timestamp_timer)
    {
#if ENABLE_REALTIME

    case EL_OPT_TS_TM_REALTIME:
        el_ts_clock_gettime(&s, &us, CLOCK_REALTIME);
        break;

#endif

#if ENABLE_MONOTONIC

    case EL_OPT_TS_TM_MONOTONIC:
        el_ts_clock_gettime(&s, &us, CLOCK_MONOTONIC);
        break;

#endif

    case EL_OPT_TS_TM_TIME:
        el_ts_time(&s, &us);
        break;

    case EL_OPT_TS_TM_CLOCK:
        el_ts_clock(&s, &us);
        break;

    default:
        /*
         * bad timer means no timer
         */

        return 0;
    }

    /*
     * then convert retrieved time into string timestamp
     */

    if (g_options.timestamp == EL_OPT_TS_LONG)
    {
        struct tm   tm;   /* timestamp splitted */
        struct tm  *tmp;  /* timestamp splitted pointer */
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    #if ENABLE_REENTRANT
        gmtime_r((const time_t *)&s, &tm);
        tmp = &tm;
    #else
        tmp = gmtime((const time_t *)&s);
    #endif

        tl = strftime(buf, 21, "[%Y-%m-%d %H:%M:%S", tmp);
    }
    else
    {
        tl = sprintf(buf, "[%ld", s);
    }

    tl += sprintf(buf + tl, ".%06ld]", us);

    return tl;

#else
    return 0;
#endif /* ENABLE_TIMESTAMP */
}


/* ==========================================================================
    returns pointer to where basename of 's' starts

    Examples:
            path                 basename
            /path/to/file.c      file.c
            path/to/file.c       file.c
            /file.c              file.c
            file.c               file.c
            ""                   ""
            NULL                 segmentation fault
   ========================================================================== */


static const char *el_basename
(
    const char  *s      /* string to basename */
)
{
    const char  *base;  /* */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    base = s;

    for (; *s; ++s)
    {
        if (s[0] == '/' && s[1] != '\0')
        {
            base = s + 1;
        }
    }

    return base;
}


/* ==========================================================================
    stores file and line information in 'buf'.  Number of  bytes  stored  in
    'buf' is returned, if file info is disabled  during  compilation  or  in
    runtime options 0 is returned
   ========================================================================== */


static size_t el_finfo
(
    char        *buf,   /* location whre to store file information */
    const char  *file,  /* path to file - will be basenamed */
    int          num    /* line number (max 99999) */
)
{
#if ENABLE_FINFO
    const char  *base;  /* basenem of the 'file' */
    size_t       fl;    /* number of bytes stored in 'buf' */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if (g_options.finfo == 0)
    {
        /*
         * no file info for this caller
         */

        return 0;
    }

    base = el_basename(file);

    buf[0] = '[';
    buf[1] = '\0';
    strncat(buf, base, EL_PRE_FINFO_LEN);
    fl  = strlen(buf);
    fl += sprintf(buf + fl, ":%d]", num);

    return fl;

#else
    return 0;
#endif /* ENABLE_FINFO */
}


/* ==========================================================================
    puts string 's' to all enabled output facilities
   ========================================================================== */


static void el_puts
(
    const char *s  /* string to put into output */
)
{
#if ENABLE_OUT_STDERR
    if (g_options.outputs & EL_OUT_STDERR)
    {
        fputs(s, stderr);
    }
#endif

#if 0 /* TODO */
    if (g_options.outputs & EL_OUT_SYSLOG)
    {
        el_puts_syslog(s);
    }

    if (g_options.outputs & EL_OUT_FILE)
    {
        el_puts_file(s);
    }

    if (g_options.outputs & EL_OUT_NET)
    {
        el_puts_net(s);
    }

    if (g_options.outputs & EL_OUT_TTY)
    {
        el_puts_tty(s);
    }
#endif
}


/* ==========================================================================
    Prints message formated by 'fmt' and 'ap'  with  timestamp,  'file'  and
    line number 'num' information, with specified  'level'  into  configured
    outputs.  Function allocates on callers heap EL_BUF_MAX of  memory.   If
    log message is longer than available buffer, it will  be  truncated  and
    part of message will be lost.

    errno:
            EINVAL      level is invalid
            EINVAL      fmt is NULL
            ECHRNG      printing is disabled
            ENOBUFS     message was too long and has been truncated
   ========================================================================== */


static int el_printv
(
    enum el_level  level,                /* log level to print message with */
    const char    *file,                 /* file name where log is printed */
    size_t         num,                  /* line number where log is printed */
    const char    *fmt,                  /* message format (see printf (3)) */
    va_list        ap                    /* additional parameters for fmt */
)
{
    char           buf[EL_BUF_MAX + 2];  /* buffer for message to print */
    size_t         w;                    /* bytes written to buf */
    size_t         flen;                 /* length of the parsed fmt output */
    int            e;                    /* error code */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    VALID(EINVAL, 0 <= level && level <= EL_LEVEL_DBG);
    VALID(EINVAL, fmt);
    VALID(ECHRNG, el_log_allowed(level));

    e = 0;

    /*
     * add preamble and colors to log line buf
     */

    w  = el_color(buf, level);
    w += el_timestamp(buf + w);
    w += el_finfo(buf + w, file, num);

    if (w != 0 && buf[w - 1] == ']')
    {
        /*
         * at least one preamble has been added, add space beetween current
         * preamble and log level
         */

        *(buf + w) = ' ';
        ++w;
    }

    w += sprintf(buf + w, "%c/", char_level[level]);

    /*
     * add requested log from format, we add + 1 to include null termination
     */

    flen = vsnprintf(buf + w, EL_LOG_MAX + 1, fmt, ap);

    if (flen > EL_LOG_MAX)
    {
        /*
         * overflow would have occur, not all bytes have been copied, output
         * will truncated. Correct flen to number of bytes actually stored in
         * buf.
         */

        flen = EL_LOG_MAX;
        e = ENOBUFS;
    }

    w += flen;

    /*
     * add terminal formatting reset sequence
     */

    w += el_color(buf + w, COLOR_RESET);

    /*
     * make sure buf is always null terminated and contains new line character
     */

    buf[w++] = '\n';
    buf[w++] = '\0';

    el_puts(buf);

    if (e)
    {
        errno = e;
        return -1;
    }

    return 0;
}


/* ==== public functions ==================================================== */


/* ==========================================================================
    calls el_print with 'fmt' and '...' parameters, but  additionaly  prints
    information about errno.  Functionaly it is similar to  perror  function
   ========================================================================== */


int el_print_error
(
    enum el_level  level,                /* log level to print message with */
    const char    *file,                 /* file name where log is printed */
    size_t         num,                  /* line number where log is printed */
    const char    *fmt,                  /* message format (see printf (3)) */
                   ...                   /* additional parameters for fmt */
)
{
    va_list        ap;                   /* arguments '...' for 'fmt' */
    int            rc;                   /* return code from el_printfv() */
    unsigned long  e;                    /* errno from upper layer */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    e = errno;

    va_start(ap, fmt);
    rc  = el_printv(level, file, num, fmt, ap);
    rc |= el_print(level, file, num,
        "errno num: %lu, strerror: %s", e, strerror(e));
    va_end(ap);
}


/* ==========================================================================
    simply calls el_printv with '...' converted to 'va_list'
   ========================================================================== */


int el_print
(
    enum el_level  level,                /* log level to print message with */
    const char    *file,                 /* file name where log is printed */
    size_t         num,                  /* line number where log is printed */
    const char    *fmt,                  /* message format (see printf (3)) */
                   ...                   /* additional parameters for fmt */
)
{
    va_list        ap;                   /* arguments '...' for 'fmt' */
    int            rc;                   /* return code from el_printfv() */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    va_start(ap, fmt);
    rc = el_printv(level, file, num, fmt, ap);
    va_end(ap);

    return rc;
}



