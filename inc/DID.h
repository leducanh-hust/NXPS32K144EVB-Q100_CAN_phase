/*
 * DID.h
 *
 *  Created on: May 26, 2025
 *      Author: anhan
 */
#ifndef DID_H_
#define DID_H_

#include "sdk_project_config.h"
#include<stdint.h>
#include<stdbool.h>

typedef struct {
    uint16_t did;
    uint32_t physical_address;
    uint8_t (*readCallback) (uint8_t *buffer);
    uint8_t (*writeCallback) (uint8_t *data, uint8_t len);
    uint8_t data_length; // Expected response data length
} did_entry_t;

/*We defined the data of ECU Lifetime contains 3 byte, the time unit is hours*/
uint8_t readECULifetime(uint8_t * buffer);
uint8_t writeECULifetime(uint8_t *data, uint8_t len);

#endif /* DID_H_ */
