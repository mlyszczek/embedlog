/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_thread_safe_main(void)
#else
int main(void)
#endif
{
	el_init();

	if (el_enable_thread_safe(1))
	{
		el_perror(ELF, "Failed to enable thread safety");
		el_cleanup();
		return 1;
	}

	el_print(ELI, "Yeaaa.... there is no really good way to show");
	el_print(ELI, "how pthread works in action, so this just checks");
	el_print(ELI, "whether locking works at all");

	el_cleanup();
	return 0;
}
