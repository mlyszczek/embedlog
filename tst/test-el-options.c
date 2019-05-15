/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "el-private.h"

#include <string.h>
#include <errno.h>

#include "mtest.h"
#include "test-group-list.h"
#include "config.h"


/* ==========================================================================
          __             __                     __   _
     ____/ /___   _____ / /____ _ _____ ____ _ / /_ (_)____   ____   _____
    / __  // _ \ / ___// // __ `// ___// __ `// __// // __ \ / __ \ / ___/
   / /_/ //  __// /__ / // /_/ // /   / /_/ // /_ / // /_/ // / / /(__  )
   \__,_/ \___/ \___//_/ \__,_//_/    \__,_/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


mt_defs_ext();           /* external variables for mtest */
extern struct el  g_el;  /* global embedlog el */


/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ========================================================================== */


static void test_prepare(void)
{
    el_init();
}


/* ==========================================================================
   ========================================================================== */


static void test_cleanup(void)
{
    el_cleanup();
}


/* ==========================================================================
                           __               __
                          / /_ ___   _____ / /_ _____
                         / __// _ \ / ___// __// ___/
                        / /_ /  __/(__  )/ /_ (__  )
                        \__/ \___//____/ \__//____/

   ========================================================================== */


static void options_init(void)
{
    struct el  default_el;  /* expected default el */
    struct el  el;          /* custom el to init */
    struct el *elp;         /* heap allocated el */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(&default_el, 0, sizeof(default_el));
    default_el.outputs             = EL_OUT_STDERR;
    default_el.level               = EL_INFO;
    default_el.level_current_msg   = EL_DBG;
    default_el.colors              = 0;
    default_el.timestamp           = EL_TS_OFF;
    default_el.timestamp_timer     = EL_TS_TM_TIME;
    default_el.timestamp_fractions = EL_TS_FRACT_OFF;
    default_el.print_log_level     = 1;
    default_el.print_newline       = 1;

#if ENABLE_OUT_FILE
    default_el.fsync_level         = EL_FATAL;
    default_el.funcinfo            = 0;
    default_el.finfo               = 0;
    default_el.frotate_number      = 0;
    default_el.fcurrent_rotate     = 0;
    default_el.frotate_size        = 0;
    default_el.fpos                = 0;
    default_el.file                = NULL;
    default_el.fsync_every         = 32768;
    default_el.fname               = NULL;
#endif

#if ENABLE_OUT_TTY
    default_el.serial_fd           = -1;
#endif

#if ENABLE_OUT_CUSTOM
    default_el.custom_puts         = NULL;
#endif

    mt_fail(el_oinit(&el) == 0);
    mt_fail(memcmp(&el, &default_el, sizeof(el)) == 0);
    mt_fail(el_ocleanup(&el) == 0);

    mt_fail(el_init() == 0);
    mt_fail(memcmp(&g_el, &default_el, sizeof(default_el)) == 0);
    mt_fail(el_cleanup() == 0);

    mt_fail((elp = el_new()) != NULL);
    mt_fail(memcmp(elp, &default_el, sizeof(*elp)) == 0);
    mt_fok(el_destroy(elp));
}


/* ==========================================================================
   ========================================================================== */


static void options_init_einval(void)
{
    errno = 0;
    mt_fail(el_oinit(NULL) == -1);
    mt_fail(errno == EINVAL);
}


/* ==========================================================================
   ========================================================================== */


static void options_level_set(void)
{
    int i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (i = 0; i != 16; ++i)
    {
        if (i <= EL_DBG)
        {
            mt_fail(el_option(EL_LEVEL, i) == 0);
            mt_fail(g_el.level == i);
        }
        else
        {
            mt_ferr(el_option(EL_LEVEL, i), EINVAL);
            mt_fail(g_el.level == EL_DBG);
        }
    }
}


/* ==========================================================================
   ========================================================================== */


