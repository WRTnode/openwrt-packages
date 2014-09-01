Introduction
===

If you want to write MAC, you must ensure that mtd factory is writable.
Remove the factory's read-only properties in the WRTNODE.dts.
	
vim WRTNODE.dts

......

	factory: partition@40000 {

		label = "factory";

		reg = <0x40000 0x10000>;

		#read-only;

	};

......

Download WRTnodemacjsonp.html form https://raw.githubusercontent.com/WRTnode/openwrt-patches/master/FlashMAC-html/WRTnodemacjsonp.html

Download guide.doc from https://github.com/WRTnode/openwrt-patches/raw/master/FlashMAC-html/guide.doc
