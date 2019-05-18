/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
     __________________________________________________________
    /                   mtest version v1.1.4                   \
    |                                                          |
    |    Simple test framework that uses TAP output format     |
    \                 http://testanything.org                  /
     ----------------------------------------------------------
            \    ,-^-.
             \   !oYo!
              \ /./=\.\______
                   ##        )\/\
                    ||-----w||
                    ||      ||

                   Cowth Vader
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include <stdio.h>


/* ==========================================================================
                                        __     __ _
                         ____   __  __ / /_   / /(_)_____
                        / __ \ / / / // __ \ / // // ___/
                       / /_/ // /_/ // /_/ // // // /__
                      / .___/ \__,_//_.___//_//_/ \___/
                     /_/

                   ____ ___   ____ _ _____ _____ ____   _____
                  / __ `__ \ / __ `// ___// ___// __ \ / ___/
                 / / / / / // /_/ // /__ / /   / /_/ /(__  )
                /_/ /_/ /_/ \__,_/ \___//_/    \____//____/

   ========================================================================== */


/* ==========================================================================
    macro with definitions, call this macro no more and no less than ONCE in
    global scope. Its task is to define some variables used by mtest macros.
   ========================================================================== */


#define mt_defs()                                                              \
    const char *curr_test;                                                     \
    int mt_test_status;                                                        \
    int mt_total_tests = 0;                                                    \
    int mt_total_failed = 0;                                                   \
    int mt_total_checks = 0;                                                   \
    int mt_checks_failed = 0;                                                  \
    static void (*mt_prepare_test)(void);                                      \
    static void (*mt_cleanup_test)(void)


/* ==========================================================================
    macro with extern declarations of variables defined by mt_defs().   This
    macro should be called in any .c file,  that uses mt_* function and does
    not have mt_defs() called in.
   ========================================================================== */


#define mt_defs_ext()                                                          \
    extern const char *curr_test;                                              \
    extern int mt_test_status;                                                 \
    extern int mt_total_tests;                                                 \
    extern int mt_total_failed;                                                \
    extern int mt_total_checks;                                                \
    extern int mt_checks_failed;                                               \
    static void (*mt_prepare_test)(void);                                      \
    static void (*mt_cleanup_test)(void)


/* ==========================================================================
    macro runs test 'f'. 'f' is just a function (without parenthesis ()).
   ========================================================================== */


#define mt_run(f) mt_run_named(f, #f)


/* ==========================================================================
    macro runs test 'f' with parameter 'p'.

   'p' can be of any type, as long as it matches prototype of 'f' function.
   ========================================================================== */


#define mt_run_param(f, p) mt_run_param_named(f, p, #f)


/* ==========================================================================
    macro runs test 'f' and instead of printing function name as a test name
    it allows to provide custom name 'n'
   ========================================================================== */


#define mt_run_named(f, n) do {                                                \
    curr_test = n;                                                             \
    mt_test_status = 0;                                                        \
    ++mt_total_tests;                                                          \
    if (mt_prepare_test) mt_prepare_test();                                    \
    f();                                                                       \
    if (mt_cleanup_test) mt_cleanup_test();                                    \
    if (mt_test_status != 0)                                                   \
    {                                                                          \
        fprintf(stdout, "not ok %d - %s\n", mt_total_tests, curr_test);        \
        ++mt_total_failed;                                                     \
    }                                                                          \
    else                                                                       \
        fprintf(stdout, "ok %d - %s\n", mt_total_tests, curr_test);            \
    } while(0)


/* ==========================================================================
    macro runs test 'f' with parameter 'p' and instead of printing function
    name as a test name, it allows to provide custom name 'n'.

   'p' can be of any type, as long as it matches prototype of 'f' function.
   ========================================================================== */


