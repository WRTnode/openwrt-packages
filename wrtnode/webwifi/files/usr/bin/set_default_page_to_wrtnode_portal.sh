#!/bin/sh
cat /www/index.html | grep -q -e 'luci'
if [ $? -eq 0 ]; then
	mv /usr/bin/set_default_page_to_wrtnode_portal.sh /usr/bin/switch_to_default_luci.sh
	mv /www/index.html /www/index.htm 
	mv /www/WRTnode_Portal.html /www/index.html
	echo 'Now we have set the default page to WRTnode Portal.'
	exit 0
fi
if [ -f "/www/webui-xhi/setwifi.html" ] ; then
	echo "Pls remove the packge webwifi by using 'opkg remove webwifi' first, then call me."
	exit 0
fi
cat /www/index.html | grep -q -e 'webui-xhi'
if [ $? -eq 0 ]; then
	rm /www/index.html  
	mv /www/index.htm /www/index.html
	rm /usr/bin/switch_to_default_luci.sh
fi
