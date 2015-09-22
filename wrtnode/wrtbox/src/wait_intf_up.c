/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 Noties:
 for /etc/init.d/dnsmasq restart when /etc/init.d/network restart fully completed!
 ##############################################*/
 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <netdb.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <net/if.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/resource.h>

// 1 up
// 0 down
static int __is_intf_up(const char * ifname)
{
	struct ifreq ifr;
	int sfd;
	int ret = 0;

	if (!((sfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0))
	{
		strcpy(ifr.ifr_name, ifname);
		if (!ioctl(sfd, SIOCGIFFLAGS, &ifr) && (ifr.ifr_flags & IFF_UP))
			ret = 1;

		close(sfd);
	}

	return ret;
}

static int select_time(int time)
{
     int rc = 0;
     struct timeval tv;
     tv.tv_sec = time;
     tv.tv_usec = 0;
     rc = select(0, NULL, NULL, NULL, &tv);
    // printf("select = %d\n", rc);
     return 0;
}

static int select_time_ms(int ms)
{
     int rc = 0;
     struct timeval tv;
     tv.tv_sec = 0;
     tv.tv_usec = ms;
     rc = select(0, NULL, NULL, NULL, &tv);
    // printf("select = %d\n", rc);
     return 0;
}

int wait_intf_up_main(int argc, char *argv[])
{
    if(argc != 2)
    {
       printf("usage: wait_intf_up <ifname>\n");
	   printf("example: wait_intf_up br-lan\n");
	   return -1;
    }

	while (!__is_intf_up(argv[1]))
       select_time_ms(100);
	printf("%s is up\n",argv[1]);
    return 0;
}
