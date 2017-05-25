#include "embedlog.h"

int main(void)
{
    el_level_set(EL_LEVEL_DBG);
    el_output_enable(EL_OUT_STDERR);

    el_print(ELE, "error message");

    return 0;
}
