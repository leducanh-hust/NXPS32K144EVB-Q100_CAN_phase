#include "sdk_project_config.h"
#include <interrupt_manager.h>
#include <stdint.h>
#include "UDS.h"
#include "NVM.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

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
//#define SLAVE 

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
#define PERIOD_BY_MS 1000UL

typedef enum
{
    LED0_CHANGE_REQUESTED = 0x00U,
    LED1_CHANGE_REQUESTED = 0x01U
} can_commands_list;

uint8_t ledRequested = (uint8_t)LED0_CHANGE_REQUESTED;
flash_ssd_config_t flashSSDConfig;

volatile bool raw_btn1 = false;
volatile bool stable_btn1 = false;
volatile bool btn1_handled = false;
volatile uint16_t btn1_press_ticks = 0;
volatile uint16_t btn1_debounce_ticks = 0;

volatile bool raw_btn2 = false;
volatile bool stable_btn2 = false;
volatile bool btn2_handled = false;
volatile uint16_t btn2_press_ticks = 0;
volatile uint16_t btn2_debounce_ticks = 0;

#define DEBOUNCE_DELAY_MS 2U         // ~20ms debounce lockout
#define LONG_PRESS_THRESHOLD_MS 488U // 5 seconds for long press

/******************************************************************************
 * Function prototypes
 ******************************************************************************/

void BoardInit(void);
void GPIOInit(void);
void CheckButtonLongPress(void);

/******************************************************************************
 * Functions
 ******************************************************************************/
/*Timer callback function*/

void timingFtmInst0_callback(void *param)
{
    static bool ledOn = false;

    // Toggle LED0 to confirm timer is ticking
    if (ledOn)
        PINS_DRV_ClearPins(GPIO_PORT, (1 << LED1)); // Turn ON (active low)
    else
        PINS_DRV_SetPins(GPIO_PORT, (1 << LED1)); // Turn OFF

    ledOn = !ledOn;
    // Debounce BTN1
    if (btn1_debounce_ticks > 0)
    {
        btn1_debounce_ticks--;
        if (btn1_debounce_ticks == 0)
        {
            if (raw_btn1 != stable_btn1)
            {
                stable_btn1 = raw_btn1;
                btn1_press_ticks = 0;
                btn1_handled = false;
            }
        }
    }

    // Debounce BTN2
    if (btn2_debounce_ticks > 0)
    {
        btn2_debounce_ticks--;
        if (btn2_debounce_ticks == 0)
        {
            if (raw_btn2 != stable_btn2)
            {
                stable_btn2 = raw_btn2;
                btn2_press_ticks = 0;
                btn2_handled = false;
            }
        }
    }

    // Long press logic BTN1
    if (stable_btn1 && !btn1_handled)
    {
        btn1_press_ticks++;
        if (btn1_press_ticks >= LONG_PRESS_THRESHOLD_MS)
        {
            btn1_handled = true;

            // Set the confirmedDTC bit in status byte
            uint8_t statusByte = NVM_Read(DTC_Snapshot_Offset(0), &statusByte, 1);
            statusByte |= (1 << 4);                            // Set the confirmedDTC bit
            NVM_Write(DTC_Snapshot_Offset(0), &statusByte, 1); // Write back the updated status byte
        }
    }

    // Long press logic BTN2
    if (stable_btn2 && !btn2_handled)
    {
        btn2_press_ticks++;
        if (btn2_press_ticks >= LONG_PRESS_THRESHOLD_MS)
        {
            btn2_handled = true;

            // Set the confirmedDTC bit in status byte
            uint8_t statusByte = NVM_Read(DTC_Snapshot_Offset(1), &statusByte, 1);
            statusByte |= (1 << 4); // Set the confirmedDTC bit
            NVM_Write(DTC_Snapshot_Offset(1), &statusByte, 1);
        }
    }
}

