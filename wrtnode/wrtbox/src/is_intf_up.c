/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 ##############################################*/
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/resource.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <dirent.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <dirent.h>
#include <netdb.h>
#include <syslog.h>
#include <pthread.h>

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

int is_intf_up_main(int argc, char *argv[])
{
    if(argc != 2)
    {
       printf("usage: is_intf_up <ifname>\n");
	   printf("example: is_intf_up br-lan\n");
	   return -1;
    }
	

	int ret = __is_intf_up(argv[1]);
	printf("%d\n",ret);
    return 0;
}

 


