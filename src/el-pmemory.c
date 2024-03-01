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
   ==========================================================================
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/
   ========================================================================== */
#include "el-private.h"

#include <ctype.h>
#include <stdio.h>


/* ==========================================================================
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ==========================================================================
    prints single memory line, example line can look like this:

    0x0020: 6e 20 70 72 69 6e 74 61 62 6c 65 09 63 68 61 72  n printable.char

    Or like this, if pmemory spacing is set to 1

    0x0020: 6e 20 70 72 69 6e 74  61 62 6c 65 09 63 68 61 72  n printa ble.char
   ========================================================================== */
static int el_print_line
(
	const char            *file,        /* file name where log is printed */
	size_t                 num,         /* line number where log is printed */
	const char            *func,        /* function name to print */
	enum el_level          level,       /* log level to print message with */
	struct el             *el,          /* el defining printing style */
	const unsigned char   *buf,         /* memory location to print */
	size_t                 line_size,   /* size of line in bytes */
	size_t                 line_number  /* line number beign processed */
)
{
	/* buffers to hold whole line of bytes
	 * representation in hex and char */
	char hex_data[EL_MEM_HEX_LEN + 1] = {0};
	char char_data[EL_MEM_CHAR_LEN + 1] = {0};
	char spacing[] = "   ";

	/* calculate buf offset value */
	const unsigned int offset = EL_MEM_LINE_SIZE * line_number;

	/* pointers to current hex and char data */
	char  *current_hex_pos = hex_data;
	char  *current_char_pos = char_data;

	unsigned char  current_byte;
	size_t bn;  /* byte number */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	/* fill data buffers with representation of bytes */
	for (bn = 0; bn < line_size; ++bn)
	{
		current_byte = *buf++;

		if (el->pmemory_space && bn == 8)
		{
			/* If this is 9th byte, then we already printed 8 bytes,
			 * id pmemory_space is set, we add additional spaces */
			sprintf(current_hex_pos, "%*s", el->pmemory_space, spacing);
			sprintf(current_char_pos, "%*s", el->pmemory_space, spacing);
			current_hex_pos += el->pmemory_space;
			current_char_pos += el->pmemory_space;
		}

		sprintf(current_hex_pos, "%02x ", current_byte);
		if (isprint(current_byte) == 0)  current_byte = '.';
		*current_char_pos = current_byte;

		/* Move pointers for current position of hex and char buffers */
		current_hex_pos += EL_MEM_SINGLE_HEX_LEN;
		current_char_pos++;
	}

	/* print constructed line */
	return el_oprint_nb(file, num, func, level, el, "0x%04x  %-*s %s",
			offset, EL_MEM_HEX_LEN - (3 - el->pmemory_space),
			hex_data, char_data);
}


/* ==========================================================================
    Function prints byte from memory location in a nice format.

    Single line of data printed will be formated like this (please note,
    that this is documented for lines that shows only 16 bytes in a line,
    this value can be changed in compile time only).

    0xNNNN  HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH  CCCCCCCCCCCCCCCC

    Where
        NNNN    Adress of the first byte in line
        HH      Byte value in hex representation
        C       Character representation of the byte (or '.' if not printable)

    First 8 bytes are fixed, and consists of offset from the first byte and
    '  ' (2 spaces). Next bytes are counted from formula 3*EL_MEM_LINE_SIZE
    (3 bytes are for 2 hex characters and a space). Next there is  2  bytes
    for space, and EL_MEM_LINE_SIZE bytes for character representation

    If table is set to 1, table will be printed, and message will look like:

    ------  -----------------------------------------------  ----------------
    offset  hex                                              ascii
    ------  -----------------------------------------------  ----------------
    0xNNNN  HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH  CCCCCCCCCCCCCCCC
    ------  -----------------------------------------------  ----------------

    If pmemory space is set to 1, like will be like this:

    0xNNNN  HH HH HH HH HH HH HH HH  HH HH HH HH HH HH HH HH  CCCCCCCC CCCCCCCC
   ========================================================================== */
