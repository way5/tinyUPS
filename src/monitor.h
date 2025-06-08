/*
#####################################################################################
# File: monitor.h                                                                   #
# Project: tinyUPS                                                                  #
# File Created: Thursday, 19th May 2022 2:48:56 pm                                  #
# Author: Sergey Ko                                                                 #
# Last Modified: Sunday, 8th June 2025 1:24:26 am                                   #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                      #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#ifndef MONITOR_H
#define MONITOR_H

#include "helpers.h"
#include "driver/temperature_sensor.h"
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

extern fLogClass logsys;
extern fLogClass logTempMon;
extern fLogClass logDataMon;
/**
 *
 *             R1 2k2         TH 1k
 *       3V3 ---[∎]--- AV -∙---[/]---∙- GND
 *
 */
const uint16_t _r1 = 2164;
const uint16_t _vin = 3300;
/*
    Vo = (Vi * R1)/(R1 + TH1)
    TH1 = (R1 * Vo) / (Vi - Vo)
*/

typedef struct
{
    float Rth = 0;
    float temp = 0;
} analog_thermistor_data_t;

float getTempCelsius(float &Rth);
float getTempFarenheit(float &Rth);

class MonitorClass
{
public:
    MonitorClass();
    ~MonitorClass();
    status_t init();
    void loop();
    float readADCmV();
    float getSysTemp();
    float getBatTemp();
    float mVtoCelsius(float &mv, analog_thermistor_data_t *td = nullptr);
    bool isCooling()
    {
        return _fan_on;
    }
    uint32_t getUPSLifeTimeSeconds()
    {
        return upsDriverGetCurrentBatteryLifeTime(monitorData.upsAdvOutputLoad, monitorData.upsAdvBatteryCapacity);
    };
    void coolingSwitchOn();
    void coolingSwitchOff();

private:
    unsigned long _last_update = 0;
    uint8_t _update_cntr = 0;
    bool _fan_on = false;
    // temperature_sensor_config_t _ts_cfg;
    // temperature_sensor_handle_t _ts_handle = NULL;
};

extern MonitorClass monitor;

#endif // MONITOR_H