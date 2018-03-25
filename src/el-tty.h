/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#ifndef EL_TTY_H
#define EL_TTY_H 1

#include "el-options.h"
#include <termios.h>

int el_tty_open(struct el_options *, const char *, speed_t);
int el_tty_puts(struct el_options *, const char *);
void el_tty_close(struct el_options *);

#endif