static int el_pmem
(
	const char     *file,   /* file name where log is printed */
	size_t          num,    /* line number where log is printed */
	const char     *func,   /* function name to print */
	enum el_level   level,  /* log level to print message with */
	struct el      *el,     /* el defining printing style */
	const void     *mem,    /* memory location to print */
	size_t          mlen,   /* number of bytes to print */
	int             table   /* print table? or not to print table? */
)
{
	/* String needed to print separator for array-like formating */
	static const char *separator =
		"-----------------------------------------------------------------------------";

	/* calculate some constants for this call based on mlen */
	const size_t msg_size = mlen;
	const size_t lines_count = msg_size / EL_MEM_LINE_SIZE;
	const size_t last_line_size = msg_size % EL_MEM_LINE_SIZE;

	size_t  line_number;  /* current line number being printed */
	int     rv;           /* return value of print functions */
	int     table_shrink; /* how many extra '-' to add depending on spacing */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	rv = 0;

	/* let's first check if logs are enabled for passed level, if
	 * not, there is no need to waste cycles on useless string
	 * preparation */
	VALID(EINVAL, mem);
	VALID(EINVAL, mlen);
	VALID(EINVAL, el);
	el_lock(el);
	VALIDC(ERANGE, el_log_allowed(el, level), el_unlock(el));


	/* print log table preamble that is:
	 *
	 * ------  -----------------------------------------------  ----------------
	 * offset  hex                                              ascii
	 * ------  -----------------------------------------------  ----------------
	 */

	/* separators in table print for hex and ascii are compile time statics,
	 * with spacing this includes additional 3 characters, and so if spacing
	 * is off, separator will be 3 characters longer than expected, like
	 *
	 *    --------
	 *    fa fc
	 *
	 * When we don't print spacing, these 3 extra '-' should be removed.
	 * Calculate how many bytes we have to remove to have nice alignment */
	table_shrink = 3 - el->pmemory_space;
	if (table)
	{
		rv = 0;
		rv |= el_oprint_nb(file, num, func, level, el, "%.*s  %.*s  %.*s",
				EL_MEM_OFFSET_LEN - 2, separator,
				EL_MEM_HEX_LEN - 1 - table_shrink, separator,
				EL_MEM_CHAR_LEN - table_shrink, separator);

		rv |= el_oprint_nb(file, num, func, level, el, "%-*s%-*s%s",
				EL_MEM_OFFSET_LEN, "offset",
				EL_MEM_HEX_LEN + 1 - table_shrink, "hex", "ascii");

		rv |= el_oprint_nb(file, num, func, level, el, "%.*s  %.*s  %.*s",
				EL_MEM_OFFSET_LEN - 2, separator,
				EL_MEM_HEX_LEN - 1 - table_shrink, separator,
				EL_MEM_CHAR_LEN - table_shrink, separator);
	}

	/* print all lines that contains EL_MEM_LINE_SIZE bytes,
	 * meaning we print all whole lines */
	for (line_number = 0; line_number < lines_count; ++line_number)
	{
		rv |= el_print_line(file, num, func, level, el,
				mem, EL_MEM_LINE_SIZE, line_number);

		/* move memory pointer to point to next chunk of data to
		 * print */
		mem = (const unsigned char *)mem + EL_MEM_LINE_SIZE;
	}

	/* if buflen is not divisible by EL_MEM_LINE_SIZE we still need
	 * to print last line which has less bytes then
	 * EL_MEM_LINE_SIZE. */
	if (last_line_size)
		rv |= el_print_line(file, num, func, level, el,
				mem, last_line_size, line_number);

	/* print ending line
	 *
	 * ------  -----------------------------------------------  ----------------
	 */

	if (table)
		rv |= el_oprint_nb(file, num, func, level, el, "%.*s  %.*s  %.*s",
				EL_MEM_OFFSET_LEN - 2, separator,
				EL_MEM_HEX_LEN - 1 - table_shrink, separator,
				EL_MEM_CHAR_LEN - table_shrink, separator);

	el_unlock(el);
	return rv;
}


/* ==========================================================================
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ==========================================================================
    same as el_pmem and prints table
   ========================================================================== */
/* public api */ int el_opmemory_table
(
	const char     *file,   /* file name where log is printed */
	size_t          num,    /* line number where log is printed */
	const char     *func,   /* function name to print */
	enum el_level   level,  /* log level to print message with */
	struct el      *el,     /* el defining printing style */
	const void     *mem,    /* memory location to print */
	size_t          mlen    /* number of bytes to print */
)
{
	return el_pmem(file, num, func, level, el, mem, mlen, 1);
}


/* ==========================================================================
    same as el_opmemory_table but uses default option object
   ========================================================================== */
/* public api */ int el_pmemory_table
(
	const char      *file,     /* file name where log is printed */
	size_t           num,      /* line number where log is printed */
	const char      *func,     /* function name to print */
	enum el_level    level,    /* log level to print message with */
	const void      *mem,      /* memory location to print */
	size_t           mlen      /* number of bytes to print */
)
{
	return el_pmem(file, num, func, level, &g_el, mem, mlen, 1);
}

/* ==========================================================================
    Same as el_pmem and prints no table
   ========================================================================== */
/* public api */ int el_opmemory
(
	const char    *file,   /* file name where log is printed */
	size_t         num,    /* line number where log is printed */
	const char    *func,   /* function name to print */
	enum el_level  level,  /* log level to print message with */
	struct el     *el,     /* el defining printing style */
	const void    *mem,    /* memory location to print */
	size_t         mlen    /* number of bytes to print */
)
{
	return el_pmem(file, num, func, level, el, mem, mlen, 0);
}


/* ==========================================================================
    same as el_opmemory but uses default option object
   ========================================================================== */
/* public api */ int el_pmemory
(
	const char      *file,     /* file name where log is printed */
	size_t           num,      /* line number where log is printed */
	const char      *func,     /* function name to print */
	enum el_level    level,    /* log level to print message with */
	const void      *mem,      /* memory location to print */
	size_t           mlen      /* number of bytes to print */
)
{
	return el_pmem(file, num, func, level, &g_el, mem, mlen, 0);
}
