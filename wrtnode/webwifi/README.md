WEBWIFI
===
webwifi is a software package can be set up WRTnode apcli0 in the web interface
Installation
---
1 Use openwrt toolchina cross compiler webwifi ipk

2 install webwifi ipk:

  `opkg install webwifi_1_ramips_24kec.ipk`

3 ipk package after the installation is complete and then execute the following script:

  `set_default_page_to_wrtnode_portal.sh`

4 Open your Browser (if ever before to access luci interface, please clean up your Browser’s cache),input 192.168.8.1, Set wifi access point and password page.

5 Choose your wireless access point, fill out a wireless password, click Save. This process because to restart the network, so to wait a little longer, then refresh the page, you will be prompted to connect the router and has been assigned to the ip address. Click to set the administrative password, you can change your wifi password.

Remove 
---
6 Remove webwifi ipk:

 `opkg remove webwifi`

7 After remove webwifi packages to execute the following script, also use the Tab key to match:

 `switch_to_default_luci.sh`

