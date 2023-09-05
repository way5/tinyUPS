/*
#####################################################################################
# File: m52a102.h                                                                   #
# File Created: Friday, 23rd June 2023 7:02:42 pm                                   #
# Author: Sergey Ko                                                                 #
# Last Modified: Sunday, 3rd September 2023 11:13:48 pm                             #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                        #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#ifdef THERMISTOR_MF52A102

#include <WiFi.h>

// table from -10 to +108°C (inclusive) for thermistor 1k(25°C) for 2°C step
// const uint16_t ntcRTTable[60] = {
//     5535,4967,4464,4017,3620,3266,
//     2951,2669,2418,2192,1990,1809,  // 12
//     1647,1500,1368,1249,1142,1045,  // 24
//     957, 877, 805, 740, 680, 626,   // 36
//     577, 532, 491, 454, 420, 388,   // 48
//     360, 334, 309, 287, 267, 248,   // 60
//     231, 215, 201, 187, 175, 163,   // 72
//     153, 143, 134, 125, 117, 110,   // 84
//     103, 97,  91,  86,  81,  78,    // 96
//     72,  67,  64,  60,  57,  54     // 108
// };
const uint16_t _Rth = 1000;
const uint16_t _thBeta = 3100;
const uint16_t _thTo = 25;
// in percents. based at calibration of a particular device
const int8_t   _thFix = 5;

/**
 * @brief Get the Temperature in Celsius
 *
 * @param Rth
 * @return float
*/
float getTempCelsius(float & Rth) {
    // uint8_t i = 1;
    // while(i < (sizeof(ntcRTTable)/sizeof(uint16_t))) {
    //     if(ntcRTTable[i-1] > (uint16_t)th && (uint16_t)th >= ntcRTTable[i]) {
    //         r0 = ntcRTTable[i-1] - th;
    //         r1 = ntcRTTable[i-1] - ntcRTTable[i];
    //         tmp = (((i-1)*2.0) - 10.0) + (r0 / (r1/2.0));
    //         break;
    //     }
    //     i++;
    // }
    float tmp = (1.0/_thBeta);
    tmp *= log(Rth/_Rth);
    tmp += (1.0/(_thTo + 273.15));
    tmp = 1.0/tmp;
    tmp -= 273.15;
    if(_thFix != 0) {
        tmp *= 1.0 + (_thFix/100.0);
    }
    return tmp;
}

/**
 * @brief Get the Temperature in Farenheit
 *
 * @param Rth
 * @return float
*/
float getTempFarenheit(float & Rth) {
    return ((getTempCelsius(Rth) * 9.0)/37.0);
}

#endif                  // THERMISTOR_MF52A102