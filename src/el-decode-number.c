/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "el-private.h"

#include <limits.h>


/* ==========================================================================
                       __     __ _          ____
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ========================================================================== */


/* ==========================================================================
    Decodes encoded 'number' and stores value into 'out'. Check description
    of el_encode_number for more information.

    returns number of bytes processed from 'number'
   ========================================================================== */


size_t el_decode_number
(
	const void          *number,  /* number to decode */
#ifdef LLONG_MAX
	unsigned long long  *out      /* decoded number */
#else
	unsigned long       *out      /* decoded number */
#endif
)
{
	const unsigned char *n;       /* just a 'number' as unsigned cahr */
	size_t               i;       /* temporary index */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	i = 0;
	*out = 0;
	n = number;

	do
	{
		/* multiple stuff is happening in this one-liner
		 *
		 * - first we take 7 bits of number
		 * - calculate weigth of current number
		 * - set current number into right position of out */

#ifdef LLONG_MAX
		*out |= (unsigned long long)(n[i] & 0x7f) << (i * 7);
#else
		*out |= (unsigned long)(n[i] & 0x7f) << (i * 7);
#endif

		/* we do this until number lacks of continuation bit, which
		 * means we are done */
	}
	while (n[i++] & 0x80);

	/* return number of bytes processed from 'number' */
	return i;
}
