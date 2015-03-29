#!/bin/sh
./setup.sh
while [ ! -f /sys/devices/ocp.3/roaster0_m150_pwm.13/run ]
do
  sleep 2
done
./roaster
