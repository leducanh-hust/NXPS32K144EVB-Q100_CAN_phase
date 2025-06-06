#include "UDS.h"

/*******************************************************************************
 * Helper Functions
 *******************************************************************************/
void SendNRC(uint8_t SID, uint8_t NRC)
{
    can_buff_config_t buffCfg = {
        .enableFD = false,
        .enableBRS = false,
        .fdPadding = 0U,
        .idType = CAN_MSG_ID_STD,
        .isRemote = false};

    /* Configure TX buffer with index TX_MAILBOX*/
    CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);

    can_message_t message = {
        .cs = 0U,
        .id = TX_MSG_ID,
        .data[0] = 0x7F,
        .data[1] = SID,
        .data[2] = NRC,
        .length = 3U};

    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
}

bool checkCondition(uint16_t did)
{
    return true;
}

/*******************************************************************************
 *Service Handler Functions
 ********************************************************************************/

/*Sub-function helper functions*/

/**
 * Handler for subfunction 0x01: Report Number of DTC by Status Mask
 *
 * Request format:  [SF(0x01)][StatusMask]
 * Response format: [SF(0x01)][StatusMask][DTCFormatIdentifier][DTCCount(2 bytes)]
 */
// void SF_UDS_ReportNumberOfDTCByStatusMask(const can_message_t *requestMsg)
// {
// }

// /**
//  * Handler for subfunction 0x02: Report DTC by Status Mask
//  *
//  * Request format:  [SF(0x02)][StatusMask]
//  * Response format: [SF(0x02)][StatusMask][DTCFormatIdentifier][DTC1(3 bytes)][Status1][DTC2(3 bytes)][Status2]...
//  */
// void SF_UDS_ReportDTCByStatusMask(const can_message_t *requestMsg)
// {
// }

/**
 * Handler for subfunction 0x06: Report DTC Extended Data Record by DTC Number
 *
 * Request format:  [SF(0x04)][DTC(3 bytes)][RecordNumber] 19 04 A3 D8 00 FF
 * Response format: [SF(0x06)][DTC(3 bytes)][RecordNumber][ExtendedData] 59 04 A3 D8 00 AE  //padding 00 00
 */
void SF_UDS_ReportDTCSnapshotByDTCNumber(const can_message_t *requestMsg)
{
    if (requestMsg->length < 6 || requestMsg->length > 8)
    {
        SendNRC(requestMsg->data[0], 0x13);
        return;
    }

    uint32_t dtcCode = (requestMsg->data[2] << 16) | (requestMsg->data[3] << 8) | requestMsg->data[4];

    for (uint8_t i = 0; i < 2; ++i)
    {
        uint8_t storedDTC[4];
        NVM_Read(DTC_Code_Offset(i), storedDTC, 4);
        uint32_t storedDTCCode = (storedDTC[1] << 16) | (storedDTC[2] << 8) | storedDTC[3];

        //Check if the requested DTC matches the stored DTC 
        if (dtcCode == storedDTCCode)
        {
            can_buff_config_t buffCfg = {
                .enableFD = false,
                .enableBRS = false,
                .fdPadding = 0U,
                .idType = CAN_MSG_ID_STD,
                .isRemote = false};

            /* Configure TX buffer with index TX_MAILBOX*/
            CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);

            /* Prepare message to be sent */
            can_message_t message = {
                .cs = 0U,
                .id = TX_MSG_ID,
                .data[0] = requestMsg->data[0] + 0x40,              // Positive response
                .data[1] = requestMsg->data[1],                     // Sub-function
                .data[2] = requestMsg->data[2],                     // DTC MSB
                .data[3] = requestMsg->data[3],                     // DTC Middle Byte
                .data[4] = requestMsg->data[4],                     // DTC LSB 
                .length = 6U};                                      // Length of the response
            
            // Read the snapshot status byte from NVM
            uint8_t snapshotStatus[1];
            NVM_Read(DTC_Snapshot_Offset(i), snapshotStatus, 1);
            message.data[5] = snapshotStatus[0]; // DTC Status Byte
            
            /* Send the message */
            CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
            return; // Exit after sending the response for the found DTC
        }
    }
}

void UDS_ReadDTCInformation(const can_message_t *requestMsg)
{
    // Handler for service 0x19 (Diagnostic Session Control)
    uint8_t subFunction = requestMsg->data[1];
    // Currently only sub-function 0x04 is supported
    if (subFunction != SF_REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER)
    {
        SendNRC(requestMsg->data[0], UDS_RESPONSE_SUBFUNCTION_NOT_SUPPORTED);
        return;
    }
    // Call the 0x04 sub-function handler
    SF_UDS_ReportDTCSnapshotByDTCNumber(requestMsg);
}

