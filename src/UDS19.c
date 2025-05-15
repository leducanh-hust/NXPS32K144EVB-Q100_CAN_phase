#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* UDS Service IDs */
#define SID_READ_DTC_INFORMATION 0x19

/* Service 19 - Read DTC Information Sub-functions */
#define SF_REPORT_NUMBER_OF_DTC_BY_STATUS_MASK 0x01
#define SF_REPORT_DTC_BY_STATUS_MASK 0x02
#define SF_REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER 0x04
#define SF_REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER 0x06

/* Negative Response Codes */
#define UDS_RESPONSE_POSITIVE 0x00
#define UDS_RESPONSE_GENERAL_REJECT 0x10
#define UDS_RESPONSE_SERVICE_NOT_SUPPORTED 0x11
#define UDS_RESPONSE_SUBFUNCTION_NOT_SUPPORTED 0x12
#define UDS_RESPONSE_INCORRECT_LENGTH_OR_FORMAT 0x13
#define UDS_RESPONSE_RESPONSE_TOO_LONG 0x14
#define UDS_RESPONSE_BUSY_REPEAT_REQUEST 0x21
#define UDS_RESPONSE_CONDITIONS_NOT_CORRECT 0x22
#define UDS_RESPONSE_REQUEST_SEQUENCE_ERROR 0x24
#define UDS_RESPONSE_NO_RESPONSE_FROM_SUBNET 0x25
#define UDS_RESPONSE_FAILURE_PREVENTS_EXECUTION 0x26
#define UDS_RESPONSE_REQUEST_OUT_OF_RANGE 0x31
#define UDS_RESPONSE_SECURITY_ACCESS_DENIED 0x33

/* DTC Status Bit Masks */
#define DTC_STATUS_TEST_FAILED 0x01
#define DTC_STATUS_TEST_FAILED_THIS_OPERATION_CYCLE 0x02
#define DTC_STATUS_PENDING 0x04
#define DTC_STATUS_CONFIRMED 0x08
#define DTC_STATUS_TEST_NOT_COMPLETED_SINCE_CLEAR 0x10
#define DTC_STATUS_TEST_FAILED_SINCE_CLEAR 0x20
#define DTC_STATUS_TEST_NOT_COMPLETED_THIS_CYCLE 0x40
#define DTC_STATUS_WARNING_INDICATOR_REQUESTED 0x80

/* DTC Format Identifiers */
#define DTC_FORMAT_ISO15031_6 0x01   /* SAE J2012 format */
#define DTC_FORMAT_ISO14229_1 0x02   /* ISO 14229-1 format */
#define DTC_FORMAT_SAE_J1939_73 0x03 /* SAE J1939-73 format */

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

/* DTC Database (For demonstration purposes - in a real system this would be stored in memory) */
#define MAX_DTC_COUNT 100
#define MAX_SNAPSHOT_COUNT 50
#define MAX_EXTENDED_COUNT 50

/* Global Variables - In a real implementation, these would typically be static or managed differently */
UDS_DTC_t g_dtcDatabase[MAX_DTC_COUNT];
uint16_t g_dtcCount = 0;

UDS_DTC_Snapshot_t g_dtcSnapshots[MAX_SNAPSHOT_COUNT];
uint16_t g_snapshotCount = 0;

UDS_DTC_ExtendedData_t g_dtcExtendedData[MAX_EXTENDED_COUNT];
uint16_t g_extendedDataCount = 0;

/* Buffer for response data */
#define MAX_RESPONSE_LENGTH 4095
uint8_t g_responseBuffer[MAX_RESPONSE_LENGTH];
uint16_t g_responseLength = 0;

/* Function Prototypes */
uint8_t UDS_Service19_Handler(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength);
uint8_t UDS_ReportNumberOfDTCByStatusMask(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength);
uint8_t UDS_ReportDTCByStatusMask(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength);
uint8_t UDS_ReportDTCSnapshotRecordByDTCNumber(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength);
uint8_t UDS_ReportDTCExtendedDataRecordByDTCNumber(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength);

/* Helper functions */
void UDS_AddDTC(uint32_t dtcCode, uint8_t status);
void UDS_AddDTCSnapshot(uint32_t dtcCode, uint8_t recordNumber, uint8_t *data, uint16_t dataLength);
void UDS_AddDTCExtendedData(uint32_t dtcCode, uint8_t recordNumber, uint8_t *data, uint16_t dataLength);
uint16_t UDS_CountDTCByStatusMask(uint8_t statusMask);

