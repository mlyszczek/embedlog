/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

/* this is very minimum program, just to check if embedlog is installed
 * on the system or not
 */

#include <embedlog.h>
#include <stdio.h>

int main(void)
{
    el_init();
    el_option(EL_OPT_OUTPUT, EL_OPT_OUT_STDERR);
    el_print(ELN, "embedlog works!");
    el_cleanup();
    return 0;
}
