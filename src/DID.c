#include "DID.h"

uint8_t readECULifetime(uint8_t * buffer)
{
    return 0;
}

uint8_t writeECULifetime(uint8_t *data, uint8_t len)
{
    return 1;
}

did_entry_t support_DID_table[] = {
    {0x1008, 0x14000000, readECULifetime, writeECULifetime, 3, 2}
};
