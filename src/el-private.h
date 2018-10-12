/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

/* this file contains stuff internal to the library
 *
 * !!!!!!! FUCKING IMPORTANT NOTICE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !!!
 * !!! MAKE SURE THIS FILE IS INCLUDED FIRST IN ALL SOURCE AND TEST FILES
 * !!! OR THINGS WILL GO WEIRD.
 * !!!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *
 * it's becuase of feature test macros that are defined here and they must
 * be defined before any system files are included
 */

#ifndef EL_PRIVATE_H
#define EL_PRIVATE_H 1


/* ==========================================================================
      ____              __                        __               __
     / __/___   ____ _ / /_ __  __ _____ ___     / /_ ___   _____ / /_ _____
    / /_ / _ \ / __ `// __// / / // ___// _ \   / __// _ \ / ___// __// ___/
   / __//  __// /_/ // /_ / /_/ // /   /  __/  / /_ /  __/(__  )/ /_ (__  )
  /_/   \___/ \__,_/ \__/ \__,_//_/    \___/   \__/ \___//____/ \__//____/

   ========================================================================== */


#if HAVE_CONFIG_H
#   include "config.h"
#endif

#if PREFER_PORTABLE_SNPRINTF || NEED_SNPRINTF_ONLY
    /* in case we use portable sprintf.c, declare function we use,
     * so compiler don't scream at us
     */

#   include <stddef.h>
#   include <stdarg.h>

    int snprintf(char *str, size_t str_m, const char *fmt, ...);
    int vsnprintf(char *str, size_t str_m, const char *fmt, va_list ap);
#else
    /* if portable snprintf is not used, define posix source to include
     * snprintf() and vsnprintf() functions
     */
#   define _POSIX_C_SOURCE 200112L
#endif

/* features definitions, order must be from higher posix standard to
 * lowest one to avoid redefinitions
 */

/* clock_gettime() was defined in 199303 issue of posix
 * but some systems need posix = 200112 for the feature
 */
#if ENABLE_TIMESTAMP
#   if ENABLE_REALTIME || ENABLE_MONOTONIC
#       if __FreeBSD__ || __QNX__ || __QNXNTO__
#           ifndef _POSIX_C_SOURCE
#           define _POSIX_C_SOURCE 200112L
#           endif
#       endif
#   endif
#endif

/* gmtime_r() was defined in posix issue 1, but these systems
 * define them in issue 199506
 */
#if ENABLE_TIMESTAMP
#   if __DragonFly__
#       ifndef _POSIX_C_SOURCE
#       define _POSIX_C_SOURCE 199506L
#       endif
#   endif
#endif

/* clock_gettime() was defined in 199303 issue of posix
 */
#if ENABLE_TIMESTAMP
#   if ENABLE_REALTIME || ENABLE_MONOTONIC
#       ifndef _POSIX_C_SOURCE
#       define _POSIX_C_SOURCE 199309L
#       endif
#   endif
#endif

/* gmtime_r() was defined in posix issue 1
 */
#if ENABLE_TIMESTAMP
#   ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 1
#   endif
#endif

/* both fsync() and fileno() functions were defined in
 * posix issue 1
 */
#if ENABLE_OUT_FILE
#   if HAVE_FSYNC && HAVE_FILENO
#       ifndef _POSIX_C_SOURCE
#       define _POSIX_C_SOURCE 1
#       endif
#   endif
#endif

/* ONLCR flag on FreeBSD needs __BSD_VISIBLE defined and solaris
 * need __EXTENSIONS__
 */
#if ENABLE_OUT_TTY
#   if HAVE_TERMIOS_H
#       if __FreeBSD__
#           ifndef __BSD_VISIBLE
#           define __BSD_VISIBLE 1
#           endif
#       elif sun || __sun
#           ifndef __EXTENSIONS__
#           define __EXTENSIONS__ 1
#           endif
#       endif
#   endif
#endif


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "embedlog.h"
#include "valid.h"

#if HAVE_TERMIOS_H
#   include <termios.h>
#endif

#include <limits.h>


/* ==========================================================================
                  __        __            __
          ____ _ / /____   / /_   ____ _ / /  _   __ ____ _ _____ _____
         / __ `// // __ \ / __ \ / __ `// /  | | / // __ `// ___// ___/
        / /_/ // // /_/ // /_/ // /_/ // /   | |/ // /_/ // /   (__  )
        \__, //_/ \____//_.___/ \__,_//_/    |___/ \__,_//_/   /____/
       /____/
   ========================================================================== */


extern struct el_options g_options;


/* ==========================================================================
                                       __                 __
            _____ ____   ____   _____ / /_ ____ _ ____   / /_ _____
           / ___// __ \ / __ \ / ___// __// __ `// __ \ / __// ___/
          / /__ / /_/ // / / /(__  )/ /_ / /_/ // / / // /_ (__  )
          \___/ \____//_/ /_//____/ \__/ \__,_//_/ /_/ \__//____/

   ========================================================================== */


/* ==== defines for el_pmemory function ===================================== */


/* ==========================================================================
    single line of el_pmemory will be similar to this

    0xNNNN  HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH  CCCCCCCCCCCCCCC
   ========================================================================== */


/* ==========================================================================
    this defines length of "0xNNNN  " part.
   ========================================================================== */


#define EL_MEM_OFFSET_LEN 8


/* ==========================================================================
    length of the "HH " part
   ========================================================================== */


#define EL_MEM_SINGLE_HEX_LEN 3


/* ==========================================================================
    length of all "HH ".   EL_MEM_LINE_SIZE  is  externally  defined  during
    compilation to tune the output
   ========================================================================== */


