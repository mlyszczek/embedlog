/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


#ifndef EL_OPTIONS_H
#define EL_OPTIONS_H 1


#include "config.h"
#include "embedlog.h"


extern struct options g_options;

int el_log_allowed(struct options *, enum el_level);

#endif
