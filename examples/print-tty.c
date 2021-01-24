/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"
#include <termios.h>

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_tty(void)
#else
int main(void)
#endif
{
	/* first we nned to initialize logger to known state */
	el_init();

	/* to use logger you need to enable at least one output,
	 * without it logs will be printed to /dev/null. Here we set
	 * output to serial device. */
	el_option(EL_OUT, EL_OUT_TTY);

	/* enbaling tty output is not enough, we still need to
	 * configure which device we want to use and at what speed.
	 * Transmission parameters are 8N1 by default. Baudrate should
	 * be taken from termios (3). */
	if (el_option(EL_TTY_DEV, "/dev/ttyUSB1", B9600) != 0)
	{
		perror("tty set failed");
		return 1;
	}

	/* now we can simply print messages like we would do it with
	 * ordinary printf - we just need to log level macro as a first
	 * argument */
	el_print(ELI, "Info message");
	el_print(ELF, "Fatal message with additional argument %d", 42);
	el_print(ELD, "Debug message that won't be printed due to log level");

	/* we can change log level in runtime as we see fit, now enable
	 * debug prints */
	el_option(EL_LEVEL, EL_DBG);
	el_print(ELD, "But now debug will be printed");

	/* altough embedlog does not use dynamic allocation by itself,
	 * system may allocate some resources (like opened file
	 * descriptors when printing to file), with el_cleanup, we can
	 * make sure all resources are freed. In this example, this
	 * function will close opened tty file descriptor.  */
	el_cleanup();

	return 0;
}
