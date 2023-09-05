/*
#####################################################################################
# File: agent.cpp                                                                   #
# Project: tinyUPS                                                                  #
# File Created: Monday, 2nd December 2019 3:22:49 pm                                #
# Author: sk                                                                        #
# Last Modified: Monday, 4th September 2023 12:23:36 pm                             #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

/*NOTE:

    ******** DEVELOPERs' MEMO

    1.

*/

#include "agent.h"

// An SNMP trap is a type of SNMP protocol data unit (PDU).
// Unlike other PDU types, with an SNMP trap, an agent can send an
// unrequested message to the manager to notify about an important event.
// SNMPTrap *trap = new SNMPTrap(config.snmpTrapCN, SNMP_VERSION_2C);
uint8_t WORD_ALIGNED_ATTR _snmpPacketBuffeer[SNMP_MAX_PACKET_LENGTH] = {0};

static ValueCallback * cbGetName;
static ValueCallback * cbGetDescr;
static ValueCallback * cbGetVendor;
static ValueCallback * cbGetUptime;
static ValueCallback * cbGetContact;
static ValueCallback * cbGetLocation;
static ValueCallback * cbGetServ;
static ValueCallback * cbGetEntModel;
static ValueCallback * cbGetEntName;
static ValueCallback * cbGetEntSerial;
static ValueCallback * cbGetEntManufact;
static ValueCallback * cbEntControlUpsOff;
static ValueCallback * cbEntControlUpsOn;
static ValueCallback * cbEntControlSimulPFail;
static ValueCallback * cbEntControlFlashAndBeep;
static ValueCallback * cbEntControlBypassSw;
static ValueCallback * cbEntControlTestDiag;
static ValueCallback * cbEntControlRuntmCalibr;
static ValueCallback * cbEntControlPhaseResetVal;
static ValueCallback * cbEntBasicOutStatus;
static ValueCallback * cbEntBasicBattStatus;
static ValueCallback * cbEntAdvBattCap;
static ValueCallback * cbEntBattTemp;
static ValueCallback * cbEntHPrecInpLineVolt;
static ValueCallback * cbEntHPrecInputMaxLineVolt;
static ValueCallback * cbEntHPrecInpMinLineVolt;
static ValueCallback * cbEntAdvInputLineVolt;
static ValueCallback * cbEntHPrecIntFreq;
static ValueCallback * cbEntAdvOutputFreq;
static ValueCallback * cbEntConfigLowTransfVolt;
static ValueCallback * cbEntConfigHighTransfVolt;
static ValueCallback * cbEntInpLineFailCause;
static ValueCallback * cbEntAdvConfigSens;
static ValueCallback * cbEntAdvTestCalibrRes;
static ValueCallback * cbEntAdvBattReplIndicator;
static ValueCallback * cbEntAdvConfigMinRetCap;
static ValueCallback * cbEntBasicCtrlConservBatt;
static ValueCallback * cbEntAdvBattActVolt;
static ValueCallback * cbEntAdvBattnominalVolt;
static ValueCallback * cbEntAdvBattCurrent;
static ValueCallback * cbEntBattNumPacks;
static ValueCallback * cbEntNumBadBattPacks;
static ValueCallback * cbEntAdvTestDiagRes;
static ValueCallback * cbEntPhaseOutFrec;
static ValueCallback * cbEntPhaseNumOutPhase;
static ValueCallback * cbEntAdvConficRateOutVolt;
static ValueCallback * cbEntAdvOutLoad;
static ValueCallback * cbEntAdvOutVolt;
static ValueCallback * cbEntHighPrecOutVolt;
static ValueCallback * cbEntAdvConfigShutDelay;
static ValueCallback * cbEntAdvConfigRetDelay;
static ValueCallback * cbEntAdvConfigLowBattRunTime;
static ValueCallback * cbEntAdvBattRunTimeRemng;
static ValueCallback * cbEntAdvIdentFirmwRev;
static ValueCallback * cbEntAdvTestLastDiagDate;
static ValueCallback * cbEntDiagBatteryStatus;

// Name
inline static char * getName() {
    char * buffer;
    _CHB(buffer, 24);
    strcpy(buffer, sysName);
    return buffer;
}

// Decription
inline static char * getDescr() {
    char * buffer;
    _CHB(buffer, 24);
    strcpy(buffer, sysDescr);
    return buffer;
}

// Vendor
inline static char * getVendor() {
    char * buffer;
    _CHB(buffer, 24);
    strcpy(buffer, sysName);
    return buffer;
}

// Uptime
inline static uint32_t getUptime() {
    return ntp.uptimeSNMP();
}

// Contact
inline static char * getContact() {
    return config.sysContact;
}

// Location
inline static char * getLocation() {
    return config.sysLocation;
}

// Service
inline static int getServiceOSI() {
    return serviceTypeOSI;
}

inline static char * snmpGetIdentSerialNumber() {
    return config.upsSerialNumber;
}

inline static char * snmpGetIdentDateOfManufact() {
    char * buffer;
    _CHB(buffer, 24);
    strcpy(buffer, IdentDateOfManufact);
    return buffer;
}

