/*
#####################################################################################
# File: monitor.cpp                                                                 #
# Project: tinyUPS                                                                  #
# File Created: Thursday, 19th May 2022 3:13:05 am                                  #
# Author: Sergey Ko                                                                 #
# Last Modified: Wednesday, 2nd August 2023 9:46:26 pm                              #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "monitor.h"

/**
 * @brief Construct a new Monitor Class:: Monitor Class object
 *
*/
MonitorClass::MonitorClass() {
    pinMode(PIN_MONITOR_FANCTL, OUTPUT);
    digitalWrite(PIN_MONITOR_FANCTL, LOW);
    pinMode(PIN_MONITOR_INFREQ, INPUT);
    pinMode(PIN_MONITOR_OUTFREQ, INPUT);
    // onboard led power/activity indicator
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);
}

/**
 * @brief Setup pins
 *
*/
status_t MonitorClass::init() {
    esp_err_t e;
    // Pins
    if(!adcAttachPin(PIN_MONITOR_ADC_TEMP_SENSOR)) {
        __DL(F("(!) adc temp sensor config failed"));
        sysLog.putts(PSTR("(!) adc temp sensor config failed"));
        return ERR;
    }
    analogSetAttenuation(ADC_6db);
    // analogSetClockDiv(1);
    // analogReadResolution(12);
    // internal temperature sensor
    temp_sensor_config_t tcfg;
    tcfg.clk_div = 6;
    tcfg.dac_offset = TSENS_DAC_L2;
    if((e = temp_sensor_set_config(tcfg)) != ESP_OK) {
        __DF(PSTR("(!) temp sensor config failed [%d]\n"), e);
        sysLog.putts(PSTR("(!) temp sensor config failed [%d]"), e);
        return ERR;
    }
    // init for a driver
    if(upsDriverInit() != ESP_OK) {
        __DF(PSTR("(!) spi init failed [%d]\n"), e);
        sysLog.putts(PSTR("(!) spi init failed [%d]"), e);
        return ERR;
    }
    // sys temp sensor
    e = temp_sensor_start();
    if(e != ESP_OK) {
        __DF(PSTR("(!) sys temp sensor init failed [%d]\n"), e);
        monitorData.upsAdvSystemTemperature = 0;
    }
    return OKAY;
}

