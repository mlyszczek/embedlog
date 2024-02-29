/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "embedlog.h"

#define WORKDIR "/tmp/embedlog-example"

#ifdef EMBEDLOG_DEMO_LIBRARY
int el_demo_print_to_file_main(void)
#else
int main(void)
#endif
{
	el_init();
	el_set_timestamp(EL_TS_LONG, EL_TS_TM_REALTIME, 0);
	el_print_extra_info(1);
	el_option(EL_FINFO, 1);

	if (mkdir(WORKDIR, 0755) != 0 && errno != EEXIST)
	{
		el_perror(ELF, "Couldn't create directory "WORKDIR);
		goto error;
	}

	/* Enable printing to file, and set log rotation. embedlog will
	 * create maximum 5 files, and non of those files will be larger
	 * than 512. This is demo only, in real life you may want to set
	 * file size to 1M or so. */
	if (el_enable_file_log(WORKDIR"/log", 5, 512))
	{
		/* embedlog will try to open file now, this may fail for
		 * various of reasons */
		if (errno == EINVAL || errno == ENAMETOOLONG)
		{
			/* EINVAL and ENAMETOOLONG are the only non-recovarable
			 * errors, if these are returned you can be sure embedlog
			 * will never successully write to a file. All other errors
			 * can be recovered from, and embedlog will start printing
			 * to file once that condition is fixed. This may be
			 * wrong permission, or not existing directory */
			el_perror(ELF, "Failed to open file for logging");
			goto error;
		}

		el_perror(ELW, "failed to open log file, we will keep trying");
	}

	el_print(ELI, "This message will appear both in standard error");
	el_print(ELI, "and in file %s", WORKDIR"/log");
	el_print(ELI, "in this file there might be another logs like this "
			"from consecutive execution if this code, as embedlog "
			"does not truncate logs but append to file");
	el_print(ELI, "If log cannot fit in current file");
	el_print(ELI, "it will be stored in new file");
	el_print(ELI, "and if library creates EL_FROTATE_NUMBER files");
	el_print(ELI, "oldest file will be deleted and new file will be created");
	el_print(ELI, "run this program multiple times and see how it works");
	el_print(ELI, "File without number, always points to newest log file");

	el_cleanup();
	return 0;

error:
	el_cleanup();
	return 1;
}