#define EL_MEM_HEX_LEN (EL_MEM_SINGLE_HEX_LEN * (EL_MEM_LINE_SIZE))


/* ==========================================================================
    length of all "C" characters
   ========================================================================== */


#define EL_MEM_CHAR_LEN (EL_MEM_LINE_SIZE)


/* ==== defines for el_print function ======================================= */


/* ==========================================================================
    length of long timestamp in a single log. Timestamp format is

        [yyyy-mm-dd hh:mm:ss]

    which is 21 bytes long
   ========================================================================== */


#define EL_PRE_TS_LONG_LEN 21


/* ==========================================================================
    length of short timestamp in a single log. Timestamp format is

        [ssssssssss]

    which is 12 bytes long. This is maximum value for short timestamp, as it
    can be shorter.
   ========================================================================== */


#define EL_PRE_TS_SHORT_LEN 12


/* ==========================================================================
    Size of the fractions of seconds, that is part after seconds like:

    [ssssssssss.fffffffff]

    or

    [yyyy-mm-dd hh:mm:ss.fffffffff]
   ========================================================================== */


#define EL_PRE_TS_FRACT_LEN 10


/* ==========================================================================
    Calculate what is the minimum needed length to hold longest timestamp
   ========================================================================== */


#if ENABLE_TIMESTAMP
#   define EL_PRE_TS_MAX (EL_PRE_TS_LONG_LEN + EL_PRE_TS_FRACT_LEN)
#else
#   define EL_PRE_TS_MAX 0
#endif


/* ==========================================================================
    Maximum length of line number.  ie, if set to 1, range 0-9  is  allowed,
    rendering  [source-file.c:5]  valid  while  [source-file.c:13]  invalid.
   ========================================================================== */


#define EL_PRE_FINFO_LINE_MAX_LEN 7


/* ==========================================================================
    maximum file info length. File info is a part with file name and line
    number, it looks like this

        [source-file.c:1337]

    EL_FLEN_MAX is a maximum length of a  file  and  is  defined  externally
    during  compilation  to  suite   users   needs,   3   is   for   special
    characters "[:]".  EL_PRE_FINFO_LINE_MAX_LEN can  be  redefined  to  any
    value.
   ========================================================================== */


#if ENABLE_FINFO
#   define EL_PRE_FINFO_LEN ((EL_FLEN_MAX) + 3 + EL_PRE_FINFO_LINE_MAX_LEN)
#else
#   define EL_PRE_FINFO_LEN 0
#endif


/* ==========================================================================
    log level preamble string length. Preamble is always "c/" where c is one
    of 'e, w, i, d" depending on the log level. 3 characters are reserved in
    a situation where another preamble (like timestamp)  is  printed  before
    level, and we need to delimiter them with space
   ========================================================================== */


#define EL_PRE_LEVEL_LEN 3


/* ==========================================================================
    maximum length of preamble which may look like this

        [2017-05-24 14:43:10.123456][source-file.c:12345] e/prefix
   ========================================================================== */


#if ENABLE_PREFIX
#   define EL_PREFIX_LEN EL_PREFIX_MAX
#else
#   define EL_PREFIX_LEN 0
#endif

#define EL_PRE_LEN (EL_PRE_TS_MAX + EL_PRE_FINFO_LEN + EL_PREFIX_LEN + \
    EL_PRE_LEVEL_LEN)


/* ==========================================================================
    defines length needed to put color information.  5 chars are  needed  to
    start coloring output, and 4 characters  are  neede  to  reset  terminal
    color to default
   ========================================================================== */


#if ENABLE_COLORS
#   define EL_COLORS_LEN (5 + 4)
#else
#   define EL_COLORS_LEN 0
#endif


/* ==========================================================================
    maximum buffer length excluding  new  line  and  null  character.   This
    defines length of final output not format.  So "%s" can  take  even  100
    bytes.    EL_LOG_MAX   is   defined   externally   during   compilation.
   ========================================================================== */


#define EL_BUF_MAX (EL_PRE_LEN + (EL_LOG_MAX) + EL_COLORS_LEN)


/* ==========================================================================
    Defines if timestamp should be stored into buffer as string or binary
   ========================================================================== */


#define TS_STRING (0)
#define TS_BINARY (1)


/* ==========================================================================
               ____                     __   _
              / __/__  __ ____   _____ / /_ (_)____   ____   _____
             / /_ / / / // __ \ / ___// __// // __ \ / __ \ / ___/
            / __// /_/ // / / // /__ / /_ / // /_/ // / / /(__  )
           /_/   \__,_//_/ /_/ \___/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


#if ENABLE_OUT_FILE
int el_file_open(struct el_options *options);
int el_file_puts(struct el_options *options, const char *s);
int el_file_putb(struct el_options *options, const void *mem, size_t mlen);
void el_file_cleanup(struct el_options *options);
#endif


#if ENABLE_OUT_TTY
int el_tty_open(struct el_options *options, const char *dev, unsigned int speed);
int el_tty_puts(struct el_options *options, const char *s);
int el_tty_close(struct el_options *options);
#endif

int el_log_allowed(struct el_options *options, enum el_level level);
size_t el_timestamp(struct el_options *options, void *buf, int binary);

#if LLONG_MAX
size_t el_encode_number(unsigned long long number, void *out);
size_t el_decode_number(const void *number, unsigned long long *out);
#else
size_t el_encode_number(unsigned long number, void *out);
size_t el_decode_number(const void *number, unsigned long *out);
#endif


#endif
