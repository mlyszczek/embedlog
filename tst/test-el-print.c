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


#include <rb.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>

#include "mtest.h"
#include "stdlib.h"
#include "embedlog.h"

#include "config-priv.h"
#include "el-options.h"


/* ==========================================================================
                  _                __           __
    ____   _____ (_)_   __ ____ _ / /_ ___     / /_ __  __ ____   ___   _____
   / __ \ / ___// /| | / // __ `// __// _ \   / __// / / // __ \ / _ \ / ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / /_ / /_/ // /_/ //  __/(__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/   \__/ \__, // .___/ \___//____/
/_/                                              /____//_/

   ========================================================================== */


struct log_message
{
    const char  *file;
    size_t       line;
    int          level;
    const char  *msg;
};


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


mt_defs_ext();
static char        logbuf[1024 * 1024];  /* output simulation */
static struct rb  *expected_logs;        /* array of expected logs */


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
    this is custom function that will be called instead of for example puts,
    after el_print constructs message.  We capture that message  and  simply
    append to logbuf, so we can analyze it later if everything is in  order.
   ========================================================================== */


static int print_to_buffer
(
    const char *s
)
{
    strcat(logbuf, s);
    return 0;
}


/* ==========================================================================
    Checks if el_print function prints everything  in  well-defined  format.
    This function cross-checks logbuf buffer where el_print function  prints
    message into, and expected_logs with raw information about  what  should
    be printed.
   ========================================================================== */


