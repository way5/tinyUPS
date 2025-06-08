/*
#####################################################################################
# File: main.cpp                                                                    #
# File Created: Monday, 22nd May 2023 3:50:32 pm                                    #
# Author: Sergey Ko                                                                 #
# Last Modified: Sunday, 8th June 2025 12:00:45 am                                  #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

/* NOTE:
    ******** DEVELOPERs' MEMO

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

#if ((WIFI_RECONNECT_METHOD == 2) || (WIFI_RECONNECT_METHOD == 3))
unsigned long _last_connection_update = 0;
#endif
unsigned long _reboot_scheduled_at = 0;

config_t config;
session_t session;
eeMemClass eemem;
monitor_data_t monitorData;
common_event_t systemEvent;
MonitorClass monitor;
AgentClass snmpagent;
AsyncWebServer httpd(80);
fLogClass logsys(_sysLogPath, SYSTEM_LOG_SIZE);
fLogClass logsnmp(_snmpLogPath, SNMP_LOG_SIZE);
fLogClass logTempMon(_logTempMonPath, MTEMP_LOG_SIZE);
fLogClass logDataMon(_logDataMonPath, MDATA_LOG_SIZE);
NTPClientClass ntp;
wifi_event_id_t wifiEvtCon, wifiEvtDscon;

/**
 * @brief Custom system reset function
 *
 */
void systemReboot()
{
    if (_reboot_scheduled_at != 0 && _reboot_scheduled_at + 2000UL >= millis())
    {
        logsys.put("-- reboot --");
        snmpagent.kill();
        FFat.end();
        __DL("(!) reboot...");
        // hard reset - can leave some of the registers in the old state
        // which can lead to problems ESP.reset(), using soft-reboot instead
        ESP.restart();
    }
}

/**
 * @brief Setup timer for upcoming reboot
 *
 */
void scheduleReboot(bool now)
{
    if (_reboot_scheduled_at == 0)
    {
        _reboot_scheduled_at = now ? millis() + 2000UL : millis();
        __DL("(i) reboot has been scheduled in 2s...");
    }
}

/**
 * @brief Unified WiFi event handler
 *
 * @param e
 */
void wifiEventHandler(arduino_event_t *e)
{
    char *b;
    _CHB(b, 128);
    if (e->event_id == ARDUINO_EVENT_WIFI_AP_STACONNECTED)
    {
        sprintf(b, "%02X:%02X:%02X:%02X:%02X:%02X",
                e->event_info.wifi_ap_staconnected.mac[0],
                e->event_info.wifi_ap_staconnected.mac[1],
                e->event_info.wifi_ap_staconnected.mac[2],
                e->event_info.wifi_ap_staconnected.mac[3],
                e->event_info.wifi_ap_staconnected.mac[4],
                e->event_info.wifi_ap_staconnected.mac[5]);
#ifdef DEBUG
        __DF("(i) %s connected, %d total\n", b, WiFi.softAPgetStationNum());
#endif
        logsys.put("(i) %s connected, %d total", b, WiFi.softAPgetStationNum());
    }
    else if (e->event_id == ARDUINO_EVENT_WIFI_AP_STADISCONNECTED)
    {
        sprintf(b, "%02X:%02X:%02X:%02X:%02X:%02X",
                e->event_info.wifi_ap_stadisconnected.mac[0],
                e->event_info.wifi_ap_stadisconnected.mac[1],
                e->event_info.wifi_ap_stadisconnected.mac[2],
                e->event_info.wifi_ap_stadisconnected.mac[3],
                e->event_info.wifi_ap_stadisconnected.mac[4],
                e->event_info.wifi_ap_stadisconnected.mac[5]);
#ifdef DEBUG
        __DF("(i) %s disconnected, %d left\n", b, WiFi.softAPgetStationNum());
#endif
        logsys.put("%s disconnected, %d left", b, WiFi.softAPgetStationNum());
    }
    else if (e->event_id == ARDUINO_EVENT_WIFI_STA_CONNECTED)
    {
#ifdef DEBUG
        __DF("(i) connected to AP: %s\n", (char *)e->event_info.wifi_sta_connected.ssid);
#endif
        logsys.put("(i) connected to AP: %s", (char *)e->event_info.wifi_sta_connected.ssid);
        systemEvent.wifiAPConnectSuccess = true;
        systemEvent.wifiIsInAPMode = false;
    }
    else if (e->event_id == ARDUINO_EVENT_WIFI_STA_DISCONNECTED)
    {
#ifdef DEBUG
        __DF("(i) disconnected from AP: %s\n", (char *)e->event_info.wifi_sta_disconnected.ssid);
#endif
        // logsys.put("(i) disconnected from AP: %s", (char *)e->event_info.wifi_sta_disconnected.ssid);
        systemEvent.wifiAPConnectSuccess = false;
    }
    _CHBD(b);
}

