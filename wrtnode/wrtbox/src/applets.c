/*##############################################
# wrtbox is a Swiss Army knife for WRTnode  
# WRTnode's busybox
# This file is part of wrtbox.
# Author: 39514004@qq.com (huamanlou,alais name intel inside)
#
# This library is free software; under the terms of the GPL
#
##############################################*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wrtbox.h"

#undef APPLET
#undef APPLET_NOUSAGE
#undef PROTOTYPES
#include "applets.h"

struct BB_applet *applet_using;

const struct BB_applet applets[] = {
#ifdef BB_DNSCLIENT
	{"dnsclient", dnsclient_main, _BB_DIR_BIN},
#endif
#ifdef BB_GET_IP
	{"get_ip", get_ip_main, _BB_DIR_USR_BIN},
#endif
#ifdef BB_HELLOWRT
	{"hellowrt", hellowrt_main, _BB_DIR_BIN},
#endif
#ifdef BB_IS_INTF_UP
	{"is_intf_up", is_intf_up_main, _BB_DIR_BIN},
#endif
#ifdef BB_WAIT_INTF_UP
	{"wait_intf_up", wait_intf_up_main, _BB_DIR_BIN},
#endif
    {"wrtbox", wrtbox_main, _BB_DIR_BIN},
	{ 0,NULL,0 }
};


/* The -1 arises because of the {0,NULL,0,-1} entry above. */
const size_t NUM_APPLETS = (sizeof (applets) / sizeof (struct BB_applet) - 1);

extern void show_usage(void)
{
	const char *format_string;
	const char *usage_string = "";
	int i;

	for (i = applet_using - applets; i > 0; ) {
		if (!*usage_string++) {
			--i;
		}
	}
	format_string = "%s\n\nUsage: %s %s\n\n";
	if(*usage_string == 0)
		format_string = "%s\n\nNo help available.\n\n";
	fprintf(stderr, format_string,
			full_version, applet_using->name, usage_string);
	exit(EXIT_FAILURE);
}

static int applet_name_compare(const void *x, const void *y)
{
	const char *name = x;
	const struct BB_applet *applet = y;
    return strcmp(name, applet->name);
}

extern const size_t NUM_APPLETS;

struct BB_applet *find_applet_by_name(const char *name)
{
	return bsearch(name, applets, NUM_APPLETS, sizeof(struct BB_applet),
        applet_name_compare);
}

void run_applet_by_name(const char *name, int argc, char **argv)
{
    static int recurse_level = 0;
	extern int been_there_done_that; 
    extern const char *applet_name;
	recurse_level++;
	if ((applet_using = find_applet_by_name(name)) != NULL) {
		applet_name = applet_using->name;
		if (argv[1] && strcmp(argv[1], "--help") == 0) {
			if (strcmp(applet_using->name, "wrtbox")==0) {
				if(argv[2])
				  applet_using = find_applet_by_name(argv[2]);
				 else
				  applet_using = NULL;
			}
			if(applet_using)
				show_usage();
			been_there_done_that=1;
			wrtbox_main(0, NULL);
		}
        exit((*(applet_using->main)) (argc, argv));
	}
	if (recurse_level == 1) {
		run_applet_by_name("wrtbox", argc, argv);
	}
	recurse_level--;
}
/* END CODE */
