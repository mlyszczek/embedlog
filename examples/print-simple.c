/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include <embedlog.h>

int main(void)
{
    /*
     * first we nned to initialize logger to known state
     */

    el_init();

    /*
     * to use logger you need to enable at least one output, without it logs
     * will be printed to /dev/null
     */

    el_option(EL_OPT_OUT, EL_OPT_OUT_STDERR);

    /*
     * now we can simply print messages like we would do it with ordinary
     * printf - we just need to log level macro as a first argument
     */

    el_print(ELI, "Info message");
    el_print(ELF, "Fatal message with additional argument %d", 42);
    el_print(ELD, "Debug message that won't be printed due to log level");

    /*
     * we can change log level in runtime as we see fit, now enable debug
     * prints
     */

    el_option(EL_OPT_LEVEL, EL_DBG);
    el_print(ELD, "But now debug will be printed");

    /*
     * altough embedlog does not use dynamic allocation by itself, system may
     * allocate some resources (like opened file descriptors when printing to
     * file), with el_cleanup, we can make sure all resources are freed.
     */

    el_cleanup();

    return 0;
}
