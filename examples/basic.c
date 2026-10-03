#include "brc.h"

#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    brc32_ctx_t context;
    const uint32_t domain = 1000u;
    const uint32_t seed = UINT32_C(0x12345678);
    uint32_t value = 0u;
    uint32_t step;

    if (!brc32_init_v1(&context, domain, seed)) {
        return 1;
    }

    for (step = 0u; step < 8u; ++step) {
        printf("%" PRIu32 "%s", value, step == 7u ? "\n" : " ");
        value = brc32_forward_v1(value, &context);
    }
    return 0;
}
