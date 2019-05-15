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
   ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "el-private.h"

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/* ==========================================================================
          __             __                     __   _
     ____/ /___   _____ / /____ _ _____ ____ _ / /_ (_)____   ____   _____
    / __  // _ \ / ___// // __ `// ___// __ `// __// // __ \ / __ \ / ___/
   / /_/ //  __// /__ / // /_/ // /   / /_/ // /_ / // /_/ // / / /(__  )
   \__,_/ \___/ \___//_/ \__,_//_/    \__,_/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


static const char char_level[8] = { 'f', 'a', 'c', 'e', 'w', 'n', 'i', 'd' };


#if ENABLE_COLORS

/* colors indexes are synced with log level
 */

#if ENABLE_COLORS_EXTENDED

/* for those that like more colors, there are definitions with more
 * colors! this will enable light version of some levels, but this
 * is not supported on all terminal! You have been warned!
 */

static const char *color[] =
{
    "\033[91m",  /* fatal             light red */
    "\033[31m",  /* alert             red */
    "\033[95m",  /* critical          light magenta */
    "\033[35m",  /* error             magenta */
    "\033[93m",  /* warning           light yellow */
    "\033[92m",  /* notice            light green */
    "\033[32m",  /* information       green */
    "\033[34m",  /* debug             blue */
    "\033[0m"    /* remove all formats */
};

#else

/* not all terminal can support extended colors with light version
 * of them, for those who want to be more standard compliant there
 * is this shortened version of colors. On downside is that some
 * level will have same colors.
 */

static const char *color[] =
{
    "\033[31m",  /* fatal             red */
    "\033[31m",  /* alert             red */
    "\033[35m",  /* critical          magenta */
    "\033[35m",  /* error             magenta */
    "\033[33m",  /* warning           yellow */
    "\033[32m",  /* notice            green */
    "\033[32m",  /* information       green */
    "\033[34m",  /* debug             blue */
    "\033[0m"    /* remove all formats */
};

#endif /* COLORS_EXTENDED */

#endif /* ENABLE_COLORS */


/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ========================================================================== */


/* ==========================================================================
    adds color information to 'buf' based on 'level'. Returns number of
    bytes stored in buf. If colors are disabled, function will return 0 and
    nothing will be stored int 'buf'.

    color can be one of log levels passed directly, or int value 8, which
    will reset colors.
   ========================================================================== */


