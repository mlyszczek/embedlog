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

#if ENABLE_BINARY_LOGS


/* ==========================================================================
          __             __                     __   _
     ____/ /___   _____ / /____ _ _____ ____ _ / /_ (_)____   ____   _____
    / __  // _ \ / ___// // __ `// ___// __ `// __// // __ \ / __ \ / ___/
   / /_/ //  __// /__ / // /_/ // /   / /_/ // /_ / // /_/ // / / /(__  )
   \__,_/ \___/ \___//_/ \__,_//_/    \__,_/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


struct log_message
{
	const unsigned char  *msg;
	size_t                msglen;
	int                   level;
};


mt_defs_ext();
static const char   *logpath = "./embedlog-pbinary-test";
static char          logbuf[1024];
static struct rb    *expected_logs;        /* array of expected logs */
static int           truncate_test;
static int           store_to_file;

static unsigned char  d1[] = { 0x01 };
static unsigned char  d2[] = { 0x53, 0x10 };
static unsigned char  d3[] = { 0x00, 0x10, 0x12 };
static unsigned char  d5[] = {  'f',  'o',  'o', 0x00,  'b' };
static unsigned char  d8[] = { 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00 };


/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ========================================================================== */


static int print_mix
(
	const char  *s,
	size_t       slen,
	void        *user
)
{
	int          pos;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	pos = *(int *)user;
	memcpy(logbuf + pos, s, slen);
	*(int *)user += slen;
	return 0;
}


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


	if (store_to_file)
	{
		stat(logpath, &st);
		fd = open(logpath, O_RDONLY);
		msg = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		msgsave = msg;
	}
	else
	{
		msg = (unsigned char *)logbuf;
	}

	while (rb_read(expected_logs, &expected, 1) == 1)
	{
		slevel = expected.level > EL_DBG ? EL_DBG : expected.level;

		/* log should not have been printed due to current log
		 * level restriction. We just continue here, because if
		 * log was indeed printed, next checks will fail
		 * anyway. */
		if (expected.level > g_el.level)  continue;

		/* read flags */
		flags = *msg++;

		/* check if level is added to flags */
		if ((flags >> 3) != expected.level)
		{
			/* no, its not set correctly */
			fprintf(stderr, "wrong level, exp: %d, got: %d\n",
					expected.level, flags >> 3);
			goto error;
		}

		/* check printing timestamp */
		if (g_el.timestamp != EL_TS_OFF)
		{
#   ifdef LLONG_MAX
			unsigned long long tmp;
#   else
			unsigned long      tmp;
#   endif
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

			len = el_decode_number(msg, &tmp);
			msg += len;

			/* we don't know exact time embedlog put into log file,
			 * so we have to trust the size */
			if ((flags & 0x01) != 0x01)
			{
				/* expected timestamp flag to be set */
				fprintf(stderr, "flag timestamp not set: %d\n", flags);
				goto error;
			}

			/* fraction of seconds checks makes sense only when ts
			 * is set */
			if (g_el.timestamp_fractions == EL_TS_FRACT_OFF)
			{
				if (flags & 0x06)
				{
					/* fraction flags set, and should not be */
					fprintf(stderr, "fraction flag set (should not be): %d\n",
							flags);
					goto error;
				}
			}
			else
			{
				len = el_decode_number(msg, &tmp);
				msg += len;
			}

			if (g_el.timestamp_fractions == EL_TS_FRACT_MS)
			{
				if (tmp > 999)
				{
					fprintf(stderr, "msec set, but value > 999: %ld\n",
							(long)tmp);
					goto error;
				}

				if (((flags & 0x06) >> 1) != 1)
				{
					fprintf(stderr, "msec flag not set: %d\n", flags);
					goto error;
				}
			}

			if (g_el.timestamp_fractions == EL_TS_FRACT_US)
			{
				if (tmp > 999999l)
				{
					fprintf(stderr, "usec set, but value > 999999: %ld\n",
							(long)tmp);
					goto error;
				}

				if (((flags & 0x06) >> 1) != 2)
				{
					fprintf(stderr, "usec flag not set: %d\n", flags);
					goto error;
				}
			}

			if (g_el.timestamp_fractions == EL_TS_FRACT_NS)
			{
				if (tmp > 999999999l)
				{
					fprintf(stderr, "nsec set, but value > 999999999: %ld\n",
							(long)tmp);
					goto error;
				}

				if (((flags & 0x06) >> 1) != 3)
				{
					fprintf(stderr, "nsec flag not set: %d\n", flags);
					goto error;
				}
			}

		}
		else
		{
			if (flags & 0x01)
			{
				/* we didn't expect timestamp flag to be set */
				fprintf(stderr, "timestamp flag set (shouldn't be): %d\n",
						flags);
				goto error;
			}

			if (flags & 0x06)
			{
				/* when timestamp is not set, we expect fraction of
				 * seconds to not be set as well */
				fprintf(stderr, "timestamp flag not set but fraction is: %d\n",
						flags);
				goto error;
			}
		}

		/* now we can check printed data */
		if (truncate_test)
		{
			int i;
			int written;
			char expect[EL_BUF_MAX];
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

			for (i = 0; i != sizeof(expect); ++i)
				expect[i] = (unsigned char)i;

			written = msg - msgsave;

			len = el_decode_number(msg, &val);

			if (val != EL_BUF_MAX - written - len)
			{
				/* truncated message has wrong length */
				fprintf(stderr, "truncated message wrong length "
						"exp: %lu, got %lu\n", EL_BUF_MAX - written - len,
						(long)val);
				return -1;
			}

			msg += len;

			if (memcmp(msg, expect, val) != 0)
			{
				/* data is incorrect */
				fprintf(stderr, "data is incorrect\n");
				return -1;
			}

			/* point to next message */
			msg += val;
		}
		else
		{
			len = el_decode_number(msg, &val);
			msg += len;

			if (memcmp(msg, expected.msg, val) != 0)
			{
				/* data different than expected */
				fprintf(stderr, "data different than expected\n");
				goto error;
			}

			/* finally set msg to point to next message */
			msg += val;
		}
	}

	if (store_to_file)
	{
		munmap(msgsave, st.st_size);
		close(fd);
	}
	return 0;

