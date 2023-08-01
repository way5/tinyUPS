/*
#####################################################################################
# File: main.cpp                                                                    #
# File Created: Monday, 22nd May 2023 3:50:32 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 31st July 2023 5:50:52 pm                                  #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

/* NOTE:
    ******** DEVELOPERs' MEMO

    1. WiFi module connection errors/statuses:
        WL_CONNECTED -> err 3
        WL_NO_SSID_AVAIL -> err 1
        WL_CONNECT_FAILED -> err 4
        WL_WRONG_PASSWORD -> err 6
        WL_IDLE_STATUS -> err 0
        WL_DISCONNECTED -> err 7

*/

#include <sys/random.h>
#include "helpers.h"
#include "FS.h"
#include "FFat.h"
#include "eemem.h"
#include "httpd.h"
#include "serial.h"
#include "monitor.h"
#include "agent.h"
#include "flog.h"
#include "ntpc.h"

config_t config;
session_t session;
eeMemClass eemem;
monitor_data_t monitorData;
volatile common_event_t systemEvent;
MonitorClass monitor;
AgentClass snmpagent;
AsyncWebServer httpd(80);
fLogClass sysLog;
fLogClass snmpLog;
fLogClass monTempLog;
fLogClass monDataLog;
NTPClientClass ntp;
wifi_event_id_t wifiEvtCon, wifiEvtDscon;

static const char _sysLogPath[] PROGMEM = "/logs/sys";
static const char _snmpLogPath[] PROGMEM = "/logs/snmp";
static const char _monTempLogPath[] PROGMEM = "/logs/montmp";
static const char _monDataLogPath[] PROGMEM = "/logs/monbdta";

/**
 * @brief Custom system reset function
 *
*/
void systemReboot() {
    sysLog.put(F("-- reboot --"));
    snmpagent.kill();
    FFat.end();
    __DL(F("(!) reboot..."));
    // hard reset - can leave some of the registers in the old state
    // which can lead to problems ESP.reset(), using soft-reboot instead
    ESP.restart();
}

/**
 * @brief Unified WiFi event handler
 *
 * @param e
*/
void wifiEventHandler(arduino_event_t *e) {
    char * b;
    _CHB(b, 128);
    if(e->event_id == ARDUINO_EVENT_WIFI_AP_STACONNECTED) {
        sprintf_P(b, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"),
                e->event_info.wifi_ap_staconnected.mac[0],
                e->event_info.wifi_ap_staconnected.mac[1],
                e->event_info.wifi_ap_staconnected.mac[2],
                e->event_info.wifi_ap_staconnected.mac[3],
                e->event_info.wifi_ap_staconnected.mac[4],
                e->event_info.wifi_ap_staconnected.mac[5]);
        #ifdef DEBUG
        __DF(PSTR("(i) %s connected, %d total\n"), b, WiFi.softAPgetStationNum());
        #endif
        sysLog.put(PSTR("(i) %s connected, %d total"), b, WiFi.softAPgetStationNum());
    } else if(e->event_id == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED) {
        sprintf_P(b, PSTR("%02X:%02X:%02X:%02X:%02X:%02X"),
                    e->event_info.wifi_ap_stadisconnected.mac[0],
                    e->event_info.wifi_ap_stadisconnected.mac[1],
                    e->event_info.wifi_ap_stadisconnected.mac[2],
                    e->event_info.wifi_ap_stadisconnected.mac[3],
                    e->event_info.wifi_ap_stadisconnected.mac[4],
                    e->event_info.wifi_ap_stadisconnected.mac[5]);
        #ifdef DEBUG
        __DF(PSTR("(i) %s disconnected, %d left\n"), b, WiFi.softAPgetStationNum());
        #endif
        sysLog.put(PSTR("%s disconnected, %d left"), b, WiFi.softAPgetStationNum());
    } else if(e->event_id == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
        #ifdef DEBUG
        __DF(PSTR("(i) join STA: %s\n"), (char *)e->event_info.wifi_sta_connected.ssid);
        #endif
        sysLog.put(PSTR("(i) join STA: %s"), (char *)e->event_info.wifi_sta_connected.ssid);
    } else if(e->event_id == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
        #ifdef DEBUG
        __DF(PSTR("(i) leave STA: %s\n"), (char *)e->event_info.wifi_sta_disconnected.ssid);
        #endif
        // TODO: may use putts when NTP was syncronized
        sysLog.put(PSTR("(i) leave STA: %s"), (char *)e->event_info.wifi_sta_disconnected.ssid);
    }
    _CHBD(b);
}

