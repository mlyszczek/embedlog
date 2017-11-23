/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================

   -------------------------------------------------------------
  / This is option module, here we parse and manage all options \
  | regarding logging. Also this is home for global default     |
  \ option object                                               /
   -------------------------------------------------------------
    \                         .       .
     \                       / `.   .' "
      \              .---.  <    > <    >  .---.
       \             |    \  \ - ~ ~ - /  /    |
         _____          ..-~             ~-..-~
        |     |   \~~~\.'                    `./~~~/
       ---------   \__/                        \__/
      .'  O    \     /               /       \  "
     (_____,    `._.'               |         }  \/~~~/
      `----.          /       }     |        /    \__/
            `-.      |       /      |       /      `. ,~~|
                ~-.__|      /_ - ~ ^|      /- _      `..-'
                     |     /        |     /     ~-.     `-. _  _  _
                     |_____|        |_____|         ~ - . _ _ _ _ _>
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "config.h"

#include <errno.h>
#include <string.h>

#include "embedlog.h"
#include "el-options.h"
#include "valid.h"
#include "el-file.h"


/* ==========================================================================
                               __        __            __
                       ____ _ / /____   / /_   ____ _ / /
                      / __ `// // __ \ / __ \ / __ `// /
                     / /_/ // // /_/ // /_/ // /_/ // /
                     \__, //_/ \____//_.___/ \__,_//_/
                    /____/
                                   _         __     __
              _   __ ____ _ _____ (_)____ _ / /_   / /___   _____
             | | / // __ `// ___// // __ `// __ \ / // _ \ / ___/
             | |/ // /_/ // /   / // /_/ // /_/ // //  __/(__  )
             |___/ \__,_//_/   /_/ \__,_//_.___//_/ \___//____/

   ========================================================================== */


struct el_options g_options;


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


static const int VALID_OUTS = 0

#if ENABLE_OUT_STDERR
    | EL_OPT_OUT_STDERR
#endif

#if ENABLE_OUT_SYSLOG
    | EL_OPT_OUT_SYSLOG
#endif

#if ENABLE_OUT_FILE
    | EL_OPT_OUT_FILE
#endif

#if ENABLE_OUT_NET
    | EL_OPT_OUT_NET
#endif

#if ENABLE_OUT_TTY
    | EL_OPT_OUT_TTY
#endif

#if ENABLE_OUT_CUSTOM
    | EL_OPT_OUT_CUSTOM
#endif
    ;


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
    sets 'option' with 'ap' values in 'options' object.

    errno
            EINVAL      option is invalid
            EINVAL      value for specific option is invalid
            ENOSYS      option was disabled during compilation
   ========================================================================== */