static void options_fsync_level_set(void)
{
    int i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (i = 0; i != 16; ++i)
    {
#if ENABLE_OUT_FILE
        if (i <= EL_DBG)
        {
            mt_fail(el_option(EL_FSYNC_LEVEL, i) == 0);
            mt_fail(g_el.fsync_level == i);
        }
        else
        {
            mt_ferr(el_option(EL_FSYNC_LEVEL, i), EINVAL);
            mt_fail(g_el.fsync_level == EL_DBG);
        }
#else
        mt_ferr(el_option(EL_FSYNC_LEVEL, i), ENOSYS);
#endif
    }
}


/* ==========================================================================
   ========================================================================== */


static void options_output(void)
{
    int i;
    int valid_outs = ALL_OUTS;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    for (i = 0; i != ALL_OUTS + 100; ++i)
    {
        int ok = 1;

        if (i > ALL_OUTS)
        {
            mt_ferr(el_option(EL_OUT, i), EINVAL);
            continue;
        }

#ifndef ENABLE_OUT_STDERR
        valid_outs &= ~EL_OUT_STDERR;
        if (i & EL_OUT_STDERR)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_STDERR
        valid_outs &= ~EL_OUT_STDOUT;
        if (i & EL_OUT_STDOUT)
        {
            ok = 0;
        }
#endif


#ifndef ENABLE_OUT_SYSLOG
        valid_outs &= ~EL_OUT_SYSLOG;
        if (i & EL_OUT_SYSLOG)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_FILE
        valid_outs &= ~EL_OUT_FILE;
        if (i & EL_OUT_FILE)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_NET
        valid_outs &= ~EL_OUT_NET;
        if (i & EL_OUT_NET)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_TTY
        valid_outs &= ~EL_OUT_TTY;
        if (i & EL_OUT_TTY)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_CUSTOM
        valid_outs &= ~EL_OUT_CUSTOM;
        if (i & EL_OUT_CUSTOM)
        {
            ok = 0;
        }
#endif

        if (ok)
        {
            mt_fok(el_option(EL_OUT, i));
            continue;
        }

        mt_ferr(el_option(EL_OUT, i), ENODEV);
    }

    mt_fok(el_option(EL_OUT, EL_OUT_ALL));
    mt_fail(g_el.outputs == valid_outs);
}


/* ==========================================================================
   ========================================================================== */


