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


#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

#include "mtest.h"
#include "embedlog.h"


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


#define WORKDIR "/tmp/embedlog-tests"
#define s9 "123456789"
#define s8 "qwertyui"
#define s5 "qwert"
#define s3 "asd"
mt_defs_ext();


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


static int file_check
(
    const char *f,
    const char *s
)
{
    char     fc[128];
    int      fd;
    ssize_t  r;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    fd = open(f, O_RDONLY);
    r = read(fd, fc, sizeof(fc));
    close(fd);

    fc[r] = '\0';
    if(strcmp(s, fc) != 0)
    {
        return -1;
    }

    return 0;
}


/* ==========================================================================

   ========================================================================== */


static void test_prepare(void)
{
    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 0);
    el_option(EL_FNAME, WORKDIR"/log");

}


/* ==========================================================================

   ========================================================================== */


static void test_cleanup(void)
{
    int i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    /*
     * remove any file that might have been created during tests.  We ignore
     * unlink return code, as we expected that some files won't exist, as in
     * are   not   needed   by   tests   and   thus   are   not    created
     */

    unlink(WORKDIR"/log");
    unlink(WORKDIR"/log-another");

    for (i = 0; i != 5; ++i)
    {
        char path1[PATH_MAX] = WORKDIR"/log.";
        char path2[PATH_MAX] = WORKDIR"/log-another.";
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        path1[strlen(path1)] = '0' + i;
        path2[strlen(path2)] = '0' + i;
        unlink(path1);
        unlink(path2);
    }

    el_cleanup();
}


/* ==========================================================================
                           __               __
                          / /_ ___   _____ / /_ _____
                         / __// _ \ / ___// __// ___/
                        / /_ /  __/(__  )/ /_ (__  )
                        \__/ \___//____/ \__//____/

   ========================================================================== */


static void file_single_message(void)
{
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log", s9));
}


/* ==========================================================================
   ========================================================================== */


static void file_multi_message(void)
{
    el_puts(s9);
    el_puts(s8);
    el_puts(s5);
    mt_fok(file_check(WORKDIR"/log", s9 s8 s5));
}


/* ==========================================================================
   ========================================================================== */


