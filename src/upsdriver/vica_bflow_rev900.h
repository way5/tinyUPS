/*
#####################################################################################
# File: vica_bflow_rev900.h                                                         #
# File Created: Thursday, 8th June 2023 9:45:49 pm                                  #
# Author: Sergey Ko                                                                 #
# Last Modified: Tuesday, 4th July 2023 10:28:56 pm                                 #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#ifndef VICA_B_FLOW_REVOLUTION_900VA_H
#define VICA_B_FLOW_REVOLUTION_900VA_H

#include "helpers.h"
#include "esp_attr.h"
#include "driver/spi_slave.h"

#define PIN_MONITOR_SPIMOSI                                 34
#define PIN_MONITOR_SPICLK                                  37
#define PIN_MONITOR_SPICS                                   33
// defaults
// #define UPS_RATED_INPUT_VOLTAGE                         120
// #define UPS_RATED_INPUT_FREQ                            60
// #define UPS_RATED_LOW_INPUT_VOLTAGE_THRESHOLD           100
// #define UPS_RATED_HIGH_INPUT_VOLTAGE_THRESHOLD          130
// #define UPS_RATED_OUTPUT_VOLTAGE                        120
// #define UPS_RATED_OUTPUT_FREQ                           60
// #define BATTERY_RATED_CHARGE_CAPACITY_Ah                18
// #define UPS_RATED_BATTERY_AMPS_MAX                      9
// length of multiples of 4 bytes
#define MONSPI_RX_BUFFER_LENGTH                             8
#define MONSPI_TICKS_TO_WAIT                                0xFF
#define MONSPI_QUEUE_LENGTH                                 12
/**< SPI mode (CPOL, CPHA) configuration:
    - 0: (0, 0)
    - 1: (0, 1)
    - 2: (1, 0)
    - 3: (1, 1)
*/
#define MONSPI_SLAVE_MODE                                    3
#define UPS_SPI_HOST                                         SPI2_HOST

typedef uint16_t rx_value_t;

esp_err_t upsDriverInit();
void upsDriverDeinit();
void upsDriverLoop();
uint32_t upsDriverGetCurrentBatteryLifeTime(uint8_t currentOutputLoad, uint8_t currentCapacity);
void IRAM_ATTR upsSPISetupComplete(spi_slave_transaction_t *trans);
void IRAM_ATTR upsSPITransferComplete(spi_slave_transaction_t *trans);

#endif                                  // VICA_B_FLOW_REVOLUTION_900VA_H