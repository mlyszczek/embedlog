/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================

   ------------------------------------------------------------
  / This module handles all el_print family functions, this is \
  | the place where message log is constructed, colors, file   |
  | name and timstamp are added here if applicable. After      |
  | processing ready string is sent to el_puts to send it to   |
  \ apropriate output facility.                                /
   ------------------------------------------------------------
    \                                ,+*^^*+___+++_
     \                         ,*^^^^              )
      \                     _+*                     ^**+_
       \                  +^       _ _++*+_+++_,         )
              _+^^*+_    (     ,+*^ ^          \+_        )
             {       )  (    ,(    ,_+--+--,      ^)      ^\
            { (@)    } f   ,(  ,+-^ __*_*_  ^^\_   ^\       )
           {:;-/    (_+*-+^^^^^+*+*<_ _++_)_    )    )      /
          ( /  (    (        ,___    ^*+_+* )   <    <      \
           U _/     )    *--<  ) ^\-----++__)   )    )       )
            (      )  _(^)^^))  )  )\^^^^^))^*+/    /       /
          (      /  (_))_^)) )  )  ))^^^^^))^^^)__/     +^^
         (     ,/    (^))^))  )  ) ))^^^^^^^))^^)       _)
          *+__+*       (_))^)  ) ) ))^^^^^^))^^^^^)____*^
          \             \_)^)_)) ))^^^^^^^^^^))^^^^)
           (_             ^\__^^^^^^^^^^^^))^^^^^^^)
             ^\___            ^\__^^^^^^))^^^^^^^^)\\
                  ^^^^^\uuu/^^\uuu/^^^^\^\^\^\^\^\^\^\
                     ___) >____) >___   ^\_\_\_\_\_\_\)
                    ^^^//\\_^^//\\_^       ^(\_\_\_\)
                      ^^^ ^^ ^^^ ^
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "config-priv.h"
#include "el-options.h"
#include "embedlog.h"
#include "valid.h"

#if ENABLE_TIMESTAMP
#include <time.h>
#endif

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/* ==========================================================================
                                   _                __
                     ____   _____ (_)_   __ ____ _ / /_ ___
                    / __ \ / ___// /| | / // __ `// __// _ \
                   / /_/ // /   / / | |/ // /_/ // /_ /  __/
                  / .___//_/   /_/  |___/ \__,_/ \__/ \___/
                 /_/
                                   _         __     __
              _   __ ____ _ _____ (_)____ _ / /_   / /___   _____
             | | / // __ `// ___// // __ `// __ \ / // _ \ / ___/
             | |/ // /_/ // /   / // /_/ // /_/ // //  __/(__  )
             |___/ \__,_//_/   /_/ \__,_//_.___//_/ \___//____/

   ========================================================================== */


static const char char_level[8] = { 'f', 'a', 'c', 'e', 'w', 'n', 'i', 'd' };


#if ENABLE_COLORS

/*
 * colors indexes are synced with log level
 */


static const char *color[] =
{
    "\e[91m",  /* fatal             light red */
    "\e[31m",  /* alert             red */
    "\e[95m",  /* critical          light magenta */
    "\e[35m",  /* error             magenta */
    "\e[93m",  /* warning           light yellow */
    "\e[92m",  /* notice            light green */
    "\e[32m",  /* information       green */
    "\e[34m",  /* debug             blue */
    "\e[0m"    /* remove all formats */
};

#endif


/* ==========================================================================
                                   _                __
                     ____   _____ (_)_   __ ____ _ / /_ ___
                    / __ \ / ___// /| | / // __ `// __// _ \
                   / /_/ // /   / / | |/ // /_/ // /_ /  __/
                  / .___//_/   /_/  |___/ \__,_/ \__/ \___/
                 /_/
               ____                     __   _
              / __/__  __ ____   _____ / /_ (_)____   ____   _____
             / /_ / / / // __ \ / ___// __// // __ \ / __ \ / ___/
            / __// /_/ // / / // /__ / /_ / // /_/ // / / /(__  )
           /_/   \__,_//_/ /_/ \___/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


/* ==========================================================================
    adds color information to 'buf' based on  'level'.   Returns  number  of
    bytes stored in buf.  If colors are disabled, function will return 0 and
    nothing will be stored int 'buf'.

    color can be one of log levels passed directly, or int  value  8,  which
    will reset colors.
   ========================================================================== */