inline static char * getAdvIdentFirmwareRevision() {
    char * buffer;
    _CHB(buffer, 24);
    str2snmpdt(config.BatteryLastReplaceDate, buffer);
    return buffer;
}

inline static char * getAdvTestLastDiagnosticsDate() {
    char * buffer;
    _CHB(buffer, 24);
    str2snmpdt(config.BatteryLastReplaceDate, buffer);
    return buffer;
}

inline static char * getBasicBatteryLastReplaceDate() {
    char * buffer;
    _CHB(buffer, 24);
    strcpy(buffer, IdentFirmwareRevision);
    return buffer;
}

inline static uint32_t snmpGetBattTemp() {
    return static_cast<uint32_t>((monitorData.upsAdvBatteryTemperature * 10));
}

inline static int getAdvControlUpsOff() {
    return static_cast<int>(monitorData.upsAdvControlUpsOff);
}

inline static void setAdvControlUpsOff(int val) {
    monitorData.upsAdvControlUpsOff = val;
}

inline static int getAdvControlTurnOnUPS() {
    return static_cast<int>(monitorData.upsAdvControlTurnOnUPS);
}

inline static void setAdvControlTurnOnUPS(int val) {
    monitorData.upsAdvControlTurnOnUPS = val;
}

inline static int getAdvControlSimulatePowerFail() {
    return static_cast<int>(monitorData.upsAdvControlSimulatePowerFail);
}

inline static void setAdvControlSimulatePowerFail(int val) {
    monitorData.upsAdvControlSimulatePowerFail = val;
}

inline static int getAdvControlFlashAndBeep() {
    return static_cast<int>(monitorData.upsAdvControlFlashAndBeep);
}
inline static void setAdvControlFlashAndBeep(int val) {
    monitorData.upsAdvControlFlashAndBeep = val;
}
inline static int getAdvControlBypassSwitch() {
    return static_cast<int>(monitorData.upsAdvControlBypassSwitch);
}
inline static void setAdvControlBypassSwitch(int val) {
    monitorData.upsAdvControlBypassSwitch = val;
}
inline static int getAdvTestDiagnostics() {
    return static_cast<int>(monitorData.upsAdvTestDiagnostics);
}
inline static void setAdvTestDiagnostics(int val) {
    monitorData.upsAdvTestDiagnostics = val;
}
inline static int getAdvTestRuntimeCalibration() {
    return static_cast<int>(monitorData.upsAdvTestRuntimeCalibration);
}
inline static void setAdvTestRuntimeCalibration(int val) {
    monitorData.upsAdvTestRuntimeCalibration = val;
}
inline static int getAdvTestDiagnosticsResults() {
    return static_cast<int>(monitorData.upsAdvTestDiagnosticsResults);
}
inline static int getPhaseResetMaxMinValues() {
    return static_cast<int>(monitorData.upsPhaseResetMaxMinValues);
}
inline static int getNotSupported() {
    // ... -1 if it's unsupported by this UPS
    return SNMP_GENERIC_ERROR;
}
inline static int getPhaseNumOutputPhases() {
    return 2;
}
inline static int getAdvConfigRatedOutputVoltage() {
    return static_cast<int>(monitorData.upsAdvConfigRatedOutputVoltage);
}
inline static void setPhaseResetMaxMinValues(int val) {
    monitorData.upsPhaseResetMaxMinValues = val;
}
inline static int getBasicOutputStatus() {
    return static_cast<int>(monitorData.upsBasicOutputStatus);
}
inline static void setBasicOutputStatus(int val) {
    monitorData.upsBasicOutputStatus = val;
}
inline static int getBasicBatteryStatus() {
    return static_cast<int>(monitorData.upsBasicBatteryStatus);
}
inline static void setBasicBatteryStatus(int val) {
    monitorData.upsBasicBatteryStatus = val;
}
inline static uint32_t getAdvBatteryCapacity() {
    return static_cast<uint32_t>(monitorData.upsAdvBatteryCapacity);
}
inline static uint32_t getHighPrecInputLineVoltage() {
    return static_cast<uint32_t>(monitorData.upsHighPrecInputLineVoltage);
}
inline static uint32_t getHighPrecInputMaxLineVoltage() {
    return static_cast<uint32_t>(monitorData.upsHighPrecInputLineVoltage);
}
inline static uint32_t getHighPrecInputMinLineVoltage() {
    return static_cast<uint32_t>(monitorData.upsHighPrecInputLineVoltage);
}
inline static uint32_t getAdvInputLineVoltage() {
    return static_cast<uint32_t>(monitorData.upsAdvInputLineVoltage);
}
inline static uint32_t getHighPrecInputFrequency() {
    return static_cast<uint32_t>(monitorData.upsHighPrecInputFrequency);
}
inline static uint32_t getAdvOutputFrequency() {
    return static_cast<uint32_t>(monitorData.upsAdvOutputFrequency);
}
inline static int getAdvConfigLowTransferVolt() {
    return static_cast<int>(monitorData.upsAdvConfigLowTransferVolt);
}
inline static int getAdvConfigHighTransferVolt() {
    return static_cast<int>(monitorData.upsAdvConfigHighTransferVolt);
}
inline static int getAdvInputLineFailCause() {
    return static_cast<int>(monitorData.upsAdvInputLineFailCause);
}
inline static int getAdvConfigSensitivity() {
    return static_cast<int>(monitorData.upsAdvConfigSensitivity);
}
inline static int getAdvTestCalibrationResults() {
    return static_cast<int>(monitorData.upsAdvTestCalibrationResults);
}
inline static int getAdvBatteryReplaceIndicator() {
    // return static_cast<int>(monitorData.upsAdvBatteryReplaceIndicator);
    return 1;
}
//
inline static int getAdvConfigMinReturnCapacity() {
    // return static_cast<int>(config.upsAdvConfigMinReturnCapacity);
    return 10;
}
inline static int getBasicControlConserveBattery() {
    return static_cast<int>(monitorData.upsBasicControlConserveBattery);
}
inline static void setBasicControlConserveBattery(int val) {
    monitorData.upsBasicControlConserveBattery = val;
}
inline static int getAdvBatteryActualVoltage() {
    // return static_cast<int>(monitorData.upsAdvBatteryActualVoltage);
    return 12;
}
// inline static int getAdvBatteryNominalVoltage() {
//     return static_cast<int>(monitorData.upsAdvBatteryActualVoltage);
// }
inline static int getAdvBatteryCurrent() {
    return static_cast<int>(monitorData.upsAdvBatteryCurrent);
}
inline static uint32_t getAdvOutputLoad() {
    return static_cast<uint32_t>(monitorData.upsAdvOutputLoad);
}
inline static uint32_t getAdvOutputVoltage() {
    return static_cast<uint32_t>(monitorData.upsAdvOutputVoltage);
}
inline static uint32_t getHighPrecOutputVoltage() {
    return static_cast<uint32_t>(monitorData.upsHighPrecOutputVoltage);
}
inline static uint32_t getAdvConfigShutoffDelay() {
    return static_cast<uint32_t>(config.upsAdvConfigShutoffDelay);
}
inline static uint32_t getAdvConfigReturnDelay() {
    return static_cast<uint32_t>(config.upsAdvConfigReturnDelay);
}
inline static uint32_t getAdvConfigLowBatteryRunTime() {
    return static_cast<uint32_t>(config.upsAdvConfigLowBatteryRunTime);
}
/**
 * @brief Get the Adv Battery Run Time Remaining object
 *
 * @return uint32_t
*/
inline static uint32_t getAdvBatteryRunTimeRemaining() {
    // using driver's definition. HUNDREDTHS of a second
    return (monitor.getUPSLifeTimeSeconds() * 100);
}
inline static int getDiagBatteryStatus() {
    return monitorData.upsDiagBatteryStatus;
}

