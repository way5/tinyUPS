/*
#####################################################################################
# File: helpers.h                                                                   #
# Project: tinyUPS                                                                  #
# File Created: Thursday, 19th May 2022 2:36:45 am                                  #
# Author: Sergey Ko                                                                 #
# Last Modified: Wednesday, 5th July 2023 11:47:19 pm                               #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/
/* NOTE:
    ******** DEVELOPERs' MEMO

    1. Debug levels:
        1 - flog
        2 - ntpc
        3 - httpd
        4 - monitor
        5 - snmp
        6 - snmp OID detection via serial

*/

#ifndef HELPERS_H
#define HELPERS_H

#include <WiFi.h>

// #define DEBUG                                   5
// #undef  ARDUHAL_LOG_LEVEL
// #define ARDUHAL_LOG_LEVEL                       ARDUHAL_LOG_LEVEL_DEBUG

typedef enum {
    OKAY =           1,
    ERR =            0,
    NOT =            2,
    WAIT =           3,
    NTP_SYNC_INTL = -1,
    NTP_SYNC_ERR =  -2,
} status_t;

#define _CHB(b, s) do {                                             \
    b = new char[s];                                                \
    memset(b, '\0', s);                                             \
} while(0)

#define _CHBC(b) do {                                               \
    memset(b, '\0', strlen(b));                                     \
} while(0)

#define _CHBD(b) do {                                               \
    delete[] b;                                                     \
    b = NULL;                                                       \
} while(0)

#if defined(DEBUG_ESP_PORT)
#define __DF(F, ...)     do {                                       \
        DEBUG_ESP_PORT.printf(F, ##__VA_ARGS__);                    \
    } while(0)
#define __DL(F)          do {                                       \
        DEBUG_ESP_PORT.println(F);                                  \
    } while(0)
#define __D(F)           do {                                       \
        DEBUG_ESP_PORT.print(F);                                    \
    } while(0)
#else
#define __DF(F ...)                     (void)0
#define __DL(F)                         (void)0
#define __D(F)                          (void)0
#endif

// config
typedef struct {                                            // Configurable, Units
    // - WiFi (both 32 octets max)
    char ssid[32] = "";                                     // V
    char ssidkey[32] = "";                                  // V
    char apkey[32] = "iesh3Iequaef";                        // X
    // - NTP
    char ntpServer[20] = "pool.ntp.org";                    // V
    char ntpServerFB[20] = "time.google.com";               // V
    uint16_t ntpSyncInterval = 10800;                       // V, sec
    // variable defines the offset in seconds for daylight 
    // saving time. It is generally one hour, that corresponds to 3600 seconds
    int32_t ntpDaylightOffset = 0;                          // X, sec
    // uint16_t ntpPort = 1337;                             // V
    // (GMT -1 = -3600, GMT 0 = 0, GMT +1 = 3600)
    int8_t ntpTimeOffset = 0;                               // V, hrs
    // - Auth (both 16 octets max)
    char admLogin[16] = "";                                 // V
    char admPassw[16] = "";                                 // V
    uint16_t authTimeoutMax = 3600;                         // V, seconds
    // - SYS: DIGIT
    float batteryTempUT = 55.00;                            // V, degree C
    float batteryTempLT = 45.00;                            // V, degree C
    float deviceTempUT = 65.00;                             // V, degree C
    float deviceTempLT = 50.00;                             // V, degree C
    // - SNMP: STRING
    char snmpGetCN[20] = "public";                          // V
    char snmpSetCN[20] = "secret";                          // V
    char snmpTrapCN[20] = "";
    char sysLocation[20] = "";                              // V
    char sysContact[30] = "";                               // V
    // SNMP: DIGIT
    uint16_t snmpPort = 161;                                // V
    uint16_t snmpTrapPort = 162;                            // V
    // - OIDs
    char BatteryLastReplaceDate[7] = "010120";              // V, mm/dd/yy
    // The delay in seconds the UPS remains on after being told to turn off
    uint16_t upsAdvConfigShutoffDelay = 50;                 // V, seconds
    // The delay in seconds after utility line power returns before the UPS will turn on
    uint16_t upsAdvConfigReturnDelay = 20;                  // V, seconds
    // The desired run time of the UPS, in seconds, once the low battery condition is reached
    uint16_t upsAdvConfigLowBatteryRunTime = 40;            // V, seconds
    // unique UPS serial number
    char upsSerialNumber[16] = "";                          // X
} config_t;
extern config_t config;

// web Server
typedef struct {
    char authToken[64] = "";
    unsigned long authTimeout = 0;
} session_t;
extern session_t session;

// system events
typedef struct {
    bool upsBatteryStatusChange = false;
    bool upsOutputStateChange = false;
    bool upsBatteryCapacityChange = false;
    // when failed to connect to config.ssid
    // bool wifiAPConnectFailed = false;
} common_event_t;
volatile extern common_event_t systemEvent;

