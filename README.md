<pre align="center" background-color="transparent">

  |   _)              |  | _ \   __| 
   _|  |    \   |  |  |  | __/ \__ \ 
 \__| _| _| _| \_, | \__/ _|   ____/ 
               ___/                  
</pre>

<p align="center" style="text-align:center">
<img src="./doc/wp.jpg" width="100%">
</p>

<p align="center" style="text-align:center">

![Dynamic JSON Badge](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2Fway5%2FtinyUPS%2Fdev%2Fconfigure.json&query=%24.version&logo=cplusplus&logoColor=white&label=Firmware%20version%3A&color=purple)
&nbsp;&nbsp;&nbsp;![UI version](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fraw.githubusercontent.com%2Fway5%2FtinyUPS%2Fdev%2Fpackage.json&query=%24.version&logo=html5&logoColor=white&style=flat&label=UI%20version%3A&color=red)&nbsp;&nbsp;&nbsp;![Static Badge](https://img.shields.io/badge/License%3A-GPL--3.0-orange?style=flat&logo=gnu&logoColor=white)

</p>

## **+ DESCRIPTION**<a id="description"></a>

**tinyUPS** is the way to extend functionality of an Uninterruptible Power Supply (UPS)/power inverters.
It provides onboard SNMP server, temperature control and a Control Panel (CP), just like on an enterprise systems. 
The project is now on beta test stage, however it shows very stable results during months 24/7. If you'll decide to modify your own UPS it's at your own risk.

<table cellpadding="0" cellspacing="0" width="100%">
<tr style="border-bottom: 1px solid;">
<td width="50px" style="text-align:center; padding-top:20px;">

![WARNING](https://friconix.com/png/fi-ensuxt-warning-solid.png)

</td>
<td>

The project requires an advanced skills in electronics. It's not a plug-n-play device. ;)

</td>
</tr>
<tr>
<td style="text-align:center; padding-top:20px;">

![WARNING](https://friconix.com/png/fi-hnsdxt-plug-alt.png)

</td>
<td>

**Follow the standard precautions! Every UPS is a potentially dangerous device because it contains an electric parts under high voltage.**

</td>
</tr>
</table>

## **+ TABLE OF CONTENTS**

- [Description](#description)
- [Toolkit](#toolkit)
   - [Controller](#controller)
   - [PCB](#pcb)
 - [WebUI](#webui)
 - [Build](#build)
   - [Configuration](#configuration)
   - [WiFi Reconnect Strategies](#reconnect_strategy)
 - [Setup](#setup)
 - [Control Panel](#control_panel)
 - [Development](#development)
   - [Drivers](#drivers)
   - [Debug](#debug)
 - [Photos](#photos)
 - [Credits](#credits)


## **+ TOOLKIT**<a id="toolkit"></a>

- PlatformIO
- KiCAD

## **+ CONTROLLER**<a id="controller"></a>
**tinyUPS** is originally based on [WEMOS S2 mini](https://www.wemos.cc/en/latest/s2/s2_mini.html) board. However very likely it would work on another ESP32 boards with minor changes in firmware.
All the configuration you may need is in [platformio.ini](platformio.ini) and the UPS driver header file. If the communication interface of a particular UPS is not SPI, you need to create a new UPS driver and to change the PCB as well.

<table cellpadding="0" cellspacing="0" width="100%">
<tr>
<td width="50px" style="text-align:center; padding-top:20px;">

![Development](https://friconix.com/png/fi-cnsuxt-handshake-solid.png)

</td>
<td>

If you've created a new driver please share it by creating merge request or attaching it in discussions.

</td>
</tr>
</table>

## **+ PCB**<a id="pcb"></a>
The [current PCB](schematics/CAM/tinyUPS.kicad_pcb) is designed for DIY via CAM method. If you want it printed profesionally, you may need to redesign the PCB.
Since the most of the UPS controllers have 5V data bus, we need a level shifter to be able to communicate with them and a small step-down PSU for ESP32. 

[Bill of materials](schematics/CAM/tinyUPS.csv). Instead of the BSS138 mosfets could be used any compatible type, ex.: IRLML2502.

If you'll be using S2 mini board you need to desolder built-in LDO IC (ME6211C33) since power will be supplied directly to 3.3V pin. It's possible to keep the LDO but you will not be able to use serial port for firmware upload and debugging. See [WEMOS S2 mini schematics](schematics/sch_s2_mini_v1.0.0.pdf).

The example driver is for a built in SPI LCD display, for a particular manufacturer and model. 
You'll probably have the very different device and may be even without any LCD display, so you'd need to figure out how to speak with the controller. This part is DIY. Feel free to call for help in Discussions.

## **+ Web UI**<a id="webui"></a>
**tinyUPS** has web UI based at [tailwindcss](https://tailwindcss.com/)/[flowbite](https://github.com/themesberg/flowbite) and [webpack](https://webpack.js.org/concepts/).

UI translations are available in [./web/lang](./web/lang) directory. You're able to add a new one or remove existing if you wish by editing the header of [common.js](web/src/common.js) script. Variable <code>i18nlang</code> contains the list of available locales to be built-in, where element 0 of the array is also a fallback (used by default) locale. Remove unnecessary locales from <code>i18nlang</code> to save space on file system partition.

The package manager is <code>yarn</code> so if you're not familiar with it continue with the folowing to build the UI:

```bash
yarn build:prod
```

that's all you need to prepare the FS to upload. Use "Upload Filesystem Image" command from PlatformIO project tasks or:

```bash
platformio run --target uploadfs [--environment [your_env]]
```


## **+ BUILD**<a id="build"></a>

### - CONFIGURATION<a id="configuration"></a>

All the sigificant parameters for your setup are inside [configrure.json](./configure.json) file. If you're adding a new parameter you'd need to run `Rebuild Intellisense Index` in PlatformIO in order to have your parameter set and available.

| PARAMETER | DESCRIPTION |
|:---|:---|
| system_log_size | Size of the system log file (in bytes) |
| snmp_log_size | Size of the SNMP server log file (in bytes) |
| mtemp_log_size | Size of the temperature management log (in bytes) |
| mdata_log_size | Size of the log of monitored events (in bytes) |
| ups_rated_input_voltage | Nominal input voltage value (V) |
| ups_rated_input_freq | Nominal input frequency (Hz) |
| ups_rated_low_input_voltage_threshold | Nominal low threshold of the input voltage (V) |
| ups_rated_high_input_voltage_threshold | Nominal hight threshold for input voltage (V) |
| ups_rated_output_voltage | Nominal output voltage (V) |
| ups_rated_output_freq | Nominal output frequency (Hz) |
| battery_rated_charge_capacity_ah | Nominal UPS battery capacity (Ah) |
| ups_rated_battery_amps_max | Nominal UPS battery current (Amps) |

### - WiFi RECONNECTION STRATEGIES<a id="reconnect_strategy"></a>

If **tinyUPS** has lost connection with AP you may wish it to fallback to AP mode (`tinyUPS.01.XXXX` network name) or it may keep trying to reconnect till the source network is available. 
You can choose the desired behavior (see `wifi_reconnect_method` parameter) before to compile firmware.

| METHOD_ID | DESCRIPTION |
|:---:|:---|
| 1 | If the configuration data is available, **tinyUPS** intents to connect to the specified AP. If the connection is not succeeded it starts AP. If connection is lost the device tryes to reconnect once (10 sec interval). If connection is not succeeded it starts AP and remains in this mode untill restart. |
|  2 <sub>[D]</sub> | If **tinyUPS** has lost connection with source network it tryes to reconnect once. If connection is not succeeded it starts AP and periodically scans WiFi networks. If the source network has been found, the device intents to reconnect. Till the connection is not succeeded it remains in AP mode. |
| 3 | Once the device has lost connection to AP it continuously tryes to reconnect to the source network. The WiFi mode remains STA. |
| 4 | Rely on built-in functionality of `setAutoReconnect()`. **tinyUPS** will remain in STA mode and be seeking for the source AP. |

>***[D]*** - the default method

## **+ SETUP**<a id="setup"></a>
When you first run **tinyUPS**, it boots up in AP mode. Look for `tinyUPS.01.XXXX` WiFi network. Default network password (config.apkey) could be changed in [helpers.h](src/helpers.h). Once connected the configuration is available on 192.168.4.1.

![tinyUPS setup page](doc/s0.jpg)

When setup is completed **tinyUPS** restarts, expecting an automatically assigned IP.

You are able to restart the device or reset configuration data to defaults using Control Panel -> Service Menu:

![tinyUPS service menu](doc/i2.jpg)

and then continue with confirmation:

![tinyUPS confirmation](doc/i3.jpg)


## **+ CONTROL PANEL**<a id="control_panel"></a>

The UI is pretty simple and displays most of the real-time parameters. There are multiple self explanatory graphical representations for the collected data.

![tinyUPS dashboard charts](doc/i0.jpg)


**tinyUPS** may be monitored remotely via JSON API, in practice it may be easily included in a local smart home network. To get access to API you need to add an API key (go to Configuration -> API):

![API control panel](doc/i1.jpg)

Now you're able to send a post request using similar url format:

```
http://tinyUPS_ip_address/command?key=154aae95aa657fc37e0fa7e712dd7856
```

## **+ DEVELOPMENT**<a id="#development"></a>

### - DRIVERS<a id="drivers"></a>
There are the following functions that must be implemented by every UPS driver: `upsDriverInit, upsDriverLoop, upsDriverDeinit`. 
You may also wish to look at the driver for thermistor (currently this is 1k M52A), it may need some changes. Depends on which thermistor you'll be using.


### - DEBUG<a id="debug"></a>
Serial monitor is used to perform the most of the tasks and to solve issues. Once you've connected use <code>?</code> to request the commands list.

![tinyUPS serial monitor](doc/i4.jpg)

## **+ PHOTOS**<a id="photos"></a>

| ![tinyUPS PCB](./doc/e0.jpg) | ![thermistor](./doc/e1.jpg) | ![PCB](./doc/e2.jpg) | ![UPS LCD](./doc/e4.jpg) | ![UPS exterior](./doc/e5.jpg) | 
|:---:|:---:|:---:|:---:|:---:|

## **+ CREDITS**<a id="credits"></a>

- Arduino [SNMP agent](https://github.com/0neblock/Arduino_SNMP) by #0neblock
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [Hash library](https://github.com/bbx10/Hash_tng)
- Icons by [Heroicons](https://heroicons.com/) & [Boxicons](https://boxicons.com/)

