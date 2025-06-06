/***********************************************************************************************************************
 * This file was generated by the S32 Configuration Tools. Any manual edits made to this file
 * will be overwritten if the respective S32 Configuration Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Peripherals v10.0
processor: S32K144
package_id: S32K144_LQFP100
mcu_data: s32sdk_s32k1xx_rtm_403
processor_version: 0.0.0
functionalGroups:
- name: BOARD_InitPeripherals
  UUID: a6d43dcb-893a-49cd-9d9d-0bb26ff0f05b
  called_from_default_init: true
  selectedCore: core0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/*******************************************************************************
 * Included files 
 ******************************************************************************/
#include "peripherals_flash_FTFC.h"

/*******************************************************************************
 * flash_FTFC initialization code
 ******************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
instance:
- name: 'flash_FTFC'
- type: 'flash'
- mode: 'general'
- custom_name_enabled: 'true'
- type_id: 'flash'
- functional_group: 'BOARD_InitPeripherals'
- peripheral: 'FTFC'
- config_sets:
  - flash_driver:
    - flashConfig:
      - 0:
        - Configuration: 'Flash_InitConfig0'
        - readOnly: 'true'
        - PFlashBase: '0x00000000'
        - PFlashSize: '0x80000'
        - DFlashBase: '0x10000000'
        - EERAMBase: '0x14000000'
        - CallBack: 'NULL_CALLBACK'
    - quick_selection: 'flash_default'
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External variable could be made static.
 * The external variables will be used in other source files in application code.
 *
 */

    /* Flash user configuration 0 */
const flash_user_config_t Flash_InitConfig0 =
{
    .PFlashBase = 0x0U,
    .PFlashSize = 0x80000U,
    .DFlashBase = 0x10000000U,
    .EERAMBase = 0x14000000U,
    .CallBack = NULL_CALLBACK
};


