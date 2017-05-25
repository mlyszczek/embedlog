/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


#ifndef EL_CONFIGPRIV_H
#define EL_CONFIGPRIV_H 1


/* ==== include files ======================================================= */


#include "config.h"


/* ==== library private macros ============================================== */


/* ==== defines for el_print_mem function =================================== */


/* ==========================================================================
    single line of el_print_mem will be similar to this

    0xNNNN: HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH  CCCCCCCCCCCCCCC
   ========================================================================== */


/* ==========================================================================
    this defines length of "0xNNNN: " part.
   ========================================================================== */


#define EL_MEM_ADR_LEN 8


/* ==========================================================================
    length of the "HH " part
   ========================================================================== */


#define EL_MEM_SINGLE_HEX_LEN 3


/* ==========================================================================
    length of all "HH ".   EL_MEM_LINE_SIZE  is  externally  defined  during
    compilation to tune the output
   ========================================================================== */


#define EL_MEM_HEX_LEN (EL_MEM_SINGLE_HEX_LEN * (EL_MEM_LINE_SIZE))


/* ==========================================================================
    length of all "C" characters
   ========================================================================== */


#define EL_MEM_CHAR_LEN (EL_MEM_LINE_SIZE)


/* ==========================================================================
    Length of the whole line that will be printed
   ========================================================================== */


#define EN_MEM_WHOLE_LINE_LEN (EL_MEM_ADR_LEN + EL_MEM_HEX_LEN + EL_MEM_CHAR_LEN)


/* ==== defines for el_print function ======================================= */


/* ==========================================================================
    length of long timestamp in a single log. Timestamp format is

        [yyyy-mm-dd hh:mm:ss.uuuuuu]

    which is 28 bytes long
   ========================================================================== */


#if ENABLE_TIMESTAMP
    #define EL_PRE_TS_LONG_LEN 28
#else
    #define EL_PRE_TS_LONG_LEN 0
#endif


/* ==========================================================================
    length of short timestamp in a single log. Timestamp format is

        [ssssssssss.uuuuuu]

    which is 19 bytes long. This is maximum value for short timestamp, as it
    can be shorter.
   ========================================================================== */


#define EL_PRE_TS_SHORT_LEN 19


/* ==========================================================================
    maximum file info length. File info is a part with file name and line
    number, it looks like this

        [source-file.c:1337]

    EL_FLEN_MAX is a maximum length of a  file  and  is  defined  externally
    during  compilation  to  suite   users   needs,   3   is   for   special
    characters    "[:]"    and    5    is    maximum    length    of    line
    number.    It   is   assumed   that   nobody    will    compile    files
    longer   than    99999    number    of lines.
   ========================================================================== */


#if ENABLE_FINFO
    #define EL_PRE_FINFO_LEN ((EL_FLEN_MAX) + 3 + 5)
#else
    #define EL_PRE_FINFO_LEN 0
#endif


/* ==========================================================================
    log level preamble string length. Preamble is always "c/" where c is one
    of 'e, w, i, d" depending on the log level. 3 characters are reserved in
    a situation where another preamble (like timestamp)  is  printed  before
    level, and we need to delimiter them with space
   ========================================================================== */


#define EL_PRE_LEVEL_LEN 3


/* ==========================================================================
    maximum length of preamble which may look like this

        [2017-05-24 14:43:10.123456][source-file.c:12345] e/
   ========================================================================== */


#define EL_PRE_LEN (EL_PRE_TS_LONG_LEN + EL_PRE_FINFO_LEN + EL_PRE_LEVEL_LEN)


/* ==========================================================================
    maximum buffer length excluding  new  line  and  null  character.   This
    defines length of final output not format.  So "%s" can  take  even  100
    bytes.    EL_LOG_MAX   is   defined   externally   during   compilation.
   ========================================================================== */


#define EL_BUF_MAX (EL_PRE_LEN + (EL_LOG_MAX))


#endif
