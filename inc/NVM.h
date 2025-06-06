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

#define NVM_START_ADDRESS ((uint32_t)0x14000000U)
#define NVM_SIZE (4096U) // 4KB

#define DID_COUNT 2
#define DTC_COUNT 2

/* fixed max sizes per slot */
#define DID_MAX_SIZE 8  // bytes per DID (max of readLen/writeLen)
#define DTC_CODE_SIZE 4 // 4 bytes to store a 3-byte code + first byte unused
#define DTC_SNAPSHOT_SIZE 1 // 4 byte for DTC status

/*——— layout offsets in EEPROM ———*/
/* DID region */
#define DID_REGION_OFFSET 0U // Start of FlexRAM region
#define DID_REGION_SIZE (DID_COUNT * DID_MAX_SIZE)

/* DTC-code region immediately after DID */
#define DTC_CODE_OFFSET (DID_REGION_OFFSET + DID_REGION_SIZE)
#define DTC_CODE_REGION_SIZE (DTC_COUNT * DTC_CODE_SIZE)
/* DTC-snapshot region immediately after DTC-code */
#define DTC_SNAPSHOT_OFFSET (DTC_CODE_OFFSET + DTC_CODE_REGION_SIZE)
#define DTC_SNAPSHOT_REGION_SIZE (DTC_COUNT * DTC_SNAPSHOT_SIZE)

typedef enum
{
    NVM_OK = 0,
    NVM_ERROR = -1,
    NVM_INVALID_PARAM = -2
} NVM_Status;

extern NVM_Status NVM_Read(uint32_t offset, uint8_t *data, uint8_t len);
extern NVM_Status NVM_Write(uint32_t offset, const uint8_t *data, uint8_t len);

#endif /* NVM_H_ */