/**
 * Main handler for UDS Service 19 (Read DTC Information)
 *
 * @param requestData Pointer to request data buffer
 * @param requestLength Length of request data
 * @param responseData Pointer to response data buffer
 * @param responseLength Pointer to response length
 * @return UDS response code
 */
uint8_t UDS_Service19_Handler(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength)
{
    uint8_t subFunction;
    uint8_t responseCode = UDS_RESPONSE_POSITIVE;

    /* Check if request is too short */
    if (requestLength < 2)
    {
        return UDS_RESPONSE_INCORRECT_LENGTH_OR_FORMAT; /*NRC 0x13*/
    }

    /* Get sub-function from request */
    subFunction = requestData[1];

    /* Set positive response SID */
    responseData[0] = SID_READ_DTC_INFORMATION + 0x40; /* Positive response = SID + 0x40 */
    responseData[1] = subFunction;
    *responseLength = 2; /* Initialize response length */

    /* Process sub-function */
    switch (subFunction)
    {
    case SF_REPORT_NUMBER_OF_DTC_BY_STATUS_MASK:
        responseCode = UDS_ReportNumberOfDTCByStatusMask(&requestData[1], requestLength - 1, &responseData[1], responseLength);
        break;

    case SF_REPORT_DTC_BY_STATUS_MASK:
        responseCode = UDS_ReportDTCByStatusMask(&requestData[1], requestLength - 1, &responseData[1], responseLength);
        break;

    case SF_REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER:
        responseCode = UDS_ReportDTCSnapshotRecordByDTCNumber(&requestData[1], requestLength - 1, &responseData[1], responseLength);
        break;

    case SF_REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER:
        responseCode = UDS_ReportDTCExtendedDataRecordByDTCNumber(&requestData[1], requestLength - 1, &responseData[1], responseLength);
        break;

    default:
        responseCode = UDS_RESPONSE_SUBFUNCTION_NOT_SUPPORTED;
        break;
    }

    /* If there was an error, send negative response */
    if (responseCode != UDS_RESPONSE_POSITIVE)
    {
        responseData[0] = 0x7F; /* Negative response format */
        responseData[1] = SID_READ_DTC_INFORMATION;
        responseData[2] = responseCode;
        *responseLength = 3;
    }

    return responseCode;
}

/**
 * Handler for subfunction 0x01: Report Number of DTC by Status Mask
 *
 * Request format:  [SF(0x01)][StatusMask]
 * Response format: [SF(0x01)][StatusMask][DTCFormatIdentifier][DTCCount(2 bytes)]
 */
uint8_t UDS_ReportNumberOfDTCByStatusMask(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength)
{
    uint8_t dtcStatusMask;
    uint16_t matchingDTCCount;

    /* Check request length */
    if (requestLength < 2)
    {
        return UDS_RESPONSE_INCORRECT_LENGTH_OR_FORMAT;
    }

    /* Get status mask from request */
    dtcStatusMask = requestData[1];

    /* Count DTCs matching the status mask */
    matchingDTCCount = UDS_CountDTCByStatusMask(dtcStatusMask);

    /* Format response */
    responseData[0] = requestData[0]; /* Echo subfunction */
    responseData[1] = dtcStatusMask;  /* Echo status mask */

    /* DTC format identifier (ISO 14229-1 DTCs) */
    responseData[2] = DTC_FORMAT_ISO14229_1; /* DTC format: ISO 14229-1 */

    /* Set DTC count in response (16-bit value) */
    responseData[3] = (uint8_t)((matchingDTCCount >> 8) & 0xFF); /* High byte */
    responseData[4] = (uint8_t)(matchingDTCCount & 0xFF);        /* Low byte */

    *responseLength = 5; /* Subfunction + StatusMask + FormatID + DTC_Count (2 bytes) */

    return UDS_RESPONSE_POSITIVE;
}

/**
 * Handler for subfunction 0x02: Report DTC by Status Mask
 *
 * Request format:  [SF(0x02)][StatusMask]
 * Response format: [SF(0x02)][StatusMask][DTCFormatIdentifier][DTC1(3 bytes)][Status1][DTC2(3 bytes)][Status2]...
 */
