/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

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
//*
static char * get_local_ip_addr(const char * if_name)
{
    int i=0;
    int sockfd;
    struct ifconf ifconf;
    unsigned char buf[512];
    struct ifreq *ifreq;
    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        perror("socket" );
        return NULL;
    }
    ioctl(sockfd, SIOCGIFCONF, &ifconf); 
    close(sockfd);
    ifreq = (struct ifreq*)buf;
    for (i=(ifconf.ifc_len/sizeof (struct ifreq)); i>0; i--)
    {
        if (strcmp(ifreq->ifr_name, if_name) == 0) 
            return inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
        ifreq++;
    }
    return NULL;
}
// */

int get_ip_main(int argc,char* argv[])
{
    if(argc != 2)
    {
       printf("usage: get_ip <ifname>\n");
	   printf("example: get_ip br-lan\n");
	   return -1;
    }
	char * ip = get_local_ip_addr(argv[1]);
    printf("%s\n",ip);
    return 0; 
}
