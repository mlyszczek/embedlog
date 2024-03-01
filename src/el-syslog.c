/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */
#include "el-private.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#if HAVE_TERMIOS_H
#   include <termios.h>
#endif


/* ==========================================================================
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ==========================================================================
    Open tty device and configure it for output only
   ========================================================================== */
int el_tty_open
(
	struct el      *el,    /* store serial file descriptor here */
	const char     *dev,   /* device to open like /dev/ttyS0 */
	speed_t         speed  /* serial port baud rate */
)
{
	struct termios  tty;   /* serial port settings */
	int             e;     /* holder for errno */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	 /* it is possible to reopen serial device (like changing output device
	 * with EL_TTY_DEV again) during runtime, so we need to close previous
	 * socket first */

	el_tty_close(el);

	if ((el->serial_fd = open(dev, O_WRONLY | O_NOCTTY | O_SYNC)) < 0)
		return -1;

#if HAVE_TERMIOS_H
	/* if termios is not available, simply open device without reconfiguring
	 * it. Some embedded OSes might configure tty during compilation and
	 * have termios disabled to save memory. */

	if (speed == B0)
		/* normally with B0 we should terminate transmission, it is used
		 * with modem lines, but since we do not use modem lines, and in
		 * logger terminating connection does not make any sense, we use
		 * this to tell embedlog *not* to change any settings but only open
		 * serial */
		return 0;

	if (tcgetattr(el->serial_fd, &tty) != 0)
		goto error;

	if (cfsetispeed(&tty, speed) != 0)
		goto error;

	tty.c_cflag &= ~CSIZE;  /* apply size mask */
	tty.c_cflag |= CS8;     /* 8bit transmission */
	tty.c_cflag &= ~PARENB; /* no parity check */
	tty.c_cflag &= ~CSTOPB; /* set one stop bit */

	tty.c_cflag |= CLOCAL;  /* ignore modem lines */
	tty.c_cflag &= ~CREAD;  /* disable receiver - we only send data */
	tty.c_oflag |= OPOST | ONLCR;   /* enable output post-processing by OS */

	if (tcsetattr(el->serial_fd, TCSANOW, &tty) != 0)
		goto error;

	return 0;

error:
	e = errno;
	close(el->serial_fd);
	el->serial_fd = -1;
	errno = e;
	return -1;

#else
	(void)tty;
	(void)e;
	return 0;
#endif
}


/* ==========================================================================
    Yup, you guessed right - it closes serial.
   ========================================================================== */
int el_tty_close
(
	struct el  *el  /* el object with serial descriptor */
)
{
	VALID(EINVAL, el->serial_fd != -1);

	close(el->serial_fd);
	el->serial_fd = -1;
	return 0;
}


/* ==========================================================================
    Simply sends string pointed by s to serial port configured in el
   ========================================================================== */
int el_tty_puts
(
	struct el    *el,  /* el object with serial descriptor */
	const char   *s    /* string to send */
)
{
	return write(el->serial_fd, s, strlen(s));
}
