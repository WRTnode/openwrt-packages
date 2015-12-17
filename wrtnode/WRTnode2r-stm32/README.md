Introduction
===
WRTnode2r-stm32 provide an update stm32 system approach.

The following actions are performed in the OpenWrt system.

Step:

1：Download & Install WRTnode2r-stm32 ipk：

	$opkg install http://d.wrtnode.com/packages/spi-bridge_1_ramips_24kec.ipk 

	$opkg install http://d.wrtnode.com/packages/WRTnode2r-stm32_1_ramips_24kec.ipk

2: Update stm32 system :

Usage: flash-stm32  

Example #1: Updated to the latest version

	$flash-stm32	

Example #2: Update to the specified version(server or local)

	$flash-stm32 http://d.wrtnode.com/2R-stm32/$Stm32Firmware

or

	$flash-stm32 /tmp/$Stm32Firmware"

