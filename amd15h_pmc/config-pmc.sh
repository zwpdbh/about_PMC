#!/bin/bash

if [ $# -ne "3" ]; then
    echo "usage: $0 counter-num hex-value <cpu0 cpu1 ... | all >"
    exit 1
fi

if [ "$3" == "all" ]; then
    ls -1 /sys/kernel/amd15h-pmc | while read cpu; do
      echo $2 > /sys/kernel/amd15h-pmc/$cpu/$1
    done
else
    for cpu in $3; do
      echo $2 > /sys/kernel/amd15h-pmc/cpu$cpu/$1
    done
fi
