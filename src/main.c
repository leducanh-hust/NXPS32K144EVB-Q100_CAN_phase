#include "sdk_project_config.h"
#include <interrupt_manager.h>
#include <stdint.h>
#include "DID.h"
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include "DTC.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/* This example is setup to work by default with EVB. To use it with other boards
   please comment the following line
*/

#define EVB

#ifdef EVB
#define LED_PORT PORTD
#define GPIO_PORT PTD
#define PCC_INDEX PCC_PORTD_INDEX
#define LED0 15U
#define LED1 16U
#define BTN_GPIO PTC
#define BTN1_PIN 13U
#define BTN2_PIN 12U
#define BTN_PORT PORTC
#define BTN_PORT_IRQn PORTC_IRQn
#define LED_BLUE 0U   /* pin PTD0 - RGB LED on DEV-KIT */
#define LED_RED 15U   /* pin PTD15 - RGB LED on DEV-KIT */
#define LED_GREEN 16U /* pin PTE16 - RGB LED on DEV-KIT */

#else
#define LED_PORT PORTC
#define GPIO_PORT PTC
#define PCC_INDEX PCC_PORTC_INDEX
#define LED0 0U
#define LED1 1U
#define BTN_GPIO PTC
#define BTN1_PIN 13U
#define BTN2_PIN 12U
#define BTN_PORT PORTC
#define BTN_PORT_IRQn PORTC_IRQn
#define LED_BLUE 0U  /* pin PTC0 - LED0 on Mother board */
#define LED_RED 1U   /* pin PTC1 - LED3 LED on Mother board */
#define LED_GREEN 2U /* pin PTC2 - LED2 LED on Mother board */
#endif

/* Use this define to specify if the application runs as master or slave */
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

#define FTM_CHANNEL 0UL
#define PERIOD_BY_NS 100000000UL

/******************************************************************************
 UDS Defines begin
 ******************************************************************************/
#define SID_22 0x22
#define SID_2E 0x2E
#define SID_19 0x19

#define UDS_POSITIVE_RESPONSE 0x00
#define NRC_RESPONSE_SUBFUNCTION_NOT_SUPPORTED 0x12 // Exclusively for Service 0x19
#define NRC_RESPONSE_INCORRECT_LENGTH_OR_FORMAT 0x13
#define NRC_RESPONSE_CONDITIONS_NOT_CORRECT 0x22
#define NRC_RESPONSE_REQUEST_OUT_OF_RANGE 0x31

/* Service 19 - Read DTC Information Sub-functions */
#define SF_REPORT_NUMBER_OF_DTC_BY_STATUS_MASK 0x01
#define SF_REPORT_DTC_BY_STATUS_MASK 0x02
#define SF_REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER 0x04
#define SF_REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER 0x06

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

/******************************************************************************
 UDS Defines end
 ******************************************************************************/


// extern const uint8_t __nvm_did_start__[]; // Start of NVM section
// extern const uint8_t __nvm_did_end__[];   // Optional: end of section

// // Define the base address as a constant
// #define DID_NVM_BASE_ADDR ((uint32_t)__nvm_did_start__)
// #define DID_NVM_END_ADDR ((uint32_t)__nvm_did_end__)
// #define DID_NVM_SIZE (DID_NVM_END_ADDR - DID_NVM_BASE_ADDR)

typedef enum
{
    LED0_CHANGE_REQUESTED = 0x00U,
    LED1_CHANGE_REQUESTED = 0x01U
} can_commands_list;

uint8_t ledRequested = (uint8_t)LED0_CHANGE_REQUESTED;

extern did_entry_t support_DID_table[];

/******************************************************************************
 * Function prototypes
 ******************************************************************************/

void BoardInit(void);
void GPIOInit(void);

/*******************************************************************************
 * Helper Functions
 *******************************************************************************/
void SendNRC(uint8_t SID, uint8_t NRC);
bool checkCondition(uint16_t did);

/******************************************************************************* 
*cService Handler Functions
********************************************************************************/
void SID_22_Handler(const can_message_t *rcvMsg);

void SID_2E_Handler(const can_message_t *requestMsg);
/******************************************************************************
 * Functions
 ******************************************************************************/
/*Timer callback function*/
// void timingFtmInst0_callback(void *userData)
//{
//     (void)userData;
//     PINS_DRV_TogglePins(GPIO_PORT, (1 << LED_GREEN));
// }

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

