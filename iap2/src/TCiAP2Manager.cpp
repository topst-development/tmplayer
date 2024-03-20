/****************************************************************************************
 *	 FileName	 : TCiAP2Manager.cpp
 *	 Description : TCiAP2Manager.cpp
 ****************************************************************************************
 *
 *	 TCC Version 1.0
 *	 Copyright (c) Telechips Inc.
 *	 All rights reserved 
 
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

#include <string>
#include <vector>

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dbus/dbus.h>
#include <inttypes.h>
#include <pthread.h>
#include "iAP2Const.h"
#include "DBusMsgDef.h"
#include "TCDBusRawAPI.h"
#include "iAP2NativeMsg.h"
#include "TCiAP2Manager.h"
#include "DBusMsgDef_iAP2.h"
#include "TMPlayerDBusiAP2Manager.h"
#include "TCiAP2FeatureHid.h"
#include "Iap2BrwsManager.h"

int g_debug = 0;
int g_brws_debug = 0;
extern int g_brws_library_debug;
#define IPOD_PLAYER_IAP2_PRINTF(format, arg...) \
	if (g_debug) \
	{ \
		fprintf(stderr, "[iPod PLAYER iAP2 MANAGER] %s: " format "", __FUNCTION__, ##arg); \
	}

static tIAP2_BasicInfo IAP2BasicInfo;
static tIAP2_PlayInfo IAP2PlayInfo;
static unsigned int iAP2TrackNumber = 0;
static unsigned int iAP2TotalCount = 0;

typedef struct __TCC_IAP2_CB__
{
    TcciAP2OnEventConnected          onEventConnected;
    TcciAP2OnEventDisconnected       onEventDisconnected;
    TcciAP2OnEventMounted            onEventMounted;
    TcciAP2OnEventPlay               onEventPlay;
    TcciAP2OnEventPlayTimeChange     onEventPlayTimeChange;
    TcciAP2OnEventTotalTimeChange    onEventTotalTimeChange;
    TcciAP2OnEventTitleInfo          onEventTitleInfo;
    TcciAP2OnEventArtistInfo         onEventArtistInfo;
    TcciAP2OnEventAlbumInfo          onEventAlbumInfo;
    TcciAP2OnEventAlbumArt           onEventAlbumArt;
    TcciAP2OnEventPlayStatusChange   onEventPlayStatusChange;
    TcciAP2OnEventTrackChange        onEventTrackChange;
    TcciAP2OnEventTotalTrackChange   onEventTotalTrackChange;
    TcciAP2OnEventRepeatChange       onEventRepeatChange;
	TcciAP2OnEventShuffleChange      onEventShuffleChange;
	TcciAP2OnEventCategoryIndexChange   onEventCategoryIndexChange;
	TcciAP2OnEventCategoryMenuChange    onEventCategoryMenuChange;
	TcciAP2OnEventCategoryNameChange    onEventCategoryNameChange;
	TcciAP2OnEventCategoryNameComplete  onEventCategoryNameComplete;
	TcciAP2OnEventAddNewLibrary         onEventAddNewLibrary;
	TcciAP2OnEventUpdateNewLibraryProgress onEventUpdateNewLibraryProgress;
    void*                               userData;
}TcciAP2CB;

typedef enum {
	iAP2CategoryAll, /*Top-level*/
	iAP2CategoryPlaylist,
	iAP2CategoryArtist,
	iAP2CategoryAlbum,
	iAP2CategoryGenre,
	iAP2CategoryTrack,
	iAP2CategoryComposer,
	iAP2CategoryAudiobook,
	iAP2CategoryPodcast,
	iAP2CategoryItunesRadio,
	TotaliAP2CategoryTypes
} iPodCategoryType;

static TcciAP2CB iAP2CB;

static tIAP2_BasicInfo *pGetIAP2BasicInfo(void);
static void iAP2EventMounted(void);
static void iAP2EventPlay(void);
static void iAP2EventPlayTimeChange(uint32_t time);
static void iAP2EventTotalTimeChange(uint32_t totaltime);
static void iAP2EventTitleInfo(const char *title, uint32_t length);
static void iAP2EventArtistInfo(const char *artist, uint32_t length);
static void iAP2EventAlbumInfo(const char *album, uint32_t length);
static void iAP2EventAlbumArt(const char *data, uint32_t length);
static void iAP2EventPlayStatusChange(unsigned long playstatus);
static void iAP2EventTrackChange(uint32_t tracknum);
static void iAP2EventTotalTrackCount(uint32_t totalnum);
static void iAP2EventRepeatChange(unsigned long repeat);
static void iAP2EventShuffleChange(unsigned long shuffle);
#if 0
static void iAP2EventNoSong(void);
static void iAP2EventError(miAP2s32 error);
#endif

static tIAP2_BasicInfo *pGetIAP2BasicInfo(void)
{
    return (&IAP2BasicInfo);
}

static tIAP2_PlayInfo *pGetIAP2PlayInfo(void)
{
    return (&IAP2PlayInfo);
}

void iAP2PlayInitialize(void)
{
	iAP2SendDBusiAP2Intialize();
}

void iAP2PlayDeinitialize(void)
{

}

void iAP2SetCallback
(
	TcciAP2OnEventConnected OnEventConnected,
	TcciAP2OnEventDisconnected OnEventDisconnected,
	TcciAP2OnEventMounted OnEventMounted,
	TcciAP2OnEventPlay OnEventPlay,
	TcciAP2OnEventPlayTimeChange OnEventPlayTimeChange,
	TcciAP2OnEventTotalTimeChange OnEventTotalTimeChange,
	TcciAP2OnEventTitleInfo OnEventTitleInfo,
	TcciAP2OnEventArtistInfo OnEventArtistInfo,
	TcciAP2OnEventAlbumInfo OnEventAlbumInfo,
	TcciAP2OnEventAlbumArt OnEventAlbumArt,
	TcciAP2OnEventPlayStatusChange OnEventPlayStatusChange,
	TcciAP2OnEventTrackChange OnEventTrackChange,
	TcciAP2OnEventTotalTrackChange OnEventTotalTrackChange,
	TcciAP2OnEventRepeatChange OnEventRepeatChange,
	TcciAP2OnEventShuffleChange OnEventShuffleChange,
	TcciAP2OnEventCategoryIndexChange OnEventCategoryIndexChange,
	TcciAP2OnEventCategoryMenuChange OnEventCategoryMenuChange,
	TcciAP2OnEventCategoryNameChange OnEventCategoryNameChange,
	TcciAP2OnEventCategoryNameComplete OnEventCategoryNameComplete,
	TcciAP2OnEventAddNewLibrary OnEventAddNewLibrary,
	TcciAP2OnEventUpdateNewLibraryProgress OnEventUpdateNewLibraryProgress
)
{
	iAP2CB.onEventConnected = OnEventConnected;
	iAP2CB.onEventDisconnected = OnEventDisconnected;
	iAP2CB.onEventMounted = OnEventMounted;
	iAP2CB.onEventPlay = OnEventPlay;
	iAP2CB.onEventPlayTimeChange = OnEventPlayTimeChange;
	iAP2CB.onEventTotalTimeChange = OnEventTotalTimeChange;
	iAP2CB.onEventTitleInfo = OnEventTitleInfo;
	iAP2CB.onEventArtistInfo = OnEventArtistInfo;
	iAP2CB.onEventAlbumInfo = OnEventAlbumInfo;
	iAP2CB.onEventAlbumArt = OnEventAlbumArt;
	iAP2CB.onEventPlayStatusChange = OnEventPlayStatusChange;
	iAP2CB.onEventTrackChange = OnEventTrackChange;
	iAP2CB.onEventTotalTrackChange = OnEventTotalTrackChange;
	iAP2CB.onEventRepeatChange = OnEventRepeatChange;
	iAP2CB.onEventShuffleChange = OnEventShuffleChange;
	iAP2CB.onEventCategoryIndexChange = OnEventCategoryIndexChange;
	iAP2CB.onEventCategoryMenuChange = OnEventCategoryMenuChange;
	iAP2CB.onEventCategoryNameChange = OnEventCategoryNameChange;
	iAP2CB.onEventCategoryNameComplete = OnEventCategoryNameComplete;
	iAP2CB.onEventAddNewLibrary = OnEventAddNewLibrary;
	iAP2CB.onEventUpdateNewLibraryProgress = OnEventUpdateNewLibraryProgress;	
}