error:
	if (store_to_file)
	{
		munmap(msgsave, st.st_size);
		close(fd);
	}
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
	el_option(EL_FSYNC_EVERY, 1024);
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


static void pbinary_mix_of_everything_check(void)
{
	mt_fok(pbinary_check());
}

static void pbinary_mix_of_everything(void)
{
	int   level;
	int   ts;
	int   printlevel;
	int   finfo;
	int   funcinfo;
	int   colors;
	int   prefix;
	int   ts_fract;
	int   nl;
	int   ts_tm;
	char  tname[512];
	int   pos;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	expected_logs = rb_new(16, sizeof(struct log_message), 0);
	truncate_test = 0;

	for (level = 0;        level <= EL_DBG;                ++level)
	for (colors = 0;       colors <= 1;                    ++colors)
	for (ts = 0;           ts != EL_TS_ERROR;              ++ts)
	for (ts_tm = 0;        ts_tm <= EL_TS_TM_ERROR;        ++ts_tm)
	for (ts_fract = 0;     ts_fract != EL_TS_FRACT_ERROR;  ++ts_fract)
	for (printlevel = 0;   printlevel <= 1;                ++printlevel)
	for (nl = 0;           nl <= 1;                        ++nl)
	for (finfo = 0;        finfo <= 1;                     ++finfo)
	for (funcinfo = 0;     funcinfo <= 1;                  ++funcinfo)
	for (prefix = 0;       prefix <= 1;                    ++prefix)
	{
		el_init();

		el_option(EL_LEVEL, level);
		el_option(EL_COLORS, colors);
		el_option(EL_TS, ts);
		el_option(EL_TS_TM, ts_tm);
		el_option(EL_TS_FRACT, ts_fract);
		el_option(EL_PRINT_LEVEL, printlevel);
		el_option(EL_PRINT_NL, nl);
		el_option(EL_FINFO, finfo);
		el_option(EL_FUNCINFO, funcinfo);
		el_option(EL_PREFIX, prefix ? "prefix" : NULL);
		el_option(EL_CUSTOM_PUT, print_mix, &pos);
		el_option(EL_OUT, EL_OUT_CUSTOM);

		pos = 0;
		add_log(EL_FATAL,  d1, 1);
		add_log(EL_ALERT,  d1, 1);
		add_log(EL_CRIT,   d1, 1);
		add_log(EL_ERROR,  d5, 5);
		add_log(EL_WARN,   d1, 1);
		add_log(EL_NOTICE, d2, 2);
		add_log(EL_INFO,   d1, 1);
		add_log(EL_DBG,    d8, 8);

		el_cleanup();

		sprintf(tname, "pbinary_mix_of_everything: level: %d, colors: %d, "
				"ts: %d, ts_tm: %d, ts_fract: %d, print_level: %d, "
				"nl: %d, finfo: %d, funcinfo: %d, prefix: %d",
				level, colors, ts, ts_tm, ts_fract, printlevel, nl,
				finfo, funcinfo, prefix);

		mt_run_named(pbinary_mix_of_everything_check, tname);
		rb_clear(expected_logs, 0);
	}

	rb_destroy(expected_logs);
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
		msg[i] = (unsigned char)i;

	el_option(EL_TS, EL_TS_LONG);
	el_option(EL_TS_FRACT, EL_TS_FRACT_NS);
	add_log(EL_INFO, msg, sizeof(msg));

	/* tell checker that this is truncate test */
	truncate_test = 1;
	mt_fok(pbinary_check());
}


/* ==========================================================================
   ========================================================================== */


#if ENABLE_PTHREAD

