#include "sdk_project_config.h"
#include <interrupt_manager.h>
#include <stdint.h>
//#include "UDS.c"
#include "NVM.h"
#include <stdbool.h>
#include <string.h>

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

typedef enum
{
    LED0_CHANGE_REQUESTED = 0x00U,
    LED1_CHANGE_REQUESTED = 0x01U
} can_commands_list;

uint8_t ledRequested = (uint8_t)LED0_CHANGE_REQUESTED;
flash_ssd_config_t flashSSDConfig;

/******************************************************************************
 * Function prototypes
 ******************************************************************************/

void BoardInit(void);
void GPIOInit(void);

/******************************************************************************
 * Functions
 ******************************************************************************/
/*Timer callback function*/
// void timingFtmInst0_callback(void *userData)
//{
//     (void)userData;
//     PINS_DRV_TogglePins(GPIO_PORT, (1 << LED_GREEN));
// }

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
    // CAN_Init(&can_pal1_instance, &can_pal1_Config0);

    // /* Set information about the data to be sent
    //  *  - Standard message ID
    //  *  - Bit rate switch enabled to use a different bitrate for the data segment
    //  *  - Flexible data rate enabled
    //  *  - Use zeros for FD padding
    //  */
    // can_buff_config_t buffCfg = {
    //     .enableFD = false,
    //     .enableBRS = false,
    //     .fdPadding = 0U,
    //     .idType = CAN_MSG_ID_STD,
    //     .isRemote = false};

    // /* Configure RX buffer with index RX_MAILBOX */
    // CAN_ConfigRxBuff(&can_pal1_instance, RX_MAILBOX, &buffCfg, RX_MSG_ID);

    // while (1)
    // {
    //     /* Define receive buffer */
    //     can_message_t recvMsg;

    //     /* Start receiving data in RX_MAILBOX. */
    //     CAN_Receive(&can_pal1_instance, RX_MAILBOX, &recvMsg);

    //     /* Wait until the previous FlexCAN receive is completed */
    //     while (CAN_GetTransferStatus(&can_pal1_instance, RX_MAILBOX) == STATUS_BUSY)
    //         ;
    //     switch (recvMsg.data[0])
    //     {
    //     case SID_22:
    //         /* code */
    //         SID_22_Handler(&recvMsg);
    //         break;
    //     case SID_2E:
    //         /* code */
    //         SID_2E_Handler(&recvMsg);
    //         break;
    //     case SID_19:
    //         /* code */
    //         break;
    //     default:
    //         break;
    //     }
    // }

    status_t status;
    /* Initialize the Flash driver */
    status = FLASH_DRV_Init(&Flash_InitConfig0, &flashSSDConfig);
    DEV_ASSERT(status == STATUS_SUCCESS);

    uint8_t writeData[] = {0x01, 0x04, 0x7A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Check if the device was already partitioned
    if ((FEATURE_FLS_HAS_FLEX_NVM == 1u) && (FEATURE_FLS_HAS_FLEX_RAM == 1u))
        // === Step 3: Partition if needed ===
        if (flashSSDConfig.EEESize == 0u)
        {
            status = FLASH_DRV_DEFlashPartition(&flashSSDConfig, 0x02u, 0x08u, 0x0u, false, true);
            DEV_ASSERT(status == STATUS_SUCCESS);

            // Re-initialize after partitioning
            status = FLASH_DRV_Init(&Flash_InitConfig0, &flashSSDConfig);
            DEV_ASSERT(status == STATUS_SUCCESS);
        }
    //Enable EEPROM emulation via FlexRAM
    status = FLASH_DRV_SetFlexRamFunction(&flashSSDConfig, EEE_ENABLE, 0x00u, NULL);
    DEV_ASSERT(status == STATUS_SUCCESS);

    NVM_Write(0x00, writeData, 8);

    //Verify Data After Write
    uint8_t readData[8] = {0};
    NVM_Read(0x00, readData, 4);

    // // uint64_t ftmResolution;

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
}



