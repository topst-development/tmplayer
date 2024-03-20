/****************************************************************************************
 *   FileName    : TMPlayerDBus.c
 *   Description : TMPlayerDBus.c
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
#include <stdbool.h>
#include <string.h>
#include <glib.h>
#include <dbus/dbus.h>
#include "DBusMsgDef.h"
#include "DBusMsgDef_MM.h"
#include "TCDBusRawAPI.h"
#include "TCTagParser.h"
#include "TMPlayer.h"
#include "TMPlayerConnect.h"
#include "TMPlayerType.h"
#include "TMPlayerDBusMediaPlayback.h"
#include "TMPlayerDBusModeManager.h"
#ifdef HAVE_IAP2
#include "TMPlayerDBusiAP2Manager.h"
#include "TMPlayeriAP2.h"
#include "DBusMsgDef_iAP2.h"
#endif
#ifdef HAVE_BT
#include "TMPlayerDBusBTManager.h"
#include "TMPlayerBT.h"
#include "DBusMsgDef_BT.h"
#endif

#include "TMPlayerDBus.h"
#include "TMPlayerAux.h"
#include "TMPlayerControl.h"

int32_t g_appID = -1;
extern int32_t g_tc_debug;

#define DEBUG_TMPLAYER_DBUS_PRINTF(format, arg...) \
	if (g_tc_debug != 0) \
	{ \
		(void)fprintf(stderr, "[TMPLAYER DBUS] %s: " format "", __FUNCTION__, ##arg); \
	}

typedef void (*DBusMethodCallFunction)(DBusMessage *message);

static DBusMsgErrorCode OnReceivedDBusSignal(DBusMessage *message, const char *interface);
static DBusMsgErrorCode OnReceivedMethodCall(DBusMessage *message, const char *interface);

static void DBusMethodSetAppID(DBusMessage *message);
static void DBusMethodConnectDebug(DBusMessage *message);
static void DBusMethodDBDebug(DBusMessage *message);
static void DBusMethodChangeContentMode(DBusMessage *message);
static void DBusMethodChangeNextDevice(DBusMessage *message);
static void DBusMethodChangeDevice(DBusMessage *message);
static void DBusMethodSetBrowsingMode(DBusMessage *message);
static void DBusMethodChangeRepeatMode(DBusMessage *message);
static void DBusMethodChangeShuffleMode(DBusMessage *message);
static void DBusMethodTrackStart(DBusMessage *message);
static void DBusMethodTrackStop(DBusMessage *message);
static void DBusMethodTrackResume(DBusMessage *message);
static void DBusMethodTrackPause(DBusMessage *message);
static void DBusMethodTrackUp(DBusMessage *message);
static void DBusMethodTrackDown(DBusMessage *message);
static void DBusMethodTrackMove(DBusMessage *message);
static void DBusMethodTrackSeek(DBusMessage *message);
static void DBusMethodTrackForward(DBusMessage *message);
static void DBusMethodTrackRewind(DBusMessage *message);
static void DBusMethodTrackFFREWind(DBusMessage * message);
static void DBusMethodUpdateDisplayInfo(DBusMessage *message);
static void DBusMethodUpdateExtDisplayInfo(DBusMessage *message);
static void DBusMethodRequestFileList(DBusMessage *message);
static void DBusMethodMetaBrowsingStart(DBusMessage *message);
static void DBusMethodMetaBrowsingRequest(DBusMessage *message);
static void DBusMethodMetaBrowsingSelect(DBusMessage *message);
static void DBusMethodMetaBrowsingHome(DBusMessage *message);
static void DBusMethodMetaBrowsingUndo(DBusMessage *message);
static void DBusMethodMetaBrowsingEnd(DBusMessage *message);
static void DBusMethodRequestDisplay(DBusMessage * message);
static void DBusMethodGoLauncherHome(DBusMessage * message);
static void DBusMethodGoLauncherBack(DBusMessage * message);
static void DBusMethodGoAVOnOff(DBusMessage * message);
static void DBusMethodNotifyDisplayDone(DBusMessage * message);

static DBusMethodCallFunction DBusMethodProcess[TotalMethodTMPlayerEvent] = {
	DBusMethodSetAppID,
	DBusMethodConnectDebug,
	DBusMethodDBDebug,
	DBusMethodChangeContentMode,
	DBusMethodChangeNextDevice,
	DBusMethodChangeDevice,
	DBusMethodSetBrowsingMode,
	DBusMethodChangeRepeatMode,
	DBusMethodChangeShuffleMode,
	DBusMethodTrackStart,
	DBusMethodTrackStop,
	DBusMethodTrackResume,
	DBusMethodTrackPause,
	DBusMethodTrackUp,
	DBusMethodTrackDown,
	DBusMethodTrackMove,
	DBusMethodTrackSeek,
	DBusMethodTrackForward,
	DBusMethodTrackRewind,
	DBusMethodTrackFFREWind,
	DBusMethodUpdateDisplayInfo,
	DBusMethodUpdateExtDisplayInfo,
	DBusMethodRequestFileList,
	DBusMethodMetaBrowsingStart,
	DBusMethodMetaBrowsingRequest,
	DBusMethodMetaBrowsingSelect,
	DBusMethodMetaBrowsingHome,
	DBusMethodMetaBrowsingUndo,
	DBusMethodMetaBrowsingEnd,
	DBusMethodRequestDisplay,
	DBusMethodGoLauncherHome,
	DBusMethodGoLauncherBack,
	DBusMethodGoAVOnOff,
	DBusMethodNotifyDisplayDone
};

void TMPlayerDBusInitialize(void)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	SetDBusPrimaryOwner(TMPLAYER_PROCESS_DBUS_NAME);
	SetCallBackFunctions(OnReceivedDBusSignal, OnReceivedMethodCall);
	(void)AddMethodInterface(TMPLAYER_EVENT_INTERFACE);
	(void)AddSignalInterface(MEDIAPLAYBACK_EVENT_INTERFACE);
	(void)AddSignalInterface(MODEMANAGER_EVENT_INTERFACE);
#ifdef HAVE_IAP2
	(void)AddMethodInterface(TMPLAYER_IAP2_INTERFACE);
	(void)AddMethodInterface(IAP2_FACILITY_FEATURE_INTERFACE);
	(void)AddSignalInterface(IAP2_PROCESS_INTERFACE);
	(void)AddSignalInterface(ADM_MANAGER_INTERFACE);
#endif
#ifdef HAVE_BT
	(void)AddSignalInterface(BLUETOOTH_APPLICATION_INTERFACE);
#endif	
	InitializeRawDBusConnection("TC PLAYER DBUS");
}

void TMPlayerDBusRelease(void)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
#ifdef HAVE_IAP2
	TMPlayeriAP2_Release();
#endif
#ifdef HAVE_BT
	TMPlayerBT_Release();
#endif
	ReleaseRawDBusConnection();
}

void SendDBusDeviceConnect(int32_t device)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerDeviceConnected],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerDeviceConnected], device);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusDeviceDisconnect(int32_t device)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerDeviceDisConnected],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerDeviceDisConnected], device);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}



void SendDBusDeviceMountStop(int32_t device)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerDeviceMountStop],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerDeviceMountStop], device);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusFileDBCompleted(int32_t device)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerFileDBCompleted],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerFileDBCompleted], device);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusFileList(int32_t device, int32_t content, int32_t number, char ** fileList, int32_t count)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;
	int32_t i;
	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerFileList],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_BYTE, &content,
								  DBUS_TYPE_INT32, &number,
								  DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &fileList, count, 
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s \n", g_signalTMPlayerEventNames[SignalTMPlayerFileList]);
			for(i = 0; i < count; i++)
			{
				DEBUG_TMPLAYER_DBUS_PRINTF("%s \n", fileList[i]);
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
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusFileListChanged(int32_t device, int32_t content, int32_t number, int32_t totalnumber)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerFileListChanged],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_BYTE, &content,
								  DBUS_TYPE_INT32, &number,
								  DBUS_TYPE_INT32, &totalnumber,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s\n", g_signalTMPlayerEventNames[SignalTMPlayerFileListChanged]);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusFileNameChanged(const char* FileName, uint32_t length)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerFileNameChanged],
								  DBUS_TYPE_STRING, &FileName,
								  DBUS_TYPE_UINT32, &length,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : filename : %s length : %d \n", g_signalTMPlayerEventNames[SignalTMPlayerFileNameChanged], FileName, length);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusFileNumberChanged(int32_t FileNumber)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerFileNumberChanged],
								  DBUS_TYPE_INT32, &FileNumber,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerFileNumberChanged], FileNumber);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusFileNumberUpdated(int32_t device, int32_t content, int32_t number, int32_t totalnumber)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerFileNumberUpdated],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_BYTE, &content,
								  DBUS_TYPE_INT32, &number,
								  DBUS_TYPE_INT32, &totalnumber,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d %d %d %d\n", g_signalTMPlayerEventNames[SignalTMPlayerFileNumberUpdated], device, content, number, totalnumber);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusFolderNameChanged(const char* FolderName, uint32_t length)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerFolderNameChanged],
								  DBUS_TYPE_STRING, &FolderName,
								  DBUS_TYPE_UINT32, &length,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : filename : %s length : %d \n", g_signalTMPlayerEventNames[SignalTMPlayerFolderNameChanged], FolderName, length);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusiD3Infomation(int32_t category, const char * info)
{
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerID3Information],
								  DBUS_TYPE_INT32, &category,
								  DBUS_TYPE_STRING, &info,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : category : %d info : %s \n", g_signalTMPlayerEventNames[SignalTMPlayerID3Information], category, info);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusAlbumArtKey(int32_t key, uint32_t size)
{
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerAlbumArtkey],
								  DBUS_TYPE_INT32, &key,
								  DBUS_TYPE_UINT32, &size,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : key : %d size : %d \n", g_signalTMPlayerEventNames[SignalTMPlayerAlbumArtkey], key, size);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusAlbumArtCompleted(int32_t number, uint32_t length)
{
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerAlbumArtCompleted],
								  DBUS_TYPE_INT32, &number,
								  DBUS_TYPE_UINT32, &length,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : number : %d length : %d \n", g_signalTMPlayerEventNames[SignalTMPlayerAlbumArtCompleted], number, length);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusAlbumArtShmData(const char *uri, uint32_t size)
{
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerAlbumArtShmData],
								  DBUS_TYPE_STRING, &uri,
								  DBUS_TYPE_UINT32, &size,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : uri : %s size : %d \n", g_signalTMPlayerEventNames[SignalTMPlayerAlbumArtShmData], uri, size);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusNoSong(int32_t device)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerNoSong],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerNoSong], device);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusNoDevice(void)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerNoDevice],
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s \n", g_signalTMPlayerEventNames[SignalTMPlayerNoDevice]);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusMetaAddInfo(uint32_t dbNum, int32_t idx, const char * name, uint32_t type)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerMetaAddInfo],
								  DBUS_TYPE_BYTE, &dbNum,
								  DBUS_TYPE_INT32, &idx,
								  DBUS_TYPE_STRING, &name,
								  DBUS_TYPE_UINT32, &type,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBus %s : %d, %d, %s %u\n",  g_signalTMPlayerEventNames[SignalTMPlayerMetaAddInfo], dbNum, idx, name, type);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusMetaCountChagned(uint32_t dbNum, uint32_t totalNum)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerMetaCountChanged],
								  DBUS_TYPE_BYTE, &dbNum,
								  DBUS_TYPE_UINT32, &totalNum,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBus %s : %d %d \n",  g_signalTMPlayerEventNames[SignalTMPlayerMetaCountChanged], dbNum, totalNum);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}	
}

void SendDBusMetaCreateTrackList(uint32_t dbNum, int32_t count)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerMetaCreateTrackList],
								  DBUS_TYPE_BYTE, &dbNum,
								  DBUS_TYPE_UINT32, &count,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBus %s : %d %d \n",  g_signalTMPlayerEventNames[SignalTMPlayerMetaCreateTrackList], dbNum, count);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}		
}

void SendDBusMetaMenuChanged(uint32_t dbNum, uint8_t mode, const char * menu)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerMetaMenuChanged],
								  DBUS_TYPE_BYTE, &dbNum,
								  DBUS_TYPE_BYTE, &mode,
								  DBUS_TYPE_STRING, &menu,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBus %s : %d, %d, %s \n",  g_signalTMPlayerEventNames[SignalTMPlayerMetaMenuChanged], dbNum, mode, menu);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusContentChanged(int32_t contentType)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerContentChanged],
								  DBUS_TYPE_BYTE, &contentType,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerContentChanged], contentType);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusDeviceChanged(int32_t device, int32_t contentType)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerDeviceChagned],
								  DBUS_TYPE_BYTE, &device, 
								  DBUS_TYPE_BYTE, &contentType,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d, %d\n", g_signalTMPlayerEventNames[SignalTMPlayerDeviceChagned], device, contentType);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusTotalNumberChanged(int32_t device, int32_t content, int32_t number)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerTotalNumberChanged],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_BYTE, &content,
								  DBUS_TYPE_INT32, &number,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : device : %d, content : %d, number : %d \n",
			g_signalTMPlayerEventNames[SignalTMPlayerTotalNumberChanged], device, content, number);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusPlayTimeChanged(uint8_t hour, uint8_t min, uint8_t sec)
{
//	DEBUG_TMPLAYER_DBUS_PRINTF("\n");	
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerPlayTimeChanged],
								  DBUS_TYPE_BYTE, &hour,
								  DBUS_TYPE_BYTE, &min,
								  DBUS_TYPE_BYTE, &sec,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			//DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : Time %d : %d : %d \n",
			//		g_signalTMPlayerEventNames[SignalTMPlayerPlayTimeChanged], hour, min, sec);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusPlaySeekCompleted(void)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerSeekCompleted],
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s \n", g_signalTMPlayerEventNames[SignalTMPlayerSeekCompleted]);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusResetInfomation(void)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerResetInormation],
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s \n", g_signalTMPlayerEventNames[SignalTMPlayerResetInormation]);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusTotalTimeChanged(uint8_t hour, uint8_t min, uint8_t sec)
{
//	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerTotalTimeChanged],
								  DBUS_TYPE_BYTE, &hour,
								  DBUS_TYPE_BYTE, &min,
								  DBUS_TYPE_BYTE, &sec,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			//DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : Time %d : %d : %d \n",
			//		g_signalTMPlayerEventNames[SignalTMPlayerTotalTimeChanged], hour, min, sec);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusRepeatModeChanged(uint32_t repeatMode)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerRepeatModeChanged],
								  DBUS_TYPE_UINT32, &repeatMode,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerRepeatModeChanged], repeatMode);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusShuffleModeChanged(uint32_t shuffleMode)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerShuffleModeChanged],
								  DBUS_TYPE_UINT32, &shuffleMode,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerShuffleModeChanged], shuffleMode);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusPlayStateChanged(int32_t status)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerPlayStateChanged],
								  DBUS_TYPE_BYTE, &status,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerPlayStateChanged], status);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusPlayFailed(int32_t code)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerPlayFailed],
								  DBUS_TYPE_INT32, &code,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerPlayFailed], code);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusSamplerate(int32_t samplerate)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerSamplerate],
								  DBUS_TYPE_INT32, &samplerate,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerSamplerate], samplerate);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusDeviceReady(int32_t device)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerDeviceReady],
								  DBUS_TYPE_BYTE, &device,
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s : %d\n", g_signalTMPlayerEventNames[SignalTMPlayerDeviceReady], device);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusDisplayOn(void)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerDisplayOn],
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s\n", g_signalTMPlayerEventNames[SignalTMPlayerDisplayOn]);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

void SendDBusDisplayOff(void)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	DBusMessage *message;

	message = CreateDBusMsgSignal(TMPLAYER_PROCESS_OBJECT_PATH, TMPLAYER_EVENT_INTERFACE,
								  g_signalTMPlayerEventNames[SignalTMPlayerDisplayOff],
								  DBUS_TYPE_INVALID);
	if(message != NULL)
	{
		if(SendDBusMessage(message, NULL) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("Send DBUS %s\n", g_signalTMPlayerEventNames[SignalTMPlayerDisplayOff]);
		}
		else
		{
			(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}
		dbus_message_unref(message);
	}
	else
	{
		(void)fprintf(stderr, "%s: Craete DBusMsgSignal failed\n", __FUNCTION__);
	}
}

static DBusMsgErrorCode OnReceivedDBusSignal(DBusMessage *message, const char *interface)
{
	int32_t rsc, app;
	const char *mode;
	DBusMsgErrorCode error = ErrorCodeNoError;

	if ((message != NULL) &&
		(interface != NULL))
	{
		uint32_t idx;
		uint32_t stop = 0;

		if (strcmp(interface, MEDIAPLAYBACK_EVENT_INTERFACE) == 0)
		{
			for (idx = (uint32_t)SignalMediaPlaybackPlaying; idx < (uint32_t)TotalSignalMediaPlaybackEvents && (stop == (uint32_t)0); idx++)
			{
				if (dbus_message_is_signal(message,
										   MEDIAPLAYBACK_EVENT_INTERFACE,
										   g_signalMediaPlaybackEventNames[idx]))
				{
					MediaPlaybackSignalDBusProcess(idx, message);
					stop = 1;
				}
			}
		}
		else if (strcmp(interface, MODEMANAGER_EVENT_INTERFACE) == 0) // Process Mode Manager's Signal
		{
			for (idx = Changed_Mode; idx < TotalSignalModeManagerEvent && !stop; idx++)
			{
				if (dbus_message_is_signal(message,
											MODEMANAGER_EVENT_INTERFACE,
											g_signalModeManagerEventNames[idx]))
				{
					ModeManagerSignalDBusProcess(idx, message);
					stop = 1;
				}
			}
		}
#ifdef HAVE_IAP2
		else if (strcmp(interface, IAP2_PROCESS_INTERFACE) == 0)
		{
			for (idx = SignaliAP2EventParcel; idx < TotalSignaliAP2Events && !stop; idx++)
			{
				if (dbus_message_is_signal(message,
											IAP2_PROCESS_INTERFACE,
											g_signaliAP2EventNames[idx]))
				{
					iAP2SignalDBusProcess(idx, message);
					stop = 1;
				}
			}
		}
		else if (strcmp(interface, ADM_MANAGER_INTERFACE) == 0)
		{
			for (idx = SignalAppleDeviceManageriAP1Connected; idx < TotalSignalAppleDeviceManagerEvents && !stop; idx++)
			{
				if (dbus_message_is_signal(message,
											ADM_MANAGER_INTERFACE,
											g_signalADMEventNames[idx]))
				{
					AppleDeviceManagerSignalDBusProcess(idx, message);
					stop = 1;
				}
			}
		}
#endif
#ifdef HAVE_BT
		else if (strcmp(interface, BLUETOOTH_APPLICATION_INTERFACE) == 0)
		{
			for (idx = SignalBTEventConnected; idx < TotalSignalBTEvents && !stop; idx++)
			{
				if (dbus_message_is_signal(message,
											BLUETOOTH_APPLICATION_INTERFACE,
											g_signalBTEventNames[idx]))
				{
					BluetoothSignalDBusProcess(idx, message);
					stop = 1;
				}
			}
		}
#endif

/*		if (!stop)
		{
			error = ErrorCodeUnknown;
		}
*/
	}

	return error;
}

