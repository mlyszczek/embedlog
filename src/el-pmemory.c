/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================

   -----------------------------------------------------------
  / This module allows caller to print memory in a nice       \
  | formatted output, there is a little dragon magic here, so |
  | be careful and don't wander too far. See this dog? He     |
  \ wandered a little bit to far                              /
   -----------------------------------------------------------
                         \                    ^    /^
                          \                  / \  // \
                           \   |\___/|      /   \//  .\
                            \  /O  O  \__  /    //  | \ \           *----*
                              /     /  \/_/    //   |  \  \          \   |
                              @___@`    \/_   //    |   \   \         \/\ \
                             0/0/|       \/_ //     |    \    \         \  \
                         0/0/0/0/|        \///      |     \     \       |  |
                      0/0/0/0/0/_|_ /   (  //       |      \     _\     |  /
                   0/0/0/0/0/0/`/,_ _ _/  ) ; -.    |    _ _\.-~       /   /
                               ,-}        _      *-.|.-~-.           .~    ~
              \     \__/        `/\      /                 ~-. _ .-~      /
               \____(oo)           *.   }            {                   /
               (    (--)          .----~-.\        \-`                 .~
               //__\\  \__ Ack!   ///.----..<        \             _ -~
              //    \\               ///-._ _ _ _ _ _ _{^ - - - - ~
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "el-private.h"

#include <ctype.h>
#include <stdio.h>


/* ==========================================================================
                                   _                __
                     ____   _____ (_)_   __ ____ _ / /_ ___
                    / __ \ / ___// /| | / // __ `// __// _ \
                   / /_/ // /   / / | |/ // /_/ // /_ /  __/
                  / .___//_/   /_/  |___/ \__,_/ \__/ \___/
                 /_/
               ____                     __   _
              / __/__  __ ____   _____ / /_ (_)____   ____   _____
             / /_ / / / // __ \ / ___// __// // __ \ / __ \ / ___/
            / __// /_/ // / / // /__ / /_ / // /_/ // / / /(__  )
           /_/   \__,_//_/ /_/ \___/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


/* ==========================================================================
    prints single memory line, example line can look like this:

    0x0020: 6e 20 70 72 69 6e 74 61 62 6c 65 09 63 68 61 72  n printable.char
   ========================================================================== */


static int el_print_line
(
    const char              *file,        /* file name where log is printed */
    size_t                   num,         /* line number where log is printed */
    enum el_level            level,       /* log level to print message with */
    struct el_options       *options,     /* options defining printing style */
    const unsigned char     *buf,         /* memory location to print */
    size_t                   line_size,   /* size of line in bytes */
    size_t                   line_number  /* line number beign processed */
)
{
    /*
     * buffers to hold whole line of bytes representation in hex and char
     */

    char hex_data[EL_MEM_HEX_LEN + 1] = {0};
    char char_data[EL_MEM_CHAR_LEN + 1] = {0};

    /*
     * calculate buf offset value
     */

    const unsigned int offset = EL_MEM_LINE_SIZE * line_number;

    /*
     * fill data buffers with representation of bytes
     */

    size_t bn;  /* byte number */

    for (bn = 0; bn < line_size; ++bn)
    {
        unsigned char  current_byte;
        char          *current_hex_pos;
        char          *current_char_pos;
        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


        current_byte = *buf++;
        current_hex_pos = hex_data + bn * EL_MEM_SINGLE_HEX_LEN;
        current_char_pos = char_data + bn;

        sprintf(current_hex_pos, "%02x ", current_byte);

        if (isprint(current_byte) == 0)
        {
            current_byte = '.';
        }

        *current_char_pos = current_byte;
    }

    /*
     * print constructed line
     */

    return el_oprint(file, num, level, options, "0x%04x  %-*s %s",
        offset, EL_MEM_HEX_LEN, hex_data, char_data);
}


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
    same as el_opmemory but uses default options object
   ========================================================================== */


int el_pmemory
(
    const char      *file,     /* file name where log is printed */
    size_t           num,      /* line number where log is printed */
    enum el_level    level,    /* log level to print message with */
    const void      *mem,      /* memory location to print */
    size_t           mlen      /* number of bytes to print */
)
{
    return el_opmemory(file, num, level, &g_options, mem, mlen);
}


