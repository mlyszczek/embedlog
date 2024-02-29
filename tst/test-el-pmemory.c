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

#include <errno.h>
#include <string.h>

#include "mtest.h"
#include "embedlog.h"

#if ENABLE_PTHREAD
#   include <pthread.h>
#endif

/* ==========================================================================
          __             __                     __   _
     ____/ /___   _____ / /____ _ _____ ____ _ / /_ (_)____   ____   _____
    / __  // _ \ / ___// // __ `// ___// __ `// __// // __ \ / __ \ / ___/
   / /_/ //  __// /__ / // /_/ // /   / /_/ // /_ / // /_/ // / / /(__  )
   \__,_/ \___/ \___//_/ \__,_//_/    \__,_/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


mt_defs_ext();
static char  logbuf[1024 * 1024];  /* output simulation */
static char  ascii[128];


/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ========================================================================== */


/* ==========================================================================
    this is custom function that will be called instead of for example puts
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
	return 0;
}


/* ==========================================================================
   ========================================================================== */


static void test_prepare(void)
{
	el_init();
	el_option(EL_CUSTOM_PUTS, print_to_buffer, logbuf);
	el_option(EL_PRINT_LEVEL, 0);
	el_option(EL_OUT, EL_OUT_CUSTOM);
	logbuf[0] = '\0';
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


static void pmemory_one_byte(void)
{
	static const char c = 'a';
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_pmemory(ELI, &c, sizeof(c));
	mt_fok(strcmp(logbuf,
"0x0000  61                                               a\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_one_line_not_full(void)
{
	static const char  *s = "test string";
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	el_pmemory(ELI, s, strlen(s));
	mt_fok(strcmp(logbuf,
"0x0000  74 65 73 74 20 73 74 72 69 6e 67                 test string\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_one_line_full(void)
{
	static const char  *s = "qwertyuiopasdfgh";
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_pmemory(ELI, s, strlen(s));
	mt_fok(strcmp(logbuf,
"0x0000  71 77 65 72 74 79 75 69 6f 70 61 73 64 66 67 68  qwertyuiopasdfgh\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_two_line_not_full(void)
{
	static const char  *s = "first line i am and i am 2";
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_pmemory(ELI, s, strlen(s));
	mt_fok(strcmp(logbuf,
"0x0000  66 69 72 73 74 20 6c 69 6e 65 20 69 20 61 6d 20  first line i am \n"
"0x0010  61 6e 64 20 69 20 61 6d 20 32                    and i am 2\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_two_line_full(void)
{
	static const char  *s = "first line i am and i am 2nd lin";
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_pmemory(ELI, s, strlen(s));
	mt_fok(strcmp(logbuf,
"0x0000  66 69 72 73 74 20 6c 69 6e 65 20 69 20 61 6d 20  first line i am \n"
"0x0010  61 6e 64 20 69 20 61 6d 20 32 6e 64 20 6c 69 6e  and i am 2nd lin\n"));
}


static void pmemory_binary_data_with_nulls(void)
{
	static const unsigned char  data[] = {
		0x05, 0x02, 0x10, 0x50, 0x53, 0x8f, 0xff, 0x3d,
		0x00, 0x00, 0x4a, 0xab, 0xfc, 0x04, 0x00, 0xfa,
		0xfd, 0xfa, 0x7f, 0x80 };
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_pmemory(ELI, data, sizeof(data));
	mt_fok(strcmp(logbuf,
"0x0000  05 02 10 50 53 8f ff 3d 00 00 4a ab fc 04 00 fa  ...PS..=..J.....\n"
"0x0010  fd fa 7f 80                                      ....\n"));
}

/* ==========================================================================
   ========================================================================== */


static void pmemory_print_ascii_table(void)
{
	el_pmemory(ELI, ascii, sizeof(ascii));
	mt_fok(strcmp(logbuf,
"0x0000  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f  ................\n"
"0x0010  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f  ................\n"
"0x0020  20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f   !\"#$%&'()*+,-./\n"
"0x0030  30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f  0123456789:;<=>?\n"
"0x0040  40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO\n"
"0x0050  50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f  PQRSTUVWXYZ[\\]^_\n"
"0x0060  60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f  `abcdefghijklmno\n"
"0x0070  70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f  pqrstuvwxyz{|}~.\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_print_ascii_table_with_table(void)
{
	el_pmemory_table(ELI, ascii, sizeof(ascii));
	mt_fok(strcmp(logbuf,
"------  -----------------------------------------------  ----------------\n"
"offset  hex                                              ascii\n"
"------  -----------------------------------------------  ----------------\n"
"0x0000  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f  ................\n"
"0x0010  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f  ................\n"
"0x0020  20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f   !\"#$%&'()*+,-./\n"
"0x0030  30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f  0123456789:;<=>?\n"
"0x0040  40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO\n"
"0x0050  50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f  PQRSTUVWXYZ[\\]^_\n"
"0x0060  60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f  `abcdefghijklmno\n"
"0x0070  70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f  pqrstuvwxyz{|}~.\n"
"------  -----------------------------------------------  ----------------\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_test_spacing_1(void)
{
	el_option(EL_PMEMORY_SPACE, 1);
	el_pmemory(ELI, ascii, 32 + 10);
	mt_fok(strcmp(logbuf,
"0x0000  00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  ........ ........\n"
"0x0010  10 11 12 13 14 15 16 17  18 19 1a 1b 1c 1d 1e 1f  ........ ........\n"
"0x0020  20 21 22 23 24 25 26 27  28 29                     !\"#$%&' ()\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_test_spacing_2(void)
{
	el_option(EL_PMEMORY_SPACE, 2);
	el_pmemory(ELI, ascii, 32 + 10);
	mt_fok(strcmp(logbuf,
"0x0000  00 01 02 03 04 05 06 07   08 09 0a 0b 0c 0d 0e 0f  ........  ........\n"
"0x0010  10 11 12 13 14 15 16 17   18 19 1a 1b 1c 1d 1e 1f  ........  ........\n"
"0x0020  20 21 22 23 24 25 26 27   28 29                     !\"#$%&'  ()\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_test_spacing_3(void)
{
	el_option(EL_PMEMORY_SPACE, 3);
	el_pmemory(ELI, ascii, 32 + 10);
	mt_fok(strcmp(logbuf,
"0x0000  00 01 02 03 04 05 06 07    08 09 0a 0b 0c 0d 0e 0f  ........   ........\n"
"0x0010  10 11 12 13 14 15 16 17    18 19 1a 1b 1c 1d 1e 1f  ........   ........\n"
"0x0020  20 21 22 23 24 25 26 27    28 29                     !\"#$%&'   ()\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_test_spacing_table_1(void)
{
	el_option(EL_PMEMORY_SPACE, 1);
	el_pmemory_table(ELI, ascii, 32 + 10);
	mt_fok(strcmp(logbuf,
"------  ------------------------------------------------  -----------------\n"
"offset  hex                                               ascii\n"
"------  ------------------------------------------------  -----------------\n"
"0x0000  00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  ........ ........\n"
"0x0010  10 11 12 13 14 15 16 17  18 19 1a 1b 1c 1d 1e 1f  ........ ........\n"
"0x0020  20 21 22 23 24 25 26 27  28 29                     !\"#$%&' ()\n"
"------  ------------------------------------------------  -----------------\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_test_spacing_table_2(void)
{
	el_option(EL_PMEMORY_SPACE, 2);
	el_pmemory_table(ELI, ascii, 32 + 10);
	mt_fok(strcmp(logbuf,
"------  -------------------------------------------------  ------------------\n"
"offset  hex                                                ascii\n"
"------  -------------------------------------------------  ------------------\n"
"0x0000  00 01 02 03 04 05 06 07   08 09 0a 0b 0c 0d 0e 0f  ........  ........\n"
"0x0010  10 11 12 13 14 15 16 17   18 19 1a 1b 1c 1d 1e 1f  ........  ........\n"
"0x0020  20 21 22 23 24 25 26 27   28 29                     !\"#$%&'  ()\n"
"------  -------------------------------------------------  ------------------\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_test_spacing_table_3(void)
{
	el_option(EL_PMEMORY_SPACE, 3);
	el_pmemory_table(ELI, ascii, 32 + 10);
	mt_fok(strcmp(logbuf,
"------  --------------------------------------------------  -------------------\n"
"offset  hex                                                 ascii\n"
"------  --------------------------------------------------  -------------------\n"
"0x0000  00 01 02 03 04 05 06 07    08 09 0a 0b 0c 0d 0e 0f  ........   ........\n"
"0x0010  10 11 12 13 14 15 16 17    18 19 1a 1b 1c 1d 1e 1f  ........   ........\n"
"0x0020  20 21 22 23 24 25 26 27    28 29                     !\"#$%&'   ()\n"
"------  --------------------------------------------------  -------------------\n"));
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_print_ascii_table_option(void)
{
	struct el  el;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_oinit(&el);
	el_ooption(&el, EL_CUSTOM_PUTS, print_to_buffer, logbuf);
	el_ooption(&el, EL_PRINT_LEVEL, 0);
	el_ooption(&el, EL_OUT, EL_OUT_CUSTOM);
	logbuf[0] = '\0';

	el_opmemory(ELI, &el, ascii, sizeof(ascii));
	mt_fok(strcmp(logbuf,
"0x0000  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f  ................\n"
"0x0010  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f  ................\n"
"0x0020  20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f   !\"#$%&'()*+,-./\n"
"0x0030  30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f  0123456789:;<=>?\n"
"0x0040  40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO\n"
"0x0050  50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f  PQRSTUVWXYZ[\\]^_\n"
"0x0060  60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f  `abcdefghijklmno\n"
"0x0070  70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f  pqrstuvwxyz{|}~.\n"));

	el_ocleanup(&el);
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_print_ascii_table_with_table_option(void)
{
	struct el  el;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_oinit(&el);
	el_ooption(&el, EL_CUSTOM_PUTS, print_to_buffer, logbuf);
	el_ooption(&el, EL_PRINT_LEVEL, 0);
	el_ooption(&el, EL_OUT, EL_OUT_CUSTOM);
	logbuf[0] = '\0';

	el_opmemory_table(ELI, &el, ascii, sizeof(ascii));
	mt_fok(strcmp(logbuf,
"------  -----------------------------------------------  ----------------\n"
"offset  hex                                              ascii\n"
"------  -----------------------------------------------  ----------------\n"
"0x0000  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f  ................\n"
"0x0010  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f  ................\n"
"0x0020  20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f   !\"#$%&'()*+,-./\n"
"0x0030  30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f  0123456789:;<=>?\n"
"0x0040  40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO\n"
"0x0050  50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f  PQRSTUVWXYZ[\\]^_\n"
"0x0060  60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f  `abcdefghijklmno\n"
"0x0070  70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f  pqrstuvwxyz{|}~.\n"
"------  -----------------------------------------------  ----------------\n"));

	el_ocleanup(&el);
}


/* ==========================================================================
   ========================================================================== */


#if ENABLE_PTHREAD

static const char expected_out[] =
"0x0000  00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f  ................\n"
"0x0010  10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f  ................\n"
"0x0020  20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f   !\"#$%&'()*+,-./\n"
"0x0030  30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f  0123456789:;<=>?\n"
"0x0040  40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f  @ABCDEFGHIJKLMNO\n"
"0x0050  50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f  PQRSTUVWXYZ[\\]^_\n"
"0x0060  60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f  `abcdefghijklmno\n"
"0x0070  70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f  pqrstuvwxyz{|}~.\n";

#define max_size sizeof(logbuf)
#define number_of_threads 32
#define print_length (sizeof(expected_out) - 1)
#define prints_per_thread (max_size / print_length / number_of_threads)
#define number_of_prints (prints_per_thread * number_of_threads)

static int print_check(void)
{
	int    i;
	char  *msg;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	/* all we care about is that there are no interlaces
	*/

	for (msg = logbuf, i = 0; i != number_of_prints; ++i, msg += print_length)
	{
		if (strncmp(msg, expected_out, print_length) != 0)
		{
			fprintf(stderr, "message does not match (pass %d) msg: %s\n",
					i, msg);
			return -1;
		}
	}

	if (*msg != '\0')
	{
		fprintf(stderr, "expected null terminator on last msg byte\n");
		return -1;
	}

	return 0;
}


static void *print_t
(
	void *arg
)
{
	int   i;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	(void)arg;

	for (i = 0; i != prints_per_thread; ++i)
		el_pmemory(ELN, ascii, sizeof(ascii));

	return NULL;
}


static void pmemory_print_threaded(void)
{
	pthread_t  pt[number_of_threads];
	int        i;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_init();
	el_option(EL_THREAD_SAFE, 1);
	el_option(EL_CUSTOM_PUTS, print_to_buffer, logbuf);
	el_option(EL_PRINT_LEVEL, 0);
	el_option(EL_OUT, EL_OUT_CUSTOM);
	logbuf[0] = '\0';

	for (i = 0; i != number_of_threads; ++i)
		pthread_create(&pt[i], NULL, print_t, NULL);

	for (i = 0; i != number_of_threads; ++i)
		pthread_join(pt[i], NULL);

	/* do it before print_check() so any buffered data is flushed
	 * to disk, otherwise files will be incomplete */

	el_flush();
	mt_fok(print_check());
	el_cleanup();
}

#endif /* ENABLE_PTHREAD */


/* ==========================================================================
   ========================================================================== */


static void pmemory_null_memory(void)
{
	mt_ferr(el_pmemory(ELI, NULL, 10), EINVAL);
}


/* ==========================================================================
   ========================================================================== */


static void pmemory_zero_size(void)
{
	static const char  *s = "some data";
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	mt_ferr(el_pmemory(ELI, s, 0), EINVAL);
}


/* ==========================================================================
             __               __
            / /_ ___   _____ / /_   ____ _ _____ ____   __  __ ____
           / __// _ \ / ___// __/  / __ `// ___// __ \ / / / // __ \
          / /_ /  __/(__  )/ /_   / /_/ // /   / /_/ // /_/ // /_/ /
          \__/ \___//____/ \__/   \__, //_/    \____/ \__,_// .___/
                                 /____/                    /_/
   ========================================================================== */


void el_pmemory_test_group(void)
{
	int  i;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	for (i = 0; i != 0x80; ++i)  ascii[i] = (char)i;

#if ENABLE_PTHREAD
	mt_run(pmemory_print_threaded);
#endif

	mt_run(pmemory_print_ascii_table_option);
	mt_run(pmemory_print_ascii_table_with_table_option);

	mt_prepare_test = &test_prepare;
	mt_cleanup_test = &test_cleanup;

	mt_run(pmemory_one_byte);
	mt_run(pmemory_one_line_not_full);
	mt_run(pmemory_one_line_full);
	mt_run(pmemory_two_line_not_full);
	mt_run(pmemory_two_line_full);
	mt_run(pmemory_binary_data_with_nulls);
	mt_run(pmemory_print_ascii_table);
	mt_run(pmemory_print_ascii_table_with_table);
	mt_run(pmemory_null_memory);
	mt_run(pmemory_zero_size);
	mt_run(pmemory_test_spacing_table_1);
	mt_run(pmemory_test_spacing_table_2);
	mt_run(pmemory_test_spacing_table_3);
	mt_run(pmemory_test_spacing_1);
	mt_run(pmemory_test_spacing_2);
	mt_run(pmemory_test_spacing_3);
}
