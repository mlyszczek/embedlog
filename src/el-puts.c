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


#include <stdio.h>

#include "el-file.h"
#include "embedlog.h"
#include "options.h"


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
    struct el_options  *options,  /* options defining printing style */
    const char         *s         /* string to put into output */
)
{
#if ENABLE_OUT_STDERR
    if (options->outputs & EL_OUT_STDERR)
    {
        return fputs(s, stderr) == EOF ? -1 : 0;
    }
#endif

#if 0 /* TODO */
    if (options->outputs & EL_OUT_SYSLOG)
    {
        el_puts_syslog(s);
    }
#endif

#if ENABLE_OUT_FILE
    if (options->outputs & EL_OUT_FILE)
    {
        return el_file_puts(options, s);
    }
#endif

#if 0
    if (options->outputs & EL_OUT_NET)
    {
        el_puts_net(s);
    }

    if (options->outputs & EL_OUT_TTY)
    {
        el_puts_tty(s);
    }
#endif

    return 0;
}
