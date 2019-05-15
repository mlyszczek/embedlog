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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "mtest.h"
#include "embedlog.h"


/* ==========================================================================
                  __        __            __
          ____ _ / /____   / /_   ____ _ / /  _   __ ____ _ _____ _____
         / __ `// // __ \ / __ \ / __ `// /  | | / // __ `// ___// ___/
        / /_/ // // /_/ // /_/ // /_/ // /   | |/ // /_/ // /   (__  )
        \__, //_/ \____//_.___/ \__,_//_/    |___/ \__,_//_/   /____/
       /____/
   ========================================================================== */


/* variable is set in el-file.c when we successfully executed fsync
 * path of the code
 */

int file_synced;


/* ==========================================================================
                                       __                 __
            _____ ____   ____   _____ / /_ ____ _ ____   / /_ _____
           / ___// __ \ / __ \ / ___// __// __ `// __ \ / __// ___/
          / /__ / /_/ // / / /(__  )/ /_ / /_/ // / / // /_ (__  )
          \___/ \____//_/ /_//____/ \__/ \__,_//_/ /_/ \__//____/

   ========================================================================== */


#define WORKDIR "./embedlog-tests"
#define s9 "123456789"
#define s8 "qwertyui"
#define s5 "qwert"
#define s3 "asd"
#define s1 "a"
mt_defs_ext();



/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
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
    el_option(EL_FPATH, WORKDIR"/log");
    el_option(EL_FSYNC_EVERY, 0);
    file_synced = 0;
}


/* ==========================================================================

   ========================================================================== */


