/****************************************************************************************
 *   FileName    : TMPlayerDBusModeManager.c
 *   Description : 
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited 
to re-distribution in source or binary form is strictly prohibited.
This source code is provided “AS IS” and nothing contained in this source code 
shall constitute any express or implied warranty of any kind, including without limitation, 
any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, 
copyright or other third party intellectual property right. 
No warranty is made, express or implied, regarding the information’s accuracy, 
completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, 
out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <dbus/dbus.h>
#include "DBusMsgDef.h"
#include "DBusMsgDef_MM.h"
#include "TCDBusRawAPI.h"
#include "TMPlayer.h"
#include "TMPlayerDBusModeManager.h"

extern int g_tc_debug;
extern int g_appID;
extern bool _audioResource;
extern bool _displayResource;

#define DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF(format, arg...) \
	if (g_tc_debug) \
	{ \
		fprintf(stderr, "[TMPLAYER DBUS MODEMANAGER] %s: " format "", __FUNCTION__, ##arg); \
	}

static int s_releaseResource;
static bool s_audioReleaseCompleted = true;
static bool s_displayReleaseCompleted = true;
static int s_resourceStatus = DO_NOTHING;

void ReleaseAllResource(int status)
{
	DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("Release All Resource (%d) D%d, A%d\n",
											status, _displayResource, _audioResource);

	if (_audioResource || _displayResource)
	{
		s_resourceStatus = status;

		if (_displayResource)
		{
			// When hiding GUI is completed, this variable is set to true
			s_displayReleaseCompleted = false;
			TMPlayer_DisplayOff();
		}

		if (_audioResource)
		{
			// When releasing media is completed, this variable is set to true
			s_audioReleaseCompleted = false;
			TMPlayer_AudioOff();
		}
	}
	else // if AV Player no resource, send change mode idle right away
	{
		if (s_resourceStatus == DO_IDLE)
		{
			SendDBusModeManagerChangeMode(IDLE);
		}
		else if (s_resourceStatus == DO_RELEASE_RESOURCE)
		{
			SendDBusModeManagerReleaseResourceDone();
		}
		else
		{
			s_resourceStatus = DO_NOTHING;
		}
	}
}

void ModeManagerSignalDBusProcess(unsigned int index, DBusMessage *message)
{
	int rsc, cur_rsc, app;
	const char *mode;

	switch (index)
	{
		case Release_Resource:
			GetArgumentFromDBusMessage(message, DBUS_TYPE_INT32, &rsc,
										DBUS_TYPE_INT32, &app, DBUS_TYPE_INVALID);

			if (app == g_appID)
			{
				DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("mode manager relese resource signal: rsc(%x), App %d \n", rsc, app);
				s_releaseResource |= rsc;

				if (_audioResource || _displayResource)
				{
					s_resourceStatus = DO_RELEASE_RESOURCE;

					if (rsc & RSC_DISPLAY)
					{
						// When hiding GUI is completed, this variable is set to true
						s_displayReleaseCompleted = false;
						TMPlayer_DisplayOff();
					}

					if (rsc & RSC_AUDIO)
					{
						// When releasing media is completed, this variable is set to true
						s_audioReleaseCompleted = false;
						TMPlayer_AudioOff();
					}
				}
				else // if no resource, AV Player sends release resource done right away
				{
					SendDBusModeManagerReleaseResourceDone();
				}
			}

			break;
		case Changed_Mode:
			GetArgumentFromDBusMessage(message, DBUS_TYPE_STRING, &mode,
										DBUS_TYPE_INT32, &app, DBUS_TYPE_INVALID);

			if (app == g_appID)
			{
				DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("mode manager signal: mode %s, App %d \n", mode, app);

				if (strncmp(mode, VIEW, strlen(VIEW)) == 0)
				{
					DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("VIEW\n");
					TMPlayer_DisplayOn(true);
				}
				else if (strncmp(mode, AUDIOBGPLAY, strlen(AUDIOBGPLAY)) == 0)
				{
					DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("Audio Background Play\n");
					TMPlayer_AudioOn(true);
				}
				else if (strncmp(mode, AUDIOPLAY, strlen(AUDIOPLAY)) == 0)
				{
					DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("Audio Play\n");
					TMPlayer_AudioOn(false);
				}
				else if (strncmp(mode, VIDEOPLAY, strlen(VIDEOPLAY)) == 0)
				{
					DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("Video Play\n");
					TMPlayer_VideoOn();
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("Invalid mode: %s\n", mode);
				}
			}
			break;
		case EndedMode:
			break;
		case SuspendMode:
			GetArgumentFromDBusMessage(message, DBUS_TYPE_INVALID);

			DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("Suspend\n");

			TMPlayer_Suspend();
			break;
		case ResumeMode:
			GetArgumentFromDBusMessage(message, DBUS_TYPE_INVALID);

			DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("Resume!\n");

			TMPlayer_Resume();
			break;
		default:
			break;
	}
}

int SendDBusModeManagerGoHome(void)
{
	int ret = 0;
	int app = LAUNCHERAPPID;
	const char *mode = "home";
	DBusMessage *message;
	message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME,
										MODEMANAGER_PROCESS_OBJECT_PATH,
										MODEMANAGER_EVENT_INTERFACE,
										CHANGE_MODE,
										DBUS_TYPE_STRING, &mode,
										DBUS_TYPE_INT32, &app, DBUS_TYPE_INVALID); // Create DBus Message

	DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("%s: Mode(%s), app(%d)\n", __FUNCTION__, mode, app);

	if (message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if (pending != NULL)
			{
				if (GetArgumentFromDBusPendingCall(pending,
												DBUS_TYPE_INT32, &ret,
												DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("%s: GetArgmentFromDBusPendingCall, ret: %d \n", __FUNCTION__, ret);
				}
				else
				{
					fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		else
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}

	return ret;
}

int SendDBusModeManagerGoBack(void)
{
	int ret = 0;
	int app = LAUNCHERAPPID;
	const char *mode = "view";
	DBusMessage *message;
	message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME,
										MODEMANAGER_PROCESS_OBJECT_PATH,
										MODEMANAGER_EVENT_INTERFACE,
										CHANGE_MODE,
										DBUS_TYPE_STRING, &mode,
										DBUS_TYPE_INT32, &app, DBUS_TYPE_INVALID); // Create DBus Message

	DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("%s: Mode(%s), app(%d)\n", __FUNCTION__, mode, app);

	if (message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if (pending != NULL)
			{
				if (GetArgumentFromDBusPendingCall(pending,
												DBUS_TYPE_INT32, &ret,
												DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("%s: GetArgmentFromDBusPendingCall, ret: %d \n", __FUNCTION__, ret);
				}
				else
				{
					fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		else
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}

	return ret;
}

void SendDBusModeManagerRscReleased(int rsc)
{
	if (rsc & RSC_AUDIO) // If Audio Resource is released
	{
		s_audioReleaseCompleted = true;
	}
	else if (rsc & RSC_DISPLAY) // If Display Resource is released
	{
		s_displayReleaseCompleted = true;
	}

	// When both releasing audio and releasing display are done
	// tmplayer sends release resource done or change mode idle
	if (s_audioReleaseCompleted && s_displayReleaseCompleted)
	{
		if (s_resourceStatus == DO_RELEASE_RESOURCE)
		{
			SendDBusModeManagerReleaseResourceDone();
		}
		else if (s_resourceStatus == DO_IDLE)
		{
			SendDBusModeManagerChangeMode(IDLE);
		}
		else if (s_resourceStatus == DO_SUSPEND)
		{
			SendDBusModeManagerGoHome();
		}

		s_resourceStatus = DO_NOTHING;
	}
}

int SendDBusModeManagerChangeMode(const char* mode)
{
	int ret = 0;
	int app = g_appID;
	DBusMessage *message;
	message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME,
										MODEMANAGER_PROCESS_OBJECT_PATH,
										MODEMANAGER_EVENT_INTERFACE,
										CHANGE_MODE,
										DBUS_TYPE_STRING, &mode,
										DBUS_TYPE_INT32, &app, DBUS_TYPE_INVALID); // Create DBus Message

	DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("%s: Mode(%s), app(%d)\n", __FUNCTION__, mode, app);

	if (message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if (pending != NULL)
			{
				if (GetArgumentFromDBusPendingCall(pending,
												DBUS_TYPE_INT32, &ret,
												DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("%s: GetArgmentFromDBusPendingCall, ret: %d \n", __FUNCTION__, ret);
				}
				else
				{
					fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		else
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}

	return ret;
}

void SendDBusModeManagerReleaseResourceDone(void)
{
	int resource = s_releaseResource;
	int app = g_appID;
    DBusMessage *message;
    message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME,
										MODEMANAGER_PROCESS_OBJECT_PATH,
										MODEMANAGER_EVENT_INTERFACE,
										RELEASE_RESOURCE_DONE,
										DBUS_TYPE_INT32, &resource,
										DBUS_TYPE_INT32, &app,
										DBUS_TYPE_INVALID);

	DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("%s: Resource(%x), app(%d)\n", __FUNCTION__, resource, app);

	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}

	s_releaseResource = RSC_NONE;
}

void SendDBusModeManagerEndMode(const char *mode)
{
    DBusMessage *message;
	int app = g_appID;
    message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME,
										MODEMANAGER_PROCESS_OBJECT_PATH,
										MODEMANAGER_EVENT_INTERFACE,
										END_MODE,
										DBUS_TYPE_STRING, &mode,
										DBUS_TYPE_INT32, &app,
										DBUS_TYPE_INVALID);

	DEBUG_TMPLAYER_DBUS_MODEMANAGER_PRINTF("%s: Mode(%s), app(%d)\n", __FUNCTION__, mode, app);

	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusModeManagerErrorOccur(void)
{
	int app = g_appID;
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(MODEMANAGER_PROCESS_DBUS_NAME,
										MODEMANAGER_PROCESS_OBJECT_PATH,
										MODEMANAGER_EVENT_INTERFACE,
										MODE_ERROR_OCCURED,
										DBUS_TYPE_INT32, &app,
										DBUS_TYPE_INVALID);

	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}