uint8_t UDS_ReportDTCByStatusMask(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength)
{
    uint8_t dtcStatusMask;
    uint16_t responseIndex = 3; /* Start after subfunction, status mask, and format ID */
    uint16_t i;

    /* Check request length */
    if (requestLength < 2)
    {
        return UDS_RESPONSE_INCORRECT_LENGTH_OR_FORMAT;
    }

    /* Get status mask from request */
    dtcStatusMask = requestData[1];

    /* Format response header */
    responseData[0] = requestData[0];        /* Echo subfunction */
    responseData[1] = dtcStatusMask;         /* Echo status mask */
    responseData[2] = DTC_FORMAT_ISO14229_1; /* DTC format: ISO 14229-1 */

    /* Add each matching DTC to response */
    for (i = 0; i < g_dtcCount; i++)
    {
        /* Check if DTC status matches the requested mask */
        if ((g_dtcDatabase[i].statusByte & dtcStatusMask) != 0)
        {
            /* Check if we have enough space in response buffer */
            if ((responseIndex + 4) > MAX_RESPONSE_LENGTH)
            {
                return UDS_RESPONSE_RESPONSE_TOO_LONG;
            }

            /* Add DTC code (3 bytes) */
            responseData[responseIndex++] = (uint8_t)((g_dtcDatabase[i].dtcCode >> 16) & 0xFF);
            responseData[responseIndex++] = (uint8_t)((g_dtcDatabase[i].dtcCode >> 8) & 0xFF);
            responseData[responseIndex++] = (uint8_t)(g_dtcDatabase[i].dtcCode & 0xFF);

            /* Add DTC status byte */
            responseData[responseIndex++] = g_dtcDatabase[i].statusByte;
        }
    }

    *responseLength = responseIndex;

    return UDS_RESPONSE_POSITIVE;
}

/**
 * Handler for subfunction 0x04: Report DTC Snapshot Record by DTC Number
 *
 * Request format:  [SF(0x04)][DTC(3 bytes)][RecordNumber (optional)]
 * Response format: [SF(0x04)][DTC(3 bytes)][RecordNumber][SnapshotData]...
 */
uint8_t UDS_ReportDTCSnapshotRecordByDTCNumber(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength)
{
    uint32_t requestedDTC;
    uint8_t requestedRecordNum = 0xFF; /* Default to all records */
    uint16_t responseIndex = 1;        /* Start after subfunction */
    uint16_t i;
    bool dtcFound = false;

    /* Check request length */
    if (requestLength < 4)
    {
        return UDS_RESPONSE_INCORRECT_LENGTH_OR_FORMAT;
    }

    /* Get requested DTC from request */
    requestedDTC = ((uint32_t)requestData[1] << 16) |
                   ((uint32_t)requestData[2] << 8) |
                   (uint32_t)requestData[3];

    /* Check if record number is specified */
    if (requestLength >= 5)
    {
        requestedRecordNum = requestData[4];
    }

    /* Format response header */
    responseData[0] = requestData[0]; /* Echo subfunction */

    /* Search for matching snapshots */
    for (i = 0; i < g_snapshotCount; i++)
    {
        /* Check if DTC matches */
        if (g_dtcSnapshots[i].dtcCode == requestedDTC)
        {
            /* Check if record number matches or if all records requested */
            if (requestedRecordNum == 0xFF || g_dtcSnapshots[i].recordNumber == requestedRecordNum)
            {
                /* Check if we have enough space in response buffer */
                if ((responseIndex + 4 + g_dtcSnapshots[i].dataLength) > MAX_RESPONSE_LENGTH)
                {
                    return UDS_RESPONSE_RESPONSE_TOO_LONG;
                }

                /* Add DTC code (3 bytes) */
                responseData[responseIndex++] = (uint8_t)((g_dtcSnapshots[i].dtcCode >> 16) & 0xFF);
                responseData[responseIndex++] = (uint8_t)((g_dtcSnapshots[i].dtcCode >> 8) & 0xFF);
                responseData[responseIndex++] = (uint8_t)(g_dtcSnapshots[i].dtcCode & 0xFF);

                /* Add snapshot record number */
                responseData[responseIndex++] = g_dtcSnapshots[i].recordNumber;

                /* Add snapshot data */
                memcpy(&responseData[responseIndex], g_dtcSnapshots[i].data, g_dtcSnapshots[i].dataLength);
                responseIndex += g_dtcSnapshots[i].dataLength;

                dtcFound = true;

                /* If specific record requested and found, we can exit loop */
                if (requestedRecordNum != 0xFF)
                {
                    break;
                }
            }
        }
    }

    if (!dtcFound)
    {
        return UDS_RESPONSE_REQUEST_OUT_OF_RANGE;
    }

    *responseLength = responseIndex;

    return UDS_RESPONSE_POSITIVE;
}