/**
 * @brief Initialize AP for setup process
 *
 */
void setAP()
{
    char *_ssid;
    _CHB(_ssid, 24);
    long salt = random(1000UL, 9999UL);
    sprintf(_ssid, "%s.%ld", sysModel, salt);
#ifdef DEBUG
    __DL("(i) setting AP");
#endif
    // WiFi events
    wifiEvtCon = WiFi.onEvent(wifiEventHandler, ARDUINO_EVENT_WIFI_AP_STACONNECTED);
    wifiEvtDscon = WiFi.onEvent(wifiEventHandler, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    // IPAddress ip;
    // Running WiFi.disconnect() is to shut down a connection
    // to an access point that module may have automatically
    // made using previously saved credentials.
    WiFi.disconnect();
    // delay(100);
    WiFi.mode(WIFI_MODE_APSTA);
    // uint8_t mac[6];
    // TODO: WiFi.softAPmacAddress(mac);
    // WiFi.softAPConfig(ip, gw, subnet);
    WiFi.softAP(_ssid, config.apkey);
    // - Sets the max transmit power, in dBm. Values range from 0 to 20.5 [dBm]
    //   inclusive, and should be multiples of 0.25. This is essentially a thin
    //   wrapper around the SDK’s system_phy_set_max_tpw() api call.
    // WiFi.setOutputPower(20.0);
    _CHBD(_ssid);
#ifdef DEBUG
    __DL("(i) AP started");
#endif
    systemEvent.wifiIsInAPMode = true;
}

/**
 * @brief Waiting for a connection to be established,
 *        initialise AP if connection failed (WIFI_RECONNECT_METHOD == [1, 2])
 *
 * @return true - connection succeeded
 * @return false - connection failed
 */
bool waitSTA()
{
    // uint8_t cntr = 0;
#if ((WIFI_RECONNECT_METHOD == 2) || (WIFI_RECONNECT_METHOD == 3))
    _last_connection_update = millis();
#endif
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
#ifdef DEBUG
        __DF("(!) connect to AP failed, err: %d\n", WiFi.status());
#endif
        logsys.put("(!) connect to AP failed, err: %d\n", WiFi.status());
#if ((WIFI_RECONNECT_METHOD == 1) || (WIFI_RECONNECT_METHOD == 2))
        setAP();
#endif
        return false;
    }
    // initialize if it hasn't been activated afore
    if (!systemEvent.isActiveSnmpAgent)
    {
        snmpagent.init();
    }
    return true;
}

/**
 * @brief Normal operation mode (setup complete)
 *
 */