/**
 * @brief Initializer for SNMP Agent
 * 
 * @return status_t 
*/
status_t AgentClass::init() {
    char *_oid;
    AgentClass::agents.push_back(this);
    if (!this->restartUDP())
    {
#ifdef DEBUG
        __DL("(!) snmp udp failed");
#endif
        return ERR;
    }

    _CHB(_oid, SNMP_BUFFER_SIZE);

    // handlers RFC
    compileOID(oidSystem, oidName, _oid);
    cbGetName = addStringHandler(_oid, getName);
    compileOID(oidSystem, oidDescr, _oid);
    cbGetDescr = addStringHandler(_oid, getDescr);
    compileOID(oidSystem, oidVendor, _oid);
    cbGetVendor = addStringHandler(_oid, getVendor);
    compileOID(oidSystem, oidUptime, _oid);
    cbGetUptime = addTimestampHandler(_oid, getUptime);
    compileOID(oidSystem, oidContact, _oid);
    cbGetContact = addStringHandler(_oid, getContact);
    compileOID(oidSystem, oidLocation, _oid);
    cbGetLocation = addStringHandler(_oid, getLocation);
    compileOID(oidSystem, oidServiceOSI, _oid);
    cbGetServ = addIntegerHandler(_oid, getServiceOSI);

    // handlers ENTERPRISE
    // Model
    compileOID(oidEnterprise, upsBasicIdentModel, _oid); // r / str
    cbGetEntModel = addStringHandler(_oid, getDescr);
    // Name
    compileOID(oidEnterprise, upsBasicIdentName, _oid); // r / str
    cbGetEntName = addStringHandler(_oid, getName);
    // Ident Serial Number
    compileOID(oidEnterprise, upsAdvIdentSerialNumber, _oid); // r / str
    cbGetEntSerial = addStringHandler(_oid, snmpGetIdentSerialNumber);
    // Ident Date of Manufacture (mm/dd/yy)
    compileOID(oidEnterprise, upsAdvIdentDateOfManufacture, _oid); // r / str
    cbGetEntManufact = addStringHandler(_oid, snmpGetIdentDateOfManufact);

    // UPS Control Begin

    // On/Off control
    compileOID(oidEnterprise, upsAdvControlUpsOff, _oid); // rw / int
    cbEntControlUpsOff = addIntegerHandler(_oid, getAdvControlUpsOff, setAdvControlUpsOff);
    // On/Off control
    compileOID(oidEnterprise, upsAdvControlTurnOnUPS, _oid); // rw / int
    cbEntControlUpsOn = addIntegerHandler(_oid, getAdvControlTurnOnUPS, setAdvControlTurnOnUPS);
    // Simulate Power Fail
    compileOID(oidEnterprise, upsAdvControlSimulatePowerFail, _oid); // rw / int
    cbEntControlSimulPFail = addIntegerHandler(_oid, getAdvControlSimulatePowerFail, setAdvControlSimulatePowerFail);
    // Flash and beep
    compileOID(oidEnterprise, upsAdvControlFlashAndBeep, _oid); // rw / int
    cbEntControlFlashAndBeep = addIntegerHandler(_oid, getAdvControlFlashAndBeep, setAdvControlFlashAndBeep);
    // Bypass mode
    compileOID(oidEnterprise, upsAdvControlBypassSwitch, _oid); // rw / int
    cbEntControlBypassSw = addIntegerHandler(_oid, getAdvControlBypassSwitch, setAdvControlBypassSwitch);
    // Test Diagnostics
    compileOID(oidEnterprise, upsAdvTestDiagnostics, _oid); // rw / int
    cbEntControlTestDiag = addIntegerHandler(_oid, getAdvTestDiagnostics, setAdvTestDiagnostics);
    // Perform Calibration
    compileOID(oidEnterprise, upsAdvTestRuntimeCalibration, _oid); // rw / int
    cbEntControlRuntmCalibr = addIntegerHandler(_oid, getAdvTestRuntimeCalibration, setAdvTestRuntimeCalibration);
    // Reset the maximum and minimum UPS values
    compileOID(oidEnterprise, upsPhaseResetMaxMinValues, _oid); // rw / int
    cbEntControlPhaseResetVal = addIntegerHandler(_oid, getPhaseResetMaxMinValues, setPhaseResetMaxMinValues);
    // UPS Control End

    // Current UPS Status
    compileOID(oidEnterprise, upsBasicOutputStatus, _oid); // r / int
    cbEntBasicOutStatus = addIntegerHandler(_oid, getBasicOutputStatus, setBasicOutputStatus);
    // Current Battery Status
    compileOID(oidEnterprise, upsBasicBatteryStatus, _oid); // r / int
    cbEntBasicBattStatus = addIntegerHandler(_oid, getBasicBatteryStatus, setBasicBatteryStatus);
    // Remaining Battery Capacity in %
    compileOID(oidEnterprise, upsAdvBatteryCapacity, _oid); // r / gauge
    cbEntAdvBattCap = addGaugeHandler(_oid, getAdvBatteryCapacity);
    // Battery Temperature
    compileOID(oidEnterprise, upsAdvBatteryTemperature, _oid); // r / gauge
    cbEntBattTemp = addGaugeHandler(_oid, snmpGetBattTemp);
    // Precision Input Voltage
    compileOID(oidEnterprise, upsHighPrecInputLineVoltage, _oid); // r / gauge
    cbEntHPrecInpLineVolt = addGaugeHandler(_oid, getHighPrecInputLineVoltage);
    // The maximum utility line voltage in tenths of VAC over the previous 1 minute period
    compileOID(oidEnterprise, upsHighPrecInputLineVoltage, _oid); // r / gauge
    cbEntHPrecInputMaxLineVolt = addGaugeHandler(_oid, getHighPrecInputMaxLineVoltage);
    // The minimum utility line voltage in tenths of VAC over the previous 1 minute period
    compileOID(oidEnterprise, upsHighPrecInputMinLineVoltage, _oid); // r / gauge
    cbEntHPrecInpMinLineVolt = addGaugeHandler(_oid, getHighPrecInputMinLineVoltage);
    // The current utility line voltage in VAC
    compileOID(oidEnterprise, upsAdvInputLineVoltage, _oid); // r / gauge
    cbEntAdvInputLineVolt = addGaugeHandler(_oid, getAdvInputLineVoltage);
    // The current input frequency to the UPS system in tenths of Hz
    compileOID(oidEnterprise, upsHighPrecInputFrequency, _oid); // r / gauge
    cbEntHPrecIntFreq = addGaugeHandler(_oid, getHighPrecInputFrequency);
    // The current output frequency of the UPS system in Hz
    compileOID(oidEnterprise, upsAdvOutputFrequency, _oid); // r / gauge
    cbEntAdvOutputFreq = addGaugeHandler(_oid, getAdvOutputFrequency);
    // The minimum line voltage in VAC allowed before the UPS system transfers to battery backup
    compileOID(oidEnterprise, upsAdvConfigLowTransferVolt, _oid); // rw / int
    cbEntConfigLowTransfVolt = addIntegerHandler(_oid, getAdvConfigLowTransferVolt);
    // The maximum line voltage in VAC allowed before the UPS system transfers to battery backup
    compileOID(oidEnterprise, upsAdvConfigHighTransferVolt, _oid); // rw / int
    cbEntConfigHighTransfVolt = addIntegerHandler(_oid, getAdvConfigHighTransferVolt);
    // The reason for the occurrence of the last transfer to UPS battery power
    compileOID(oidEnterprise, upsAdvInputLineFailCause, _oid); // r / int
    cbEntInpLineFailCause = addIntegerHandler(_oid, getAdvInputLineFailCause);
    // The sensitivity of the UPS to utility line abnormalities or noises
    compileOID(oidEnterprise, upsAdvConfigSensitivity, _oid); // rw / int
    cbEntAdvConfigSens = addIntegerHandler(_oid, getAdvConfigSensitivity);
    // The results of the last runtime calibration
    compileOID(oidEnterprise, upsAdvTestCalibrationResults, _oid); // r / int
    cbEntAdvTestCalibrRes = addIntegerHandler(_oid, getAdvTestCalibrationResults);
    // Indicates whether the UPS batteries need replacing
    compileOID(oidEnterprise, upsAdvBatteryReplaceIndicator, _oid); // r / int
    cbEntAdvBattReplIndicator = addIntegerHandler(_oid, getAdvBatteryReplaceIndicator);
    // The minimum battery capacity required before the UPS will return from a low battery shutdown condition
    compileOID(oidEnterprise, upsAdvConfigMinReturnCapacity, _oid); // rw / int
    cbEntAdvConfigMinRetCap = addIntegerHandler(_oid, getAdvConfigMinReturnCapacity);
    // Setting this variable to turnUpsOffToConserveBattery(2) causes a UPS on battery to be put into 'sleep' mode
    compileOID(oidEnterprise, upsBasicControlConserveBattery, _oid); // rw / int
    cbEntBasicCtrlConservBatt = addIntegerHandler(_oid, getBasicControlConserveBattery, setBasicControlConserveBattery);
    // The actual battery bus voltage in Volts
    compileOID(oidEnterprise, upsAdvBatteryActualVoltage, _oid); // r / int
    cbEntAdvBattActVolt = addIntegerHandler(_oid, getAdvBatteryActualVoltage);
    // The nominal battery voltage in Volts
    compileOID(oidEnterprise, upsAdvBatteryNominalVoltage, _oid); // r / int
    cbEntAdvBattnominalVolt = addIntegerHandler(_oid, getAdvBatteryActualVoltage);
    // The battery current in Amps
    compileOID(oidEnterprise, upsAdvBatteryCurrent, _oid); // r / int
    cbEntAdvBattCurrent = addIntegerHandler(_oid, getAdvBatteryCurrent);
    // The number of external battery packs connected to the UPS. If
    // the UPS does not use smart cells then the agent reports ERROR_NO_SUCH_NAME
    compileOID(oidEnterprise, upsAdvBatteryNumOfBattPacks, _oid); // r / int
    cbEntBattNumPacks = addIntegerHandler(_oid, getNotSupported);
    // The number of external battery packs connected to the UPS that are defective.
    // If the UPS does not use smart cells then the agent reports ERROR_NO_SUCH_NAME
    compileOID(oidEnterprise, upsAdvBatteryNumOfBadBattPacks, _oid); // r / int
    cbEntNumBadBattPacks = addIntegerHandler(_oid, getNotSupported);
    // The results of the last UPS diagnostics test performed
    compileOID(oidEnterprise, upsAdvTestDiagnosticsResults, _oid); // r / int
    cbEntAdvTestDiagRes = addIntegerHandler(_oid, getAdvTestDiagnosticsResults);
    // The output frequency in 0.1 Hertz, or -1 if it's unsupported by this UPS
    compileOID(oidEnterprise, upsPhaseOutputFrequency, _oid); // r / int
    cbEntPhaseOutFrec = addIntegerHandler(_oid, getNotSupported);
    // The number of output phases utilized in this device
    compileOID(oidEnterprise, upsPhaseNumOutputPhases, _oid); // r / int
    cbEntPhaseNumOutPhase = addIntegerHandler(_oid, getPhaseNumOutputPhases);
    // The nominal output voltage from the UPS in VAC
    compileOID(oidEnterprise, upsAdvConfigRatedOutputVoltage, _oid); // rw / int
    cbEntAdvConficRateOutVolt = addIntegerHandler(_oid, getAdvConfigRatedOutputVoltage);
    // The current UPS load expressed in percent of rated capacity
    compileOID(oidEnterprise, upsAdvOutputLoad, _oid); // r / gauge
    cbEntAdvOutLoad = addGaugeHandler(_oid, getAdvOutputLoad);
    // The output voltage of the UPS system in VAC
    compileOID(oidEnterprise, upsAdvOutputVoltage, _oid); // r / gauge
    cbEntAdvOutVolt = addGaugeHandler(_oid, getAdvOutputVoltage);
    // The output voltage of the UPS system in tenths of VAC
    compileOID(oidEnterprise, upsHighPrecOutputVoltage, _oid); // r / gauge
    cbEntHighPrecOutVolt = addGaugeHandler(_oid, getHighPrecOutputVoltage);
    // The delay in seconds the UPS remains on after being told to turn off
    compileOID(oidEnterprise, upsAdvConfigShutoffDelay, _oid); // rw / timeTicks
    cbEntAdvConfigShutDelay = addTimestampHandler(_oid, getAdvConfigShutoffDelay);
    // The delay in seconds after utility line power returns before the UPS will turn on
    compileOID(oidEnterprise, upsAdvConfigReturnDelay, _oid); // rw / timeTicks
    cbEntAdvConfigRetDelay = addTimestampHandler(_oid, getAdvConfigReturnDelay);
    // The desired run time of the UPS, in seconds, once the low battery condition is reached.
    // During this time the UPS will produce a constant warning tone which can not be disabled.
    compileOID(oidEnterprise, upsAdvConfigLowBatteryRunTime, _oid); // rw / timeTicks
    cbEntAdvConfigLowBattRunTime = addTimestampHandler(_oid, getAdvConfigLowBatteryRunTime);
    // The UPS battery run time remaining before battery exhaustion
    compileOID(oidEnterprise, upsAdvBatteryRunTimeRemaining, _oid); // r / timeTicks
    cbEntAdvBattRunTimeRemng = addTimestampHandler(_oid, getAdvBatteryRunTimeRemaining);
    // The firmware revision of the UPS system's microprocessor
    compileOID(oidEnterprise, upsAdvIdentFirmwareRevision, _oid); // r / string
    cbEntAdvIdentFirmwRev = addStringHandler(_oid, getAdvIdentFirmwareRevision);
    // The date the last UPS diagnostics test was performed in mm/dd/yy format
    compileOID(oidEnterprise, upsAdvTestLastDiagnosticsDate, _oid); // r / string
    cbEntAdvTestLastDiagDate = addStringHandler(_oid, getAdvTestLastDiagnosticsDate);
    // The status of the battery
    compileOID(oidEnterprise, upsDiagBatteryStatus, _oid); // r / string
    cbEntDiagBatteryStatus = addIntegerHandler(_oid, getDiagBatteryStatus);

    _CHBD(_oid);

    // Ensure to sortHandlers after adding/removing
    // and OID callbacks - this makes snmpwalk work
    sortHandlers();

    systemEvent.isActiveSnmpAgent = true;

    __DL("(i) snmp init done");
    // snmp inform #1
    logsnmp.put("-- init at ports: %d / %d", config.snmpPort, config.snmpTrapPort);

    return OKAY;
}