/**
 * Handler for subfunction 0x06: Report DTC Extended Data Record by DTC Number
 *
 * Request format:  [SF(0x06)][DTC(3 bytes)][RecordNumber]
 * Response format: [SF(0x06)][DTC(3 bytes)][RecordNumber][ExtendedData]
 */
uint8_t UDS_ReportDTCExtendedDataRecordByDTCNumber(uint8_t *requestData, uint16_t requestLength, uint8_t *responseData, uint16_t *responseLength)
{
    uint32_t requestedDTC;
    uint8_t requestedRecordNum;
    uint16_t responseIndex = 1; /* Start after subfunction */
    uint16_t i;
    bool recordFound = false;

    /* Check request length */
    if (requestLength < 5)
    {
        return UDS_RESPONSE_INCORRECT_LENGTH_OR_FORMAT;
    }

    /* Get requested DTC from request */
    requestedDTC = ((uint32_t)requestData[1] << 16) |
                   ((uint32_t)requestData[2] << 8) |
                   (uint32_t)requestData[3];

    /* Get record number */
    requestedRecordNum = requestData[4];

    /* Format response header */
    responseData[0] = requestData[0]; /* Echo subfunction */

    /* Search for matching extended data record */
    for (i = 0; i < g_extendedDataCount; i++)
    {
        /* Check if DTC and record number match */
        if (g_dtcExtendedData[i].dtcCode == requestedDTC &&
            g_dtcExtendedData[i].recordNumber == requestedRecordNum)
        {

            /* Check if we have enough space in response buffer */
            if ((responseIndex + 4 + g_dtcExtendedData[i].dataLength) > MAX_RESPONSE_LENGTH)
            {
                return UDS_RESPONSE_RESPONSE_TOO_LONG;
            }

            /* Add DTC code (3 bytes) */
            responseData[responseIndex++] = (uint8_t)((g_dtcExtendedData[i].dtcCode >> 16) & 0xFF);
            responseData[responseIndex++] = (uint8_t)((g_dtcExtendedData[i].dtcCode >> 8) & 0xFF);
            responseData[responseIndex++] = (uint8_t)(g_dtcExtendedData[i].dtcCode & 0xFF);

            /* Add record number */
            responseData[responseIndex++] = g_dtcExtendedData[i].recordNumber;

            /* Add extended data */
            memcpy(&responseData[responseIndex], g_dtcExtendedData[i].data, g_dtcExtendedData[i].dataLength);
            responseIndex += g_dtcExtendedData[i].dataLength;

            recordFound = true;
            break;
        }
    }

    if (!recordFound)
    {
        return UDS_RESPONSE_REQUEST_OUT_OF_RANGE;
    }

    *responseLength = responseIndex;

    return UDS_RESPONSE_POSITIVE;
}

/**
 * Helper function to count DTCs matching a status mask
 */
uint16_t UDS_CountDTCByStatusMask(uint8_t statusMask)
{
    uint16_t count = 0;
    uint16_t i;

    for (i = 0; i < g_dtcCount; i++)
    {
        if ((g_dtcDatabase[i].statusByte & statusMask) != 0)
        {
            count++;
        }
    }

    return count;
}

/**
 * Helper function to add a DTC to the database
 */
void UDS_AddDTC(uint32_t dtcCode, uint8_t status)
{
    if (g_dtcCount < MAX_DTC_COUNT)
    {
        g_dtcDatabase[g_dtcCount].dtcCode = dtcCode;
        g_dtcDatabase[g_dtcCount].statusByte = status;
        g_dtcCount++;
    }
}

/**
 * Helper function to add a DTC snapshot record
 */
void UDS_AddDTCSnapshot(uint32_t dtcCode, uint8_t recordNumber, uint8_t *data, uint16_t dataLength)
{
    if (g_snapshotCount < MAX_SNAPSHOT_COUNT)
    {
        g_dtcSnapshots[g_snapshotCount].dtcCode = dtcCode;
        g_dtcSnapshots[g_snapshotCount].recordNumber = recordNumber;

        /* Allocate memory for snapshot data */
        g_dtcSnapshots[g_snapshotCount].data = (uint8_t *)malloc(dataLength);
        if (g_dtcSnapshots[g_snapshotCount].data != NULL)
        {
            memcpy(g_dtcSnapshots[g_snapshotCount].data, data, dataLength);
            g_dtcSnapshots[g_snapshotCount].dataLength = dataLength;
            g_snapshotCount++;
        }
    }
}

/**
 * Helper function to add a DTC extended data record
 */
