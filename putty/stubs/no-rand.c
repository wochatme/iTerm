/*
 * Stub implementations of RNG functions for applications without an RNG. 
 */

#include "putty.h"

void random_read(void *out, size_t size)
{
    unreachable("Random numbers are not available in this application");
}

void random_save_seed(void)
{
#if 0
    MessageBoxA(NULL, "random", "x", MB_OK);
#endif 
}

void random_destroy_seed(void)
{
}

void noise_ultralight(NoiseSourceId id, unsigned long data)
{
}
