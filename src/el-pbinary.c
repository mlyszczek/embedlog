/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
         -------------------------------------------------------------
        / Module reponsible for printing binary logs. And by binary I \
        | don't mean only metadata and logs in ASCII, but full binary |
        | including data. This may be useful when one need to log ie. |
        | every message it saw on CAN. It would be a waste of space   |
        | to log it in ASCII when binary data can be less by an order |
        \ of magnitude                                                /
         -------------------------------------------------------------
          \
           \ ,   _ ___.--'''`--''//-,-_--_.
              \`"' ` || \\ \ \\/ / // / ,-\\`,_
             /'`  \ \ || Y  | \|/ / // / - |__ `-,
            /@"\  ` \ `\ |  | ||/ // | \/  \  `-._`-,_.,
           /  _.-. `.-\,___/\ _/|_/_\_\/|_/ |     `-._._)
           `-'``/  /  |  // \__/\__  /  \__/ \
                `-'  /-\/  | -|   \__ \   |-' |
                  __/\ / _/ \/ __,-'   ) ,' _|'
                 (((__/(((_.' ((___..-'((__,'
   ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "el-private.h"

#include <errno.h>
#include <string.h>
#include <time.h>


/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ========================================================================== */


/* ==========================================================================
    Puts flags with log information to buf. Flags is single byte with fields

    bit 0       timestamp enabled/disabled
    bit 1-2     fraction timestamp (off, nsec, usec, msec)
    bit 3-5     severity of the log
    bit 6-7     reserved
   ========================================================================== */


static size_t el_flags
(
	enum el_level   level,
	struct el      *el,
	unsigned char  *buf
)
{
	*buf = 0;

#if ENABLE_TIMESTAMP
	if (el->timestamp != EL_TS_OFF)
	{
		*buf |= EL_FLAG_TS;

#   if ENABLE_FRACTIONS
		/* fraction of seconds can be printed only when timestamp
		 * is on */

		*buf |= el->timestamp_fractions << EL_FLAG_TS_FRACT_SHIFT;
#   endif
	}
#endif

	*buf |= level << EL_FLAG_LEVEL_SHIFT;
	return 1;
}


/* ==========================================================================
                       __     __ _          ____
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ========================================================================== */


/* ==========================================================================
    Stores data pointed by "memory" of size "mlen" into destination
    specified by "el". All metadata amd "memory" itself will be stored in
    binary format.

    function uses specific format to store metadata to save as much space as
    possible. All numerical values in metadata are variadic in size. Order
    of fields are as follows:

           +-------+------------+--------------+-------------+------+
           | flags | ts_seconds | ts_fractions | data_length | data |
           +-------+------------+--------------+-------------+------+

    The only mandatory fields are flags, data_length and data. flags
    determin what fields are present. flags field is always 1 byte in size
    and its format is

       +------+-------+-------------------------------------------------+
       | bits | value | description                                     |
       +------+-------+-------------------------------------------------+
       |    0 |   0   | both ts_seconds and ts_fraction will not appear |
       |      +-------+-------------------------------------------------+
       |      |   1   | at least ts_seconds will appear, ts_fraction    |
       |      |       | appearance depends on 1..2 bits values          |
       +------+-------+-------------------------------------------------+
       | 1..2 |   0   | ts_fractions will not appear                    |
       |      +-------+-------------------------------------------------+
       |      |   1   | ts_fractions will hold milliseconds value       |
       |      +-------+-------------------------------------------------+
       |      |   2   | ts_fractions will hold microseconds value       |
       |      +-------+-------------------------------------------------+
       |      |   3   | ts_fractions will hold nanoseconds value        |
       +------+-------+-------------------------------------------------+
       | 3..5 |  0..7 | severity of the log                             |
       +------+-------+-------------------------------------------------+
       | 6..7 |  0..3 | reserved                                        |
       +------+-------+-------------------------------------------------+
   ========================================================================== */


/* public api */ int el_opbinary
(
	enum el_level   level,            /* log severity level */
	struct el      *el,               /* el object with info how to print */
	const void     *memory,           /* binary data to store to print */
	size_t          mlen              /* length of "memory" buffer */
)
{
	unsigned char   buf[EL_BUF_MAX];  /* buffer for message to print */
	size_t          l;                /* length of encoded mlen */
	size_t          w;                /* bytes written to buf */
	int             e;                /* cache for errno */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	VALID(EINVAL, mlen);
	VALID(EINVAL, memory);
	VALID(EINVAL, el);
	el_lock(el);
	VALIDC(ENODEV, el->outputs, el_unlock(el));
	VALIDC(ERANGE, el_log_allowed(el, level), el_unlock(el));

	e = 0;
	w  = el_flags(level, el, buf);
	w += el_timestamp(el, buf + w, TS_BINARY);

	/* encode mlen to know how much bytes we are
	 * going to need */
	l = el_encode_number(mlen, buf + w);

	if (w + l + mlen > sizeof(buf))
	{
		/* user tries to print more that than we
		 * can hold in our buffer, buffer overflow
		 * attack? Not going to happen! We
		 * truncate it. */
		mlen = EL_BUF_MAX - w - l;
		e = ENOBUFS;
	}

	/* now that we know real value of mlen, we can
	 * encode mlen again */
	w += el_encode_number(mlen, buf + w);
	memcpy(buf + w, memory, mlen);
	el->level_current_msg = level;
	w += mlen;

	if (el_oputb_nb(el, buf, w) != 0)
	{
		el->level_current_msg = EL_DBG;
		el_unlock(el);
		return -1;
	}

	el->level_current_msg = EL_DBG;

	if (e)
	{
		errno = e;
		el_unlock(el);
		return -1;
	}

	el_unlock(el);
	return 0;
}


/* ==========================================================================
    Same as el_opbinary but uses global el object
   ========================================================================== */


/* public api */ int el_pbinary
(
	enum el_level   level,   /* log severity level */
	const void     *memory,  /* binary data to store to print */
	size_t          mlen     /* length of "memory" buffer */
)
{
	return el_opbinary(level, &g_el, memory, mlen);
}