static void test_cleanup(void)
{
    int i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    /* remove any file that might have been created during tests.
     * We ignore unlink return code, as we expected that some files
     * won't exist, as in are not needed by tests and thus are not
     * created
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
    el_option(EL_FPATH, WORKDIR"/log");
    el_option(EL_FSYNC_EVERY, 0);

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
    el_option(EL_FPATH, WORKDIR"/log-another");
    el_option(EL_FSYNC_EVERY, 0);

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


static void file_directory_deleted(void)
{
    el_puts(s9);
    unlink(WORKDIR"/log");
    rmdir(WORKDIR);
    mt_ferr(el_puts(s9), ENOENT);
    mkdir(WORKDIR, 0755);
}


/* ==========================================================================
   ========================================================================== */


static void file_directory_reappear_after_delete(void)
{
    el_puts(s9);
    unlink(WORKDIR"/log");
    rmdir(WORKDIR);
    mt_ferr(el_puts(s9), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(el_puts(s9));
    mt_fok(file_check(WORKDIR"/log", s9));
}


/* ==========================================================================
   ========================================================================== */


static void file_write_after_failed_open(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);

    rmdir(WORKDIR);
    mt_ferr(el_option(EL_FPATH, WORKDIR"/log"), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(el_puts(s9));
    mt_fok(file_check(WORKDIR"/log", s9));

    unlink(WORKDIR"/log");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_write_after_failed_open_to_existing_file(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);

    rmdir(WORKDIR);
    mt_ferr(el_option(EL_FPATH, WORKDIR"/log"), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(system("echo test > \""WORKDIR"/log\""));
    mt_fok(el_puts(s9));
    mt_fok(file_check(WORKDIR"/log", "test\n"s9));

    unlink(WORKDIR"/log");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_and_directory_reapear(void)
{
    el_puts(s9);
    unlink(WORKDIR"/log");
    rmdir(WORKDIR);
    mt_ferr(el_puts(s9), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(system("echo test > \""WORKDIR"/log\""));
    mt_fok(el_puts(s9));
    mt_fok(file_check(WORKDIR"/log", "test\n"s9));
}


/* ==========================================================================
   ========================================================================== */


static void file_filename_too_long(void)
{
    char  path[8192];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(path, 'a', sizeof(path));
    path[sizeof(path) - 1] = '\0';
    mt_ferr(el_option(EL_FPATH, path), ENAMETOOLONG);
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
    mt_ferr(el_option(EL_FPATH, path), ENAMETOOLONG);
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
    el_option(EL_FPATH, WORKDIR"/log");
    el_option(EL_FSYNC_EVERY, 0);
    el_cleanup();
    mt_ferr(el_puts("whatev"), ENODEV);
    unlink(WORKDIR"/log");
}


/* ==========================================================================
   ========================================================================== */


static void file_print_without_setting_file(void)
{
    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FROTATE_SIZE, 16);
    el_option(EL_FROTATE_NUMBER, 0);
    el_option(EL_FSYNC_EVERY, 0);
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
    el_option(EL_FPATH, WORKDIR"/log");
    el_option(EL_FSYNC_EVERY, 0);

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
    el_option(EL_FPATH, WORKDIR"/log");
    el_option(EL_FSYNC_EVERY, 0);

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
    el_option(EL_FPATH, WORKDIR"/log");
    el_option(EL_FSYNC_EVERY, 0);

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
    mt_fok(file_check(WORKDIR"/log.2", "edc"));
    mt_fok(file_check(WORKDIR"/log.3", "rfv"));
    mt_fok(file_check(WORKDIR"/log.4", "123"));

    el_puts("456");

    mt_fok(file_check(WORKDIR"/log.0", "qaz"));
    mt_fok(file_check(WORKDIR"/log.1", "edc"));
    mt_fok(file_check(WORKDIR"/log.2", "rfv"));
    mt_fok(file_check(WORKDIR"/log.3", "123"));
    mt_fok(file_check(WORKDIR"/log.4", "456"));

    el_puts("789");

    mt_fok(file_check(WORKDIR"/log.0", "edc"));
    mt_fok(file_check(WORKDIR"/log.1", "rfv"));
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

    el_option(EL_FPATH, WORKDIR"/log-another");

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


static void file_rotate_directory_deleted(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);
    el_option(EL_FROTATE_SIZE, 3);
    el_option(EL_FROTATE_NUMBER, 5);
    mt_fok(el_option(EL_FPATH, WORKDIR"/log"));

    el_puts("qaz");
    el_puts("wsx");
    el_puts("edc");

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    rmdir(WORKDIR);
    mt_ferr(el_puts("rfv"), ENOENT);

    mkdir(WORKDIR, 0755);
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_directory_reappear_after_delete(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);
    el_option(EL_FROTATE_SIZE, 3);
    el_option(EL_FROTATE_NUMBER, 5);
    mt_fok(el_option(EL_FPATH, WORKDIR"/log"));

    el_puts("qaz");
    el_puts("wsx");
    el_puts("edc");

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    rmdir(WORKDIR);
    mt_ferr(el_puts("rfv"), ENOENT);

    mkdir(WORKDIR, 0755);

    mt_fok(el_puts("rfv"));
    mt_fok(el_puts("tgb"));
    mt_fok(el_puts("yhn"));
    mt_fok(el_puts("ujm"));
    mt_fok(file_check(WORKDIR"/log.0", "rfv"));
    mt_fok(file_check(WORKDIR"/log.1", "tgb"));
    mt_fok(file_check(WORKDIR"/log.2", "yhn"));
    mt_fok(file_check(WORKDIR"/log.3", "ujm"));
    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_write_after_failed_open(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);
    el_option(EL_FROTATE_SIZE, 3);
    el_option(EL_FROTATE_NUMBER, 5);

    rmdir(WORKDIR);
    mt_ferr(el_option(EL_FPATH, WORKDIR"/log"), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(el_puts("qaz"));
    mt_fok(file_check(WORKDIR"/log.0", "qaz"));

    unlink(WORKDIR"/log.0");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_write_after_failed_open_to_existing_file(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);
    el_option(EL_FROTATE_SIZE, 3);
    el_option(EL_FROTATE_NUMBER, 5);

    rmdir(WORKDIR);
    mt_ferr(el_option(EL_FPATH, WORKDIR"/log"), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(system("echo qa > \""WORKDIR"/log.0\""));
    mt_fok(system("echo w > \""WORKDIR"/log.1\""));
    mt_fok(el_puts("edc"));
    mt_fok(file_check(WORKDIR"/log.0", "qa\n"));
    mt_fok(file_check(WORKDIR"/log.1", "w\n"));
    mt_fok(file_check(WORKDIR"/log.2", "edc"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_write_after_failed_open_to_existing_file_with_holes(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);
    el_option(EL_FROTATE_SIZE, 3);
    el_option(EL_FROTATE_NUMBER, 5);

    rmdir(WORKDIR);
    mt_ferr(el_option(EL_FPATH, WORKDIR"/log"), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(system("echo qa > \""WORKDIR"/log.0\""));
    mt_fok(system("echo ws > \""WORKDIR"/log.2\""));
    mt_fok(system("echo e > \""WORKDIR"/log.4\""));
    mt_fok(el_puts("123"));
    mt_fok(el_puts("456"));

    mt_fok(file_check(WORKDIR"/log.0", "ws\n"));
    mt_fok(file_check(WORKDIR"/log.2", "e\n"));
    mt_fok(file_check(WORKDIR"/log.3", "123"));
    mt_fok(file_check(WORKDIR"/log.4", "456"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_write_after_failed_open_to_existing_file_with_holes2(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);
    el_option(EL_FROTATE_SIZE, 3);
    el_option(EL_FROTATE_NUMBER, 5);

    rmdir(WORKDIR);
    mt_ferr(el_option(EL_FPATH, WORKDIR"/log"), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(system("echo qa > \""WORKDIR"/log.0\""));
    mt_fok(system("echo ws > \""WORKDIR"/log.3\""));
    mt_fok(system("echo e > \""WORKDIR"/log.4\""));
    mt_fok(el_puts("123"));
    mt_fok(el_puts("456"));

    mt_fok(file_check(WORKDIR"/log.0", "qa\n"));
    mt_fok(file_check(WORKDIR"/log.1", "ws\n"));
    mt_fok(file_check(WORKDIR"/log.2", "e\n"));
    mt_fok(file_check(WORKDIR"/log.3", "123"));
    mt_fok(file_check(WORKDIR"/log.4", "456"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_write_after_failed_open_to_existing_file_with_holes3(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);
    el_option(EL_FROTATE_SIZE, 3);
    el_option(EL_FROTATE_NUMBER, 5);

    rmdir(WORKDIR);
    mt_ferr(el_option(EL_FPATH, WORKDIR"/log"), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(system("echo qa > \""WORKDIR"/log.1\""));
    mt_fok(system("echo ws > \""WORKDIR"/log.3\""));
    mt_fok(system("echo e > \""WORKDIR"/log.4\""));
    mt_fok(el_puts("123"));
    mt_fok(el_puts("456"));
    mt_fok(el_puts("789"));
    mt_fok(file_check(WORKDIR"/log.0", "ws\n"));
    mt_fok(file_check(WORKDIR"/log.1", "e\n"));
    mt_fok(file_check(WORKDIR"/log.2", "123"));
    mt_fok(file_check(WORKDIR"/log.3", "456"));
    mt_fok(file_check(WORKDIR"/log.4", "789"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_and_directory_reappear(void)
{
    /*
     * mt_prepare_test not running here
     */

    el_init();
    el_option(EL_OUT, EL_OUT_FILE);
    el_option(EL_FSYNC_EVERY, 0);
    el_option(EL_FROTATE_SIZE, 3);
    el_option(EL_FROTATE_NUMBER, 5);
    mt_fok(el_option(EL_FPATH, WORKDIR"/log"));

    mt_fok(el_puts("123"));
    mt_fok(el_puts("456"));
    mt_fok(el_puts("789"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");

    rmdir(WORKDIR);
    mt_ferr(el_puts(s9), ENOENT);
    mkdir(WORKDIR, 0755);

    mt_fok(system("echo 12 > \""WORKDIR"/log.0\""));
    mt_fok(system("echo 45 > \""WORKDIR"/log.1\""));
    mt_fok(system("echo 78 > \""WORKDIR"/log.2\""));

    mt_fok(el_puts("qaz"));
    mt_fok(file_check(WORKDIR"/log.0", "12\n"));
    mt_fok(file_check(WORKDIR"/log.1", "45\n"));
    mt_fok(file_check(WORKDIR"/log.2", "78\n"));
    mt_fok(file_check(WORKDIR"/log.3", "qaz"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    el_cleanup();
}


/* ==========================================================================
   ========================================================================== */


static void file_multi_message_on_el_new(void)
{
    struct el  *el;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    mt_assert((el = el_new()) != NULL);
    el_ooption(el, EL_OUT, EL_OUT_FILE);
    el_ooption(el, EL_FSYNC_EVERY, 0);
    el_ooption(el, EL_FROTATE_NUMBER, 0);
    mt_fok(el_ooption(el, EL_FPATH, WORKDIR"/log"));

    el_oputs(el, s9);
    el_oputs(el, s8);
    el_oputs(el, s5);
    mt_fok(file_check(WORKDIR"/log", s9 s8 s5));

    unlink(WORKDIR"/log");
    mt_fok(el_destroy(el));
}


/* ==========================================================================
   ========================================================================== */


static void file_no_dir_for_logs(void)
{
    mt_ferr(el_option(EL_FPATH, WORKDIR"/i-dont/exist"), ENOENT);
    mt_ferr(el_puts("whatever"), ENOENT);
}


/* ==========================================================================
   ========================================================================== */


static void file_dir_removed_after_open_then_created_back_again(void)
{
    mt_fok(el_puts(s8));
    unlink(WORKDIR"/log");
    rmdir(WORKDIR);
    mt_ferr(el_puts(s3), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(el_puts(s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_dir_no_access(void)
{
    mkdir(WORKDIR"/embedlog-no-write", 0555);
    if (getuid() == 0)
    {
        /*
         * root just doesn't give a fuck about no-write-permissions
         */

        mt_fok(el_option(EL_FPATH, WORKDIR"/embedlog-no-write/log"));
        mt_fok(el_puts(s3));
        mt_fok(file_check(WORKDIR"/embedlog-no-write/log", s3));
    }
    else
    {
        mt_ferr(el_option(EL_FPATH, WORKDIR"/embedlog-no-write/log"), EACCES);
        mt_ferr(el_puts(s3), EACCES);
    }
    unlink(WORKDIR"/embedlog-no-write/log");
    rmdir(WORKDIR"/embedlog-no-write");
}


/* ==========================================================================
   ========================================================================== */


static void file_no_access_to_file(void)
{
    int fd;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    mkdir(WORKDIR"/embedlog-no-write", 0755);
    fd = open(WORKDIR"/embedlog-no-write/log", O_CREAT, 0444);
    close(fd);

    if (getuid() == 0)
    {
        mt_fok(el_option(EL_FPATH, WORKDIR"/embedlog-no-write/log"));
        mt_fok(el_puts(s5));
        mt_fok(file_check(WORKDIR"/embedlog-no-write/log", s5));
    }
    else
    {
        mt_ferr(el_option(EL_FPATH, WORKDIR"/embedlog-no-write/log"), EACCES);
        mt_ferr(el_puts("whatever"), EACCES);
    }
    unlink(WORKDIR"/embedlog-no-write/log");
    rmdir(WORKDIR"/embedlog-no-write");
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_no_dir_for_logs(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    mt_ferr(el_option(EL_FPATH, WORKDIR"/i-dont/exist"), ENOENT);
    mt_ferr(el_puts("whatever"), ENOENT);
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
    mt_ferr(el_puts(s3), ENOENT);
    mkdir(WORKDIR, 0755);
    mt_fok(el_puts(s8));
    mt_fok(el_puts(s5));

    mt_fok(file_check(WORKDIR"/log.0", s8 s5));

    mt_fok(el_puts(s8));
    mt_fok(el_puts(s3));
    mt_fok(el_puts(s5));

    mt_fok(file_check(WORKDIR"/log.0", s8 s5));
    mt_fok(file_check(WORKDIR"/log.1", s8 s3 s5));

    mt_fok(el_puts(s9));
    mt_fok(el_puts(s5));

    mt_fok(file_check(WORKDIR"/log.0", s8 s5));
    mt_fok(file_check(WORKDIR"/log.1", s8 s3 s5));
    mt_fok(file_check(WORKDIR"/log.2", s9 s5));

    mt_fok(el_puts(s3));
    mt_fok(el_puts(s8));

    mt_fok(file_check(WORKDIR"/log.0", s8 s5));
    mt_fok(file_check(WORKDIR"/log.1", s8 s3 s5));
    mt_fok(file_check(WORKDIR"/log.2", s9 s5));
    mt_fok(file_check(WORKDIR"/log.3", s3 s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_dir_no_access(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    mkdir(WORKDIR"/embedlog-no-write", 0555);

    if (getuid() == 0)
    {
        mt_fok(el_option(EL_FPATH, WORKDIR"/embedlog-no-write/log"));
        mt_fok(el_puts(s3));
        mt_fok(file_check(WORKDIR"/embedlog-no-write/log.0", s3));
    }
    else
    {
        mt_ferr(el_option(EL_FPATH, WORKDIR"/embedlog-no-write/log"), EACCES);
        mt_ferr(el_puts(s3), EACCES);
    }

    unlink(WORKDIR"/embedlog-no-write/log.0");
    rmdir(WORKDIR"/embedlog-no-write");
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_no_access_to_file(void)
{
    int fd;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    el_option(EL_FROTATE_NUMBER, 5);
    mkdir(WORKDIR"/embedlog-no-write", 0755);
    fd = open(WORKDIR"/embedlog-no-write/log.0", O_CREAT, 0444);
    close(fd);

    if (getuid() == 0)
    {
        mt_fok(el_option(EL_FPATH, WORKDIR"/embedlog-no-write/log"));
        mt_fok(el_puts(s8));
        mt_fok(file_check(WORKDIR"/embedlog-no-write/log.0", s8));
    }
    else
    {
        mt_ferr(el_option(EL_FPATH, WORKDIR"/embedlog-no-write/log"), EACCES);
        mt_ferr(el_puts("whatever"), EACCES);
    }

    unlink(WORKDIR"/embedlog-no-write/log.0");
    rmdir(WORKDIR"/embedlog-no-write");
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_filename_too_long(void)
{
    char  path[8192];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    memset(path, 'a', sizeof(path));
    path[sizeof(path) - 1] = '\0';
    el_option(EL_FROTATE_NUMBER, 5);
    mt_ferr(el_option(EL_FPATH, path), ENAMETOOLONG);
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
    mt_ferr(el_option(EL_FPATH, path), ENAMETOOLONG);
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

    mt_fok(file_check(WORKDIR"/log.0", s8));
}


/* ==========================================================================
   ========================================================================== */


static void file_sync_always(void)
{
    el_option(EL_FSYNC_EVERY, 0);
    mt_fok(el_puts(s8));
    mt_fail(file_synced == 1);
}


/* ==========================================================================
   ========================================================================== */


static void file_sync_via_flush_function(void)
{
    el_option(EL_FSYNC_EVERY, 16);
    mt_fok(el_puts(s8));
    mt_fail(file_synced == 0);
    mt_fok(el_flush());
    mt_fail(file_synced == 1);
}


/* ==========================================================================
   ========================================================================== */


static void file_sync_periodic(void)
{
    el_option(EL_FSYNC_EVERY, 8);
    mt_fok(el_puts(s5));
    mt_fail(file_synced == 0);
    mt_fok(el_puts(s3));
    mt_fail(file_synced == 1);
    file_synced = 0;

    mt_fok(el_puts(s5));
    mt_fail(file_synced == 0);
    mt_fok(el_puts(s8));
    mt_fail(file_synced == 1);
    file_synced = 0;

    mt_fok(el_puts(s5));
    mt_fail(file_synced == 0);
    mt_fok(el_puts(s5));
    mt_fail(file_synced == 1);
}


/* ==========================================================================
   ========================================================================== */


static void file_sync_level(void)
{
    el_option(EL_FSYNC_EVERY, 1024);
    el_option(EL_FSYNC_LEVEL, EL_ERROR);
    el_option(EL_LEVEL, EL_DBG);
    mt_fok(el_print(ELW, s8));
    mt_fail(file_synced == 0);
    mt_fok(el_print(ELW, s8));
    mt_fail(file_synced == 0);
    mt_fok(el_puts(s5));
    mt_fail(file_synced == 0);

    mt_fok(el_print(ELE, s1));
    mt_fail(file_synced == 1);
    file_synced = 0;

    mt_fok(el_print(ELC, s1));
    mt_fail(file_synced == 1);
    file_synced = 0;

    mt_fok(el_print(ELA, s1));
    mt_fail(file_synced == 1);
    file_synced = 0;

    mt_fok(el_print(ELF, s1));
    mt_fail(file_synced == 1);
    file_synced = 0;

    mt_fok(el_print(ELW, s1));
    mt_fail(file_synced == 0);

    mt_fok(el_print(ELN, s1));
    mt_fail(file_synced == 0);

    mt_fok(el_print(ELI, s1));
    mt_fail(file_synced == 0);

    mt_fok(el_print(ELD, s1));
    mt_fail(file_synced == 0);

    mt_fok(el_pmemory_table(ELI, s8, sizeof(s8)));
    mt_fail(file_synced == 0);

    mt_fok(el_pmemory_table(ELF, s8, sizeof(s8)));
    mt_fail(file_synced == 1);
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
#if ENABLE_OUT_FILE
    mkdir(WORKDIR, 0755);

    mt_run(file_print_without_init);
    mt_run(file_print_after_cleanup);
    mt_run(file_print_without_setting_file);
    mt_run(file_write_after_failed_open);
    mt_run(file_write_after_failed_open_to_existing_file);
    mt_run(file_rotate_directory_deleted);
    mt_run(file_rotate_directory_reappear_after_delete);
    mt_run(file_rotate_write_after_failed_open);
    mt_run(file_rotate_write_after_failed_open_to_existing_file);
    mt_run(file_rotate_write_after_failed_open_to_existing_file_with_holes);
    mt_run(file_rotate_write_after_failed_open_to_existing_file_with_holes2);
    mt_run(file_rotate_write_after_failed_open_to_existing_file_with_holes3);
    mt_run(file_rotate_and_directory_reappear);
    mt_run(file_multi_message_on_el_new);

    mt_prepare_test = &test_prepare;
    mt_cleanup_test = &test_cleanup;

    mt_run(file_single_message);
    mt_run(file_multi_message);
    mt_run(file_reopen);
    mt_run(file_reopen_different_file);
    mt_run(file_unexpected_third_party_delete);
    mt_run(file_directory_deleted);
    mt_run(file_directory_reappear_after_delete);
    mt_run(file_and_directory_reapear);
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
    mt_run(file_sync_always);
    mt_run(file_sync_periodic);
    mt_run(file_sync_level);
    mt_run(file_sync_via_flush_function);

    rmdir(WORKDIR);
#endif
}
