/*
#####################################################################################
# File: agent.h                                                                     #
# Project: Smart station                                                            #
# File Created: Tuesday, 3rd December 2019 4:54:06 pm                               #
# Author: sk                                                                        #
# Last Modified: Tuesday, 1st August 2023 11:06:30 am                               #
# Modified By: Sergey Ko                                                            #
# License: GPL-3.0 (https://www.gnu.org/licenses/gpl-3.0.txt)                       #
#####################################################################################
# CHANGELOG:                                                                        #
#####################################################################################
*/

/* NOTE:
    ******** DEVELOPERs' MEMO

    *** RFC1155
    1. Gauge - This application-wide type represents a non-negative integer, which
    may increase or decrease, but which latches at a maximum value. This
    memo specifies a maximum value of 2^32-1 (4294967295 decimal) for
    gauges.
    2. TimeTicks - This application-wide type represents a non-negative integer which
    counts the time in HUNDREDTHS of a second since some epoch. When
    object types are defined in the MIB which use this ASN.1 type, the
    description of the object type identifies the reference epoch.
    3. IpAddress - This application-wide type represents a 32-bit internet address. It
    is represented as an OCTET STRING of length 4, in network byte-order.
    When this ASN.1 type is encoded using the ASN.1 basic encoding rules,
    only the primitive encoding form shall be used.

*/

#ifndef SNMP_AGENT_H
#define SNMP_AGENT_H

#include <list>
#include <deque>
#include <string>

#include "helpers.h"
#include "monitor.h"
#include "snmp/BER.h"
#include "snmp/VarBinds.h"
#include "snmp/defs.h"
#include "snmp/SNMPPacket.h"
#include "snmp/SNMPResponse.h"
#include "snmp/ValueCallbacks.h"
#include "snmp/SNMPParser.h"
#include "snmp/SNMPInform.h"
#include "snmp/SNMPTrap.h"
#include "ntpc.h"

static WiFiUDP snmpUDP;

const uint8_t serviceTypeOSI = 72;
const char sysName[] PROGMEM = "tinyUPS";
const char sysModel[] PROGMEM = "tinyUPS.01";
const char IdentFirmwareRevision[] PROGMEM = "v1.1.0";
const char sysDescr[] PROGMEM = "tinyUPS Invertor";
const char IdentDateOfManufact[] PROGMEM = "01/01/22";

// ███████╗████████╗ █████╗ ███╗   ██╗██████╗  █████╗ ██████╗ ██████╗
// ██╔════╝╚══██╔══╝██╔══██╗████╗  ██║██╔══██╗██╔══██╗██╔══██╗██╔══██╗
// ███████╗   ██║   ███████║██╔██╗ ██║██║  ██║███████║██████╔╝██║  ██║
// ╚════██║   ██║   ██╔══██║██║╚██╗██║██║  ██║██╔══██║██╔══██╗██║  ██║
// ███████║   ██║   ██║  ██║██║ ╚████║██████╔╝██║  ██║██║  ██║██████╔╝
// ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═════╝

// SNMP MIB2 System
const char oidSystem[] PROGMEM = ".1.3.6.1.2.1.1.";
/*
A textual description of the entity. This value
should include the full name and version
identification of the system's hardware type,
software operating-system, and networking
software. It is mandatory that this only contain
printable ASCII characters
*/
const char oidDescr[] PROGMEM = "1.0";      // r / str (0...255)
/*
The vendor's authoritative identification of the
network management subsystem contained in the
entity. This value is allocated within the SMI
enterprises subtree (1.3.6.1.4.1) and provides an
easy and unambiguous means for determining `what
kind of box' is being managed. For example, if
vendor `Flintstones, Inc.' was assigned the
subtree 1.3.6.1.4.1.4242, it could assign the
identifier 1.3.6.1.4.1.4242.1.1 to its `Fred Router'.
*/
const char oidVendor[] PROGMEM = "2.0";     // r / OID (str)
/*
The time (in hundredths of a second) since the network management portion of the system was last re-initialized
*/
const char oidUptime[] PROGMEM = "3.0";     // r / TimeTicks
/*
The textual identification of the contact person
for this managed node, together with information
on how to contact this person.
*/
const char oidContact[] PROGMEM = "4.0";    // rw / str (0...255)
/*
An administratively-assigned name for this
managed node. By convention, this is the node's
fully-qualified domain name
*/
const char oidName[] PROGMEM = "5.0";       // r / str (0...255)
/*
The physical location of this node (e.g.,`telephone closet, 3rd floor')
*/
const char oidLocation[] PROGMEM = "6.0";   // rw / str (0...255)
/*
A value which indicates the set of services that
this entity primarily offers.
The value is a sum. This sum initially takes the
value zero, Then, for each layer, L, in the range
1 through 7, that this node performs transactions
for, 2 raised to (L - 1) is added to the sum. For
example, a node which performs primarily routing
functions would have a value of 4 (2^(3-1)). In
contrast, a node which is a host offering
application services would have a value of 72
(2^(4-1) + 2^(7-1)). Note that in the context of
the Internet suite of protocols, values should be
calculated accordingly:

layer functionality
- 1 physical (e.g., repeaters)
- 2 datalink/subnetwork (e.g., bridges)
- 3 internet (e.g., IP gateways)
- 4 end-to-end (e.g., IP hosts)
- 7 applications (e.g., mail relays)

For systems including OSI protocols, layers 5 and
6 may also be counted.
*/
const char oidServiceOSI[] PROGMEM = "7.0"; // r / int (0...127)

