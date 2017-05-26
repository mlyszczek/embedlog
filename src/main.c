#include "embedlog.h"

int main(void)
{
    el_level_set(EL_LEVEL_DBG);
    el_output_enable(EL_OUT_STDERR);

    el_option(EL_OPT_TS, EL_OPT_TS_OFF);
    el_option(EL_OPT_FINFO, 1);
    el_option(EL_OPT_TS_TM, EL_OPT_TS_TM_REALTIME);
    el_option(EL_OPT_COLORS, 1);

    el_print(ELE, "err message");
    el_print(ELW, "wrn message");
    el_print(ELI, "inf message");
    el_print(ELD, "dbg message");

    el_option(EL_OPT_TS, EL_OPT_TS_SHORT);
    el_print(ELE, "short timestamp");

    el_option(EL_OPT_TS, EL_OPT_TS_LONG);
    el_print(ELE, "long timestamp");

    el_option(EL_OPT_TS_TM, EL_OPT_TS_TM_CLOCK);
    el_print(ELE, "clock timestamp");

    el_option(EL_OPT_TS_TM, EL_OPT_TS_TM_TIME);
    el_print(ELE, "time timestamp");

    el_option(EL_OPT_TS_TM, EL_OPT_TS_TM_REALTIME);
    el_print(ELE, "realtime timestamp");
    el_print(ELE, "realtime timestamp");
    el_print(ELE, "realtime timestamp");
    el_print(ELE, "realtime timestamp");

    el_option(EL_OPT_TS_TM, EL_OPT_TS_TM_MONOTONIC);
    el_print(ELE, "monotonic timestamp");
    el_print(ELE, "monotonic timestamp");
    el_print(ELE, "monotonic timestamp");
    el_print(ELE, "monotonic timestamp");

    el_option(EL_OPT_FINFO, 0);
    el_print(ELE, "without info");

    el_option(EL_OPT_TS, EL_OPT_TS_OFF);
    el_print(ELE, "and without ts");

    return 0;
}
