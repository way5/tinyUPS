/*
#####################################################################################
# File: monitor.h                                                                   #
# Project: tinyUPS                                                                  #
# File Created: Thursday, 19th May 2022 2:48:56 pm                                  #
# Author: Sergey Ko                                                                 #
# Last Modified: Tuesday, 1st August 2023 2:34:57 pm                                #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                      #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#ifndef MONITOR_H
#define MONITOR_H

#include "helpers.h"
#include "driver/temp_sensor.h"
#include "eemem.h"
#include "ntpc.h"
#include "flog.h"
#include "snmp/defs.h"
// device drivers
#ifdef VICA_B_FLOW_REV900
    #include "upsdriver/vica_bflow_rev900.h"
#else
    // another UPS model driver
#endif

extern fLogClass sysLog;
extern fLogClass monTempLog;
extern fLogClass monDataLog;
/**
 *
 *             R1 2k2         TH 1k
 *       3V3 ---[∎]--- AV -∙---[/]---∙- GND
 *
 */
static const uint16_t _r1 = 2164;
static const uint16_t _vin = 3300;
/*
    Vo = (Vi * R1)/(R1 + TH1)
    TH1 = (R1 * Vo) / (Vi - Vo)
*/

typedef struct {
    float Rth = 0;
    float temp = 0;
} analog_thermistor_data_t;

float getTempCelsius(float & Rth);
float getTempFarenheit(float & Rth);

class MonitorClass {
    public:
        MonitorClass();
        ~MonitorClass() {
            temp_sensor_stop();
            upsDriverDeinit();
        };
        status_t init();
        void loop();
        float readADCmV();
        float getSysTemp();
        float getBatTemp();
        float mVtoCelsius(float & mv, analog_thermistor_data_t *td = nullptr);
        bool isCooling() {
            return _fan_on;
        }
        uint32_t getUPSLifeTimeSeconds() {
            return upsDriverGetCurrentBatteryLifeTime(monitorData.upsAdvOutputLoad, monitorData.upsAdvBatteryCapacity);
        };
        void coolingSwitchOn();
        void coolingSwitchOff();

    private:
        unsigned long _last_update = 0;
        uint8_t _update_cntr = 0;
        bool _fan_on = false;
};

extern MonitorClass monitor;

#endif                          // MONITOR_H