static size_t el_color
(
    struct el  *el,    /* el defining printing style */
    char       *buf,   /* buffer where to store color info */
    int         level  /* log level or 8 for reset */
)
{
#if ENABLE_COLORS

    if (el->colors == 0)
    {
        /*
         * no colors, you got it!
         */

        return 0;
    }

    strcpy(buf, color[level]);
    return strlen(color[level]);

#else

    (void)el;
    (void)buf;
    (void)level;
    return 0;

#endif
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
    const char  *base;  /* pointer to base name of path */
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
    stores file and line information in 'buf'. Number of bytes stored in
    'buf' is returned. If file info is disabled during compilation or in
    runtime, value 0 is returned
   ========================================================================== */


static size_t el_finfo
(
    struct el    *el,    /* el defining printing style */
    char         *buf,   /* location whre to store file information */
    const char   *file,  /* path to file - will be basenamed */
    int           num    /* line number (max 99999) */
)
{
#if ENABLE_FINFO
    const char   *base;  /* basenem of the 'file' */
    size_t        fl;    /* number of bytes stored in 'buf' */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if (el->finfo == 0 || file == NULL || num == 0)
    {
        /* no file info for this caller
         */

        return 0;
    }

    base = el_basename(file);

    buf[0] = '[';
    buf[1] = '\0';
    strncat(buf, base, EL_PRE_FINFO_LEN);
    fl  = strlen(buf);
    fl += sprintf(buf + fl, ":%d", num);

    if (el->funcinfo == 0)
    {
        /* when function info is printed, we don't close ']', to
         * let it print function in same [] brackets, we close it
         * only when there is no finfo
         */

        buf[fl++] = ']';
    }

    return fl;

#else
    return 0;
#endif /* ENABLE_FINFO */
}


/* ==========================================================================
    Stores function name with appended "()" into 'buf'.
   ========================================================================== */


static size_t el_funcinfo
(
    struct el   *el,   /* el defining printing style */
    char        *buf,  /* location whre to store file information */
    const char  *func  /* function name to print */
)
{
#if ENABLE_FUNCINFO

    size_t       fl;   /* number of bytes stored in 'buf' */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if (el->funcinfo == 0 || func == NULL)
    {
        /* no function print for this caller
         */

        return 0;
    }

    /* if finfo is enabled, we need to add ':' to delimit ourselfs
     * from fifno, but, if there is no file info, we need to open
     * '[' bracket, as it was not already opened by finfo
     */

    buf[0] = el->finfo ? ':' : '[';

    /* copy function name to buffer
     */

    strncpy(buf + 1, func, EL_FUNCLEN_MAX);

    /* in case func is too big, nullify EL_FUNCLEN_MAXth character,
     * to prevent strlen showing bad results, 1 is because of
     * previous ':'/'[' characters
     */

    buf[1 + EL_FUNCLEN_MAX] = '\0';
    fl = strlen(buf);

    /* and add leading "()" to indicate it is a function name
     */

    buf[fl++] = '(';
    buf[fl++] = ')';

    /* close ']' bracket, it doesn't matter if finfo is enabled or
     * not, we print always print last so it's our responsibility
     * to close it
     */

    buf[fl++] = ']';
    return fl;

#else

    (void)el;
    (void)buf;
    (void)func;
    return 0;

#endif
}


/* ==========================================================================
                       __     __ _          ____
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ========================================================================== */


/* ==========================================================================
    simply calls el_printv with '...' converted to 'va_list'
   ========================================================================== */


int el_print
(
    const char    *file,   /* file name where log is printed */
    size_t         num,    /* line number where log is printed */
    const char    *func,   /* function name to print */
    enum el_level  level,  /* log level to print message with */
    const char    *fmt,    /* message format (see printf (3)) */
                   ...     /* additional parameters for fmt */
)
{
    va_list        ap;     /* arguments '...' for 'fmt' */
    int            rc;     /* return code from el_printfv() */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    va_start(ap, fmt);
    rc = el_ovprint(file, num, func, level, &g_el, fmt, ap);
    va_end(ap);

    return rc;
}


/* ==========================================================================
    el_print but with custom el
   ========================================================================== */


int el_oprint
(
    const char    *file,   /* file name to print in log */
    size_t         num,    /* line number to print in log */
    const char    *func,   /* function name to print */
    enum el_level  level,  /* log level to print log with */
    struct el     *el,     /* printing style el */
    const char    *fmt,    /* message format (man printf) */
                   ...     /* additional params for fmt */
)
{
    va_list        ap;     /* arguments '...' for 'fmt' */
    int            rc;     /* return code from el_printfv() */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    va_start(ap, fmt);
    rc = el_ovprint(file, num, func, level, el, fmt, ap);
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
    const char    *func,   /* function name to print */
    enum el_level  level,  /* log level to print message with */
    const char    *fmt,    /* message format (see printf (3)) */
    va_list        ap      /* additional parameters for fmt */
)
{
    return el_ovprint(file, num, func, level, &g_el, fmt, ap);
}


/* ==========================================================================
    Prints message formated by 'fmt' and 'ap' with timestamp, 'file' and
    line number 'num' information, with specified 'level' into configured
    outputs. Function allocates on callers stack EL_BUF_MAX of memory. If
    log message is longer than available buffer, it will be truncated and
    part of message will be lost. Additionally el may be passed to tune
    printing style in runtime

    errno:
            EINVAL      level is invalid
            EINVAL      fmt is NULL
            ERANGE      printing is disabled
            ENOBUFS     message was too long and has been truncated
   ========================================================================== */


int el_ovprint
(
    const char    *file,                 /* file name to print in log */
    size_t         num,                  /* line number to print in log */
    const char    *func,                 /* function name to print */
    enum el_level  level,                /* log level to print log with */
    struct el     *el,                   /* el defining print style */
    const char    *fmt,                  /* message format (man printf) */
    va_list        ap                    /* additional params for fmt */
)
{
    char           buf[EL_BUF_MAX + 2];  /* buffer for message to print */
    size_t         w;                    /* bytes written to buf */
    size_t         flen;                 /* length of the fmt output */
    int            e;                    /* error code */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    VALID(EINVAL, fmt);
    VALID(EINVAL, el);
    VALID(ERANGE, el_log_allowed(el, level));
    VALID(ENODEV, el->outputs);

    e = 0;

    if (level > EL_DBG)
    {
        /* level is larger than predefined and we don't have colors
         * for those log levels, so we force colors to be of EL_DBG
         * level, since every level larger than EL_DBG is threaded
         * as more verbose debug anyway.
         */

        level = EL_DBG;
    }

    /* add preamble and colors to log line buf
     */

    buf[0] = '\0';
    w  = el_color(el, buf, level);
    w += el_timestamp(el, buf + w, TS_STRING);
    w += el_finfo(el, buf + w, file, num);
    w += el_funcinfo(el, buf + w, func);

    if (w != 0 && buf[w - 1] == ']')
    {
        /* at least one preamble has been added, add space beetween
         * current preamble and log level
         */

        *(buf + w) = ' ';
        ++w;
    }

    if (el->print_log_level)
    {
        w += sprintf(buf + w, "%c/", char_level[level]);
    }

#if ENABLE_PREFIX
    if (el->prefix)
    {
        /* there is a case where buf[w] will point to something
         * different than '\0'. This is not wrong but will confuse
         * strncat function and logs will be printed incorectly.
         */

        buf[w] = '\0';
        strncat(buf + w, el->prefix, EL_PREFIX_LEN);

        if ((flen = strlen(el->prefix)) > EL_PREFIX_LEN)
        {
            /* dodge a bullet - overflow would have occured
             */

            flen = EL_PREFIX_LEN;
        }

        w += flen;
    }
#endif

    /* add requested log from format, we add + 1 to include null
     * termination
     */

    flen = vsnprintf(buf + w, EL_LOG_MAX + 1, fmt, ap);

    if (flen > EL_LOG_MAX)
    {
        /* overflow would have occur, not all bytes have been
         * copied, output will truncated. Correct flen to number of
         * bytes actually stored in buf.
         */

        flen = EL_LOG_MAX;
        e = ENOBUFS;
    }

    w += flen;

    /* add terminal formatting reset sequence
     */

    w += el_color(el, buf + w, 8 /* reset colors */);

    if (el->print_newline)
    {
        /* add new line to log
         */

        buf[w++] = '\n';
    }

    buf[w++] = '\0';

    /* some modules (like el-file) needs to know level of message
     * they are printing, and such information may not be available
     * from string, thus we set it here in a 'object global'
     * variable
     */

    el->level_current_msg = level;

    if (el_oputs(el, buf) != 0)
    {
        el->level_current_msg = EL_DBG;
        return -1;
    }

    /* after message is printed set current messasge level to
     * debug, as next call might be using el_puts, which does not
     * contain log level, and we thread all el_puts messages as
     * they were debug. Note, it does not apply to log filtering,
     * el_puts does not filter messages, but modules like el-file,
     * will need this information to determin wheter fsync() data
     * to file or not.
     */

    el->level_current_msg = EL_DBG;

    if (e)
    {
        errno = e;
        return -1;
    }

    return 0;
}
