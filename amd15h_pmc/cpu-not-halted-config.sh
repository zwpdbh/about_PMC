#!/bin/bash

# Setup counters:
#  [35 - 32]	 event mask [11 - 8]
#  [31 - 24]	 counter mask
#  [22]		 counter enable
#  [17]		 operating system mode
#  [16]		 user mode
#  [15 - 8]	 unit mask
#  [7 - 0]	 event mask [7 - 0]

# EventSelect 076h CPU Clocks not Halted 
let "v = 0x0 << 32 |
	 1 << 24 |  
	 1 << 22 | 
	 0 << 18 |
	 1 << 17 | 
	 1 << 16 | 
	 0x0 << 8 | 
	 0x76"

v=`printf "%x" $v`
./config-pmc.sh $1 $v all
