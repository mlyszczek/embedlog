/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
         _____________________________________________________________
        / here we handle el_flush function which should make sure all \
        | logs are flushed from buffers and are received by           |
        \ underlying device                                           /
         -------------------------------------------------------------
          \
           \   \
                \ /\
                ( )
              .( o ).
   ==========================================================================
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/
   ========================================================================== */
#include "el-private.h"

#include <errno.h>
#include <stdio.h>


/* ==========================================================================
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ==========================================================================
    Does whatever it takes to make sure data is flushed from buffers and is
    received by underlying devices (which may be block device or remote
    server).
   ========================================================================== */
/* public api */ int el_flush
(
	void
)
{
	return el_oflush(&g_el);
}


/* ==========================================================================
    Same as el_flush() but takes el object as argument
   ========================================================================== */
/* public api */ int el_oflush
(
	struct el  *el   /* options defining printing style */
)
{
	int         rv;  /* return value from function */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	VALID(EINVAL, el);
	el_lock(el);
	VALIDC(ENODEV, el->outputs != 0, el_unlock(el));

	rv = 0;

#if ENABLE_OUT_STDERR
	if (el->outputs & EL_OUT_STDERR) rv |= fflush(stderr);
#endif

#if ENABLE_OUT_STDERR
	if (el->outputs & EL_OUT_STDOUT) rv |= fflush(stdout);
#endif

#if ENABLE_OUT_FILE
	if (el->outputs & EL_OUT_FILE) rv |= el_file_flush(el);
#endif

#if 0
	if (el->outputs & EL_OUT_NET) el_net_flush(el);
#endif

	el_unlock(el);
	return rv;
}