static size_t el_color
(
    struct el_options  *options,  /* options defining printing style */
    char               *buf,      /* buffer where to store color info */
    int                 level     /* log level or 8 for reset */
)
{
#if ENABLE_COLORS
    if (options->colors == 0)
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

#if ENABLE_CLOCK

static void el_ts_clock
(
    time_t   *s,   /* seconds will be stored here */
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

#endif /* ENABLE_CLOCK */

/* ==========================================================================
    returns seconds and microseconds calculated from time() function.
   ========================================================================== */


static void el_ts_time
(
    time_t  *s,  /* seconds will be stored here */
    long    *us  /* microseconds will be stored here */
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
    time_t         *s,     /* seconds will be stored here */
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
    struct el_options  *options,  /* options defining printing style */
    char               *buf       /* buffer where timestamp will be stored */
)
{
#if ENABLE_TIMESTAMP
    time_t           s;        /* timestamp seconds */
    long             us;       /* timestamp microseconds */
    size_t           tl;       /* timestamp length */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if (options->timestamp == EL_TS_OFF)
    {
        /*
         * user doesn't want us to print timestamp, that's fine
         */

        return 0;
    }

    /*
     * first we get seconds and microseconds from proper timer
     */

    switch (options->timestamp_timer)
    {
#if ENABLE_REALTIME

    case EL_TS_TM_REALTIME:
        el_ts_clock_gettime(&s, &us, CLOCK_REALTIME);
        break;

#endif

#if ENABLE_MONOTONIC

    case EL_TS_TM_MONOTONIC:
        el_ts_clock_gettime(&s, &us, CLOCK_MONOTONIC);
        break;

#endif

    case EL_TS_TM_TIME:
        el_ts_time(&s, &us);
        break;

#if ENABLE_CLOCK

    case EL_TS_TM_CLOCK:
        el_ts_clock(&s, &us);
        break;

#endif

    default:
        /*
         * bad timer means no timer
         */

        return 0;
    }

    /*
     * then convert retrieved time into string timestamp
     */

    if (options->timestamp == EL_TS_LONG)
    {
        struct tm   tm;   /* timestamp splitted */
        struct tm  *tmp;  /* timestamp splitted pointer */
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#   if ENABLE_REENTRANT
        tmp = gmtime_r(&s, &tm);
#   else
        tmp = gmtime(&s);
#   endif

        tl = strftime(buf, 21, "[%Y-%m-%d %H:%M:%S", tmp);
    }
    else
    {
#ifdef LLONG_MAX
        tl = sprintf(buf, "[%lld", (long long)s);
#else
        /*
         * if long long is not available, code may be suscible to 2038  bug.
         * If you are sure your compiler does support long  long  type,  but
         * doesn't define LLONG_MAX, define this value  yourself  to  enable
         * long long.
         */
        tl = sprintf(buf, "[%ld", (long)s);
#endif
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
    struct el_options  *options,  /* options defining printing style */
    char               *buf,      /* location whre to store file information */
    const char         *file,     /* path to file - will be basenamed */
    int                 num       /* line number (max 99999) */
)
{
#if ENABLE_FINFO
    const char      *base;     /* basenem of the 'file' */
    size_t           fl;       /* number of bytes stored in 'buf' */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if (options->finfo == 0 || file == NULL || num == 0)
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
                                        __     __ _
                         ____   __  __ / /_   / /(_)_____
                        / __ \ / / / // __ \ / // // ___/
                       / /_/ // /_/ // /_/ // // // /__
                      / .___/ \__,_//_.___//_//_/ \___/
                     /_/
               ____                     __   _
              / __/__  __ ____   _____ / /_ (_)____   ____   _____
             / /_ / / / // __ \ / ___// __// // __ \ / __ \ / ___/
            / __// /_/ // / / // /__ / /_ / // /_/ // / / /(__  )
           /_/   \__,_//_/ /_/ \___/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


/* ==========================================================================
    simply calls el_printv with '...' converted to 'va_list'
   ========================================================================== */


int el_print
(
    const char    *file,                 /* file name where log is printed */
    size_t         num,                  /* line number where log is printed */
    enum el_level  level,                /* log level to print message with */
    const char    *fmt,                  /* message format (see printf (3)) */
                   ...                   /* additional parameters for fmt */
)
{
    va_list        ap;                   /* arguments '...' for 'fmt' */
    int            rc;                   /* return code from el_printfv() */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    va_start(ap, fmt);
    rc = el_ovprint(file, num, level, &g_options, fmt, ap);
    va_end(ap);

    return rc;
}


/* ==========================================================================
    el_print but with custom options
   ========================================================================== */


int el_oprint
(
    const char         *file,                 /* file name to print in log */
    size_t              num,                  /* line number to print in log */
    enum el_level       level,                /* log level to print log with */
    struct el_options  *options,              /* printing style options */
    const char         *fmt,                  /* message format (man printf) */
                        ...                   /* additional params for fmt */
)
{
    va_list          ap;                   /* arguments '...' for 'fmt' */
    int              rc;                   /* return code from el_printfv() */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    va_start(ap, fmt);
    rc = el_ovprint(file, num, level, options, fmt, ap);
    va_end(ap);

    return rc;
}


/* ==========================================================================
    el_print but accepts variadic argument list object instead of '...'
   ========================================================================== */


int el_vprint
(
    const char    *file,   /* file name where log is printed */
    size_t         num,    /* line number where log is printed */
    enum el_level  level,  /* log level to print message with */
    const char    *fmt,    /* message format (see printf (3)) */
    va_list        ap      /* additional parameters for fmt */
)
{
    return el_ovprint(file, num, level, &g_options, fmt, ap);
}


/* ==========================================================================
    Prints message formated by 'fmt' and 'ap'  with  timestamp,  'file'  and
    line number 'num' information, with specified  'level'  into  configured
    outputs.  Function allocates on  callers  stack  EL_BUF_MAX  of  memory.
    If log message is longer than available buffer,  it  will  be  truncated
    and part of message will be lost.  Additionally options may be passed to
    tune printing style in runtime

    errno:
            EINVAL      level is invalid
            EINVAL      fmt is NULL
            ERANGE      printing is disabled
            ENOBUFS     message was too long and has been truncated
   ========================================================================== */


int el_ovprint
(
    const char         *file,                 /* file name to print in log */
    size_t              num,                  /* line number to print in log */
    enum el_level       level,                /* log level to print log with */
    struct el_options  *options,              /* options defining print style */
    const char         *fmt,                  /* message format (man printf) */
    va_list             ap                    /* additional params for fmt */
)
{
    char                buf[EL_BUF_MAX + 2];  /* buffer for message to print */
    size_t              w;                    /* bytes written to buf */
    size_t              flen;                 /* length of the fmt output */
    int                 e;                    /* error code */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    VALID(EINVAL, fmt);
    VALID(ERANGE, el_log_allowed(options, level));
    VALID(ENODEV, options->outputs);

    e = 0;

    if (level > EL_DBG)
    {
        /*
         * level is larger than predefined and  we  don't  have  colors  for
         * those log levels, so we force colors to be of EL_DBG level, since
         * every level larger than EL_DBG is threaded as more verbose  debug
         * anyway.
         */

        level = EL_DBG;
    }

    /*
     * add preamble and colors to log line buf
     */

    w  = el_color(options, buf, level);
    w += el_timestamp(options, buf + w);
    w += el_finfo(options, buf + w, file, num);

    if (w != 0 && buf[w - 1] == ']')
    {
        /*
         * at least one preamble has been added, add space beetween current
         * preamble and log level
         */

        *(buf + w) = ' ';
        ++w;
    }

    if (options->print_log_level)
    {
        w += sprintf(buf + w, "%c/", char_level[level]);
    }

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

    w += el_color(options, buf + w, 8 /* reset colors */);

    /*
     * make sure buf is always null terminated and contains new line character
     */

    buf[w++] = '\n';
    buf[w++] = '\0';

    if (el_oputs(options, buf) != 0)
    {
        return -1;
    }

    if (e)
    {
        errno = e;
        return -1;
    }

    return 0;
}
