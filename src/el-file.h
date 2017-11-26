/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#ifndef EL_FILE_H
#define EL_FILE_H 1

#include "el-options.h"

int el_file_open(struct el_options *);
int el_file_puts(struct el_options *, const char *);
void el_file_cleanup(struct el_options *);

#endif