/* ==========================================================================
    Function prints byte from memory location in a nice format.

    Single line of data printed will be formated  like  this  (please  note,
    that this is documented for lines that shows only 16 bytes  in  a  line,
    this value can be changed in compile time only).

    0xNNNN  HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH  CCCCCCCCCCCCCCCC

    Where
        NNNN    Adress of the first byte in line
        HH      Byte value in hex representation
        C       Character representation of the byte (or '.' if not printable)

    First 8 bytes are fixed, and consists of offset from the first byte  and
    '  ' (2 spaces).  Next bytes are counted from formula 3*EL_MEM_LINE_SIZE
    (3 bytes are for 2 hex characters and a space).  Next there is  2  bytes
    for space,  and  EL_MEM_LINE_SIZE  bytes  for  character  representation
   ========================================================================== */


int el_opmemory
(
    const char         *file,     /* file name where log is printed */
    size_t              num,      /* line number where log is printed */
    enum el_level       level,    /* log level to print message with */
    struct el_options  *options,  /* options defining printing style */
    const void         *mem,      /* memory location to print */
    size_t              mlen      /* number of bytes to print */
)
{
    /*
     * String needed to print separator for array-like formating
     */

    static const char *separator =
"-----------------------------------------------------------------------------";

    /*
     * calculate some constants for this call based on mlen
     */

    const size_t msg_size = mlen;
    const size_t lines_count = msg_size / EL_MEM_LINE_SIZE;
    const size_t last_line_size = msg_size % EL_MEM_LINE_SIZE;

    size_t  line_number;  /* current line number being printed */
    int     rv;           /* return value of print functions */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


    /*
     * let's first check if logs are enabled for passed level, if not, there is
     * no need to waste cycles on useless string preparation
     */

    VALID(EINVAL, mem);
    VALID(EINVAL, mlen);
    VALID(EINVAL, options);
    VALID(ERANGE, el_log_allowed(options, level));

    /*
     * print log table preamble that is:
     *
     * ------  -----------------------------------------------  ----------------
     * offset  hex                                              ascii
     * ------  -----------------------------------------------  ----------------
     */

    rv = 0;
    rv |= el_oprint(file, num, level, options, "%.*s  %.*s  %.*s",
        EL_MEM_OFFSET_LEN - 2, separator,
        EL_MEM_HEX_LEN - 1, separator,
        EL_MEM_CHAR_LEN, separator);

    rv |= el_oprint(file, num, level, options, "%-*s%-*s%s",
        EL_MEM_OFFSET_LEN, "offset",
        EL_MEM_HEX_LEN + 1, "hex", "ascii");

    rv |= el_oprint(file, num, level, options, "%.*s  %.*s  %.*s",
        EL_MEM_OFFSET_LEN - 2, separator,
        EL_MEM_HEX_LEN - 1, separator,
        EL_MEM_CHAR_LEN, separator);


    /*
     * print all lines that contains EL_MEM_LINE_SIZE bytes, meaning we print
     * all whole lines
     */

    for (line_number = 0; line_number < lines_count; ++line_number)
    {
        rv |= el_print_line(file, num, level, options,
            mem, EL_MEM_LINE_SIZE, line_number);

        /*
         * move memory pointer to point to next chunk of data to print
         */

        mem = (const unsigned char *)mem + EL_MEM_LINE_SIZE;
    }

    /*
     * if buflen is not divisible by EL_MEM_LINE_SIZE we still need to print
     * last line which has less bytes then EL_MEM_LINE_SIZE.
     */

    if (last_line_size)
    {
        rv |= el_print_line(file, num, level, options,
            mem, last_line_size, line_number);
    }

    /*
     * print ending line
     *
     * ------  -----------------------------------------------  ----------------
     */

    rv |= el_oprint(file, num, level, options, "%.*s  %.*s  %.*s",
        EL_MEM_OFFSET_LEN - 2, separator,
        EL_MEM_HEX_LEN - 1, separator,
        EL_MEM_CHAR_LEN, separator);

    return rv;
}
