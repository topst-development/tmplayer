/****************************************************************************************
 *   FileName    : main.cpp
 *   Description : main.cpp
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited 
to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code 
shall constitute any express or implied warranty of any kind, including without limitation, 
any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, 
copyright or other third party intellectual property right. 
No warranty is made, express or implied, regarding the information¡¯s accuracy, 
completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, 
out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <glib.h>
#include <stdbool.h>

#include "TMPlayer.h"
#include "TMPlayerDBus.h"


static void SignalHandler(int32_t sig);
static void Daemonize(void);

static GMainLoop *mainLoop = NULL;

int32_t g_tc_debug = 0;

int main(int argc, char *argv[])
{
	int32_t ret;
	int32_t idx;
	int32_t daemonize = 1;

	if (argc > 1)
	{
		for (idx = 1; idx < argc; idx++)
		{
			if (strncmp(argv[idx], "--debug", 7) == 0)
			{
				g_tc_debug = 1;
			}
			else if (strncmp(argv[idx], "--no-daemon", 11) == 0)
			{
				daemonize = 0;
			}
			else
			{
				continue;
			}
		}
	}

	if (daemonize == 1)
	{
		Daemonize();
		(void)fprintf(stderr, "Telechips Media Player Daemonized\n");
	}
	
	(void)signal(SIGINT, SignalHandler);
	(void)signal(SIGTERM, SignalHandler);
	(void)signal(SIGABRT, SignalHandler);

	mainLoop = g_main_loop_new(NULL, FALSE);

	if (mainLoop != NULL)
	{
		ret = TMPlayer_Initialize();
		if (ret == 0)
		{
			TMPlayerDBusInitialize();
			g_main_loop_run(mainLoop);
			g_main_loop_unref(mainLoop);
			mainLoop = NULL;
			TMPlayerDBusRelease();
			TMPlayer_Release();
		}
		else
		{
			(void)fprintf(stderr, "TC PLAYER Service initialize failed\n");
			ret = -1;
		}
	}
	else
	{
		(void)fprintf(stderr, "g_main_loop_new failed\n");
		ret = -1;
	}
	return ret;
}

static void SignalHandler(int32_t sig)
{
	if (mainLoop != NULL)
	{
		g_main_loop_quit(mainLoop);
		(void)fprintf(stderr, "[TC PLAYER Service] %s: received signal(%d)\n", __FUNCTION__, sig);
	}
}

static void Daemonize(void)
{
	pid_t pid;

	// create child process
	pid = fork();

	// fork failed
	if (pid < 0)
	{
		(void)fprintf(stderr, "fork failed\n");
		exit(1);
	}

	// parent process
	if (pid > 0)
	{
		// exit parent process for daemonize
		exit(0);
	}

	// umask the file mode
	(void)umask(0);

	// set new session
	if (setsid() < 0)
	{
		(void)fprintf(stderr ,"set new session failed\n");
		exit(1);
	}

	// change the current working directory for safety
	if (chdir("/") < 0)
	{
		(void)fprintf(stderr, "change directory failed\n");
		exit(1);
	}
}

