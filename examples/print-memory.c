/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_memory_main(void)
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

#if 0
	el_pmemory(ELI, ascii, sizeof(ascii));
	el_pmemory(ELI, s, sizeof(s));
#endif

	el_option(EL_PMEMORY_SPACE, 1);
	//el_print(ELI, "Adding spacing might increase visibility in some cases");
	//el_pmemory_table(ELI, ascii, sizeof(ascii));
	//el_pmemory_table(ELI, s, sizeof(s));
	el_pmemory_table(ELI, s, sizeof(s) - 5 - 6 - 1);

	return 0;
}
