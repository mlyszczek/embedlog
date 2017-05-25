/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


#ifndef EL_VALID_H
#define EL_VALID_H 1


/* ==== include files ======================================================= */


#include <errno.h>


/* ==== public macros ======================================================= */


/* ==========================================================================
    If expression 'x' evaluates to false,  macro will set errno value to 'e'
    and will force function to return with code '-1'
   ========================================================================== */


#define VALID(e ,x) if (!(x)) { errno = (e); return -1; }


#define VALIDGO(e, x, l) if (!(x)) { errno = (e); goto l; }

#endif