bool isBlank(uint32_t address)
{
    uint8_t *ptr = (uint8_t *)address;
    for (uint32_t i = 0; i < 8; ++i)
    {
        if (*ptr != 0xFF)
            return false;
        ptr++;
    }
    return true;
}

void SID_22_Handler(const can_message_t *requestMsg)
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
    uint8_t num_DID_support = sizeof(support_DID_table) / sizeof(did_entry_t);
    for (uint8_t i = 0; i < num_DID_support; ++i)
    {

        if (support_DID_table[i].did == did && checkCondition(support_DID_table[i].did))
        {
            hasValidDID = true;
            continue;
        }
        else
        {
            SendNRC(requestMsg->data[0], 0x22);
            return;
        }
    }

    if (hasValidDID == false)
    {
        SendNRC(requestMsg->data[0], 0x31);
        return;
    }

    uint8_t databuf[10] = {0};
    uint8_t data_len = readDID(did, databuf);
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
        .length = (6U)};
    for (uint8_t i = 1; i <= data_len; ++i)
    {
        message.data[i + 3] = databuf[i];
    }
    /* Send the information via CAN */
    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
    // SendPositiveResponse();
}

void SID_2E_Handler(const can_message_t *requestMsg)
{
    if (requestMsg < 4 || requestMsg->length > 8)
    {
        SendNRC(requestMsg->data[0], 0x13);
        return;
    }

    uint16_t did = requestMsg->data[1] << 8 | requestMsg->data[2];

    did_entry_t *did_entry = findByDID(did);
    if (did_entry == NULL)
    {
        SendNRC(requestMsg->data[0], 0x31);
        return;
    }

    uint8_t len_by_request = requestMsg->length - 3;
    if (len_by_request != did_entry->data_length)
    {
        SendNRC(requestMsg->data[0], 0x13);
        return;
    }

    if (checkCondition(did_entry->did) == false)
    {
        SendNRC(requestMsg->data[0], 0x22);
        return;
    }

    /*Send positive response*/
    can_buff_config_t buffCfg = {
        .enableFD = false,
        .enableBRS = false,
        .fdPadding = 0U,
        .idType = CAN_MSG_ID_STD,
        .isRemote = false
    };

    /* Configure TX buffer with index TX_MAILBOX*/
    CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);

    /* Prepare message to be sent */
    can_message_t message = {
        .cs = 0U,
        .id = TX_MSG_ID,
        .data[0] = requestMsg->data[0] + 0x40,
        .data[1] = requestMsg->data[1],
        .data[2] = requestMsg->data[2],
        .length = (3U)
    };
    /* Send the information via CAN */
    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
}

/*
 * @brief : Initialize clocks, pins and power modes
 */
void BoardInit(void)
{

    /* Initialize and configure clocks
     *  -   Setup system clocks, dividers
     *  -   Configure FlexCAN clock, GPIO
     *  -   see clock manager component for more details
     */
    CLOCK_DRV_Init(&clockMan1_InitConfig0);

    /* Initialize pins
     *  -   Init FlexCAN and GPIO pins
     *  -   See PinSettings component for more info
     */
    PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
}

/*
 * @brief Function which configures the LEDs and Buttons
 */
void GPIOInit(void)
{
    /* Output direction for LEDs */
    PINS_DRV_SetPinsDirection(GPIO_PORT, (1 << LED1) | (1 << LED0));

    /* Set Output value LEDs */
    PINS_DRV_ClearPins(GPIO_PORT, (1 << LED1) | (1 << LED0));

    /* Setup button pin */
    PINS_DRV_SetPinsDirection(BTN_GPIO, ~((1 << BTN1_PIN) | (1 << BTN2_PIN)));

    //    /* Setup button pins interrupt */
    //    PINS_DRV_SetPinIntSel(BTN_PORT, BTN1_PIN, PORT_INT_RISING_EDGE);
    //    PINS_DRV_SetPinIntSel(BTN_PORT, BTN2_PIN, PORT_INT_RISING_EDGE);
}

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - __start (startup asm routine)
 * - __init_hardware()
 * - main()
 *     - Common_Init()
 *     - Peripherals_Init()
*/

