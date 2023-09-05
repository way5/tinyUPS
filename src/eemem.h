/*
#####################################################################################
# File: eemem.h                                                                     #
# Project: tinyUPS                                                                  #
# File Created: Tuesday, 31st May 2022 8:50:42 pm                                   #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 4th September 2023 12:02:53 pm                             #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#ifndef EEMEMCLASS_H
#define EEMEMCLASS_H

#include "helpers.h"
#include <Preferences.h>

const char eepromFName[] = "eeprom";

/**
 * @brief
 *
*/
class eeMemClass {
    public:
        void init();
        bool commit();
        bool begin();
        void end();
        /**
         * @brief
         *       DO NOT FORGET TO BEGIN() A EEPROM OBJECT BEFORE TO UPDATE
         *
         * @param param
         * @param value
        */
        bool setSSID(const char * value);
        bool setSSIDKEY(const char * value);
        bool setAdmLogin(const char * value);
        bool setAdmPassw(const char * value);
        bool setNTPServer(const char * value);
        bool setNTPServerFB(const char * value);
        bool setNTPSyncInterval(uint16_t & value);
        bool setNTPTimeOffset(const char * value);
        bool setNTPDaylightOffset(const char * value);
        bool setBatteryTempLT(float & value);
        bool setBatteryTempUT(float & value);
        bool setDeviceTempLT(float & value);
        bool setDeviceTempUT(float & value);
        bool setSNMPPort(uint16_t & value);
        bool setSNMPTrapPort(uint16_t & value);
        bool setUPSAdvConfigShutoffDelay(uint16_t & value);
        bool setUPSAdvConfigReturnDelay(uint16_t & value);
        bool setUPSAdvConfigLowBatteryRunTime(uint8_t & value);
        bool setBatteryLastReplaceDate(const char * value);
        bool setAuthTimeoutMax(uint16_t & value);
        bool setGetCN(const char * value);
        bool setSetCN(const char * value);
        bool setTrapCN(const char * value);
        bool setSysContact(const char * value);
        bool setSysLocation(const char * value);
        /**
         * @brief
         *
        */
        void restore();

    protected:
        Preferences pref;
};

extern eeMemClass eemem;

#endif                  // EEMEMCLASS_H