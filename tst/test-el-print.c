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

#include <rb.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdint.h>
#include <math.h>

#if ENABLE_PTHREAD
#   include <pthread.h>
#endif

#include "mtest.h"
#include "stdlib.h"
#include "embedlog.h"


/* ==========================================================================
          __             __                     __   _
     ____/ /___   _____ / /____ _ _____ ____ _ / /_ (_)____   ____   _____
    / __  // _ \ / ___// // __ `// ___// __ `// __// // __ \ / __ \ / ___/
   / /_/ //  __// /__ / // /_/ // /   / /_/ // /_ / // /_/ // / / /(__  )
   \__,_/ \___/ \___//_/ \__,_//_/    \__,_/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


struct log_message
{
	const char  *file;
	size_t       line;
	const char  *func;
	int          level;
	const char  *msg;
};


mt_defs_ext();
static char       *logbuf;         /* output simulation */
static struct rb  *expected_logs;  /* array of expected logs */

/* declarations for threaded tests
 */

#if ENABLE_PTHREAD

#define prints_per_thread 1024
#define number_of_threads 32
#define msg_size 31

/* this multiplication must result in number being a power of 2
 * (2, 4, 8, 16 etc.)
 */

#define number_of_messages (prints_per_thread * number_of_threads)

static int                thread_test;
static char               msgs[number_of_threads][prints_per_thread][msg_size];
static struct log_message expected_msgs[number_of_threads];

#endif


/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ========================================================================== */


/* ==========================================================================
    Extracts thread id from message during thread test.
   ========================================================================== */


static int thread_str_to_int
(
	const char  *msg
)
{
	char        *tids;
	char         mcpy[1024];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	/* msg is in format "msg [TTT:III]" where TTT is thread id */
	strcpy(mcpy, msg);

	/* turn : into null */
	tids = strrchr(mcpy, ':');
	*tids = '\0';

	/* point tids to beginning of thread id */
	tids = strrchr(mcpy, '[') + 1;

	/* convert thread id to int */
	return atoi(tids);
}


/* ==========================================================================
    this is custom function that will be called instead of for example puts,
    after el_print constructs message.  We capture that message  and  simply
    append to logbuf, so we can analyze it later if everything is in  order.
   ========================================================================== */


