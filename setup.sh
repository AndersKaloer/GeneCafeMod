#!/bin/sh
cp dts/BB-ROASTER-01-00A0.dtbo /lib/firmware/
cp dts/BB-ROASTERPWM-01-00A0.dtbo /lib/firmware/
#cp dts/BB-SPI0-01-00A0.dtbo /lib/firmware/
#echo BB-SPI0-01 > /sys/devices/bone_capemgr.9/slots
#echo BB-ROASTER-01 > /sys/devices/bone_capemgr.9/slots
#echo cape-bone-iio > /sys/devices/bone_capemgr.9/slots
#echo BB-ROASTERPWM-01 > /sys/devices/bone_capemgr.9/slots

echo 1 > /sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip0/export
