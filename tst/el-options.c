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
    default_options.outputs         = 0;
    default_options.level           = EL_INFO;
    default_options.colors          = 0;
    default_options.timestamp       = EL_OPT_TS_OFF;
    default_options.timestamp_timer = EL_OPT_TS_TM_CLOCK;
    default_options.print_log_level = 1;
    default_options.custom_puts     = NULL;

    default_options.finfo           = 0;
    default_options.frotate_number  = 0;
    default_options.fcurrent_rotate = 0;
    default_options.frotate_size    = 0;
    default_options.fpos            = 0;
    default_options.file            = NULL;
    default_options.fname           = NULL;

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


    mt_assert(el_init() == 0);

    for (i = 0; i != 32; ++i)
    {
        mt_fail(el_level_set(i) == 0);
        mt_fail(g_options.level == i);
    }

    mt_fail(el_cleanup() == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_level_einval(void)
{
    mt_assert(el_init() == 0);
    errno = 0;
    mt_fail(el_olevel_set(NULL, EL_NOTICE) == -1);
    mt_fail(errno == EINVAL);
    mt_fail(el_cleanup() == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_output_enable(void)
{
    int current_outputs;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    current_outputs = 0;
    mt_assert(el_init() == 0);

#if ENABLE_OUT_STDERR
    mt_fail(el_output_enable(EL_OUT_STDERR) == 0);
    current_outputs |= EL_OUT_STDERR;
    mt_fail(current_outputs == g_options.outputs);
#endif

#if ENABLE_OUT_SYSLOG
    mt_fail(el_output_enable(EL_OUT_SYSLOG) == 0);
    current_outputs |= EL_OUT_STDERR;
    mt_fail(current_outputs == g_options.outputs);
#endif

#if ENABLE_OUT_FILE
    mt_fail(el_output_enable(EL_OUT_FILE) == 0);
    current_outputs |= EL_OUT_FILE;
    mt_fail(current_outputs == g_options.outputs);
#endif

#if ENABLE_OUT_NET
    mt_fail(el_output_enable(EL_OUT_NET) == 0);
    current_outputs |= EL_OUT_NET;
    mt_fail(current_outputs == g_options.outputs);
#endif

#if ENABLE_OUT_TTY
    mt_fail(el_output_enable(EL_OUT_TTY) == 0);
    current_outputs |= EL_OUT_TTY;
    mt_fail(current_outputs == g_options.outputs);
#endif

    mt_fail(el_cleanup() == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_output_disable(void)
{
    int current_outputs;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    current_outputs = 0
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

    mt_assert(el_init() == 0);
    g_options.outputs = current_outputs;

#if ENABLE_OUT_STDERR
    mt_fail(el_output_disable(EL_OUT_STDERR) == 0);
    current_outputs &= ~EL_OUT_STDERR;
    mt_fail(current_outputs == g_options.outputs);
#endif

#if ENABLE_OUT_SYSLOG
    mt_fail(el_output_disable(EL_OUT_SYSLOG) == 0);
    current_outputs &= ~EL_OUT_SYSLOG;
    mt_fail(current_outputs == g_options.outputs);
#endif

#if ENABLE_OUT_FILE
    mt_fail(el_output_disable(EL_OUT_FILE) == 0);
    current_outputs &= ~EL_OUT_FILE;
    mt_fail(current_outputs == g_options.outputs);
#endif

#if ENABLE_OUT_NET
    mt_fail(el_output_disable(EL_OUT_NET) == 0);
    current_outputs &= ~EL_OUT_NET;
    mt_fail(current_outputs == g_options.outputs);
#endif

#if ENABLE_OUT_TTY
    mt_fail(el_output_disable(EL_OUT_TTY) == 0);
    current_outputs &= ~EL_OUT_TTY;
    mt_fail(current_outputs == g_options.outputs);
#endif

    mt_fail(el_cleanup() == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_output_enable_einval(void)
{
    mt_assert(el_init() == 0);

    errno = 0;
    mt_fail(el_output_enable(0x1fff) == -1);
    mt_fail(errno == EINVAL);

    errno = 0;
    mt_fail(el_output_disable(0x10ff) == -1);
    mt_fail(errno == EINVAL);

    mt_fail(el_cleanup() == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_output_enable_enosys(void)
{
    mt_assert(el_init() == 0);

#if !ENABLE_OUT_STDERR
    errno = 0;
    mt_fail(el_output_enable(EL_OUT_STDERR) == -1);
    mt_fail(errno == ENOSYS);

    errno = 0;
    mt_fail(el_output_disable(EL_OUT_STDERR) == -1);
    mt_fail(errno = ENOSYS);
#endif

#if !ENABLE_OUT_SYSLOG
    errno = 0;
    mt_fail(el_output_enable(EL_OUT_SYSLOG) == -1);
    mt_fail(errno == ENOSYS);

    errno = 0;
    mt_fail(el_output_disable(EL_OUT_SYSLOG) == -1);
    mt_fail(errno = ENOSYS);
#endif

#if !ENABLE_OUT_FILE
    errno = 0;
    mt_fail(el_output_enable(EL_OUT_FILE) == -1);
    mt_fail(errno == ENOSYS);

    errno = 0;
    mt_fail(el_output_disable(EL_OUT_FILE) == -1);
    mt_fail(errno = ENOSYS);
#endif

#if !ENABLE_OUT_NET
    errno = 0;
    mt_fail(el_output_enable(EL_OUT_NET) == -1);
    mt_fail(errno == ENOSYS);

    errno = 0;
    mt_fail(el_output_disable(EL_OUT_NET) == -1);
    mt_fail(errno = ENOSYS);
#endif

#if !ENABLE_OUT_TTY
    errno = 0;
    mt_fail(el_output_enable(EL_OUT_TTY) == -1);
    mt_fail(errno == ENOSYS);

    errno = 0;
    mt_fail(el_output_disable(EL_OUT_TTY) == -1);
    mt_fail(errno = ENOSYS);
#endif

}


/* ==========================================================================
   ========================================================================== */


static void options_log_allowed(void)
{
    mt_assert(el_init() == 0);
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

    g_options.outputs = 0;

    mt_fail(el_log_allowed(&g_options, EL_FATAL)  == 0);
    mt_fail(el_log_allowed(&g_options, EL_ALERT)  == 0);
    mt_fail(el_log_allowed(&g_options, EL_CRIT)   == 0);
    mt_fail(el_log_allowed(&g_options, EL_ERROR)  == 0);
    mt_fail(el_log_allowed(&g_options, EL_WARN)   == 0);
    mt_fail(el_log_allowed(&g_options, EL_NOTICE) == 0);
    mt_fail(el_log_allowed(&g_options, EL_INFO)   == 0);
    mt_fail(el_log_allowed(&g_options, EL_DBG)    == 0);

    mt_fail(el_cleanup() == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_opt_print_level(void)
{
    mt_assert(el_init() == 0);

    mt_fail(el_option(EL_OPT_PRINT_LEVEL, 0) == 0);
    mt_fail(g_options.print_log_level == 0);
    mt_fail(el_option(EL_OPT_PRINT_LEVEL, 1) == 0);
    mt_fail(g_options.print_log_level == 1);

    errno = 0;
    mt_fail(el_option(EL_OPT_PRINT_LEVEL, 2) == -1);
    mt_fail(errno == EINVAL);
    errno = 0;
    mt_fail(el_option(EL_OPT_PRINT_LEVEL, 3) == -1);
    mt_fail(errno == EINVAL);

    mt_fail(el_cleanup() == 0);
}


/* ==========================================================================
   ========================================================================== */


static void options_opt_colors(void)
{
#if ENABLE_COLORS
    mt_assert(el_init() == 0);

    mt_fok(el_option(EL_OPT_COLORS, 0));
    mt_fail(g_options.colors == 0);
    mt_fok(el_option(EL_OPT_COLORS, 1));
    mt_fail(g_options.colors == 1);

    mt_ferr(el_option(EL_OPT_COLORS, 2), EINVAL);
    mt_ferr(el_option(EL_OPT_COLORS, 3), EINVAL);

    mt_fok(el_cleanup());
#endif
}


/* ==========================================================================
   ========================================================================== */


static void options_opt_timestamp(void)
{
#if ENABLE_TIMESTAMP
    mt_assert(el_init() == 0);

    mt_fok(el_option(EL_OPT_TS, 1));
#endif
}


/* ==========================================================================
   ========================================================================== */




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
    mt_run(options_level_set);
    mt_run(options_level_einval);
    mt_run(options_output_enable);
    mt_run(options_output_disable);
    mt_run(options_output_enable_einval);
    mt_run(options_output_enable_enosys);
    mt_run(options_log_allowed);
    mt_run(options_opt_print_level);
    mt_run(options_opt_colors);
}
