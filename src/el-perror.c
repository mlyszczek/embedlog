/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================

   -----------------------------------------------------------
  / This module handles printing messages with standard errno \
  \ message, no magic to be found here                        /
   -----------------------------------------------------------
         \   ,__,
          \  (oo)____
             (__)    )\
                ||--|| *
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include <errno.h>
#include <stdarg.h>
#include <string.h>

#include "embedlog.h"
#include "el-options.h"


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
    calls el_print with 'fmt' and '...' parameters, but  additionaly  prints
    information about errno.  Functionaly it is similar to  perror  function
   ========================================================================== */


int el_perror
(
    enum el_level  level,                /* log level to print message with */
    const char    *file,                 /* file name where log is printed */
    size_t         num,                  /* line number where log is printed */
    const char    *fmt,                  /* message format (see printf (3)) */
                   ...                   /* additional parameters for fmt */
)
{
    va_list        ap;                   /* arguments '...' for 'fmt' */
    int            rc;                   /* return code from el_print() */
    unsigned long  e;                    /* errno from upper layer */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    e = errno;

    va_start(ap, fmt);
    rc  = el_ovprint(level, file, num, &g_options, fmt, ap);
    rc |= el_oprint(level, file, num, &g_options,
        "errno num: %lu, strerror: %s", e, strerror(e));
    va_end(ap);
}


/* ==========================================================================
    el_perror function with custom options
   ========================================================================== */


int el_operror
(
    enum el_level      level,     /* log level to print message with */
    const char        *file,      /* file name where log is printed */
    size_t             num,       /* line number where log is printed*/
    struct el_options  *options,  /* options defining printing style */
    const char        *fmt,       /* message format (see printf (3)) */
                       ...        /* additional parameters for fmt */
)
{
    va_list            ap;        /* arguments '...' for 'fmt' */
    int                rc;        /* return code from el_print() */
    unsigned long      e;         /* errno from upper layer */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    e = errno;

    va_start(ap, fmt);
    rc  = el_ovprint(level, file, num, options, fmt, ap);
    rc |= el_oprint(level, file, num, options,
        "errno num: %lu, strerror: %s", e, strerror(e));
    va_end(ap);
}
