#!/bin/bash

# Setup counters for NB events:
#  [35 - 32]     event mask [11 - 8]
#  [22]          counter enable
#  [20]          enable APIC interrupt
#  [15 - 8]      unit mask
#  [7 - 0]       event mask [7 - 0]

let "v = 0x04 << 32 |
	 1 << 22 | 
	 0xf7 << 8 | 
	 0xe1"

v=`printf "%x" $v`
./config-pmc.sh $1 $v all
