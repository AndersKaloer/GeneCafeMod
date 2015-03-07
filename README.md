# Gene Cafe Modifications
This project page describes several modifications made for my Gene
Cafe coffee roaster.

The roaster is controlled by a BeagleBone Black [1].

The purpose of the modifications is to:
- Enhance the control of the roaster to ensure consistent roasts
- Enhance the roast profile overview (temperature curves, etc.).
- Shorten the learning curve by keeping an automatically generated
  roast log.
- Use a broad variety of theory in practice.

The current list of modifications is the following:
- [x] [Enhanced Heater Control (Dimmer Mod)](#enhanced-heater-control-dimmer-mod)
- [x] [Control Interface](#control-interface)
- [x] [Ingoing Air Temperature Probe](#ingoing-air-temperature-probe)
- [x] [Remote Data Logging Software](#remote-data-logging-software)
- [ ] [Remote Control/Monitor GUI Software](#remote-control-monitor-gui-software)
- [ ] [Outgoing Air Temperature Probe](#outgoing-air-temperature)
- [ ] [Automatic Bean Crack Detection](#automatic-bean-crack-detection)
- [ ] [Linux Kernel Device Driver for the LCD Display](#linux-kernel-device-driver-for-the-lcd-display)

## Enhanced Heater Control (Dimmer Mod)
The heater control has been enhanced by placing an AC phase control module
before the heating element of the roaster. The modification is similar
to the one described at
http://coffeetimex.wikidot.com/gene-cafe-dimmer-control-mod-stage-1. However,
instead of using a dimmer module I have used an M028N power control module (4000 watt) from KEMO
electronic [2]. The M028N is used with an M150 DC controller module
[3] which allows the phase produced by the M028N to be controlled
using a TTL signal (PWM/voltage).

## Control Interface
The roaster control interface contains an LCD display for monitoring
time, temperatures, heater, etc. The heater power is controlled
using a potentiometer, which is sampled by the BeagleBone and a PWM
signal to the M150 module with a corresponding duty cycle is
outputted.

The control interface also contains a single button which is used to
start/stop the roasting process as well as to log bean cracks (until
the [Automatic Bean Crack Detection](#automatic-bean-crack-detection)
feature has been implemented). When a bean crack has been indicated, a
new timer is started so that the post-crack time can be monitored.

## Ingoing Air Temperature Probe
In order to monitor the temperature into the roasting chamber, a
thermocouple has been monitored inside the heating element near the
extruder. This has been done as described in
https://plus.google.com/u/0/photos/103376994765292557175/albums/5848581250979871313.

The thermocouple is connected to an Adafruit SPI-compatible max31855
[4] breakout
board [5]. The max31855 is connected to the BeagleBone.

## Remote Data Logging Software
In order to log the roast profile (temperature, heater power, time, etc.)
remotely, a server has been implemented on the BeagleBone. The server
simply sends the sensor data to all connected clients, which can then
log the data to a file, plot it, etc.

The *roaster_cli* project contains a simple file data logger and a Matlab
script to plot the data.

## Remote Control/Monitor GUI Software

## Outgoing Air Temperature Probe

## Automatic Bean Crack Detection

## Linux Kernel Device Driver for the LCD Display


----------
\[1\]: http://beagleboard.org/BLACK  
\[2\]:
http://www.kemo-electronic.de/en/Light-Sound/Effects/Modules/M028N-Power-control-110-240-V-AC-4000-VA.php  
\[3\]:
http://www.kemo-electronic.de/en/Transformer-Dimmer/Converter/M150-DC-pulse-converter.php  
\[4\]:
http://www.maximintegrated.com/en/products/analog/sensors-and-sensor-interface/MAX31855.html  
\[5\]: http://www.adafruit.com/product/269  
