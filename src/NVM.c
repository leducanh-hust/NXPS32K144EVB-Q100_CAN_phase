#include "NVM.h"

 // Assuming this is defined elsewhere
 extern flash_ssd_config_t flashSSDConfig;

NVM_Status NVM_Read(uint32_t offset, uint8_t *data, uint8_t len)
{
    if (offset + len > EEPROM_SIZE) {
        return NVM_INVALID_PARAM;
    }

    const uint8_t *eepAddr = (const uint8_t*)(EEPROM_START_ADDRESS + offset);
    // Simulate reading from EEPROM
    for (uint8_t i = 0; i < len; i++) {
        data[i] = (volatile uint8_t)(eepAddr[i]);
    }

    return NVM_OK;
}

NVM_Status NVM_Write(uint32_t offset, const uint8_t *data, uint8_t len)
{
    if (offset + len > EEPROM_SIZE) {
        return NVM_INVALID_PARAM;
    }

    //volatile uint8_t *eepAddr = (volatile uint8_t*)(EEPROM_START_ADDRESS + offset);

    status_t ret = FLASH_DRV_EEEWrite(&flashSSDConfig, EEPROM_START_ADDRESS + offset, len, (uint8_t*)data);

    return (ret == STATUS_SUCCESS) ? NVM_OK : NVM_ERROR;
}