static int el_vooption
(
    struct el_options  *options,  /* options object to set option to */
    enum el_option      option,   /* option to set */
    va_list             ap        /* option value(s) */
)
{
    int          value_int;       /* ap value treated as integer */
    long         value_long;      /* ap value treated as long */
    const char  *value_str;       /* ap value treated as string */
    void       (*value_ptr)();    /* ap value threated as function pointer */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    VALID(EINVAL, 0 <= option && option < EL_OPT_ERROR);

    switch (option)
    {
    case EL_OPT_LEVEL:
        value_int = va_arg(ap, int);
        options->level = value_int;
        return 0;

    case EL_OPT_OUTPUT:
        value_int = va_arg(ap, int);
        VALID(EINVAL, (value_int & ~EL_OPT_OUT_ALL) == 0x00);
        VALID(ENOMEDIUM, (value_int & ~VALID_OUTS) == 0x00);
        options->outputs = value_int;
        return 0;

    case EL_OPT_PRINT_LEVEL:
        value_int = va_arg(ap, int);
        VALID(EINVAL, (value_int & ~1) == 0);

        options->print_log_level = value_int;
        return 0;

    #if ENABLE_COLORS

    case EL_OPT_COLORS:
        /*
         * only 1 or 0 is allowed, if any other bit is set return EINVAL
         */

        value_int = va_arg(ap, int);
        VALID(EINVAL, (value_int & ~1) == 0);

        options->colors = value_int;
        return 0;

    #endif /* ENABLE_COLORS */

    #if ENABLE_TIMESTAMP

    case EL_OPT_TS:
        value_int = va_arg(ap, int);
        VALID(EINVAL, 0 <= value_int && value_int < EL_OPT_TS_ERROR);

        options->timestamp = value_int;
        return 0;

    case EL_OPT_TS_TM:
        value_int = va_arg(ap, int);
        VALID(EINVAL, 0 <= value_int && value_int < EL_OPT_TS_TM_ERROR);

        options->timestamp_timer = value_int;
        return 0;

    #endif /* ENABLE_TIMESTAMP */

    #if ENABLE_FINFO

    case EL_OPT_FINFO:
        value_int = va_arg(ap, int);
        VALID(EINVAL, (value_int & ~1) == 0);

        options->finfo = value_int;
        return 0;

    #endif /* ENABLE_FINFO */

    #if ENABLE_OUT_FILE

    case EL_OPT_FNAME:
        value_str = va_arg(ap, const char *);
        VALID(EINVAL, value_str);
        options->fname = value_str;
        return el_file_open(options);

    case EL_OPT_FROTATE_NUMBER:
    {
        int previous_frotate; /* previous value of frotate number */
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


        value_int = va_arg(ap, int);
        VALID(EINVAL, value_int >= 0);
        previous_frotate = options->frotate_number;
        options->frotate_number = value_int;

        if (previous_frotate == 0 && options->file)
        {
            /*
             * user turned on file rotation  when  file  is  already  opened
             * without rotation.  To prevent weird situations and even  data
             * loss, we reopen file as opening with log rotation  is  a  bit
             * different.  el_file_open  function  will  close  file  before
             * reopening
             */

            return el_file_open(options);
        }

        return 0;
    }

    case EL_OPT_FROTATE_SIZE:
        value_long = va_arg(ap, long);
        VALID(EINVAL, value_long >= 1);
        options->frotate_size = value_long;
        return 0;

    #endif  /* ENABLE_OUT_FILE */

    #if ENABLE_OUT_CUSTOM

    case EL_OPT_CUSTOM_PUTS:
        value_ptr = va_arg(ap, void (*)());
        options->custom_puts = (el_custom_puts)value_ptr;
        return 0;

    #endif /* ENABLE_OUT_CUSTOM */

    default:
        /*
         * if we get here, user used option that was disabled during compilation
         * time and is not implemented
         */

        errno = ENOSYS;
        return -1;
    }
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
    initializes global options to default state
   ========================================================================== */


int el_init
(
    void
)
{
    return el_oinit(&g_options);
}


/* ==========================================================================
    Sets options object to sane values

    errno
            EINVAL      options is invalid (null)
   ========================================================================== */


int el_oinit
(
    struct el_options  *options  /* options object */
)
{
    VALID(EINVAL, options);

    memset(options, 0, sizeof(struct el_options));
    options->print_log_level = 1;
    options->level = EL_INFO;
    return 0;
}


/* ==========================================================================
    cleans up whatever has been initialized/reserved by el_init
   ========================================================================== */


int el_cleanup
(
    void
)
{
    return el_ocleanup(&g_options);
}


/* ==========================================================================
    cleans up  whatever  has  been  initialized/reserved  by  el_option  and
    el_init calls

    errno
            EINVAL      options is invalid (null)
   ========================================================================== */


int el_ocleanup
(
    struct el_options  *options  /* options object */
)
{
    VALID(EINVAL, options);

    g_options.outputs = 0;
#if ENABLE_OUT_FILE
    el_file_cleanup(options);
#endif

    return 0;
}


/* ==========================================================================
    checks wheter log print is allowed if not. Print is not allowed when log
    level  is  no  high  enough  or   no   output   has   been   configured.
   ========================================================================== */


int el_log_allowed
(
    struct el_options  *options,   /* options object */
    enum el_level       level      /* log level to check */
)
{
    return options->level >= level;
}


/* ==========================================================================
    el_ooptions but for default g_options object
   ========================================================================== */


int el_option
(
    enum el_option   option,   /* option to set */
                     ...       /* option value */
)
{
    va_list ap;  /* variadic arguments */
    int     rc;  /* return code from el_voooption */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    va_start(ap, option);
    rc = el_vooption(&g_options, option, ap);
    va_end(ap);

    return rc;
}


/* ==========================================================================
    same as el_vooptions but accepts variadic arguments
   ========================================================================== */


int el_ooption
(
    struct el_options  *options,  /* options object to set option to */
    enum el_option      option,   /* option to set */
                        ...       /* option value(s) */
)
{
    va_list ap;  /* variadic arguments */
    int     rc;  /* return code from el_voooption */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    va_start(ap, option);
    rc = el_vooption(options, option, ap);
    va_end(ap);

    return rc;
}
