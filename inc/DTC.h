/*
 * DTC.h
 *
 *  Created on: May 26, 2025
 *      Author: anhan
 */
#ifndef DTC_H_
#define DTC_H_

#include <stdint.h>
#include "NVM.h"

static inline uint32_t DTC_Code_Offset(uint8_t index)
{
    return DTC_CODE_OFFSET + index * DTC_CODE_SIZE;
}

static inline uint32_t DTC_Snapshot_Offset(uint8_t index)
{
    return DTC_SNAPSHOT_OFFSET + index * DTC_SNAPSHOT_SIZE;
}

/* DTC Structure */
typedef struct
{
    uint32_t dtcCode;  /* DTC code (3 bytes typically) */
    uint8_t *snapShot; /*In this demo, this is DTC Status Byte*/
    uint32_t offset_index;
} UDS_DTC_t;

extern UDS_DTC_t dtcDB[2];

#endif /* DTC_H_ */