static void options_log_allowed(void)
{
    g_el.level = EL_ERROR;
    g_el.outputs = EL_OUT_STDERR;

    mt_fail(el_log_allowed(&g_el, EL_FATAL)  == 1);
    mt_fail(el_log_allowed(&g_el, EL_ALERT)  == 1);
    mt_fail(el_log_allowed(&g_el, EL_CRIT)   == 1);
    mt_fail(el_log_allowed(&g_el, EL_ERROR)  == 1);
    mt_fail(el_log_allowed(&g_el, EL_WARN)   == 0);
    mt_fail(el_log_allowed(&g_el, EL_NOTICE) == 0);
    mt_fail(el_log_allowed(&g_el, EL_INFO)   == 0);
    mt_fail(el_log_allowed(&g_el, EL_DBG)    == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_opt_print_level(void)
{
    mt_fail(el_option(EL_PRINT_LEVEL, 0) == 0);
    mt_fail(g_el.print_log_level == 0);
    mt_fail(el_option(EL_PRINT_LEVEL, 1) == 0);
    mt_fail(g_el.print_log_level == 1);

    errno = 0;
    mt_fail(el_option(EL_PRINT_LEVEL, 2) == -1);
    mt_fail(errno == EINVAL);
    errno = 0;
    mt_fail(el_option(EL_PRINT_LEVEL, 3) == -1);
    mt_fail(errno == EINVAL);
}


/* ==========================================================================
   ========================================================================== */


static void options_opt_colors(void)
{
#if ENABLE_COLORS
    mt_fok(el_option(EL_COLORS, 0));
    mt_fail(g_el.colors == 0);
    mt_fok(el_option(EL_COLORS, 1));
    mt_fail(g_el.colors == 1);

    mt_ferr(el_option(EL_COLORS, 2), EINVAL);
    mt_ferr(el_option(EL_COLORS, 3), EINVAL);
#else
    mt_ferr(el_option(EL_COLORS, 0), ENOSYS);
    mt_ferr(el_option(EL_COLORS, 1), ENOSYS);
    mt_ferr(el_option(EL_COLORS, 2), ENOSYS);
    mt_ferr(el_option(EL_COLORS, 3), ENOSYS);
#endif
}


/* ==========================================================================
   ========================================================================== */


static void options_opt_timestamp(void)
{
    int  i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    for (i = EL_TS_OFF; i != EL_TS_ERROR; ++i)
    {
#if ENABLE_TIMESTAMP
        mt_fok(el_option(EL_TS, i));
        mt_fail(g_el.timestamp == i);
#else
        mt_ferr(el_option(EL_TS, i), ENOSYS);
#endif
    }

#if ENABLE_TIMESTAMP
    mt_ferr(el_option(EL_TS, i), EINVAL);
#else
    mt_ferr(el_option(EL_TS, i), ENOSYS);
#endif

}


/* ==========================================================================
   ========================================================================== */


static void options_opt_timestamp_timer(void)
{
    int  i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (i = EL_TS_TM_CLOCK; i != EL_TS_TM_ERROR; ++i)
    {
#if ENABLE_TIMESTAMP
#   ifndef ENABLE_REALTIME
        if (i == EL_TS_TM_REALTIME)
        {
            mt_ferr(el_option(EL_TS_TM, i), ENODEV);
            continue;
        }
#   endif

#   ifndef ENABLE_MONOTONIC
        if (i == EL_TS_TM_MONOTONIC)
        {
            mt_ferr(el_option(EL_TS_TM, i), ENODEV);
            continue;
        }
#   endif

#   if ENABLE_CLOCK == 0
        if (i == EL_TS_TM_CLOCK)
        {
            mt_ferr(el_option(EL_TS_TM, i), ENODEV);
            continue;
        }
#   endif

        mt_fok(el_option(EL_TS_TM, i));
        mt_fail(g_el.timestamp_timer == i);
#else
        mt_ferr(el_option(EL_TS, i), ENOSYS);
#endif
    }

#if ENABLE_TIMESTAMP
    mt_ferr(el_option(EL_TS_TM, i), EINVAL);
#else
    mt_ferr(el_option(EL_TS_TM, i), ENOSYS);
#endif
}


/* ==========================================================================
   ========================================================================== */


static void options_opt_timestamp_fraction(void)
{
    int i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (i = EL_TS_FRACT_OFF; i != EL_TS_FRACT_ERROR; ++i)
    {
#if ENABLE_FRACTIONS && ENABLE_TIMESTAMP
        mt_fok(el_option(EL_TS_FRACT, i));
        mt_fail(g_el.timestamp_fractions == i);
#else
        mt_ferr(el_option(EL_TS_FRACT, i), ENOSYS);
#endif
    }

#if ENABLE_FRACTIONS && ENABLE_TIMESTAMP
    mt_ferr(el_option(EL_TS_FRACT, i), EINVAL);
#else
    mt_ferr(el_option(EL_TS_FRACT, i), ENOSYS);
#endif
}

/* ==========================================================================
   ========================================================================== */


static void options_ooption_test(void)
{
    struct el  opts;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#if ENABLE_TIMESTAMP
    el_ooption(&opts, EL_TS_TM, EL_TS_TM_TIME);
    mt_fail(opts.timestamp_timer == EL_TS_TM_TIME);
#else
    mt_ferr(el_ooption(&opts, EL_TS_TM, EL_TS_TM_TIME), ENOSYS);
#endif
}


/* ==========================================================================
   ========================================================================== */


static void options_prefix(void)
{
#if ENABLE_PREFIX
    mt_fok(el_option(EL_PREFIX, "prefix"));
    mt_fok(strcmp("prefix", g_el.prefix));
    mt_fok(el_option(EL_PREFIX, NULL));
    mt_fail(g_el.prefix == NULL);
#else
    mt_ferr(el_option(EL_PREFIX, "prefix"), ENOSYS);
    mt_ferr(el_option(EL_PREFIX, NULL), ENOSYS);
#endif

}


/* ==========================================================================
   ========================================================================== */


static void options_einval(void)
{
    mt_ferr(el_option(10000, 5), EINVAL);
}


/* ==========================================================================
   ========================================================================== */


static void options_global_el_after_el_cleanup(void)
{
    struct el  default_el; /* expected default el */
    struct el  el;         /* custom el to init */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(&default_el, 0, sizeof(default_el));
    default_el.outputs             = EL_OUT_STDERR;
    default_el.level               = EL_INFO;
    default_el.level_current_msg   = EL_DBG;
    default_el.colors              = 0;
    default_el.timestamp           = EL_TS_OFF;
    default_el.timestamp_timer     = EL_TS_TM_TIME;
    default_el.timestamp_fractions = EL_TS_FRACT_OFF;
    default_el.print_log_level     = 1;
    default_el.print_newline       = 1;

#if ENABLE_OUT_FILE
    default_el.fsync_level         = EL_FATAL;
    default_el.funcinfo            = 0;
    default_el.finfo               = 0;
    default_el.frotate_number      = 0;
    default_el.fcurrent_rotate     = 0;
    default_el.frotate_size        = 0;
    default_el.fpos                = 0;
    default_el.file                = NULL;
    default_el.fsync_every         = 32768;
    default_el.fname               = NULL;
#endif

#if ENABLE_OUT_TTY
    default_el.serial_fd           = -1;
#endif

#if ENABLE_OUT_CUSTOM
    default_el.custom_puts         = NULL;
#endif



    el_init();
    el_oinit(&el);
    mt_fail(memcmp(&g_el, &default_el, sizeof(default_el)) == 0);
    mt_fail(g_el.outputs == EL_OUT_STDERR);
    mt_fail(el.outputs == EL_OUT_STDERR);

    el_ocleanup(&el);

    /* global el should not be altered when el_ocleanup is called
     */

    mt_fail(memcmp(&g_el, &default_el, sizeof(default_el)) == 0);
    mt_fail(el.outputs == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_set_funcinfo(void)
{
#if ENABLE_FUNCINFO && (__STDC_VERSION__ >= 199901L)
    mt_fok(el_option(EL_FUNCINFO, 0));
    mt_fail(g_el.funcinfo == 0);
    mt_fok(el_option(EL_FUNCINFO, 1));
    mt_fail(g_el.funcinfo == 1);

    mt_ferr(el_option(EL_FUNCINFO, 2), EINVAL);
    mt_ferr(el_option(EL_FUNCINFO, 3), EINVAL);
#else
    mt_ferr(el_option(EL_FUNCINFO, 0), ENOSYS);
    mt_ferr(el_option(EL_FUNCINFO, 1), ENOSYS);
    mt_ferr(el_option(EL_FUNCINFO, 2), ENOSYS);
    mt_ferr(el_option(EL_FUNCINFO, 3), ENOSYS);
#endif
}


/* ==========================================================================
   ========================================================================== */


static void options_get_global_el(void)
{
    const struct el *opts;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    opts = el_get_el();
    mt_fail(opts == &g_el);
}


/* ==========================================================================
             __               __
            / /_ ___   _____ / /_   ____ _ _____ ____   __  __ ____
           / __// _ \ / ___// __/  / __ `// ___// __ \ / / / // __ \
          / /_ /  __/(__  )/ /_   / /_/ // /   / /_/ // /_/ // /_/ /
          \__/ \___//____/ \__/   \__, //_/    \____/ \__,_// .___/
                                 /____/                    /_/
   ========================================================================== */


void el_options_test_group(void)
{
    mt_run(options_init);
    mt_run(options_init_einval);

    mt_prepare_test = &test_prepare;
    mt_cleanup_test = &test_cleanup;

    mt_run(options_level_set);
    mt_run(options_fsync_level_set);
    mt_run(options_output);
    mt_run(options_log_allowed);
    mt_run(options_opt_print_level);
    mt_run(options_opt_colors);
    mt_run(options_opt_timestamp);
    mt_run(options_opt_timestamp_timer);
    mt_run(options_opt_timestamp_fraction);
    mt_run(options_ooption_test);
    mt_run(options_einval);
    mt_run(options_prefix);
    mt_run(options_global_el_after_el_cleanup);
    mt_run(options_set_funcinfo);
    mt_run(options_get_global_el);
}