void iAP2EventConnected(void)
{
    tIAP2_BasicInfo *pIAP2BasicInfo;
    Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
    
    IPOD_PLAYER_IAP2_PRINTF(" brwsManager[%p]", brwsManager);
    pIAP2BasicInfo = pGetIAP2BasicInfo();
    pIAP2BasicInfo->iap2Connected = ENABLE;

	if(iAP2CB.onEventConnected)
	{
		iAP2CB.onEventConnected();
	}
}

void iAP2EventDisconnected(void)
{
    tIAP2_BasicInfo *pIAP2BasicInfo;
    
    IPOD_PLAYER_IAP2_PRINTF("\n");
    pIAP2BasicInfo = pGetIAP2BasicInfo();
    pIAP2BasicInfo->iap2Connected = DISABLE;

	if(iAP2CB.onEventDisconnected)
	{
		iAP2CB.onEventDisconnected();
	}
}

static void iAP2EventMounted(void)
{
    tIAP2_BasicInfo *pIAP2BasicInfo;
    
    IPOD_PLAYER_IAP2_PRINTF("\n");
    pIAP2BasicInfo = pGetIAP2BasicInfo();
    if (ENABLE == pIAP2BasicInfo->iap2Connected)
    {
		if(iAP2CB.onEventMounted)
		{
			iAP2CB.onEventMounted();
		}
    }
}

static void iAP2EventPlay(void)
{
    tIAP2_BasicInfo *pIAP2BasicInfo;
    
    IPOD_PLAYER_IAP2_PRINTF("\n");
    pIAP2BasicInfo = pGetIAP2BasicInfo();
    if (ENABLE == pIAP2BasicInfo->iap2Connected)
    {
        if(iAP2CB.onEventPlay)
		{
			iAP2CB.onEventPlay();
		}
    }
}

static void iAP2EventPlayTimeChange(unsigned int time)
{
	//IPOD_PLAYER_IAP2_PRINTF("time:%u\n", time);

	if(iAP2CB.onEventPlayTimeChange)
	{
		iAP2CB.onEventPlayTimeChange(time);
	}
}

static void iAP2EventTotalTimeChange(unsigned int totaltime)
{
	IPOD_PLAYER_IAP2_PRINTF("totaltime:%u\n", totaltime);

	if(iAP2CB.onEventTotalTimeChange)
	{
		iAP2CB.onEventTotalTimeChange(totaltime);
	}
}
static void iAP2EventTitleInfo(const char *title, uint32_t length)
{
    IPOD_PLAYER_IAP2_PRINTF("title:%s\n", title);

	if(iAP2CB.onEventTitleInfo)
	{
		iAP2CB.onEventTitleInfo(title, length);
	}
}

static void iAP2EventArtistInfo(const char *artist, uint32_t length)
{
    IPOD_PLAYER_IAP2_PRINTF("artist:%s\n", artist);
	if(iAP2CB.onEventArtistInfo)
	{
		iAP2CB.onEventArtistInfo(artist, length);
	}
}

static void iAP2EventAlbumInfo(const char *album, uint32_t length)
{
	IPOD_PLAYER_IAP2_PRINTF("album:%s\n", album);
	if(iAP2CB.onEventAlbumInfo)
	{
		iAP2CB.onEventAlbumInfo(album, length);
	}
}

static void iAP2EventAlbumArt(const char *uri, uint32_t length)
{
    IPOD_PLAYER_IAP2_PRINTF("length %d\n", length);
	if(iAP2CB.onEventAlbumArt)
	{
		iAP2CB.onEventAlbumArt(uri, length);
	}
}

static void iAP2EventPlayStatusChange(unsigned long playstatus)
{
    IPOD_PLAYER_IAP2_PRINTF("play status(%lu)\n", playstatus);
	if(iAP2CB.onEventPlayStatusChange)
	{
		iAP2CB.onEventPlayStatusChange(playstatus);
	}
}

static void iAP2EventTrackChange(uint32_t tracknum)
{
    IPOD_PLAYER_IAP2_PRINTF("tracknum:%u\n", tracknum);
	if(iAP2CB.onEventTrackChange)
	{
		iAP2CB.onEventTrackChange(tracknum);
	}
}

static void iAP2EventTotalTrackCount(uint32_t totalnum)
{
    IPOD_PLAYER_IAP2_PRINTF("totalnum:%u\n", totalnum);
	if(iAP2CB.onEventTotalTrackChange)
	{
		iAP2CB.onEventTotalTrackChange(totalnum);
	}
}

static void iAP2EventRepeatChange(unsigned long repeat)
{
    IPOD_PLAYER_IAP2_PRINTF("Repeat(%lu)\n", repeat);
	if(iAP2CB.onEventRepeatChange)
	{
		iAP2CB.onEventRepeatChange(repeat);
	}
}

static void iAP2EventShuffleChange(unsigned long shuffle)
{
    IPOD_PLAYER_IAP2_PRINTF("Shuffle(%lu)\n", shuffle);
	if(iAP2CB.onEventShuffleChange)
	{
		iAP2CB.onEventShuffleChange(shuffle);
	}
}

void iAP2EventCategoryIndexChange(unsigned int currentnum, unsigned int totalnum)
{
    IPOD_PLAYER_IAP2_PRINTF(" currentnum:%u, totalnum:%u\n",  currentnum, totalnum);
	if(iAP2CB.onEventCategoryIndexChange)
	{
		iAP2CB.onEventCategoryIndexChange(currentnum, totalnum);
	}
}

void iAP2EventCategoryMenuChange(unsigned int mode, const char *menuname, unsigned int length)
{
    IPOD_PLAYER_IAP2_PRINTF(" mode:%u, menu:%s\n",  mode, menuname);
	if(iAP2CB.onEventCategoryMenuChange)
	{
		iAP2CB.onEventCategoryMenuChange(mode, menuname, length);
	}
}

void iAP2EventCategoryNameChange(int32_t index, const char *name, unsigned int length)
{
    IPOD_PLAYER_IAP2_PRINTF(" index:%d, name:%s, length:%u\n", index, name, length);
	if(iAP2CB.onEventCategoryNameChange)
	{
		iAP2CB.onEventCategoryNameChange(index, name, length);
	}
}

void iAP2EventCategoryNameComplete(void)
{
    IPOD_PLAYER_IAP2_PRINTF(" \n");
	if(iAP2CB.onEventCategoryNameComplete)
	{
		iAP2CB.onEventCategoryNameComplete();
	}
}

void iAP2EventAddNewLibrary(int32_t type)
{
	IPOD_PLAYER_IAP2_PRINTF(" \n");
	if(iAP2CB.onEventAddNewLibrary)
	{
		iAP2CB.onEventAddNewLibrary(type);
	}
}