/**
 * @brief
 *
*/
std::list<AgentClass*> AgentClass::agents = std::list<AgentClass*>();

/**
 * @brief Restart UDP listener
 *
 * @return true
 * @return false
*/
bool AgentClass::restartUDP() {
    snmpUDP.stop();
    return snmpUDP.begin(config.snmpPort);
};

/**
 * @brief Close all UDP ports
 *
*/
void AgentClass::kill() {
    snmpUDP.stop();
};

/**
 * @brief OID to character buffer _dest
 *
 * @param baseOID
 * @param specOID
 * @param _dest
*/
inline void AgentClass::compileOID(const char * baseOID, const char * specOID, char * dest) {
    memset(dest, '\0', SNMP_BUFFER_SIZE);
    strcpy(dest, baseOID);
    strcat(dest, specOID);
}


/**
 * @brief Check SNMP variables if any changes has occurred,
 *        then proceed with custom handlers
 *
 */
// void AgentClass::handleChanges()
// {

//     if (monitorData.upsAdvControlUpsOff != 1)
//     {

//         monitorData.upsAdvControlUpsOff = 1;
//     }
//     else if (monitorData.upsAdvControlTurnOnUPS != 2)
//     {

//         monitorData.upsAdvControlTurnOnUPS = 2;
//     }
//     else if (monitorData.upsAdvControlSimulatePowerFail != 1)
//     {

