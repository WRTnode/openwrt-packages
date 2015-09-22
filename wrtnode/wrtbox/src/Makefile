##############################################
# wrtbox is a Swiss Army knife for WRTnode  
# WRTnode's busybox
# This file is part of wrtbox.
# Author: 39514004@qq.com (huamanlou,alais name intel inside)
#
# This library is free software; under the terms of the GPL
#
##############################################

CFLAGS=-Wall 
EXTRALIB=-ldl -lm -lc
OBJS=applets.o hellowrt.o dnsclient.o get_ip.o is_intf_up.o wait_intf_up.o wrtbox.o 
all:wrtbox

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

wrtbox: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(EXTRALIB)	

clean:
	rm -f rbcfg *.o

