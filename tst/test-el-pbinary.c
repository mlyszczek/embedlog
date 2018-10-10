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


#include "config.h"

#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <rb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

#include "mtest.h"
#include "el-private.h"

#if ENABLE_BINARY_LOGS

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
    const unsigned char  *msg;
    size_t                msglen;
    int                   level;
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
static const char   *logpath = "./embedlog-pbinary-test";
static struct rb    *expected_logs;        /* array of expected logs */
static int           truncate_test;

static unsigned char  d1[] = { 0x01 };
static unsigned char  d2[] = { 0x53, 0x10 };
static unsigned char  d3[] = { 0x00, 0x10, 0x12 };
static unsigned char  d5[] = {  'f',  'o',  'o', 0x00,  'b' };
static unsigned char  d8[] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };


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
    Checks if el_print function prints everything  in  well-defined  format.
    This function cross-checks logbuf buffer where el_print function  prints
    message into, and expected_logs with raw information about  what  should
    be printed.
   ========================================================================== */


static int pbinary_check(void)
{
    struct log_message  expected;
    struct stat         st;
    unsigned char      *msg;
    unsigned char      *msgsave;
    int                 fd;
    unsigned char       tmp[1024];
    int                 i;
    int                 slevel;
    size_t              msglen;
    size_t              len;
    unsigned char       flags;

#ifdef LLONG_MAX
    unsigned long long  val;
#else
    unsigned long       val;
#endif
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    stat(logpath, &st);
    fd = open(logpath, O_RDONLY);
    msg = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    msgsave = msg;

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

        /*
         * read flags
         */

        flags = *msg++;

        /*
         * check if level is added to flags
         */

        if ((flags >> 3) != expected.level)
        {
            /*
             * no, its not set correctly
             */

            goto error;
        }

        /*
         * check printing timestamp
         */

        if (g_options.timestamp != EL_TS_OFF)
        {
#   ifdef LLONG_MAX
            unsigned long long tmp;
#   else
            unsigned long      tmp;
#   endif
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

            len = el_decode_number(msg, &tmp);
            msg += len;

            /*
             * we don't know exact time embedlog put into log file, so we
             * have to trust the size
             */

            if (flags & 0x01 != 0x01)
            {
                /*
                 * expected timestamp flag to be set
                 */

                goto error;
            }

            /*
             * fraction of seconds checks makes sense only when ts is set
             */

            if (g_options.timestamp_fractions)
            {
#   ifdef LLONG_MAX
                unsigned long long usec;
#   else
                unsigned long      usec;
#   endif
                /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

                len = el_decode_number(msg, &usec);
                msg += len;

                if (usec > 999999)
                {
                    /*
                     * we can't check exact value of microseconds, but we surely
                     * can check if this value is not too big (and thus corrupt)
                     */

                    goto error;
                }

                if (flags & 0x04 != 0x04)
                {
                    /*
                     * usec not set, and should be
                     */

                    goto error;
                }
            }
            else
            {
                if (flags & 0x06)
                {
                    /*
                     * fraction flags set, and should not be
                     */

                    goto error;
                }
            }


        }
        else
        {
            if (flags & 0x01)
            {
                /*
                 * we didn't expect timestamp flag to be set
                 */

                goto error;
            }

            if (flags & 0x06)
            {
                /*
                 * when timestamp is not set, we expect fraction of seconds
                 * to not be set as well
                 */

                goto error;
            }
        }

        /*
         * now we can check printed data
         */

        if (truncate_test)
        {
            int i;
            int written;
            char expect[EL_BUF_MAX];
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

            for (i = 0; i != sizeof(expect); ++i)
            {
                expect[i] = (unsigned char)i;
            }

            written = msg - msgsave;

            len = el_decode_number(msg, &val);

            if (val != EL_BUF_MAX - written - len)
            {
                /*
                 * truncated message has wrong length
                 */

                return -1;
            }

            msg += len;

            if (memcmp(msg, expect, val) != 0)
            {
                /*
                 * data is incorrect
                 */

                return -1;
            }

            /*
             * point to next message
             */

            msg += val;
        }
        else
        {
            len = el_decode_number(msg, &val);
            msg += len;

            if (memcmp(msg, expected.msg, val) != 0)
            {
                /*
                 * data different than expected
                 */

                goto error;
            }

            /*
             * finally set msg to point to next message
             */

            msg += val;
        }
    }

    munmap(msgsave, st.st_size);
    close(fd);
    return 0;