static int print_check(void)
{
#define IS_DIGIT() if (!isdigit(*msg++)) return -1
#define IS_CHAR(c) if (*msg++ != c) return -1

    static const char  *levelstr = "facewnid";
    struct log_message  expected;
    char               *msg;
    char                tmp[1024];
    int                 i;
    int                 slevel;
    size_t              msglen;
    static const char  *color[] =
    {
        "\e[91m",  /* fatal             light red */
        "\e[31m",  /* alert             red */
        "\e[95m",  /* critical          light magenta */
        "\e[35m",  /* error             magenta */
        "\e[93m",  /* warning           light yellow */
        "\e[92m",  /* notice            light green */
        "\e[32m",  /* information       green */
        "\e[34m",  /* debug             blue */
        "\e[0m"    /* remove all formats */
    };
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    msg = logbuf;
    while (rb_read(expected_logs, &expected, 1) == 1)
    {
        slevel = expected.level > EL_DBG ? EL_DBG : expected.level;

        if (expected.level > g_options.level)
        {
            /*
             * log should not have been printed due to current log level
             * restriction. We just continue here, because if log was indeed
             * printed, next checks will fail anyway.
             */

            continue;
        }

        if (g_options.colors)
        {
            if (strncmp(msg, color[slevel], 5) != 0)
            {
                /*
                 * no color information or el_print generated wrong color
                 */

                return -1;
            }

            /*
             * move msg by 5 bytes - begging of color is always 5char long
             */

            msg += 5;
        }

        /*
         * check printing timestamp
         */

        if (g_options.timestamp == EL_TS_LONG)
        {
            IS_CHAR('[');
            IS_DIGIT();
            IS_DIGIT();
            IS_DIGIT();
            IS_DIGIT();
            IS_CHAR('-');
            IS_DIGIT();
            IS_DIGIT();
            IS_CHAR('-');
            IS_DIGIT();
            IS_DIGIT();
            IS_CHAR(' ');
            IS_DIGIT();
            IS_DIGIT();
            IS_CHAR(':');
            IS_DIGIT();
            IS_DIGIT();
            IS_CHAR(':');
            IS_DIGIT();
            IS_DIGIT();
            IS_CHAR('.');

            for (i = 0; i != 6; ++i)
            {
                IS_DIGIT();
            }
            IS_CHAR(']');

        }
        else if (g_options.timestamp == EL_TS_SHORT)
        {
            IS_CHAR('[');
            while (*msg != '.' )
            {
                IS_DIGIT();
            }

            ++msg; /* skip the '.' character */

            for (i = 0; i != 6; ++i)
            {
                IS_DIGIT();
            }
            IS_CHAR(']');
        }
        else if (g_options.timestamp == EL_TS_OFF)
        {
            /*
             * we check for nothing here
             */
        }
        else
        {
            /*
             * wrong timestamp option value
             */

           return -1;
        }

        /*
         * check printing file information
         */

        if (g_options.finfo && expected.file != NULL && expected.line != 0)
        {
            char  expected_file[8192];
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


            IS_CHAR('[');
            i = 0;

            while (*msg != ':')
            {
                tmp[i] = *msg++;
                if (i++ == EL_FLEN_MAX)
                {
                    /*
                     * we didn't find ':' character withing maximum lenght
                     * of file - file is too long
                     */

                    return -1;
                }
            }

            msg++;  /* skip ':' character */
            tmp[i] = '\0';
            strcpy(expected_file, expected.file);

            if (strcmp(tmp, basename(expected_file)) != 0)
            {
                /*
                 * file name in printed log is different than what we set
                 */

                return -1;
            }

            i = 0;

            while (*msg != ']')
            {
                tmp[i] = *msg;
                if (i++ == EL_PRE_FINFO_LINE_MAX_LEN)
                {
                    /*
                     * file line lenght is too large, or ending ']' is missing
                     */

                    return -1;
                }

                /*
                 * file line should be a number ;-)
                 */

                IS_DIGIT();

            }

            msg++;  /* skip ']' character */
            tmp[i] = '\0';

            if ((size_t)atoi(tmp) != expected.line)
            {
                /*
                 * line number in printed log is different than what was set
                 */

                return -1;
            }
        }

        if ((g_options.finfo && expected.file != NULL && expected.line != 0) ||
             g_options.timestamp != EL_TS_OFF)
        {
            /*
             * file info or timestamp information is enabled, in that case
             * we check for additional space between info and log message
             */

            if (*msg++ != ' ')
            {
                return -1;
            }
        }

        /*
         * check printing log level
         */

        if (g_options.print_log_level)
        {
            if (*msg++ != levelstr[slevel])
            {
                return -1;
            }

            if (*msg++ != '/')
            {
                return -1;
            }
        }

        msglen = strlen(expected.msg);

        if (strncmp(msg, expected.msg, msglen) != 0)
        {
            return -1;
        }

        msg += msglen;

        if (g_options.colors)
        {
            if (strncmp(msg, color[8], 4) != 0)
            {
                /*
                 * expected end of color, got some shit
                 */

                return -1;
            }

            /*
             * end of color is always 4 bytes long
             */

            msg += 4;
        }

        if (*msg != '\n')
        {
            /*
             * all logs should be ended with new line
             */

            return -1;
        }

        /*
         * set msg to point to next message
         */

        ++msg;
    }

    return 0;

#undef IS_CHAR
#undef IS_DIGIT
}


/* ==========================================================================
    adds log to array of expected logs and then  sends  passed  log  message
    into el_print
   ========================================================================== */


static void add_log
(
    const char    *file,
    size_t         line,
    enum el_level  level,
    const char    *msg
)
{
    struct log_message  expected;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    expected.file = file;
    expected.line = line;
    expected.level = level;
    expected.msg = msg;
    rb_write(expected_logs, &expected, 1);
    el_print(file, line, level, msg);
}


/* ==========================================================================
    Called before every test,  initializes  embedlog  to  known  state,  and
    allocates memory for expected_logs buffer
   ========================================================================== */


static void test_prepare(void)
{
    el_init();
    el_option(EL_CUSTOM_PUTS, print_to_buffer);
    el_option(EL_PRINT_LEVEL, 0);
    el_option(EL_OUT, EL_OUT_CUSTOM);
    memset(logbuf, 0, sizeof(logbuf));
    expected_logs = rb_new(1024, sizeof(struct log_message), 0);
}


/* ==========================================================================
    Called after every test. Frees all memory allocated by test_prepare
   ========================================================================== */


static void test_cleanup(void)
{
    el_cleanup();
    rb_destroy(expected_logs);
}


/* ==========================================================================
                           __               __
                          / /_ ___   _____ / /_ _____
                         / __// _ \ / ___// __// ___/
                        / /_ /  __/(__  )/ /_ (__  )
                        \__/ \___//____/ \__//____/

   ========================================================================== */