static DBusMsgErrorCode OnReceivedMethodCall(DBusMessage *message, const char *interface)
{
	DBusMsgErrorCode error = ErrorCodeNoError;	
	
	if (interface != NULL &&
		strcmp(interface, TMPLAYER_EVENT_INTERFACE) == 0)
	{
		uint32_t idx;
		uint32_t stop = 0;
		for (idx = MethodTMPlayerSetAppID; idx < TotalMethodTMPlayerEvent && !stop; idx++)
		{
			if (dbus_message_is_method_call(message, 
											TMPLAYER_EVENT_INTERFACE, 
											g_methodTMPlayerEventNames[idx]))
			{
				DEBUG_TMPLAYER_DBUS_PRINTF("method : %d, %s \n", idx, g_methodTMPlayerEventNames[idx]);
				DBusMethodProcess[idx](message);
				stop = 1;
			}
		}
	}
#ifdef HAVE_IAP2
	else if (message != NULL && interface != NULL &&
				strcmp(interface, TMPLAYER_IAP2_INTERFACE) == 0)
    {
		if (dbus_message_is_method_call(message,
										TMPLAYER_IAP2_INTERFACE, 
										METHOD_PLAYER_IAP2_NOTIFY))
		{
			DBusMethodiAP2Notify(message);
		}
		else if (dbus_message_is_method_call(message,
											TMPLAYER_IAP2_INTERFACE, 
											METHOD_PLAYER_LOG_ENABLE))
		{
			DBusMethodPlayerLogEnable(message);
		}
		else if (dbus_message_is_method_call(message,
											TMPLAYER_IAP2_INTERFACE,
											METHOD_PLAYER_IAP2_CHANGED_PLAYER))
		{
			DBusMethodChangePlayer(message);
		}
    }
#endif
	return error;
}

