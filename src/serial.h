/*
#####################################################################################
# File: serial.h                                                                    #
# Project: tinyUPS                                                                  #
# File Created: Friday, 10th June 2022 8:44:02 pm                                   #
# Author: Sergey Ko                                                                 #
# Last Modified: Wednesday, 2nd August 2023 7:03:03 pm                              #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                      #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#ifndef SERIAL_H
#define SERIAL_H

#include "helpers.h"
#include "FS.h"
#include "FFat.h"
#include "eemem.h"
#include "ntpc.h"
#include "monitor.h"
#include "agent.h"
#include "flog.h"

extern void setAP();
extern void setSTA();
extern void systemReboot();

extern fLogClass sysLog;

/**
 * @brief Visualize the FS structure
 *
 * @param dir
 * @param offset
*/
void printRoot(File dir, uint8_t offset = 4) {
    while (true) {
        File entry =  dir.openNextFile();
        if (!entry) {
            // no more files
            break;
        }
        // indent
        for (uint8_t i = 0; i < offset; i++) {
            __D(' ');
        }
        __D(entry.name());
        if (entry.isDirectory()) {
            __DL('/');
            printRoot(entry, offset + 2);
        } else {
            // print in columns
            uint8_t i = 0;
            while(i <= (17 - strlen(entry.name()))) {
                __D(' ');
                i++;
            }
            // files have sizes, directories do not
            __D(' ');
            __DF("%ub\n", entry.size());
        }
    }
}

