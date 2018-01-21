#!/bin/sh
./setup.sh
while [ ! -f /sys/devices/platform/ocp/48302000.epwmss/48302200.pwm/pwm/pwmchip0/pwm1/enable ]
do
  sleep 2
done
while [ ! -f "/sys/bus/iio/devices/iio:device0/in_voltage6_raw" ]
do
  sleep 2
done
./roaster &
sh -c 'cd webserver && node index.js'