/**
 * @brief Initialize AP for setup process
 *
*/
void setAP() {
    char * _ssid;
    _CHB(_ssid, 24);
    long salt = random(1000UL, 9999UL);
    sprintf(_ssid, "%s.%ld", sysModel, salt);
#ifdef DEBUG
    __DL(F("(i) setting AP"));
#endif
    // WiFi events
    wifiEvtCon = WiFi.onEvent(wifiEventHandler, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
    wifiEvtDscon = WiFi.onEvent(wifiEventHandler, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    // IPAddress ip;
    // Running WiFi.disconnect() is to shut down a connection
    // to an access point that module may have automatically
    // made using previously saved credentials.
    WiFi.disconnect();
    delay(1000);
    // - WIFI_MODE_NULL = 0, WIFI_MODE_STA = 1, WIFI_MODE_AP = 2, WIFI_MODE_APSTA = 3
    WiFi.mode(WIFI_MODE_APSTA);
    // uint8_t mac[6];
    // WiFi.softAPmacAddress(mac);
    // WiFi.softAPConfig(ip, gw, subnet);
    WiFi.softAP(_ssid, config.apkey);
    // - Sets the max transmit power, in dBm. Values range from 0 to 20.5 [dBm]
    //   inclusive, and should be multiples of 0.25. This is essentially a thin
    //   wrapper around the SDK’s system_phy_set_max_tpw() api call.
    // WiFi.setOutputPower(20.0);
    // do preliminary network scan
    WiFi.scanNetworks(true, false);
    _CHBD(_ssid);
    // init web server
    httpdInit();
#ifdef DEBUG
    __DL(F("(i) AP started"));
#endif
}

/**
 * @brief Normal operation mode (setup complete)
 *
*/
void setSTA() {
    uint8_t cntr = 0;
#ifdef DEBUG
    __DL(F("(i) setting up STA"));
#endif
    if(strlen(config.ssid) == 0) {
        __DL(F("(!) no ssid"));
        return;
    }
    // WiFi events
    wifiEvtCon = WiFi.onEvent(wifiEventHandler, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    wifiEvtDscon = WiFi.onEvent(wifiEventHandler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    // Running WiFi.disconnect() is to shut down a connection
    // to an access point that module may have automatically
    // made using previously saved credentials.
    WiFi.disconnect();
    delay(100);
    // - WIFI_MODE_NULL = 0, WIFI_MODE_STA = 1, WIFI_MODE_AP = 2, WIFI_MODE_APSTA = 3
    WiFi.mode(WIFI_MODE_STA);
    // - WIFI_PS_NONE, WIFI_PS_MIN_MODEM, WIFI_PS_MAX_MODEM
    WiFi.setSleep(WIFI_PS_NONE);
    WiFi.setAutoReconnect(true);
    WiFi.begin(config.ssid, config.ssidkey);

    while(WiFi.status() != WL_CONNECTED && cntr <= 60) {
        delay(500);
        feedLoopWDT();
        cntr++;
    }
    // check if connected
    if(WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG
        __DF(PSTR("(!) connect to AP failed, err: %d\n"), WiFi.status());
    #endif
        sysLog.put(PSTR("(!) connect to AP failed, err: %d\n"), WiFi.status());
        // systemEvent.wifiAPConnectFailed = true;
        setAP();
        return;
    }
    // initialize if it's not active afore
    if(!snmpagent.isActive()) {
        // if connected, init SNMP
        snmpagent.init();
        // init web server
        httpdInit();
    }
#ifdef DEBUG
    __DL(F("(i) STA started"));
#endif
}

/**
 * @brief
 *
*/
void setup() {
    // init serial
    Serial.begin(SERIAL_BAUD);
    delay(1000);

    enableLoopWDT();

    if(!FFat.begin(false, "", 10U, "storage")) {
        __DL(F("(!) storage mount failed"));
        return;
    }
#if defined(DEBUG) && DEBUG != 6
    else {
        unsigned int totalBytes = FFat.totalBytes();
        unsigned int usedBytes = FFat.usedBytes();
        __DL("File sistem info:");
        __DF("  Total space:      %u byte\n", totalBytes);
        __DF("  Total space used: %u byte\n", usedBytes);
    }
#endif
    // checking if there are required directories
    if(!FFat.exists(_logDirPath) && !FFat.mkdir(_logDirPath)) {
        __DL(F("(!) make log dir failed"));
        return;
    }
    if(!FFat.exists(_dataDirPath) && !FFat.mkdir(_dataDirPath)) {
        __DL(F("(!) make data dir failed"));
        return;
    }
    // Config
    eemem.init();
    // System log
    sysLog.init(_sysLogPath, SYSLOG_SIZE);
    // SNMP log
    snmpLog.init(_snmpLogPath, SNMPLOG_SIZE);
    // monitor
    if(monitor.init() == OKAY) {
        __DL(F("(i) hardware monitor init done"));
        // monitor temperature log
        monTempLog.init(_monTempLogPath, MONTMPLOG_SIZE);
        // monitor battery info log
        monDataLog.init(_monDataLogPath, MONDATALOG_SIZE);
    }
    // WiFi
    if(strlen(config.ssid) == 0) {
        // Inform #1
        sysLog.put(F("-- init AP --"));
        setAP();
    } else {
        // Inform #1
        sysLog.put(F("-- init STA --"));
        setSTA();
    }
}

/**
 * @brief main()
 *
*/
void loop() {
    if(ntp.loop() == OKAY) {
        sysLog.putts(PSTR("%s sync: tz(%i) dl(%i)"), config.ntpServer, config.ntpTimeOffset, config.ntpDaylightOffset);
    #if defined(DEBUG) && DEBUG != 6
        __DF(PSTR("%s sync: tz(%i) dl(%i)\n"), config.ntpServer, config.ntpTimeOffset, config.ntpDaylightOffset);
    #endif
    }
    loopSerial();
    if(WiFi.status() == WL_CONNECTED) {
        snmpagent.loop();
        monitor.loop();
    }
    httpdLoop();
}