static void DBusMethodSetAppID(DBusMessage *message)
{
	int32_t ret = -1;
	int32_t appID = 0;

	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	if (message != NULL)
	{
		DBusMessage* returnMessage;
		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_INT32, &appID,
										DBUS_TYPE_INVALID) != 0)
		{
			DEBUG_TMPLAYER_DBUS_PRINTF("App ID is %d\n", appID);
			if (appID > 0)
			{
				g_appID = appID;
				ret = 1;
			}

			returnMessage = CreateDBusMsgMethodReturn(message,
													DBUS_TYPE_INT32, &ret,
													DBUS_TYPE_INVALID);
			if(returnMessage != NULL)
			{
				if(SendDBusMessage(returnMessage, NULL) == 0)
				{
					(void)fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_message_unref(returnMessage);
			}

#ifdef HAVE_IAP2
			TMPlayeriAP2_Initialize();
#endif
#ifdef HAVE_BT
			// reserved
#endif
			TMPlayer_DetectEnable();
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
}

static void DBusMethodConnectDebug(DBusMessage *message)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	if(message != NULL)
	{
		dbus_bool_t debug;
		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BOOLEAN, &debug,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayerConnect_Debug(debug);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
}

static void DBusMethodDBDebug(DBusMessage * message)
{
	DEBUG_TMPLAYER_DBUS_PRINTF("\n");
	if(message != NULL)
	{
		dbus_bool_t debug;
		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BOOLEAN, &debug,
										DBUS_TYPE_INVALID) != 0)
		{
			//libtcdb debug func
			TMPlayerDB_Debug(debug);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
}

static void DBusMethodChangeContentMode(DBusMessage * message)
{
	(void)message;
	TMPlayer_ChangeContentTypeProcess();
}
static void DBusMethodChangeNextDevice(DBusMessage *message)
{
	if (message != NULL)
	{
		dbus_bool_t play = FALSE;

		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BOOLEAN, &play,
										DBUS_TYPE_INVALID) != 0)
		{
			if (play == TRUE)
			{
				TMPlayer_ChangeNextDeviceProcess(true);
			}
			else
			{
				TMPlayer_ChangeNextDeviceProcess(false);
			}
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}
static void DBusMethodChangeDevice(DBusMessage * message)
{
	if (message != NULL)
	{
		int32_t device = TotalDeviceSources;
		uint8_t dev;
		dbus_bool_t play = FALSE;

		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_BYTE, &dev,
										DBUS_TYPE_BOOLEAN, &play,
										DBUS_TYPE_INVALID) != 0)
		{
			(void)fprintf(stderr, "device : %ud \n", dev);
			device = (int32_t)dev;
			if (play == TRUE)
			{
				TMPlayer_ChangeDeviceProcess(device, true);
			}
			else
			{
				TMPlayer_ChangeDeviceProcess(device, false);
			}
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodSetBrowsingMode(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t device = 0;
		uint8_t content = 0;
		dbus_bool_t meta = FALSE;

		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_UINT32, &device,
										DBUS_TYPE_BYTE, &content,
										DBUS_TYPE_BOOLEAN, &meta,
										DBUS_TYPE_INVALID) != 0)
		{
			if (meta == TRUE)
			{
				TMPlayer_SetBrowsingMode(device, content, true);
			}
			else
			{
				TMPlayer_SetBrowsingMode(device, content, false);
			}
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodChangeRepeatMode(DBusMessage * message)
{
	if (message != NULL)
	{
		uint8_t mode = 0;

		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_ChangeRepeatModeProcess();
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodChangeShuffleMode(DBusMessage * message)
{
	if (message != NULL)
	{
		uint8_t mode = 0;

		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_ChangeShuffleModeProcess();
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodTrackStart(DBusMessage * message)
{
	(void)message;
	TMPlayer_TrackStartProcess();
}

static void DBusMethodTrackStop(DBusMessage * message)
{
	(void)message;
	TMPlayer_TrackStopProcess();
}

static void DBusMethodTrackResume(DBusMessage * message)
{
	(void)message;
	TMPlayer_TrackResumeProcess();
}

static void DBusMethodTrackPause(DBusMessage * message)
{
	(void)message;
	TMPlayer_TrackPauseProcess();
}

static void DBusMethodTrackUp(DBusMessage * message)
{
	(void)message;
	TMPlayer_TrackUpProcess();
}

static void DBusMethodTrackDown(DBusMessage * message)
{
	(void)message;
	TMPlayer_TrackDownProcess();
}

static void DBusMethodTrackMove(DBusMessage * message)
{
	if (message != NULL)
	{
		int32_t number = 0;

		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_INT32, &number,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_TrackMoveProcess(number);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodTrackSeek(DBusMessage * message)
{
	if (message != NULL)
	{
		uint8_t hour = 0, min = 0, sec = 0;

		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BYTE, &hour,
										DBUS_TYPE_BYTE, &min,
										DBUS_TYPE_BYTE, &sec,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_TrackSeekProcess(hour, min, sec);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodTrackForward(DBusMessage * message)
{
	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_TrackForwardProcess();
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodTrackRewind(DBusMessage * message)
{
	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_TrackRewindProcess();
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodTrackFFREWind(DBusMessage * message)
{
	if (message != NULL)
	{
		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_TrackFFREWEndProcess();
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodUpdateDisplayInfo(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t x = 0, y = 0, width = 0, height =0;
		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_INT32, &x,
										DBUS_TYPE_INT32, &y,
										DBUS_TYPE_INT32, &width,
										DBUS_TYPE_INT32, &height,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayerControl_SetDisplay(x, y, width, height);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodUpdateExtDisplayInfo(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t x = 0, y = 0, width = 0, height =0;
		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_INT32, &x,
										DBUS_TYPE_INT32, &y,
										DBUS_TYPE_INT32, &width,
										DBUS_TYPE_INT32, &height,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayerControl_SetDualDisplay(x, y, width, height);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodRequestFileList(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t device = 0, count =0;
		int32_t number = 0;
		uint8_t content = 0;

		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BYTE, &device,
										DBUS_TYPE_BYTE, &content,
										DBUS_TYPE_INT32, &number,
										DBUS_TYPE_UINT32, &count,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_FileListRequestProcess(device, content, number, count);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}	
}

static void DBusMethodMetaBrowsingStart(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t device = 0, count =0;
		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BYTE, &device,
										DBUS_TYPE_UINT32, &count,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_MetaBrowsingStartBrowsing(device, count);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}	
}

static void DBusMethodMetaBrowsingRequest(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t device = 0;
		int32_t idx =0;
		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BYTE, &device,
										DBUS_TYPE_INT32, &idx,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_MetaBrowsingRequestList(device, idx);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}	
}

static void DBusMethodMetaBrowsingSelect(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t device = 0;
		int32_t idx =0;
		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BYTE, &device,
										DBUS_TYPE_INT32, &idx,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_MetaBrowsingSelectItem(device, idx);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}	
}

static void DBusMethodMetaBrowsingHome(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t device = 0, count =0;
		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BYTE, &device,
										DBUS_TYPE_UINT32, &count,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_MetaBrowsingMoveToHome(device, count);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodMetaBrowsingUndo(DBusMessage * message)
{
	if (message != NULL)
	{
		uint32_t device = 0;
		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BYTE, &device,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_MetaBrowsingMoveToPreviousMenu(device);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}	
}

static void DBusMethodMetaBrowsingEnd(DBusMessage * message)
{
if (message != NULL)
	{
		uint32_t device = 0;
		int32_t idx =0;
		if (GetArgumentFromDBusMessage(message, 
										DBUS_TYPE_BYTE, &device,
										DBUS_TYPE_INT32, &idx,
										DBUS_TYPE_INVALID) != 0)
		{
			TMPlayer_MetaBrowsingEndBrowsing(device, idx);
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}

static void DBusMethodRequestDisplay(DBusMessage * message)
{
	(void)message;
	// User pressed key to show AV Player
	TMPlayer_RequestDisplay();
}

static void DBusMethodGoLauncherHome(DBusMessage * message)
{
	(void)message;
	// User pressed Home Key
	TMPlayer_GoLauncherHome();
}

static void DBusMethodGoLauncherBack(DBusMessage * message)
{
	(void)message;
	// User pressed Back Key
	TMPlayer_GoLauncherBack();
}

static void DBusMethodGoAVOnOff(DBusMessage * message)
{
	(void)message;
	// User pressed AV On/Off Key
	TMPlayer_GoAVOnOff();
}

static void DBusMethodNotifyDisplayDone(DBusMessage * message)
{
	if (message != NULL)
	{
		dbus_bool_t on = FALSE;
		if (GetArgumentFromDBusMessage(message,
										DBUS_TYPE_BOOLEAN, &on,
										DBUS_TYPE_INVALID) != 0)
		{
			if (on == FALSE)
			{
				// Hide GUI is completed
				SendDBusModeManagerRscReleased(RSC_DISPLAY);
			}
		}
		else
		{
			(void)fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
		}
	}
	else
	{
		(void)fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
	}
}
