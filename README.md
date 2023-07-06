<pre style="text-align:center; background-color: transparent;">

  |   _)              |  | _ \   __| 
   _|  |    \   |  |  |  | __/ \__ \ 
 \__| _| _| _| \_, | \__/ _|   ____/ 
               ___/                  
</pre>
<p style="text-align:center">
<img src="./doc/wp.jpg" width="100%">
</p>


## **+ DESCRIPTION** 
**tinyUPS** is the way to extend functionality of an Uninterruptible Power Supply (UPS)/power inverters.
It provides onboard SNMP server, temperature control and a Control Panel (CP), just like on an enterprise systems. 
The project is now on beta test stage, however it shows very stable results during months 24/7. If you'll decide to modify your own UPS it's at your own risk.

<table cellpadding="0" cellspacing="0" width="100%" style="border: 1px solid;">
<tr style="border-bottom: 1px solid;">
<td width="50px" style="text-align:center; padding-top:20px; border-right: 1px solid;">

![WARNING](https://friconix.com/png/fi-ensuxt-warning-solid.png)

</td>
<td>

The project requires an advanced skills in electronics. It's not a plug-n-play device. ;)

</td>
</tr>
<tr>
<td style="text-align:center; padding-top:20px; border-right: 1px solid;">

![ATTENTION](https://friconix.com/png/fi-owpdxt-plug-alt.png)

</td>
<td>

**Follow the standard precautions! Every UPS is a potentially dangerous device because it contains an electric parts under high voltage.**

</td>
</tr>
</table>


## **+ TOOLKIT**

- PlatformIO
- KiCAD

## **+ CONTROLLER**
**tinyUPS** is originally based on WEMOS S2 mini board. However very likely it would work on another ESP32 boards with minor changes in firmware.
All the configuration you may need is in [platformio.ini](platformio.ini) and the UPS driver header file. If the communication interface of a particular UPS is not SPI, you need to create a new UPS driver and to change the PCB as well.

<table cellpadding="0" cellspacing="0" width="100%" style="border:1px solid;">
<tr>
<td width="50px" style="text-align:center; padding-top:20px;  border-right: 1px solid;">

![Development](https://friconix.com/png/fi-cnsuxt-handshake-solid.png)

</td>
<td>

If you've created a new driver please share it by creating merge request or attaching it in discussions.

</td>
</tr>
</table>

## **+ PCB**
The [current PCB](schematics/CAM/tinyUPS.kicad_pcb) is designed for DIY via CAM method. If you want it printed profesionally, you may need to redesign the PCB.
Since the most of the UPS controllers have 5V data bus, we need a level shifter to be able to communicate with them and a small step-down psu for the ESP32. 

[Bill of materials](schematics/CAM/tinyUPS.csv). Instead of the BSS138 mosfets could be used any compatible type, ex.: IRLML2502.

If you'll be using S2 mini board you need to desolder built-in LDO IC (ME6211C33) since power will be supplied directly to 3.3V pin. It's possible to keep the LDO but you will not be able to use serial port for firmware upload and debugging. See [WEMOS S2 mini schematics](schematics/sch_s2_mini_v1.0.0.pdf).

The example driver is for a built in SPI LCD display, for a particular manufacturer and model. 
You'll probably have the very different device and may be even without any display, so you'd need to figure out how to speak with the controller. This part is DIY. Feel free to call for help in Discussions.

## **+ Web UI**
**tinyUPS** has web UI based at [tailwindcss](https://tailwindcss.com/)/[flowbite](https://github.com/themesberg/flowbite) and [webpack](https://webpack.js.org/concepts/). The package manager is yarn so if you're not familiar with them:

```bash
cd ./web
yarn build:prod
```

that is all you need to prepare the FS to upload. Use "Upload Filesystem Image" command from PlatformIO project tasks or:

```bash
platformio run --target uploadfs --environment esp32s2_lolin
```

<table cellpadding="0" cellspacing="0" width="100%" style="border:1px solid;">
<tr>
<td width="50px" style="text-align:center; padding-top:20px;  border-right: 1px solid;">

![Hint](https://friconix.com/png/fi-cnsuht-bulb-solid.png)

</td>
<td>

Webpack output directory "web/data" must contain "log" directory. Create it if doesn't exists.

</td>
</tr>
</table>

**tinyUPS** may be monitored remotely via JSON API, in practice it may be easily included in a local smart home network.

<br>

## **+ SETUP**
When you first run **tinyUPS** it boots in AP mode. Look for `tinyUPS.01.XXXX` WiFi network. Once connected the configuration is available on 192.168.4.1.
Once configured, **tinyUPS** restarts and expects an automatically assigned IP.

<br>

## **+ DRIVERS**
There are the following functions that must be implemented by every UPS driver: `upsDriverInit, upsDriverLoop, upsDriverDeinit`. 
You may also wish to look at the driver for thermistor (currently this is 1k M52A), it may need some changes. Depends on which thermistor you'll be using.

<br>



<br>

## **+ CREDITS**

- Arduino [SNMP agent](https://github.com/0neblock/Arduino_SNMP) by #0neblock
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [Hash library](https://github.com/bbx10/Hash_tng)
- Icons by [Heroicons](https://heroicons.com/)