// ███████╗███╗   ██╗████████╗███████╗██████╗ ██████╗ ██████╗ ██╗███████╗███████╗
// ██╔════╝████╗  ██║╚══██╔══╝██╔════╝██╔══██╗██╔══██╗██╔══██╗██║██╔════╝██╔════╝
// █████╗  ██╔██╗ ██║   ██║   █████╗  ██████╔╝██████╔╝██████╔╝██║███████╗█████╗
// ██╔══╝  ██║╚██╗██║   ██║   ██╔══╝  ██╔══██╗██╔═══╝ ██╔══██╗██║╚════██║██╔══╝
// ███████╗██║ ╚████║   ██║   ███████╗██║  ██║██║     ██║  ██║██║███████║███████╗
// ╚══════╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝  ╚═╝╚═╝╚══════╝╚══════╝

// UPS System
const char oidEnterprise[] PROGMEM = ".1.3.6.1.4.1.318.1.1.1.";

/*
The UPS model name (e.g. 'APC Smart-UPS 600').
*/
const char upsBasicIdentModel[] PROGMEM = "1.1.1.0";             // r / string
/*
An 8 byte ID string identifying the UPS. This objectcan
be set by the administrator
*/
const char upsBasicIdentName[] PROGMEM = "1.1.2.0";              // rw / string
/*
Setting this variable to turnUpsOff(2) causes
the UPS to shut off. When in this state, the UPS
will not provide output power regardless of the input
line state.
Setting this variable to turnUpsOffGracefully(3) causes
the UPS to shut off after a delay period. This allows the
host to shut down in a graceful manner. When in this state,
the UPS will not provide output power regardless of the
input line state.
Setting this value to noTurnUpsOff(1) has no
effect.
The value noTurnUpsOff(1) will always be returned
when the variable is read.
- noTurnUpsOff          (1),
- turnUpsOff            (2),
- turnUpsOffGracefully      (3),
- turnUpsSyncGroupOff       (4),
- turnUpsSyncGroupOffAfterDelay     (5),
- turnUpsSyncGroupOffGracefully     (6)
*/
const char upsAdvControlUpsOff[] PROGMEM = "6.2.1.0";            // rw / int /
/*
Setting this variable to turnOnUPS(2) causes the
UPS to be turned on immediately.
If this UPS is an active member of a Synchronized Control
Group (SCG), the turnOnUPSSyncGroup(3) command will perform
a Synchronized Turn On of all active Group members
regardless of their current AC output status.
Setting this value to noTurnOnUPS(1) has no
effect.
The value noTurnOnUPS(1) will always be returned
when the variable is read
- noTurnOnUPS           (1),
- turnOnUPS             (2),
- turnOnUPSSyncGroup    (3)
*/
const char upsAdvControlTurnOnUPS[] PROGMEM = "6.2.6.0";         // rw / int
/*
Setting this variable to turnUpsOffToConserveBattery(2)
causes a UPS on battery to be put into 'sleep' mode. The
UPS will turn back on when utility power is restored.
Attempting to turn off a UPS that is not on battery will
result in a badValue error.
Setting this value to noTurnOffUps(1) has no
effect.
The value noTurnOffUps(1) will always be returned
when the variable is read.
- noTurnOffUps 	(1),
- turnOffUpsToConserveBattery 	(2)
*/
const char upsBasicControlConserveBattery[] PROGMEM = "6.1.1.0"; // rw / int
/*
Setting this variable to simulatePowerFailure(2) causes
the UPS switch to battery power.
Setting this value to noSimulatePowerFailure(1) has no
effect.
The value noSimulatePowerFailure(1) will always be returned
when the variable is read.
- noSimulatePowerFailure 	(1),
- simulatePowerFailure 	(2)
*/
const char upsAdvControlSimulatePowerFail[] PROGMEM = "6.2.4.0"; // rw / int
/*
Setting this variable to flashAndBeep(2) causes the
UPS to beep and simultaneously turn on the UPS front
panel lights (Smart-UPS only).
If this UPS is an active member of a Synchronized Control
Group (SCG), the flashAndBeepSyncGroup(3) command will
Flash and Beep all active Group members regardless of
current AC output status.
Setting this value to noFlashAndBeep(1) has no
effect.
Setting this value to flashAndBeepCont (4) commandcauses
the UPS to beep and light the front panel lights until
the flashAndBeepCancel (5) command is received.
The value noFlashAndBeep(1) will always be returned
when the variable is read.
- noFlashAndBeep            (1),
- flashAndBeep              (2),
- flashAndBeepSyncGroup     (3),
- flashAndBeepCont          (4),
- flashAndBeepCancel        (5)
*/
const char upsAdvControlFlashAndBeep[] PROGMEM = "6.2.5.0";      // rw / int
/*
This switch puts the UPS in or out of bypass mode.
- noBypassSwitch            (1),
- switchToBypass            (2),
- switchOutOfBypass         (3)
*/
const char upsAdvControlBypassSwitch[] PROGMEM = "6.2.7.0";      // rw / int
/*
Setting this variable to testDiagnostics(2) causes
the UPS to perform a diagnostic self test.
Setting this value to noTestDiagnostics(1) has no
effect.
The value noTestDiagnostics(1) will always be returned
when the variable is read.
- noTestDiagnostics         (1),
- testDiagnostics           (2)
*/
const char upsAdvTestDiagnostics[] PROGMEM = "7.2.2.0";          // rw / int
/*
Setting this variable to performCalibration(2) causes
the UPS to discharge to calibrate the UPS.
The test will only start if the battery capacity is 100%.
The test runs until capacity is less than 25%.
Setting this variable to cancelCurrentCalibration(3)
after setting performCalibration(2) will cancel the
current discharge.
Setting this variable to noPerformCalibration(1)
will have no effect.
The value noPerformCalibration(1) will always be returned
when the variable is read.
The result of the calibration will be saved in
upsAdvTestCalibrationResult.
- noPerformCalibration 	(1),
- performCalibration 	(2),
- cancelCurrentCalibration 	(3)
*/
const char upsAdvTestRuntimeCalibration[] PROGMEM = "7.2.5.0";   // rw / int
/*
Reset the maximum and minimum UPS values:
upsPhaseInputMaxVoltage, upsPhaseInputMinVoltage,
upsPhaseInputMaxCurrent, upsPhaseInputMinCurrent,
upsPhaseInputMaxPower, upsPhaseInputMinPower,
upsPhaseOutputMaxCurrent, upsPhaseOutputMinCurrent,
upsPhaseOutputMaxLoad, upsPhaseOutputMinLoad,
upsPhaseOutputMaxPercentLoad, upsPhaseOutputMinPercentLoad,
upsPhaseOutputMaxPower, upsPhaseOutputMinPower,
upsPhaseOutputMaxPercentPower, upsPhaseOutputMinPercentPower
- none 	(1), *
- reset 	(2)
*/
const char upsPhaseResetMaxMinValues[] PROGMEM = "9.1.1.0";      // rw / int
/*
An 8-character string identifying the serial number of
the UPS internal microprocessor. This number is set at
the factory. NOTE: This number does NOT correspond to
the serial number on the rear of the UPS.
*/
const char upsAdvIdentSerialNumber[] PROGMEM = "1.2.3.0";        // r / str
/*
The date when the UPS was manufactured in mm/dd/yy (or yyyy) format.
*/
const char upsAdvIdentDateOfManufacture[] PROGMEM = "1.2.2.0";   // r / str
/*
The current utility line voltage in tenths of VAC
*/
const char upsHighPrecInputLineVoltage[] PROGMEM = "3.3.1.0";    // r / gauge
/*
The maximum utility line voltage in tenths of VAC over the
previous 1 minute period
*/
const char upsHighPrecInputMaxLineVoltage[] PROGMEM = "3.3.2.0"; // r / gauge
/*
The minimum utility line voltage in tenths of VAC over the
previous 1 minute period
*/
const char upsHighPrecInputMinLineVoltage[] PROGMEM = "3.3.3.0"; // r / gauge
/*
The current utility line voltage in VAC.
*/
const char upsAdvInputLineVoltage[] PROGMEM = "3.2.1.0";         // r / gauge
/*
The maximum utility line voltage in VAC over the
previous 1 minute period.
*/
const char upsAdvInputMaxLineVoltage[] PROGMEM = "3.2.2.0";      // r / gauge
/*
The minimum utility line voltage in VAC over the
previous 1 minute period.
*/
const char upsAdvInputMinLineVoltage[] PROGMEM = "3.2.3.0";      // r / gauge
/*
The number of input phases utilized in this device. The sum of all the upsPhaseNumInputPhases
variable indicates the number of rows in the input phase table
*/
const char upsPhaseNumInputPhases[] PROGMEM = "9.2.2.1.2.1"; // r / int
/*
The input voltage in VAC, or -1 if it's unsupported by this UPS
*/
const char upsPhaseInputVoltage[] PROGMEM = "9.2.3.1.3.1.1.1"; // r / int
// const char upsUnknown33[] PROGMEM = "9.2.3.1.3.1.1.2";
// const char upsUnknown34[] PROGMEM = "9.2.3.1.3.1.1.3";
/*
The maximum input voltage in VAC measured since the last reset (upsPhaseResetMaxMinValues),
or -1 if it's unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseInputMaxVoltage[] PROGMEM = "9.2.3.1.4.1.1.1"; // r / int
// const char upsUnknown36[] PROGMEM = "9.2.3.1.4.1.1.2";
// const char upsUnknown37[] PROGMEM = "9.2.3.1.4.1.1.3";
/*
The minimum input voltage in VAC measured since the last reset (upsPhaseResetMaxMinValues), or
-1 if it's unsupported by this UPS. Sampled every 30 seconds
*/
const char upsPhaseInputMinVoltage[] PROGMEM = "9.2.3.1.5.1.1.1"; // r / int
// const char upsUnknown39[] PROGMEM = "9.2.3.1.5.1.1.2";
// const char upsUnknown40[] PROGMEM = "9.2.3.1.5.1.1.3";
/*
The input current in 0.1 amperes, or -1 if it's unsupported by this UPS.
*/
const char upsPhaseInputCurrent[] PROGMEM = "9.2.3.1.6.1.1.1"; // r / int
// const char upsUnknown42[] PROGMEM = "9.2.3.1.6.1.1.2";
// const char upsUnknown43[] PROGMEM = "9.2.3.1.6.1.1.3";
/*
The maximum input current in 0.1 amperes measured since the last reset (upsPhaseResetMaxMinValues), or
-1 if it's unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseInputMaxCurrent[] PROGMEM = "9.2.3.1.7.1.1.1"; // r / int
// const char upsUnknown45[] PROGMEM = "9.2.3.1.7.1.1.2";
// const char upsUnknown46[] PROGMEM = "9.2.3.1.7.1.1.3";
/*
The minimum input current in 0.1 amperes measured since the last reset (upsPhaseResetMaxMinValues), or
-1 if it's unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseInputMinCurrent[] PROGMEM = "9.2.3.1.8.1.1.1"; // r / int
// const char upsUnknown48[] PROGMEM = "9.2.3.1.8.1.1.2";
// const char upsUnknown49[] PROGMEM = "9.2.3.1.8.1.1.3";
/*
The input frequency in 0.1 Hertz, or -1 if it's unsupported by this UPS.
*/
const char upsPhaseInputFrequency[] PROGMEM = "9.2.2.1.4.1";     // r / int
/*
The current input frequency to the UPS system in tenths of Hz
*/
const char upsHighPrecInputFrequency[] PROGMEM = "3.3.4.0";      // r / gauge
/*
The current input frequency to the UPS system in Hz
*/
const char upsAdvInputFrequency[] PROGMEM = "3.2.4.0";           // r / gauge
/*
The minimum line voltage in VAC allowed before the
UPS system transfers to battery backup.
For a list of allowed values supported by your UPS model,
see the UPS User's Manual.
If a value other than a supported value is provided in a
set request, the UPS interprets it as the next lower
acceptable value. If the provided value is lower than
the lowest acceptable value, the lowest acceptable
value is used
*/
const char upsAdvConfigLowTransferVolt[] PROGMEM = "5.2.3.0";    // rw / int
/*
The maximum line voltage in VAC allowed before the
UPS system transfers to battery backup.
For a list of allowed values supported by your UPS model,
see the UPS User's Manual.
If a value other than a supported value is provided in a
set request, the UPS interprets it as a the next higher
acceptable value. If the provided value is higher than
the highest acceptable value, the highest acceptable
value is used
*/
const char upsAdvConfigHighTransferVolt[] PROGMEM = "5.2.2.0";   // rw / int
/*
The reason for the occurrence of the last transfer to UPS
battery power. The variable is set to:
- noTransfer(1) -- if there is no transfer yet.
- highLineVoltage(2) -- if the transfer to battery is caused
by an over voltage greater than the high transfer voltage.
- brownout(3) -- if the duration of the outage is greater than
five seconds and the line voltage is between 40% of the
rated output voltage and the low transfer voltage.
- blackout(4) -- if the duration of the outage is greater than five
seconds and the line voltage is between 40% of the rated
output voltage and ground.
- smallMomentarySag(5) -- if the duration of the outage is less
than five seconds and the line voltage is between 40% of the
rated output voltage and the low transfer voltage.
- deepMomentarySag(6) -- if the duration of the outage is less
than five seconds and the line voltage is between 40% of the
rated output voltage and ground. The variable is set to
- smallMomentarySpike(7) -- if the line failure is caused by a
rate of change of input voltage less than ten volts per cycle.
- largeMomentarySpike(8) -- if the line failure is caused by
a rate of change of input voltage greater than ten volts per cycle.
- selfTest(9) -- if the UPS was commanded to do a self test.
- rateOfVoltageChange(10) -- if the failure is due to the rate of change of
the line voltage
- noTransfer 	(1),
- highLineVoltage 	(2),
- brownout 	(3),
- blackout 	(4),
- smallMomentarySag 	(5), *
- deepMomentarySag 	(6),
- smallMomentarySpike 	(7),
- largeMomentarySpike 	(8),
- selfTest 	(9),
- rateOfVoltageChange 	(10)
*/
const char upsAdvInputLineFailCause[] PROGMEM = "3.2.5.0";       // r / int
/*
The sensitivity of the UPS to utility line abnormalities
or noises
- auto          (1),
- low           (2),
- medium        (3),
- high          (4)
*/
const char upsAdvConfigSensitivity[] PROGMEM = "5.2.7.0";        // rw / int
/*
The current state of the UPS. If the UPS is unable to
determine the state of the UPS this variable is set
to unknown(1).
- unknown                   (1), *
- onLine                    (2),
- onBattery                 (3),
- onSmartBoost              (4),
- timedSleeping             (5),
- softwareBypass            (6),
- off                       (7),
- rebooting                 (8),
- switchedBypass            (9),
- hardwareFailureBypass     (10),
- sleepingUntilPowerReturn 	(11),
- onSmartTrim               (12),
- ecoMode                   (13),
- hotStandby                (14),
- onBatteryTest             (15)
*/
const char upsBasicOutputStatus[] PROGMEM = "4.1.1.0";           // r / int
/*
The status of the ups batteries. A batteryLow(3) value
indicates the UPS will be unable to sustain the current
load, and its services will be lost if power is not restored.
The amount of run time in reserve at the time of low battery
can be configured by the upsAdvConfigLowBatteryRunTime.
A batteryInFaultCondition(4)value indicates that a battery
installed has an internal error condition
- unknown                   (1), *
- batteryNormal             (2),
- batteryLow                (3),
- batteryInFaultCondition   (4)
*/
const char upsBasicBatteryStatus[] PROGMEM = "2.1.1.0";          // r / int
/*
The results of the last runtime calibration.
Value ok(1) means a successful runtime calibration.
Value invalidCalibration(2) indicates last calibration did
not take place since the battery capacity was below
100%.
Value calibrationInProgress(3) means a calibration
is occurring now
- ok 	(1),
- invalidCalibration 	(2),
- calibrationInProgress 	(3)
*/
const char upsAdvTestCalibrationResults[] PROGMEM = "7.2.6.0";   // r / int
/*
Indicates whether the UPS batteries need replacing
- noBatteryNeedsReplacing 	(1),
- batteryNeedsReplacing 	(2)
*/
const char upsAdvBatteryReplaceIndicator[] PROGMEM = "2.2.4.0";  // r / int
/*
The current internal UPS temperature expressed in
tenths of degrees Celsius
*/
//  const char upsHighPrecBatteryTemperature[] PROGMEM = "2.3.2.0"; // r / gauge
/*
The current internal UPS temperature expressed in Celsius
*/
const char upsAdvBatteryTemperature[] PROGMEM = "2.2.2.0";       // r / gauge
/*
The current UPS load expressed in tenths of percent of rated capacity
*/
//  const char upsHighPrecOutputLoad[] PROGMEM = "4.3.3.0"; // r / gauge
/*
The current UPS load expressed in percent of rated capacity
*/
const char upsAdvOutputLoad[] PROGMEM = "4.2.3.0";               // r / gauge
/*
The firmware revision of the UPS system's microprocessor
*/
const char upsAdvIdentFirmwareRevision[] PROGMEM = "1.2.1.0";    // r / string
/*
The delay in seconds the UPS remains on after being told
to turn off.
For a list of allowed values supported by your UPS model,
see the UPS User's Manual.
If a value other than a supported value is provided in a
set request, the UPS interprets it as a the next higher
acceptable value. If the provided value is higher than
the highest acceptable value, the highest acceptable
value is used
*/
const char upsAdvConfigShutoffDelay[] PROGMEM = "5.2.10.0";      // rw / timeTicks
/*
The delay in seconds after utility line power returns
before the UPS will turn on. This value is also used
when the UPS comes out of a reboot and before the UPS
wakes up from 'sleep' mode.
For a list of allowed values supported by your UPS model,
see the UPS User's Manual.
If a value other than a supported value is provided in a
set request, the UPS interprets it as a the next higher
acceptable value. If the provided value is higher than
the highest acceptable value, the highest acceptable
value is used
*/
const char upsAdvConfigReturnDelay[] PROGMEM = "5.2.9.0";        // rw / timeTicks
/*
The remaining battery capacity expressed in
tenths of percent of full capacity
*/
//  const char upsHighPrecBatteryCapacity[] PROGMEM = "2.3.1.0"; // r / gauge
/*
The remaining battery capacity expressed in percent of full capacity
*/
const char upsAdvBatteryCapacity[] PROGMEM = "2.2.1.0";          // r / gauge
/*
The minimum battery capacity required before the UPS will
return from a low battery shutdown condition. The capacity is
measured from 0% battery capacity (or Low Battery) as a percent
of full capacity (100%). In other words, the UPS will not re-energize
the output until the battery has charged so that its' capacity is equal
to this value.
For a list of allowed values supported by your UPS model,
see the UPS User's Manual.
If a value other than a supported value is provided in a
set request, the UPS interprets it as a the next higher
acceptable value. If the provided value is higher than
the highest acceptable value, the highest acceptable
value is used
*/
const char upsAdvConfigMinReturnCapacity[] PROGMEM = "5.2.6.0";  // rw / int
/*
The UPS battery run time remaining before battery exhaustion
*/
const char upsAdvBatteryRunTimeRemaining[] PROGMEM = "2.2.3.0";  // r / timeTicks
/*
The desired run time of the UPS, in seconds, once the
low battery condition is reached. During this time the UPS will
produce a constant warning tone which can not be disabled.
For a list of allowed values supported by your UPS model,
see the UPS User's Manual.
If a value other than a supported value is provided in a set
request, the UPS interprets the value as the next higher
acceptable value. If the provided value is higher than the
highest acceptable value, the highest acceptable value is used
*/
const char upsAdvConfigLowBatteryRunTime[] PROGMEM = "5.2.8.0";  // rw / timeTicks
/*
The actual battery bus voltage in tenths of Volts
*/
//  const char upsHighPrecBatteryActualVoltage[] PROGMEM = "2.3.4.0"; // r / int
/*
The actual battery bus voltage in Volts
*/
const char upsAdvBatteryActualVoltage[] PROGMEM = "2.2.8.0";     // r / int
/*
The nominal battery voltage in Volts
*/
const char upsAdvBatteryNominalVoltage[] PROGMEM = "2.2.7.0";    // r / int
/*
The battery current in tenths of Amps
*/
//  const char upsHighPrecBatteryCurrent[] PROGMEM = "2.3.5.0"; // r / int
/*
The battery current in Amps
*/
const char upsAdvBatteryCurrent[] PROGMEM = "2.2.9.0";           // r / int
/*
The total DC current in tenths of Amps
*/
//  const char upsHighPrecTotalDCCurrent[] PROGMEM = "2.3.6.0"; // r / int
/*
The number of external battery packs connected to the UPS. If
the UPS does not use smart cells then the agent reports
ERROR_NO_SUCH_NAME
*/
const char upsAdvBatteryNumOfBattPacks[] PROGMEM = "2.2.5.0";    // r / int
/*
The number of external battery packs connected to the UPS that
are defective. If the UPS does not use smart cells then the
agent reports ERROR_NO_SUCH_NAME
*/
const char upsAdvBatteryNumOfBadBattPacks[] PROGMEM = "2.2.6.0"; // r / int
/*
The date when the UPS system's batteries were last replaced
in mm/dd/yy (or yyyy) format. For Smart-UPS models, this value
is originally set in the factory. When the UPS batteries
are replaced, this value should be reset by the administrator.
For Symmetra PX 250/500 this OID is read only and is configurable in the local display only
*/
const char upsBasicBatteryLastReplaceDate[] PROGMEM = "2.1.3.0"; // rw / string
/*
The results of the last UPS diagnostics test performed
- ok 	(1),
- failed 	(2),
- invalidTest 	(3),
- testInProgress 	(4)
*/
const char upsAdvTestDiagnosticsResults[] PROGMEM = "7.2.3.0";   // r / int
/*
The date the last UPS diagnostics test was performed in
mm/dd/yy format
*/
const char upsAdvTestLastDiagnosticsDate[] PROGMEM = "7.2.4.0";  // r / string
/*
The output voltage of the UPS system in tenths of VAC
*/
const char upsHighPrecOutputVoltage[] PROGMEM = "4.3.1.0";       // r / gauge
/*
The output voltage of the UPS system in VAC
*/
const char upsAdvOutputVoltage[] PROGMEM = "4.2.1.0";            // r / gauge
/*
The number of output phases utilized in this
device. The sum of all the upsPhaseNumOutputPhases
variable indicates the number of rows in the
output phase table
*/
const char upsPhaseNumOutputPhases[] PROGMEM = "9.3.2.1.2.1";    // r / int
/*
The output frequency in 0.1 Hertz, or -1 if it's
unsupported by this UPS
*/
const char upsPhaseOutputFrequency[] PROGMEM = "9.3.2.1.4.1";    // r / int
/*
The current output frequency of the UPS system in tenths of Hz
*/
//  const char upsHighPrecOutputFrequency[] PROGMEM = "4.3.2.0"; // r / gauge
/*
The current output frequency of the UPS system in Hz
*/
const char upsAdvOutputFrequency[] PROGMEM = "4.2.2.0";          // r / gauge
/*
The current in tenths of amperes drawn by the load on the UPS
*/
//  const char upsHighPrecOutputCurrent[] PROGMEM = "4.3.4.0"; // r / gauge
/*
The current in amperes drawn by the load on the UPS
*/
const char upsAdvOutputCurrent[] PROGMEM = "4.2.4.0";            // r / gauge
/*
The output voltage in VAC, or -1 if it's unsupported by this UPS.
*/
const char upsPhaseOutputVoltage[] PROGMEM = "9.3.3.1.3.1.1.1"; // r / int
// const char upsUnknown02[] PROGMEM = "9.3.3.1.3.1.1.2";
// const char upsUnknown03[] PROGMEM = "9.3.3.1.3.1.1.3";
/*
The output current in 0.1 amperes drawn by the load on the UPS, or -1 if it's unsupported by this UPS.
*/
const char upsPhaseOutputCurrent[] PROGMEM = "9.3.3.1.4.1.1.1"; // r / int
// const char upsUnknown05[] PROGMEM = "9.3.3.1.4.1.1.2";
// const char upsUnknown06[] PROGMEM = "9.3.3.1.4.1.1.3";
/*
The maximum output current in 0.1 amperes measured since the last reset (upsPhaseResetMaxMinValues),
or -1 if it's unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseOutputMaxCurrent[] PROGMEM = "9.3.3.1.5.1.1.1"; // r / int
// const char upsUnknown08[] PROGMEM = "9.3.3.1.5.1.1.2";
// const char upsUnknown09[] PROGMEM = "9.3.3.1.5.1.1.3";
/*
The minimum output current in 0.1 amperes measured since the last reset (upsPhaseResetMaxMinValues), or
-1 if it's unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseOutputMinCurrent[] PROGMEM = "9.3.3.1.6.1.1.1"; // r / int
// const char upsUnknown11[] PROGMEM = "9.3.3.1.6.1.1.2";
// const char upsUnknown12[] PROGMEM = "9.3.3.1.6.1.1.3";
/*
The output load in VA, or -1 if it's unsupported by this UPS.
*/
const char upsPhaseOutputLoad[] PROGMEM = "9.3.3.1.7.1.1.1"; // r / int
// const char upsUnknown14[] PROGMEM = "9.3.3.1.7.1.1.2";
// const char upsUnknown15[] PROGMEM = "9.3.3.1.7.1.1.3";
/*
The maximum output load in VA measured since the last reset (upsPhaseResetMaxMinValues), or
-1 if it's unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseOutputMaxLoad[] PROGMEM = "9.3.3.1.8.1.1.1"; // r / int
// const char upsUnknown17[] PROGMEM = "9.3.3.1.8.1.1.2";
// const char upsUnknown18[] PROGMEM = "9.3.3.1.8.1.1.3";
/*
The minimum output load in VA measured since the last reset (upsPhaseResetMaxMinValues), or
-1 if it's unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseOutputMinLoad[] PROGMEM = "9.3.3.1.9.1.1.1"; // r / int
// const char upsUnknown20[] PROGMEM = "9.3.3.1.9.1.1.2";
// const char upsUnknown21[] PROGMEM = "9.3.3.1.9.1.1.3";
/*
The percentage of the UPS load capacity in VA at redundancy @ (n + x) presently
being used on this output phase, or -1 if it's unsupported by this UPS.
*/
const char upsPhaseOutputPercentLoad[] PROGMEM = "9.3.3.1.10.1.1.1"; // r / int
// const char upsUnknown23[] PROGMEM = "9.3.3.1.10.1.1.2";
// const char upsUnknown24[] PROGMEM = "9.3.3.1.10.1.1.3";
/*
The maximum percentage of the UPS load capacity in VA measured at redundancy @ (n + x) presently
being used on this output phase since the last reset (upsPhaseResetMaxMinValues), or -1 if it's unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseOutputMaxPercentLoad[] PROGMEM = "9.3.3.1.11.1.1.1"; // r / int
// const char upsUnknown26[] PROGMEM = "9.3.3.1.11.1.1.2";
// const char upsUnknown27[] PROGMEM = "9.3.3.1.11.1.1.3";
/*
The minimum percentage of the UPS load capacity in VA measured at redundancy @ (n + x) presently
being used on this output phase since the last reset (upsPhaseResetMaxMinValues), or -1 if it's
unsupported by this UPS. Sampled every 30 seconds.
*/
const char upsPhaseOutputMinPercentLoad[] PROGMEM = "9.3.3.1.12.1.1.1"; // r / int
// const char upsUnknown29[] PROGMEM = "9.3.3.1.12.1.1.2";
// const char upsUnknown30[] PROGMEM = "9.3.3.1.12.1.1.3";

