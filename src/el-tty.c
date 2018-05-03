/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "el-private.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#if HAVE_TERMIOS_H
#include <termios.h>
#endif

/* ==========================================================================
                                        __     __ _
                         ____   __  __ / /_   / /(_)_____
                        / __ \ / / / // __ \ / // // ___/
                       / /_/ // /_/ // /_/ // // // /__
                      / .___/ \__,_//_.___//_//_/ \___/
                     /_/
               ____                     __   _
              / __/__  __ ____   _____ / /_ (_)____   ____   _____
             / /_ / / / // __ \ / ___// __// // __ \ / __ \ / ___/
            / __// /_/ // / / // /__ / /_ / // /_/ // / / /(__  )
           /_/   \__,_//_/ /_/ \___/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


/* ==========================================================================
    Open tty device and configure it for output only
   ========================================================================== */


int el_tty_open
(
    struct el_options  *options,  /* store serial file descriptor here */
    const char         *dev,      /* device to open like /dev/ttyS0 */
    unsigned int        speed     /* serial port baud rate */
)
{
#if HAVE_TERMIOS_H
    struct termios      tty;      /* serial port settings */
    int                 e;        /* holder for errno */
#endif
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    /*
     * it is possible to reopen serial device (like changing output device
     * with EL_TTY_DEV again) during runtime, so we need to close previous
     * socket first
     */

    el_tty_close(options);

    if ((options->serial_fd = open(dev, O_WRONLY | O_NOCTTY | O_SYNC)) < 0)
    {
        return -1;
    }

#if HAVE_TERMIOS_H
    /*
     * if termios is not available, simply open device without reconfiguring
     * it.  Some embedded OSes might configure tty  during  compilation  and
     * have termios disabled to save memory.
     */

    if (speed == B0)
    {
        /*
         * normally with B0 we should terminate  transmission,  it  is  used
         * with modem lines, but since we do not use  modem  lines,  and  in
         * logger terminating connection does not make  any  sense,  we  use
         * this to tell embedlog *not* to change any settings but only  open
         * serial
         */

        return 0;
    }

    if (tcgetattr(options->serial_fd, &tty) != 0)
    {
        goto error;
    }

    if (cfsetispeed(&tty, (speed_t)speed) != 0)
    {
        goto error;
    }

    tty.c_cflag &= ~CSIZE;  /* apply size mask */
    tty.c_cflag |= CS8;     /* 8bit transmission */
    tty.c_cflag &= ~PARENB; /* no parity check */
    tty.c_cflag &= ~CSTOPB; /* set one stop bit */

    tty.c_cflag |= CLOCAL;  /* ignore modem lines */
    tty.c_cflag &= ~CREAD;  /* disable receiver - we only send data */
    tty.c_oflag |= OPOST | ONLCR;   /* enable output post-processing by OS */

    if (tcsetattr(options->serial_fd, TCSANOW, &tty) != 0)
    {
        goto error;
    }

    return 0;

error:
    e = errno;
    close(options->serial_fd);
    options->serial_fd = -1;
    errno = e;
    return -1;

#else
    return 0;
#endif
}


/* ==========================================================================
    Yup, you guessed right - it closes serial.
   ========================================================================== */


int el_tty_close
(
    struct el_options  *options  /* options object with serial descriptor */
)
{
    VALID(EINVAL, options->serial_fd != -1);

    close(options->serial_fd);
    options->serial_fd = -1;
    return 0;
}


/* ==========================================================================
    Simply sends string pointed by s to serial port configured in options
   ========================================================================== */


int el_tty_puts
(
    struct el_options  *options,  /* options object with serial descriptor */
    const char         *s         /* string to send */
)
{
    return write(options->serial_fd, s, strlen(s));
}
