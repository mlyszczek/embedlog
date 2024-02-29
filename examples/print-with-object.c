/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

#define EL_OPTIONS_OBJECT g_log

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_simple_main(void)
#else
int main(void)
#endif
{
	/* Declaration on stack, this should only be used on RTOS, where
	 * stable ABI is not a concern, but allocating on stack is desireable */
	/* struct el g_log; */
	/* Heap allocating will provide stable ABI in systems with dynamic
	 * linking, your program will not have to be recompilled each time
	 * embedlog changes */
	struct el *g_log;

	/* Stack initialization of embedlog object */
	/* el_oinit(&g_log); */
	/* Heap allocation of embedlog object */
	g_log = el_new();

	/* Now we can print almost just as easy as with default functions.
	 * We just have to use slightly different function name and LEVEL
	 * macro */
	el_oprint(OELI, "Info message");
	el_oprint(OELF, "Fatal message with additional argument %d", 42);
	el_oprint(OELD, "Debug message that won't be printed due to log level");

	/* we can change log level in runtime as we see fit, now enable
	 * debug prints */
	el_oset_log_level(g_log, EL_DBG);
	el_oprint(OELD, "But now debug will be printed");

	/* Cleaning up stack allocated object */
	/* el_ocleanup(&g_log); */
	/* Cleaning up heap allocated object */
	el_destroy(g_log);

	return 0;
}