#define total_prints 125000
#define line_length (2 * sizeof(int) + 2)
#define number_of_threads 25

#define file_size (total_prints * line_length)
#define prints_per_rotate (file_size / line_length)
#define prints_per_thread (total_prints / number_of_threads)

static int sort_pbinary
(
	const void  *p1,
	const void  *p2
)
{
	int          m1[2];
	int          m2[2];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	memcpy(m1, (unsigned char *)p1 + 2, sizeof(m1));
	memcpy(m2, (unsigned char *)p2 + 2, sizeof(m2));

	if (m1[0] < m2[0])  return -1;
	if (m1[0] > m2[0])  return 1;

	/* thread id equal, let message id decide euqualness */
	if (m1[1] < m2[1])  return -1;
	if (m1[1] > m2[1])  return 1;

	/* everything is equal, there is bug, upper layer will
	 * take care of it */
	fprintf(stderr, "sort_pbinary, m1 == m2\n");
	return 0;
}


static int print_check(void)
{
	struct stat     st;
	unsigned char  *msg;
	unsigned char  *msgsave;
	int             fd;
	unsigned char   flags;
	unsigned char   len;
	int             tid;
	int             mid;
	int             curr_tid;
	int             curr_mid;
	unsigned        i;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	stat(logpath, &st);

	if (st.st_size != file_size)
	{
		fprintf(stderr, "file size different than expected\n");
		return -1;
	}

	fd = open(logpath, O_RDONLY);
	msg = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	msgsave = msg;
	curr_tid = 0;
	curr_mid = 0;


	qsort(msg, total_prints, line_length, sort_pbinary);

	for (i = 0; i < file_size; i += line_length)
	{
		flags = msg[i + 0];
		len = msg[i + 1];
		memcpy(&tid, &msg[i + 2], sizeof(tid));
		memcpy(&mid, &msg[i + 6], sizeof(mid));

		/* flag should contain only severity level (info == 6) */
		if (flags != 6 << 3)
		{
			fprintf(stderr, "incorect flags: %d\n", flags);
			munmap(msgsave, st.st_size);
			close(fd);
			return -1;
		}

		if (len != 2 * sizeof(int))
		{
			fprintf(stderr, "incorect line length: %d\n", len);
			munmap(msgsave, st.st_size);
			close(fd);
			return -1;
		}

		if (tid != curr_tid)
		{
			fprintf(stderr, "msg_tid (%d) != curr_tid (%d)\n",
					tid, curr_tid);
			munmap(msgsave, st.st_size);
			close(fd);
			return -1;
		}

		if (mid != curr_mid)
		{
			fprintf(stderr, "msg_mid (%d) != curr_mid (%d)\n",
					mid, curr_mid);
			munmap(msgsave, st.st_size);
			close(fd);
			return -1;
		}

		if (++curr_mid == prints_per_thread)
		{
			++curr_tid;
			curr_mid = 0;
		}
	}

	if (curr_tid != number_of_threads || curr_mid != 0)
	{
		fprintf(stderr, "file didn't contain all entries, tid: %d, mid: %d\n",
				curr_tid, curr_mid);
		munmap(msgsave, st.st_size);
		close(fd);
		return -1;
	}

	munmap(msgsave, st.st_size);
	close(fd);
	return 0;
}

static void *print_t
(
	void  *arg
)
{
	int    msg[2];
	int    i;
	int    n;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	n = *(int *)arg;

	for (i = 0; i != prints_per_thread; ++i)
	{
		msg[0] = n;
		msg[1] = i;
		el_pbinary(EL_INFO, msg, sizeof(msg));
	}

	return NULL;
}


static void prbinary_threaded(void)
{
	pthread_t  pt[number_of_threads];
	int        i, j, n[number_of_threads];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	unlink(logpath);
	el_init();

	el_option(EL_THREAD_SAFE, 1);
	el_option(EL_PRINT_LEVEL, 0);
	el_option(EL_OUT, EL_OUT_FILE);
	el_option(EL_FROTATE_NUMBER, 0);
	el_option(EL_FPATH, logpath);
	el_option(EL_FSYNC_EVERY, file_size / 4);

	for (i = 0; i != number_of_threads; ++i)
	{
		n[i] = i;
		pthread_create(&pt[i], NULL, print_t, &n[i]);
	}

	for (i = 0; i != number_of_threads; ++i)
		pthread_join(pt[i], NULL);

	/* do it before print_check() so any buffered data is flushed
	 * to disk, otherwise files will be incomplete
	 */

	el_flush();
	mt_fok(print_check());
	el_cleanup();
}

#endif /* ENABLE_PTHREAD */
#endif /* ENABLE_BINARY_LOGS */



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
	pbinary_mix_of_everything();

	store_to_file = 1;

#if ENABLE_PTHREAD

	mt_run(prbinary_threaded);

#endif

	mt_run(pbinary_different_clocks);

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