/**
 * @brief This goes into main loop()
 *        Does tempertature reading every 60 sec and
 *        saves into log values of every 5th minute
 *
*/
void MonitorClass::loop() {
    if (this->_last_update == 0 || (millis() - this->_last_update >= 60000UL)) {
        esp_err_t e = temp_sensor_read_celsius(&monitorData.upsAdvSystemTemperature);
        if(e != ESP_OK) {
            __DF(PSTR("(!) sys temp sensor read failed [%d]\n"), e);
        }
        float adcVal = readADCmV();
        monitorData.upsAdvBatteryTemperature = mVtoCelsius(adcVal);
        // writing each 5th minute
        if(_update_cntr == 5) {
            monTempLog.putdts(monitorData.upsAdvSystemTemperature, true, false);
            monTempLog.putdts(monitorData.upsAdvBatteryTemperature, false, true);
            _update_cntr = 0;
        }
    #if DEBUG == 4
        __DF(PSTR("(i) sys: %.2f°C, bat: %.2f°C\n"), monitorData.upsAdvSystemTemperature, monitorData.upsAdvBatteryTemperature);
    #endif
        // trigger the control pin
        if(monitorData.upsAdvBatteryTemperature  >= config.batteryTempUT) {
            if(!_fan_on) {
                coolingSwitchOn();
                sysLog.putts(PSTR("battery:%.2f°C, cooler: ON"), monitorData.upsAdvBatteryTemperature );
            }
        } else {
            if(monitorData.upsAdvSystemTemperature >= config.deviceTempUT) {
                if(!_fan_on) {
                    coolingSwitchOn();
                    sysLog.putts(PSTR("system:%.2f°C, cooler: ON"), monitorData.upsAdvSystemTemperature );
                }
            } else if(_fan_on) {
                if(monitorData.upsAdvBatteryTemperature <= config.batteryTempLT && monitorData.upsAdvSystemTemperature <= config.deviceTempLT) {
                    coolingSwitchOff();
                    sysLog.putts(PSTR("battery:%.2f°C, system:%.2f°C, cooler: OFF"),
                                monitorData.upsAdvBatteryTemperature, monitorData.upsAdvSystemTemperature );
                }
            }
        }
        this->_last_update = millis();
        _update_cntr++;
    }
    // call driver loop routine
    upsDriverLoop();
    // handle the recent events
    if(systemEvent.upsBatteryCapacityChange) {
        sysLog.putts(PSTR("(e) battery capacity: %d%"), monitorData.upsAdvBatteryCapacity);
    #ifdef DEBUG
        __DF(PSTR("(e) battery capacity: %d%\n"), monitorData.upsAdvBatteryCapacity);
    #endif
        systemEvent.upsBatteryCapacityChange = false;
    }
    if(systemEvent.upsBatteryStatusChange) {
        sysLog.putts(PSTR("(e) battery status: %s (%d)"), upsBatteryStatusCodeToString(monitorData.upsBasicBatteryStatus).c_str(), monitorData.upsBasicBatteryStatus);
    #ifdef DEBUG
        __DF(PSTR("(e) battery status: %s (%d)\n"), upsBatteryStatusCodeToString(monitorData.upsBasicBatteryStatus).c_str(), monitorData.upsBasicBatteryStatus);
    #endif
        monDataLog.putdts(monitorData.upsBasicBatteryStatus, true, false);
        monDataLog.put("1");
        systemEvent.upsBatteryStatusChange = false;
    }
    if(systemEvent.upsOutputStateChange) {
        sysLog.putts(PSTR("(e) power status: %s (%d)"), upsOutputStatusCodeToString(monitorData.upsBasicOutputStatus).c_str(), monitorData.upsBasicOutputStatus);
    #ifdef DEBUG
        __DF(PSTR("(e) power status: %s (%d)\n"), upsOutputStatusCodeToString(monitorData.upsBasicOutputStatus).c_str(), monitorData.upsBasicOutputStatus);
    #endif
        monDataLog.putdts(monitorData.upsBasicOutputStatus, true, false);
        monDataLog.put("0");
        systemEvent.upsOutputStateChange = false;
    }
}

/**
 * @brief Swithes cooler fan ON
 *
*/
void MonitorClass::coolingSwitchOn() {
    digitalWrite(PIN_MONITOR_FANCTL, HIGH);
    this->_fan_on = true;
}

/**
 * @brief Swithes cooler fan OFF
 *
*/
void MonitorClass::coolingSwitchOff() {
    digitalWrite(PIN_MONITOR_FANCTL, LOW);
    this->_fan_on = false;
}

/**
 * @brief Read analog pin and returns an average
 *        value for precise readings
 *
 * @return float
*/
float MonitorClass::readADCmV() {
    uint32_t d = 0;
    uint8_t cntr = 0;
    while(cntr < 3) {
        d += analogReadMilliVolts(PIN_MONITOR_ADC_TEMP_SENSOR);
        delay(rand() % 100 + 500);
        cntr ++;
    }
    return (d/3.0);
}

/**
 * @brief Returns SYSTEM temperature in °C
 *
 * @return float
*/
float MonitorClass::getSysTemp() {
    return monitorData.upsAdvSystemTemperature;
}

/**
 * @brief Returns BATTERY temperature in °C
 *
 * @return float
*/
float MonitorClass::getBatTemp() {
    return monitorData.upsAdvBatteryTemperature;
}

/**
 * @brief Converts millivolts to Celcius using ntcRTTable
 *
 * @param mv
 * @return float
*/
float MonitorClass::mVtoCelsius(float & mv, analog_thermistor_data_t *td) {
    float Rth = (_r1 * mv)/(_vin - mv);
    float tmp = getTempCelsius(Rth);
    if(td != nullptr) {
        td->Rth = Rth;
        td->temp = tmp;
    }
    return tmp;
}
