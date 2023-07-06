/*
#####################################################################################
# File: vica_bflow_rev900.cpp                                                       #
# File Created: Thursday, 8th June 2023 10:53:51 pm                                 #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 3rd July 2023 12:18:28 pm                                  #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "vica_bflow_rev900.h"

static rx_value_t WORD_ALIGNED_ATTR _rxBuffer[MONSPI_RX_BUFFER_LENGTH] = {0};
spi_slave_interface_config_t spiSlaveConfig;
spi_bus_config_t spiBusConfig;
spi_slave_transaction_t spiTrans;

static const uint8_t _voltageAvgCounterMax = 20;
static const uint8_t _loadAvgCounterMax = 5;

volatile static uint8_t _lastAddress = 0;
volatile static uint16_t _inputVoltageTmp = 0;
volatile static uint16_t _outputVoltageTmp = 0;
volatile static uint16_t _inputVoltageAvgSum = 0;
volatile static uint8_t _inputVoltageAvgCount = 1;
volatile static uint16_t _outputVoltageAvgSum = 0;
volatile static uint8_t _outputVoltageAvgCount = 1;
volatile static uint16_t _loadAvgSum = 0;
volatile static uint8_t _loadAvgCount = 1;
// event data buffers
volatile static uint8_t _eventBufferCursor = 0;
volatile static uint8_t _eventBatteryStatusChange[3] = {0};
volatile static uint8_t _eventBatteryCapacityChange[3] = {0};
volatile static uint8_t _eventOutputStatusChange[3] = {0};

/**
 * @brief Clear the RX buffer
 * 
*/
inline static void spiBuffereClear() {
    uint8_t i = 0;
    while(i < MONSPI_RX_BUFFER_LENGTH) {
        _rxBuffer[i] = 0;
        i ++;
    }
}

/**
 * @brief Decode segment display digit code into decimal value
 *
 * @param value
 * @return uint8_t
*/
inline static uint8_t lcdDigitToValue(uint8_t & value) {
    uint8_t r = 0;
    switch(value) {
        default:
        case 0b01111101:
            // 0
            break;
        case 0b00000101:
            r = 1;
            break;
        case 0b01011110:
            r = 2;
            break;
        case 0b01001111:
            r = 3;
            break;
        case 0b00100111:
            r = 4;
            break;
        case 0b01101011:
            r = 5;
            break;
        case 0b01111011:
            r = 6;
            break;
        case 0b01000101:
            r = 7;
            break;
        case 0b01111111:
            r = 8;
            break;
        case 0b01101111:
            r = 9;
            break;
    }
    return r;
}

/**
 * @brief Driver initializer
 * 
 * @return esp_err_t 
*/
esp_err_t upsDriverInit() {
    esp_err_t e;
    spiSlaveConfig.spics_io_num = PIN_MONITOR_SPICS;
    spiSlaveConfig.mode = MONSPI_SLAVE_MODE;
    spiSlaveConfig.post_setup_cb = upsSPISetupComplete;
    spiSlaveConfig.post_trans_cb = upsSPITransferComplete;
    // see current driver settings 
    spiSlaveConfig.queue_size = MONSPI_QUEUE_LENGTH;
    // spiSlaveConfig.flags = SPI_SLAVE_RXBIT_LSBFIRST;
    // spiBusConfig.flags = SPICOMMON_BUSFLAG_SLAVE | SPICOMMON_BUSFLAG_SCLK | SPICOMMON_BUSFLAG_MOSI;
    spiBusConfig.sclk_io_num = PIN_MONITOR_SPICLK;
    spiBusConfig.mosi_io_num = PIN_MONITOR_SPIMOSI;
    spiBusConfig.miso_io_num = -1;
    spiBuffereClear();
    spiTrans.rx_buffer = _rxBuffer;
    spiTrans.tx_buffer = NULL;
    // length in bits
    spiTrans.length = MONSPI_RX_BUFFER_LENGTH * 8;
    e = spi_slave_initialize(UPS_SPI_HOST, &spiBusConfig, &spiSlaveConfig, SPI_DMA_CH_AUTO);
    return e;
}