void iAP2EventUpdateNewLibraryProgress(int32_t type, int32_t progress)
{
	IPOD_PLAYER_IAP2_PRINTF(" \n");
	if(iAP2CB.onEventUpdateNewLibraryProgress)
	{
		iAP2CB.onEventUpdateNewLibraryProgress(type, progress);
	}
}

///////////////////////////////////////////////// DBUS METHOD /////////////////////////////////////////////////

void SendDBusiAP2MethodCall(const char *methodName, void *payload)
{
    int length = 0;
    DBusMessage *message;
    IPOD_PLAYER_IAP2_PRINTF(" methodName %s \n",methodName);
    if(methodName != NULL)
    {
        if (payload != NULL)
        {
            memcpy(&length,payload,4);
            length += 4;
            message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
    									  IAP2_PROCESS_INTERFACE,
    									  methodName,
    									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
    									  &payload,length,
    									  DBUS_TYPE_INVALID);
            IPOD_PLAYER_IAP2_PRINTF(" create msg with payload %s \n",methodName);
        }
        else if (payload == NULL)
        {
    	    message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
    									  IAP2_PROCESS_INTERFACE,
    									  methodName,
    									  DBUS_TYPE_INVALID);
            IPOD_PLAYER_IAP2_PRINTF(" create msg %s \n",methodName);
        }

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

        IPOD_PLAYER_IAP2_PRINTF(" send msg %s \n",methodName);
    }
}

void SendDBusiAP2NowPlayingStart(void)
{
    SendDBusiAP2MethodCall(METHOD_iAP2_NOW_PLAYING_START, NULL);
}

void SendDBusiAP2NowPlayingStop(void)
{
    SendDBusiAP2MethodCall(METHOD_iAP2_NOW_PLAYING_STOP, NULL);
}

void SendDBusiAP2NowPlayingSetInformation(void *payload)
{
    SendDBusiAP2MethodCall(METHOD_iAP2_NOW_PLAYING_SET_INFO,payload);
}

void SendDBusiAP2NowPlayingSetInformationElapsedTime(uint32_t elapsedTime)
{
    iAP2NativeMsg *msg = new iAP2NativeMsg();
    uint8_t *payload = NULL;
    msg->addIntItem(set_nowplaying_ElapsedTime, IAP2_PARAM_NUMU32,elapsedTime);
    payload = msg->getRawNativeMsg();
    SendDBusiAP2NowPlayingSetInformation((void *)payload);
    if(payload != NULL)
        free(payload);
    delete msg;
}

void SendDBusiAP2NowPlayingSetInformationPlaybackQueueIndex(uint32_t index)
{
    iAP2NativeMsg *msg = new iAP2NativeMsg();
    uint8_t *payload = NULL;
    msg->addIntItem(set_nowplaying_PlaybackQueueIndex, IAP2_PARAM_NUMU32,index);
    payload = msg->getRawNativeMsg();
    SendDBusiAP2NowPlayingSetInformation((void *)payload);
    if(payload != NULL)
        free(payload);
    delete msg;
}

void SendDBusiAP2PlayMediaLibraryCurrentSelection(void *payload)
{
    SendDBusiAP2MethodCall(METHOD_iAP2_PLAYMEDIALIBRARY_CURRENT_SELECTION,payload);
}

void SendDBusiAP2PlayMediaLibraryItems(void *payload)
{
    SendDBusiAP2MethodCall(METHOD_iAP2_PLAYMEDIALIBRARY_ITEMS,payload);
}

void SendDBusiAP2PlayMediaLibraryCollection(void *payload)
{
    SendDBusiAP2MethodCall(METHOD_iAP2_PLAYMEDIALIBRARY_COLLECTION,payload);
}

void SendDBusiAP2PlayMediaLibrarySpecial(void *payload)
{
    SendDBusiAP2MethodCall(METHOD_iAP2_PLAYMEDIALIBRARY_SPECIAL,payload);
}

int GetDBusGetLibraryType(char *libUid) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | libUid[%s]\n", __FUNCTION__, libUid);

    char *argLibUid = libUid;
    if (argLibUid == NULL) 
    {
        fprintf(stderr, "%s | invaild library Uid\n", __FUNCTION__);
        return -1;
    }

    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_GETLIBTYPE,
									  DBUS_TYPE_BYTE, argLibUid,
									  DBUS_TYPE_INVALID);
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return -2;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return -3;
    }

    int replyType = -1;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_INT32, &replyType, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -4;
    }

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return replyType;
}