error:
    munmap(msgsave, st.st_size);
    close(fd);
    return 1;
}


/* ==========================================================================
    adds log to array of expected logs and then  sends  passed  log  message
    into el_print
   ========================================================================== */


static void add_log
(
    enum el_level  level,
    const void    *mem,
    size_t         mlen
)
{
    struct log_message  expected;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    expected.level = level;
    expected.msg = mem;
    expected.msglen = mlen;
    rb_write(expected_logs, &expected, 1);
    el_pbinary(level, mem, mlen);
}


/* ==========================================================================
    Called before every test,  initializes  embedlog  to  known  state,  and
    allocates memory for expected_logs buffer
   ========================================================================== */


static void test_prepare(void)
{
    el_init();
    unlink(logpath);
    el_option(EL_PRINT_LEVEL, 0);
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_PREFIX, NULL);
    el_option(EL_COLORS, 0);
    el_option(EL_TS, EL_TS_OFF);
    el_option(EL_TS_FRACT, EL_TS_FRACT_OFF);
    el_option(EL_PRINT_LEVEL, 0);
    el_option(EL_FROTATE_NUMBER, 0);
    el_option(EL_FILE_SYNC_EVERY, 1024);
    el_option(EL_FPATH, logpath);
    truncate_test = 0;
    expected_logs = rb_new(1024, sizeof(struct log_message), 0);
}


/* ==========================================================================
    Called after every test. Frees all memory allocated by test_prepare
   ========================================================================== */


static void test_cleanup(void)
{
    el_cleanup();
    rb_destroy(expected_logs);
    unlink(logpath);
}


/* ==========================================================================
                           __               __
                          / /_ ___   _____ / /_ _____
                         / __// _ \ / ___// __// ___/
                        / /_ /  __/(__  )/ /_ (__  )
                        \__/ \___//____/ \__//____/

   ========================================================================== */


