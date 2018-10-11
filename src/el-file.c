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
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */

#if HAVE_CONFIG_H
#   include "config.h"
#endif

#if HAVE_FSYNC && HAVE_FILENO
#   ifndef _POSIX_C_SOURCE
#       define _POSIX_C_SOURCE 1
#   endif
#endif

#include "el-private.h"

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


#ifdef RUN_TESTS

/*
 * there is no easy way to check if file has been fsynced to disk or not,
 * so we introduce this helper variable to know if we executed syncing
 * code or not
 */
extern int file_synced;

#endif


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


#ifndef PATH_MAX
/*
 * for systems that don't define PATH_MAX we use our MAX_PATH macro that  is
 * defined in config.h during ./configure process
 */

#define PATH_MAX MAX_PATH

#endif


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


static int el_file_exists
(
    const char  *path  /* file to check */
)
{
#if HAVE_ACCESS
    /*
     * access is the fastest
     */

    return access(path, F_OK) == 0;

#elif HAVE_STAT
    /*
     * but if we don't have access (some embedded systems like  nuttx  don't
     * have users and will always return OK when access is called -  wrongly
     */

    struct stat st;
    /*
     * if stat return 0 we are sure file exists, -1 is returned for when
     * file doesn't exist or there is other error, in any case we assume
     * file doesn't exist
     */

    return stat(path, &st) == 0;

#else
    /*
     * slowest, worst but  highly  portable  solution,  for  when  there  is
     * nothing left but you still want to work
     */

    FILE *f;

    if ((f = fopen(path, "r")) == NULL)
    {
        return 0;
    }

    fclose(f);
    return 1;
#endif
}


/* ==========================================================================
    function rotates file, meaning if currently opened file has  suffix  .3,
    function will close that file and will open file  with  suffix  .4.   If
    creating  new  suffix  is  impossible  (we  reached  frotate_number   of
    suffixed) function will remove oldest log file  and  new  file  will  be
    created
   ========================================================================== */


static int el_file_rotate
(
    struct el_options  *options
)
{
    unsigned int        i;       /* simple iterator for loop */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    if (options->file)
    {
        fclose(options->file);
        options->file = NULL;
    }

    options->fcurrent_rotate++;
    if (options->fcurrent_rotate == options->frotate_number)
    {
        /*
         * it appears we used all rotating slots, reset counter and rename
         * all files in order (for frotate_number == 4)
         *
         * .1 -> .0
         * .2 -> .1
         * .3 -> .2
         *
         * As we can see, oldest file .0 will be deleted, and file .3 will
         * disapear making space for new log
         */

        options->fcurrent_rotate = options->frotate_number - 1;

        if (options->frotate_number == 1)
        {
            /*
             * if frotate_number is equal to 1, this means only one file can
             * exist (with suffix .0), so we cannot rotate that (like rename
             * it from .0 to .0, pointless) file and  thus  we  simply  skip
             * this foor loop, and later file will just be  truncated  to  0
             */

            goto skip_rotate;
        }

        for (i = 1; i != options->frotate_number; ++i)
        {
            char  old_name[PATH_MAX + 1];  /* ie .2 suffix */
            char  new_name[PATH_MAX + 1];  /* ie .3 suffix */
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

            /*
             * we do not check for overflow here,  as  overflow  is  already
             * tested in el_file_open function, there we check opening  file
             * with biggest suffix. Assuming if it works there, it will work
             * here as well
             */

            sprintf(old_name, "%s.%d", options->fname, i);
            sprintf(new_name, "%s.%d", options->fname, i - 1);

            rename(old_name, new_name);

            /*
             * rename can fail, but it is usually  because  someone  removed
             * some logs, like there are  logs:  .1  .2  .4  .5,  so  .3  is
             * missing, and if we want to rename .3 to .2, rename will fail,
             * it doesn't really matter, as .4 will be renamed  to  .3,  and
             * after full loop there will be .0 .1 .3 .4 .5  and  after  one
             * more rotation, we'll have .0 .1 .2 .3 .4 .5, so full complet.
             * So  such  situation  will  heal   by   itself   over   time.
             */
        }
    }

skip_rotate:
    /*
     * now we can safely open file with  suffix  .n,  we're  sure  we  won't
     * overwrite anything (well except if someone sets frotate_number to  1,
     * then current .0 file will be truncated).  We don't need to check  for
     * length of created file, as it is checked in el_file_open  and  if  it
     * passes there, it will pass here as well
     */

    sprintf(options->current_log, "%s.%d",
        options->fname, options->fcurrent_rotate);

    if ((options->file = fopen(options->current_log, "w")) == NULL)
    {
        options->fcurrent_rotate--;
        return -1;
    }

    options->fpos = 0;

    return 0;
}