//         monitorData.upsAdvControlSimulatePowerFail = 1;
//     }
//     else if (monitorData.upsAdvControlBypassSwitch != 1)
//     {

//         monitorData.upsAdvControlBypassSwitch = 1;
//     }
//     else if (monitorData.upsAdvControlFlashAndBeep != 1)
//     {

//         monitorData.upsAdvControlFlashAndBeep = 1;
//     }
//     else if (monitorData.upsAdvTestDiagnostics != 1)
//     {

//         monitorData.upsAdvTestDiagnostics = 1;
//     }
//     else if (monitorData.upsPhaseResetMaxMinValues == 2)
//     {
//         // Reset the maximum and minimum UPS values (!CHECK DEFAULT VALUES!)
//         // monitorData.upsPhaseInputMaxVoltage = 1;
//         // monitorData.upsPhaseInputMinVoltage = 1;
//         // monitorData.upsPhaseInputMaxCurrent = 1;
//         // monitorData.upsPhaseInputMinCurrent = 1;
//         // monitorData.upsPhaseInputMaxPower = 1;
//         // monitorData.upsPhaseInputMinPower = 1;
//         // monitorData.upsPhaseOutputMaxCurrent = 1;
//         // monitorData.upsPhaseOutputMinCurrent = 1;
//         // monitorData.upsPhaseOutputMaxLoad = 1;
//         // monitorData.upsPhaseOutputMinLoad = 1;
//         // monitorData.upsPhaseOutputMaxPercentLoad = 1;
//         // monitorData.upsPhaseOutputMinPercentLoad = 1;
//         // monitorData.upsPhaseOutputMaxPower = 1;
//         // monitorData.upsPhaseOutputMinPower = 1;
//         // monitorData.upsPhaseOutputMaxPercentPower = 1;
//         // monitorData.upsPhaseOutputMinPercentPower = 1;

