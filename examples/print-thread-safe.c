/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_thread_safe(void)
#else
int main(void)
#endif
{
	el_init();

	if (el_option(EL_THREAD_SAFE, 1) != 0)
	{
		el_perror(ELF, "Failed to enable thread safety");
		goto error;
	}

	el_print(ELI, "Yeaaa.... there is no really good way to show");
	el_print(ELI, "how pthread works in action, so this just checks");
	el_print(ELI, "whether locking works at all");

error:
	el_cleanup();
}
