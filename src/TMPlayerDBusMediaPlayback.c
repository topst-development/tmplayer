/****************************************************************************************
 *   FileName    : TMPlayerDBusMediaPlayback.c
 *   Description : TMPlayerDBusMediaPlayback.c
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
#include <stdint.h>
#include <stdbool.h>
#include <dbus/dbus.h>
#include <pthread.h>
#include "DBusMsgDef.h"
#include "TCDBusRawAPI.h"
#include "TMPlayer.h"
#include "TMPlayerType.h"
#include "TMPlayerDBus.h"
#include "TMPlayerDBusMediaPlayback.h"
#include "TMPlayerDBusModeManager.h"

static int32_t s_prevID;
extern int32_t g_tc_debug;

#define DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF(format, arg...) \
	if (g_tc_debug != 0) \
	{ \
		(void)fprintf(stderr, "[TMPLAYER DBUS] %s: " format "", __FUNCTION__, ##arg); \
	}

static void MediaPlaybackSignalGetPlayID(DBusMessage *message, int32_t *playID);
static void MediaPlaybackSignalGetValue(DBusMessage *message, int32_t *value, int32_t *playID);
static void MediaPlaybackSignalGetiD3Info(DBusMessage *message, int32_t *category, const char **info, int32_t *playID);
static void MediaPlaybackSignalGetSize(DBusMessage *message, int32_t *playID, uint32_t* size);
static void MediaPlaybackSignalGetTime(DBusMessage *message, uint8_t *hour, uint8_t *min, uint8_t *sec, int32_t *playID);
static int32_t playbackStatus = MULTIMEDIA_NORMAL;


void MediaPlaybackSignalDBusProcess(uint32_t id, DBusMessage * message)
{
	int32_t ret;
	int32_t value;
	int32_t number, playID;
	uint32_t size;
	uint8_t hour, min, sec;
	const char * iD3Info;
	if(id < (uint32_t)TotalSignalMediaPlaybackEvents)
	{
		playID = TMPlayer_GetPlayID();
		if(id != SignalMediaPlaybackPlayPostion)
		{
			DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Received signal [%s(%d)]\n", g_signalMediaPlaybackEventNames[id], id);
		}
		switch(id)
		{
			case SignalMediaPlaybackPlaying :
				MediaPlaybackSignalGetPlayID(message, &number);
				if (number == playID)
				{
					TMPlayer_SetPlayStatus(PlayStatusPlaying);
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
			case SignalMediaPlaybackStopped :
				MediaPlaybackSignalGetPlayID(message, &number);
				NotifyMediaReleased();
				if ((number == s_prevID) && (playID != 0))
				{
					if(playbackStatus == MULTIMEDIA_BUSY)
					{
						TMPlayer_TrackRestartProcess();
						playbackStatus = MULTIMEDIA_NORMAL;
					}
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, s_prevID);
				}
				break;
			case SignalMediaPlaybackPaused :
				MediaPlaybackSignalGetPlayID(message, &number);
				if (number == playID)
				{
					TMPlayer_SetPlayStatus(PlayStatusPause);
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
			case SignalMediaPlaybackDuration :
				MediaPlaybackSignalGetTime(message, &hour, &min, &sec, &number);
				if (number == playID)
				{
					SendDBusTotalTimeChanged(hour, min, sec);
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
			case SignalMediaPlaybackPlayPostion :
				MediaPlaybackSignalGetTime(message, &hour, &min, &sec, &number);
				if (number == playID)
				{
					TMPlayer_UpdateTimeProcess(hour, min, sec);
					SendDBusPlayTimeChanged(hour, min, sec);
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
			case SignalMediaPlaybackTagInfo :
				MediaPlaybackSignalGetiD3Info(message, &value, &iD3Info, &number);
				if (number == playID)
				{
					SendDBusiD3Infomation(value, iD3Info);
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
			case SignalMediaPlaybackAlbumArtCompleted :
				MediaPlaybackSignalGetSize(message, &number, &size);
				if (number == playID)
				{
					SendDBusAlbumArtCompleted(number, size);
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
			case SignalMediaPlaybackPlayEnded :
				MediaPlaybackSignalGetPlayID(message, &number);
				if (number == playID)
				{
					TMPlayer_TrackUpProcess();
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
			case SignalMediaPlaybackSeekCompleted :
				SendDBusPlaySeekCompleted();
				break;
			case SignalMediaPlaybackError :
				MediaPlaybackSignalGetValue(message, &value, &number);
				if (number == playID)
				{
					SendDBusPlayFailed(value);
					TMPlayer_TrackUpProcess();
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
			case SignalMediaPlaybackSamplerate :
				MediaPlaybackSignalGetValue(message, &value, &number);
				if (number == playID)
				{
					SendDBusSamplerate(value);
				}
				else
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("Invalid Play ID (%d:%d)\n", number, playID);
				}
				break;
		}

	}
}

int32_t SendDBusMediaStart(int32_t content, const char * path, uint8_t hour, uint8_t min, uint8_t sec, int32_t playID, char pause)
{
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;
	int32_t ret;
	int32_t prev = 0;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_PLAY_START,
									DBUS_TYPE_STRING, &path,
									DBUS_TYPE_BYTE, &hour,
									DBUS_TYPE_BYTE, &min,
									DBUS_TYPE_BYTE, &sec,
									DBUS_TYPE_BYTE, &content,
									DBUS_TYPE_INT32, &playID,
									DBUS_TYPE_BYTE, &pause,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if(pending != NULL)
			{
				if(GetArgumentFromDBusPendingCall(pending,
													DBUS_TYPE_INT32, &ret,
													DBUS_TYPE_INT32, &prev,
													DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("%s: GetArgmentFromDBusPendingCall, Daemon Busy : %d \n", __FUNCTION__, ret);
					playbackStatus = ret;
					if (ret == MULTIMEDIA_BUSY) // Meida Playback is Busy
					{
						s_prevID = prev;
					}
					else // Play  Success
					{
						s_prevID = playID;
					}
				}
				else
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
	return ret;
}

int32_t SendDBusMediaStop(int32_t playID)
{
	int32_t ret = -1;
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("play ID: %d\n", playID);
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_PLAY_STOP,
									DBUS_TYPE_INT32, &playID,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if(pending != NULL)
			{
				if(GetArgumentFromDBusPendingCall(pending,
													DBUS_TYPE_INT32, &ret,
													DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("%s: GetArgmentFromDBusPendingCall, Stop return: %d \n", __FUNCTION__, ret);
				}
				else
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}

	return ret;
}

int32_t SendDBusMediaPause(int32_t playID)
{
	int32_t ret = -1;
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_PLAY_PAUSE,
									DBUS_TYPE_INT32, &playID,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if(pending != NULL)
			{
				if(GetArgumentFromDBusPendingCall(pending,
													DBUS_TYPE_INT32, &ret,
													DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("%s: GetArgmentFromDBusPendingCall, Pause return: %d \n", __FUNCTION__, ret);
				}
				else
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}

	return ret;
}

int32_t SendDBusMediaResume(int32_t playID)
{
	int32_t ret = -1;
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;
	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_PLAY_RESUME,
									DBUS_TYPE_INT32, &playID,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if(pending != NULL)
			{
				if(GetArgumentFromDBusPendingCall(pending,
													DBUS_TYPE_INT32, &ret,
													DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("%s: GetArgmentFromDBusPendingCall, Resume return: %d \n", __FUNCTION__, ret);
				}
				else
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}

	return ret;
}


int32_t SendDBusMediaSeek(uint8_t hour, uint8_t min, uint8_t sec, int32_t playID)
{
	int32_t ret = -1;
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_PLAY_SEEK,
									DBUS_TYPE_BYTE, &hour,
									DBUS_TYPE_BYTE, &min,
									DBUS_TYPE_BYTE, &sec,
									DBUS_TYPE_INT32, &playID,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if(pending != NULL)
			{
				if(GetArgumentFromDBusPendingCall(pending,
													DBUS_TYPE_INT32, &ret,
													DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("%s: GetArgmentFromDBusPendingCall, Seek return: %d \n", __FUNCTION__, ret);
				}
				else
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}

	return ret;
}

void SendDBusMediaGetAlbumArtKey(void)
{
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;
	int32_t number;
	uint32_t size;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_GET_ALBUMART_KEY,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if(pending != NULL)
			{
				if(GetArgumentFromDBusPendingCall(pending,
													DBUS_TYPE_INT32, &number,
													DBUS_TYPE_UINT32, &size,
													DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("%s: GetArgmentFromDBusPendingCall, Album Art Key %d (%u) \n",
																__FUNCTION__, number, size);
					SendDBusAlbumArtKey(number, size);
				}
				else
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

int32_t SendDBusMediaGetStatus(void)
{
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;
	int32_t ret;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_GET_STATUS,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if(pending != NULL)
			{
				if(GetArgumentFromDBusPendingCall(pending,
													DBUS_TYPE_INT32, &ret,
													DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("%s: GetArgmentFromDBusPendingCall, Daemon Busy : %d \n", __FUNCTION__, ret);
				}
				else
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
	return ret;
}

void SendDBusMediaSetDisplay(int32_t x, int32_t y, int32_t width, int32_t height)
{
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_SET_DISPLAY,
									DBUS_TYPE_INT32, &x,
									DBUS_TYPE_INT32, &y,
									DBUS_TYPE_INT32, &width,
									DBUS_TYPE_INT32, &height,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusMediaSetDualDisplay(int32_t x, int32_t y, int32_t width, int32_t height)
{
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_SET_DUAL_DISPLAY,
									DBUS_TYPE_INT32, &x,
									DBUS_TYPE_INT32, &y,
									DBUS_TYPE_INT32, &width,
									DBUS_TYPE_INT32, &height,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

int32_t SendDBusMediaGetCurrentPlayID(void)
{
	DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("\n");
	DBusMessage *message;
	int32_t cur_playID;

	message = CreateDBusMsgMethodCall(MEDIAPLAYBACK_PROCESS_DBUS_NAME,MEDIAPLAYBACK_PROCESS_OBJECT_PATH,
									MEDIAPLAYBACK_EVENT_INTERFACE,
									METHOD_MEDIAPLAYBACK_GET_PLAY_ID,
									DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if(pending != NULL)
			{
				if(GetArgumentFromDBusPendingCall(pending,
													DBUS_TYPE_INT32, &cur_playID,
													DBUS_TYPE_INVALID))
				{
					DEBUG_TMPLAYER_DBUS_MEDIAPLAYBACK_PRINTF("%s: GetArgmentFromDBusPendingCall, PlayID : %d \n", __FUNCTION__, cur_playID);
				}
				else
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
			}
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
	return cur_playID;
}

void NotifyMediaReleased(void)
{
	// Media Playback released Audio Resource
	TMPlayer_SetPlayStatus(PlayStatusStop);
	SendDBusModeManagerRscReleased(RSC_AUDIO);
}

int32_t GetPreviousID(void)
{
	return s_prevID;
}

void ClearPreviousID(void)
{
	s_prevID = 0;
}

static void MediaPlaybackSignalGetPlayID(DBusMessage *message, int32_t *playID)
{
	if (message != NULL)
	{
		if (!GetArgumentFromDBusMessage(message,
									   DBUS_TYPE_INT32, playID,
									   DBUS_TYPE_INVALID))
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
}

static void MediaPlaybackSignalGetValue(DBusMessage *message, int32_t *value, int32_t *playID)
{
	if (message != NULL)
	{
		if (!GetArgumentFromDBusMessage(message,
									   DBUS_TYPE_INT32, value,
									   DBUS_TYPE_INT32, playID,
									   DBUS_TYPE_INVALID))
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
}

static void MediaPlaybackSignalGetiD3Info(DBusMessage * message, int32_t * category, const char ** info, int32_t *playID)
{
	if (message != NULL)
	{
		if (!GetArgumentFromDBusMessage(message,
									  DBUS_TYPE_INT32, category,
									  DBUS_TYPE_STRING, info,
									   DBUS_TYPE_INT32, playID,
									  DBUS_TYPE_INVALID))
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
}

static void MediaPlaybackSignalGetSize(DBusMessage * message, int32_t *playID, uint32_t * size)
{
	if (message != NULL)
	{
		if (!GetArgumentFromDBusMessage(message,
									  DBUS_TYPE_INT32, playID,
									  DBUS_TYPE_UINT32, size,
									  DBUS_TYPE_INVALID))
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
}

static void MediaPlaybackSignalGetTime(DBusMessage * message, uint8_t *hour, uint8_t *min, uint8_t *sec, int32_t *playID)
{
	if (message != NULL)
	{
		if (!GetArgumentFromDBusMessage(message,
									   DBUS_TYPE_BYTE, hour,
									   DBUS_TYPE_BYTE, min,
									   DBUS_TYPE_BYTE, sec,
									   DBUS_TYPE_INT32, playID,
									   DBUS_TYPE_INVALID))
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
}

