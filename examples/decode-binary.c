/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "embedlog.h"

#define die_perror(x, ...) do { el_perror(ELF, x, __VA_ARGS__); exit(1); } while (0)

static size_t decode_number
(
	const void          *number,  /* number to decode */
	unsigned long long  *out      /* decoded number */
)
{
	const unsigned char *n;       /* just a 'number' as unsigned char */
	size_t               i;       /* temporary index */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	i = 0;
	*out = 0;
	n = number;

	do
	{
		/* multiple stuff is happening in this one-liner
		 *
		 * - first we take 7 bits of number
		 * - calculate weigth of current number
		 * - set current number into right position of out */
		*out |= (n[i] & 0x7f) << (i * 7);

		/* we do this until number lacks of continuation bit, which
		 * means we are done */
	}
	while (n[i++] & 0x80);

	/* return number of bytes processed from 'number' */

	return i;
}

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_decode_binary_main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
	int fd;
	unsigned char *log;
	struct stat st;
	ssize_t i;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	el_init();

	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <binary-file>\n", argv[0]);
		return 1;
	}

	if (stat(argv[1], &st) != 0)
		die_perror("stat(%s)", argv[1]);

	if ((fd = open(argv[1], O_RDONLY)) < 0)
		die_perror("open(%s)", argv[1]);

	if ((log = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) == NULL)
		die_perror("mmap(NULL, %zu, PROT_READ, MAP_PRIVATE, %s)",
				st.st_size, argv[1]);

	/* Parse file, until all frames are read */
	for (i = 0; i < st.st_size;)
	{
		char timestamp[128];
		int tslen;
		unsigned char flags;
		unsigned long long seconds;
		unsigned long long fract;
		unsigned long long data_len;
		struct tm *tm;
		int level;

		/* flag is always 1st byte of frame */
		flags = log[i++];

		timestamp[0] = '\0';

		/* Get log level of message in frame */
		level = flags >> EL_FLAG_LEVEL_SHIFT & EL_FLAG_LEVEL_MASK;

		if (flags & EL_FLAG_TS)
		{
			/* Timestamp is present, read seconds from frame, right
			 * now length of seconds is unknown, decode_number() will
			 * return us number of bytes on which seconds were encoded */
			i += decode_number(log + i, &seconds);

			/* Create timstamp string from the read seconds */
			tm = gmtime((time_t *)&seconds);
			tslen = strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", tm);

			if (flags >> EL_FLAG_TS_FRACT_SHIFT & EL_FLAG_TS_FRACT_MASK)
			{
				/* Fractions of seconds are present, read them too */
				i += decode_number(log + i, &fract);

				/* Append fraction of second into timestamp string,
				 * we will be appending a little differently depending
				 * in fraction unit. */

				switch (flags >> EL_FLAG_TS_FRACT_SHIFT & EL_FLAG_TS_FRACT_MASK)
				{
				case 1: /* nanosecond precision */
					tslen += sprintf(timestamp + tslen, ".%09lld", fract);
					break;

				case 2: /* microsecond precision */
					tslen += sprintf(timestamp + tslen, ".%06lld", fract);
					break;

				case 3: /* millisecond precision */
					tslen += sprintf(timestamp + tslen, ".%03lld", fract);
					break;
				}
			}
		}

		/* Get length of data in the frame */
		i += decode_number(log + i, &data_len);

		/* If timestamp is set, print it with received log level */
		if (timestamp[0] != '\0')
			/* first 3 arguments are 0, since we don't want to print
			 * log level location, it's useless in this case, we we would
			 * only print this very location, not when log originally
			 * was printed */
			el_print(0, 0, 0, level, "%s", timestamp);

		/* Dump received data as hexdump */
		el_pmemory(0, 0, 0, level, log + i, data_len);

		/* Move to next frame */
		i += data_len;
	}

	close(fd);
	el_cleanup();
	return 0;
}
