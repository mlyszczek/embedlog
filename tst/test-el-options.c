/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include <string.h>
#include <errno.h>

#include "embedlog.h"
#include "el-options.h"
#include "mtest.h"
#include "test-group-list.h"
#include "config.h"


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


mt_defs_ext();                        /* external variables for mtest */
extern struct el_options  g_options;  /* global embedlog options */


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
    struct el_options          default_options; /* expected default options */
    struct el_options          options;         /* custom options to init */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(&default_options, 0, sizeof(default_options));
    default_options.outputs             = 0;
    default_options.level               = EL_INFO;
    default_options.colors              = 0;
    default_options.timestamp           = EL_TS_OFF;
    default_options.timestamp_timer     = EL_TS_TM_TIME;
    default_options.timestamp_useconds  = 1;
    default_options.print_log_level     = 1;
    default_options.print_newline       = 1;
    default_options.custom_puts         = NULL;
    default_options.serial_fd           = -1;

    default_options.finfo               = 0;
    default_options.frotate_number      = 0;
    default_options.fcurrent_rotate     = 0;
    default_options.frotate_size        = 0;
    default_options.fpos                = 0;
    default_options.file                = NULL;
    default_options.file_sync_every     = 32768;
    default_options.fname               = NULL;

    mt_fail(el_oinit(&options) == 0);
    mt_fail(memcmp(&options, &default_options, sizeof(options)) == 0);
    mt_fail(el_ocleanup(&options) == 0);

    mt_fail(el_init() == 0);
    mt_fail(memcmp(&g_options, &default_options, sizeof(default_options)) == 0);
    mt_fail(el_cleanup() == 0);
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


    for (i = 0; i != 32; ++i)
    {
        mt_fail(el_option(EL_LEVEL, i) == 0);
        mt_fail(g_options.level == i);
    }
}


/* ==========================================================================
   ========================================================================== */


static void options_output(void)
{
    int current_outputs;
    int i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    current_outputs = 0;

    for (i = 0; i != EL_OUT_ALL; ++i)
    {
        int ok = 1;

#ifndef ENABLE_OUT_STDERR
        if (i & EL_OUT_STDERR)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_SYSLOG
        if (i & EL_OUT_SYSLOG)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_FILE
        if (i & EL_OUT_FILE)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_NET
        if (i & EL_OUT_NET)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_TTY
        if (i & EL_OUT_TTY)
        {
            ok = 0;
        }
#endif

#ifndef ENABLE_OUT_CUSTOM
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

    mt_ferr(el_option(EL_OUT, EL_OUT_ALL + 7), EINVAL);
}


/* ==========================================================================
   ========================================================================== */


static void options_log_allowed(void)
{
    g_options.level = EL_ERROR;
    g_options.outputs = EL_OUT_STDERR;

    mt_fail(el_log_allowed(&g_options, EL_FATAL)  == 1);
    mt_fail(el_log_allowed(&g_options, EL_ALERT)  == 1);
    mt_fail(el_log_allowed(&g_options, EL_CRIT)   == 1);
    mt_fail(el_log_allowed(&g_options, EL_ERROR)  == 1);
    mt_fail(el_log_allowed(&g_options, EL_WARN)   == 0);
    mt_fail(el_log_allowed(&g_options, EL_NOTICE) == 0);
    mt_fail(el_log_allowed(&g_options, EL_INFO)   == 0);
    mt_fail(el_log_allowed(&g_options, EL_DBG)    == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_opt_print_level(void)
{
    mt_fail(el_option(EL_PRINT_LEVEL, 0) == 0);
    mt_fail(g_options.print_log_level == 0);
    mt_fail(el_option(EL_PRINT_LEVEL, 1) == 0);
    mt_fail(g_options.print_log_level == 1);

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
    mt_fail(g_options.colors == 0);
    mt_fok(el_option(EL_COLORS, 1));
    mt_fail(g_options.colors == 1);

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
        mt_fail(g_options.timestamp == i);
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
        mt_fail(g_options.timestamp_timer == i);
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


static void options_ooption_test(void)
{
    struct el_options  opts;
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
    el_option(EL_PREFIX, "prefix");
#if ENABLE_PREFIX
    mt_fok(strcmp("prefix", g_options.prefix));
#else
    mt_fail(g_options.prefix == NULL);
#endif

    el_option(EL_PREFIX, NULL);
    mt_fail(g_options.prefix == NULL);
}


/* ==========================================================================
   ========================================================================== */


static void options_einval(void)
{
    mt_ferr(el_option(10000, 5), EINVAL);
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
    mt_run(options_output);
    mt_run(options_log_allowed);
    mt_run(options_opt_print_level);
    mt_run(options_opt_colors);
    mt_run(options_opt_timestamp);
    mt_run(options_opt_timestamp_timer);
    mt_run(options_ooption_test);
    mt_run(options_einval);
    mt_run(options_prefix);
}
