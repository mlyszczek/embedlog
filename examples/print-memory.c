/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_memory(void)
#else
int main(void)
#endif
{
	char   s[] = "some message\0that contains\0null characters";
	char   ascii[128];
	int    i;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	for (i = 0; i != 128; ++i)  ascii[i] = (char)i;

	el_init();

	el_pmemory(ELI, ascii, sizeof(ascii));
	el_pmemory(ELI, s, sizeof(s));

	return 0;
}
