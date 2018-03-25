/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


#ifndef EL_OPTIONS_H
#define EL_OPTIONS_H 1


#include "embedlog.h"


extern struct el_options g_options;

int el_log_allowed(struct el_options *, enum el_level);

#endif