#define mt_run_param_named(f, p, n) do {                                       \
    curr_test = n;                                                             \
    mt_test_status = 0;                                                        \
    ++mt_total_tests;                                                          \
    if (mt_prepare_test) mt_prepare_test();                                    \
    f(p);                                                                      \
    if (mt_cleanup_test) mt_cleanup_test();                                    \
    if (mt_test_status != 0)                                                   \
    {                                                                          \
        fprintf(stdout, "not ok %d - %s\n", mt_total_tests, curr_test);        \
        ++mt_total_failed;                                                     \
    }                                                                          \
    else                                                                       \
        fprintf(stdout, "ok %d - %s\n", mt_total_tests, curr_test);            \
    } while(0)


/* ==========================================================================
    simple assert, when expression 'e' is evaluated to false, assert message
    will be logged, and macro will force function to return
   ========================================================================== */


#define mt_assert(e) do {                                                      \
    ++mt_total_checks;                                                         \
    if (!(e))                                                                  \
    {                                                                          \
        fprintf(stderr, "# assert [%s:%d] %s, %s\n",                           \
                __FILE__, __LINE__, curr_test, #e);                            \
        mt_test_status = -1;                                                   \
        ++mt_checks_failed;                                                    \
        return;                                                                \
    } } while (0)


/* ==========================================================================
    same as mt_assert, but function is not forced to return,  and  test  can
    continue
   ========================================================================== */


#define mt_fail(e) do {                                                        \
    ++mt_total_checks;                                                         \
    if (!(e))                                                                  \
    {                                                                          \
        fprintf(stderr, "# assert [%s:%d] %s, %s\n",                           \
                __FILE__, __LINE__, curr_test, #e);                            \
        mt_test_status = -1;                                                   \
        ++mt_checks_failed;                                                    \
    } } while (0)


/* ==========================================================================
    Checks if function exits with success (return value == 0).
   ========================================================================== */


#define mt_fok(e) do {                                                         \
    int ret;                                                                   \
    ++mt_total_checks;                                                         \
    if ((ret = e) != 0)                                                        \
    {                                                                          \
        fprintf(stderr, "# assert [%s:%d] %s, %s != ok\n",                     \
                __FILE__, __LINE__, curr_test, #e);                            \
        fprintf(stderr, "# assert [%s:%d] %s, return: %d, errno: %d\n",        \
                __FILE__, __LINE__, curr_test, ret, errno);                    \
        mt_test_status = -1;                                                   \
        ++mt_checks_failed;                                                    \
    } } while (0)


/* ==========================================================================
    shortcut macro to test if function fails as expected, with return code
    set to -1, and expected errno errn
   ========================================================================== */


#define mt_ferr(e, errn) do {                                                  \
    errno = 0;                                                                 \
    mt_fail(e == -1);                                                          \
    mt_fail(errno == errn);                                                    \
    if (errno != errn)                                                         \
    {                                                                          \
        fprintf(stderr, "# assert [%s:%d] %s, got errno: %d\n",                \
                __FILE__, __LINE__, curr_test, errno);                         \
    } } while (0)


/* ==========================================================================
    prints test plan, in format 1..<number_of_test_run>.  If all tests  have
    passed, macro will return current function with code 0, else it  returns
    number of failed tests.  If number of failed tests exceeds 254, then 254
    will be returned
   ========================================================================== */


#define mt_return() do {                                                       \
    fprintf(stdout, "1..%d\n", mt_total_tests);                                \
    fprintf(stderr, "# total tests.......:%4d\n", mt_total_tests);            \
    fprintf(stderr, "# passed tests......:%4d\n", mt_total_tests - mt_total_failed); \
    fprintf(stderr, "# failed tests......:%4d\n", mt_total_failed);           \
    fprintf(stderr, "# total checks......:%4d\n", mt_total_checks);           \
    fprintf(stderr, "# passed checks.....:%4d\n", mt_total_checks - mt_checks_failed); \
    fprintf(stderr, "# failed checks.....:%4d\n", mt_checks_failed);          \
    return mt_total_failed > 254 ? 254 : mt_total_failed; } while(0)