/* ==========================================================================
                                        __     __ _
                         ____   __  __ / /_   / /(_)_____
                        / __ \ / / / // __ \ / // // ___/
                       / /_/ // /_/ // /_/ // // // /__
                      / .___/ \__,_//_.___//_//_/ \___/
                     /_/
               ____                     __   _
              / __/__  __ ____   _____ / /_ (_)____   ____   _____
             / /_ / / / // __ \ / ___// __// // __ \ / __ \ / ___/
            / __// /_/ // / / // /__ / /_ / // /_/ // / / /(__  )
           /_/   \__,_//_/ /_/ \___/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


/* ==========================================================================
    opens log file specified in options and sets file position, so we can
    track it.
   ========================================================================== */


int el_file_open
(
    struct el_options  *options  /* options with file information */
)
{
    if (options->current_log == NULL)
    {
        /*
         * yes, we need to dynamically allocate memory here. It's because we
         * need to keep current log privately in each of el_options  objects
         * or there will be problems  in  embedded  systems  that  use  flat
         * memory.  Since when working with files, OS will always make  some
         * dynamic allocation, we will be doing one too.
         */

        if ((options->current_log = malloc(PATH_MAX + 1)) == NULL)
        {
            errno = ENOMEM;
            return -1;
        }
    }

    if (options->file)
    {
        /*
         * to prevent any memory leak in  case  of  double  open,  we  first
         * close already opened file Such situation may happen when  library
         * user changes file name using EL_FPATH option,
         */

        fclose(options->file);
        options->file = NULL;
    }

    if (options->frotate_number)
    {
        FILE   *f;      /* opened file */
        int     i;      /* simple interator for loop */
        size_t  pathl;  /* length of path after snprintf */
        long    fsize;  /* size of the opened file */
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        /*
         * file rotation is enabled, in such case we need  to  find,  oldest
         * rotate file, as app could have  been  restarted,  and  we  surely
         * don't want to overwrite the newest file.  Oldest file has  suffix
         * .0 while the newest one has suffix  .${frotate_number}  (or  less
         * if there are less files).
         */

        for (i = options->frotate_number - 1; i >= 0; --i)
        {
            pathl = snprintf(options->current_log, PATH_MAX + 1, "%s.%d",
                options->fname, i);

            if (pathl > PATH_MAX)
            {
                /*
                 * path is too long,  we  cannot  safely  create  file  with
                 * suffix, return error as opening such truncated file  name
                 * could result in some data lose on the disk.
                 */

                options->current_log[0] = '\0';
                errno = ENAMETOOLONG;
                return -1;
            }

#if HAVE_STAT

            if (i != 0)
            {
                struct stat  st;
                /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

                /*
                 * if i is 0, then this is last file to check (there are  no
                 * logs from embed log in directory) and there is no need to
                 * check if file exists or not - we open it unconditionally,
                 * thus this path is not taken in such case.
                 */

                if (el_file_exists(options->current_log) == 0)
                {
                    /*
                     * current log file does not exist, this is not the file
                     * you are looking for
                     */

                    continue;
                }

                if (stat(options->current_log, &st) != 0)
                {
                    /*
                     * error while stating file, probably don't have  access
                     * to that file, or directory,  or  whatever,  something
                     * bad has happend and we don't  want  to  pursue  this,
                     * exit with error from stat.
                     */

                    options->current_log[0] = '\0';
                    return -1;
                }

                if (st.st_size == 0)
                {
                    /*
                     * there is an empty file, maybe we created it and  then
                     * crashed?  Or maybe someone is trying to pull our legs
                     * and drops bombs under our feets.  Either way, this is
                     * definitely not oldest file.  We also remove it so  it
                     * doesn't botter us later
                     */

                    remove(options->current_log);
                    continue;
                }
            }

            /*
             * we got our file, let's open it for writing and let's call  it
             * a day
             */

            if ((f = fopen(options->current_log, "a")) == NULL)
            {
                /*
                 * couldn't open file, probably directory doesn't exist,  or
                 * we have no permissions to create file here
                 */

                return -1;
            }

            /*
             * we need to check for currently opened file size  once  again,
             * as if i equal 0 here, we never called stat() on our file  and
             * we don't know the size of it
             */

            fseek(f, 0, SEEK_END);
            fsize = ftell(f);

#else /* HAVE_STAT */

            if ((f = fopen(options->current_log, "a")) == NULL)
            {
                /*
                 * if we cannot open file, that means there is some kind  of
                 * error, like permision denied or system is out of  memory,
                 * it's pointless to continue
                 */

                options->current_log[0] = '\0';
                return -1;
            }

            /*
             * position returned by ftell is implementation specific it  can
             * be either end or begin of file (check stdio(3))
             */

            fseek(f, 0, SEEK_END);

            if (i == 0)
            {
                /*
                 * this is the last file we check, we don't check  for  file
                 * size here, since even if this file is empty, we are  sure
                 * this is the oldest file.  File is already opened  so,  we
                 * simply return from the function
                 */

                options->fcurrent_rotate = i;
                options->fpos = ftell(f);
                options->file = f;
                return 0;
            }

            if ((fsize = ftell(f)) == 0)
            {
                /*
                 * we've created (or opened ) an empty file, this means  our
                 * file is not the oldest one, i.e.  we opened file  .6  and
                 * oldest could be .3.  We remove this file and continue our
                 * search for the glory file.
                 */

                fclose(f);
                remove(options->current_log);
                continue;
            }

#endif /* HAVE_STAT */

            /*
             * oldest file found, file is already opened so we simply return
             * from the function
             */

            options->fcurrent_rotate = i;
            options->fpos = fsize;
            options->file = f;
            return 0;
        }
    }

    /*
     * rotation is disabled, simply open file with append flag
     */

    if (strlen(options->fname) > PATH_MAX)
    {
        options->current_log[0] = '\0';
        errno = ENAMETOOLONG;
        return -1;
    }

    strcpy(options->current_log, options->fname);

    if ((options->file = fopen(options->current_log, "a")) == NULL)
    {
        /*
         * we couldn't open file, but don't set clear  options->current_log,
         * we will try to reopen this file each time we print to file.  This
         * is usefull when user tries to open log file in  not-yet  existing
         * directory.  Error is of course returned to user is aware of  this
         * whole situation
         */

        return -1;
    }

    fseek(options->file, 0, SEEK_END);
    options->fpos = ftell(options->file);
    return 0;
}