/*
The nominal output voltage from the UPS in VAC.
For a list of allowed values supported by your UPS model,
see the UPS User's Manual.
If a value other than a supported value is provided in a
set request, the UPS interprets it as the next lower
acceptable value. If the provided value is lower than
the lowest acceptable value, the lowest acceptable
value is used.
*/
const char upsAdvConfigRatedOutputVoltage[] PROGMEM = "5.2.1.0"; // rw / int

/*
The relative humidity as a percentage for Probe 1
*/
const char mUpsEnvironRelativeHumidity[] PROGMEM = "2.1.2.0"; // r / gauge
/*
The ambient temperature in Celsius for Probe 1
*/
const char mUpsEnvironAmbientTemperature[] PROGMEM = "2.1.1.0"; // r / gauge

// The status of the battery
//   - unknown(1) indicates the device status is unknown.
//   - notInstalled(2) indicates the device is not installed.
//   - ok(3) indicates the battery status is OK.
//   - failed(4) indicates the battery status is failed.
//   - highTemperature(5) indicates the battery has a high temperature condition.
//   - replaceImmediately(6) indicates the battery must be replaced immediately.
//   - lowCapacity(7) indicates the battery has a low capacity."
const char upsDiagBatteryStatus[] PROGMEM = "13.3"; // r / int