int main(void)
{

    /* Do the initializations required for this application */
    BoardInit();
    GPIOInit();

    CAN_Init(&can_pal1_instance, &can_pal1_Config0);

    /* Set information about the data to be sent
     *  - Standard message ID
     *  - Bit rate switch enabled to use a different bitrate for the data segment
     *  - Flexible data rate enabled
     *  - Use zeros for FD padding
     */
    can_buff_config_t buffCfg = {
        .enableFD = false,
        .enableBRS = false,
        .fdPadding = 0U,
        .idType = CAN_MSG_ID_STD,
        .isRemote = false};

    /* Configure RX buffer with index RX_MAILBOX */
    CAN_ConfigRxBuff(&can_pal1_instance, RX_MAILBOX, &buffCfg, RX_MSG_ID);

    while (1)
    {
        /* Define receive buffer */
        can_message_t recvMsg;

        /* Start receiving data in RX_MAILBOX. */
        CAN_Receive(&can_pal1_instance, RX_MAILBOX, &recvMsg);

        /* Wait until the previous FlexCAN receive is completed */
        while (CAN_GetTransferStatus(&can_pal1_instance, RX_MAILBOX) == STATUS_BUSY)
            ;
        switch (recvMsg.data[0])
        {
        case SID_22:
            /* code */
            SID_22_Handler(&recvMsg);
            break;
        case SID_2E:
            /* code */
            SID_2E_Handler(&recvMsg);
            break;
        case SID_19:
            /* code */
            break;
        default:
            break;
        }
    }
    uint32_t DFlashAddr;
    flash_ssd_config_t flashSSDConfig;
    // // uint64_t ftmResolution;
    // status_t status;
    // /* Initialize clock module */
    // status = CLOCK_DRV_Init(&clockMan1_InitConfig0);
    // DEV_ASSERT(status == STATUS_SUCCESS);
    // /* Initialize LEDs and Button configuration */
    // status = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    // DEV_ASSERT(status == STATUS_SUCCESS);

    // status = TIMING_Init(&timingFtmInst0, &timingFtm_InitConfig0);
    // DEV_ASSERT(status == STATUS_SUCCESS);

    // status = TIMING_GetResolution(&timingFtmInst0, TIMER_RESOLUTION_TYPE_NANOSECOND, &ftmResolution);
    // DEV_ASSERT(status == STATUS_SUCCESS);

    // status = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    // DEV_ASSERT(status == STATUS_SUCCESS);

    // status = FLASH_DRV_Init(&Flash_InitConfig0, &flashSSDConfig);
    // DEV_ASSERT(status == STATUS_SUCCESS);

    // uint8_t writeData[3] = {0x01, 0x04, 0x7A};
    // // FLASH_DRV_Program(&flashSSDConfig, 0x10010000, 4, writeData);
    // volatile uint8_t readData[8] = {0};
    // uint32_t i;

    // // === Step 4: Enable EEPROM emulation via FlexRAM ===
    // status = FLASH_DRV_SetFlexRamFunction(&flashSSDConfig, EEE_ENABLE, 0x00u, NULL);
    // DEV_ASSERT(status == STATUS_SUCCESS);

    // // === Step 5: Write EEPROM ===
    // DFlashAddr = flashSSDConfig.DFlashBase;
    // status = FLASH_DRV_EEEWrite(&flashSSDConfig, DFlashAddr, 8, writeData);
    // DEV_ASSERT(status == STATUS_SUCCESS);

    // // === Step 6: Read back ===
    // for (i = 0; i < 8; i++)
    // {
    //     readData[i] = *((volatile uint8_t *)(DFlashAddr + i));
    // }
}

uint16_t UDS_CountDTCByStatusMask(uint8_t statusMask)
{
    uint16_t count = 0;
    uint16_t i;
    for (i = 0; i < NUM_DTC_ENTRIES; i++)
    {
        if ((dtcTable[i].statusByte & statusMask) != 0)
        {
            count++;
        }
    }

    return count;
}