/* ==========================================================================
    writes memory block pointed by mem of size mlen into a file,  if  needed
    also rotates file
   ========================================================================== */


int el_file_putb
(
    struct el_options    *options,  /* printing options */
    const void           *mem,      /* memory to 'put' into file */
    size_t                mlen      /* size of buffer 'mlen' */

)
{
    VALID(EINVAL, mem);
    VALID(EINVAL, mlen);
    VALID(EINVAL, options);
    VALID(EBADF, options->current_log);
    VALID(EBADF, options->current_log[0] != '\0');


    /*
     * we need to reopen our file if it wasn't opened yet,  or  was  removed
     * due to error or deliberate acion of user
     */

    if (el_file_exists(options->current_log) == 0 || options->file == NULL)
    {
        if (el_file_open(options) != 0)
        {
            return -1;
        }
    }

    if (options->frotate_number)
    {
        if (options->fpos != 0 && options->fpos + mlen > options->frotate_size)
        {
            /*
             * we get here only when frotate  is  enabled,  and  writing  to
             * current file would result in exceding frotate_size  of  file.
             * In such case we rotate the file by closing the  old  one  and
             * opening new file, and if  we  already  filled  frotate_number
             * files,  we  remove  the oldest one.
             */

            if (el_file_rotate(options) != 0)
            {
                return -1;
            }
        }

        if (mlen > options->frotate_size)
        {
            /*
             * we can't fit message even in an empty file, in such  case  we
             * need to truncate log, so we don't  create  file  bigger  than
             * configured frotate_size
             */

            mlen = options->frotate_size;
        }
    }

    if (fwrite(mem, mlen, 1, options->file) != 1)
    {
        return -1;
    }

    options->fpos += mlen;
    options->written_after_sync += mlen;

    if (options->written_after_sync >= options->file_sync_every ||
        options->level_current_msg <= options->file_sync_level)
    {
        /*
         * file store operation has particular issue.  You can  like,  write
         * hundreds of megabytes into disk and that data  indeed  will  land
         * into your block device *but* metadata will not!  That is if power
         * lose occurs (even after those hundreds of  megs),  file  metadata
         * might not be updated and we will  lose  whole,  I  say  it  again
         * *whole* file.  To prevent data lose on power down we try to  sync
         * file and  its  metadata  to  block  device.   We  do  this  every
         * configured "sync_every" field since syncing  every  single  write
         * would simply kill performance - its up to user to decide how much
         * data he is willing to lose.  It is also possible that altough  we
         * sync our file  and  metadata,  parent  directory  might  not  get
         * flushed and there will not be entry for our file -  meaning  file
         * will be lost too, but such situations are ultra  rare  and  there
         * isn't really much we can do about it here but praying.
         */

#if HAVE_FSYNC && HAVE_FILENO
        int fd;  /* systems file descriptor for options->file */
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#endif /* HAVE_FSYNC && HAVE_FILENO */

        /*
         * first flush data from stdio library buffers into kernel
         */

        if (fflush(options->file) != 0)
        {
            return -1;
        }

#if HAVE_FSYNC && HAVE_FILENO
        /*
         * and then sync data into block device
         */

        if ((fd = fileno(options->file)) < 0)
        {
            return -1;
        }

        if (fsync(fd) != 0)
        {
            return -1;
        }
#else /* HAVE_FSYNC && HAVE_FILENO */
        /*
         * if system does not implement fileno and fsync our only hope  lies
         * in closing (which should sync file  to  block  device)  and  then
         * opening file again.  Yup, performance will  suck,  but  hey,  its
         * about data safety!
         */

        fclose(options->file);

        if ((options->file = fopen(options->current_log, "a")) == NULL)
        {
            errno = EBADF;
            return -1;
        }

        fseek(options->file, 0, SEEK_END);
        options->fpos = ftell(options->file);
#endif /* HAVE_FSYNC && HAVE_FILENO */

#ifdef RUN_TESTS
        file_synced = 1;
#endif /* RUN_TESTS */

        options->written_after_sync = 0;
    }

    return 0;
}


/* ==========================================================================
    Puts string 's' into file if needed rotates file
   ========================================================================== */


int el_file_puts
(
    struct el_options  *options,  /* printing options */
    const char         *s         /* string to 'put' into file */
)
{
    size_t              slen;     /* size of string 's' */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    slen = strlen(s);
    return el_file_putb(options, (unsigned char *)s, slen);
}


/* ==========================================================================
    free resources allocated by this module
   ========================================================================== */


void el_file_cleanup
(
    struct el_options  *options  /* file options */
)
{
    if (options->file)
    {
        fclose(options->file);
    }

    options->file = NULL;

    if (options->current_log)
    {
        options->current_log[0] = '\0';
        free(options->current_log);
        options->current_log = NULL;
    }
}