static void file_reopen(void)
{
    el_puts(s9);
    el_cleanup();

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 0);
    el_option(EL_FNAME, WORKDIR"/log");

    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log", s9 s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_reopen_different_file(void)
{
    el_puts(s9);
    el_cleanup();

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 0);
    el_option(EL_FNAME, WORKDIR"/log-another");

    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log", s9));
    mt_fok(file_check(WORKDIR"/log-another", s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_unexpected_third_party_delete(void)
{
    el_puts(s9);
    unlink(WORKDIR"/log");
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log", s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_filename_too_long(void)
{
    char  path[NAME_MAX + 1 + 1];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(path, 'a', sizeof(path));
    path[sizeof(path) - 1] = '\0';
    mt_ferr(el_option(EL_FNAME, path), ENAMETOOLONG);
}


/* ==========================================================================
   ========================================================================== */


static void file_path_too_long(void)
{
    char  path[PATH_MAX + 2];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(path, 'a', sizeof(path));
    path[0] = '/';
    path[sizeof(path) - 5] = '/';
    path[sizeof(path) - 4] = 'f';
    path[sizeof(path) - 4] = 'i';
    path[sizeof(path) - 3] = 'l';
    path[sizeof(path) - 2] = 'e';
    path[sizeof(path) - 1] = '\0';
    mt_ferr(el_option(EL_FNAME, path), ENAMETOOLONG);
}


/* ==========================================================================
   ========================================================================== */


static void file_print_without_init(void)
{
    mt_ferr(el_puts("whatev..."), ENODEV);
}


/* ==========================================================================
   ========================================================================== */


static void file_print_after_cleanup(void)
{
    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 0);
    el_option(EL_FNAME, WORKDIR"/log");
    el_cleanup();
    mt_ferr(el_puts("whatev"), ENODEV);
}


/* ==========================================================================
   ========================================================================== */


static void file_print_without_setting_file(void)
{
    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 0);
    mt_ferr(el_puts("no file set"), EBADF);
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log.0", s9));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_exact_print(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s8);
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s8 s8));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_overflow_but_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s9 s5 s5);
    mt_fok(file_check(WORKDIR"/log.0", s9 s5 "qw"));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_overflow(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s9);
    el_puts(s9);
    el_puts(s3);
    mt_fok(file_check(WORKDIR"/log.0", s9 s3));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_overflow_exact(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s8);
    el_puts(s8);
    el_puts(s5);
    mt_fok(file_check(WORKDIR"/log.0", s5));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_reopen(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s5);

    el_cleanup();
    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 1);
    el_option(EL_FNAME, WORKDIR"/log");

    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s5 s8));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_unexpected_third_party_remove(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s5);
    el_puts(s8);
    unlink(WORKDIR"/log.0");
    el_puts(s3);
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s3 s8));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_change_size_up(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s9);
    el_puts(s5);
    el_option(EL_FROTATE_SIZE, 32);
    el_puts(s9);
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s9 s5 s9 s8));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_change_size_down(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s9);
    el_option(EL_FROTATE_SIZE, 8);
    el_puts(s5);
    mt_fok(file_check(WORKDIR"/log.0", s5));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_exact_print(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s8);
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s8 s8));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_overflow_but_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9 s5 s5);
    mt_fok(file_check(WORKDIR"/log.0", s9 s5 "qw"));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_overflow(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9);
    el_puts(s9);
    el_puts(s3);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fok(file_check(WORKDIR"/log.1", s9 s3));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_overflow_exact(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s8);
    el_puts(s8);
    el_puts(s5);
    mt_fok(file_check(WORKDIR"/log.0", s8 s8));
    mt_fok(file_check(WORKDIR"/log.1", s5));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_reopen(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9);
    el_puts(s8);

    el_cleanup();
    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 2);
    el_option(EL_FNAME, WORKDIR"/log");

    el_puts(s5);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fok(file_check(WORKDIR"/log.1", s8 s5));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_unexpected_third_party_remove(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s5);
    el_puts(s8);
    unlink(WORKDIR"/log.0");
    el_puts(s3);
    el_puts(s8);
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log.0", s3 s8));
    mt_fok(file_check(WORKDIR"/log.1", s9));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_change_size_up(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9);
    el_puts(s5);
    el_puts(s9);
    el_option(EL_FROTATE_SIZE, 32);
    el_puts(s9);
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s9 s5));
    mt_fok(file_check(WORKDIR"/log.1", s9 s9 s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_change_size_down(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9);
    el_puts(s9);
    el_option(EL_FROTATE_SIZE, 8);
    el_puts(s5);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fok(file_check(WORKDIR"/log.1", s5));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_many_rotates(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9);
    el_puts(s8);
    el_puts(s5);
    el_puts(s5);
    el_puts(s3);
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log.0", s5 s3));
    mt_fok(file_check(WORKDIR"/log.1", s9));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_exact_print_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    el_puts(s8);
    el_puts(s8);
    el_puts(s8);
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s8 s8));
    mt_fok(file_check(WORKDIR"/log.1", s8 s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_overflow_but_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    el_puts(s8);
    el_puts(s8);
    el_puts(s9 s5 s5);
    mt_fok(file_check(WORKDIR"/log.0", s8 s8));
    mt_fok(file_check(WORKDIR"/log.1", s9 s5 "qw"));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_overflow(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    el_puts(s9);
    el_puts(s9);
    el_puts(s3);
    el_puts(s3);
    el_puts(s8);
    el_puts(s5);
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fok(file_check(WORKDIR"/log.1", s9 s3 s3));
    mt_fok(file_check(WORKDIR"/log.2", s8 s5));
    mt_fok(file_check(WORKDIR"/log.3", s9));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_overflow_exact(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    el_puts(s8);
    el_puts(s8);
    el_puts(s8);
    el_puts(s8);
    el_puts(s5);
    mt_fok(file_check(WORKDIR"/log.0", s8 s8));
    mt_fok(file_check(WORKDIR"/log.1", s8 s8));
    mt_fok(file_check(WORKDIR"/log.2", s5));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_reopen(void)
{
    el_option(EL_FROTATE_NUMBER, 5);

    el_puts(s9);
    el_puts(s9);
    el_puts(s3);
    el_puts(s3);
    el_puts(s8);
    el_puts(s5);

    el_cleanup();
    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 5);
    el_option(EL_FNAME, WORKDIR"/log");

    el_puts(s9);
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fok(file_check(WORKDIR"/log.1", s9 s3 s3));
    mt_fok(file_check(WORKDIR"/log.2", s8 s5));
    mt_fok(file_check(WORKDIR"/log.3", s9));
    mt_fok(file_check(WORKDIR"/log.4", s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_hole_in_log_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    el_option(EL_FROTATE_SIZE, 3);
    el_puts("qaz");
    el_puts("wsx");
    el_puts("edc");
    el_puts("rfv");
    el_puts("tgb");

    mt_fok(file_check(WORKDIR"/log.0", "qaz"));
    mt_fok(file_check(WORKDIR"/log.1", "wsx"));
    mt_fok(file_check(WORKDIR"/log.2", "edc"));
    mt_fok(file_check(WORKDIR"/log.3", "rfv"));
    mt_fok(file_check(WORKDIR"/log.4", "tgb"));

    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.4");

    el_puts("123");

    mt_fok(file_check(WORKDIR"/log.0", "qaz"));
    mt_fok(file_check(WORKDIR"/log.1", "edc"));
    mt_fok(file_check(WORKDIR"/log.2", "rfv"));
    mt_fok(file_check(WORKDIR"/log.4", "123"));

    el_puts("456");

    mt_fok(file_check(WORKDIR"/log.0", "edc"));
    mt_fok(file_check(WORKDIR"/log.1", "rfv"));
    mt_fok(file_check(WORKDIR"/log.3", "123"));
    mt_fok(file_check(WORKDIR"/log.4", "456"));

    el_puts("789");

    mt_fok(file_check(WORKDIR"/log.0", "rfv"));
    mt_fok(file_check(WORKDIR"/log.2", "123"));
    mt_fok(file_check(WORKDIR"/log.3", "456"));
    mt_fok(file_check(WORKDIR"/log.4", "789"));

    el_puts("qwe");

    mt_fok(file_check(WORKDIR"/log.0", "rfv"));
    mt_fok(file_check(WORKDIR"/log.1", "123"));
    mt_fok(file_check(WORKDIR"/log.2", "456"));
    mt_fok(file_check(WORKDIR"/log.3", "789"));
    mt_fok(file_check(WORKDIR"/log.4", "qwe"));

    el_puts("asd");

    mt_fok(file_check(WORKDIR"/log.0", "123"));
    mt_fok(file_check(WORKDIR"/log.1", "456"));
    mt_fok(file_check(WORKDIR"/log.2", "789"));
    mt_fok(file_check(WORKDIR"/log.3", "qwe"));
    mt_fok(file_check(WORKDIR"/log.4", "asd"));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_rename_file_halfway(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    el_option(EL_FROTATE_SIZE, 3);
    el_puts("qaz");
    el_puts("wsx");
    el_puts("edc");
    el_puts("rfv");
    el_puts("tgb");

    el_option(EL_FNAME, WORKDIR"/log-another");

    el_puts("123");
    el_puts("456");
    el_puts("789");
    el_puts("qwe");
    el_puts("asd");

    mt_fok(file_check(WORKDIR"/log.0", "qaz"));
    mt_fok(file_check(WORKDIR"/log.1", "wsx"));
    mt_fok(file_check(WORKDIR"/log.2", "edc"));
    mt_fok(file_check(WORKDIR"/log.3", "rfv"));
    mt_fok(file_check(WORKDIR"/log.4", "tgb"));

    mt_fok(file_check(WORKDIR"/log-another.0", "123"));
    mt_fok(file_check(WORKDIR"/log-another.1", "456"));
    mt_fok(file_check(WORKDIR"/log-another.2", "789"));
    mt_fok(file_check(WORKDIR"/log-another.3", "qwe"));
    mt_fok(file_check(WORKDIR"/log-another.4", "asd"));
}


/* ==========================================================================
   ========================================================================== */


static void file_no_dir_for_logs(void)
{
    mt_ferr(el_option(EL_FNAME, "/tmp/i-dont/exist"), ENOENT);
    mt_ferr(el_puts("whatever"), EBADF);
}


/* ==========================================================================
   ========================================================================== */


static void file_dir_removed_after_open_then_created_back_again(void)
{
    mt_fok(el_puts(s8));
    unlink(WORKDIR"/log");
    rmdir(WORKDIR);
    mt_ferr(el_puts(s3), EBADF);
    mkdir(WORKDIR, 0755);
    mt_fok(el_puts(s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_dir_no_access(void)
{
    mkdir("/tmp/embedlog-no-write", 0555);
    if (getuid() == 0)
    {
        /*
         * root just doesn't give a fuck about no-write-permissions
         */

        mt_fok(el_option(EL_FNAME, "/tmp/embedlog-no-write/log"));
        mt_fok(el_puts(s3));
        mt_fok(file_check("/tmp/embedlog-no-write/log", s3));
    }
    else
    {
        mt_ferr(el_option(EL_FNAME, "/tmp/embedlog-no-write/log"), EACCES);
        mt_ferr(el_puts(s3), EBADF);
    }
    unlink("/tmp/embedlog-no-write/log");
    rmdir("/tmp/embedlog-no-write");
}


/* ==========================================================================
   ========================================================================== */


static void file_no_access_to_file(void)
{
    int fd;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    mkdir("/tmp/embedlog-no-write", 0755);
    fd = open("/tmp/embedlog-no-write/log", O_CREAT, 0444);
    close(fd);

    if (getuid() == 0)
    {
        mt_fok(el_option(EL_FNAME, "/tmp/embedlog-no-write/log"));
        mt_fok(el_puts(s5));
        mt_fok(file_check("/tmp/embedlog-no-write/log", s5));
    }
    else
    {
        mt_ferr(el_option(EL_FNAME, "/tmp/embedlog-no-write/log"), EACCES);
        mt_ferr(el_puts("whatever"), EBADF);
    }
    unlink("/tmp/embedlog-no-write/log");
    rmdir("/tmp/embedlog-no-write");
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_no_dir_for_logs(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    mt_ferr(el_option(EL_FNAME, "/tmp/i-dont/exist"), ENOENT);
    mt_ferr(el_puts("whatever"), EBADF);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_dir_removed_after_open_then_created_back_again(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    mt_fok(el_puts(s8));
    mt_fok(el_puts(s8));
    mt_fok(el_puts(s8));
    mt_fok(el_puts(s8));
    mt_fok(el_puts(s8));
    unlink(WORKDIR"/log");
    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    rmdir(WORKDIR);
    mt_ferr(el_puts(s3), EBADF);
    mkdir(WORKDIR, 0755);
    mt_fok(el_puts(s8));
    mt_fok(el_puts(s5));

    mt_fok(file_check(WORKDIR"/log.2", s8 s5));

    mt_fok(el_puts(s8));
    mt_fok(el_puts(s3));
    mt_fok(el_puts(s5));

    mt_fok(file_check(WORKDIR"/log.2", s8 s5));
    mt_fok(file_check(WORKDIR"/log.3", s8 s3 s5));

    mt_fok(el_puts(s9));
    mt_fok(el_puts(s5));

    mt_fok(file_check(WORKDIR"/log.2", s8 s5));
    mt_fok(file_check(WORKDIR"/log.3", s8 s3 s5));
    mt_fok(file_check(WORKDIR"/log.4", s9 s5));

    mt_fok(el_puts(s3));
    mt_fok(el_puts(s8));

    mt_fok(file_check(WORKDIR"/log.1", s8 s5));
    mt_fok(file_check(WORKDIR"/log.2", s8 s3 s5));
    mt_fok(file_check(WORKDIR"/log.3", s9 s5));
    mt_fok(file_check(WORKDIR"/log.4", s3 s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_dir_no_access(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    mkdir("/tmp/embedlog-no-write", 0555);

    if (getuid() == 0)
    {
        mt_fok(el_option(EL_FNAME, "/tmp/embedlog-no-write/log"));
        mt_fok(el_puts(s3));
        mt_fok(file_check("/tmp/embedlog-no-write/log.0", s3));
    }
    else
    {
        mt_ferr(el_option(EL_FNAME, "/tmp/embedlog-no-write/log"), EACCES);
        mt_ferr(el_puts(s3), EBADF);
    }

    unlink("/tmp/embedlog-no-write/log.0");
    rmdir("/tmp/embedlog-no-write");
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_no_access_to_file(void)
{
    int fd;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    el_option(EL_FROTATE_NUMBER, 5);
    mkdir("/tmp/embedlog-no-write", 0755);
    fd = open("/tmp/embedlog-no-write/log.0", O_CREAT, 0444);
    close(fd);

    if (getuid() == 0)
    {
        mt_fok(el_option(EL_FNAME, "/tmp/embedlog-no-write/log"));
        mt_fok(el_puts(s8));
        mt_fok(file_check("/tmp/embedlog-no-write/log.0", s8));
    }
    else
    {
        mt_ferr(el_option(EL_FNAME, "/tmp/embedlog-no-write/log"), EACCES);
        mt_ferr(el_puts("whatever"), EBADF);
    }

    unlink("/tmp/embedlog-no-write/log.0");
    rmdir("/tmp/embedlog-no-write");
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_filename_too_long(void)
{
    char  path[NAME_MAX];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(path, 'a', sizeof(path));
    path[sizeof(path) - 1] = '\0';
    el_option(EL_FROTATE_NUMBER, 5);
    mt_ferr(el_option(EL_FNAME, path), ENAMETOOLONG);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_path_too_long(void)
{
    char  path[PATH_MAX + 2];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(path, 'a', sizeof(path));
    path[0] = '/';
    path[sizeof(path) - 5] = '/';
    path[sizeof(path) - 4] = 'f';
    path[sizeof(path) - 4] = 'i';
    path[sizeof(path) - 3] = 'l';
    path[sizeof(path) - 2] = 'e';
    path[sizeof(path) - 1] = '\0';
    el_option(EL_FROTATE_NUMBER, 5);
    mt_ferr(el_option(EL_FNAME, path), ENAMETOOLONG);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_fail(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    mt_fok(el_puts(s8));
    mt_fok(el_puts(s8));
    unlink(WORKDIR"/log");
    unlink(WORKDIR"/log.0");
    rmdir(WORKDIR);
    mt_ferr(el_puts(s3), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(el_puts(s8));

    mt_fok(file_check(WORKDIR"/log.1", s8));
}


/* ==========================================================================
             __               __
            / /_ ___   _____ / /_   ____ _ _____ ____   __  __ ____
           / __// _ \ / ___// __/  / __ `// ___// __ \ / / / // __ \
          / /_ /  __/(__  )/ /_   / /_/ // /   / /_/ // /_/ // /_/ /
          \__/ \___//____/ \__/   \__, //_/    \____/ \__,_// .___/
                                 /____/                    /_/
   ========================================================================== */


void el_file_test_group(void)
{
    mkdir(WORKDIR, 0755);

    mt_run(file_print_without_init);
    mt_run(file_print_after_cleanup);
    mt_run(file_print_without_setting_file);

    mt_prepare_test = &test_prepare;
    mt_cleanup_test = &test_cleanup;

    mt_run(file_single_message);
    mt_run(file_multi_message);
    mt_run(file_reopen);
    mt_run(file_reopen_different_file);
    mt_run(file_unexpected_third_party_delete);
    mt_run(file_filename_too_long);
    mt_run(file_path_too_long);
    mt_run(file_rotate_1_no_rotate);
    mt_run(file_rotate_1_exact_print);
    mt_run(file_rotate_1_overflow_but_no_rotate);
    mt_run(file_rotate_1_overflow);
    mt_run(file_rotate_1_overflow_exact);
    mt_run(file_rotate_1_reopen);
    mt_run(file_rotate_1_unexpected_third_party_remove);
    mt_run(file_rotate_1_change_size_up);
    mt_run(file_rotate_1_change_size_down);
    mt_run(file_rotate_2_no_rotate);
    mt_run(file_rotate_2_exact_print);
    mt_run(file_rotate_2_overflow_but_no_rotate);
    mt_run(file_rotate_2_overflow);
    mt_run(file_rotate_2_overflow_exact);
    mt_run(file_rotate_2_reopen);
    mt_run(file_rotate_2_unexpected_third_party_remove);
    mt_run(file_rotate_2_change_size_up);
    mt_run(file_rotate_2_change_size_down);
    mt_run(file_rotate_2_many_rotates);
    mt_run(file_rotate_5_no_rotate);
    mt_run(file_rotate_5_exact_print_rotate);
    mt_run(file_rotate_5_overflow_but_no_rotate);
    mt_run(file_rotate_5_overflow);
    mt_run(file_rotate_5_overflow_exact);
    mt_run(file_rotate_5_reopen);
    mt_run(file_rotate_5_hole_in_log_rotate);
    mt_run(file_rotate_5_rename_file_halfway);
    mt_run(file_no_dir_for_logs);
    mt_run(file_dir_removed_after_open_then_created_back_again);
    mt_run(file_dir_no_access);
    mt_run(file_no_access_to_file);
    mt_run(file_rotate_no_dir_for_logs);
    mt_run(file_rotate_dir_removed_after_open_then_created_back_again);
    mt_run(file_rotate_dir_no_access);
    mt_run(file_rotate_no_access_to_file);
    mt_run(file_rotate_filename_too_long);
    mt_run(file_rotate_path_too_long);
    mt_run(file_rotate_fail);

    rmdir(WORKDIR);
}
