/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


#ifndef EL_VALID_H
#define EL_VALID_H 1


#include <errno.h>


/* ==========================================================================
    If expression 'x' evaluates to false, macro will set errno value to 'e'
    and will force function to return with code '-1'
   ========================================================================== */


#define VALID(e, x) if (!(x)) { errno = (e); return -1; }


/* ==========================================================================
    Same as VALID but when expression fails, message is printed to default
    embedlog facility
   ========================================================================== */


#define VALIDP(e, x) if (!(x)) { el_print(ELW, "VALID FAILED "#x); \
    errno = (e); return -1; }


/* ==========================================================================
    Same as VALIDP but message is printed to default option, defined by
    EL_OPTIONS_OBJECT
   ========================================================================== */


#define VALIDOP(e, x) if (!(x)) { el_oprint(OELW, "VALID FAILED "#x); \
    errno = (e); return -1; }


/* ==========================================================================
    If expression 'x' evaluates to false, macro will set errno value to 'e'
    and will force function to return value 'v'
   ========================================================================== */


#define VALIDR(e, v, x) if (!(x)) { errno = (e); return (v); }


/* ==========================================================================
    If expression 'x' evaluates to false, macro will set errno value to 'e'
    and will jump to lable 'l'
   ========================================================================== */


#define VALIDG(e, l, x) if (!(x)) { errno = (e); goto l; }


/* ==========================================================================
    If expression 'x' evaluates to false, macro will call code 'c', set
    errno to 'e', and return -1
   ========================================================================== */


#define VALIDC(e, x, c) if (!(x)) { c; errno = (e); return -1; }

#endif
