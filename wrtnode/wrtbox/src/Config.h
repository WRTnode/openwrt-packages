/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 Noties:
 This file defines the feature set to be compiled into wrtbox.
 When you turn things off here, they won't be compiled in at all.
 This file is parsed by sed.  You MUST use single line comments.
 i.e.,  //#define BB_BLAH

##############################################*/

// wrtbox Applications
#define BB_HELLOWRT
#define BB_DNSCLIENT
#define BB_GET_IP
#define BB_IS_INTF_UP
#define BB_WAIT_INTF_UP
// End of Applications List
