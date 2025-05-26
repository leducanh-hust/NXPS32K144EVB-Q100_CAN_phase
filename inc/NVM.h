/*
 * NVM.h
 *
 *  Created on: May 26, 2025
 *      Author: anhan
 */

#ifndef NVM_H_
#define NVM_H_

#include "flash_driver.h"
#include "sdk_project_config.h"
#include <stddef.h>
#define EEPROM_START_ADDRESS ((uint32_t)0x14000000U)
#define EEPROM_SIZE (4096U) // 4KB

typedef enum {
    NVM_OK = 0,
    NVM_ERROR = -1,
    NVM_INVALID_PARAM = -2
} NVM_Status;

NVM_Status NVM_Read(uint32_t offset, uint8_t *data, uint8_t len);
NVM_Status NVM_Write(uint32_t offset, const uint8_t *data, uint8_t len);

#endif /* NVM_H_ */