/**
 * @brief
 *
*/
void loopSerial() {
    // Terminal
    if(Serial.available()) {
        char * buffer;
        _CHB(buffer, 64);
        // can implement args here, split on space
        String input = Serial.readString();
        if(input.length() != 0) {
            for(uint8_t i = 0; i < input.length(); i++) {
                if(input[i] == '\0'
                    || input[i] == '\r'
                        || input[i] == '\n') break;
                *(buffer+i) = input[i];
            }
            input.clear();

            __DF(PSTR("> cmd: %s\n\n"), buffer);
            sysLog.putts(PSTR("(i) serial cmd: %s"), buffer);

            if(strncmp_P(buffer, PSTR("?"), 1) == 0)  {
                __DL(F("(i) config, configreset, wlstatus, printfs, freemem, dropauth, uptime, modeap, modesta, apkey, fanon, fanoff, battemp, reboot, genserial, id"));
            } else if(strncmp_P(buffer, PSTR("configreset"), 10) == 0)  {
                eemem.restore();
                systemReboot();
            } else if(strncmp_P(buffer, PSTR("uptime"), 6) == 0) {
                char uptime[68] = "";
                ntp.uptimeHR(uptime);
                __DL(uptime);
            } else if(strncmp_P(buffer, PSTR("dropauth"), 8) == 0) {
                if(strlen(session.authToken) != 0) {
                    __DF(PSTR("  targer: %s\n"), session.authToken);
                    memset(session.authToken, '\0', sizeof(session.authToken));
                    __DL(F("  done\n"));
                } else
                    __DL(F("  there's no session yet\n"));
            } else if(strncmp_P(buffer, PSTR("freemem"), 8) == 0) {
                multi_heap_info_t inf;
                heap_caps_get_info(&inf, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
                __DF(PSTR("  Total free bytes:  %.2f kb\n"), (inf.total_free_bytes/1000.0));     // total currently free in all non-continues blocks
                __DF(PSTR("  Minimum free:      %.2f kb\n"), (inf.minimum_free_bytes/1000.0));   // minimum free ever
                __DF(PSTR("  Largest free:      %.2f kb\n"), (inf.largest_free_block/1000.0));   // largest continues block to allocate big array
            } else if(strncmp_P(buffer, PSTR("reboot"), 5) == 0) {
                systemReboot();
            } else if(strncmp_P(buffer, PSTR("formatfs"), 9) == 0) {
                if(!FFat.format(false, (char *)("storage")))
                    __DL(F("(!) error storage format"));
                else
                    __DL("\n  format done\n");
            } else if(strncmp_P(buffer, PSTR("printfs"), 7) == 0) {
                File dir = FFat.open("/");
                printRoot(dir, 1);
            } else if(strncmp_P(buffer, PSTR("modeap"), 6) == 0) {
                setAP();
            } else if(strncmp_P(buffer, PSTR("modesta"), 7) == 0) {
                setSTA();
            } else if(strlen(buffer) > 7 && strncmp_P(buffer, PSTR("apkey "), 6) == 0) {
                std::string input(buffer);
                std::string key = input.substr(6);
                if(key.length() < 12) {
                    __DL(F(" (i) AP key length must be 12+ symbols long"));
                } else {
                    strcpy(config.apkey, key.c_str());
                    eemem.commit();
                    __DL(F(" (i) AP key successfuly changed and saved"));
                }
            } else if(strncmp_P(buffer, PSTR("config"), 7) == 0) {
                __DF(" admLogin = %s\n admPassw = %s\n ssid = %s\n ssidkey = %s\n apkey = %s\n authTimeoutMax = %d\n ntpServer = %s\n ntpServerFB = %s\n" \
                    " ntpSyncInterval = %d\n ntpTimeOffset = %d\n ntpDaylightOffset = %d\n snmpGetCN = %s\n snmpSetCN = %s\n snmpTrapCN = %s\n" \
                    " snmpPort = %d\n snmpTrapPort = %d\n batteryTempLT = %.2f\n batteryTempUT = %.2f\n deviceTempLT = %.2f\n deviceTempUT = %.2f\n" \
                    " BatteryLastReplaceDate = %s\n sysContact = %s\n sysLocation = %s\n upsSerialNumber = %s\n upsAdvConfigLowBatteryRunTime = %d\n" \
                    " upsAdvConfigReturnDelay = %d\n upsAdvConfigShutoffDelay = %d\n",
                        config.admLogin,
                        config.admPassw,
                        config.ssid,
                        config.ssidkey,
                        config.apkey,
                        config.authTimeoutMax,
                        config.ntpServer,
                        config.ntpServerFB,
                        config.ntpSyncInterval,
                        config.ntpTimeOffset,
                        config.ntpDaylightOffset,
                        config.snmpGetCN,
                        config.snmpSetCN,
                        config.snmpTrapCN,
                        config.snmpPort,
                        config.snmpTrapPort,
                        config.batteryTempLT,
                        config.batteryTempUT,
                        config.deviceTempLT,
                        config.deviceTempUT,
                        config.BatteryLastReplaceDate,
                        config.sysContact,
                        config.sysLocation,
                        config.upsSerialNumber,
                        config.upsAdvConfigLowBatteryRunTime,
                        config.upsAdvConfigReturnDelay,
                        config.upsAdvConfigShutoffDelay
                    );
            } else if(strncmp_P(buffer, PSTR("wlstatus"), 9) == 0) {
                switch(WiFi.getMode()) {
                    case WIFI_MODE_AP:
                    case WIFI_MODE_APSTA:
                        __DF(PSTR(" (i) mode: AP ssid: %s IP: %s\n"), WiFi.softAPSSID().c_str(), (WiFi.softAPIP()).toString().c_str());
                        break;
                    case WIFI_MODE_STA:
                        __DF(PSTR(" (i) mode: STA MAC: %s IP: %s\n"), WiFi.macAddress().c_str(), (WiFi.localIP()).toString().c_str());
                        break;
                    default:
                        __DL(F(" (!) WiFi is in unknown state"));
                        break;
                }
            } else if(strncmp_P(buffer, PSTR("fanon"), 6) == 0) {
                __DL(F(" (i) switching cooler on"));
                monitor.coolingSwitchOn();
            } else if(strncmp_P(buffer, PSTR("fanoff"), 7) == 0) {
                __DL(F(" (i) switching cooler off"));
                monitor.coolingSwitchOff();
            } else if(strncmp_P(buffer, PSTR("battemp"), 8) == 0) {
                analog_thermistor_data_t * td = new analog_thermistor_data_t();
                float adcVal = monitor.readADCmV();
                monitor.mVtoCelsius(adcVal, td);
                __DF(PSTR("  Rth = %.2f Ohm(s)\n"), td->Rth);
                __DF(PSTR("  Temp = %.2f°C\n"), td->temp);
            } else if(strncmp_P(buffer, PSTR("genserial"), 10) == 0) {
                if(WiFi.status() != WL_CONNECTED) {
                    __DL(F("(!) internet connection required"));
                } else {
                    char * b;
                    uint16_t salt = random(10000, 0xFFFF);
                    _CHB(b, 64);
                    ntp.getDatetime(b, "%Y%m%d");
                    sprintf(config.upsSerialNumber, "%s.%d", b, salt);
                    _CHBD(b);
                    __DF(PSTR(" (i) ups serial number: %s\n"), config.upsSerialNumber);
                    eemem.commit();
                }
            } else if(strncmp_P(buffer, PSTR("id"), 2) == 0) {
                __DF(PSTR("   tinyUPS core v.%s, ui v.%s\n"), IdentFirmwareRevision, IdentWebUIRevision);
            } else {
                __DL(F(" .(o_0). "));   // tiny confused
            }
            __DL();
        }
        _CHBD(buffer);
    }
}

#endif                              // SERIAL_H
