/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
         ------------------------------------------------------------
        / This file contains functions to put string into specified  \
        | output. String are not processed here, nothing is added or |
        \ removed from passed string, even log level is not checked! /
         ------------------------------------------------------------
         \     /\  ___  /\
          \   // \/   \/ \\
             ((    O O    ))
              \\ /     \ //
               \/  | |  \/
                |  | |  |
                |  | |  |
                |   o   |
                | |   | |
                |m|   |m|
   ==========================================================================
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/
   ========================================================================== */
#include "el-private.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if ENABLE_OUT_SYSLOG
#   include <syslog.h>
#endif


/* ==========================================================================
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ==========================================================================
    puts string 's' to all enabled output facilities specified by default
    el object
   ========================================================================== */
/* public api */ int el_puts
(
	const char  *s  /* string to put into output */
)
{
	return el_oputs(&g_el, s);
}


/* ==========================================================================
    Just like el_oputs_nb() but can lock mutex.
   ========================================================================== */
/* public api */ int el_oputs
(
	struct el   *el,   /* el defining printing style */
	const char  *s     /* string to put into output */
)
{
	int          ret;  /* return from el_oputs_nb() */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_lock(el);
	ret = el_oputs_nb(el, s);
	el_unlock(el);
	return ret;
}



/* ==========================================================================
    puts string 's' to all enabled output facilities specified by el. Does
    not do any locking, thus _nb (non blocking).
   ========================================================================== */
int el_oputs_nb
(
	struct el   *el,  /* el defining printing style */
	const char  *s    /* string to put into output */
)
{
	int          rv;  /* return value from function */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	VALID(EINVAL, s);
	VALID(EINVAL, el);
	VALID(ENODEV, el->outputs != 0);

	rv = 0;

#if ENABLE_OUT_STDERR
	if (el->outputs & EL_OUT_STDERR)
		rv |= fputs(s, stderr) == EOF ? -1 : 0;
#endif

#if ENABLE_OUT_STDERR
	if (el->outputs & EL_OUT_STDOUT)
		rv |= fputs(s, stdout) == EOF ? -1 : 0;
#endif

#if ENABLE_OUT_SYSLOG
	if (el->outputs & EL_OUT_SYSLOG)
		syslog(el->level, s);
#endif

#if ENABLE_OUT_FILE
	if (el->outputs & EL_OUT_FILE)
		rv |= el_file_puts(el, s);
#endif

#if 0
	if (el->outputs & EL_OUT_NET)
		el_puts_net(s);
#endif

#if ENABLE_OUT_TTY
	if (el->outputs & EL_OUT_TTY)
		rv |= el_tty_puts(el, s);
#endif

#if ENABLE_OUT_CUSTOM
	if (el->outputs & EL_OUT_CUSTOM && el->custom_put)
		rv |= el->custom_put(s, strlen(s) + 1, el->custom_put_user);
#endif

	return rv;
}


/* ==========================================================================
    puts memory 'mem' to all supported output facilities specified by
    default el object. Not all outputs support printing binary data
   ========================================================================== */
/* public api */ int el_putb
(
	const void  *mem,  /* memory location to 'print' */
	size_t       mlen  /* size of the mem buffer */
)
{
	return el_oputb(&g_el, mem, mlen);
}


/* ==========================================================================
    Just like el_oputb_nb() but also can perform pthread locking.
   ========================================================================== */
/* public api */ int el_oputb
(
	struct el   *el,   /* el defining printing style */
	const void  *mem,  /* memory location to 'print' */
	size_t       mlen  /* size of the mem buffer */
)
{
	int          ret;  /* return value from el_oputb_nb() */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el_lock(el);
	ret = el_oputb_nb(el, mem, mlen);
	el_unlock(el);
	return ret;
}


/* ==========================================================================
    Puts binary data 'mem' of size 'mlen' to enabled output facilities
    specified by 'el object'. No all outputs support printing binary data.

    This functions does not do any locking - thus _nb (non blocking).
   ========================================================================== */
int el_oputb_nb
(
	struct el   *el,      /* el defining printing style */
	const void  *mem,     /* memory location to 'print' */
	size_t       mlen     /* size of the mem buffer */
)
{
	int          rv;      /* return value from function */
	int          called;  /* at least one output was called */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	VALID(EINVAL, mem);
	VALID(EINVAL, mlen);
	VALID(EINVAL, el);
	VALID(ENODEV, el->outputs != 0);

	rv = 0;
	called = 0;

#if ENABLE_OUT_FILE
	if (el->outputs & EL_OUT_FILE)
	{
		called = 1;
		rv |= el_file_putb(el, mem, mlen);
	}
#endif

#if ENABLE_OUT_CUSTOM
	if (el->outputs & EL_OUT_CUSTOM && el->custom_put)
	{
		called = 1;
		rv |= el->custom_put(mem, mlen, el->custom_put_user);
	}
#endif


	if (called == 0)
	{
		/* couldn't find any supported output for
		 * binary logging */
		errno = ENODEV;
		return -1;
	}

	return rv;
}
