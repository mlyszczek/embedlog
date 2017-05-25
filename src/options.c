/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


/* ==== include files ======================================================= */


#include "embedlog.h"
#include "config.h"
#include "options.h"
#include "valid.h"

#include <errno.h>

/* ==== global variables ==================================================== */


struct options g_options;


/* ==== private variables =================================================== */


static const int VALID_OUTS = 0

    #if ENABLE_OUT_STDERR
        | EL_OUT_STDERR
    #endif

    #if ENABLE_OUT_SYSLOG
        | EL_OUT_SYSLOG
    #endif

    #if ENABLE_OUT_FILE
        | EL_OUT_FILE
    #endif

    #if ENABLE_OUT_NET
        | EL_OUT_NET
    #endif

    #if ENABLE_OUT_TTY
        | EL_OUT_TTY
    #endif
        ;


/* ==== public functions ==================================================== */


/* ==========================================================================
    Sets current log level

    errno:
            EINVAL      passed level is not a valid log level
   ========================================================================== */


int el_level_set
(
    enum el_level level  /* log level to set */
)
{
    VALID(EINVAL, 0 <= level && level <= EL_LEVEL_DBG);

    g_options.level = level;

    return 0;
}


/* ==========================================================================
    Enables specified output for prints

    errno:
            EINVAL      passwd output is invalid
            ENOSYS      specified output is not implemented, ie. was not
                        enabled during compilation
   ========================================================================== */


int el_output_enable
(
    enum el_output output  /* output to enable */
)
{
    VALID(EINVAL, (output & ~EL_OUT_ALL) == 0x00);
    VALID(ENOSYS, (output & ~VALID_OUTS) == 0x00);

    g_options.outputs |= output;

    return 0;
}


/* ==========================================================================
    Disables specified output from prints

    errno:
            EINVAL      passed output is invalid
            ENOSYS      specified output is not implemented, ie. was not
                        enabled during compilation
   ========================================================================== */


int el_output_disable
(
    enum el_output output  /* output to disable */
)
{
    VALID(EINVAL, (output & ~EL_OUT_ALL) == 0x00);
    VALID(ENOSYS, (output & ~VALID_OUTS) == 0x00);

    g_options.outputs &= ~output;

    return 0;
}


/* ==========================================================================
    checks wheter log print is allowed if not. Print is not allowed when log
    level  is  no  high  enough  or   no   output   has   been   configured.
   ========================================================================== */


int el_log_allowed
(
    enum el_level level  /* log level to check */
)
{
    return g_options.level >= level && g_options.outputs;
}


/* ==========================================================================
    sets 'option' with 'value'

    errno:
            EINVAL      option is invalid
            EINVAL      value for specific option is invalid
            ENOSYS      option was disabled during compilation
   ========================================================================== */


int el_option
(
    enum el_option option,
    int            value
)
{
    VALID(EINVAL, 0 <= option && option <= EL_OPT_FINFO);

    switch (option)
    {
    #if ENABLE_COLORS

    case EL_OPT_COLORS:
        /*
         * only 1 or 0 is allowed, if any other bit is set return EINVAL
         */

        VALID(EINVAL, (value & ~1) == 0);

        g_options.colors = value;
        return 0;

    #endif /* ENABLE_COLORS */

    #if ENABLE_TIMESTAMP

    case EL_OPT_TS:
        VALID(EINVAL, 0 <= value && value <= EL_OPT_TS_LONG);

        g_options.timestamp = value;
        return 0;

    case EL_OPT_TS_TM:
        VALID(EINVAL, 0 <= value && value <= EL_OPT_TS_TM_CLOCK);

        g_options.timestamp_timer = value;
        return 0;

    #endif /* ENABLE_TIMEOUT */

    #if ENABLE_FINFO

    case EL_OPT_FINFO:
        VALID(EINVAL, (value & ~1) == 0);

        g_options.finfo = value;
        return 0;

    #endif /* ENABLE_TIMEOUT */

    default:
        /*
         * if we get here, user used option that was disabled during compilation
         * time and is not implemented
         */

        errno = ENOSYS;
        return -1;
    }
}