//         monitorData.upsPhaseResetMaxMinValues = 1;
//     }
// }

/**
 * @brief Call it from the main loop
 *
 * @return SNMP_ERROR_RESPONSE
*/
SNMP_ERROR_RESPONSE AgentClass::loop(){
    int packetLength = snmpUDP.parsePacket();
    if(packetLength > 0) {
        // __DF("received packet from: %s, of size: %d\n", _udp->remoteIP().toString().c_str(), packetLength);
    #if DEBUG == 5
        __DF("received packet from: %s, of size: %d\n", snmpUDP.remoteIP().toString().c_str(), packetLength);
    #endif
        if(packetLength < 0 || packetLength > SNMP_MAX_PACKET_LENGTH) {
            __DF("(!) incoming packet too large: %d\n", packetLength);
            logsnmp.putts("(!) incoming packet too large: %d", packetLength);
            return SNMP_REQUEST_TOO_LARGE;
        }

        memset(_snmpPacketBuffeer, 0, SNMP_MAX_PACKET_LENGTH);

        // int readBytes = _udp->read(_snmpPacketBuffeer, packetLength);
        int readBytes = snmpUDP.read(_snmpPacketBuffeer, packetLength);
        if(readBytes != packetLength){
            __DF("(!) packet length mismatch: expected: %d, actual: %d\n", packetLength, readBytes);
            logsnmp.putts("(!) packet length mismatch: expected: %d, actual: %d", packetLength, readBytes);
            return SNMP_REQUEST_INVALID;
        }

        int responseLength = 0;
        SNMP_ERROR_RESPONSE response = handlePacket(_snmpPacketBuffeer, packetLength, &responseLength,
                                                    /* SNMP_MAX_PACKET_LENGTH,  */this->callbacks,
                                                    /* _community, _readOnlyCommunity, */
                                                    informCallback, (void*)this);
        if(response > 0 && response != SNMP_INFORM_RESPONSE_OCCURRED){
            // send it
        #if DEBUG == 5
            __DF("sending response to: %s:%d\n", snmpUDP.remoteIP().toString().c_str(), snmpUDP.remotePort());
        #endif
            snmpUDP.beginPacket(snmpUDP.remoteIP(), snmpUDP.remotePort());
            snmpUDP.write(_snmpPacketBuffeer, responseLength);

            if(!snmpUDP.endPacket()) {
                __DL("(!) failed to send response packet");
                logsnmp.putts("(!) failed to send response for: %s", snmpUDP.remoteIP().toString().c_str());
            }
        }

        if(response == SNMP_SET_OCCURRED){
            setOccurred = true;
        }

        this->handleInformQueue();
        return response;
    }

    this->handleInformQueue();
    return SNMP_NO_PACKET;
}