int GetDBusDbInitHandler(void *handler, int libType) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] library type[%d]\n",
                                       __FUNCTION__, handler, libType);

    int argLibType = libType;
    if (argLibType < 0) 
    {
        fprintf(stderr, "%s | invaild library type[%d]\n",
                __FUNCTION__, argLibType);
        return -2;
    }

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return -1;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return -1;
    }

    unsigned int sizeOfHandler = -1;
    memcpy(&sizeOfHandler, argHandler, sizeof(unsigned int));
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_INITHANDLER,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INT32, &argLibType,
									  DBUS_TYPE_INVALID);
    if(argHandler != NULL)
        free(argHandler);
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return -3;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return -4;
    }

    uint8_t *replyHandler = NULL;
    unsigned int sizeOfReplyHandler = 0;
    int replyCount = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &replyHandler, &sizeOfReplyHandler,
                                        DBUS_TYPE_INT32, &replyCount, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -5;
    }

    if (replyHandler == NULL) 
    {
        fprintf(stderr, "%s | fail to get Handler\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -6;
    }

    uint8_t *byteStream = (uint8_t *) malloc(sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memset(byteStream, 0x00, sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memcpy(byteStream, replyHandler, sizeof(uint8_t) * sizeOfReplyHandler);
    pHandler->initVariable();
    pHandler->parcelToDbHandler((void *)byteStream,true);

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return replyCount;
}

int GetDBusDatabaseInit(void *handler, int category) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] category[%d]\n",
                                       __FUNCTION__, handler, category);

    int argCategory = category;
    if (argCategory < 0) 
    {
        fprintf(stderr, "%s | invaild category[%d]\n",
                __FUNCTION__, argCategory);
        return -2;
    }

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return -1;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return -1;
    }

    unsigned int sizeOfHandler = -1;
    memcpy(&sizeOfHandler, argHandler, sizeof(unsigned int));
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_INIT,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INT32, &argCategory,
									  DBUS_TYPE_INVALID);
    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return -3;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -4;
    }

    uint8_t *replyHandler = NULL;
    unsigned int sizeOfReplyHandler = 0;
    int replyCount = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &replyHandler, &sizeOfReplyHandler,
                                        DBUS_TYPE_INT32, &replyCount, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -5;
    }

    if (replyHandler == NULL) 
    {
        fprintf(stderr, "%s | fail to get Handler\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -6;
    }

    uint8_t *byteStream = (uint8_t *) malloc(sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memset(byteStream, 0x00, sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memcpy(byteStream, replyHandler, sizeof(uint8_t) * sizeOfReplyHandler);
    pHandler->initVariable();
    pHandler->parcelToDbHandler((void *)byteStream,true);

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return replyCount;
}

int GetDBusDatabaseInitWithLibType(void *handler, int category, int libType) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] category[%d] libType[%d]\n",
                                       __FUNCTION__, handler, category, libType);

    int argCategory = category;
    if (argCategory < 0) 
    {
        fprintf(stderr, "%s | invaild category[%d]\n",
                __FUNCTION__, argCategory);
        return -2;
    }

    int argLibType = libType;
    if (argLibType < 0) 
    {
        fprintf(stderr, "%s | invaild libType[%d]\n",
                __FUNCTION__, argLibType);
        return -3;
    }
    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return -1;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return -1;
    }    

    unsigned int sizeOfHandler = -1;
    memcpy(&sizeOfHandler, argHandler, sizeof(unsigned int));
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_INITWITHLIBTYPE,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INT32, &argCategory,
									  DBUS_TYPE_INT32, &argLibType,
									  DBUS_TYPE_INVALID);
    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return -4;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return -5;
    }

    uint8_t *replyHandler = NULL;
    unsigned int sizeOfReplyHandler = 0;
    int replyCount = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &replyHandler, &sizeOfReplyHandler,
                                        DBUS_TYPE_INT32, &replyCount, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -6;
    }

    if (replyHandler == NULL) 
    {
        fprintf(stderr, "%s | fail to get Handler\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -7;
    }

    uint8_t *byteStream = (uint8_t *) malloc(sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memset(byteStream, 0x00, sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memcpy(byteStream, replyHandler, sizeof(uint8_t) * sizeOfReplyHandler);
    pHandler->initVariable();
    pHandler->parcelToDbHandler((void *)byteStream,true);

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return replyCount;
}

int GetDBusDatabaseSelect(void *handler, int category, uint64_t persistentId) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] category[%d] persistentId[%llu][%lld]\n",
                                       __FUNCTION__, handler, category, (unsigned long long) persistentId, (long long) persistentId);

    int argCategory = category;
    if (argCategory < 0) 
    {
        fprintf(stderr, "%s | invaild category[%d]\n",
                __FUNCTION__, argCategory);
        return -2;
    }

    uint64_t argPersistentId = persistentId;
    if (argPersistentId == 0) 
    {
        fprintf(stderr, "%s | invaild persistentId[%llu][%lld]\n",
                __FUNCTION__, (unsigned long long) argPersistentId, (long long) argPersistentId);
        return -3;
    }

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return -1;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return -1;
    }

    unsigned int sizeOfHandler = -1;
    memcpy(&sizeOfHandler, argHandler, sizeof(unsigned int));
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_SELECT,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INT32, &argCategory,
									  DBUS_TYPE_UINT64, &argPersistentId,
									  DBUS_TYPE_INVALID);
    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return -4;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return -5;
    }

    uint8_t *replyHandler = NULL;
    unsigned int sizeOfReplyHandler = 0;
    int replyCount = 0;
    if (!GetArgumentFromDBusPendingCall(pending, 
                                        DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &replyHandler, &sizeOfReplyHandler,
                                        DBUS_TYPE_INT32, &replyCount, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -6;
    }

    if (replyHandler == NULL) 
    {
        fprintf(stderr, "%s | fail to get Handler\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -7;
    }

    uint8_t *byteStream = (uint8_t *) malloc(sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memset(byteStream, 0x00, sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memcpy(byteStream, replyHandler, sizeof(uint8_t) * sizeOfReplyHandler);
    pHandler->initVariable();
    pHandler->parcelToDbHandler((void *)byteStream,true);

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return replyCount;
}

int GetDBusDatabaseSelectAll(void *handler, int category) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] category[%d]\n",
                                       __FUNCTION__, handler, category);

    int argCategory = category;
    if (argCategory < 0) 
    {
        fprintf(stderr, "%s | invaild category[%d]\n",
                __FUNCTION__, argCategory);
        return -2;
    }

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return -1;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return -1;
    }

    unsigned int sizeOfHandler = -1;
    memcpy(&sizeOfHandler, argHandler, sizeof(unsigned int));
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_SELECTALL,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INT32, &argCategory,
									  DBUS_TYPE_INVALID);

    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return -3;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return -4;
    }

    uint8_t *replyHandler = NULL;
    unsigned int sizeOfReplyHandler = 0;
    int replyCount = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &replyHandler, &sizeOfReplyHandler,
                                        DBUS_TYPE_INT32, &replyCount, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -5;
    }

    if (replyHandler == NULL) 
    {
        fprintf(stderr, "%s | fail to get Handler\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -6;
    }

    uint8_t *byteStream = (uint8_t *) malloc(sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memset(byteStream, 0x00, sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memcpy(byteStream, replyHandler, sizeof(uint8_t) * sizeOfReplyHandler);
    pHandler->initVariable();
    pHandler->parcelToDbHandler((void *)byteStream,true);

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return replyCount;
}

int GetDBusDatabaseMoveUpper(void *handler) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p]\n", __FUNCTION__, handler);

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return -1;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return -1;
    }

    unsigned int sizeOfHandler = -1;
    memcpy(&sizeOfHandler, argHandler, sizeof(unsigned int));
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_MOVEUPPER,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INVALID);

    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return -2;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return -3;
    }

    uint8_t *replyHandler = NULL;
    unsigned int sizeOfReplyHandler = 0;
    int replyCount = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &replyHandler, &sizeOfReplyHandler,
                                        DBUS_TYPE_INT32, &replyCount, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -4;
    }

    if (replyHandler == NULL) 
    {
        fprintf(stderr, "%s | fail to get Handler\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -5;
    }

    uint8_t *byteStream = (uint8_t *) malloc(sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memset(byteStream, 0x00, sizeof(uint8_t) * sizeOfReplyHandler + 1);
    memcpy(byteStream, replyHandler, sizeof(uint8_t) * sizeOfReplyHandler);
    pHandler->initVariable();
    pHandler->parcelToDbHandler((void *)byteStream,true);

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return replyCount;
}

int GetDBusPlayBackListCount() 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | playback count called\n", __FUNCTION__);
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_PLAYBACKLISTCOUNT,
									  DBUS_TYPE_INVALID);
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return -1;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return -2;
    }

    int replyCount = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_INT32, &replyCount, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return -3;
    }

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return replyCount;
}

uint8_t *GetDBusGetIdListPlayBack() 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | playback count called\n", __FUNCTION__);
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_GETIDLISTPLAYBACK,
									  DBUS_TYPE_INVALID);
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return NULL;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return NULL;
    }

    unsigned int length = 0;
    uint8_t *reply = NULL;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &reply, &length, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return NULL;
    }

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);
    return reply;
}

uint8_t *GetDBusDatabaseGetName(void *handler, int startIndex, int count) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] startIndex[%d] count[%d]\n",
                                       __FUNCTION__, handler, startIndex, count);

    int argStartIndex = startIndex;
    if (argStartIndex < 0) 
    {
        fprintf(stderr, "%s | invaild start index[%d]: must be greater or equal than zero\n",
                __FUNCTION__, argStartIndex);
        return NULL;
    }

    int argCount = count;
    if (argCount <= 0) 
    {
        fprintf(stderr, "%s | invaild count[%d]: must be greater than zero\n",
                __FUNCTION__, argCount);
        return NULL;
    }

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return NULL;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return NULL;
    }    

    unsigned int sizeOfHandler = 0;
    memcpy(&sizeOfHandler, argHandler, 4);
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_GETNAME,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INT32, &argStartIndex,
									  DBUS_TYPE_INT32, &argCount,
									  DBUS_TYPE_INVALID);

    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return NULL;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        //dbus_message_unref(message);
        return NULL;
    }

    uint8_t *reply = NULL;
    int lengthOfReply = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
                                        &reply, &lengthOfReply, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        //dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return NULL;
    }

    //dbus_message_unref(message);
    dbus_pending_call_unref(pending);

    uint8_t *result = (uint8_t *) malloc(sizeof(uint8_t) * lengthOfReply + 1);
    memset(result, 0x00, sizeof(uint8_t) * lengthOfReply + 1);
    memcpy(result, reply, sizeof(uint8_t) * lengthOfReply);
    return result;
}

