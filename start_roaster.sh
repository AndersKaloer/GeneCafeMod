#!/bin/sh
./setup.sh

while [ ! -f "/sys/bus/iio/devices/iio:device0/in_voltage6_raw" ]
do
  sleep 2
done
./roaster &
sh -c 'cd webserver && node index.js'