static void print_simple_message(void)
{
    add_log(ELF, "print_simple_message");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_simple_multiple_message(void)
{
    add_log(ELF, "print_simple_multiple_message first");
    add_log(ELF, "print_simple_multiple_message second");
    add_log(ELF, "print_simple_multiple_message third");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_log_level(void)
{
    el_option(EL_PRINT_LEVEL, 1);
    el_option(EL_LEVEL, EL_DBG);
    add_log(ELF, "print_log_level fatal message");
    add_log(ELA, "print_log_level alert message");
    add_log(ELC, "print_log_level critical message");
    add_log(ELE, "print_log_level error message");
    add_log(ELW, "print_log_level warning message");
    add_log(ELN, "print_log_level notice message");
    add_log(ELI, "print_log_level info message");
    add_log(ELD, "print_log_level debug message");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_colorful_output(void)
{
    el_option(EL_COLORS, 1);
    el_option(EL_LEVEL, EL_DBG + 2);
    add_log(ELF, "print_colorful_output fatal message");
    add_log(ELA, "print_colorful_output alert message");
    add_log(ELC, "print_colorful_output critical message");
    add_log(ELE, "print_colorful_output error message");
    add_log(ELW, "print_colorful_output warning message");
    add_log(ELN, "print_colorful_output notice message");
    add_log(ELI, "print_colorful_output info message");
    add_log(ELD, "print_colorful_output debug message");
    add_log(ELD + 1, "print_colorful_output debug + 1 message");
    add_log(ELD + 2, "print_colorful_output debug + 2 message");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_custom_log_level(void)
{
    el_option(EL_PRINT_LEVEL, 1);
    el_option(EL_LEVEL, EL_DBG + 5);
    add_log(ELD + 4, "print_custom_log_level custom debug 4");
    add_log(ELD + 5, "print_custom_log_level custom debug 5");
    add_log(ELD + 6, "print_custom_log_level custom debug 6");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_short(void)
{
    el_option(EL_TS, EL_TS_SHORT);
    add_log(ELF, "print_timestamp_short first");
    add_log(ELF, "print_timestamp_short second");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_long(void)
{
    el_option(EL_TS, EL_TS_LONG);
    add_log(ELF, "print_timestamp_long first");
    add_log(ELF, "print_timestamp_long second");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_finfo(void)
{
    el_option(EL_FINFO, 1);
    add_log(ELF, "print_finfo first");
    add_log(ELF, "print_finfo second");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_different_clocks(void)
{
    int  i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (i = EL_TS_TM_CLOCK; i != EL_TS_TM_ERROR; ++i)
    {
        test_prepare();
        el_option(EL_TS_TM, i);
        el_option(EL_TS, EL_TS_LONG);
        add_log(ELI, "clock test");
        mt_fok(print_check());
        test_cleanup();
    }
}


/* ==========================================================================
   ========================================================================== */


static void print_mix_of_everything(void)
{
    int level;
    int timestamp;
    int printlevel;
    int finfo;
    int colors;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (level = EL_FATAL;          level <= EL_DBG;              ++level)
    for (timestamp = EL_TS_OFF;     timestamp != EL_TS_ERROR;     ++timestamp)
    for (printlevel = 0;            printlevel <= 1;              ++printlevel)
    for (finfo = 0;                 finfo <= 1;                   ++finfo)
    for (colors = 0;                colors <= 1;                  ++colors)
    {
        test_prepare();
        el_option(EL_LEVEL, level);
        el_option(EL_TS, timestamp);
        el_option(EL_PRINT_LEVEL, printlevel);
        el_option(EL_FINFO, finfo);
        el_option(EL_COLORS, colors);

        add_log(ELF, "fatal message");
        add_log(ELA, "alert message");
        add_log(ELC, "critical message");
        add_log(ELE, "error message");
        add_log(ELW, "warning message");
        add_log(ELN, "notice message");
        add_log(ELI, "info message");
        add_log(ELD, "debug message");
        mt_fok(print_check());

        test_cleanup();
    }
}


/* ==========================================================================
   ========================================================================== */


static void print_too_long_print_truncate(void)
{
    char  msg[EL_LOG_MAX + 3];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(msg, 'a', sizeof(msg));
    msg[sizeof(msg) - 1] = '\0';
    msg[sizeof(msg) - 2] = '3';
    msg[sizeof(msg) - 3] = '2';
    msg[sizeof(msg) - 4] = '1';
    msg[sizeof(msg) - 4] = '0';

    add_log(ELI, "not truncated");
    add_log(ELI, msg);

    /*
     * while el_print will make copy of msg, our test  print_check  function
     * will just use pointer to our msg here, and since we expect message to
     * be truncated, we truncate it here  and  print_check  will  take  this
     * truncated message as expected one.
     */

    msg[sizeof(msg) - 3] = '\0';

    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_truncate_with_date(void)
{
    char  msg[EL_LOG_MAX + 3];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    el_option(EL_TS, EL_TS_LONG);
    memset(msg, 'a', sizeof(msg));
    msg[sizeof(msg) - 1] = '\0';
    msg[sizeof(msg) - 2] = '3';
    msg[sizeof(msg) - 3] = '2';
    msg[sizeof(msg) - 4] = '1';
    msg[sizeof(msg) - 4] = '0';

    add_log(ELI, "not truncated");
    add_log(ELI, msg);

    msg[sizeof(msg) - 3] = '\0';

    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_truncate_with_all_options(void)
{
    char  msg[EL_LOG_MAX + 3];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    el_option(EL_TS, EL_TS_LONG);
    el_option(EL_FINFO, 1);
    el_option(EL_PRINT_LEVEL, 1);
    memset(msg, 'a', sizeof(msg));
    msg[sizeof(msg) - 1] = '\0';
    msg[sizeof(msg) - 2] = '3';
    msg[sizeof(msg) - 3] = '2';
    msg[sizeof(msg) - 4] = '1';
    msg[sizeof(msg) - 4] = '0';

    add_log(ELI, "not truncated");
    add_log(ELI, msg);

    msg[sizeof(msg) - 3] = '\0';

    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_with_no_output_available(void)
{
    el_option(EL_OUT, EL_OUT_NONE);
    mt_ferr(el_print(ELI, "i'll be back"), ENODEV);
}


/* ==========================================================================
   ========================================================================== */


static void print_level_not_high_enough(void)
{
    mt_ferr(el_print(ELD, "i won't be printed"), ERANGE);
}


/* ==========================================================================
   ========================================================================== */


static void print_finfo_path(void)
{
    el_option(EL_FINFO, 1);
    add_log("source/code/file.c", 10, EL_ALERT, "some message");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_nofinfo(void)
{
    el_option(EL_FINFO, 1);
    add_log(NULL, 0, EL_ALERT, "no file info");
    mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_null(void)
{
    mt_ferr(el_print(ELA, NULL), EINVAL);
}

/* ==========================================================================
             __               __
            / /_ ___   _____ / /_   ____ _ _____ ____   __  __ ____
           / __// _ \ / ___// __/  / __ `// ___// __ \ / / / // __ \
          / /_ /  __/(__  )/ /_   / /_/ // /   / /_/ // /_/ // /_/ /
          \__/ \___//____/ \__/   \__, //_/    \____/ \__,_// .___/
                                 /____/                    /_/
   ========================================================================== */


void el_print_test_group(void)
{
    mt_run(print_different_clocks);
    mt_run(print_mix_of_everything);

    mt_prepare_test = &test_prepare;
    mt_cleanup_test = &test_cleanup;

    mt_run(print_simple_message);
    mt_run(print_simple_multiple_message);
    mt_run(print_log_level);
    mt_run(print_colorful_output);
    mt_run(print_custom_log_level);
    mt_run(print_timestamp_short);
    mt_run(print_timestamp_long);
    mt_run(print_finfo);
    mt_run(print_too_long_print_truncate);
    mt_run(print_truncate_with_date);
    mt_run(print_truncate_with_all_options);
    mt_run(print_with_no_output_available);
    mt_run(print_level_not_high_enough);
    mt_run(print_finfo_path);
    mt_run(print_nofinfo);
    mt_run(print_null);
}
