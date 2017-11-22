/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"
#include "mtest.h"
#include "test-group-list.h"

mt_defs();  /* definitions for mtest */

int main(void)
{
    el_options_test_group();
    el_print_test_group();
    el_file_test_group();
    mt_return();
}
