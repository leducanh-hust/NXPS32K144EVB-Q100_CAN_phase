#pragma once
#include <stdint.h>

#define NUM_DTC_ENTRIES 2

/* DTC Structure */
typedef struct
{
    uint32_t dtcCode;   /* DTC code (3 bytes typically) */
    uint8_t statusByte; /* Status byte as per ISO 14229-1 */
} UDS_DTC_t;

/* DTC Snapshot Record Structure */
typedef struct
{
    uint32_t dtcCode;     /* DTC code that triggered the snapshot */
    uint8_t recordNumber; /* Record number */
    uint8_t *data;        /* Snapshot data */
    uint16_t dataLength;  /* Length of snapshot data */
} UDS_DTC_Snapshot_t;

/* DTC Extended Data Record Structure */
typedef struct
{
    uint32_t dtcCode;     /* DTC code associated with extended data */
    uint8_t recordNumber; /* Record number */
    uint8_t *data;        /* Extended data */
    uint16_t dataLength;  /* Length of extended data */
} UDS_DTC_ExtendedData_t;

UDS_DTC_t dtcTable[2] = {
    {0x00A3D800, 0x00}, // DTC Description: Steering Button Stuck (Left button fault)
    {0x00A3D900, 0x01}, // DTC Description: Trailer Reverse Steering Button Stuck ( Right button fault)
};