/**
 * @brief
 *
*/
SortableOIDType* AgentClass::buildOIDWithPrefix(const char *oid) {
    std::string temp;
    temp.append(oid);
    SortableOIDType * newOid = new SortableOIDType(temp);
    if(newOid->valid){
        return newOid;
    }
#if DEBUG == 5
    __DF("cannot build. wrong oid: %s ?", temp.c_str());
#endif
    delete newOid;
    return nullptr;
}


/**
 * @brief
 *
*/
ValueCallback * AgentClass::addIntegerHandler(const char * oid, cbfGetInteger get, cbfSetInteger set) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new IntegerCallback(oidType, get, set);
    return this->addHandler(cb);
}

/**
 * @brief
 *
 * @param oid
 * @param get
 * @param set
 * @return ValueCallback*
*/
ValueCallback * AgentClass::addTimestampHandler(const char * oid, cbfGetTimestamp get, cbfSetTimestamp set) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new TimestampCallback(oidType, get, set);
    return this->addHandler(cb);
}

/**
 * @brief
 *
 * @param oid
 * @param buffer
 * @param max_len
 * @return ValueCallback*
*/
ValueCallback * AgentClass::addStringHandler(const char * oid, char * buffer, size_t max_len) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new StringCallback(oidType, &buffer, max_len);
    return this->addHandler(cb);
}

/**
 * @brief
 *
 * @param oid
 * @param get
 * @param set
 * @return ValueCallback*
*/
ValueCallback * AgentClass::addStringHandler(const char * oid, cbfGetString get, cbfSetString set) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new StringCallback(oidType, get, set);
    return this->addHandler(cb);
}

/**
 * @brief
 *
 * @param oid
 * @param get
 * @param set
 * @return ValueCallback*
*/
ValueCallback * AgentClass::addOpaqueHandler(const char * oid, cbfGetOpaque get, cbfSetOpaque set) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new OpaqueCallback(oidType, get, set);
    return this->addHandler(cb);
}