uint8_t *GetDBusPlayBackListGetName(void *handler, uint64_t *idList,
                                     unsigned int idCount, int startIndex, int count) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] idList[%p] count[%u]\n",
                                       __FUNCTION__, handler, idList, count);

    uint64_t *argIdList = idList;
    if (argIdList == NULL) 
    {
        fprintf(stderr, "%s | invaild id List is NULL\n", __FUNCTION__);
        return NULL;
    }

    int argIdCount = idCount;
    if (argIdCount == 0) 
    {
        fprintf(stderr, "%s | invaild id list count[%d]: must be greater than zero\n",
                __FUNCTION__, argIdCount);
        return NULL;
    }

    int argStartIndex = startIndex;
    if (argStartIndex < 0) 
    {
        fprintf(stderr, "%s | invaild start index[%d]: must be greater than zero\n",
                __FUNCTION__, argStartIndex);
        return NULL;
    }

    int argCount = count;
    if (argCount <= 0) 
    {
        fprintf(stderr, "%s | invaild count[%d]: must be greater than zero\n",
                __FUNCTION__, argCount);
        return NULL;
    }

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return NULL;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return NULL;
    }

    unsigned int sizeOfHandler = 0;
    memcpy(&sizeOfHandler, argHandler, 4);
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_PLAYBACKLISTGETNAME,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_UINT64, &argIdList, argIdCount,
									  DBUS_TYPE_INT32, &argStartIndex,
									  DBUS_TYPE_INT32, &argCount,
									  DBUS_TYPE_INVALID);

    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return NULL;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return NULL;
    }

    uint8_t *reply = NULL;
    int lengthOfReply = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
                                        &reply, &lengthOfReply, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return NULL;
    }

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);

    uint8_t *result = (uint8_t *) malloc(sizeof(uint8_t) * lengthOfReply + 1);
    memset(result, 0x00, sizeof(uint8_t) * lengthOfReply + 1);
    memcpy(result, reply, sizeof(uint8_t) * lengthOfReply);
    return result;
}

uint8_t *GetDBusPlayBackListGetNameByIds(void *handler, uint64_t *idList, unsigned int count) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] idList[%p] count[%u]\n",
                                       __FUNCTION__, handler, idList, count);

    uint64_t *argIdList = idList;
    if (argIdList == NULL) 
    {
        fprintf(stderr, "%s | invaild id List is NULL\n", __FUNCTION__);
        return NULL;
    }

    int argCount = count;
    if (argCount == 0) 
    {
        fprintf(stderr, "%s | invaild count[%d]: must be greater than zero\n",
                __FUNCTION__, argCount);
        return NULL;
    }
    
    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return NULL;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return NULL;
    }

    unsigned int sizeOfHandler = 0;
    memcpy(&sizeOfHandler, argHandler, 4);
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_PLAYBACKLISTGETNAMEBYID,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_UINT64, &argIdList, argCount,
									  DBUS_TYPE_INVALID);

    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return NULL;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return NULL;
    }

    uint8_t *reply = NULL;
    int lengthOfReply = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
                                        &reply, &lengthOfReply, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return NULL;
    }

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);

    uint8_t *result = (uint8_t *) malloc(sizeof(uint8_t) * lengthOfReply + 1);
    memset(result, 0x00, sizeof(uint8_t) * lengthOfReply + 1);
    memcpy(result, reply, sizeof(uint8_t) * lengthOfReply);
    return result;
}

uint8_t *GetDBusPlayBackListGetProp(void *handler, int colMask, uint64_t persistentId) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] colMask[%d] persistentId[%llu][%lld]\n",
                                       __FUNCTION__, handler, colMask, (unsigned long long) persistentId, (long long) persistentId);

    int argColMask = colMask;
    if (argColMask == 0) 
    {
        fprintf(stderr, "%s | invaild column mask\n", __FUNCTION__);
        return NULL;
    }

    uint64_t argPersistentId = persistentId;
    if (argPersistentId == 0) 
    {
        fprintf(stderr, "%s | invaild persistentId[%llu]: must not be zero\n",
                __FUNCTION__, (unsigned long long) argPersistentId);
        return NULL;
    }

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return NULL;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return NULL;
    }

    unsigned int sizeOfHandler = 0;
    memcpy(&sizeOfHandler, argHandler, 4);
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_PLAYBACKLISTGETPROP,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INT32, &argColMask,
									  DBUS_TYPE_UINT64, &argPersistentId,
									  DBUS_TYPE_INVALID);

    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return NULL;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return NULL;
    }

    uint8_t *reply = NULL;
    int lengthOfReply = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
                                        &reply, &lengthOfReply, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return NULL;
    }

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);

    uint8_t *result = (uint8_t *) malloc(sizeof(uint8_t) * lengthOfReply + 1);
    memset(result, 0x00, sizeof(uint8_t) * lengthOfReply + 1);
    memcpy(result, reply, sizeof(uint8_t) * lengthOfReply);
    return result;
}

uint8_t *GetDBusPlayBackListGetPropByIds(void *handler, int colMask, uint64_t *persistentIds, int count) 
{
    IPOD_PLAYER_IAP2_PRINTF("%s | handler[%p] colMask[%d] persistentIds[%p] count[%d]\n",
                                       __FUNCTION__, handler, colMask, persistentIds, count);

    int argColMask = colMask;
    if (argColMask == 0) 
    {
        fprintf(stderr, "%s | invaild column mask\n", __FUNCTION__);
        return NULL;
    }

    int argCount = count;
    if (argCount == 0) 
    {
        fprintf(stderr, "%s | invaild count[%d]: must be greater than zero\n",
                __FUNCTION__, argCount);
        return NULL;
    }

    uint64_t *argIdList = persistentIds;
    if (argIdList == 0) 
    {
        fprintf(stderr, "%s | invaild persistentIds[%p]: must not be zero\n",
                __FUNCTION__, argIdList);
        return NULL;
    }

    if(handler == NULL)
    {
        fprintf(stderr, "%s |  handler is NULL \n", __FUNCTION__);
        return NULL;
    }

    Iap2DbHandler *pHandler = (Iap2DbHandler *)handler;
    uint8_t *argHandler = pHandler->getRawNativeMsg();
    if (argHandler == NULL) 
    {
        fprintf(stderr, "%s | database handler conversion fail\n", __FUNCTION__);
        return NULL;
    }

    unsigned int sizeOfHandler = 0;
    memcpy(&sizeOfHandler, argHandler, 4);
    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, IAP2_PROCESS_OBJECT_PATH,
									  IAP2_PROCESS_INTERFACE, METHOD_iAP2_DB_PLAYBACKLISTGETPROPBYID,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &argHandler, sizeOfHandler + 4,
									  DBUS_TYPE_INT32, &argColMask,
									  DBUS_TYPE_ARRAY, DBUS_TYPE_UINT64, &argIdList, argCount,
									  DBUS_TYPE_INVALID);

    if(argHandler != NULL)
        free(argHandler);
    
    if (message == NULL) 
    {
        fprintf(stderr, "%s | dbus message creation fail\n", __FUNCTION__);
        return NULL;
    }

    DBusPendingCall *pending = NULL;
    SendDBusMessage(message, &pending);
    if (pending == NULL) 
    {
        fprintf(stderr, "%s | dbus pending call fail\n", __FUNCTION__);
        dbus_message_unref(message);
        return NULL;
    }

    uint8_t *reply = NULL;
    int lengthOfReply = 0;
    if (!GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
                                        &reply, &lengthOfReply, DBUS_TYPE_INVALID)) 
    {
        fprintf(stderr, "%s | fail to get reply\n", __FUNCTION__);
        dbus_message_unref(message);
        dbus_pending_call_unref(pending);
        return NULL;
    }

    dbus_message_unref(message);
    dbus_pending_call_unref(pending);

    uint8_t *result = (uint8_t *) malloc(sizeof(uint8_t) * lengthOfReply + 1);
    memset(result, 0x00, sizeof(uint8_t) * lengthOfReply + 1);
    memcpy(result, reply, sizeof(uint8_t) * lengthOfReply);
    return result;
}

