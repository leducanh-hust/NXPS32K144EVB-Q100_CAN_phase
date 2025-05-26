/*
 * DTC.h
 *
 *  Created on: May 26, 2025
 *      Author: anhan
 */
#ifndef DTC_H_
#define DTC_H_

#include <stdint.h>

#define NUM_DTC_ENTRIES 2

/* DTC Structure */
typedef struct
{
    uint32_t dtcCode;   /* DTC code (3 bytes typically) */
    uint8_t statusByte; /* Status byte as per ISO 14229-1 */
} UDS_DTC_t;

/* DTC Extended Data Record Structure */
typedef struct
{
    uint32_t dtcCode;     /* DTC code associated with extended data */
    uint8_t recordNumber; /* Record number */
    uint8_t *data;        /* Extended data */
    uint16_t dataLength;  /* Length of extended data */
} UDS_DTC_ExtendedData_t;

//UDS_DTC_t dtcDB[2] = {
//    {0x00A3D800, 0x01, {0x08}, 0x01}, // DTC Description: Steering Button Stuck (Left button fault)
//    {0x00A3D900, 0x01, {0x09}, 0x01}, // DTC Description: Trailer Reverse Steering Button Stuck ( Right button fault)
//};



#endif /* DTC_H_ */
