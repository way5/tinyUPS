/*
#####################################################################################
# File: ntpc.h                                                                      #
# Project: tinyUPS                                                                  #
# File Created: Monday, 6th June 2022 9:34:33 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 4th September 2023 10:59:39 am                             #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                      #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/
/* NOTE:
    ******** DEVELOPERs' MEMO

    1. print struct tm data via printf()
        %A	Full weekday name
        %B	Full month name
        %d	Day of the month
        %Y	Year
        %H	Hour in 24h format
        %I	Hour in 12h format
        %M	Minute
        %S	Second
    2. struct tm:
        tm_sec: seconds after the minute;
        tm_min: minutes after the hour;
        tm_hour: hours since midnight;
        tm_mday: day of the month;
        tm_year: years since 1900;
        tm_wday: days since Sunday;
        tm_yday: days since January 1;
        tm_isdst: Daylight Saving Time flag;

*/

#ifndef NTPCCLASS_H
#define NTPCCLASS_H

#include <sys/time.h>
#include "helpers.h"
#include "esp_sntp.h"

const char formatDatetime[] = "%d %b,%Y %T";
const uint32_t _sec_in_year = 31556952;
const uint32_t _sec_in_day = 86400;

class NTPClientClass {
    public:
        tm getTS();
        time_t getEpoch();
        status_t loop();
        void getDatetime(char * b, const char * format = nullptr);
        // returns uptime in hundreds of second
        unsigned long uptimeSNMP();
        void uptimeHR(char * buffer);
        unsigned long uptimeSeconds();
        void timestampToString(char * buffer);
        unsigned long getTimestamp();
        status_t forceUpdate();

    protected:
        time_t _last_update = 0;
        time_t _startTime = 0;
};

extern NTPClientClass ntp;

#endif                          // NTPCCLASS_H