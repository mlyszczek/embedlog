#include <embedlog.h>
#include <errno.h>

int main(void) {
	const char *file = "/tmp/embedlog-test-file.log";

	el_init();

	el_print(ELN, "first print");

	el_set_timestamp(EL_TS_LONG, EL_TS_TM_REALTIME, EL_TS_FRACT_US);
	el_print(ELW, "warning with timestamp");

	el_print_extra_info(1);
	el_print(ELF, "fatal error, with code location");
	el_print(ELI, "normal log with timestamp and code location");
	el_print(ELD, "won't be printed, log level too low");
	el_set_log_level(EL_DBG);
	el_print(ELD, "but now it will be printed");

	el_enable_file_log(file, 0, 0);
	el_print(ELN, "from now on, logs will be printed to both file and stderr");

	errno = ENAMETOOLONG;
	el_perror(ELA, "simulated error, failed to open file: %s", file);

	el_cleanup();
	return 0;
}