/**
 * @brief
 *
 * @param oid
 * @param get
 * @param set
 * @return ValueCallback*
*/
ValueCallback * AgentClass::addOIDHandler(const char * oid, cbfGetOID get, cbfSetOID set) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new OIDCallback(oidType, get, set);
    return this->addHandler(cb);
}

/**
 * @brief
 *
 * @param oid
 * @param get
 * @param set
 * @return ValueCallback*
*/
ValueCallback * AgentClass::addGaugeHandler(const char * oid,  cbfGetGauge get, cbfSetGauge set) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new Gauge32Callback(oidType, get, set);
    return this->addHandler(cb);
}

/**
 * @brief
 *
 * @param oid
 * @param get
 * @param set
 * @return ValueCallback*
*/
ValueCallback * AgentClass::addCounter32Handler(const char * oid, cbfGetCounter32 get, cbfSetCounter32 set) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new Counter32Callback(oidType, get, set);
    return this->addHandler(cb);
}

/**
 * @brief
 *
 * @param oid
 * @param get
 * @param set
 * @return ValueCallback*
*/
ValueCallback * AgentClass::addCounter64Handler(const char * oid, cbfGetCounter64 get, cbfSetCounter64 set) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    ValueCallback * cb = new Counter64Callback(oidType, get, set);
    return this->addHandler(cb);
}

/**
 * @brief
 *
*/
ValueCallback* AgentClass::addReadWriteStringHandler(const char *oid, char** value, size_t max_len, bool isSettable){
    if(!value || !*value) return nullptr;
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    return addHandler(new StringCallback(oidType, value, max_len)); //, isSettable);
}

/**
 * @brief
 *
*/
ValueCallback *AgentClass::addReadOnlyStaticStringHandler(const char *oid, const std::string& value) {
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    return addHandler(new ReadOnlyStringCallback(oidType, value)); //, false);
}

/**
 * @brief
 *
*/
ValueCallback* AgentClass::addReadOnlyIntegerHandler(const char *oid, int value){
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    return addHandler(new StaticIntegerCallback(oidType, value)); //, false);
}

ValueCallback* AgentClass::addDynamicIntegerHandler(const char *oid, GETINT_FUNC callback_func){
    if(!callback_func) return nullptr;
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) return nullptr;
    return addHandler(new DynamicIntegerCallback(oidType, callback_func)); //, false);
}

/**
 * @brief
 *
*/
ValueCallback* AgentClass::addDynamicReadOnlyTimestampHandler(const char *oid, GETUINT_FUNC callback_func){
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) {
        return nullptr;
    }
    return addHandler(new DynamicTimestampCallback(oidType, callback_func)); //, false);
}

/**
 * @brief
 *
*/
ValueCallback* AgentClass::addDynamicReadOnlyStringHandler(const char *oid, GETSTRING_FUNC callback_func){
    SortableOIDType* oidType = buildOIDWithPrefix(oid);
    if(!oidType) {
        return nullptr;
    }
    return addHandler(new DynamicStringCallback(oidType, callback_func)); //, false);
}

/**
 * @brief
 *
*/
ValueCallback * AgentClass::addHandler(ValueCallback *callback) {
// #if DEBUG == 5
//     __DL("adding new callback");
// #endif
    this->callbacks.push_back(callback);
    return callback;
}

bool AgentClass::removeHandler(ValueCallback* callback) {
    // this will remove the callback from the list and shift everything in
    // the list back so there are no gaps, this will not delete the actual callback
// #if DEBUG == 5
//     __DL("removing callback");
// #endif
    remove_handler(this->callbacks, callback);
    return true;
}

/**
 * @brief call this method every time adding
 *        or removing callbacks
 *
 * @return true
 * @return false
*/
bool AgentClass::sortHandlers(){
    sort_handlers(this->callbacks);
    return true;
}

/**
 * @brief
 *
 * @param trap
 * @param ip
 * @param replaceQueuedRequests
 * @param retries
 * @param delay_ms
 * @return snmp_request_id_t
*/
snmp_request_id_t AgentClass::sendTrapTo(SNMPTrap* trap, const IPAddress& ip, bool replaceQueuedRequests, int retries, int delay_ms){
    return queue_and_send_trap(this->informList, trap, ip, replaceQueuedRequests, retries, delay_ms);
}

/**
 * @brief
 *
 * @param ctx
 * @param requestID
 * @param responseReceiveSuccess
*/
void AgentClass::informCallback(void* ctx, snmp_request_id_t requestID, bool responseReceiveSuccess){
    if(!ctx) return;
    AgentClass* agent = static_cast<AgentClass*>(ctx);
    return inform_callback(agent->informList, requestID, responseReceiveSuccess);
}

/**
 * @brief
 *
*/
void AgentClass::handleInformQueue(){
    handle_inform_queue(this->informList);
}

/**
 * @brief
 *
 * @param trap
*/
void AgentClass::markTrapDeleted(SNMPTrap* trap){
    for(auto agent : AgentClass::agents){
        mark_trap_deleted(agent->informList, trap);
    }
}