void SID_19_Handler(const can_message_t *requestMsg)
{
    uint8_t subFunction = requestMsg->data[1];
    switch (subFunction)
    {
    case SF_REPORT_NUMBER_OF_DTC_BY_STATUS_MASK:
        UDS_ReportNumberOfDTCByStatusMask(requestMsg);
        break;
        
    case SF_REPORT_DTC_BY_STATUS_MASK:
        UDS_ReportDTCByStatusMask(requestMsg);
        break;

    case SF_REPORT_DTC_SNAPSHOT_RECORD_BY_DTC_NUMBER:
        // UDS_ReportDTCByStatusMask(requestMsg);
        break;

    case SF_REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER:
        // UDS_ReportDTCByStatusMask(requestMsg);
        break;

    default:
        /* Send negative response code */
        SendNRC(requestMsg->data[0], NRC_RESPONSE_SUBFUNCTION_NOT_SUPPORTED);
        break;
    }
}

/**
 * Handler for subfunction 0x01: Report Number of DTC by Status Mask
 *
 * Request format:  [SF(0x01)][StatusMask]
 * Response format: [SF(0x01)][StatusMask][DTCFormatIdentifier][DTCCount(2 bytes)]
 */
void UDS_ReportNumberOfDTCByStatusMask(const can_message_t *requestMsg)
{
    uint8_t dtcStatusMask;
    uint16_t matchDTCCount;
    if(requestMsg->length < 2)
    {
        SendNRC(requestMsg->data[0], NRC_RESPONSE_INCORRECT_LENGTH_OR_FORMAT);
        return;
    }

    dtcStatusMask = requestMsg->data[2];
    matchDTCCount = UDS_CountDTCByStatusMask(dtcStatusMask);
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
        .data[2] = dtcStatusMask,
        .data[3] = DTC_FORMAT_ISO14229_1,
        .data[4] = (uint8_t)((matchDTCCount >> 8) & 0xFF),
        .data[5] = (uint8_t)(matchDTCCount & 0xFF),
        .length = 5U
    };
    /* Send the information via CAN */
    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
}

/**
 * Handler for subfunction 0x02: Report DTC by Status Mask
 *
 * Request format:  [SF(0x02)][StatusMask]
 * Response format: [SF(0x02)][StatusMask][DTCFormatIdentifier][DTC1(3 bytes)][Status1][DTC2(3 bytes)][Status2]...
 */
void UDS_ReportDTCByStatusMask(const can_message_t *requestMsg)
{
    uint8_t dtcStatusMask;

    if(requestMsg->length < 2)
    {
        SendNRC(requestMsg->data[0], NRC_RESPONSE_INCORRECT_LENGTH_OR_FORMAT);
        return;
    }

    dtcStatusMask = requestMsg->data[2];

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
        .data[2] = dtcStatusMask,
        .data[3] = DTC_FORMAT_ISO14229_1,
        .length = 3U
    };
    for(uint8_t i = 0; i < NUM_DTC_ENTRIES; i++)
    {
        if((dtcTable[i].statusByte & dtcStatusMask) != 0)
        {
            message.data[message.length++] = (uint8_t)((dtcTable[i].dtcCode >> 16) & 0xFF);
            message.data[message.length++] = (uint8_t)((dtcTable[i].dtcCode >> 8) & 0xFF);
            message.data[message.length++] = (uint8_t)(dtcTable[i].dtcCode & 0xFF);
            message.data[message.length++] = dtcTable[i].statusByte;
        }
    }
    /* Send the information via CAN */
    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
}

/**
 * Handler for subfunction 0x04: Report DTC Snapshot Record by DTC Number
 *
 * Request format:  [SF(0x04)][DTC(3 bytes)][RecordNumber (optional)]
 * Response format: [SF(0x04)][DTC(3 bytes)][RecordNumber][SnapshotData]...
 */
uint8_t UDS_ReportDTCSnapshotRecordByDTCNumber(const can_message_t *requestMsg)
{
    uint8_t dtc[3];
    uint8_t recordNumber;
    uint8_t responseData[64];
    uint16_t responseLength = 0;

    if (requestMsg->length < 4)
    {
        SendNRC(requestMsg->data[0], NRC_RESPONSE_INCORRECT_LENGTH_OR_FORMAT);
        return NRC_RESPONSE_INCORRECT_LENGTH_OR_FORMAT;
    }

    dtc[0] = requestMsg->data[1];
    dtc[1] = requestMsg->data[2];
    dtc[2] = requestMsg->data[3];

    if (requestMsg->length > 4)
    {
        recordNumber = requestMsg->data[4];
    }
    else
    {
        recordNumber = 0x00; // Default record number
    }

}