void iAP2CommandPlayStart(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_Play);
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandPlayResume(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_Play);
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandPlayStop(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_Pause);
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandPlayPause(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_Pause);
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandTrackUp(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_TransportRight);
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandTrackDown(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_TransportLeft);
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandTrackSeek(unsigned char hour, unsigned char min, unsigned char sec)
{
    unsigned int msecTime;
    msecTime = (unsigned int)((hour*MSEC_PER_HOUR)+(min*MSEC_PER_MIN)+(sec*MSEC_PER_SEC));
    SendDBusiAP2NowPlayingSetInformationElapsedTime(msecTime);
}

void iAP2CommandTrackForward(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_TransportRight);
}

void iAP2CommandTrackRewind(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_TransportLeft);
}

void iAP2CommandTrackFFREWEnd(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandChangeRepeat(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_Repeat);
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandChangeShuffle(void)
{
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    hidManager->sendHidReport(hid_key_Shuffle);
    hidManager->sendHidReport(hid_key_Release);
}

void iAP2CommandMetaBrowsingStart(unsigned int listcount)
{
    Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
    brwsManager->homeBrws(listcount);
}

void iAP2CommandMetaBrowsingUp(int index)
{
    Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
    brwsManager->move(index);
}

void iAP2CommandMetaBrowsingDown(int index)
{
    Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
    brwsManager->move(index);
}

void iAP2CommandMetaBrowsingSelect(int index)
{
    Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
    brwsManager->selectItem(index);
}

void iAP2CommandMetaBrowsingHome(unsigned int listcount)
{
    Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
    brwsManager->homeBrws(listcount);
}

void iAP2CommandMetaBrowsingUndo(void)
{
    Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
    brwsManager->undoitem();
}

void iAP2CommandMetaBrowsingEnd(int index)
{
    Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
    brwsManager->exitBrws(index);
}

void *TestPendingMsg(void *payload)
{
    void *array = NULL;
    DBusMessage *message;
    IPOD_PLAYER_IAP2_PRINTF("1  \n");
    int length = 0;
    if(payload != NULL)
    {
        memcpy(&length,payload,4);
        length += 4;
    }
        
    message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, 
                                                            IAP2_PROCESS_OBJECT_PATH,
                                                            IAP2_PROCESS_INTERFACE,
                                                            METHOD_iAP2_TEST_PENDING_MSG,
								                            DBUS_TYPE_ARRAY, DBUS_TYPE_INT32,
								                            &payload,length,
                                                            DBUS_TYPE_INVALID);
    if (message != NULL)
    {
        DBusPendingCall *pending = NULL;
        IPOD_PLAYER_IAP2_PRINTF("2  \n");
        if (!SendDBusMessage(message, &pending))
        {
            fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
        }
        else
        {
	    int rspLength = 0;
            IPOD_PLAYER_IAP2_PRINTF("3  \n");
            if(GetArgumentFromDBusPendingCall(pending,
                                                                    DBUS_TYPE_ARRAY, DBUS_TYPE_INT32,
                                                                    &array, &rspLength,
                                                                    DBUS_TYPE_INVALID))
            {
                IPOD_PLAYER_IAP2_PRINTF("4  \n");
                fprintf(stderr, "%s data length :%d ", __FUNCTION__, rspLength);
            }
            else
            {
                IPOD_PLAYER_IAP2_PRINTF("5  \n");
            }
        }
        if(pending != NULL)
            dbus_pending_call_unref(pending);

        dbus_message_unref(message);
    }
    else
    {
        fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
    }
    return array;
}

///////////////////////////////////////////////// DBUS SIGNAL /////////////////////////////////////////////////

