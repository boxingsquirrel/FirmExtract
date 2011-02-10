/*
 *  baseband.c
 *  FirmExtract-OSX
 *
 *  Created by boxingsquirrel on 11/21/10.
 *  Copyright 2010 N/A. All rights reserved.
 *
 */

#include <stdlib.h>
#include "baseband.h"
#include "ipsw.h"
#include "ipsw_t.h"

// The iPhone4 is /very/ different from the others...
int iPhone4_do_bb_extract (char *ipsw, ipsw_t ipsw_file)
{
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->bb, (const char *)ipsw_file->bb);
	return 0;
}

// This is the way all iOS devices except the iPhone4 with a baseband do it AFAIK...
int iPhone_do_bb_extract(char *app_bundle, char *ipsw, ipsw_t ipsw_file)
{
	// Likely overkill, but we free up when we're done, so...
	char cmd[1024];
	snprintf(cmd, 1024, "%s/Contents/MacOS/hfsplus %s extractall %s", app_bundle, ipsw_file->restore_ramdisk, ipsw_file->bb);
	int res=system(cmd);

	free(cmd);
	return res;
}