void UDS_AddDTCExtendedData(uint32_t dtcCode, uint8_t recordNumber, uint8_t *data, uint16_t dataLength)
{
    if (g_extendedDataCount < MAX_EXTENDED_COUNT)
    {
        g_dtcExtendedData[g_extendedDataCount].dtcCode = dtcCode;
        g_dtcExtendedData[g_extendedDataCount].recordNumber = recordNumber;

        /* Allocate memory for extended data */
        g_dtcExtendedData[g_extendedDataCount].data = (uint8_t *)malloc(dataLength);
        if (g_dtcExtendedData[g_extendedDataCount].data != NULL)
        {
            memcpy(g_dtcExtendedData[g_extendedDataCount].data, data, dataLength);
            g_dtcExtendedData[g_extendedDataCount].dataLength = dataLength;
            g_extendedDataCount++;
        }
    }
}

/**
 * Initialize DTC database with sample data (for testing purposes)
 */
void UDS_InitializeDTCDatabase(void)
{
    /* Add sample DTCs */
    UDS_AddDTC(0x123456, DTC_STATUS_CONFIRMED | DTC_STATUS_TEST_FAILED);
    UDS_AddDTC(0x234567, DTC_STATUS_PENDING | DTC_STATUS_TEST_FAILED_THIS_OPERATION_CYCLE);
    UDS_AddDTC(0x345678, DTC_STATUS_WARNING_INDICATOR_REQUESTED | DTC_STATUS_CONFIRMED);

    /* Sample snapshot data */
    uint8_t snapshot1[] = {0x01, 0x23, 0x45, 0x67, 0x89}; /* Sample snapshot data */
    UDS_AddDTCSnapshot(0x123456, 0x01, snapshot1, sizeof(snapshot1));

    uint8_t snapshot2[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; /* Sample snapshot data */
    UDS_AddDTCSnapshot(0x123456, 0x02, snapshot2, sizeof(snapshot2));

    /* Sample extended data */
    uint8_t extData1[] = {0x11, 0x22, 0x33, 0x44, 0x55}; /* Sample extended data */
    UDS_AddDTCExtendedData(0x123456, 0x01, extData1, sizeof(extData1));

    uint8_t extData2[] = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33}; /* Sample extended data */
    UDS_AddDTCExtendedData(0x234567, 0x01, extData2, sizeof(extData2));
}

/**
 * Example usage function
 */
void UDS_Example(void)
{
    uint8_t request[10];
    uint8_t response[MAX_RESPONSE_LENGTH];
    uint16_t responseLength;
    uint8_t result;

    /* Initialize DTC database with sample data */
    UDS_InitializeDTCDatabase();

    /* Example 1: Report Number of DTCs by Status Mask */
    request[0] = SID_READ_DTC_INFORMATION;
    request[1] = SF_REPORT_NUMBER_OF_DTC_BY_STATUS_MASK;
    request[2] = DTC_STATUS_CONFIRMED; /* Request DTCs with 'confirmed' status */

    result = UDS_Service19_Handler(request, 3, response, &responseLength);
    /* Process result and response */

    /* Example 2: Report DTCs by Status Mask */
    request[0] = SID_READ_DTC_INFORMATION;
    request[1] = SF_REPORT_DTC_BY_STATUS_MASK;
    request[2] = DTC_STATUS_CONFIRMED | DTC_STATUS_PENDING; /* Request DTCs with 'confirmed' or 'pending' status */

    result = UDS_Service19_Handler(request, 3, response, &responseLength);
    /* Process result and response */

    /* Example 3: Report DTC Snapshot Record by DTC Number */
    request[0] = SID_READ_DTC_INFORMATION;
    request[1] = SF_REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER;
    request[2] = 0x12; /* DTC code high byte */
    request[3] = 0x34; /* DTC code middle byte */
    request[4] = 0x56; /* DTC code low byte */
    request[5] = 0x01; /* Record number */

    result = UDS_Service19_Handler(request, 6, response, &responseLength);
    /* Process result and response */

    /* Example 4: Report DTC Extended Data Record by DTC Number */
    request[0] = SID_READ_DTC_INFORMATION;
    request[1] = SF_REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER;
    request[2] = 0x12; /* DTC code high byte */
    request[3] = 0x34; /* DTC code middle byte */
    request[4] = 0x56; /* DTC code low byte */
    request[5] = 0x01; /* Record number */

    result = UDS_Service19_Handler(request, 6, response, &responseLength);
    /* Process result and response */
}

/**
 * Main function (example of how to use this implementation)
 */
// int main(void)
// {
//     /* Run the UDS Service 19 examples */
//     UDS_Example();

//     return 0;
// }