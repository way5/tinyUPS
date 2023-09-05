/*
#####################################################################################
# File: eemem.cpp                                                                   #
# Project: tinyUPS                                                                  #
# File Created: Tuesday, 31st May 2022 8:50:35 pm                                   #
# Author: Sergey Ko                                                                 #
# Last Modified: Monday, 4th September 2023 12:23:36 pm                             #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

#include "eemem.h"

/**
 * @brief
 *
 */
bool eeMemClass::begin()
{
    if (!pref.begin(eepromFName, false))
    {
        __DL("(i) eemem init failed");
        return false;
    }
    return true;
}

/**
 * @brief
 *
 */
void eeMemClass::end()
{
    pref.end();
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
void eeMemClass::init()
{
    char *b;
    begin();
    size_t cfgS = 0;
    cfgS = pref.getBytesLength(eepromFName);
    if (!cfgS)
    {
        __DL("(!) eemem seems empty");
        // nvs erase partition
        pref.clear();
        pref.putBytes(eepromFName, &config, sizeof(config));
    }
    else
    {
        _CHB(b, cfgS);
        pref.getBytes(eepromFName, b, cfgS);
        memcpy(&config, reinterpret_cast<config_t *>(b), cfgS);
        _CHBD(b);
    }
    end();
}

/**
 * @brief Restore eemem factory defaults
 *
 */
void eeMemClass::restore()
{
    config_t cf;
    begin();
    pref.clear();
    pref.putBytes(eepromFName, &cf, sizeof(cf));
    end();
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool eeMemClass::commit()
{
    begin();
    pref.putBytes(eepromFName, &config, sizeof(config));
    end();
    return true;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setSSID(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.ssid, value) != 0)
    {
        memcpy(config.ssid, value, vs);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setSSIDKEY(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.ssidkey, value) != 0)
    {
        memcpy(config.ssidkey, value, vs);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setAdmLogin(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.admLogin, value) != 0)
    {
        memcpy(config.admLogin, value, vs);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setAdmPassw(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.admPassw, value) != 0)
    {
        strcpy(config.admPassw, value);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setNTPServer(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.ntpServer, value) != 0)
    {
        strcpy(config.ntpServer, value);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setNTPServerFB(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.ntpServerFB, value) != 0)
    {
        strcpy(config.ntpServerFB, value);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setNTPSyncInterval(uint16_t &value)
{
    if (config.ntpSyncInterval != value)
    {
        config.ntpSyncInterval = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setNTPTimeOffset(const char *value)
{
    size_t vs = strlen(value);
    int b = atoi(value);
    if (vs != 0 && config.ntpTimeOffset != b)
    {
        config.ntpTimeOffset = b;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setNTPDaylightOffset(const char *value)
{
    size_t vs = strlen(value);
    int32_t b = atoi(value);
    if (vs != 0 && config.ntpDaylightOffset != b)
    {
        config.ntpDaylightOffset = b;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setBatteryTempLT(float &value)
{
    if (config.batteryTempLT != value)
    {
        config.batteryTempLT = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setBatteryTempUT(float &value)
{
    if (config.batteryTempUT != value)
    {
        config.batteryTempUT = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setDeviceTempLT(float &value)
{
    if (config.deviceTempLT != value)
    {
        config.deviceTempLT = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setDeviceTempUT(float &value)
{
    if (config.deviceTempUT != value)
    {
        config.deviceTempUT = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setSNMPPort(uint16_t &value)
{
    if (config.snmpPort != value)
    {
        config.snmpPort = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setSNMPTrapPort(uint16_t &value)
{
    if (config.snmpTrapPort != value)
    {
        config.snmpTrapPort = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setUPSAdvConfigShutoffDelay(uint16_t &value)
{
    if (config.upsAdvConfigShutoffDelay != value)
    {
        config.upsAdvConfigShutoffDelay = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setUPSAdvConfigReturnDelay(uint16_t &value)
{
    if (config.upsAdvConfigReturnDelay != value)
    {
        config.upsAdvConfigReturnDelay = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setUPSAdvConfigLowBatteryRunTime(uint8_t &value)
{
    if (config.upsAdvConfigLowBatteryRunTime != value)
    {
        config.upsAdvConfigLowBatteryRunTime = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setBatteryLastReplaceDate(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.BatteryLastReplaceDate, value) != 0)
    {
        strcpy(config.BatteryLastReplaceDate, value);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setAuthTimeoutMax(uint16_t &value)
{
    if (config.authTimeoutMax != value)
    {
        config.authTimeoutMax = value;
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setGetCN(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.snmpGetCN, value) != 0)
    {
        strcpy(config.snmpGetCN, value);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setSetCN(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.snmpSetCN, value) != 0)
    {
        strcpy(config.snmpSetCN, value);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setTrapCN(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.snmpTrapCN, value) != 0)
    {
        strcpy(config.snmpTrapCN, value);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setSysContact(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.sysContact, value) != 0)
    {
        strcpy(config.sysContact, value);
        return true;
    }
    return false;
}

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
*/
bool eeMemClass::setSysLocation(const char *value)
{
    size_t vs = strlen(value);
    if (vs != 0 && strcmp(config.sysLocation, value) != 0)
    {
        strcpy(config.sysLocation, value);
        return true;
    }
    return false;
}