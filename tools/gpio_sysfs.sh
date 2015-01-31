#!/bin/bash

for gpio in 4 17 18 27 22 23 24 10 9 25 11 8 7; do
    echo $gpio > /sys/class/gpio/export
done

while [ 1 ]; do 
for gpio in 4 17 18 27 22 23 24 10 9 25 11 8 7; do
    echo -n "gpio$gpio: " 
    cat /sys/class/gpio/gpio${gpio}/value
done
sleep 1
echo "-----------------------------------"
done
