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


#ifdef RUN_TESTS

/* variable is set in el-file.c when we successfully executed fsync
 * path of the code
 */

extern int file_synced;
#endif


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


static int symlink_check
(
    const char *t,
    const char *n
)
{
    char        dst[PATH_MAX];
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    /* if target does not exist, link 'n' should not exist too */

    if (t == NULL)
        return -!access(n, F_OK);

    memset(dst, 0, sizeof(dst));
    if (readlink(n, dst, sizeof(dst)) == -1)
        return -1;

    if (strcmp(t, dst) != 0)
        return -1;

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
#ifdef RUN_TESTS
    file_synced = 0;
#endif
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
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_exact_print(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s8);
    el_puts(s8);
    mt_fok(file_check(WORKDIR"/log.0", s8 s8));
    mt_fok(file_check(WORKDIR"/log", s8 s8));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_1_overflow_but_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 1);
    el_puts(s9 s5 s5);
    mt_fok(file_check(WORKDIR"/log.0", s9 s5 "qw"));
    mt_fok(file_check(WORKDIR"/log", s9 s5 "qw"));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s9 s3));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s5));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s5 s8));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s3 s8));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s9 s5 s9 s8));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s5));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fok(file_check(WORKDIR"/log", s9));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s8 s8));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
    mt_fail(access(WORKDIR"/log.1", F_OK) == -1);
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_2_overflow_but_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 2);
    el_puts(s9 s5 s5);
    mt_fok(file_check(WORKDIR"/log.0", s9 s5 "qw"));
    mt_fok(file_check(WORKDIR"/log", s9 s5 "qw"));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s9 s3));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s5));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s8 s5));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s9));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s9 s9 s8));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s5));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s9));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
}


/* ==========================================================================
   ========================================================================== */