static void pbinary_simple_message(void)
{
    add_log(EL_FATAL, d5, 5);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_simple_multiple_message(void)
{
    add_log(EL_FATAL, d1, 1);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_ts_without_fractions(void)
{
    el_option(EL_TS_FRACT, EL_TS_FRACT_OFF);
    add_log(EL_FATAL, d1, 1);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_ts_fractions_ms(void)
{
    el_option(EL_TS_FRACT, EL_TS_FRACT_MS);
    add_log(EL_FATAL, d1, 1);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_ts_fractions_us(void)
{
    el_option(EL_TS_FRACT, EL_TS_FRACT_US);
    add_log(EL_FATAL, d1, 1);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_ts_fractions_ns(void)
{
    el_option(EL_TS_FRACT, EL_TS_FRACT_NS);
    add_log(EL_FATAL, d1, 1);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_timestamp_short(void)
{
    el_option(EL_TS, EL_TS_SHORT);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_timestamp_long(void)
{
    el_option(EL_TS, EL_TS_LONG);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_timestamp_short_no_fractions(void)
{
    el_option(EL_TS_FRACT, EL_TS_FRACT_OFF);
    el_option(EL_TS, EL_TS_SHORT);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_timestamp_long_no_fractions(void)
{
    el_option(EL_TS_FRACT, EL_TS_FRACT_OFF);
    el_option(EL_TS, EL_TS_LONG);
    add_log(EL_FATAL, d2, 2);
    add_log(EL_FATAL, d5, 5);
    add_log(EL_FATAL, d8, 8);
    mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_different_clocks(void)
{
    int  i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (i = EL_TS_TM_CLOCK; i != EL_TS_TM_ERROR; ++i)
    {
        test_prepare();
        el_option(EL_TS_TM, i);
        el_option(EL_TS, EL_TS_LONG);
        add_log(EL_FATAL, d8, 8);
        add_log(EL_FATAL, d8, 8);
        mt_fok(pbinary_check());
        test_cleanup();
    }
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_mix_of_everything(void)
{
    int level;
    int timestamp;
    int printlevel;
    int finfo;
    int colors;
    int prefix;
    int fract;
    int nl;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (fract = EL_TS_FRACT_OFF;   fract != EL_TS_FRACT_ERROR;   ++fract)
    for (level = EL_FATAL;          level <= EL_DBG;              ++level)
    for (timestamp = EL_TS_OFF;     timestamp != EL_TS_ERROR;     ++timestamp)
    for (printlevel = 0;            printlevel <= 1;              ++printlevel)
    for (finfo = 0;                 finfo <= 1;                   ++finfo)
    for (colors = 0;                colors <= 1;                  ++colors)
    for (prefix = 0;                prefix <= 1;                  ++prefix)
    for (nl = 0;                    nl <= 1;                      ++nl)
    {
        test_prepare();
        el_option(EL_LEVEL, level);
        el_option(EL_TS, timestamp);
        el_option(EL_PRINT_LEVEL, printlevel);
        el_option(EL_PRINT_NL, nl);
        el_option(EL_FINFO, finfo);
        el_option(EL_COLORS, colors);
        el_option(EL_PREFIX, prefix ? "prefix" : NULL);
        el_option(EL_TS_FRACT, fract);

        add_log(EL_FATAL,  d1, 1);
        add_log(EL_ALERT,  d1, 1);
        add_log(EL_CRIT,   d1, 1);
        add_log(EL_ERROR,  d5, 5);
        add_log(EL_WARN,   d1, 1);
        add_log(EL_NOTICE, d2, 2);
        add_log(EL_INFO,   d1, 1);
        add_log(EL_DBG,    d8, 8);

        /*
         * cleanup so embedlog closes test file, which will flush data into
         * disk so pbinary_check() can read that data
         */

        el_cleanup();
        mt_fok(pbinary_check());

        test_cleanup();
    }
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_with_no_output_available(void)
{
    el_option(EL_OUT, EL_OUT_NONE);
    mt_ferr(el_pbinary(EL_INFO, d2, 2), ENODEV);
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_level_not_high_enough(void)
{
    mt_ferr(el_pbinary(EL_DBG, d8, 8), ERANGE);
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_null(void)
{
    mt_ferr(el_pbinary(EL_ALERT, NULL, 0), EINVAL);
    mt_ferr(el_pbinary(EL_ALERT, NULL, 5), EINVAL);
    mt_ferr(el_pbinary(EL_ALERT, d5,   0), EINVAL);
}


/* ==========================================================================
   ========================================================================== */


static void pbinary_truncate(void)
{
    int   i;
    char  msg[EL_BUF_MAX + 3];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    for (i = 0; i != sizeof(msg); ++i)
    {
        msg[i] = (unsigned char)i;
    }

    el_option(EL_TS, EL_TS_LONG);
    el_option(EL_TS_FRACT, EL_TS_FRACT_NS);
    add_log(EL_INFO, msg, sizeof(msg));

    /*
     * tell checker that this is truncate test
     */

    truncate_test = 1;
    mt_fok(pbinary_check());
}

#endif
/* ==========================================================================
             __               __
            / /_ ___   _____ / /_   ____ _ _____ ____   __  __ ____
           / __// _ \ / ___// __/  / __ `// ___// __ \ / / / // __ \
          / /_ /  __/(__  )/ /_   / /_/ // /   / /_/ // /_/ // /_/ /
          \__/ \___//____/ \__/   \__, //_/    \____/ \__,_// .___/
                                 /____/                    /_/
   ========================================================================== */


void el_pbinary_test_group(void)
{
#if ENABLE_BINARY_LOGS
    mt_run(pbinary_different_clocks);
    mt_run(pbinary_mix_of_everything);

    mt_prepare_test = &test_prepare;
    mt_cleanup_test = &test_cleanup;

    mt_run(pbinary_simple_message);
    mt_run(pbinary_simple_multiple_message);
    mt_run(pbinary_ts_without_fractions);
    mt_run(pbinary_timestamp_short);
    mt_run(pbinary_timestamp_long);
    mt_run(pbinary_ts_fractions_ms);
    mt_run(pbinary_ts_fractions_us);
    mt_run(pbinary_ts_fractions_ns);
    mt_run(pbinary_timestamp_short_no_fractions);
    mt_run(pbinary_timestamp_long_no_fractions);
    mt_run(pbinary_with_no_output_available);
    mt_run(pbinary_level_not_high_enough);
    mt_run(pbinary_null);
    mt_run(pbinary_truncate);
#endif
}
