/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

#if HAVE_CONFIG_H
#   include "config.h"
#endif

#if HAVE_TERMIOS_H
#   include <termios.h>
#endif

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

#if HAVE_TERMIOS_H
	/* enbaling tty output is not enough, we still need to
	 * configure which device we want to use and at what speed.
	 * Transmission parameters are 8N1 by default. Baudrate should
	 * be taken from termios (3). */
	if (el_option(EL_TTY_DEV, "/dev/ttyUSB1", B9600) != 0)
#else
	/* if termios is not available on your system, you can specify
	 * 0 as baund rate, this will tell embedlog not to configure
	 * serial port and use it as is - ie. it can be configured
	 * at system startup or even during compile time */
	if (el_option(EL_TTY_DEV, "/dev/ttyUSB1", 0) != 0)
#endif
	{
		perror("tty set failed");
		el_cleanup();
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
