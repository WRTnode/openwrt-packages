/*##############################################
 wrtbox is a Swiss Army knife for WRTnode  
 WRTnode's busybox
 This file is part of wrtbox.
 Author: 39514004@qq.com (huamanlou,alais name intel inside)

 This library is free software; under the terms of the GPL

 Noties:
 ##############################################*/
 
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include "wrtbox.h"

int been_there_done_that = 0; /* Also used in applets.c */
const char *applet_name;

int main(int argc, char **argv)
{
	const char *s;
	applet_name = argv[0];
	if (applet_name[0] == '-')
		applet_name++;

	for (s = applet_name; *s != '\0';) {
		if (*s++ == '/')
			applet_name = s;
	}
	
    //printf("applet is %s\n",applet_name);
    run_applet_by_name(applet_name, argc, argv);
	
    printf("applet not found\n");
}

int wrtbox_main(int argc, char **argv)
{
	int col = 0, len, i;
	argc--;

	/* If we've already been here once, exit now */
	if (been_there_done_that == 1 || argc < 1) {
		const struct BB_applet *a = applets;

		fprintf(stderr, "%s\n\n"
				"Usage: wrtbox [function] [arguments]...\n"
				"   or: [function] [arguments]...\n\n"
				"\twrtbox is a multi-call binary that combines many common Unix\n"
				"\tutilities into a single executable.  Most people will create a\n"
				"\tlink to wrtbox for each function they wish to use, and wrtbox\n"
				"\twill act like whatever it was invoked as.\n" 
				"\nCurrently defined functions:\n", full_version);

		while (a->name != 0) {
			col +=
				fprintf(stderr, "%s%s", ((col == 0) ? "\t" : ", "),
						(a++)->name);
			if (col > 60 && a->name != 0) {
				fprintf(stderr, ",\n");
				col = 0;
			}
		}
		fprintf(stderr, "\n\n");
		exit(0);
	}

	/* Flag that we've been here already */
	been_there_done_that = 1;
	
	/* Move the command line down a notch */
	len = argv[argc] + strlen(argv[argc]) - argv[1];
	memmove(argv[0], argv[1], len);
	memset(argv[0] + len, 0, argv[1] - argv[0]);

	/* Fix up the argv pointers */
	len = argv[1] - argv[0];
	memmove(argv, argv + 1, sizeof(char *) * (argc + 1));
	for (i = 0; i < argc; i++)
		argv[i] -= len;

	return (main(argc, argv));
}

