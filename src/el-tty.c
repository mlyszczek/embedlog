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

#include "el-options.h"

#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>


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
    speed_t             speed     /* serial port baud rate */
)
{
    struct termios      tty;      /* serial port settings */
    int                 sfd;      /* serial port file descriptor */
    int                 e;        /* holder for errno */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    if ((sfd = open(dev, O_WRONLY | O_NOCTTY | O_SYNC)) < 0)
    {
        return -1;
    }

    if (tcgetattr(sfd, &tty) != 0)
    {
        goto error;
    }

    if (cfsetispeed(&tty, speed) != 0)
    {
        goto error;
    }

    tty.c_cflag &= ~CSIZE;  /* apply size mask */
    tty.c_cflag |= CS8;     /* 8bit transmission */
    tty.c_cflag &= ~PARENB; /* no parity check */
    tty.c_cflag &= ~CSTOPB; /* set one stop bit */

    tty.c_cflag |= CLOCAL;  /* ignore modem lines */
    tty.c_cflag &= ~CREAD;  /* disable receiver - we only send data */
    tty.c_oflag |= OPOST;   /* enable output post-processing by OS */

    if (tcsetattr(sfd, TCSANOW, &tty) != 0)
    {
        goto error;
    }

    options->serial_fd = sfd;
    return 0;

error:
    e = errno;
    close(sfd);
    errno = e;
    return -1;
}


/* ==========================================================================
    Yup, you guessed right - it closes serial.
   ========================================================================== */


int el_tty_close
(
    struct el_options  *options  /* options object with serial descriptor */
)
{
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
