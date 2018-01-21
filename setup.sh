#!/bin/sh
cp dts/BB-ROASTER-01-00A0.dtbo /lib/firmware/
cp dts/BB-ROASTERPWM-01-00A0.dtbo /lib/firmware/
#cp dts/BB-SPI0-01-00A0.dtbo /lib/firmware/
#echo BB-SPI0-01 > /sys/devices/bone_capemgr.9/slots
#echo BB-ROASTER-01 > /sys/devices/bone_capemgr.9/slots
#echo cape-bone-iio > /sys/devices/bone_capemgr.9/slots
#echo BB-ROASTERPWM-01 > /sys/devices/bone_capemgr.9/slots

echo 0 > /sys/class/pwm/pwmchip0/export
