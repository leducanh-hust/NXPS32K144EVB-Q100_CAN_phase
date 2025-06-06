#include "DID.h"

const uint8_t support_DID_count = DID_COUNT; // =2
const did_entry_t support_DID_table[] = {
    // {did,offset_index, readCallback, writeCallback, data_length}
    {0x1008, 0, readECULifetime, writeECULifetime, 3},
};

uint8_t readECULifetime(uint8_t *buffer)
{
    if(NVM_Read(DID_Offset(0), buffer, 3) != NVM_OK)
        return 0;
    return 3;
}

uint8_t writeECULifetime(uint8_t *data, uint8_t len)
{
    if (len != 3)
        return 0;
    if(NVM_Write(DID_Offset(0), data, len) != NVM_OK)
        return 0;
    return 3;
}