/**
 * Handler for subfunction 0x01: Report Number of DTC by Status Mask
 *
 * Request format:  [SF(0x01)][StatusMask]
 * Response format: [SF(0x01)][StatusMask][DTCFormatIdentifier][DTCCount(2 bytes)]
 */
//void UDS_ReportNumberOfDTCByStatusMask(const can_message_t *requestMsg)
//{
//    uint8_t dtcStatusMask;
//    uint16_t matchDTCCount;
//    if (requestMsg->length < 2)
//    {
//        SendNRC(requestMsg->data[0], NRC_RESPONSE_INCORRECT_LENGTH_OR_FORMAT);
//        return;
//    }
//
//    dtcStatusMask = requestMsg->data[2];
//    matchDTCCount = UDS_CountDTCByStatusMask(dtcStatusMask);
//    can_buff_config_t buffCfg = {
//        .enableFD = false,
//        .enableBRS = false,
//        .fdPadding = 0U,
//        .idType = CAN_MSG_ID_STD,
//        .isRemote = false};
//    /* Configure TX buffer with index TX_MAILBOX*/
//    CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);
//
//    /* Prepare message to be sent */
//    can_message_t message = {
//        .cs = 0U,
//        .id = TX_MSG_ID,
//        .data[0] = requestMsg->data[0] + 0x40,
//        .data[1] = requestMsg->data[1],
//        .data[2] = dtcStatusMask,
//        .data[3] = DTC_FORMAT_ISO14229_1,
//        .data[4] = (uint8_t)((matchDTCCount >> 8) & 0xFF),
//        .data[5] = (uint8_t)(matchDTCCount & 0xFF),
//        .length = 5U};
//    /* Send the information via CAN */
//    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
//}
//
///**
// * Handler for subfunction 0x02: Report DTC by Status Mask
// *
// * Request format:  [SF(0x02)][StatusMask]
// * Response format: [SF(0x02)][StatusMask][DTCFormatIdentifier][DTC1(3 bytes)][Status1][DTC2(3 bytes)][Status2]...
// */
//void UDS_ReportDTCByStatusMask(const can_message_t *requestMsg)
//{
//    uint8_t dtcStatusMask;
//
//    if (requestMsg->length < 2)
//    {
//        SendNRC(requestMsg->data[0], NRC_RESPONSE_INCORRECT_LENGTH_OR_FORMAT);
//        return;
//    }
//
//    dtcStatusMask = requestMsg->data[2];
//
//    can_buff_config_t buffCfg = {
//        .enableFD = false,
//        .enableBRS = false,
//        .fdPadding = 0U,
//        .idType = CAN_MSG_ID_STD,
//        .isRemote = false};
//    /* Configure TX buffer with index TX_MAILBOX*/
//    CAN_ConfigTxBuff(&can_pal1_instance, TX_MAILBOX, &buffCfg);
//
//    /* Prepare message to be sent */
//    can_message_t message = {
//        .cs = 0U,
//        .id = TX_MSG_ID,
//        .data[0] = requestMsg->data[0] + 0x40,
//        .data[1] = requestMsg->data[1],
//        .data[2] = dtcStatusMask,
//        .data[3] = DTC_FORMAT_ISO14229_1,
//        .length = 3U};
//    for (uint8_t i = 0; i < NUM_DTC_ENTRIES; i++)
//    {
//        if ((dtcDB[i].statusByte & dtcStatusMask) != 0)
//        {
//            message.data[message.length++] = (uint8_t)((dtcDB[i].dtcCode >> 16) & 0xFF);
//            message.data[message.length++] = (uint8_t)((dtcDB[i].dtcCode >> 8) & 0xFF);
//            message.data[message.length++] = (uint8_t)(dtcDB[i].dtcCode & 0xFF);
//            message.data[message.length++] = dtcDB[i].statusByte;
//        }
//    }
//    /* Send the information via CAN */
//    CAN_Send(&can_pal1_instance, TX_MAILBOX, &message);
//}
