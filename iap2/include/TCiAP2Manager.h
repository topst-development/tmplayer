/****************************************************************************************
 *	 FileName	 : TCiAP2Manager.h
 *	 Description : TCiAP2Manager.h
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

#ifndef TC_IPOD_PLAYER_IAP2_MANAGER_H
#define TC_IPOD_PLAYER_IAP2_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#define iAP2_NCM_NUM    	0
#define CARPLAY_NCM_NUM 	1
#define ALBUMART_MAXSIZE    1024*1024

#ifndef DISABLE
#define DISABLE	0
#endif
#ifndef ENABLE
#define ENABLE	1
#endif
#ifdef NONE
#define NONE	0
#endif

#define MSEC_PER_HOUR   3600000
#define MSEC_PER_MIN    60000
#define MSEC_PER_SEC    1000

#define iAP2_STRING_MAX_LENGTH  1024

typedef struct iAP2String_st {
    uint32_t length;
    char string[iAP2_STRING_MAX_LENGTH];
} iAP2String_t;

typedef struct
{
    int     authDone;       // received IAP2_NOTI_IDEN_ACCEPTED message
    int     idenDone;       // received IAP2_NOTI_AUTH_SUCCESS message
    int     autoPlayEnable; // auto play status (1: enable, 0: disable)
    int     iap2Connected;  // iap2 connect status (1: connected, 0: disconnected)      
} tIAP2_BasicInfo;

typedef struct
{
    unsigned int currTrackNum;
    unsigned int totalTrackCnt;
    unsigned int albumArtSize;
} tIAP2_PlayInfo;

typedef struct
{
	unsigned short  sessionID;
	unsigned char   protocolID;
} tEA_Protocol;

//===========================
// Callback Functions
//===========================
typedef void (*TcciAP2OnEventConnected)(void);
typedef void (*TcciAP2OnEventDisconnected)(void);
typedef void (*TcciAP2OnEventMounted)(void);
typedef void (*TcciAP2OnEventPlay)(void);
typedef void (*TcciAP2OnEventPlayTimeChange)(uint32_t time);
typedef void (*TcciAP2OnEventTotalTimeChange)(uint32_t totaltime);
typedef void (*TcciAP2OnEventTitleInfo)(const char *title, uint32_t length);
typedef void (*TcciAP2OnEventArtistInfo)(const char *artist, uint32_t length);
typedef void (*TcciAP2OnEventAlbumInfo)(const char *album, uint32_t length);
typedef void (*TcciAP2OnEventAlbumArt)(const char *data, uint32_t length);
typedef void (*TcciAP2OnEventPlayStatusChange)(uint32_t playstatus);
typedef void (*TcciAP2OnEventTrackChange)(uint32_t tracknum);
typedef void (*TcciAP2OnEventTotalTrackChange)(uint32_t totalnum);
typedef void (*TcciAP2OnEventRepeatChange)(uint32_t repeat);
typedef void (*TcciAP2OnEventShuffleChange)(uint32_t shuffle);
typedef void (*TcciAP2OnEventCategoryIndexChange)(uint32_t currentnum, uint32_t totalnum);
typedef void (*TcciAP2OnEventCategoryMenuChange)(uint32_t mode, const char *menuname, uint32_t length);
typedef void (*TcciAP2OnEventCategoryNameChange)(int32_t index, const char *name, uint32_t length);
typedef void (*TcciAP2OnEventCategoryNameComplete)(void);
typedef void (*TcciAP2OnEventAddNewLibrary)(int32_t type);
typedef void (*TcciAP2OnEventUpdateNewLibraryProgress)(int32_t type, int32_t progress);

extern void iAP2PlayInitialize(void);
extern void iAP2PlayDeinitialize(void);
extern void iAP2SetCallback
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
);
extern void iAP2EventConnected(void);
extern void iAP2EventDisconnected(void);
extern void iAP2EventCategoryIndexChange(unsigned int currentnum, unsigned int totalnum);
extern void iAP2EventCategoryMenuChange(unsigned int mode, const char *menuname, unsigned int length);
extern void iAP2EventCategoryNameChange(int32_t index, const char *name, unsigned int length);
extern void iAP2EventCategoryNameComplete(void);
extern void iAP2EventAddNewLibrary(int32_t type);
extern void iAP2EventUpdateNewLibraryProgress(int32_t type, int32_t progress);
extern void SendDBusiAP2MethodCall(const char *methodName, void *payload);
extern void SendDBusiAP2HidStart(void);
extern void SendDBusiAP2HidStop(void);
extern void SendDBusiAP2NowPlayingStart(void);
extern void SendDBusiAP2NowPlayingStop(void);
extern void SendDBusiAP2NowPlayingSetInformationElapsedTime(uint32_t elapsedTime);
extern void SendDBusiAP2NowPlayingSetInformationPlaybackQueueIndex(uint32_t index);
extern void SendDBusiAP2PlayMediaLibraryCurrentSelection(void *payload);
extern void SendDBusiAP2PlayMediaLibraryItems(void *payload);
extern void SendDBusiAP2PlayMediaLibraryCollection(void *payload);
extern void SendDBusiAP2PlayMediaLibrarySpecial(void *payload);
extern int GetDBusGetLibraryType(char *libUid);
extern int GetDBusDbInitHandler(void *handler, int libType) ;
extern int GetDBusDatabaseInit(void *handler, int category) ;
extern int GetDBusDatabaseInitWithLibType(void *handler, int category, int libType)  ;
extern int GetDBusDatabaseSelect(void *handler, int category, uint64_t persistentId) ;
extern int GetDBusDatabaseSelectAll(void *handler, int category) ;
extern int GetDBusDatabaseMoveUpper(void *handler) ;
extern int GetDBusPlayBackListCount() ;
extern uint8_t *GetDBusGetIdListPlayBack() ;
extern uint8_t *GetDBusDatabaseGetName(void *handler, int startIndex, int count) ;
extern uint8_t *GetDBusPlayBackListGetName(void *handler, uint64_t *idList,
                                     unsigned int idCount, int startIndex, int count) ;
extern uint8_t *GetDBusPlayBackListGetNameByIds(void *handler, uint64_t *idList, unsigned int count) ;
extern uint8_t *GetDBusPlayBackListGetProp(void *handler, int colMask, uint64_t persistentId) ;
extern uint8_t *GetDBusPlayBackListGetPropByIds(void *handler, int colMask, uint64_t *persistentIds, int count) ;

extern void iAP2CommandPlayStart(void);
extern void iAP2CommandPlayResume(void);
extern void iAP2CommandPlayStop(void);
extern void iAP2CommandPlayPause(void);
extern void iAP2CommandTrackUp(void);
extern void iAP2CommandTrackDown(void);
extern void iAP2CommandTrackSeek(unsigned char hour, unsigned char min, unsigned char sec);
extern void iAP2CommandTrackForward(void);
extern void iAP2CommandTrackRewind(void);
extern void iAP2CommandTrackFFREWEnd(void);
extern void iAP2CommandChangeRepeat(void);
extern void iAP2CommandChangeShuffle(void);
extern void iAP2CommandMetaBrowsingStart(unsigned int listcount);
extern void iAP2CommandMetaBrowsingUp(int index);
extern void iAP2CommandMetaBrowsingDown(int index);
extern void iAP2CommandMetaBrowsingSelect(int index);
extern void iAP2CommandMetaBrowsingHome(unsigned int listcount);
extern void iAP2CommandMetaBrowsingUndo(void);
extern void iAP2CommandMetaBrowsingEnd(int index);

extern void iAP2SignalDBusProcess(unsigned int id, DBusMessage *message);
extern void iAP2NotifyFromProcess(DBusMessage *message);
extern void iAP2LogEnable(unsigned int flag);
extern void iAP2SendDBusiAP2Intialize();

#ifdef __cplusplus
}
#endif

#endif

