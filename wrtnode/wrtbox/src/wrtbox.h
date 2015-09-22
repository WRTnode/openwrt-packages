/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 ##############################################*/
 
#ifndef	_BB_INTERNAL_H_
#define	_BB_INTERNAL_H_    1

#include "Config.h"

#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#include <features.h>

#define full_version "WrtBox 0.0.0.1!"

enum Location {
	_BB_DIR_ROOT = 0,
	_BB_DIR_BIN,
	_BB_DIR_SBIN,
	_BB_DIR_USR_BIN,
	_BB_DIR_USR_SBIN
};

struct BB_applet {
	const	char*	name;
	int	(*main)(int argc, char** argv);
	enum	Location	location;
};
extern const struct BB_applet applets[];
#define PROTOTYPES
#include "applets.h"
#undef PROTOTYPES
#ifdef BB_FEATURE_BUFFERS_GO_ON_STACK
#define RESERVE_BB_BUFFER(buffer,len)           char buffer[len]
#define RESERVE_BB_UBUFFER(buffer,len) unsigned char buffer[len]
#define RELEASE_BB_BUFFER(buffer)      ((void)0)
#else
#ifdef BB_FEATURE_BUFFERS_GO_IN_BSS
#define RESERVE_BB_BUFFER(buffer,len)  static          char buffer[len]
#define RESERVE_BB_UBUFFER(buffer,len) static unsigned char buffer[len]
#define RELEASE_BB_BUFFER(buffer)      ((void)0)
#else
#define RESERVE_BB_BUFFER(buffer,len)           char *buffer=xmalloc(len)
#define RESERVE_BB_UBUFFER(buffer,len) unsigned char *buffer=xmalloc(len)
#define RELEASE_BB_BUFFER(buffer)      free (buffer)
#endif
#endif
#ifndef setbit
#define NBBY            CHAR_BIT
#define setbit(a,i)     ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i)     ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i)      ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define isclr(a,i)      (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)
#endif

#ifndef RB_POWER_OFF
#define RB_POWER_OFF   0x4321fedc
#endif
#include <limits.h>
#include <sys/param.h> 
#ifndef PATH_MAX 
#define  PATH_MAX         256
#endif
#endif /* _BB_INTERNAL_H_ */
