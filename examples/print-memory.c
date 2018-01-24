/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

int main(void)
{
    char   s[] = "some message\0that contains\0null characters";
    char   ascii[128];
    int    i;
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    for (i = 0; i != 128; ++i)
    {
        ascii[i] = (char)i;
    }

    el_init();
    el_option(EL_OUT, EL_OUT_STDERR);

    el_pmemory(ELI, ascii, sizeof(ascii));
    el_pmemory(ELI, s, sizeof(s));

    return 0;
}