//
// ███████╗███╗   ███╗ ██████╗ ██████╗ ███╗   ██╗███████╗██╗ ██████╗
// ██╔════╝████╗ ████║██╔════╝██╔═══██╗████╗  ██║██╔════╝██║██╔════╝
// █████╗  ██╔████╔██║██║     ██║   ██║██╔██╗ ██║█████╗  ██║██║  ███╗
// ██╔══╝  ██║╚██╔╝██║██║     ██║   ██║██║╚██╗██║██╔══╝  ██║██║   ██║
// ███████╗██║ ╚═╝ ██║╚██████╗╚██████╔╝██║ ╚████║██║     ██║╚██████╔╝
// ╚══════╝╚═╝     ╚═╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝     ╚═╝ ╚═════╝

const char oidEmterpriseEm[] PROGMEM = ".1.3.6.1.4.1.38.1.1.10.";
/*
The high temperature alarm threshold for the probe. Units are displayed in the scale
selected in the 'emConfigProbeTempUnits' OID (Celsius or Fahrenheit).
*/
const char emConfigProbeHighTempThreshold[] PROGMEM = "1.2.2.1.3.1"; // rw / int
/*
The low temperature alarm threshold for the probe. Units are displayed in the scale selected
in the 'emConfigProbeTempUnits' OID (Celsius or Fahrenheit).
*/
const char emConfigProbeLowTempThreshold[] PROGMEM = "1.2.2.1.4.1"; // rw / int
/*
The high humidity alarm threshold for the probe in percent relative humidity.
*/
const char emConfigProbeHighHumidThreshold[] PROGMEM = "1.2.2.1.6.1"; // rw / int
/*
The low humidity alarm threshold for the probe in percent relative humidity
*/
const char emConfigProbeLowHumidThreshold[] PROGMEM = "1.2.2.1.7.1"; // rw / int
/*
The current temperature reading from the probe displayed in the units shown in the
'iemStatusProbeTempUnits' OID (Celsius or Fahrenheit).
*/
const char iemStatusProbeCurrentTemp[] PROGMEM = "2.3.2.1.4.1"; // r / int
/*
The current humidity reading from the probe in percent relative humidity
*/
const char iemStatusProbeCurrentHumid[] PROGMEM = "2.3.2.1.6.1"; // r / int

