/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================

   ------------------------------------------------------------
  / This file contains functions to put string into specified  \
  | output. String are not processed here, nothing is added or |
  \ removed from passed string, even log level is not checked! /
   ------------------------------------------------------------
   \     /\  ___  /\
    \   // \/   \/ \\
       ((    O O    ))
        \\ /     \ //
         \/  | |  \/
          |  | |  |
          |  | |  |
          |   o   |
          | |   | |
          |m|   |m|
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "el-private.h"

#include <errno.h>
#include <stdio.h>


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
    puts string 's' to all enabled output facilities  specified  by  default
    options
   ========================================================================== */


int el_puts
(
    const char  *s  /* string to put into output */
)
{
    return el_oputs(&g_options, s);
}


/* ==========================================================================
    puts string 's' to all enabled output facilities specified by options
   ========================================================================== */


int el_oputs
(
    struct el_options  *options,   /* options defining printing style */
    const char         *s          /* string to put into output */
)
{
    int                 rv;        /* return value from function */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    VALID(EINVAL, s);
    VALID(EINVAL, options);
    VALID(ENODEV, options->outputs != 0);

    rv = 0;

#if ENABLE_OUT_STDERR
    if (options->outputs & EL_OUT_STDERR)
    {
        rv |= fputs(s, stderr) == EOF ? -1 : 0;
    }
#endif

#if ENABLE_OUT_SYSLOG
    if (options->outputs & EL_OUT_SYSLOG)
    {
        syslog(options->level, s);
    }
#endif

#if ENABLE_OUT_FILE
    if (options->outputs & EL_OUT_FILE)
    {
        rv |= el_file_puts(options, s);
    }
#endif

#if 0
    if (options->outputs & EL_OUT_NET)
    {
        el_puts_net(s);
    }
#endif

#if ENABLE_OUT_TTY
    if (options->outputs & EL_OUT_TTY)
    {
        rv |= el_tty_puts(options, s);
    }
#endif

#if ENABLE_OUT_CUSTOM
    if (options->outputs & EL_OUT_CUSTOM && options->custom_puts)
    {
        rv |= options->custom_puts(s);
    }
#endif

    return rv;
}


/* ==========================================================================
    puts memory 'mem'  to  all  supported  output  facilities  specified  by
    default options. Not all outputs support printing binary data
   ========================================================================== */


int el_putb
(
    const void         *mem,  /* memory location to 'print' */
    size_t              mlen  /* size of the mem buffer */
)
{
    return el_oputb(&g_options, mem, mlen);
}


/* ==========================================================================
    Puts binary data 'mem' of size 'mlen' to enabled output facilities
    specified by 'options'. No all outputs support printing binary data
   ========================================================================== */


int el_oputb
(
    struct el_options  *options,   /* options defining printing style */
    const void         *mem,       /* memory location to 'print' */
    size_t              mlen       /* size of the mem buffer */
)
{
    int                 rv;        /* return value from function */
    int                 called;    /* at least one output was called */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    VALID(EINVAL, mem);
    VALID(EINVAL, mlen);
    VALID(EINVAL, options);
    VALID(ENODEV, options->outputs != 0);

    rv = 0;
    called = 0;

#if ENABLE_OUT_FILE
    if (options->outputs & EL_OUT_FILE)
    {
        called = 1;
        rv |= el_file_putb(options, mem, mlen);
    }
#endif

    if (called == 0)
    {
        /*
         * couldn't find any supported output for binary logging
         */

        errno = ENODEV;
        return -1;
    }

    return rv;
}
