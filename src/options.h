/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


/* ==== include files ======================================================= */


#include "config.h"


/* ==== public types ======================================================== */


/* ==========================================================================
    options used in library
   ========================================================================== */


struct options
{
    /*
     * list of enabled outputs
     */

    int outputs;

    /*
     * current log level
     */

    int level;

#if ENABLE_COLORS
    /*
     * if set, output will be colored (on supported terminals only)
     */

    int colors;
#endif

#if ENABLE_TIMESTAMP
    /*
     * if enabled, timestamp will be added to the log
     *
     *      EL_OPT_TS_OFF
     *              no timestamp will be added
     *
     *      EL_OPT_TS_SHORT
     *              short timestamp will be added in format seconds.microseconds
     *
     *      EL_OPT_TS_LONG
     *                  full date will added to each log
     */

    int timestamp;

    /*
     * timer to use to calculate timestamp
     *
     *      EL_OPT_TS_TM_REALTIME
     *              It usually measures wall clock, it doesn't wrap, but can
     *              move move forward or backward without a warning. Available
     *              since POSIX 199309L
     *
     *      EL_OPT_TS_TM_MONOTONIC
     *              Clock that measures time from unspecified time since boot.
     *              Changes every boot, but doesn't jump forward of backward.
     *              This clock is an optional POSIX clock.
     *
     *      EL_OPT_TS_TM_TIME
     *              Measures wall clock - just like REALTIME but its resolution
     *              is only to 1 second. Use only when you build for non-posix.
     *              This is available where C89 is available.
     *
     *      EL_OPT_TS_TM_CLOCK
     *              Measures cpu clocks. Similar to MONOTONIC, but wraps pretty
     *              quickly, on systems where CLOCKS_PER_SEC is 1000 (embedded
     *              systems) clock will wrap in around 24 days (on 32bits), and
     *              this is high implementation specific. This timer is
     *              available where C89 is available.
     */

    int timestamp_timer;
#endif

#if ENABLE_FINFO
    /*
     * if enabled, information about location from where log is printed will be
     * added
     */

    int finfo;
#endif
};


/* ==== global variables ==================================================== */


extern struct options g_options;


/* ==== public functions ==================================================== */


int el_log_allowed(enum el_level);
