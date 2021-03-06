#!/bin/bash

# Setup counters:
#  [35 - 32]	 event mask [11 - 8]
#  [31 - 24]	 counter mask
#  [22]		 counter enable
#  [17]		 operating system mode
#  [16]		 user mode
#  [15 - 8]	 unit mask
#  [7 - 0]	 event mask [7 - 0]

# EventSelect 046h L1 DTLB and L2 DTLB Miss 
let "v = 0x0 << 32 |
	 1 << 24 |  
	 1 << 22 | 
	 1 << 17 | 
	 1 << 16 | 
	 0x07 << 8 | 
	 0x46"

v=`printf "%x" $v`
config-pmc.sh $1 $v all