static int print_to_buffer
(
	const char  *s,
	size_t       slen,
	void        *user
)
{
	strcat(user, s);

#if ENABLE_PTHREAD
	if (thread_test)
	{
		int  tid;
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		/* we can now add previously cached log info to expected logs.
		 * We do it here, so we can be sure it will go into the list
		 * in the same order as "s" into "user" (log_buf).
		 */

		tid = thread_str_to_int(s);
		rb_write(expected_logs, &expected_msgs[tid], 1);
	}
#endif

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
#define IS_DIGIT() if (!isdigit(*msg++)) { \
	fprintf(stderr, "%c not a digit in %s\n", *(msg - 1), line); \
	return -1; \
}
#define IS_CHAR(c) if (*msg++ != c) { \
	fprintf(stderr, "%c not a char %c in %s\n", *(msg - 1), c, line); \
	return -1; \
}

	static const char  *levelstr = "facewnid";
	struct log_message  expected;
	char               *msg;
	char                line[1024];
	char                tmp[1024];
	int                 i;
	int                 slevel;
	size_t              msglen;
	#if ENABLE_COLORS_EXTENDED
	static const char  *color[] =
	{
		"\033[91m",  /* fatal             light red */
		"\033[31m",  /* alert             red */
		"\033[95m",  /* critical          light magenta */
		"\033[35m",  /* error             magenta */
		"\033[93m",  /* warning           light yellow */
		"\033[92m",  /* notice            light green */
		"\033[32m",  /* information       green */
		"\033[34m",  /* debug             blue */
		"\033[0m"    /* remove all formats */
	};
	#else
	static const char  *color[] =
	{
		"\033[31m",  /* fatal             light red */
		"\033[31m",  /* alert             red */
		"\033[35m",  /* critical          light magenta */
		"\033[35m",  /* error             magenta */
		"\033[33m",  /* warning           light yellow */
		"\033[32m",  /* notice            light green */
		"\033[32m",  /* information       green */
		"\033[34m",  /* debug             blue */
		"\033[0m"    /* remove all formats */
	};
	#endif
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	msg = logbuf;
	while (rb_read(expected_logs, &expected, 1) == 1)
	{
		slevel = expected.level > EL_DBG ? EL_DBG : expected.level;

		/* put current line into buf, we will print in in event of error */
		{
			char *nl;
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


			strncpy(line, msg, sizeof(line));
			line[sizeof(line) - 1] = '\0';
			nl = strchr(line, '\n');

			if (nl != NULL)  *nl = '\0';
		}


		/* log should not have been printed due to current log
		 * level restriction. We just continue here, because if
		 * log was indeed printed, next checks will fail
		 * anyway. */
		if (expected.level > g_el.level)  continue;

		if (g_el.colors)
		{
			if (strncmp(msg, color[slevel], 5) != 0)
			{
				/* no color information or el_print generated wrong
				 * color */
				fprintf(stderr, "no color info in %s\n", line);
				return -1;
			}

			/* move msg by 5 bytes - begging of color is always
			 * 5char long */
			msg += 5;
		}

		/* check printing timestamp */
		if (g_el.timestamp == EL_TS_LONG)
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

			if (g_el.timestamp_fractions)
			{
				IS_CHAR('.');

				for (i = 0; i != 3 * g_el.timestamp_fractions; ++i)
					IS_DIGIT();
			}

			IS_CHAR(']');

		}
		else if (g_el.timestamp == EL_TS_SHORT)
		{
			IS_CHAR('[');

			if (g_el.timestamp_fractions)
			{
				while (*msg != '.')
					IS_DIGIT();

				++msg; /* skip the '.' character */

				for (i = 0; i != 3 * g_el.timestamp_fractions; ++i)
					IS_DIGIT();
			}
			else
			{
				while (*msg != ']')
					IS_DIGIT();
			}

			IS_CHAR(']');
		}
		else if (g_el.timestamp == EL_TS_OFF)
		{
			/* we check for nothing here */
		}
		else
		{
			/* wrong timestamp option value */
			fprintf(stderr, "wrong timestamp opton value in %s\n", line);
			return -1;
		}

		/* check printing file information
		*/

		if (g_el.finfo && expected.file != NULL && expected.line != 0)
		{
			char  expected_file[8192];
			char  f_func_info_separator;
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


			IS_CHAR('[');
			i = 0;

			while (*msg != ':')
			{
				tmp[i] = *msg++;
				if (i++ == EL_FLEN_MAX)
				{
					/* we didn't find ':' character withing maximum
					 * lenght of file - file is too long */
					fprintf(stderr, "':' not found in finfo in %s\n", line);
					return -1;
				}
			}

			msg++;  /* skip ':' character */
			tmp[i] = '\0';
			strcpy(expected_file, expected.file);

			if (strncmp(tmp, basename(expected_file), EL_FLEN_MAX) != 0)
			{
				/* file name in printed log is different than what
				 * we set */

				fprintf(stderr, "wrong filename %s != %s in %s\n",
						tmp, expected_file, line);
				return -1;
			}

			i = 0;

			/* when funcinfo is enabled, there should be ':' instead
			 * of closing bracked as print will look like this:
			 *
			 * [file.c:2:foo()] instead of [file.c:2] */

			f_func_info_separator = g_el.funcinfo ? ':' : ']';

			while (*msg != f_func_info_separator)
			{
				tmp[i] = *msg;
				if (i++ == EL_PRE_FINFO_LINE_MAX_LEN)
				{
					/* file line lenght is too large, or ending ']'
					 * is missing */

					fprintf(stderr, "']' not found in finfo in %s\n", line);
					return -1;
				}

				/* file line should be a number ;-) */
				IS_DIGIT();

			}

			msg++;  /* skip ']' or ':' character */
			tmp[i] = '\0';

			if (expected.line > EL_PRE_FINFO_LINE_MAX_NUM)
				expected.line = EL_PRE_FINFO_LINE_MAX_NUM;

			if ((size_t)atoi(tmp) != expected.line)
			{
				/* line number in printed log is different than
				 * what was set */
				fprintf(stderr, "wrong line number %ld != %ld in %s\n",
						(long)atoi(tmp), (long)expected.line, line);
				return -1;
			}

		}

		/* check for function information */
		if (g_el.funcinfo && expected.func != NULL)
		{
			if (g_el.finfo == 0 || expected.file == NULL ||
					expected.line == 0)
			{
				/* file info was not printed, so check for opening
				 * '[' character */
				if (*msg++ != '[')
				{
					fprintf(stderr, "missing '[' in %s\n", line);
					return -1;
				}
			}

			i = 0;

			/* search until we hit appended "()" */
			while (*msg != '(')
			{
				tmp[i] = *msg++;
				if (i++ == EL_FUNCLEN_MAX)
				{
					/* function is too long or ending (
					 * is missing */
					fprintf(stderr, "missing ( in funcinfo in %s\n", line);
					return -1;
				}
			}

			tmp[i] = '\0';
			if (strncmp(tmp, expected.func, EL_FUNCLEN_MAX) != 0)
			{
				/* function is different than what we expected */
				fprintf(stderr, "function is wrong %s != %s in %s\n",
						tmp, expected.func, line);
				return -1;
			}

			/* check closing stuff, we should see ()] */
			IS_CHAR('(');
			IS_CHAR(')');
			IS_CHAR(']');
		}

		if ((g_el.finfo && expected.file != NULL && expected.line != 0) ||
				g_el.timestamp != EL_TS_OFF ||
				(g_el.funcinfo && expected.func != NULL))
		{
			/* prefix or timestamp information is enabled, in that
			 * case we check for additional space between info and
			 * log message */
			if (*msg++ != ' ')
			{
				fprintf(stderr, "missing space delimiter in %s\n", line);
				return -1;
			}
		}

		/* check printing log level */
		if (g_el.print_log_level)
		{
			if (*msg++ != levelstr[slevel])
			{
				fprintf(stderr, "wrong level char %c != %c in %s\n",
						*(msg - 1), levelstr[slevel], line);
				return -1;
			}

			if (*msg++ != '/')
			{
				fprintf(stderr, "missing '/' near log level in %s\n", line);
				return -1;
			}
		}

		/* check for prefix */
	#if ENABLE_PREFIX
		if (g_el.prefix)
		{
			char expected_prefix[EL_PREFIX_LEN + 1] = {0};
			size_t expected_prefix_len;
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

			strncat(expected_prefix, g_el.prefix, EL_PREFIX_LEN);
			expected_prefix_len = strlen(expected_prefix);

			if (strncmp(expected_prefix, msg, expected_prefix_len) != 0)
			{
				fprintf(stderr, "wrong prefix expected %s in %s\n",
						expected_prefix, line);
				return -1;
			}

			msg += expected_prefix_len;
		}
	#endif

		msglen = strlen(expected.msg);

		if (strncmp(msg, expected.msg, msglen) != 0)
		{
			fprintf(stderr, "log msg is bad, expected %s in %s\n",
					expected.msg, line);
			return -1;
		}

		msg += msglen;

		if (g_el.colors)
		{
			if (strncmp(msg, color[8], 4) != 0)
			{
				/* expected end of color, got some shit */
				fprintf(stderr, "missing color ending code in %s\n", line);
				return -1;
			}

			/* end of color is always 4 bytes long */
			msg += 4;
		}

		if (g_el.print_newline)
		{
			if (*msg != '\n')
			{
				/* when new line is enabled, log should end with
				 * new line */

				fprintf(stderr, "missing new line char in %s\n", line);
				return -1;
			}

			/* set msg to point to next message, there is no need
			 * to move msg pointer if newline is not printed as msg
			 * already points to next message */
			++msg;
		}
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
	const char    *func,
	enum el_level  level,
	const char    *msg
)
{
#if ENABLE_PTHREAD
	if (thread_test == 0)
#endif
	{
		struct log_message  expected;
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		expected.file = file;
		expected.line = line;
		expected.func = func;
		expected.level = level;
		expected.msg = msg;
		rb_write(expected_logs, &expected, 1);
	}
#if ENABLE_PTHREAD
	else
	{
		int    tid;
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		/* when threaded test is performed, cache expected logs and
		 * add them in custom print function. This is due to the
		 * fact, that multiple threads can get here, and expected
		 * logs buffer can be in different order than logs in
		 * log_buf. Custom print function should be protected with
		 * mutexes so no such situation should happen there.
		 *
		 * msg is in format "msg [TTT:III]" where TTT is thread id */

		tid = thread_str_to_int(msg);

		/* now cache entries */
		expected_msgs[tid].file = file;
		expected_msgs[tid].line = line;
		expected_msgs[tid].func = func;
		expected_msgs[tid].level = level;
		expected_msgs[tid].msg = msg;
	}
#endif

	el_print(file, line, func, level, msg);
}


