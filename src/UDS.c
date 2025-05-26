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
    if (did == 1008)
        return true;
    return false;
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
void SF_UDS_ReportNumberOfDTCByStatusMask(const can_message_t *requestMsg)
{
    
}

/**
 * Handler for subfunction 0x02: Report DTC by Status Mask
 *
 * Request format:  [SF(0x02)][StatusMask]
 * Response format: [SF(0x02)][StatusMask][DTCFormatIdentifier][DTC1(3 bytes)][Status1][DTC2(3 bytes)][Status2]...
*/
void UDS_ReportDTCByStatusMask(const can_message_t *requestMsg)
{

}

/**
 * Handler for subfunction 0x06: Report DTC Extended Data Record by DTC Number
 *
 * Request format:  [SF(0x06)][DTC(3 bytes)][RecordNumber]
 * Response format: [SF(0x06)][DTC(3 bytes)][RecordNumber][ExtendedData]
*/
void SF_UDS_ReportDTCExtendedDataRecordByDTCNumber(const can_message_t *requestMsg)
{

}


void UDS_ReadDTCInformation(const can_message_t *requestMsg)
{
    // Handler for service 0x19 (Diagnostic Session Control)
    // This function is currently not implemented
}

void UDS_ReadDataByIdentifier(const can_message_t *requestMsg)
{
//    if (requestMsg->length < 3 || (requestMsg->length - 1) % 2 != 0)
//    {
//        SendNRC(requestMsg->data[0], 0x13);
//        return;
//    }
//
//    if (requestMsg->length > 8)
//    {
//        SendNRC(requestMsg->data[0], 0x13);
//        return;
//    }
//
//    uint16_t did = requestMsg->data[1] << 8 | requestMsg->data[2];
//
//    volatile bool hasValidDID = false;
//    uint8_t num_DID_support = sizeof(support_DID_table) / sizeof(did_entry_t);
//    for (uint8_t i = 0; i < num_DID_support; ++i)
//    {
//
//        if (support_DID_table[i].did == did && checkCondition(support_DID_table[i].did))
//        {
//            hasValidDID = true;
//            continue;
//        }
//        else
//        {
//            SendNRC(requestMsg->data[0], 0x22);
//            return;
//        }
//    }
//
//    if (hasValidDID == false)
//    {
//        SendNRC(requestMsg->data[0], 0x31);
//        return;
//    }
//
//    uint8_t databuf[10] = {0};
//    uint8_t data_len = readDID(did, databuf);
//    can_buff_config_t buffCfg = {
//        .enableFD = false,
//        .enableBRS = false,
//        .fdPadding = 0U,
//        .idType = CAN_MSG_ID_STD,
//        .isRemote = false};
//
//    /* Configure TX buffer with index TX_MAILBOX*/
//    CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);
//
//    /* Prepare message to be sent */
//    can_message_t message = {
//        .cs = 0U,
//        .id = TX_MSG_ID,
//        .data[0] = requestMsg->data[0] + 0x40,
//        .data[1] = requestMsg->data[1],
//        .data[2] = requestMsg->data[2],
//        .length = (6U)};
//    for (uint8_t i = 1; i <= data_len; ++i)
//    {
//        message.data[i + 3] = databuf[i];
//    }
//    /* Send the information via CAN */
//    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
//    // SendPositiveResponse();
}

void UDS_WriteDataByIdentifier(const can_message_t *requestMsg)
{
//    if (requestMsg < 4 || requestMsg->length > 8)
//    {
//        SendNRC(requestMsg->data[0], 0x13);
//        return;
//    }
//
//    uint16_t did = requestMsg->data[1] << 8 | requestMsg->data[2];
//
//    did_entry_t *did_entry = findByDID(did);
//    if (did_entry == NULL)
//    {
//        SendNRC(requestMsg->data[0], 0x31);
//        return;
//    }
//
//    uint8_t len_by_request = requestMsg->length - 3;
//    if (len_by_request != did_entry->data_length)
//    {
//        SendNRC(requestMsg->data[0], 0x13);
//        return;
//    }
//
//    if (checkCondition(did_entry->did) == false)
//    {
//        SendNRC(requestMsg->data[0], 0x22);
//        return;
//    }
//
//    /*Send positive response*/
//    can_buff_config_t buffCfg = {
//        .enableFD = false,
//        .enableBRS = false,
//        .fdPadding = 0U,
//        .idType = CAN_MSG_ID_STD,
//        .isRemote = false};
//
//    /* Configure TX buffer with index TX_MAILBOX*/
//    CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);
//
//    /* Prepare message to be sent */
//    can_message_t message = {
//        .cs = 0U,
//        .id = TX_MSG_ID,
//        .data[0] = requestMsg->data[0] + 0x40,
//        .data[1] = requestMsg->data[1],
//        .data[2] = requestMsg->data[2],
//        .length = (3U)};
//    /* Send the information via CAN */
//    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
}