class AgentClass
{
public:
    bool restartUDP();
    void kill();
    status_t init();
    bool isActive()
    {
        return this->active;
    };
    // bool handleGetRequestPDU(SNMP_PACKET_T * packet);*/
    inline void compileOID(const char *baseOID, const char *specOID, char *dest);
    ValueCallback *addIntegerHandler(const char *oid, cbfGetInteger get, cbfSetInteger set = nullptr);
    ValueCallback *addTimestampHandler(const char *oid, cbfGetTimestamp get, cbfSetTimestamp set = nullptr);
    ValueCallback *addStringHandler(const char *oid, char *buffer, size_t max_len);
    ValueCallback *addStringHandler(const char *oid, cbfGetString get, cbfSetString set = nullptr);
    ValueCallback *addOpaqueHandler(const char *oid, cbfGetOpaque get, cbfSetOpaque set = nullptr);
    ValueCallback *addOIDHandler(const char *oid, cbfGetOID get, cbfSetOID set = nullptr);
    ValueCallback *addGaugeHandler(const char *oid, cbfGetGauge get, cbfSetGauge set = nullptr);
    ValueCallback *addCounter32Handler(const char *oid, cbfGetCounter32 get, cbfSetCounter32 set = nullptr);
    ValueCallback *addCounter64Handler(const char *oid, cbfGetCounter64 get, cbfSetCounter64 set = nullptr);

