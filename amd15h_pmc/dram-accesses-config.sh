#!/bin/bash

# Setup counters for NB events:
#  [35 - 32]	 event mask [11 - 8]
#  [22]		 counter enable
#  [20]		 enable APIC interrupt
#  [15 - 8]	 unit mask
#  [7 - 0]	 event mask [7 - 0]

let "v = 0x0 << 32 |
	 1 << 22 | 
	 0x3f << 8 | 
	 0xe0"

v=`printf "%x" $v`
./config-pmc.sh $1 $v all
