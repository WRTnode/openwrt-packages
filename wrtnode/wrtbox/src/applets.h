/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 Noties:
 applets.h - a listing of all wrtbox applets.
 If you write a new applet, you need to add an entry to this list to make
 wrtbox aware of it.
 ##############################################*/

extern int wrtbox_main(int argc, char **argv);
extern int wait_intf_up_main(int argc, char *argv[]);
extern int is_intf_up_main(int argc, char *argv[]);
extern int hellowrt_main(int argc, char *argv[]);
extern int get_ip_main(int argc, char *argv[]);
extern int dnsclient_main(int argc, char *argv[]);