    // ValueCallback* addIntegerHandler(const char *oid, int* value, bool isSettable = false/* , bool overwritePrefix */);
    ValueCallback *addReadOnlyIntegerHandler(const char *oid, int value /* , bool overwritePrefix */);
    ValueCallback *addDynamicIntegerHandler(const char *oid, GETINT_FUNC callback_func /* , bool overwritePrefix */);
    ValueCallback *addReadWriteStringHandler(const char *oid, char **value, size_t max_len = 0, bool isSettable = false /* , bool overwritePrefix */);
    ValueCallback *addReadOnlyStaticStringHandler(const char *oid, const std::string &value /* , bool overwritePrefix */);
    ValueCallback *addDynamicReadOnlyStringHandler(const char *oid, GETSTRING_FUNC callback_func /* , bool overwritePrefix */);
    // ValueCallback* addOpaqueHandler(const char *oid, uint8_t* value, size_t data_len, bool isSettable = false/* , bool overwritePrefix */);
    // ValueCallback* addTimestampHandler(const char *oid, uint32_t* value, bool isSettable = false/* , bool overwritePrefix */);
    ValueCallback *addDynamicReadOnlyTimestampHandler(const char *oid, GETUINT_FUNC callback_func /* , bool overwritePrefix */);
    // ValueCallback* addOIDHandler(const char *oid, const std::string& value/* , bool overwritePrefix */);
    // ValueCallback* addCounter64Handler(const char *oid, uint64_t* value/* , bool overwritePrefix */);
    // ValueCallback* addCounter32Handler(const char *oid, uint32_t* value/* , bool overwritePrefix */);
    // ValueCallback* addGaugeHandler(const char *oid, uint32_t* value/* , bool overwritePrefix */);
    // Depreciated, use addGaugeHandler()
    // __attribute__((deprecated)) ValueCallback* addGuageHandler(const char *oid, uint32_t* value/* , bool overwritePrefix */) {
    //     return addGaugeHandler(oid, value, overwritePrefix);
    // }

    SNMP_ERROR_RESPONSE loop();
    bool setOccurred = false;
    void resetSetOccurred()
    {
        setOccurred = false;
    };
    bool removeHandler(ValueCallback *callback);
    bool sortHandlers();
    snmp_request_id_t sendTrapTo(SNMPTrap *trap, const IPAddress &ip, bool replaceQueuedRequests = true, int retries = 0, int delay_ms = 30000);
    static void markTrapDeleted(SNMPTrap *trap);

protected:
    AgentClass *agent;
    static std::list<AgentClass *> agents;
    std::list<struct InformItem *> informList;
    std::deque<ValueCallback *> callbacks;
    std::string oidPrefix;
    // void handleChanges();
    void handleInformQueue();
    static void informCallback(void *, snmp_request_id_t, bool);
    ValueCallback *addHandler(ValueCallback *callback);
    SortableOIDType *buildOIDWithPrefix(const char *oid);

private:
    bool active = false;
};

extern AgentClass snmpagent;

#endif                                  // SNMP_AGENT_H
