#pragma once
#include "sdk_project_config.h"
#include<stdint.h>
#include<stdbool.h>

typedef struct {
    uint16_t did;
    uint32_t physical_address;
    uint8_t (*readCallback) (uint8_t *buffer);
    uint8_t (*writeCallback) (uint8_t *data, uint8_t len);
    uint8_t data_length; // Expected response data length
    uint8_t access_permission;
} did_entry_t;

/*We defined the data of ECU Lifetime contains 3 byte, the time unit is hours*/
did_entry_t *findByDID(uint16_t did)
{
    uint8_t n = sizeof(support_DID_table) / sizeof(did_entry_t);
    for(uint8_t i = 0; i < n; ++i)
    {
        if(support_DID_table[i].did == did)
            return &support_DID_table[i];
    }
    return NULL;
}

uint8_t readECULifetime(uint8_t * buffer)
{
    buffer[0] = 0x01;
    buffer[1] = 0x04;
    buffer[2] = 0x7A;
    return 3; // length
}

uint8_t writeECULifetime(uint8_t *data, uint8_t len)
{
    
}

did_entry_t support_DID_table[] = {
    {0x1008, 0x14000000, readECULifetime, writeECULifetime, 3, 2}
};

uint8_t readDID(uint16_t did, uint8_t *databuf)
{
    uint8_t n = sizeof(support_DID_table) / sizeof(did_entry_t);
    for(uint8_t i = 0; i < n; ++i)
    {
        if(support_DID_table[i].did == did)
        {
            if(support_DID_table[i].readCallback != NULL)
                return support_DID_table[i].readCallback(databuf);
        }
        else 
            return 0;
    }
    return 0;
}