// monitored parameters
typedef struct {
    // rw
    uint8_t upsAdvControlUpsOff = 1;
    uint8_t upsAdvControlTurnOnUPS = 2;
    uint8_t upsAdvControlSimulatePowerFail = 1;
    uint8_t upsAdvControlBypassSwitch = 1;
    uint8_t upsAdvControlFlashAndBeep = 1;
    uint8_t upsAdvTestDiagnostics = 1;
    uint8_t upsAdvTestRuntimeCalibration = 1;
    uint8_t upsPhaseResetMaxMinValues = 1;
    uint8_t upsBasicOutputStatus = 2;
    uint8_t upsBasicBatteryStatus = 2;
    float upsAdvBatteryTemperature = 0;                  // , tenths of C°
    float upsAdvSystemTemperature = 0;                   // , tenths of C°
    // The current utility line voltage in VAC.
    uint16_t upsAdvInputLineVoltage = UPS_RATED_INPUT_VOLTAGE;
    // The current utility line voltage in tenths of VAC
    uint16_t upsHighPrecInputLineVoltage = UPS_RATED_INPUT_VOLTAGE * 10;
    // The current input frequency to the UPS system in tenths of Hz
    uint16_t upsHighPrecInputFrequency = UPS_RATED_INPUT_FREQ * 10;
    // The output voltage of the UPS system in VAC
    uint16_t upsAdvOutputVoltage = UPS_RATED_OUTPUT_VOLTAGE;
    // The output voltage of the UPS system in tenths of VAC
    uint16_t upsHighPrecOutputVoltage = UPS_RATED_OUTPUT_VOLTAGE * 10;
    // The output frequency in 0.1 Hertz, or -1 if it's unsupported by this UPS
    uint8_t upsAdvOutputFrequency = UPS_RATED_OUTPUT_FREQ;
    // The nominal output voltage from the UPS in VAC
    uint8_t upsAdvConfigRatedOutputVoltage = UPS_RATED_OUTPUT_VOLTAGE;
    // The minimum line voltage in VAC allowed before the UPS system transfers to battery backup
    uint8_t upsAdvConfigLowTransferVolt = UPS_RATED_LOW_INPUT_VOLTAGE_THRESHOLD;
    // The maximum line voltage in VAC allowed before the UPS system transfers to battery backup
    uint8_t upsAdvConfigHighTransferVolt = UPS_RATED_HIGH_INPUT_VOLTAGE_THRESHOLD;
    // The reason for the occurrence of the last transfer to UPS battery power
    uint8_t upsAdvInputLineFailCause = 1;
    // The sensitivity of the UPS to utility line abnormalities or noises
    uint8_t upsAdvConfigSensitivity = 1;
    // The results of the last runtime calibration
    uint8_t upsAdvTestCalibrationResults = 1;
    // Indicates whether the UPS batteries need replacing
    // uint8_t upsAdvBatteryReplaceIndicator = 1;          // Always NOT
    // The current UPS load expressed in (?) tenths of percent of rated capacity
    uint8_t upsAdvOutputLoad = 0;
    // The remaining battery capacity expressed in percent of full capacity
    uint8_t upsAdvBatteryCapacity = 99;
    // The minimum battery capacity required before the UPS will return from a low battery shutdown condition
    uint8_t getAdvConfigMinReturnCapacity = 5;
    // Setting this variable to turnUpsOffToConserveBattery(2) causes a UPS on battery to be put into 'sleep' mode
    uint8_t upsBasicControlConserveBattery = 1;
    // The UPS battery run time remaining before battery exhaustion (seconds)
    // const uint16_t upsAdvBatteryRunTimeRemaining = UPS_RATED_BATTERY_RUNTIME_SEC;
    // The battery current in Amps
    uint8_t upsAdvBatteryCurrent = UPS_RATED_BATTERY_AMPS_MAX;
    // The results of the last UPS diagnostics test performed
    uint8_t upsAdvTestDiagnosticsResults = 1;
    // Current battery status determined during the last diagnostic test
    uint8_t upsDiagBatteryStatus = 3;
} monitor_data_t;
extern monitor_data_t monitorData;

void val2str(float val, char * buffer, uint8_t prec = 2);
void val2str(int val, char * buffer);
void val2str(unsigned int val, char * buffer);
void val2str(long val, char * buffer);
void val2str(unsigned long val, char * buffer);

void str2val(char * buffer, long * val);
void str2val(char * buffer, unsigned long * val);
void str2val(char * buffer, int * val);
void str2val(char * buffer, unsigned int * val);

void pgm_str(const char * data, char * buffer, bool append = false);
// template <typename T> void val2hex(T n, char * b) {
//     char h[32] = "";
//     uint8_t i = 0;
//     while(n > 0) {
//         switch(n%16) {
//             case 10: h [i] = 'A'; break;
//             case 11: h [i] = 'B'; break;
//             case 12: h [i] = 'C'; break;
//             case 13: h [i] = 'D'; break;
//             case 14: h [i] = 'E'; break;
//             case 15: h [i] = 'F'; break;
//             default: h [i] = ( n%16 ) + 0x30;
//         }
//         n /= 16;
//         i++;
//     }
//     for(uint8_t a = (i-1); a >= 0; a--, b++) {
//         *b = h[a];
//     }
// };
void str2dt(const char * str, char * buffer);
void str2snmpdt(const char * str, char * buffer);
void dt2str(const char * dt, char * buffer);

#endif                                      // HELPERS_H