void buttonISR(void)
{
    uint32_t flag = PINS_DRV_GetPortIntFlag(BTN_PORT);

    if (flag & (1 << BTN1_PIN))
    {
        raw_btn1 = (PINS_DRV_ReadPins(BTN_GPIO) & (1 << BTN1_PIN)); // Active LOW
        btn1_debounce_ticks = DEBOUNCE_DELAY_MS;
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN1_PIN);
    }

    if (flag & (1 << BTN2_PIN))
    {
        raw_btn2 = (PINS_DRV_ReadPins(BTN_GPIO) & (1 << BTN2_PIN));
        btn2_debounce_ticks = DEBOUNCE_DELAY_MS;
        PINS_DRV_ClearPinIntFlagCmd(BTN_PORT, BTN2_PIN);
    }
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
    PINS_DRV_SetPinsDirection(GPIO_PORT, (1 << LED1) | (1 << LED0)); //

    /* Set Output value LEDs */
    PINS_DRV_ClearPins(GPIO_PORT, (1 << LED1) | (1 << LED0));

    PINS_DRV_SetPins(GPIO_PORT, (1 << LED0) | (1 << LED1));

    /* Setup button pin */
    PINS_DRV_SetPinsDirection(BTN_GPIO, ~((1 << BTN1_PIN) | (1 << BTN2_PIN)));

    /* Setup button pins interrupt */
    PINS_DRV_SetPinIntSel(BTN_PORT, BTN1_PIN, PORT_INT_EITHER_EDGE);
    PINS_DRV_SetPinIntSel(BTN_PORT, BTN2_PIN, PORT_INT_EITHER_EDGE);

    /* Install buttons ISR */
    INT_SYS_InstallHandler(BTN_PORT_IRQn, &buttonISR, NULL);

    /* Enable buttons interrupt */
    INT_SYS_EnableIRQ(BTN_PORT_IRQn);
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
    uint64_t ftmResolution;
    BoardInit();
    GPIOInit();
    CAN_Init(&can_pal1_instance, &can_pal1_Config0);
    status_t status;
    /* Initialize the Flash driver */
    status = FLASH_DRV_Init(&Flash_InitConfig0, &flashSSDConfig);
    DEV_ASSERT(status == STATUS_SUCCESS);

    /*Initialize Timer driver*/
    status = TIMING_Init(&timingFtmInst0, &timingFtm_InitConfig0);
    DEV_ASSERT(status == STATUS_SUCCESS);

    status = TIMING_GetResolution(&timingFtmInst0, TIMER_RESOLUTION_TYPE_MICROSECOND, &ftmResolution);
    DEV_ASSERT(status == STATUS_SUCCESS);

    TIMING_StartChannel(&timingFtmInst0, FTM_CHANNEL, 0);

    /*Timer part ends here*/

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
    // Enable EEPROM emulation via FlexRAM
    status = FLASH_DRV_SetFlexRamFunction(&flashSSDConfig, EEE_ENABLE, 0x00u, NULL);
    DEV_ASSERT(status == STATUS_SUCCESS);

    // // Write DID 1008 data first for testing purpose
    // uint8_t writeData[] = {0x01, 0x05, 0x6A, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    // NVM_Write(0x00, writeData, sizeof(writeData) / sizeof(writeData[0]));

    // // Wrtie DTC codes for testing purpose
    // uint8_t dtcCodes[] = {0x00, 0xA3, 0xD8, 0x00, 0x00, 0xA3, 0xD9, 0x00};
    // NVM_Write(DTC_CODE_OFFSET, dtcCodes, 8);

    // // Write DTC snapshots for testing purpose
    // // First 2 bytes are DTC Status Bytes, rest are padding bytes
    // uint8_t dtcSnapshots[] = {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    // NVM_Write(DTC_SNAPSHOT_OFFSET, dtcSnapshots, 8);
    // Verify Data After Write

    // volatile uint8_t readBuf[8] = {0};
    // /*Read the current DTC Snapshot*/
    // NVM_Read(DTC_SNAPSHOT_OFFSET, readBuf, 2);
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
        // Then check if a CAN frame was received
        while (CAN_GetTransferStatus(&can_pal1_instance, RX_MAILBOX) == STATUS_BUSY)
            ;
        can_message_t recvMsg;

        // Start reception (non-blocking)
        CAN_Receive(&can_pal1_instance, RX_MAILBOX, &recvMsg);

        // Wait briefly until message is ready (still safe if not busy)
        while (CAN_GetTransferStatus(&can_pal1_instance, RX_MAILBOX) == STATUS_BUSY)
            ;

        switch (recvMsg.data[0])
        {
        case 0x22:
            UDS_ReadDataByIdentifier(&recvMsg);
            break;
        case 0x2E:
            UDS_WriteDataByIdentifier(&recvMsg);
            break;
        case 0x19:
            UDS_ReadDTCInformation(&recvMsg);
            break;
        default:
            SendNRC(recvMsg.data[0], UDS_RESPONSE_SERVICE_NOT_SUPPORTED);
            break;
        }
    }
    //

    //

    // /* Initialize clock module */
    // status = CLOCK_DRV_Init(&clockMan1_InitConfig0);
    // DEV_ASSERT(status == STATUS_SUCCESS);
    // /* Initialize LEDs and Button configuration */
    // status = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    // DEV_ASSERT(status == STATUS_SUCCESS);
}
