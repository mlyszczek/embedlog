/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

int main(void)
{
    el_init();
    el_option(EL_OUT, EL_OUT_STDERR);

    el_option(EL_PRINT_LEVEL, 0);
    el_print(ELI, "We can disable information about log level");
    el_print(ELF, "message still will be filtered by log level");
    el_print(ELA, "but there is no way to tell what level message is");
    el_print(ELD, "like this message will not be printed");

    el_option(EL_TS, EL_TS_SHORT);
    el_print(ELF, "As every respected logger, we also have timestamps");
    el_print(ELF, "which work well with time from clock()");
    el_option(EL_TS_TM, EL_TS_TM_MONOTONIC);
    el_print(ELF, "or CLOCK_MONOTONIC from POSIX");
    el_option(EL_TS, EL_TS_LONG);
    el_option(EL_TS_TM, EL_TS_TM_TIME);
    el_print(ELF, "we also have long format that works well with time()");
    el_option(EL_TS_TM, EL_TS_TM_REALTIME);
    el_print(ELF, "if higher precision is needed we can use CLOCK_REALTIME");
    el_option(EL_TS, EL_TS_SHORT);
    el_print(ELF, "we can also mix REALTIME with short format");
    el_option(EL_TS, EL_TS_LONG);
    el_option(EL_TS_TM, EL_TS_TM_CLOCK);
    el_print(ELF, "or long with clock() if you desire");
    el_option(EL_TS, EL_TS_OFF);
    el_print(ELF, "no time information, if your heart desire it");

    el_option(EL_FINFO, 1);
    el_print(ELF, "log location is very usefull for debuging");

    el_option(EL_TS, EL_TS_LONG);
    el_option(EL_TS_TM, EL_TS_TM_REALTIME);
    el_option(EL_PRINT_LEVEL, 1);
    el_print(ELF, "Different scenarios need different options");
    el_print(ELA, "So we can mix options however we want");

    el_option(EL_COLORS, 1);
    el_option(EL_LEVEL, EL_DBG);
    el_print(ELD, "And if we have");
    el_print(ELI, "modern terminal");
    el_print(ELN, "we can enable colors");
    el_print(ELW, "to spot warnings");
    el_print(ELE, "or errors");
    el_print(ELC, "with a quick");
    el_print(ELA, "glance into");
    el_print(ELF, "log file");

    el_option(EL_PREFIX, "embedlog: ");
    el_print(ELI, "you can also use prefixes");
    el_print(ELI, "to every message you send");

    el_option(EL_PREFIX, NULL);
    el_print(ELI, "set prefix to null to disable it");

    el_cleanup();
}