static void file_rotate_5_no_rotate(void)
{
    el_option(EL_FROTATE_NUMBER, 5);
    el_puts(s9);
    mt_fok(file_check(WORKDIR"/log.0", s9));
    mt_fok(file_check(WORKDIR"/log", s9));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s8 s8));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s9 s5 "qw"));
    mt_fok(symlink_check("log.1", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s9));
    mt_fok(symlink_check("log.3", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s5));
    mt_fok(symlink_check("log.2", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", s8));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", "tgb"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.4");

    el_puts("123");

    mt_fok(file_check(WORKDIR"/log.0", "qaz"));
    mt_fok(file_check(WORKDIR"/log.2", "edc"));
    mt_fok(file_check(WORKDIR"/log.3", "rfv"));
    mt_fok(file_check(WORKDIR"/log.4", "123"));
    mt_fok(file_check(WORKDIR"/log", "123"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    el_puts("456");

    mt_fok(file_check(WORKDIR"/log.0", "qaz"));
    mt_fok(file_check(WORKDIR"/log.1", "edc"));
    mt_fok(file_check(WORKDIR"/log.2", "rfv"));
    mt_fok(file_check(WORKDIR"/log.3", "123"));
    mt_fok(file_check(WORKDIR"/log.4", "456"));
    mt_fok(file_check(WORKDIR"/log", "456"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    el_puts("789");

    mt_fok(file_check(WORKDIR"/log.0", "edc"));
    mt_fok(file_check(WORKDIR"/log.1", "rfv"));
    mt_fok(file_check(WORKDIR"/log.2", "123"));
    mt_fok(file_check(WORKDIR"/log.3", "456"));
    mt_fok(file_check(WORKDIR"/log.4", "789"));
    mt_fok(file_check(WORKDIR"/log", "789"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    el_puts("qwe");

    mt_fok(file_check(WORKDIR"/log.0", "rfv"));
    mt_fok(file_check(WORKDIR"/log.1", "123"));
    mt_fok(file_check(WORKDIR"/log.2", "456"));
    mt_fok(file_check(WORKDIR"/log.3", "789"));
    mt_fok(file_check(WORKDIR"/log.4", "qwe"));
    mt_fok(file_check(WORKDIR"/log", "qwe"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    el_puts("asd");

    mt_fok(file_check(WORKDIR"/log.0", "123"));
    mt_fok(file_check(WORKDIR"/log.1", "456"));
    mt_fok(file_check(WORKDIR"/log.2", "789"));
    mt_fok(file_check(WORKDIR"/log.3", "qwe"));
    mt_fok(file_check(WORKDIR"/log.4", "asd"));
    mt_fok(file_check(WORKDIR"/log", "asd"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));
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
    mt_fok(file_check(WORKDIR"/log", "tgb"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    mt_fok(file_check(WORKDIR"/log-another.0", "123"));
    mt_fok(file_check(WORKDIR"/log-another.1", "456"));
    mt_fok(file_check(WORKDIR"/log-another.2", "789"));
    mt_fok(file_check(WORKDIR"/log-another.3", "qwe"));
    mt_fok(file_check(WORKDIR"/log-another.4", "asd"));
    mt_fok(file_check(WORKDIR"/log-another", "asd"));
    mt_fok(symlink_check("log-another.4", WORKDIR"/log-another"));
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
    unlink(WORKDIR"/log");
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
    unlink(WORKDIR"/log");
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
    mt_fok(file_check(WORKDIR"/log", "ujm"));
    mt_fok(symlink_check("log.3", WORKDIR"/log"));
    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    unlink(WORKDIR"/log");
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
    mt_fok(file_check(WORKDIR"/log", "qaz"));
    mt_fok(symlink_check("log.0", WORKDIR"/log"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log");
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
    mt_fok(file_check(WORKDIR"/log", "edc"));
    mt_fok(symlink_check("log.2", WORKDIR"/log"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log");
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
    mt_fok(file_check(WORKDIR"/log", "456"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    unlink(WORKDIR"/log");
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
    mt_fok(file_check(WORKDIR"/log", "456"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    unlink(WORKDIR"/log");
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
    mt_fok(file_check(WORKDIR"/log", "789"));
    mt_fok(symlink_check("log.4", WORKDIR"/log"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    unlink(WORKDIR"/log");
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
    unlink(WORKDIR"/log");

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
    mt_fok(file_check(WORKDIR"/log", "qaz"));
    mt_fok(symlink_check("log.3", WORKDIR"/log"));

    unlink(WORKDIR"/log.0");
    unlink(WORKDIR"/log.1");
    unlink(WORKDIR"/log.2");
    unlink(WORKDIR"/log.3");
    unlink(WORKDIR"/log.4");
    unlink(WORKDIR"/log");
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
    mt_fok(file_check(WORKDIR"/log", s8 s5));

    mt_fok(el_puts(s8));
    mt_fok(el_puts(s3));
    mt_fok(el_puts(s5));

    mt_fok(file_check(WORKDIR"/log.0", s8 s5));
    mt_fok(file_check(WORKDIR"/log.1", s8 s3 s5));
    mt_fok(file_check(WORKDIR"/log", s8 s3 s5));

    mt_fok(el_puts(s9));
    mt_fok(el_puts(s5));

    mt_fok(file_check(WORKDIR"/log.0", s8 s5));
    mt_fok(file_check(WORKDIR"/log.1", s8 s3 s5));
    mt_fok(file_check(WORKDIR"/log.2", s9 s5));
    mt_fok(file_check(WORKDIR"/log", s9 s5));

    mt_fok(el_puts(s3));
    mt_fok(el_puts(s8));

    mt_fok(file_check(WORKDIR"/log.0", s8 s5));
    mt_fok(file_check(WORKDIR"/log.1", s8 s3 s5));
    mt_fok(file_check(WORKDIR"/log.2", s9 s5));
    mt_fok(file_check(WORKDIR"/log.3", s3 s8));
    mt_fok(file_check(WORKDIR"/log", s3 s8));
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
    mt_fok(file_check(WORKDIR"/log", s8));
}


/* ==========================================================================
   ========================================================================== */


#ifdef RUN_TESTS
static void file_sync_always(void)
{
    el_option(EL_FSYNC_EVERY, 0);
    mt_fok(el_puts(s8));
    mt_fail(file_synced == 1);
}
#endif


/* ==========================================================================
   ========================================================================== */


#ifdef RUN_TESTS
static void file_sync_via_flush_function(void)
{
    el_option(EL_FSYNC_EVERY, 16);
    mt_fok(el_puts(s8));
    mt_fail(file_synced == 0);
    mt_fok(el_flush());
    mt_fail(file_synced == 1);
}
#endif


/* ==========================================================================
   ========================================================================== */


#ifdef RUN_TESTS
static void file_consecutive_sync_with_flush_function(void)
{
    el_option(EL_FSYNC_EVERY, 16);
    mt_fok(el_puts(s8));
    mt_fail(file_synced == 0);
    mt_fok(el_flush());
    mt_fail(file_synced == 1);
    file_synced = 0;
    mt_fok(el_flush());
    mt_fail(file_synced == 0);
    mt_fok(el_flush());
    mt_fail(file_synced == 0);
    mt_fok(el_puts(s8));
    mt_fail(file_synced == 0);
    mt_fok(el_flush());
    mt_fail(file_synced == 1);
}
#endif


/* ==========================================================================
   ========================================================================== */


#ifdef RUN_TESTS
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
#endif


/* ==========================================================================
   ========================================================================== */


#ifdef RUN_TESTS
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
#endif


/* ==========================================================================
   ========================================================================== */


static void file_rotate_with_padding_in_name
(
    int            *arg
)
{
    int            i;
    int            rotate_number_count;
    int            rotate_number;
    struct dirent  *dirent;
    DIR            *dir;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    /* rotate number = 10, will produce single digit number
     * from .0 to .9. Smart counting logic is in production
     * code so avoid it here to make sure we do not make
     * same bug in prod and test code which could make test
     * do false pasitive.
     */
    rotate_number = *arg;
    if (rotate_number <= 10)
        rotate_number_count = 1;
    else if (rotate_number <= 100)
        rotate_number_count = 2;
    else if (rotate_number <= 1000)
        rotate_number_count = 3;
    else if (rotate_number <= 10000)
        rotate_number_count = 4;

    el_option(EL_FROTATE_SIZE, 1);
    el_option(EL_FROTATE_NUMBER, rotate_number);

    /* perform some more rotation to make sure we create all
     * possible files
     */
    for (i = 0; i != rotate_number + 5; ++i)
        el_puts(s1);

    for (i = 0; i != rotate_number; ++i)
    {
        char expected_file[PATH_MAX];

        sprintf(expected_file, WORKDIR"/log.%0*d", rotate_number_count, i);
        mt_fail(access(expected_file, F_OK) == 0);
        unlink(expected_file);
    }

    unlink(WORKDIR"/log");

    dir = opendir(WORKDIR);
    while ((dirent = readdir(dir)))
    {
        if (strcmp(dirent->d_name, ".") == 0 ||
                strcmp(dirent->d_name, "..") == 0)
            continue;
        mt_fail(0 && "unexpected file left in WORKDIR");
        fprintf(stderr, "# %s\n", dirent->d_name);
    }
    system("rm -f "WORKDIR"/*");
}


/* ==========================================================================
   ========================================================================== */


#if ENABLE_PTHREAD

#define max_file_size 200000 /* 200kB */
#define line_length 7 /* max number will be "999999\n" 0 padded */
#define number_of_rotation 7
#define number_of_threads 32
#define prints_per_rotate (max_file_size / line_length)
#define total_prints (prints_per_rotate * number_of_rotation)
#define prints_per_thread (total_prints / number_of_threads)

static int print_check(void)
{
    char   line[line_length + 1];
    char   expected[line_length + 1];
    FILE  *f;
    int    curr_tid;
    int    curr_mid;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    /* concat and sort files, let's take shortcut with system tools,
     * because why not?
     */

    system("cat "WORKDIR"/log* | sort > "WORKDIR"/result");

    f = fopen(WORKDIR"/result", "r");
    if (f == NULL)
    {
        fprintf(stderr, WORKDIR"/result does not exist\n");
        return -1;
    }

    curr_tid = 0;
    curr_mid = 0;
    for (;;)
    {
        line[sizeof(line) - 1] = 0x55;

        if (fgets(line, sizeof(line), f) == NULL)
        {
            if (feof(f))
            {
                /* got all lines without errors
                 */

                break;
            }

            perror("fgets()");
            fclose(f);
            return -1;
        }

        if (line[sizeof(line) - 1] == '\0' && line[sizeof(line) - 2] != '\n')
        {
            fprintf(stderr, "line is too long '%s', thread interlacing?\n",
                    line);
            fclose(f);
            return -1;
        }

        sprintf(expected, "%02d%04d\n", curr_tid, curr_mid);
        if (strcmp(expected, line) != 0)
        {
            fprintf(stderr, "expected different than line: %s != %s\n",
                    expected, line);
            fclose(f);
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
        fclose(f);
        return -1;
    }

    fclose(f);
    return 0;
}


static void *print_t
(
    void  *arg
)
{
    char   msg[line_length + 1];
    int    i;
    int    n;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    n = *(int *)arg;

    for (i = 0; i != prints_per_thread; ++i)
    {
        sprintf(msg, "%02d%04d", n, i);
        el_print(ELN, "%s", msg);
    }

    return NULL;
}


static void file_print_threaded(void)
{
    pthread_t  pt[number_of_threads];
    int        i, n[number_of_threads];;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    el_init();

    el_option(EL_THREAD_SAFE, 1);
    el_option(EL_PRINT_LEVEL, 0);
    el_option(EL_OUT, EL_OUT_FILE);

    /* rotate around 200kB, line is 7 bytes long, so this will
     * get us 28571 prints before rotation
     */

    el_option(EL_FROTATE_SIZE, max_file_size);
    el_option(EL_FROTATE_NUMBER, 99);
    el_option(EL_FROTATE_SYMLINK, 0);
    el_option(EL_FPATH, WORKDIR"/log");
    el_option(EL_FSYNC_EVERY, max_file_size / 4);

    for (i = 0; i != number_of_threads; ++i)
    {
        n[i] = i;
        pthread_create(&pt[i], NULL, print_t, &n[i]);
    }

    for (i = 0; i != number_of_threads; ++i)
    {
        pthread_join(pt[i], NULL);
    }

    /* do it before print_check() so any buffered data is flushed
     * to disk, otherwise files will be incomplete
     */

    el_flush();

    mt_fok(print_check());

    for (i = 0; i != 99; ++i)
    {
        char path[PATH_MAX];
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


        sprintf(path, WORKDIR"/log.%02d", i);
        unlink(path);
    }

    unlink(WORKDIR"/result");
    el_cleanup();
}

#endif /* ENABLE_PTHREAD */


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
    int  arg;   /* argument for tests */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


#if ENABLE_OUT_FILE
    mkdir(WORKDIR, 0755);

#if ENABLE_PTHREAD
    mt_run(file_print_threaded);
#endif

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

#define FILE_ROTATE_WITH_PADDING_IN_NAME(ARG) \
    arg = ARG; \
    mt_run_param_named(file_rotate_with_padding_in_name, &arg, \
            "file_rotate_with_padding_in_name_" #ARG);
    FILE_ROTATE_WITH_PADDING_IN_NAME(4);
    FILE_ROTATE_WITH_PADDING_IN_NAME(9);
    FILE_ROTATE_WITH_PADDING_IN_NAME(10);
    FILE_ROTATE_WITH_PADDING_IN_NAME(11);
    FILE_ROTATE_WITH_PADDING_IN_NAME(41);
    FILE_ROTATE_WITH_PADDING_IN_NAME(99);
    FILE_ROTATE_WITH_PADDING_IN_NAME(100);
    FILE_ROTATE_WITH_PADDING_IN_NAME(101);
    FILE_ROTATE_WITH_PADDING_IN_NAME(242);
    FILE_ROTATE_WITH_PADDING_IN_NAME(999);
    FILE_ROTATE_WITH_PADDING_IN_NAME(1000);
    FILE_ROTATE_WITH_PADDING_IN_NAME(1001);

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
#ifdef RUN_TESTS
    mt_run(file_sync_always);
    mt_run(file_sync_periodic);
    mt_run(file_sync_level);
    mt_run(file_sync_via_flush_function);
    mt_run(file_consecutive_sync_with_flush_function);
#endif

    rmdir(WORKDIR);
#endif
}
