/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */
#ifndef EL_PRIVATE_H
#define EL_PRIVATE_H 1


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
#endif


/* ==========================================================================
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
          ____ _ / /____   / /_   ____ _ / /  _   __ ____ _ _____ _____
         / __ `// // __ \ / __ \ / __ `// /  | | / // __ `// ___// ___/
        / /_/ // // /_/ // /_/ // /_/ // /   | |/ // /_/ // /   (__  )
        \__, //_/ \____//_.___/ \__,_//_/    |___/ \__,_//_/   /____/
       /____/
   ========================================================================== */
extern struct el g_el;


/* ==========================================================================
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
    length of all "HH ". EL_MEM_LINE_SIZE is externally defined during
    compilation to tune the output. We add +3 for possible spacing after
    8 printed bytes.
   ========================================================================== */
#define EL_MEM_HEX_LEN (EL_MEM_SINGLE_HEX_LEN * (EL_MEM_LINE_SIZE) + 3)


/* ==========================================================================
    length of all "C" characters. We add +3 for possible spacing after
    8 printed bytes.
   ========================================================================== */
#define EL_MEM_CHAR_LEN (EL_MEM_LINE_SIZE + 3)


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
    Maximum length of line number. ie, if set to 1, range 0-9 is allowed,
    rendering [source-file.c:5] valid while [source-file.c:13] invalid.
   ========================================================================== */
#define EL_PRE_FINFO_LINE_MAX_LEN 7


/* ==========================================================================
    Numerical limit of line max, its stringified strlen() should not exceed
    EL_PRE_FINFO_LINE_MAX_LEN. So if EL_PRE_FINFO_LINE_MAX_LEN is 2, best to
    define it to 99, when 5 -> 99999.
   ========================================================================== */
#define EL_PRE_FINFO_LINE_MAX_NUM 9999999l


/* ==========================================================================
    maximum file info length. File info is a part with file name and line
    number, it looks like this

        [source-file.c:1337]

    EL_FLEN_MAX is a maximum length of a file and is defined externally
    during compilation to suite users needs, 3 is for special characters
    "[:]". EL_PRE_FINFO_LINE_MAX_LEN can be redefined to any value.
   ========================================================================== */
#if ENABLE_FINFO
#   define EL_PRE_FINFO_LEN ((EL_FLEN_MAX) + 3 + EL_PRE_FINFO_LINE_MAX_LEN)
#else
#   define EL_PRE_FINFO_LEN 0
#endif


/* ==========================================================================
    Maximum length of function field, 2 are added for appended "()" to
    indicate function. 1 is added in case function is printed with file
    info, in such case we will be adding ':' character also. This May look
    like this:

        [source-file.c:1337:foo()]

    or if finfo is disabled. Last +1 is for closing ']'

        [foo()]

    EL_FUNCLEN_MAX is defined during compilation.
   ========================================================================== */
#if ENABLE_FUNCINFO
#   define EL_PRE_FUNCINFO_LEN ((EL_FUNCLEN_MAX) + 2 + 1 + 1)
#else
#   define EL_PRE_FUNCINFO_LEN 0
#endif


/* ==========================================================================
    log level preamble string length. Preamble is always "c/" where c is one
    of 'e, w, i, d" depending on the log level. 3 characters are reserved in
    a situation where another preamble (like timestamp) is printed before
    level, and we need to delimiter them with space
   ========================================================================== */
#define EL_PRE_LEVEL_LEN 3


/* ==========================================================================
    maximum length of preamble which may look like this

        [2017-05-24 14:43:10.123456][source-file.c:12345:function()] e/prefix
   ========================================================================== */
#if ENABLE_PREFIX
#   define EL_PREFIX_LEN EL_PREFIX_MAX
#else
#   define EL_PREFIX_LEN 0
#endif

#define EL_PRE_LEN (EL_PRE_TS_MAX + EL_PRE_FINFO_LEN + EL_PRE_FUNCINFO_LEN + \
        EL_PREFIX_LEN + EL_PRE_LEVEL_LEN)


/* ==========================================================================
    defines length needed to put color information. 5 chars are needed to
    start coloring output, and 4 characters are neede to reset terminal
    color to default
   ========================================================================== */
#if ENABLE_COLORS
#   define EL_COLORS_LEN (5 + 4)
#else
#   define EL_COLORS_LEN 0
#endif


/* ==========================================================================
    maximum buffer length excluding new line and null character. This
    defines length of final output not format. So "%s" can take even 100
    bytes. EL_LOG_MAX is defined externally during compilation.
   ========================================================================== */
#define EL_BUF_MAX (EL_PRE_LEN + (EL_LOG_MAX) + EL_COLORS_LEN)


/* ==========================================================================
    Defines if timestamp should be stored into buffer as string or binary
   ========================================================================== */
#define TS_STRING (0)
#define TS_BINARY (1)


/* ==========================================================================
    ALL_OUTS is a mask of all implemented outputs. This is mask for *all*
    implemented outputs - it doesn't matter if it's enabled during
    compilation or not. This is used to return EINVAL value when user
    provides any output that is not on that list (if specified output is on
    that list but was not compiled, it will return ENODEV and it's done in
    another check). This is basically an bitwise OR of all fields in enum
    el_output
   ========================================================================== */
#define ALL_OUTS (EL_OUT_STDERR | EL_OUT_STDOUT | EL_OUT_SYSLOG | \
    EL_OUT_FILE | EL_OUT_NET | EL_OUT_TTY | EL_OUT_CUSTOM)


/* ==========================================================================
              / __/__  __ ____   _____ / /_ (_)____   ____   _____
             / /_ / / / // __ \ / ___// __// // __ \ / __ \ / ___/
            / __// /_/ // / / // /__ / /_ / // /_/ // / / /(__  )
           /_/   \__,_//_/ /_/ \___/ \__//_/ \____//_/ /_//____/
   ========================================================================== */
#if ENABLE_OUT_FILE
int el_file_open(struct el *el);
int el_file_puts(struct el *el, const char *s);
int el_file_putb(struct el *el, const void *mem, size_t mlen);
void el_file_cleanup(struct el *el);
int el_file_flush(struct el *el);
#endif


int el_oputb_nb(struct el *el, const void *mem, size_t mlen);
int el_oputs_nb(struct el *el, const char *s);
int el_ovprint_nb(const char *file, size_t num, const char *func,
		enum el_level level, struct el *el, const char *fmt, va_list ap);
int el_oprint_nb(const char *file, size_t num, const char *func,
		enum el_level level, struct el *el, const char *fmt, ...);


#if ENABLE_PTHREAD
void el_lock(struct el *el);
void el_unlock(struct el *el);
#else
#define el_lock(x)
#define el_unlock(x)
#endif


#if ENABLE_OUT_TTY
int el_tty_open(struct el *el, const char *dev, unsigned int speed);
int el_tty_puts(struct el *el, const char *s);
int el_tty_close(struct el *el);
#endif

int el_log_allowed(struct el *el, enum el_level level);
size_t el_timestamp(struct el *el, void *buf, int binary);

#if LLONG_MAX
size_t el_encode_number(unsigned long long number, void *out);
size_t el_decode_number(const void *number, unsigned long long *out);
#else
size_t el_encode_number(unsigned long number, void *out);
size_t el_decode_number(const void *number, unsigned long *out);
#endif

#endif
