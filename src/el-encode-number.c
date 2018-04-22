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


#include <limits.h>

#include "el-private.h"


/* ==========================================================================
                                        __     __ _
                         ____   __  __ / /_   / /(_)_____
                        / __ \ / / / // __ \ / // // ___/
                       / /_/ // /_/ // /_/ // // // /__
                      / .___/ \__,_//_.___//_//_/ \___/
                     /_/
               ____                     __   _
              / __/__  __ ____   _____ / /_ (_)____   ____   _____
             / /_ / / / // __ \ / ___// __// // __ \ / __ \ / ___/
            / __// /_/ // / / // /__ / /_ / // /_/ // / / /(__  )
           /_/   \__,_//_/ /_/ \___/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


/* ==========================================================================
    This will code 'number' to into as small buffer as possible.  Each  byte
    of encoded value can  hold  7  bits  of  data  and  1  bit  (oldest)  of
    continuation information.  If continuation bit is set, that  means  next
    byte contains next 7bit of number information.  This allows us to  store
    value up to 127 in one byte and values up to 16383 in two bytes  and  so
    on.  'out' must be long enough to hold enough encoded bytes or undefined
    behaviour will occur.

    returns number of bytes stored into 'out' buffer.
   ========================================================================== */


size_t el_encode_number
(
#ifdef LLONG_MAX
    unsigned long long  number,  /* value to encode */
#else
    unsigned long       number,  /* value to encode */
#endif
    void               *out      /* memory where encoded value will be set */
)
{
    unsigned char      *o;       /* just 'out' as unsigned char */
    size_t              n;       /* number of bytes stored in 'out' */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    o = out;
    n = 0;

    do
    {
        /*
         * put only youngest 7 bits into out - that's a max number single
         * byte can hold
         */

        o[n] = number & 0x7f;

        /*
         * remove those 7 bits from number, they are used and no longer
         * needed
         */

        number >>= 7;

        /*
         * if we didn't process whole number, set oldest bit to 1, this is
         * continuation bit, it will tell decoder that next bytes will hold
         * next 7bits of number
         */

        if (number)
        {
            o[n] |= 0x80;
        }

        /*
         * increment n to indicate we store a byte in 'out' buffer
         */

        ++n;

        /*
         * do this until there is anyting else left in number
         */
    }
    while (number);

    /*
     * after job's done, return number of bytes stored in 'out'
     */

    return n;
}