/**
 * @brief Get the Current Battery Life Time value in seconds
 *        Since the battery might be replaced with non standard we cannot evaluate
 *        the time using the conventional method. The following calculations
 *        can be changed in order to match with your current situation.
 *        Here we also can aproximate the remaining time by using known estimated
 *        battery life time with 100% load, current load and the
 *        charge in percents.
 *
 * @param currentOutputLoad
 * @param currentCapacity
 * @return uint32_t
*/
uint32_t upsDriverGetCurrentBatteryLifeTime(uint8_t currentOutputLoad, uint8_t currentCapacity) {
    // (charge capacity) = I*t
    float r = (BATTERY_RATED_CHARGE_CAPACITY_Ah * (currentCapacity/100.0));
    r /= (UPS_RATED_BATTERY_AMPS_MAX * (currentOutputLoad/100.0));
    r *= 3600;
    return static_cast<uint32_t>(r);
}

/**
 * @brief Callback called after the SPI registers are loaded with new data.
 *
 * @param trans
*/
void IRAM_ATTR upsSPISetupComplete(spi_slave_transaction_t *trans) {
    // do something
}

/**
 * @brief Callback called after a transaction is done.
 *
 * @param trans
*/
void IRAM_ATTR upsSPITransferComplete(spi_slave_transaction_t *trans) {
    uint8_t addr = 0;
    uint8_t data = 0;
    rx_value_t * rx = reinterpret_cast<rx_value_t *>(trans->rx_buffer);
    // process only words, skip 7 bytes of header
    if(trans->trans_len == 16) {
        addr = *rx & 0xFF;
        data = *rx >> 8;
        // data reading consistancy control
        if(addr == 160 && _lastAddress != 0) {
            _lastAddress = 0;
            goto vica_spi_transfer_complete_end;
        } else if(addr == 170) {
            _lastAddress = 0;
        } else if((addr < 160 && addr > 170) || (addr != 160 && addr != (_lastAddress+1))) {
            _lastAddress = 0;
            goto vica_spi_transfer_complete_end;
        } else
            _lastAddress = addr;
        // decode data
        switch(addr) {
            // 1st digit
            case 160:
                _inputVoltageTmp = lcdDigitToValue(data) * 100;
                break;
            // 2nd digit
            case 161:
                _inputVoltageTmp += lcdDigitToValue(data) * 10;
                break;
            // 3rd digit
            case 162:
                // this prevents wrong readings to be used when the 
                // rapid SPI data change occurs. sometimes 160 carries data 0
                // or data doesn't read correctly
                _inputVoltageTmp += lcdDigitToValue(data);
                 // cheating here, the reason is the screwed UPS controller SPI implementation
                if(_inputVoltageTmp >= UPS_RATED_LOW_INPUT_VOLTAGE_THRESHOLD) {
                    _inputVoltageAvgSum += _inputVoltageTmp;
                    monitorData.upsAdvInputLineVoltage = _inputVoltageAvgSum/_inputVoltageAvgCount;
                    monitorData.upsHighPrecInputLineVoltage = monitorData.upsAdvInputLineVoltage * 10;
                    _inputVoltageAvgCount++;
                    if(_inputVoltageAvgCount > _voltageAvgCounterMax) {
                        _inputVoltageAvgCount = 2;
                        _inputVoltageAvgSum = monitorData.upsAdvInputLineVoltage;
                    }
                } else if(_inputVoltageTmp == 0) {
                    monitorData.upsAdvInputLineVoltage = 0;
                    monitorData.upsHighPrecInputLineVoltage = 0;
                }
                break;
            // 1st digit
            case 163:
                _outputVoltageTmp = lcdDigitToValue(data) * 100;
                break;
            // 2nd digit
            case 164:
                    _outputVoltageTmp += lcdDigitToValue(data) * 10;
                break;
            // 3rd digit
            case 165:
                // this prevents wrong readings to be used when the 
                // rapid SPI data change occurs. sometimes 163 carries data 0
                // or data doesn't read correctly, theoretically here 
                // should be used the UPS specific threshold value for output voltage
                _outputVoltageTmp += lcdDigitToValue(data);
                // cheating here, the reason is the screwed UPS controller SPI implementation
                if(_outputVoltageTmp >= 90) {
                    _outputVoltageAvgSum += _outputVoltageTmp;
                    monitorData.upsAdvOutputVoltage = _outputVoltageAvgSum/_outputVoltageAvgCount;
                    monitorData.upsHighPrecOutputVoltage = monitorData.upsAdvOutputVoltage * 10;
                    _outputVoltageAvgCount++;
                    if(_outputVoltageAvgCount > _voltageAvgCounterMax) {
                        _outputVoltageAvgCount = 2;
                        _outputVoltageAvgSum = monitorData.upsAdvOutputVoltage;
                    }
                } else if(_outputVoltageTmp == 0) {
                    monitorData.upsAdvOutputVoltage = 0;
                    monitorData.upsHighPrecOutputVoltage = 0;
                }
                break;
            // BATT MODE+FAULT icons + LOAD BAR + LOW BATT + UNDERLINE
            case 166:
                {
                    uint8_t load = data & 0b00000111;
                    // BATT MODE, BATT MODE+LOW BATT, BATT MODE+FAULT, AC POWER (NO SYMBOL),
                    // FAULT (IMMEDIATE POWER OFF), BATT MODE + LOW BATT + FAULT
                    uint8_t mode = data >> 3;
                    monitorData.upsDiagBatteryStatus = 3;
                    switch(load) {
                        case 0b00000111:
                            monitorData.upsAdvOutputLoad = 100;
                            break;
                        case 0b00000110:
                            monitorData.upsAdvOutputLoad = 70;
                            break;
                        case 0b00000100:
                            monitorData.upsAdvOutputLoad = 40;
                            break;
                        default:
                        case 0:
                            monitorData.upsAdvOutputLoad = 10;
                            break;
                    }
                    // average value
                    _loadAvgSum += monitorData.upsAdvOutputLoad;
                    monitorData.upsAdvOutputLoad = _loadAvgSum/_loadAvgCount;
                    _loadAvgCount++;
                    if(_loadAvgCount > _loadAvgCounterMax) {
                        _loadAvgCount = 2;
                        _loadAvgSum = monitorData.upsAdvOutputLoad;
                    }
                    switch(mode) {
                        // normal operation
                        default:
                        case 0b00001000:
                            _eventOutputStatusChange[_eventBufferCursor] = 2;
                            _eventBatteryStatusChange[_eventBufferCursor] = 2;
                            break;
                        // on battery
                        case 0b00001001:
                            _eventOutputStatusChange[_eventBufferCursor] = 3;
                            _eventBatteryStatusChange[_eventBufferCursor] = 2;
                            break;
                        // on battery + batterey low (!)
                        case 0b00001011:
                            _eventOutputStatusChange[_eventBufferCursor] = 3;
                            _eventBatteryStatusChange[_eventBufferCursor] = 3;
                            break;
                        // on battery + fault
                        case 0b00001101:
                            _eventOutputStatusChange[_eventBufferCursor] = 3;
                            _eventBatteryStatusChange[_eventBufferCursor] = 4;
                            break;
                        // battery low (hypothetical mode)
                        case 0b00001010:
                            _eventOutputStatusChange[_eventBufferCursor] = 2;
                            _eventBatteryStatusChange[_eventBufferCursor] = 3;
                            monitorData.upsDiagBatteryStatus = 7;
                            break;
                        // all at once (immediately shuts off)
                        case 0b00001111:
                            _eventOutputStatusChange[_eventBufferCursor] = 3;
                            _eventBatteryStatusChange[_eventBufferCursor] = 4;
                            break;
                    }
                    // events. 
                    if(_eventBufferCursor == 2) {
                        // TODO: avoiding false-positives - check if received data values are consistent using hardcoded method
                        if(_eventOutputStatusChange[0] == _eventOutputStatusChange[1] 
                            && _eventOutputStatusChange[1] == _eventOutputStatusChange[2] 
                                && monitorData.upsBasicOutputStatus != _eventOutputStatusChange[_eventBufferCursor]) {
                            monitorData.upsBasicOutputStatus = _eventOutputStatusChange[_eventBufferCursor];
                            systemEvent.upsOutputStateChange = true;
                        }
                        if(_eventBatteryStatusChange[0] == _eventBatteryStatusChange[1] 
                            && _eventBatteryStatusChange[1] == _eventBatteryStatusChange[2] 
                                && monitorData.upsBasicBatteryStatus != _eventBatteryStatusChange[_eventBufferCursor]) {
                            monitorData.upsBasicBatteryStatus = _eventBatteryStatusChange[_eventBufferCursor];
                            systemEvent.upsBatteryStatusChange = true;
                        }
                    }
                }
                break;
            // AC MODE+OVERLOAD icons + BATTERY BAR + UNDERLINE
            case 167:
                {
                    uint8_t battery = data & 0b00000111;
                    // AC MODE, AC MODE + OVERLOAD, OVERLOAD
                    uint8_t mode = data >> 3;
                    switch(battery) {
                        case 0b00000111:
                            _eventBatteryCapacityChange[_eventBufferCursor] = 100;
                            break;
                        case 0b00000110:
                            _eventBatteryCapacityChange[_eventBufferCursor] = 70;
                            break;
                        case 0b00000100:
                            _eventBatteryCapacityChange[_eventBufferCursor] = 40;
                            break;
                        default:
                        case 0:
                            _eventBatteryCapacityChange[_eventBufferCursor] = 10;
                            break;
                    }
                    switch(mode) {
                        // AC mode (normal operation)
                        case 0b00001011:
                            if(monitorData.upsBasicOutputStatus == 3) {
                                // probably low input voltage
                                _eventOutputStatusChange[_eventBufferCursor] = 4;
                            }
                            break;
                        // overload
                        case 0b00001101:
                        case 0b00001111:
                            _eventOutputStatusChange[_eventBufferCursor] = 8;
                            break;
                        // looks like the battery mode
                        default:
                        case 0b00001001:
                            // doing nothing
                            break;
                    }
                    // events
                    if(_eventBufferCursor == 2) {
                        // avoiding false-positives
                        if(_eventOutputStatusChange[0] == _eventOutputStatusChange[1] 
                            && _eventOutputStatusChange[1] == _eventOutputStatusChange[2] 
                                && monitorData.upsBasicOutputStatus != _eventOutputStatusChange[_eventBufferCursor]) {
                            monitorData.upsBasicOutputStatus = _eventOutputStatusChange[_eventBufferCursor];
                            systemEvent.upsOutputStateChange = true;
                        }
                        if(_eventBatteryCapacityChange[0] == _eventBatteryCapacityChange[1] 
                            && _eventBatteryCapacityChange[1] == _eventBatteryCapacityChange[2] 
                                && monitorData.upsAdvBatteryCapacity != _eventBatteryCapacityChange[_eventBufferCursor]) {
                            monitorData.upsAdvBatteryCapacity = _eventBatteryCapacityChange[_eventBufferCursor];
                            systemEvent.upsBatteryCapacityChange = true;
                        }
                    }
                    _eventBufferCursor++;
                    if(_eventBufferCursor == 3) {
                        _eventBufferCursor = 0;
                    }
                }
                break;
            // IN/OUT AUXILIAR
            case 168:
                // skip this for now
                break;
            default:
                // 169, 170 - aren't used
                break;
        }
    }
vica_spi_transfer_complete_end:
    // the buffer'd be managed here
    spiBuffereClear();
}

/**
 * @brief Polling the SPI transmit queue
 *        using device specific configuration
 * 
*/
void upsDriverLoop() {
    spi_slave_transmit(UPS_SPI_HOST, &spiTrans, MONSPI_TICKS_TO_WAIT);
}

/**
 * @brief Destructor
 * 
*/
void upsDriverDeinit() {
    spi_slave_free(UPS_SPI_HOST);
}