/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include "embedlog.h"

#define EL_OPTIONS_OBJECT &opts

void like_this(void) { el_print(ELN, "yup, I really like this!"); }
void or_this(void)   { el_print(ELN, "this I like too"); }
void too_long_function_to_present_trimming_but_it_could_be_impossible(void)
{
	el_print(ELN, "trimmed (I hope) function name");
}

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_options(void)
#else
int main(void)
#endif
{
	struct el  opts;

	el_init();

	el_option(EL_PRINT_LEVEL, 0);
	el_print(ELI, "We can disable information about log level\b");
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
	el_option(EL_TS_FRACT, EL_TS_FRACT_OFF);
	el_print(ELF, "and iff you don't need high resolution");
	el_print(ELF, "you can disable fractions of seconds to save space!");
	el_option(EL_TS_FRACT, EL_TS_FRACT_MS);
	el_print(ELF, "or enable only millisecond resolution");
	el_option(EL_TS_FRACT, EL_TS_FRACT_US);
	el_print(ELF, "or enable only microsecond resolution");
	el_option(EL_TS_FRACT, EL_TS_FRACT_NS);
	el_print(ELF, "or enable only nanosecond resolution");
	el_option(EL_TS, EL_TS_LONG);
	el_option(EL_TS_TM, EL_TS_TM_CLOCK);
	el_print(ELF, "or long with clock() if you desire");
	el_option(EL_TS, EL_TS_OFF);
	el_print(ELF, "no time information, if your heart desire it");
	el_option(EL_TS_FRACT, EL_TS_FRACT_NS);

	el_option(EL_FUNCINFO, 1);
	el_print(ELN, "logs can contain function name from which they were called");
	like_this();
	or_this();
	too_long_function_to_present_trimming_but_it_could_be_impossible();

	el_option(EL_FINFO, 1);
	el_print(ELF, "log location is very usefull for debuging");

	el_option(EL_TS, EL_TS_LONG);
	el_option(EL_TS_TM, EL_TS_TM_REALTIME);
	el_option(EL_PRINT_LEVEL, 1);
	el_print(ELF, "Different scenarios need different el object");
	el_print(ELA, "So we can mix options however we want");

	el_option(EL_PRINT_NL, 0);
	el_print(ELF, "you can also remove printing new line ");
	el_puts("to join el_print and el_puts in a single ");
	el_puts("long line as needed\n");
	el_option(EL_PRINT_NL, 1);

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


	el_oinit(&opts);
	el_ooption(&opts, EL_OUT, EL_OUT_STDERR);
	el_oprint(ELI, &opts, "you can do same thing as about with custom");
	el_oprint(ELI, &opts, "el object for two or more logger.");
	el_oprint(OELE, "and if you define EL_OPTIONS_OBJECT you will be");
	el_oprint(OELF, "able to print messages without passing el object");
	el_oprint(OELW, "each time to print functions");
	el_ocleanup(&opts);
}
