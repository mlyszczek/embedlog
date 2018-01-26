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


#include "config.h"
#include "el-options.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


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

static char  current_log[PATH_MAX + 1];  /* full path to current log file */


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
    void
)
{
#if HAVE_ACCESS
    return access(current_log, F_OK) == 0;
#else
    FILE *f;

    if ((f = fopen(current_log, "r")) == NULL)
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
    int                 i;       /* simple iterator for loop */
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

    sprintf(current_log, "%s.%d", options->fname, options->fcurrent_rotate);

    if ((options->file = fopen(current_log, "w")) == NULL)
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
    if (options->file)
    {
        /*
         * to prevent any memory leak in  case  of  double  open,  we  first
         * close already opened file Such situation may happen when  library
         * user changes file name using EL_FNAME option,
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
            pathl = snprintf(current_log, PATH_MAX + 1, "%s.%d",
                options->fname, i);

            if (pathl > PATH_MAX)
            {
                /*
                 * path is too long,  we  cannot  safely  create  file  with
                 * suffix, return error as opening such truncated file  name
                 * could result in some data lose on the disk.
                 */

                current_log[0] = '\0';
                errno = ENAMETOOLONG;
                return -1;
            }

            if ((f = fopen(current_log, "a")) == NULL)
            {
                /*
                 * if we cannot open file, that means there is some kind  of
                 * error, like permision denied or system is out of  memory,
                 * it's pointless to continue
                 */

                current_log[0] = '\0';
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
                remove(current_log);
                continue;
            }

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
        current_log[0] = '\0';
        errno = ENAMETOOLONG;
        return -1;
    }

    strcpy(current_log, options->fname);

    if ((options->file = fopen(current_log, "a")) == NULL)
    {
        current_log[0] = '\0';
        return -1;
    }

    options->fpos = ftell(options->file);
    return 0;
}


/* ==========================================================================
    puts string s into file from options, if needed also rotates files
   ========================================================================== */


int el_file_puts
(
    struct el_options  *options,  /* printing options */
    const char         *s         /* string to 'put' into file */
)
{
    int  sl;  /* size of the s message */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    if (current_log[0] == '\0')
    {
        /*
         * file has not been opened, prevent segfault
         */

        errno = EBADF;
        return -1;
    }

    sl = strlen(s);

    if (options->frotate_number)
    {
        if (options->fpos != 0 &&  options->fpos + sl > options->frotate_size)
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

        if (sl > options->frotate_size)
        {
            /*
             * we can't fit message even in an empty file, in such  case  we
             * need to truncate log, so we don't  create  file  bigger  than
             * configured frotate_size
             */

            sl = options->frotate_size;
        }
    }

    if (el_file_exists() == 0)
    {
        /*
         * file doesn't exist, it may happen when someone unlinks  currently
         * opened file. Even tough user unlinked our log file, we still have
         * file opened, so this file exists in the system, but  not  in  the
         * file system tree and  as  such,  every  write  to  such  file  is
         * effectively writing to /dev/null.  To prevent that, we close  and
         * reopen our current log file
         */

        if (options->file)
        {
            fclose(options->file);
        }

        if ((options->file = fopen(current_log, "a")) == NULL)
        {
            errno = EBADF;
            return -1;
        }

        options->fpos = 0;
    }

    if (fwrite(s, sl, 1, options->file) != 1)
    {
        return -1;
    }

    if (fflush(options->file) != 0)
    {
        return -1;
    }

    options->fpos += sl;
    return 0;
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
    current_log[0] = '\0';
}
