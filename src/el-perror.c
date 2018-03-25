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


#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "el-options.h"
#include "embedlog.h"

#include <errno.h>
#include <stdarg.h>
#include <string.h>


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
    calls el_print with 'fmt' and '...' parameters, but  additionaly  prints
    information about errno.  Functionaly it is similar to  perror  function
   ========================================================================== */


static int el_ovperror
(
    const char         *file,     /* file name where log is printed */
    size_t              num,      /* line number where log is printed */
    enum el_level       level,    /* log level to print message with */
    struct el_options  *options,  /* options defining printing style */
    const char         *fmt,      /* message format (see printf (3)) */
    va_list             ap        /* additional parameters for fmt */
)
{
    int                 rc;       /* return code from el_print() */
    unsigned long       e;        /* errno from upper layer */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    e = errno;
    rc = 0;

    if (fmt)
    {
        /*
         * we print formatted message only when it is supplied by the user,
         * otherwise only errno message will be printed
         */

        rc |= el_ovprint(file, num, level, options, fmt, ap);
    }

    rc |= el_oprint(file, num, level, options,
        "errno num: %lu, strerror: %s", e, strerror(e));

    return rc;
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
    calls el_print with 'fmt' and '...' parameters, but  additionaly  prints
    information about errno.  Functionaly it is similar to  perror  function
   ========================================================================== */


int el_perror
(
    const char    *file,   /* file name where log is printed */
    size_t         num,    /* line number where log is printed */
    enum el_level  level,  /* log level to print message with */
    const char    *fmt,    /* message format (see printf (3)) */
                   ...     /* additional parameters for fmt */
)
{
    int            rc;     /* return code from el_operror() */
    va_list        ap;     /* argument pointer for variadic variables */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    va_start(ap, fmt);
    rc = el_ovperror(file, num, level, &g_options, fmt, ap);
    va_end(ap);

    return rc;
}


/* ==========================================================================
    el_perror function with custom options
   ========================================================================== */


int el_operror
(
    const char        *file,      /* file name where log is printed */
    size_t             num,       /* line number where log is printed*/
    enum el_level      level,     /* log level to print message with */
    struct el_options  *options,  /* options defining printing style */
    const char        *fmt,       /* message format (see printf (3)) */
                       ...        /* additional parameters for fmt */
)
{
    int                rc;        /* return code from el_operror() */
    va_list            ap;     /* argument pointer for variadic variables */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    va_start(ap, fmt);
    rc = el_ovperror(file, num, level, options, fmt, ap);
    va_end(ap);

    return rc;
}