void iAP2ParseMediaItem(void *obj,int length)
{
    iAP2NativeMsg *msg = NULL;
    Iap2ObjGroup *rootGroup = NULL;
    Iap2ObjItem *item = NULL;
    int cnt;

    IPOD_PLAYER_IAP2_PRINTF("%s length %d \n",__func__,length);

    if(obj == NULL)
    {
        IPOD_PLAYER_IAP2_PRINTF("%s obj == NULL \n",__FUNCTION__);
        return;
    }

    msg = new iAP2NativeMsg();//(uint8_t *)obj,length,false);
    rootGroup = msg->getRootGroup();
    rootGroup->setPayloadData((uint8_t *)obj, length);
    IPOD_PLAYER_IAP2_PRINTF("%s itemCount %d \n",__FUNCTION__,rootGroup->getItemCount());
    for(cnt=0; cnt < rootGroup->getItemCount(); cnt++)
    {
        item = rootGroup->getObjItem(cnt);
        switch(item->mParamType)
        {
            case IAP2_PARAM_STRING:
                {
                    unsigned char *pStr;
                    int strLen = 0;
                    pStr = item->getItemString(&strLen);
                    
                    switch(item->mParamId)
                    {
                        case nowplaying_media_Title:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, Title [%s]\n",item->mParamId,pStr);
                            iAP2EventTitleInfo((const char *)pStr,strLen);
                            break;
                        case nowplaying_media_AlbumTitle:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, Album [%s]\n",item->mParamId,pStr);
                            iAP2EventAlbumInfo((const char *)pStr, strLen);
                            break;
                        case nowplaying_media_Artist:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, Artist [%s]\n",item->mParamId,pStr);
                            iAP2EventArtistInfo((const char *)pStr, strLen);
                            break;
                        case nowplaying_media_Albumartist:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, Albumartist [%s]\n",item->mParamId,pStr);
                            iAP2EventArtistInfo((const char *)pStr, strLen);
                            break;
                        case nowplaying_media_Genre:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, Genre [%s]\n",item->mParamId,pStr);
                            break;
                        case nowplaying_media_Composer:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, Composer [%s]\n",item->mParamId,pStr);
                            break;
                        default:
                            break;    
                    }
                }
                break;
            case IAP2_PARAM_NUMU64:
                {
                    unsigned long long data64 = 0;
                    data64 = item->getItemLong();
                    switch(item->mParamId)
                    {
                        case nowplaying_media_PersistIdentifier:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, PersistId [%llu]\n",item->mParamId,data64);
                            break;
                        case nowplaying_media_AlbumPersistIdentifer:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, AlbumPersistId [%llu]\n",item->mParamId,data64);
                            break;
                        case nowplaying_media_ArtistPersistIdentifier:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, ArtistPersistId [%llu]\n",item->mParamId,data64);
                            break;
                        case nowplaying_media_AlbumartistPersistIdentifier:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, AlbumartistPersistId [%llu]\n",item->mParamId,data64);
                            break;
                        case nowplaying_media_GenrePersistIdentifier:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, GenrePersistId [%llu]\n",item->mParamId,data64);
                            break;
                        case nowplaying_media_ComposePersistIdentifier:                        
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, ComposerPersistId [%llu]\n",item->mParamId,data64);
                            break;
                        default:
                            break;    
                    }
                }
                break;
            case IAP2_PARAM_ENUM:
            case IAP2_PARAM_BOOLEAN:
            case IAP2_PARAM_NUM8:
            case IAP2_PARAM_NUM16:
            case IAP2_PARAM_NUM32:
            case IAP2_PARAM_NUMU8:
            case IAP2_PARAM_NUMU16:
            case IAP2_PARAM_NUMU32:
                {
                    unsigned int data32 = item->getItemInt();
                    switch(item->mParamId)
                    {
                        case nowplaying_media_MediaType:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, meidaType [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_Rating:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, Rating [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_PlaybackDurationMilliseconds:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, PlaybackDurationMilliseconds [%d]\n",item->mParamId,data32);
                            iAP2EventTotalTimeChange(data32);
                            break;
                        case nowplaying_media_AlbumTrackNumber:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, AlbumTrackNumber [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_AlbumTrackCount:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, AlbumTrackCount [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_AlbumDiscNumber:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, AlbumDiscNumber [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_AlbumDiscCount:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, AlbumDiscCount [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_IsPartofCompilation:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, IsPartofCompilation [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_IsLikeSupported:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, IsLikeSupported [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_IsBanSupported:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, IsBanSupported [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_IsLiked:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, IsLiked [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_IsBanned:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, IsBanned [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_ResidentOnDevice:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, ResidentOnDevice [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_ArtworkFileTransIdentifier:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, ArtworkFileTransIdentifier [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_media_ChapterCount:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, ChapterCount [%d]\n",item->mParamId,data32);
                            break;
                        default:
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }
}

void iAP2ParsePlayback(void *obj,int length)
{
    iAP2NativeMsg *msg = NULL;
    Iap2ObjGroup *rootGroup = NULL;
    Iap2ObjItem *item = NULL;

    if(obj == NULL)
        return;

    msg = new iAP2NativeMsg();//(uint8_t *)obj,length,false);
    rootGroup = msg->getRootGroup();
    rootGroup->setPayloadData((uint8_t *)obj, length);
    for(int i = 0; i < rootGroup->getItemCount();i++)
    {
        item = rootGroup->getObjItem(i);
        switch(item->mParamType)
        {
            case IAP2_PARAM_STRING:
                {
                    unsigned char *pStr;
                    int strLen = 0;
                    pStr = item->getItemString(&strLen);
                    
                    switch(item->mParamId)
                    {
                        case nowplaying_playback_AppName:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, ApplicationName [%s]\n",item->mParamId,pStr);
                            break;
                        case nowplaying_playback_PBMediaLibraryUniqueIdentifer:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, PBMediaLibraryUniqueIdentifer [%s]\n",item->mParamId,pStr);
                            break;
                        case nowplaying_playback_PBiTunesRadioStationName:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, PBiTunesRadioStationName [%s]\n",item->mParamId,pStr);
                            break;
                        case nowplaying_playback_PlaybackAppBundleID:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, PlaybackAppBundleID [%s]\n",item->mParamId,pStr);
                            break;
                        default:
                            break;    
                    }
                }
                break;
            case IAP2_PARAM_NUMU64:
                {
                    unsigned long long data64 = 0;
                    data64 = item->getItemLong();
                    switch(item->mParamId)
                    {
                        case nowplaying_playback_PBiTunesRadioStationMediaPlaylistPersistID:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, PBiTunesRadioStationMediaPlaylistPersistID [%llu]\n",item->mParamId,data64);
                            break;
                        default:
                            break;    
                    }
                }
                break;
            case IAP2_PARAM_ENUM:
            case IAP2_PARAM_BOOLEAN:
            case IAP2_PARAM_NUM8:
            case IAP2_PARAM_NUM16:
            case IAP2_PARAM_NUM32:
            case IAP2_PARAM_NUMU8:
            case IAP2_PARAM_NUMU16:
            case IAP2_PARAM_NUMU32:
                {
                    unsigned int data32 = item->getItemInt();
                    switch(item->mParamId)
                    {
                        case nowplaying_playback_Status: // enum #0/1   => PlaybackStatus
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            iAP2EventPlayStatusChange(data32);
                            break;
                        case nowplaying_playback_ElapsedTimeInMilliseconds: // uint32 #0/1
                            //IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            iAP2EventPlayTimeChange(data32);
                            break;
                        case nowplaying_playback_QueueIndex: // uint32 #0/1 // Current Track Number
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            iAP2EventTrackChange(data32);
                            break;
                        case nowplaying_playback_QueueCount: // uint32 #0/1 // Total Track Count
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            iAP2EventTotalTrackCount(data32);
                            break;
                        case nowplaying_playback_QueueChapterIndex: // uint32 #0/1
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_playback_ShuffleMode: // enum #0/1=> PlaybackShuffle
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            iAP2EventShuffleChange(data32);
                            break;
                        case nowplaying_playback_RepeatMode: // enum #0/1  => PlaybackRepeat
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            iAP2EventRepeatChange(data32);
                            break;
                        case nowplaying_playback_PBiTunesRadioAd: // bool #0/1
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_playback_PlaybackSpeed: // uint16 0/1
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_playback_SetElapsedTimeAvailable: // bool 0/1
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_playback_PlaybackQueueListAvail: // bool 0/1
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            break;
                        case nowplaying_playback_PlaybackQueueListTransferID: // uint8 #0/1
                            IPOD_PLAYER_IAP2_PRINTF("id = %d,  [%d]\n",item->mParamId,data32);
                            break;
                        default:
                            break;    
                    }
                }
                break;
            default:
                break;
        }
    }
}

void iAP2ParseArtwork(void *obj,int length)
{
    iAP2NativeMsg *msg = NULL;
    Iap2ObjGroup *rootGroup = NULL;
    Iap2ObjItem *item = NULL;
    char * AlbumArtBuf;
    tIAP2_PlayInfo *pPlayInfo;

    pPlayInfo = pGetIAP2PlayInfo();
    
    if(obj == NULL)
    {
        fprintf(stderr,"ERROR!! %s OBJ is NULL\n", __FUNCTION__);
        return;
    }

    msg = new iAP2NativeMsg();//(uint8_t *)obj,length,false);
    rootGroup = msg->getRootGroup();
    rootGroup->setPayloadData((uint8_t *)obj, length);
    for(int cnt = 0; cnt < rootGroup->getItemCount(); cnt++)
    {
        item = rootGroup->getObjItem(cnt);
        switch(item->mParamType)
        {
            case IAP2_PARAM_STRING:
                {
                    const char *pStr;
                    int strLen = 0;
                    pStr = (const char *)item->getItemString(&strLen);
                    switch(item->mParamId)
                    {
                        FILE *fp;
                        
                        case nowplaying_media_ArtworkDataUri:
#if 1
                            if (pPlayInfo->albumArtSize > 0)
                            {
                                pStr = "/dev/shm/albumart-mmap-data";
                                iAP2EventAlbumArt(pStr, pPlayInfo->albumArtSize);
                                pPlayInfo->albumArtSize = 0;
                            }
#else
                            pStr = "/dev/shm/albumart-mmap-data";
                            if((fp = fopen(pStr, "r")) == NULL)
                            {
                                fprintf(stderr, "%s: Error fopen AlbumArt file (%s)\n", __FUNCTION__, pStr);
                            }

                            if (pPlayInfo->albumArtSize > 0)
                            {
                                AlbumArtBuf = (char *)malloc(pPlayInfo->albumArtSize);
                                if(AlbumArtBuf == NULL)
                                {
                                    fprintf(stderr, "%s: Error malloc AlbumArt\n", __FUNCTION__);
                                }
                                else
                                {
                                    fread(AlbumArtBuf, pPlayInfo->albumArtSize, 1, fp);
                                    iAP2EventAlbumArt(AlbumArtBuf, pPlayInfo->albumArtSize);
                                    free(AlbumArtBuf);
                                    pPlayInfo->albumArtSize = 0;
                                }
                            }
                            else
                            {
                                fprintf(stderr, "%s: Error Album Art Image Size is 0\n", __FUNCTION__);
                            }
#endif
                            break;
                        default:
                            break;
                    }
                }
                break;
            case IAP2_PARAM_BLOB:
                {
                    uint8_t *data = 0;
                    int length = 0;

                    data = item->getItemBytes(&length);
                    switch(item->mParamId)
                    {
                        case nowplaying_media_ArtworkData:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d length %d \n",item->mParamId,length);
							if (length == 0)
							{
								iAP2EventAlbumArt((const char *)data,length);
							}
                            break;
                        default:
                            break;
                    }
                }
                break;
            case IAP2_PARAM_NUMU8:
                {
                    unsigned int data32 = item->getItemInt();
                    switch(item->mParamId)
                    {
                        case nowplaying_media_ArtworkFileTransIdentifier: // enum #0/1   => PlaybackStatus
                            IPOD_PLAYER_IAP2_PRINTF("fileId = %d,  [%d]\n",item->mParamId,data32);
                            break;
                        default:
                            break;
                    }
                }
                break;
            case IAP2_PARAM_NUMU32:
                {
                    unsigned int data32 = item->getItemInt();
                    switch(item->mParamId)
                    {
                        case nowplaying_media_ArtworkDataLength:
                            IPOD_PLAYER_IAP2_PRINTF("id = %d, data size [%d]\n",item->mParamId, data32);
                            pPlayInfo->albumArtSize = data32;
                            if (data32 > ALBUMART_MAXSIZE)
                            {
                                pPlayInfo->albumArtSize = ALBUMART_MAXSIZE;
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }
    delete msg;
}

void iAP2SendDBusiAP2Intialize(void)
{
    DBusMessage *message;
    const char *dest = TMPLAYER_PROCESS_DBUS_NAME;
    const char *objectPath = TMPLAYER_PROCESS_OBJECT_PATH;
    const char *interfaceName = TMPLAYER_IAP2_INTERFACE;
    const char *methodNotifyName = METHOD_PLAYER_IAP2_NOTIFY;
    const char *methodConnectedName = METHOD_PLAYER_IAP2_CONNECTED;
    const char *methodPlayerLogEnable = METHOD_PLAYER_LOG_ENABLE;
    IPOD_PLAYER_IAP2_PRINTF("%s \n",__FUNCTION__);

    message = CreateDBusMsgMethodCall(IAP2_PROCESS_DBUS_NAME, 
                                        IAP2_PROCESS_OBJECT_PATH,
                                        IAP2_PROCESS_INTERFACE,
                                        METHOD_iAP2_INIT_APP,
                                        DBUS_TYPE_STRING,&dest,
                                        DBUS_TYPE_STRING,&objectPath,
                                        DBUS_TYPE_STRING,&interfaceName,
                                        DBUS_TYPE_STRING,&methodNotifyName,
                                        DBUS_TYPE_STRING,&methodConnectedName,
                                        DBUS_TYPE_STRING,&methodPlayerLogEnable,
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

void iAP2SuccessConnection(void)
{
    tIAP2_BasicInfo *pIAP2BasicInfo;
    iAP2Hid *hidManager = iAP2Hid::getInstance();
    
    pIAP2BasicInfo = pGetIAP2BasicInfo();
    SendDBusiAP2NowPlayingStart();
    pIAP2BasicInfo->idenDone = 0;
    pIAP2BasicInfo->authDone = 0;
    pIAP2BasicInfo->autoPlayEnable = ENABLE;

    hidManager->sendHidStart();
    iAP2EventMounted();
}


void iAP2NotifyFromProcess(DBusMessage *message)
{
    if (message != NULL)
    {
        int what, arg1, arg2, length;
        void *array;

        if (GetArgumentFromDBusMessage(message, 
                                        DBUS_TYPE_INT32, &what, 		  
                                        DBUS_TYPE_INT32, &arg1,
                                        DBUS_TYPE_INT32, &arg2,
                                        DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE,
                                        &array, &length,
                                        DBUS_TYPE_INVALID))
        {
            //IPOD_PLAYER_IAP2_PRINTF("%s: what(%d), arg1(%d), arg2(%d), len(%d)\n", __FUNCTION__, what, arg1, arg2, length);

            switch (what)
            {
                tIAP2_BasicInfo *pIAP2BasicInfo;

                case IAP2_NOTI_IDEN_ACCEPTED:
                    pIAP2BasicInfo = pGetIAP2BasicInfo();
                    pIAP2BasicInfo->idenDone = 1;
                    if (pIAP2BasicInfo->authDone)
                    {
                        iAP2SuccessConnection();
                    }
                    break;
                case IAP2_NOTI_AUTH_SUCCESS:
                    pIAP2BasicInfo = pGetIAP2BasicInfo();
                    pIAP2BasicInfo->authDone = 1;
                    if (pIAP2BasicInfo->idenDone)
                    {
                        iAP2SuccessConnection();
                    }
                    break;
                case IAP2_NOTI_VEHICLE_STATUS_START:

                    break;
                case IAP2_NOTI_POWER:

                    break;
                case IAP2_NOTI_DEV_INFO_NOTI:

                    break;
                case IAP2_NOTI_DEV_LANGUAGE_NOTI:

                    break;
                case IAP2_NOTI_DEV_TIME_NOTI:

                    break;
                case IAP2_NOTI_DEV_UUID_NOTI:

                    break;
                case IAP2_NOTI_NOW_PLAYING_MEDIA_ITEM:
                    iAP2ParseMediaItem(array, length);
                    break;        
                case IAP2_NOTI_NOW_PLAYING_PLAYBACK:
                    iAP2ParsePlayback(array, length);
                    pIAP2BasicInfo = pGetIAP2BasicInfo();
                    if(pIAP2BasicInfo->autoPlayEnable == ENABLE)
                    {
                        iAP2EventPlay();
                        pIAP2BasicInfo->autoPlayEnable = DISABLE;
                    }
                    break;
                case IAP2_NOTI_NOW_PLAYING_ARTWORK:
                    iAP2ParseArtwork(array,length);
                    break;
                case IAP2_NOTI_MEDIA_LIB_PLAYBACK_LIST:

                    break;
                case IAP2_NOTI_MEDIA_LIB_NEW:
                case IAP2_NOTI_MEDIA_LIB_PROGRESS:
                case IAP2_NOTI_MEDIA_LIB_CHANGED:
                    {
                        Iap2BrwsManager *brwsManager = Iap2BrwsManager::getInstance();
                        //IPOD_PLAYER_IAP2_PRINTF(" before brwsManager\n");
                        if(brwsManager != NULL)
                        {
                            brwsManager->dispatchNotify(what, arg1,arg2,array,length);
                        }
                        else
                        {
                            IPOD_PLAYER_IAP2_PRINTF(" brwsManager is null\n");
                        }
                        //IPOD_PLAYER_IAP2_PRINTF(" after brwsManager\n");
                    }
                    break;
                case IAP2_NOTI_CERTIFICATE:

                    break;
                default:
                    fprintf(stderr, "%s: undefine signal, what(%d)\n", __FUNCTION__, what);
                    break;
            }
        }
        else
        {
            fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
        }

    }
}

void iAP2LogEnable(unsigned int flag)
{
    if (flag != 0)
    {
        g_debug = true;
        g_brws_debug = true;
        g_brws_library_debug = true;
    }
    else
    {
        g_debug = false;
        g_brws_debug = false;
        g_brws_library_debug = false;
    }
}

void iAP2SignalDBusProcess(unsigned int id, DBusMessage *message)
{
	if (id < TotalSignaliAP2Events)
	{
		iAP2NotifyFromProcess(message);
	}
	else
	{
		fprintf(stderr, "%s: WRONG RECEIVED SIGNAL[%d]\n", __FUNCTION__, id);
	}
}

