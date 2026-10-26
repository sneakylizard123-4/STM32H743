#include "dfsd.h"

#include "console.h"

int dfsd_test(void)
{
    console_puts("MIC: skipped, rev A PDM microphone clock wiring is not usable\n");
    console_puts("MIC: MIC1 clock is wired to DFSDM_CKIN1 (an input), "
                 "not to a clock output\n");
    return 0;
}
