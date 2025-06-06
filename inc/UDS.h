/*
 * UDS.h
 *
 *  Created on: May 26, 2025
 *      Author: anhan
 */

#ifndef UDS_H_
#define UDS_H_

#include "sdk_project_config.h"
#include <interrupt_manager.h>
#include "DID.h"
#include "DTC.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* UDS Service IDs */
#define SID_READ_DTC_INFORMATION 0x19
#define SID_READ_DATA_BY_IDENTIFIER 0x22
#define SID_WRITE_DATA_BY_IDENTIFIER 0x2E

/* Service 19 - Read DTC Information Sub-functions */
#define SF_REPORT_NUMBER_OF_DTC_BY_STATUS_MASK 0x01
#define SF_REPORT_DTC_BY_STATUS_MASK 0x02
#define SF_REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER 0x04
#define SF_REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER 0x06

/* Negative Response Codes */
#define UDS_RESPONSE_SERVICE_NOT_SUPPORTED 0x11
#define UDS_RESPONSE_SUBFUNCTION_NOT_SUPPORTED 0x12
#define UDS_RESPONSE_INCORRECT_LENGTH_OR_FORMAT 0x13
#define UDS_RESPONSE_CONDITIONS_NOT_CORRECT 0x22
#define UDS_RESPONSE_REQUEST_OUT_OF_RANGE 0x31

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

#define MASTER
/* #define SLAVE */

/* Definition of the TX and RX message buffers depending on the bus role */
#if defined(MASTER)
#define TX_MAILBOX (0UL)
#define TX_MSG_ID (0x768)
#define RX_MAILBOX (01UL)
#define RX_MSG_ID (0x769)
#elif defined(SLAVE)
#define TX_MAILBOX (0UL)
#define TX_MSG_ID (0x769)
#define RX_MAILBOX (1UL)
#define RX_MSG_ID (0x768)
#endif

/******************************************************************************
 * Function prototypes
 ******************************************************************************/

/*Helper Functions*/
void SendNRC(uint8_t SID, uint8_t NRC);
bool checkCondition(uint16_t did);

/*Service Handler Functions*/

//void SF_UDS_ReportNumberOfDTCByStatusMask(const can_message_t *requestMsg);          // Subfunction 0x01
//void SF_UDS_ReportDTCByStatusMask(const can_message_t *requestMsg);                  // Subfunction 0x02
void SF_UDS_ReportDTCSnapshotByDTCNumber(const can_message_t *requestMsg); // Subfunction 0x06

void UDS_ReadDTCInformation(const can_message_t *requestMsg);    // Service 19 Handler
void UDS_WriteDataByIdentifier(const can_message_t *requestMsg); // Service 22 Handler
void UDS_ReadDataByIdentifier(const can_message_t *requestMsg);  // Service 2E Handler

#endif /* UDS_H_ */`