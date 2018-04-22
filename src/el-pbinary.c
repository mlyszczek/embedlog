/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
     -------------------------------------------------------------
    / Module reponsible for printing binary logs. And by binary I \
    | don't mean only metadata and logs in ASCII, but full binary |
    | including data. This may be useful when one need to log ie. |
    | every message it saw on CAN. It would be a waste of space   |
    | to log it in ASCII when binary data can be less by an order |
    \ of magnitude                                                /
     -------------------------------------------------------------
      \
       \ ,   _ ___.--'''`--''//-,-_--_.
          \`"' ` || \\ \ \\/ / // / ,-\\`,_
         /'`  \ \ || Y  | \|/ / // / - |__ `-,
        /@"\  ` \ `\ |  | ||/ // | \/  \  `-._`-,_.,
       /  _.-. `.-\,___/\ _/|_/_\_\/|_/ |     `-._._)
       `-'``/  /  |  // \__/\__  /  \__/ \
            `-'  /-\/  | -|   \__ \   |-' |
              __/\ / _/ \/ __,-'   ) ,' _|'
             (((__/(((_.' ((___..-'((__,'
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#if HAVE_CONFIG_H
#   include "config.h"
#endif

#include "el-private.h"

#include <errno.h>
#include <string.h>
#include <time.h>


/* ==========================================================================
                  _                __           __
    ____   _____ (_)_   __ ____ _ / /_ ___     / /_ __  __ ____   ___   _____
   / __ \ / ___// /| | / // __ `// __// _ \   / __// / / // __ \ / _ \ / ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / /_ / /_/ // /_/ //  __/(__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/   \__/ \__, // .___/ \___//____/
/_/                                              /____//_/
   ========================================================================== */


#define FLAG_TS             (0x01)

#define FLAG_TS_FRACT_NONE  (0x00)
#define FLAG_TS_FRACT_NSEC  (0x01)
#define FLAG_TS_FRACT_USEC  (0x02)
#define FLAG_TS_FRACT_MSEC  (0x03)
#define FLAG_TS_FRACT_MASK  (0x03)
#define FLAG_TS_FRACT_SHIFT (1)

#define FLAG_LEVEL_MASK     (0x07)
#define FLAG_LEVEL_SHIFT    (3)


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
    Puts flags with log information to buf. Flags is single byte with fields

    bit 0       timestamp enabled/disabled
    bit 1-2     fraction timestamp (off, nsec, usec, msec)
    bit 3-5     severity of the log
    bit 6-7     reserved
   ========================================================================== */


static size_t el_flags
(
    enum el_level       level,
    struct el_options  *options,
    unsigned char      *buf
)
{
    *buf = 0;

#if ENABLE_TIMESTAMP
    if (options->timestamp != EL_TS_OFF)
    {
        *buf |= FLAG_TS;

#   if ENABLE_FRACTIONS
        /*
         * fraction of seconds can be printed only when timestamp is on
         */

        *buf |= options->timestamp_fractions << FLAG_TS_FRACT_SHIFT;
#   endif
    }
#endif

    *buf |= level << FLAG_LEVEL_SHIFT;

    return 1;
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


int el_opbinary
(
    enum el_level       level,
    struct el_options  *options,
    const void         *memory,
    size_t              mlen
)
{
    unsigned char       buf[EL_BUF_MAX];  /* buffer for message to print */
    size_t              l;                /* length of encoded mlen */
    size_t              w;                /* bytes written to buf */
    int                 e;                /* cache for errno */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    VALID(EINVAL, mlen);
    VALID(EINVAL, memory);
    VALID(EINVAL, options);
    VALID(ENODEV, options->outputs);
    VALID(ERANGE, el_log_allowed(options, level));

    e = 0;
    w  = el_flags(level, options, buf);
    w += el_timestamp(options, buf + w, TS_BINARY);

    /*
     * encode mlen to know how much bytes we are going to need
     */

    l = el_encode_number(mlen, buf + w);

    if (w + l + mlen > sizeof(buf))
    {
        /*
         * user tries to print more that than we can hold in our buffer,
         * buffer overflow attack? Not going to happen! We truncate it.
         */

        mlen = EL_BUF_MAX - w - l;
        e = ENOBUFS;
    }

    /*
     * know that we know real value of mlen, we can encode mlen again
     */

    w += el_encode_number(mlen, buf + w);
    memcpy(buf + w, memory, mlen);
    options->level_current_msg = level;
    w += mlen;

    if (el_oputb(options, buf, w) != 0)
    {
        options->level_current_msg = EL_DBG;
        return -1;
    }

    options->level_current_msg = EL_DBG;

    if (e)
    {
        errno = e;
        return -1;
    }

    return 0;
}


/* ==========================================================================
    Same as el_opbinary but uses global options object
   ========================================================================== */


int el_pbinary
(
    enum el_level       level,
    const void         *memory,
    size_t              mlen
)
{
    return el_opbinary(level, &g_options, memory, mlen);
}
