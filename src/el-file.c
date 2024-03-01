/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
         -------------------------------------------------------------
        / module designated to print messages into files, it also has \
        \ capabilities to rotate files if user wishes to              /
         -------------------------------------------------------------
                  \           \  /
                   \           \/
                       (__)    /\
                       (oo)   O  O
                       _\/_   //
                 *    (    ) //
                  \  (\\    //
                   \(  \\    )
                    (   \\   )   /\
          ___[\______/^^^^^^^\__/) o-)__
         |\__[=======______//________)__\
         \|_______________//____________|
             |||      || //||     |||
             |||      || @.||     |||
              ||      \/  .\/      ||
                         . .
                        '.'.`

                    COW-OPERATION
   ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "el-private.h"
#include "el-utils.h"

#if HAVE_UNISTD_H
#   include <unistd.h>
#endif

#if HAVE_STAT
#   include <sys/types.h>
#   include <sys/stat.h>
#endif

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* ==========================================================================
                  __        __            __
          ____ _ / /____   / /_   ____ _ / /  _   __ ____ _ _____ _____
         / __ `// // __ \ / __ \ / __ `// /  | | / // __ `// ___// ___/
        / /_/ // // /_/ // /_/ // /_/ // /   | |/ // /_/ // /   (__  )
        \__, //_/ \____//_.___/ \__,_//_/    |___/ \__,_//_/   /____/
       /____/
   ========================================================================== */


#ifdef RUN_TESTS

/* there is no easy way to check if file has been fsynced to disk
 * or not, so we introduce this helper variable to know if we
 * executed syncing code or not
 */
int file_synced;

#endif


/* ==========================================================================
          __             __                     __   _
     ____/ /___   _____ / /____ _ _____ ____ _ / /_ (_)____   ____   _____
    / __  // _ \ / ___// // __ `// ___// __ `// __// // __ \ / __ \ / ___/
   / /_/ //  __// /__ / // /_/ // /   / /_/ // /_ / // /_/ // / / /(__  )
   \__,_/ \___/ \___//_/ \__,_//_/    \__,_/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


#ifndef PATH_MAX
/*
 * for systems that don't define PATH_MAX we use our MAX_PATH macro that  is
 * defined in config.h during ./configure process
 */

#define PATH_MAX MAX_PATH

#endif


/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ========================================================================== */


static void el_date_rotate_str
(
	struct el *el,
	char      *out
)
{
	const char *dfmt;
	time_t now;
	struct tm *tmp;
	struct tm tm;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	switch (el->frotate_type)
	{
	case EL_ROT_DATE_YEAR: dfmt = "%Y"; break;
	case EL_ROT_DATE_MON:  dfmt = "%Y-%m"; break;
	case EL_ROT_DATE_DAY:  dfmt = "%Y-%m-%d"; break;
	case EL_ROT_DATE_HOUR: dfmt = "%Y-%m-%d--%H"; break;
	case EL_ROT_DATE_MIN:  dfmt = "%Y-%m-%d--%H-%M"; break;
	case EL_ROT_DATE_SEC:  dfmt = "%Y-%m-%d--%H-%M-%S"; break;
	}

	now = time(NULL);
#if ENABLE_REENTRANT
	tmp = gmtime_r(&now, &tm);
#else
	tmp = gmtime(&now);
#endif

	/* Construct date format from current UTC time, we are
	 * working on static variables, so we know it won't fail
	 * or overflow timstr buffer */
	strftime(out, 21, dfmt, tmp);
}


/* ==========================================================================
    Checks if file at "path" exists. Functions tries fastests method first,
    and slower last.

    returns
            0       file does not exist
            1       file exists
   ========================================================================== */


static int el_file_exists
(
	const char  *path  /* file to check */
)
{
#if HAVE_ACCESS
	/* access is the fastest */
	return access(path, F_OK) == 0;

#elif HAVE_STAT
	/* but if we don't have access (some embedded systems like
	 * nuttx don't have users and will always return OK when
	 * access is called - wrongly) */
	struct stat st;

	/* if stat return 0 we are sure file exists, -1 is returned for
	 * when file doesn't exist or there is other error, in any case
	 * we assume file doesn't exist */
	return stat(path, &st) == 0;

#else
	/* slowest, worst but highly portable solution, for when there
	 * is nothing left but you still want to work */

	FILE  *f;
	if ((f = fopen(path, "r")) == NULL)
		return 0;

	fclose(f);
	return 1;
#endif
}


/* ==========================================================================
    create symlink without any suffix to newest log file.  Rationale: it's
    very often that user wants to view newest log file, but with the fact
    that newest file has non deterministic suffix, it may be hard and
    unconvenient to first check newest file and then read it, so we create
    that nice symlink in a way:

        log.0
        log.1
        log.5
        log.7
        log -> log.7

    Function does not return anything as errors are not checked here.
    Symlink is optional and if it is not created no data is lost and
    returning error would confuse caller.
   ========================================================================== */


static void el_symlink_to_newest_log
(
	struct el     *el  /* embedlog object to work on */
)
{
#if HAVE_SYMLINK
	if (el->frotate_symlink)
	{
		(void)remove(el->fname);
		(void)symlink(el_basename(el->fcurrent_log), el->fname);
	}
#endif
}


/* ==========================================================================
    function rotates file, meaning if currently opened file has suffix .3,
    function will close that file and will open file with suffix .4. If
    creating new suffix is impossible (we reached frotate_number of
    suffixed) function will remove oldest log file and new file will be
    created
    ========================================================================== */


static int el_file_rotate
(
	struct el     *el
)
{
	unsigned int   i;  /* simple iterator for loop */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	if (el->file)
	{
		fclose(el->file);
		el->file = NULL;
	}

	el->fcurrent_rotate++;
	if (el->fcurrent_rotate == el->frotate_number)
	{
		/* it appears we used all rotating slots, reset counter and
		 * rename all files in order (for frotate_number == 4)
		 *
		 * .1 -> .0
		 * .2 -> .1
		 * .3 -> .2
		 *
		 * As we can see, oldest file .0 will be deleted, and file
		 * .3 will disapear making space for new log */
		el->fcurrent_rotate = el->frotate_number - 1;

		/* if frotate_number is equal to 1, this means only one
		 * file can exist (with suffix .0), so we cannot rotate
		 * that (like rename it from .0 to .0, pointless) file
		 * and thus we simply skip this foor loop, and later
		 * file will just be truncated to 0 */
		if (el->frotate_number == 1) goto skip_rotate;

		for (i = 1; i != el->frotate_number; ++i)
		{
			char  old_name[PATH_MAX + 6];  /* ie .2 suffix */
			char  new_name[PATH_MAX + 6];  /* ie .3 suffix */
			int   pad_len;
			/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

			/* we do not check for overflow here, as overflow is
			 * already tested in el_file_open function, there we
			 * check opening file with biggest suffix. Assuming if
			 * it works there, it will work here as well */
			pad_len = el->frotate_number_count;
			sprintf(old_name, "%s.%0*d", el->fname, pad_len, i);
			sprintf(new_name, "%s.%0*d", el->fname, pad_len, i - 1);
			rename(old_name, new_name);

			/* rename can fail, but it is usually because someone
			 * removed some logs, like there are logs: .1 .2 .4 .5,
			 * so .3 is missing, and if we want to rename .3 to .2,
			 * rename will fail, it doesn't really matter, as .4
			 * will be renamed to .3, and after full loop there
			 * will be .0 .1 .3 .4 .5 and after one more rotation,
			 * we'll have .0 .1 .2 .3 .4 .5, so full complet. So
			 * such situation will heal by itself over time.  */
		}
	}

skip_rotate:
	/* now we can safely open file with suffix .n, we're sure we
	 * won't overwrite anything (well except if someone sets
	 * frotate_number to 1, then current .0 file will be
	 * truncated). We don't need to check for length of created
	 * file, as it is checked in el_file_open and if it passes
	 * there, it will pass here as well */
	sprintf(el->fcurrent_log, "%s.%0*d", el->fname,
			el->frotate_number_count, el->fcurrent_rotate);

	if ((el->file = fopen(el->fcurrent_log, "w")) == NULL)
	{
		el->fcurrent_rotate--;
		return -1;
	}

	el_symlink_to_newest_log(el);
	el->fpos = 0;

	return 0;
}


/* ==========================================================================
                       __     __ _          ____
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ========================================================================== */


/* ==========================================================================
    opens log file specified in el and sets file position, so we can
    track it.
   ========================================================================== */


int el_file_open
(
	struct el  *el  /* el object with file information */
)
{
	size_t       pathlen;  /* length of log path */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	/* we need current_log filename for ourself - we modify it when
	 * we rotate file. In theory this could be done in struct
	 * declaration, but PATH_MAX is such a waste of memory and
	 * programs usually do not get even close to that amount. So we
	 * just add what is necessary. */

	/* pathlen must be with null termination character, as this
	 * variable is passed to snprintf() */
	pathlen = strlen(el->fname) + 1;

	if (el->frotate_type == EL_ROT_FSIZE)
		/* Allocate enough memory for fname and max of 5 digits for
		 * rotate + 1 for separator dot */
		pathlen += 5 + 1;
	else if (EL_IS_ROT_DATE(el))
		/* if we are doing date base rotation, maximum additional
		 * file length will be strlen(YYYY-mm-dd--HH-MM-SS) = 20 */
		pathlen += 20 + 1;

	if (el->fcurrent_log == NULL || pathlen > strlen(el->fcurrent_log))
	{
		/* If new pathlen is larger than length of current log file,
		 * than we need to allocate more memory. If we never opened
		 * file before, fcurrent_log will be NULL, in which case
		 * we also must allocate memory */
		el->fcurrent_log = realloc(el->fcurrent_log, pathlen);
		VALID(ENOMEM, el->fcurrent_log != NULL)
	}

	if (el->file)
	{
		/* to prevent any memory leak in case of double open, we
		 * first close already opened file Such situation may
		 * happen when library user changes file name using
		 * EL_FPATH option, */
		fclose(el->file);
		el->file = NULL;
	}

	/* We are opening new file, so we don't have current log file. */
	el->fcurrent_log[0] = '\0';

	if (el->frotate_type == EL_ROT_FSIZE && el->frotate_number)
	{
		FILE   *f;      /* opened file */
		int     i;      /* simple interator for loop */
		size_t  pathl;  /* length of path after snprintf */
		long    fsize;  /* size of the opened file */
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		/* file rotation is enabled, in such case we need to find,
		 * oldest rotate file, as app could have been restarted,
		 * and we surely don't want to overwrite the newest file.
		 * Oldest file has suffix .0 while the newest one has
		 * suffix .${frotate_number} (or less if there are less
		 * files).
		 *
		 * Reason for such order is so user could do `cat logs.*`
		 * and in result concatenate all logs in order.  */

		for (i = el->frotate_number - 1; i >= 0; --i)
		{
			pathl = snprintf(el->fcurrent_log, pathlen, "%s.%0*d",
					el->fname, el->frotate_number_count, i);

			if (pathl > PATH_MAX)
			{
				/* path is too long, we cannot safely create file
				 * with suffix, return error as opening such
				 * truncated file name could result in some data
				 * lose on the disk.  */
				el->fcurrent_log[0] = '\0';
				errno = ENAMETOOLONG;
				return -1;
			}

#if HAVE_STAT

			/* if i is 0, then this is last file to check
			 * (there are no logs from embed log in directory)
			 * and there is no need to check if file exists or
			 * not - we open it unconditionally, thus this path
			 * is not taken in such case.  */
			if (i != 0)
			{
				struct stat  st;  /* file information */
				/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


				/* current log file does not exist, this is not
				 * the file you are looking for */
				if (el_file_exists(el->fcurrent_log) == 0) continue;

				if (stat(el->fcurrent_log, &st) != 0)
				{
					/* error while stating file, probably don't
					 * have access to that file, or directory, or
					 * whatever, something bad has happend and we
					 * don't want to pursue this, exit with error
					 * from stat.  */
					el->fcurrent_log[0] = '\0';
					return -1;
				}

				if (st.st_size == 0)
				{
					/* there is an empty file, maybe we created it
					 * and then crashed? Or maybe someone is trying
					 * to pull our legs and drops bombs under our
					 * feets. Either way, this is definitely not
					 * oldest file. We also remove it so it doesn't
					 * bother us later */
					remove(el->fcurrent_log);
					continue;
				}
			}

			/* we got our file, let's open it for writing and let's
			 * call it a day */

			/* couldn't open file, probably directory doesn't
			 * exist, or we have no permissions to create file
			 * here */
			if ((f = fopen(el->fcurrent_log, "a")) == NULL) return -1;

			/* we need to check for currently opened file size once
			 * again, as if i equal 0 here, we never called stat()
			 * on our file and we don't know the size of it */
			fseek(f, 0, SEEK_END);
			fsize = ftell(f);

#else /* HAVE_STAT */

			if ((f = fopen(el->fcurrent_log, "a")) == NULL)
			{
				/* if we cannot open file, that means there is some
				 * kind of error, like permision denied or system
				 * is out of memory, it's pointless to continue */
				el->fcurrent_log[0] = '\0';
				return -1;
			}

			/* position returned by ftell is implementation
			 * specific it can be either end or begin of file
			 * (check stdio(3)) */
			fseek(f, 0, SEEK_END);

			if (i == 0)
			{
				/* this is the last file we check, we don't check
				 * for file size here, since even if this file is
				 * empty, we are sure this is the oldest file.
				 * File is already opened so, we simply return from
				 * the function */
				el->fcurrent_rotate = i;
				el->fpos = ftell(f);
				el->file = f;
				return 0;
			}

			if ((fsize = ftell(f)) == 0)
			{
				/* we've created (or opened ) an empty file, this
				 * means our file is not the oldest one, i.e. we
				 * opened file .6 and oldest could be .3. We remove
				 * this file and continue our search for the glory
				 * file.  */
				fclose(f);
				remove(el->fcurrent_log);
				continue;
			}

#endif /* HAVE_STAT */

			/* oldest file found, file is already opened so we
			 * simply update symlink to newest log file and return. */
			el_symlink_to_newest_log(el);
			el->fcurrent_rotate = i;
			el->fpos = fsize;
			el->file = f;
			return 0;
		}
	}

	if (EL_IS_ROT_DATE(el))
	{
		/* We are rotating by date. This it not as complex case
		 * as with rotate by size, we just open file with current
		 * date (based on rotate timescale */

		char timstr[20 + 1];
		size_t     pathl;      /* length of path after snprintf */
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		el_date_rotate_str(el, timstr);

		/* Construct file name with selected date rotation in format
		 * user-filename.$date */
		pathl = snprintf(el->fcurrent_log, pathlen, "%s.%s", el->fname, timstr);
		if (pathl > PATH_MAX)
		{
			/* path is too long, we cannot safely create file
			 * with suffix, return error as opening such
			 * truncated file name could result in some data
			 * lose on the disk.  */
			el->fcurrent_log[0] = '\0';
			errno = ENAMETOOLONG;
			return -1;
		}
	}

	/* rotation is disabled or we deal with date rotation, simply
	 * open file with append flag */

	if (strlen(el->fname) > pathlen)
	{
		/* path will not fit into our internal buffer */
		el->fcurrent_log[0] = '\0';
		errno = ENAMETOOLONG;
		return -1;
	}

	/* fcurrent log might have been already set (in date rotation),
	 * so set fcurrent_log only when it was not already set */
	if (el->fcurrent_log[0] == '\0')
		strcpy(el->fcurrent_log, el->fname);

	/* in case we couldn't open file, don't set clear
	 * el->fcurrent_log - we will try to reopen this file each time
	 * we print to file. This is usefull when user tries to open
	 * log file in not-yet existing directory. Error is of course
	 * returned to user is aware of this whole situation */
	if ((el->file = fopen(el->fcurrent_log, "a")) == NULL) return -1;

	fseek(el->file, 0, SEEK_END);
	el->fpos = ftell(el->file);
	return 0;
}


/* ==========================================================================
    Does whatever it takes to make sure that logs are flushed from any
    buffers to physical disk. It's not always possible, but boy, do we try.
   ========================================================================== */


int el_file_flush
(
	struct el  *el  /* printing options */
)
{
	/* file store operation has particular issue. You can like,
	 * write hundreds of megabytes into disk and that data indeed
	 * will land into your block device *but* metadata will not!
	 * That is if power lose occurs (even after those hundreds of
	 * megs), file metadata might not be updated and we will lose
	 * whole, I say it again *whole* file. To prevent data lose on
	 * power down we try to sync file and its metadata to block
	 * device. We do this every configured "sync_every" field since
	 * syncing every single write would simply kill performance -
	 * its up to user to decide how much data he is willing to
	 * lose. It is also possible that altough we sync our file and
	 * metadata, parent directory might not get flushed and there
	 * will not be entry for our file - meaning file will be lost
	 * too, but such situations are ultra rare and there isn't
	 * really much we can do about it here but praying.
	 */

#if HAVE_FSYNC && HAVE_FILENO

	int fd;  /* systems file descriptor for el->file */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#endif /* HAVE_FSYNC && HAVE_FILENO */

	VALID(EINVAL, el);
	VALID(EBADF, el->fcurrent_log);
	VALID(EBADF, el->fcurrent_log[0] != '\0');

	/* if no writes have been performed between consecutive
	 * flush, then don't flush - since there is nothing to
	 * flush anyway */
	if (el->fwritten_after_sync == 0) return 0;

	/* first flush data from stdio library buffers into kernel */
	if (fflush(el->file) != 0) return -1;

#if HAVE_FSYNC && HAVE_FILENO

	/* and then sync data into block device */
	if ((fd = fileno(el->file)) < 0) return -1;
	if (fsync(fd) != 0) return -1;

#else /* HAVE_FSYNC && HAVE_FILENO */

	/* if system does not implement fileno and fsync our only hope
	 * lies in closing (which should sync file to block device) and
	 * then opening file again. Yup, performance will suck, but
	 * hey, its about data safety! */
	fclose(el->file);

	el->file = fopen(el->fcurrent_log, "a");
	VALID(EBADF, el->file);
	fseek(el->file, 0, SEEK_END);
	el->fpos = ftell(el->file);

#endif /* HAVE_FSYNC && HAVE_FILENO */

#ifdef RUN_TESTS

	file_synced = 1;

#endif /* RUN_TESTS */

	/* after syncing data, update written after sync field, so we
	 * don't trigger another flush right after this one */
	el->fwritten_after_sync = 0;
	return 0;
}


/* ==========================================================================
    writes memory block pointed by mem of size mlen into a file, if needed
    also rotates file
   ========================================================================== */


int el_file_putb
(
	struct el    *el,   /* printing el */
	const void   *mem,  /* memory to 'put' into file */
	size_t        mlen  /* size of buffer 'mlen' */
)
{
	VALID(EINVAL, mem);
	VALID(EINVAL, mlen);
	VALID(EINVAL, el);
	VALID(EBADF, el->fcurrent_log);
	VALID(EBADF, el->fcurrent_log[0] != '\0');


	/* we need to reopen our file if it wasn't opened yet, or was
	 * removed due to error or deliberate acion of user */
	if (el_file_exists(el->fcurrent_log) == 0 || el->file == NULL)
		if (el_file_open(el) != 0)
			return -1;

	if (el->frotate_type == EL_ROT_FSIZE && el->frotate_number)
	{
		/* we get here only when frotate is enabled. check if
		 * writing to current file would result in exceding
		 * frotate_size of file. if so we rotate the file by
		 * closing the old one and opening new file, and if we
		 * already filled frotate_number files, we remove the
		 * oldest one. */
		if (el->fpos != 0 && el->fpos + mlen > el->frotate_size)
			if (el_file_rotate(el) != 0)
				return -1;

		/* if we can't fit message even in an empty file, we need
		 * to truncate log, so we don't create file bigger than
		 * configured frotate_size */
		if (mlen > el->frotate_size)
			mlen = el->frotate_size;
	}

	if (EL_IS_ROT_DATE(el))
	{
		/* If we are date rotating, check it time has come
		 * to rotate the file. And by rotate I simply mean
		 * open new file with new date. */

		char        timstr[20 + 1];
		const char *file_timstr;
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		/* get current rotate time that should be */
		el_date_rotate_str(el, timstr);
		/* point file_timestr to beginning of date rotate suffix.
		 * for file log.2024-01-01, it will point to 2024-01-01 */
		file_timstr = el->fname + strlen(el->fname) + 1;

		/* If timstr (current, now, value) is different than
		 * file_timstr (currently opened file), it means that
		 * second, minut, hour, day... has changed, and we need to
		 * open new file */
		if (strcmp(timstr, file_timstr))
			if (el_file_open(el))
				return -1;
	}

	if (fwrite(mem, mlen, 1, el->file) != 1) return -1;
	el->fpos += mlen;
	el->fwritten_after_sync += mlen;

	/* flush logs to block device if we've written enough bytes
	 * user configured in sync_every or log level is sever enough
	 * to trigger flush. */
	if (el->fwritten_after_sync >= el->fsync_every ||
			el->level_current_msg <= el->fsync_level)
		return el_file_flush(el);

	return 0;
}


/* ==========================================================================
    Puts string 's' into file if needed rotates file
   ========================================================================== */


int el_file_puts
(
	struct el   *el,    /* printing options */
	const char  *s      /* string to 'put' into file */
)
{
	size_t       slen;  /* size of string 's' */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	slen = strlen(s);
	return el_file_putb(el, (unsigned char *)s, slen);
}


/* ==========================================================================
    free resources allocated by this module
   ========================================================================== */


void el_file_cleanup
(
	struct el  *el  /* file options */
)
{
	if (el->file) fclose(el->file);
	el->file = NULL;

	if (el->fcurrent_log)
	{
		el->fcurrent_log[0] = '\0';
		free(el->fcurrent_log);
		el->fcurrent_log = NULL;
	}
}