/* ==========================================================================
    Called before every test,  initializes  embedlog  to  known  state,  and
    allocates memory for expected_logs buffer
   ========================================================================== */


static void test_prepare(void)
{
	el_init();
	logbuf = calloc(1, 1024 * 1024);
	el_option(EL_CUSTOM_PUTS, print_to_buffer, logbuf);
	el_option(EL_PRINT_LEVEL, 0);
	el_option(EL_OUT, EL_OUT_CUSTOM);
	expected_logs = rb_new(1024, sizeof(struct log_message), 0);
}


/* ==========================================================================
    Called after every test. Frees all memory allocated by test_prepare
   ========================================================================== */


static void test_cleanup(void)
{
	el_cleanup();
	free(logbuf);
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


static void print_simple_message_no_newline(void)
{
	el_option(EL_PRINT_NL, 0);
	add_log(ELF, "print simple no newline");
	add_log(ELF, "print simple no newline second");
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
	el_option(EL_LEVEL, EL_DBG);
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
	el_option(EL_LEVEL, EL_DBG);
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


static void print_timestamp_short_no_fractions(void)
{
	el_option(EL_TS_FRACT, EL_TS_FRACT_OFF);
	el_option(EL_TS, EL_TS_SHORT);
	add_log(ELF, "first meaningless message");
	add_log(ELF, "second stupid log");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_long_no_fractions(void)
{
	el_option(EL_TS_FRACT, EL_TS_FRACT_OFF);
	el_option(EL_TS, EL_TS_LONG);
	add_log(ELF, "they don't even care");
	add_log(ELF, "what I put in here");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_short_fractions_ms(void)
{
	el_option(EL_TS_FRACT, EL_TS_FRACT_MS);
	el_option(EL_TS, EL_TS_SHORT);
	add_log(ELF, "first meaningless message");
	add_log(ELF, "second stupid log");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_long_fractions_ms(void)
{
	el_option(EL_TS_FRACT, EL_TS_FRACT_MS);
	el_option(EL_TS, EL_TS_LONG);
	add_log(ELF, "they don't even care");
	add_log(ELF, "what I put in here");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_short_fractions_us(void)
{
	el_option(EL_TS_FRACT, EL_TS_FRACT_US);
	el_option(EL_TS, EL_TS_SHORT);
	add_log(ELF, "first meaningless message");
	add_log(ELF, "second stupid log");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_long_fractions_us(void)
{
	el_option(EL_TS_FRACT, EL_TS_FRACT_US);
	el_option(EL_TS, EL_TS_LONG);
	add_log(ELF, "they don't even care");
	add_log(ELF, "what I put in here");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_short_fractions_ns(void)
{
	el_option(EL_TS_FRACT, EL_TS_FRACT_NS);
	el_option(EL_TS, EL_TS_SHORT);
	add_log(ELF, "first meaningless message");
	add_log(ELF, "second stupid log");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_timestamp_long_fractions_ns(void)
{
	el_option(EL_TS_FRACT, EL_TS_FRACT_NS);
	el_option(EL_TS, EL_TS_LONG);
	add_log(ELF, "they don't even care");
	add_log(ELF, "what I put in here");
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


#if ENABLE_PTHREAD

static void *print_t
(
	void  *arg
)
{
	int    i;
	int    n;
	char  *m;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	n = *(int *)arg;

	for (i = 0; i != prints_per_thread; ++i)
	{
		m = msgs[n][i];
		sprintf(m, "print_t [%d:%d]", n, i);
		add_log(ELF, m);
	}

	return NULL;
}

static void print_threaded(void)
{
	pthread_t  pt[number_of_threads];
	int        i, n[number_of_threads];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_init();

	/* allocate memory that can hold all messages with full
	 * pre information (timestamp, file info) and message of length
	 * 31 characters
	 */

	logbuf = calloc(1, number_of_messages *
			(EL_PRE_LEN + EL_COLORS_LEN + msg_size));
	expected_logs = rb_new(number_of_messages, sizeof(struct log_message), 0);

	el_option(EL_CUSTOM_PUTS, print_to_buffer, logbuf);
	el_option(EL_PRINT_LEVEL, 1);
	el_option(EL_TS, EL_TS_LONG);
	el_option(EL_TS_TM, EL_TS_TM_REALTIME);
	el_option(EL_TS_FRACT, EL_TS_FRACT_US);
	el_option(EL_FINFO, 1);
	el_option(EL_FUNCINFO, 1);
	el_option(EL_COLORS, 1);
	el_option(EL_PREFIX, "thread-test");
	el_option(EL_OUT, EL_OUT_CUSTOM);
	el_option(EL_LEVEL, EL_DBG);
	el_option(EL_THREAD_SAFE, 1);

	for (i = 0; i != number_of_threads; ++i)
	{
		n[i] = i;
		pthread_create(&pt[i], NULL, print_t, &n[i]);
	}

	for (i = 0; i != number_of_threads; ++i)
		pthread_join(pt[i], NULL);

	mt_fok(print_check());
	rb_destroy(expected_logs);
	free(logbuf);

	el_cleanup();
}

#endif /* ENABLE_PTHREAD */


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


static void print_mix_of_everything_check(void)
{
	mt_fok(print_check());
}

static void print_mix_of_everything(void)
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
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


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
		test_prepare();
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

		add_log(ELF, "fatal message");
		add_log(ELA, "alert message");
		add_log(ELC, "critical message");
		add_log(ELE, "error message");
		add_log(ELW, "warning message");
		add_log(ELN, "notice message");
		add_log(ELI, "info message");
		add_log(ELD, "debug message");

		sprintf(tname, "print_mix_of_everything: level: %d, colors: %d, "
				"ts: %d, ts_tm: %d, ts_fract: %d, print_level: %d, "
				"nl: %d, finfo: %d, funcinfo: %d, prefix: %d",
				level, colors, ts, ts_tm, ts_fract, printlevel, nl,
				finfo, funcinfo, prefix);
		mt_run_named(print_mix_of_everything_check, tname);

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
	msg[sizeof(msg) - 5] = '0';

	add_log(ELI, "not truncated");
	add_log(ELI, msg);

	/* while el_print will make copy of msg, our test print_check
	 * function will just use pointer to our msg here, and since we
	 * expect message to be truncated, we truncate it here and
	 * print_check will take this truncated message as expected
	 * one. */

	msg[sizeof(msg) - 3] = '\0';

	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_too_long_print_truncate_no_newline(void)
{
	char  msg[EL_LOG_MAX + 3];
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_option(EL_PRINT_NL, 0);
	memset(msg, 'a', sizeof(msg));
	msg[sizeof(msg) - 1] = '\0';
	msg[sizeof(msg) - 2] = '3';
	msg[sizeof(msg) - 3] = '2';
	msg[sizeof(msg) - 4] = '1';
	msg[sizeof(msg) - 5] = '0';

	add_log(ELI, "not truncated");
	add_log(ELI, msg);

	/* while el_print will make copy of msg, our test print_check
	 * function will just use pointer to our msg here, and since we
	 * expect message to be truncated, we truncate it here and
	 * print_check will take this truncated message as expected
	 * one. */

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
	char   msg[EL_LOG_MAX + 3];
	char   finfo[EL_FLEN_MAX + 3];
	char   funcinfo[EL_FUNCLEN_MAX + 3];
	char   prefix[EL_PREFIX_MAX + 3];
	size_t fline;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_option(EL_TS, EL_TS_LONG);
	el_option(EL_TS_FRACT, EL_TS_FRACT_NS);
	el_option(EL_TS_TM, EL_TS_TM_REALTIME);
	el_option(EL_FINFO, 1);
	el_option(EL_FUNCINFO, 1);
	el_option(EL_COLORS, 1);
	el_option(EL_PREFIX, prefix);
	el_option(EL_PRINT_LEVEL, 1);
	memset(msg, 'a', sizeof(msg));
	memset(finfo, 'b', sizeof(finfo));
	memset(prefix, 'c', sizeof(prefix));
	memset(funcinfo, 'f', sizeof(funcinfo));
	finfo[sizeof(finfo) - 1] = '\0';
	prefix[sizeof(prefix) - 1] = '\0';
	funcinfo[sizeof(funcinfo) - 1] = '\0';
	fline = (size_t)pow(10, EL_PRE_FINFO_LINE_MAX_LEN + 2) - 1;
	msg[sizeof(msg) - 1] = '\0';
	msg[sizeof(msg) - 2] = '3';
	msg[sizeof(msg) - 3] = '2';
	msg[sizeof(msg) - 4] = '1';
	msg[sizeof(msg) - 4] = '0';

	add_log(ELI, "not truncated");
	add_log(finfo, fline, funcinfo, EL_FATAL, msg);

	msg[sizeof(msg) - 3] = '\0';
	puts(logbuf);

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
	add_log("source/code/file.c", 10, "print_finfo_path", EL_ALERT,
			"some message");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_nofinfo(void)
{
	el_option(EL_FINFO, 1);
	add_log(NULL, 0, NULL, EL_ALERT, "no file info");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_null(void)
{
	mt_ferr(el_print(ELA, NULL), EINVAL);
}


/* ==========================================================================
   ========================================================================== */


static void print_prefix(void)
{
	el_option(EL_PREFIX, "prefix");
	add_log(ELI, "message with prefix");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_prefix_full(void)
{
	char p[EL_PREFIX_LEN + 1] = {0};
	int i;

	for (i = 0; i != EL_PREFIX_LEN; ++i)
		p[i] = '0' + (i % 32);

	el_option(EL_PREFIX, p);
	add_log(ELI, "message with fill prefix");
	mt_fok(print_check());
}


/* ==========================================================================
   ========================================================================== */


static void print_prefix_overflow(void)
{
	char p[EL_PREFIX_LEN + 10] = {0};
	int i;

	for (i = 0; i != sizeof(p) - 1; ++i)
		p[i] = '0' + (i % 32);

	el_option(EL_PREFIX, p);
	add_log(ELI, "message with overflown prefix");
	mt_fok(print_check());
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
#if ENABLE_PTHREAD
	thread_test = 1;
	mt_run(print_threaded);
	thread_test = 0;
#endif

	mt_run(print_different_clocks);
	print_mix_of_everything();

	mt_prepare_test = &test_prepare;
	mt_cleanup_test = &test_cleanup;

	mt_run(print_simple_message);
	mt_run(print_simple_message_no_newline);
	mt_run(print_simple_multiple_message);
	mt_run(print_log_level);
	mt_run(print_colorful_output);
	mt_run(print_custom_log_level);
	mt_run(print_timestamp_short);
	mt_run(print_timestamp_long);
	mt_run(print_timestamp_short_no_fractions);
	mt_run(print_timestamp_long_no_fractions);
	mt_run(print_timestamp_short_fractions_ms);
	mt_run(print_timestamp_long_fractions_ms);
	mt_run(print_timestamp_short_fractions_us);
	mt_run(print_timestamp_long_fractions_us);
	mt_run(print_timestamp_short_fractions_ns);
	mt_run(print_timestamp_long_fractions_ns);
	mt_run(print_finfo);
	mt_run(print_too_long_print_truncate);
	mt_run(print_too_long_print_truncate_no_newline);
	mt_run(print_truncate_with_date);
	mt_run(print_truncate_with_all_options);
	mt_run(print_with_no_output_available);
	mt_run(print_level_not_high_enough);
	mt_run(print_finfo_path);
	mt_run(print_nofinfo);
	mt_run(print_null);
	mt_run(print_prefix);
	mt_run(print_prefix_full);
	mt_run(print_prefix_overflow);
}
