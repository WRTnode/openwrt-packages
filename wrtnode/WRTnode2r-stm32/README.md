Introduction
===
WRTnode2r-stm32 provide an update stm32 system approach.

The following actions are performed in the OpenWrt system.

Step:

1：Install WRTnode2r-stm32 ipk：

	1.1：opkg update 

	1.2 opkg install WRTnode2r-stm32

2: Update stm32:

STM32 system files can be placed in the local, but also directly download from the network.

for example:

	download from the network:

	flash-stm32 http://d.wrtnode.com/2R-stm32/WRTnode2r_stm32_V1.bin

	or

	Local file:

	flash-stm32 /tmp/WRTnode2r_stm32_V1.bin	