// Service 22 - Write Data by Identifier Handler
void UDS_ReadDataByIdentifier(const can_message_t *requestMsg)
{
    if (requestMsg->length < 3 || (requestMsg->length - 1) % 2 != 0)
    {
        SendNRC(requestMsg->data[0], 0x13);
        return;
    }

    if (requestMsg->length > 8)
    {
        SendNRC(requestMsg->data[0], 0x13);
        return;
    }

    uint16_t did = requestMsg->data[1] << 8 | requestMsg->data[2];

    volatile bool hasValidDID = false;
    did_entry_t *foundDID = NULL;
    uint8_t num_DID_support = sizeof(support_DID_table) / sizeof(did_entry_t);
    for (uint8_t i = 0; i < num_DID_support; ++i)
    {
        if (support_DID_table[i].did == did)
        {
            if (checkCondition(support_DID_table[i].did) == false)
            {
                SendNRC(requestMsg->data[0], 0x22);
                return;
            }
            hasValidDID = true;
            foundDID = &support_DID_table[i];
            continue;
        }
    }

    if (hasValidDID == false)
    {
        SendNRC(requestMsg->data[0], 0x31);
        return;
    }

    uint8_t readBuf[8] = {0};
    uint8_t data_len = foundDID->readCallback(readBuf);

    can_buff_config_t buffCfg = {
        .enableFD = false,
        .enableBRS = false,
        .fdPadding = 0U,
        .idType = CAN_MSG_ID_STD,
        .isRemote = false};

    /* Configure TX buffer with index TX_MAILBOX*/
    CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);

    /* Prepare message to be sent */
    can_message_t message = {
        .cs = 0U,
        .id = TX_MSG_ID,
        .data[0] = requestMsg->data[0] + 0x40,
        .data[1] = requestMsg->data[1],
        .data[2] = requestMsg->data[2],
        .length = (3U + data_len)};
    for (uint8_t i = 0; i < data_len; ++i)
    {
        message.data[i + 3] = readBuf[i];
    }
    /* Send the information via CAN */
    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
    // SendPositiveResponse();
}

// Service 2E - Write Data by Identifier Handler
void UDS_WriteDataByIdentifier(const can_message_t *requestMsg)
{
    // Check length of request message
    if (requestMsg->length < 4 || requestMsg->length > 8)
    {
        SendNRC(requestMsg->data[0], 0x13);
        return;
    }

    // Check if the request did is supported
    uint16_t did = requestMsg->data[1] << 8 | requestMsg->data[2];

    did_entry_t *findDID = NULL;
    for (uint8_t i = 0; i < support_DID_count; ++i)
    {
        if (support_DID_table[i].did == did)
        {
            findDID = &support_DID_table[i];
            break;
        }
    }

    if (findDID == NULL)
    {
        SendNRC(requestMsg->data[0], 0x31);
        return;
    }

    uint8_t len_by_request = requestMsg->length - 3;
    if (len_by_request != findDID->data_length)
    {
        SendNRC(requestMsg->data[0], 0x13);
        return;
    }

    if (checkCondition(findDID->did) == false)
    {
        SendNRC(requestMsg->data[0], 0x22);
        return;
    }

    /*Write data and Send positive response*/
    uint8_t writeData[8] = {0};
    for (uint8_t i = 0; i < len_by_request; ++i)
    {
        writeData[i] = requestMsg->data[i + 3];
    }
    for (uint8_t i = len_by_request; i < 8; ++i)
    {
        writeData[i] = 0xFF; // Fill remaining bytes with 0xFF for padding
    }
    findDID->writeCallback(writeData, len_by_request);

    can_buff_config_t buffCfg = {
        .enableFD = false,
        .enableBRS = false,
        .fdPadding = 0U,
        .idType = CAN_MSG_ID_STD,
        .isRemote = false};

    /* Configure TX buffer with index TX_MAILBOX*/
    CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);

    /* Prepare message to be sent */
    can_message_t message = {
        .cs = 0U,
        .id = TX_MSG_ID,
        .data[0] = requestMsg->data[0] + 0x40,
        .data[1] = requestMsg->data[1],
        .data[2] = requestMsg->data[2],
        .length = (3U)};
    /* Send the information via CAN */
    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
}
