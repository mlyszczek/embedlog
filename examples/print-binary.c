/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"
#include <string.h>

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_binary_main(void)
#else
int main(void)
#endif
{
	const char *s;
	el_init();

	/* We can't print binary data to stderr, so we need to print file */
	if (el_enable_file_log("/tmp/embedlog-print-binary.bin", 0, 0))
		perror("Error opening file");

	/* Print some data, we print string, but this can be absolutely
	 * anything, First message won't contain timestamp. Notice that
	 * these functions do not take LEVEL macro, but level enum.
	 * There is no point in logging log location in code when doing
	 * binary logs. */
	s = "not so binary";
	el_pbinary(EL_NOTICE, s, strlen(s));

	/* Enable timestamps, but no fractions. */
	el_set_timestamp(EL_TS_SHORT, EL_TS_TM_REALTIME, EL_TS_FRACT_OFF);
	s = "ts in, fract out";
	el_pbinary(EL_WARN, s, strlen(s));

	/* Enable timestamps, with fractions. */
	el_set_timestamp(EL_TS_SHORT, EL_TS_TM_REALTIME, EL_TS_FRACT_US);
	s = "we are all in";
	el_pbinary(EL_FATAL, s, strlen(s));

	el_cleanup();

	return 0;
}