void setSTA()
{
#ifdef DEBUG
    __DL("(i) setting up STA");
#endif
    if (strlen(config.ssid) == 0)
    {
        __DL("(!) no ssid");
        return;
    }
    // WiFi events
    wifiEvtCon = WiFi.onEvent(wifiEventHandler, ARDUINO_EVENT_WIFI_STA_CONNECTED);
    wifiEvtDscon = WiFi.onEvent(wifiEventHandler, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    // Running WiFi.disconnect() is to shut down a connection
    // to an access point that module may have automatically
    // made using previously saved credentials.
    WiFi.disconnect();
    // delay(100);
    // beacon idle timeout
    // esp_wifi_set_inactive_time();
    // WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.mode(WIFI_MODE_STA);
    WiFi.setSleep(WIFI_PS_NONE);
#if WIFI_RECONNECT_METHOD == 4
    WiFi.setAutoReconnect(true);
#endif
    WiFi.begin(config.ssid, config.ssidkey);
    waitSTA();
}

/**
 * @brief
 *
 */
void setup()
{
    Serial.begin(SERIAL_BAUD);
    // init FS
    if (!FFat.begin(false, "", 10U, "storage"))
    {
        __DL("(!) storage mount failed");
        return;
    }
#if defined(DEBUG) && DEBUG != 6
    else
    {
        unsigned int totalBytes = FFat.totalBytes();
        unsigned int usedBytes = FFat.usedBytes();
        __DL("File sistem info:");
        __DF("  Total space:      %u byte\n", totalBytes);
        __DF("  Total space used: %u byte\n", usedBytes);
    }
#endif
    // checking if there are required directories
    if (!FFat.exists(_logDirPath) && !FFat.mkdir(_logDirPath))
    {
        __DL("(!) make log dir failed");
        return;
    }
    if (!FFat.exists(_dataDirPath) && !FFat.mkdir(_dataDirPath))
    {
        __DL("(!) make data dir failed");
        return;
    }
    systemEvent.isActiveFilesystem = true;
    // Config
    eemem.init();
    // assure all the logs are ready
    logsys.touch();
    logsnmp.touch();
    logTempMon.touch();
    logDataMon.touch();
    // monitor
    if (monitor.init() == OKAY)
    {
        __DL("(i) hardware monitor init done");
    }
    // WiFi
    if (strlen(config.ssid) == 0)
    {
        // Inform #1
        logsys.put("-- init AP --");
        setAP();
    }
    else
    {
        // Inform #1
        logsys.put("-- init STA --");
        setSTA();
    }
    // in both cases init web server
    httpdInit();
}

/**
 * @brief
 *
 */
void loop()
{
    if (!systemEvent.updateInProgress)
    {
        serialLoop();
        if (!systemEvent.isActiveFilesystem)
            return;
        if (ntp.loop() == OKAY)
        {
            logsys.putts("%s sync: tz(%i) dl(%i)", config.ntpServer, config.ntpTimeOffset, config.ntpDaylightOffset);
#if defined(DEBUG) && DEBUG != 6
            __DF("%s sync: tz(%i) dl(%ld)\n", config.ntpServer, config.ntpTimeOffset, config.ntpDaylightOffset);
#endif
        }
        // always query
        monitor.loop();
        // resolve connection issues
        if (systemEvent.wifiAPConnectSuccess)
        {
            snmpagent.loop();
        }
#if WIFI_RECONNECT_METHOD == 2
        else if (systemEvent.wifiIsInAPMode && strlen(config.ssid) != 0 && WiFi.softAPgetStationNum() == 0
            && (_last_connection_update == 0 || (millis() - _last_connection_update >= 120000UL)))
        {
            // testSTA();
            WiFi.mode(WIFI_MODE_STA);
            WiFi.reconnect();
            _last_connection_update = millis();
        }
#endif
#if ((WIFI_RECONNECT_METHOD == 1) || (WIFI_RECONNECT_METHOD == 2))
        else if (!systemEvent.wifiIsInAPMode)
        {
            WiFi.reconnect();
            waitSTA();
        }
#elif WIFI_RECONNECT_METHOD == 3
        else if (!systemEvent.wifiIsInAPMode && strlen(config.ssid) != 0
            && (_last_connection_update == 0 || (millis() - _last_connection_update >= 120000UL)))
        {
            WiFi.reconnect();
            waitSTA();
        }
#endif
        httpdLoop();
    }
    systemReboot();
}
