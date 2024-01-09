/*
#####################################################################################
# File: ntpc.cpp                                                                    #
# Project: tinyUPS                                                                  #
# File Created: Monday, 6th June 2022 9:34:40 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 8th January 2024 6:06:09 pm                                #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                      #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "ntpc.h"

/**
 * @brief Use in main loop()
 *
 * @return true if sync with NTP successful
 * @return false if QRTC used
*/
status_t NTPClientClass::loop() {
    if (WiFi.status() == WL_CONNECTED) {
        if (this->_last_update == 0 || (millis() - this->_last_update >= (config.ntpSyncInterval * 1000UL))) {
            status_t s = OKAY;
            if(forceUpdate() != OKAY) {
                s = NTP_SYNC_ERR;
            }
            this->_last_update = millis();
            return s;
        }
    }
    return NTP_SYNC_INTL;
}

/**
 * @brief returns seconds since 1970s
 *
 * @return time_t
*/
time_t NTPClientClass::getEpoch() {
    struct tm timeinfo = getTS();
	return mktime(&timeinfo);
}

/**
 * @brief Do update NTP -> RTC
 *
*/
status_t NTPClientClass::forceUpdate() {
    configTime((config.ntpTimeOffset*3600UL), config.ntpDaylightOffset, config.ntpServer, config.ntpServerFB);
    struct tm timeinfo;
    // uint8_t cntr = 0;
    if(!getLocalTime(&timeinfo)) {
    #if DEBUG == 2
        __DL("(!) failed to retrieve local time\n");
    #endif
        return NTP_SYNC_ERR;
    }
#if DEBUG == 2
    __DF("%s update, tz(%i)\n", config.ntpServer, config.ntpTimeOffset);
#endif
    struct timeval tval;
    time_t timeSinceEpoch = mktime(&timeinfo);
    // Y2K2036 See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/api-reference/system/system_time.html#year-2036-and-2038-overflow-issues
    tval.tv_sec = timeSinceEpoch;               // epoch time (seconds)
    tval.tv_usec = 0;
    if(_startTime == 0) {
        _startTime = timeSinceEpoch - (millis()/1000UL);
    }
    settimeofday(&tval, NULL);
    return OKAY;
}

/**
 * @brief Formatted date & time string or "-" if is not synchronized with NTP
 *
 * @param b
 * @param format
*/
void NTPClientClass::getDatetime(char *b, const char * format) {
    if(WiFi.status() == WL_CONNECTED) {
        struct tm timeinfo = getTS();
        if(format == nullptr) {
            format = formatDatetime;
        }
        char * ds;
        _CHB(ds, 128);
        strftime(ds, 128, format, &timeinfo);
        strcpy(b, ds);
        _CHBD(ds);
    } else {
        strcpy(b, "_");
    }
}

/**
 * @brief Current timestamp to character array
 *
 * @param buffer
 */
void NTPClientClass::timestampToString(char *b) {
    if (WiFi.status() == WL_CONNECTED) {
        time_t e = this->getEpoch();
        sprintf(b, "%ld", e);
    } else {
        strcpy(b, "0");
    }
}

/**
 * @brief Returns timestamp in seconds or 0 if not syncronized
 *
 * @return unsigned long
*/
unsigned long NTPClientClass::getTimestamp() {
    if (WiFi.status() == WL_CONNECTED) {
        return (unsigned long)this->getEpoch();
    }
    return 0;
}

/**
 * @brief Returns device uptime in hundreds of second
 *
 * @return unsigned long
*/
unsigned long NTPClientClass::uptimeSNMP() {
    if (WiFi.status() == WL_CONNECTED) {
        return (unsigned long)((this->getEpoch() - _startTime)*100);
    }
    return (millis()/10);
}

/**
 * @brief Device uptime in human readable format
 *
 * @param buffer
 */
void NTPClientClass::uptimeHR(char *buffer) {
    uint8_t year = 0;
    uint8_t day = 0;
    uint8_t hour = 0;
    uint8_t min = 0;
    memset(buffer, '\0', sizeof(*buffer));
    time_t upt;
    if (WiFi.status() == WL_CONNECTED)
        upt = (getEpoch() - _startTime);
    else
        upt = millis()/1000;

    if (upt > _sec_in_year) {
        year = floor(upt / _sec_in_year);
        upt = upt % _sec_in_year;
        val2str(year, buffer);
        strcat(buffer, " year ");
    }
    if (upt > _sec_in_day) {
        day = floor(upt / _sec_in_day);
        upt = upt % _sec_in_day;
        val2str(day, buffer);
        strcat(buffer, " day ");
    }
    if (upt > 3600) {
        hour = floor(upt / 3600);
        upt = upt % 3600;
        val2str(hour, buffer);
        strcat(buffer, " hrs ");
    }
    if(upt > 60) {
        min = floor(upt / 60);
        upt = upt % 60;
        val2str(min, buffer);
        strcat(buffer, " min ");
    }
    if(upt != 0) {
        val2str(upt, buffer);
        strcat(buffer, " sec");
    }
}

/**
 * @brief Returns uptime in seconds
 *
 * @return unsigned long
*/
unsigned long NTPClientClass::uptimeSeconds() {
    if (WiFi.status() == WL_CONNECTED)
        return (unsigned long)(getEpoch() - _startTime);
    else
        return millis()/1000;
}

/**
 * @brief Returns local time data
 *
 * @return tm
*/
tm NTPClientClass::getTS() {
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);
    time_t tt = mktime (&timeinfo);
    struct tm * tn = localtime(&tt);
    return *tn;
}