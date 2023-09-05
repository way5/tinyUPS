/*
#####################################################################################
# File: serial.h                                                                    #
# Project: tinyUPS                                                                  #
# File Created: Friday, 10th June 2022 8:44:02 pm                                   #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 4th September 2023 9:49:08 pm                              #
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

extern fLogClass logsys;

/**
 * @brief Visualize the FS structure
 *
 * @param dir
 * @param offset
*/
void printRoot(File dir, uint8_t offset = 4) {
    while (true) {
        fs::File entry =  dir.openNextFile();
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
 * @brief Serial command responder
 *
*/
void serialLoop() {
    if(Serial.available()) {
        // can implement args here, split on space
        String buffer = Serial.readString();

        __DF("> cmd: %s\n\n", buffer.substring(0, buffer.length()).c_str());
        logsys.putts("(i) serial cmd: %s", buffer);

        if(buffer.startsWith("?")) {    // strncmp(buffer, "?", 1) == 0)  {
            __DL("(i) config, configreset, wlstatus, printfs, freemem, dropauth, uptime, modeap, modesta, setapkey, fanon, fanoff, battemp, reboot, genserial, id");
        } else if(buffer.startsWith("configreset")) { // strncmp(buffer, "configreset", 10) == 0)  {
            eemem.restore();
            systemReboot();
        } else if(buffer.startsWith("uptime")) {  // strncmp(buffer, "uptime", 6) == 0) {
            char uptime[68] = "";
            ntp.uptimeHR(uptime);
            __DL(uptime);
        } else if(buffer.startsWith("dropauth")) {     // strncmp(buffer, "dropauth", 8) == 0) {
            if(strlen(session.authToken) != 0) {
                __DF("  targer: %s\n", session.authToken);
                memset(session.authToken, '\0', sizeof(session.authToken));
                __DL("  done\n");
            } else
                __DL("  there's no session yet\n");
        } else if(buffer.startsWith("freemem")) {      // strncmp(buffer, "freemem", 8) == 0) {
            multi_heap_info_t inf;
            heap_caps_get_info(&inf, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            __DF("  Total free bytes:  %.2f kb\n", (inf.total_free_bytes/1000.0));     // total currently free in all non-continues blocks
            __DF("  Minimum free:      %.2f kb\n", (inf.minimum_free_bytes/1000.0));   // minimum free ever
            __DF("  Largest free:      %.2f kb\n", (inf.largest_free_block/1000.0));   // largest continues block to allocate big array
        } else if(buffer.startsWith("reboot")) { // strncmp(buffer, "reboot", 5) == 0) {
            systemReboot();
        } else if(buffer.startsWith("formatfs")) {          // strncmp(buffer, "formatfs", 9) == 0) {
            if(!FFat.format(false, (char *)("storage")))
                __DL("(!) error storage format");
            else
                __DL("\n  format done\n");
        } else if(buffer.startsWith("printfs")) {       // strncmp(buffer, "printfs", 7) == 0) {
            fs::File dir = FFat.open("/");
            printRoot(dir, 1);
        } else if(buffer.startsWith("modeap")) {       // strncmp(buffer, "modeap", 6) == 0) {
            setAP();
        } else if(buffer.startsWith("modesta")) {       // strncmp(buffer, "modesta", 7) == 0) {
            setSTA();
        } else if(buffer.startsWith("setapkey")) {       // strncmp(buffer, "setapkey", 8) == 0) {
            if(buffer.length() <= 9) {
                __DL(" (i) usage: setapkey yout_ap_key_length_12+");
                goto serial_loop_end;
            }
            String key = buffer.substring(buffer.lastIndexOf(' ')+1);
            if(key.length() < 12) {
                __DL(" (i) AP key length must be 12+ symbols long");
                goto serial_loop_end;
            }
            strcpy(config.apkey, key.substring(0, 32).c_str());
            eemem.commit();
            __DF(" (i) new AP key: %s\n", key.c_str());
            delay(1000);
            systemReboot();
        } else if(buffer.startsWith("config")) {       // strncmp(buffer, "config", 7) == 0) {
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
        } else if(buffer.startsWith("wlstatus")) {       // strncmp(buffer, "wlstatus", 9) == 0) {
            switch(WiFi.getMode()) {
                case WIFI_MODE_AP:
                case WIFI_MODE_APSTA:
                    __DF(" (i) mode: AP ssid: %s IP: %s\n", WiFi.softAPSSID().c_str(), (WiFi.softAPIP()).toString().c_str());
                    break;
                case WIFI_MODE_STA:
                    __DF(" (i) mode: STA MAC: %s IP: %s\n", WiFi.macAddress().c_str(), (WiFi.localIP()).toString().c_str());
                    break;
                default:
                    __DL(" (!) WiFi is in unknown state");
                    break;
            }
        } else if(buffer.startsWith("fanon")) {       // strncmp(buffer, "fanon", 6) == 0) {
            __DL(" (i) switching cooler on");
            monitor.coolingSwitchOn();
        } else if(buffer.startsWith("fanoff")) {       // strncmp(buffer, "fanoff", 7) == 0) {
            __DL(" (i) switching cooler off");
            monitor.coolingSwitchOff();
        } else if(buffer.startsWith("battemp")) {       // strncmp(buffer, "battemp", 8) == 0) {
            analog_thermistor_data_t * td = new analog_thermistor_data_t();
            float adcVal = monitor.readADCmV();
            monitor.mVtoCelsius(adcVal, td);
            __DF("  Rth = %.2f Ohm(s)\n", td->Rth);
            __DF("  Temp = %.2f°C\n", td->temp);
        } else if(buffer.startsWith("genserial")) {       // strncmp(buffer, "genserial", 10) == 0) {
            if(WiFi.status() != WL_CONNECTED) {
                __DL("(!) internet connection required");
                goto serial_loop_end;
            }
            char * b;
            uint16_t salt = random(10000, 0xFFFF);
            _CHB(b, 64);
            ntp.getDatetime(b, "%Y%m%d");
            sprintf(config.upsSerialNumber, "%s.%d", b, salt);
            _CHBD(b);
            __DF(" (i) ups serial number: %s\n", config.upsSerialNumber);
            eemem.commit();
        } else if(buffer.startsWith("id")) {       // strncmp(buffer, "id", 2) == 0) {
            __DF("   tinyUPS core v.%s, ui v.%s\n", IdentFirmwareRevision, IdentWebUIRevision);
        } else {
            __DL(" .(o_0). ");   // tiny confused
        }
serial_loop_end:
        __DL();
    }
}

#endif                              // SERIAL_H
