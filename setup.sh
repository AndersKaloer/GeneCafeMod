#!/bin/sh
cp BB-ROASTER-01-00A0.dtbo /lib/firmware/
cp BB-ROASTERPWM-01-00A0.dtbo /lib/firmware/
cp BB-SPI0-01-00A0.dtbo /lib/firmware/
echo BB-SPI0-01 > /sys/devices/bone_capemgr.9/slots
echo BB-ROASTER-01 > /sys/devices/bone_capemgr.9/slots
echo cape-bone-iio > /sys/devices/bone_capemgr.9/slots
echo BB-ROASTERPWM-01 > /sys/devices/bone_capemgr.9/slots
