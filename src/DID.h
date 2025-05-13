#pragma once
#include<stdint.h>
#include<stdbool.h>

typedef struct {
    uint16_t did;
    uint8_t (*readCallback) (uint8_t *buffer);
    uint8_t (*writeCallback) (uint8_t *data, uint8_t len);
    uint8_t data_length; // Expected response data length
    uint8_t access_permission;
} did_entry_t;

/*We defined the data of ECU Lifetime contains 3 byte, the time unit is hours*/
uint8_t readECULifetime(uint8_t * buffer)
{
    buffer[0] = 0x01;
    buffer[1] = 0x04;
    buffer[2] = 0x7A;
    return 3; // length
}

did_entry_t support_DID_table[] = {
    {0x1008, readECULifetime, NULL, 3, 0}
};

