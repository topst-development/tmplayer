/****************************************************************************************
 *	 FileName	 : TMPlayeriAP2.c
 *	 Description : TMPlayeriAP2.c
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

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dbus/dbus.h>
#include <inttypes.h>

#include "TMPlayerDBusiAP2Manager.h"
#include "TCiAP2Manager.h"
#include "TMPlayerType.h"
#include "TMPlayerDBus.h"
#include "TMPlayer.h"
#include "TMPlayeriAP2.h"

extern int32_t g_tc_debug;
#define DEBUG_TMPLAYER_IAP2_PRINTF(format, arg...) \
	if (g_tc_debug != 0) \
	{ \
		(void)fprintf(stderr, "[TM PLAYER iAP2] %s: " format "", __FUNCTION__, ##arg); \
	}
#define IAP2_NAME_MAX	(512)
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef enum {
	iAP2RepeatOff,
	iAP2RepeatOneTrack,
	iAP2RepeatAllTracks,
	TotaliAP2RepeatModes
} iAP2RepeatMode;

typedef enum {
	iAP2ShuffleOff,
	iAP2ShuffleTracks,
	iAP2ShuffleAlbums,
	TotaliAP2ShuffleModes
} iAP2ShuffleMode;

typedef enum {
	CategoryTitle,
	CategoryArtist,
	CategoryAlbum,
	CategoryGenre,
	CategoryLibrary,
	TotalCategories
} iAP2Category;

typedef struct {
	uint32_t playStatus;
	uint32_t currentTime;
	uint32_t totalTime;
	uint32_t currentNum;
	uint32_t totalNum;
	uint32_t shuffle;
	uint32_t repeat;
	char title[IAP2_NAME_MAX];
	char artist[IAP2_NAME_MAX];
	char album[IAP2_NAME_MAX];
	const char *albumArt;
	uint32_t albumArtLength;
} iAP2Info;

static iAP2Info s_iAP2Info;

static int32_t TMPlayeriAP2_isiAP2Device(void);

static void TMPlayeriAP2_Connected(void);
static void TMPlayeriAP2_Disconnected(void);
static void TMPlayeriAP2_Mounted(void);
static void TMPlayeriAP2_Play(void);
static void TMPlayeriAP2_PlayTimeChange(uint32_t time);
static void TMPlayeriAP2_TotalTimeChange(uint32_t totaltime);
static void TMPlayeriAP2_TitleInfo(const char *title, uint32_t length);
static void TMPlayeriAP2_ArtistInfo(const char *artist, uint32_t length);
static void TMPlayeriAP2_AlbumInfo(const char *album, uint32_t length);
static void TMPlayeriAP2_AlbumArt(const char *data, uint32_t length);
static void TMPlayeriAP2_PlayStatusChange(uint32_t playstatus);
static void TMPlayeriAP2_TrackChange(uint32_t tracknum);
static void TMPlayeriAP2_TotalTrackChange(uint32_t totalnum);
static void TMPlayeriAP2_RepeatMode(uint32_t repeatMode);
static void TMPlayeriAP2_ShuffleMode(uint32_t shuffleMode);
static void TMPlayeriAP2_RepeatChange(uint32_t repeat);
static void TMPlayeriAP2_ShuffleChange(uint32_t shuffle);
static void TMPlayeriAP2_CategoryIndexChange(unsigned int currentnum, unsigned int totalnum);
static void TMPlayeriAP2_CategoryMenuChange(unsigned int mode, const char *menuname, unsigned int length);
static void TMPlayeriAP2_CategoryInfoChange(int32_t index, const char *name, unsigned int length);
static void TMPlayeriAP2_CategoryInfoComplete(void);
static void TMPlayeriAP2_AddNewLibrary(int32_t type);
static void TMPlayeriAP2_UpdateNewLibraryProgress(int32_t type, int32_t progress);

void TMPlayeriAP2_Initialize(void)
{
	iAP2LogEnable(1);
	iAP2PlayInitialize();
	SendDBusiAP1Initialize();
	SendDBusiAP2Initialize();

	iAP2SetCallback
	(
		TMPlayeriAP2_Connected,
		TMPlayeriAP2_Disconnected,
		TMPlayeriAP2_Mounted,
		TMPlayeriAP2_Play,
		TMPlayeriAP2_PlayTimeChange,
		TMPlayeriAP2_TotalTimeChange,
		TMPlayeriAP2_TitleInfo,
		TMPlayeriAP2_ArtistInfo,
		TMPlayeriAP2_AlbumInfo,
		TMPlayeriAP2_AlbumArt,
		TMPlayeriAP2_PlayStatusChange,
		TMPlayeriAP2_TrackChange,
		TMPlayeriAP2_TotalTrackChange,
		TMPlayeriAP2_RepeatChange,
		TMPlayeriAP2_ShuffleChange,
		TMPlayeriAP2_CategoryIndexChange,
		TMPlayeriAP2_CategoryMenuChange,
		TMPlayeriAP2_CategoryInfoChange,
		TMPlayeriAP2_CategoryInfoComplete,
		TMPlayeriAP2_AddNewLibrary,
		TMPlayeriAP2_UpdateNewLibraryProgress
	);
}

void TMPlayeriAP2_Release(void)
{
	SendDBusiAP1Release();
	SendDBusiAP2Release();
	iAP2PlayDeinitialize();
}

/****************************/
/*     Command Function     */
/****************************/
void TMPlayeriAP2_PlayStart(void)
{
	iAP2CommandPlayStart();
}

void TMPlayeriAP2_PlayResume(void)
{
	iAP2CommandPlayResume();
}

void TMPlayeriAP2_PlayRestart(void)
{
	iAP2CommandPlayStart();

	TMPlayeriAP2_RefreshInfomation();
}

void TMPlayeriAP2_PlayStop(void)
{
	s_iAP2Info.playStatus = PlayStatusStop;
	iAP2CommandPlayStop();
}

void TMPlayeriAP2_PlayPause(void)
{
	if (s_iAP2Info.playStatus == PlayStatusStop)
	{
		// The iPod does not use Stopped Mode but Paused Mode
		// After tmplayer stopped iPod and set pause, iPod does not send Paused signal
		// So Add this code
		TMPlayer_SetPlayStatus_iAP2((int32_t)PlayStatusPause);
		s_iAP2Info.playStatus == PlayStatusPause;
	}
	else
	{
		iAP2CommandPlayPause();
	}
}

void TMPlayeriAP2_TrackUp(void)
{
	iAP2CommandTrackUp();
}

void TMPlayeriAP2_TrackDown(void)
{
	iAP2CommandTrackDown();
}

void TMPlayeriAP2_TrackSeek(unsigned char hour, unsigned char min, unsigned char sec)
{
	iAP2CommandTrackSeek(hour, min, sec);
}

void TMPlayeriAP2_TrackForward(void)
{
	iAP2CommandTrackForward();
}

void TMPlayeriAP2_TrackRewind(void)
{
	iAP2CommandTrackRewind();
}

void TMPlayeriAP2_TrackFFREWEnd(void)
{
	iAP2CommandTrackFFREWEnd();
}

void TMPlayeriAP2_ChangeRepeat(void)
{
	iAP2CommandChangeRepeat();
}

void TMPlayeriAP2_ChangeShuffle(void)
{
	iAP2CommandChangeShuffle();
}

void TMPlayeriAP2_MetaBrowsingStart(unsigned int listcount)
{
	iAP2CommandMetaBrowsingStart(listcount);
}

void TMPlayeriAP2_MetaBrowsingUp(int index)
{
	iAP2CommandMetaBrowsingUp(index);
}

void TMPlayeriAP2_MetaBrowsingDown(int index)
{
	iAP2CommandMetaBrowsingDown(index);
}

void TMPlayeriAP2_MetaBrowsingSelect(int index)
{
	iAP2CommandMetaBrowsingSelect(index);
}

void TMPlayeriAP2_MetaBrowsingHome(unsigned int listcount)
{
	iAP2CommandMetaBrowsingHome(listcount);
}

void TMPlayeriAP2_MetaBrowsingUndo(void)
{
	iAP2CommandMetaBrowsingUndo();
}

void TMPlayeriAP2_MetaBrowsingEnd(int index)
{
	iAP2CommandMetaBrowsingEnd(index);
}

/***************************/
/*  iAP2 Utility Function  */
/***************************/
static int32_t TMPlayeriAP2_isiAP2Device(void)
{
	int32_t isiAP2 = false;

	if(TMPlayer_GetDevice() == DeviceSourceiAP2)
	{
		isiAP2 = true;
	}
	return isiAP2;
}

void TMPlayeriAP2_RefreshInfomation(void)
{
	TMPlayeriAP2_TitleInfo(s_iAP2Info.title, strnlen(s_iAP2Info.title, 512));
	TMPlayeriAP2_ArtistInfo(s_iAP2Info.artist, strnlen(s_iAP2Info.artist, 512));
	TMPlayeriAP2_AlbumInfo(s_iAP2Info.album, strnlen(s_iAP2Info.album, 512));
	TMPlayeriAP2_AlbumArt(s_iAP2Info.albumArt, s_iAP2Info.albumArtLength);
	TMPlayeriAP2_RepeatMode(s_iAP2Info.repeat);
	TMPlayeriAP2_ShuffleMode(s_iAP2Info.shuffle);
	TMPlayeriAP2_PlayTimeChange(s_iAP2Info.currentTime);
	TMPlayeriAP2_TotalTimeChange(s_iAP2Info.totalTime);
	TMPlayeriAP2_TotalTrackChange(s_iAP2Info.totalNum);
	TMPlayeriAP2_TrackChange(s_iAP2Info.currentNum - 1);
}

void TMPlayeriAP2_ClearInformation(void)
{
	(void)memset(s_iAP2Info.title, 0x00, 512);
	(void)memset(s_iAP2Info.artist, 0x00, 512);
	(void)memset(s_iAP2Info.album, 0x00, 512);

	s_iAP2Info.albumArtLength = 0;
	s_iAP2Info.repeat = RepeatModeOff;
	s_iAP2Info.shuffle = ShuffleModeOff;
	s_iAP2Info.currentTime = 0;
	s_iAP2Info.totalTime = 0;
	s_iAP2Info.totalNum = 0;
}

/***************************/
/* Event Callback Function */
/***************************/
static void TMPlayeriAP2_Connected(void)
{
	DEBUG_TMPLAYER_IAP2_PRINTF("\n");
	DeviceConnectProcess("iAP");
}

static void TMPlayeriAP2_Disconnected(void)
{
	DEBUG_TMPLAYER_IAP2_PRINTF("\n");
	DeviceDisconnectProcess("iAP");
	TMPlayeriAP2_ClearInformation();
}

static void TMPlayeriAP2_Mounted(void)
{
	DEBUG_TMPLAYER_IAP2_PRINTF("\n");
	DeviceMountProcess("iAP", NULL);
}

static void TMPlayeriAP2_Play(void)
{
	DEBUG_TMPLAYER_IAP2_PRINTF("\n");
	SendDBusPlayStateChanged(PlayStatusPlaying);
}

static void TMPlayeriAP2_PlayTimeChange(uint32_t time)
{
    //DEBUG_TMPLAYER_IAP2_PRINTF("time:%u\n", time);

    unsigned char hour, min, sec;
    uint32_t timesec = time/1000;

	s_iAP2Info.currentTime = time;
    hour = (unsigned char)(timesec/3600);
    min = (unsigned char)((timesec/60)%60);
    sec = (unsigned char) (timesec%60);

	if(TMPlayeriAP2_isiAP2Device())
	{
	    SendDBusPlayTimeChanged(hour, min, sec);
	}
}

static void TMPlayeriAP2_TotalTimeChange(uint32_t totaltime)
{
	DEBUG_TMPLAYER_IAP2_PRINTF("totaltime:%u\n", totaltime);

    unsigned char hour, min, sec;
    uint32_t timesec = totaltime/1000;

	s_iAP2Info.totalTime = totaltime;
    hour = (unsigned char)(timesec/3600);
    min = (unsigned char)((timesec/60)%60);
    sec = (unsigned char)(timesec%60);

	if(TMPlayeriAP2_isiAP2Device())
	{
	    SendDBusTotalTimeChanged(hour, min, sec);
	}
}

static void TMPlayeriAP2_TitleInfo(const char *title, uint32_t length)
{
	uint32_t len;

	DEBUG_TMPLAYER_IAP2_PRINTF("title:%s, len:%d\n", title, length);

	len = MIN(length, IAP2_NAME_MAX-1);

	if (s_iAP2Info.title != title)
	{
		if ((title != NULL) && (length != 0))
		{
			memcpy(s_iAP2Info.title, title, len);
			s_iAP2Info.title[len] = (char)0x00;
		}
		else
		{
			s_iAP2Info.title[0] = (char)0x00;
		}
	}

	if (TMPlayeriAP2_isiAP2Device())
	{
		SendDBusFileNameChanged(s_iAP2Info.title, len);
		SendDBusiD3Infomation(CategoryTitle, s_iAP2Info.title);
	}
}

static void TMPlayeriAP2_ArtistInfo(const char *artist, uint32_t length)
{
	uint32_t len;

	DEBUG_TMPLAYER_IAP2_PRINTF("artist:%s, len:%d\n", artist, length);

	len = MIN(length, IAP2_NAME_MAX-1);

	if (s_iAP2Info.artist != artist)
	{
		if ((artist != NULL) && (length != 0))
		{
			memcpy(s_iAP2Info.artist, artist, len);
			s_iAP2Info.artist[len] = (char)0x00;
		}
		else
		{
			s_iAP2Info.artist[0] = (char)0x00;
		}
	}

	if (TMPlayeriAP2_isiAP2Device())
	{
		SendDBusiD3Infomation(CategoryArtist, s_iAP2Info.artist);
	}
}

static void TMPlayeriAP2_AlbumInfo(const char *album, uint32_t length)
{
	uint32_t len;

	DEBUG_TMPLAYER_IAP2_PRINTF("album:%s, len:%d\n", album, length);

	len = MIN(length, IAP2_NAME_MAX-1);

	if (s_iAP2Info.album != album)
	{
		if ((album != NULL) && (length != 0))
		{
			memcpy(s_iAP2Info.album, album, len);
			s_iAP2Info.album[len] = (char)0x00;
		}
		else
		{
			s_iAP2Info.album[0] = (char)0x00;
		}
	}

	if (TMPlayeriAP2_isiAP2Device())
	{
		SendDBusiD3Infomation(CategoryAlbum, s_iAP2Info.album);
	}
}

static void TMPlayeriAP2_AlbumArt(const char *uri, uint32_t length)
{
    DEBUG_TMPLAYER_IAP2_PRINTF("uri : %s, length %d\n", uri, length);

	if (uri != NULL)
	{
		s_iAP2Info.albumArt = uri;
	}
	else
	{
		s_iAP2Info.albumArt = "\0";
	}
	s_iAP2Info.albumArtLength = length;

	if(TMPlayeriAP2_isiAP2Device())
	{
		SendDBusAlbumArtShmData(s_iAP2Info.albumArt, s_iAP2Info.albumArtLength);
	}
}

static void TMPlayeriAP2_PlayStatusChange(uint32_t playstatus)
{
    int ret;
    DEBUG_TMPLAYER_IAP2_PRINTF("play status(%lu)\n", playstatus);

	if ((s_iAP2Info.playStatus == PlayStatusStop) && (playstatus == PlayStatusPause))
	{
		playstatus = PlayStatusStop;
	}

	s_iAP2Info.playStatus = playstatus;
	if(s_iAP2Info.playStatus == PlayStatusPlaying)
	{
		ret = TMPlayer_RequestChangeDevice(DeviceSourceiAP2, true);
		if(ret)
		{
			TMPlayer_SetPlayStatus_iAP2((int32_t)playstatus);
		}
		else
		{
			TMPlayeriAP2_PlayStop();
		}
	}
	else
	{
		if(TMPlayeriAP2_isiAP2Device())
		{
			TMPlayer_SetPlayStatus_iAP2((int32_t)playstatus);
		}
	}
}

static void TMPlayeriAP2_TrackChange(uint32_t tracknum)
{
    DEBUG_TMPLAYER_IAP2_PRINTF("tracknum:%u\n", tracknum);
    tracknum++;
   	s_iAP2Info.currentNum = tracknum;
    if(TMPlayeriAP2_isiAP2Device() && (tracknum > 0))
    {
		SendDBusFileNumberUpdated(DeviceSourceiAP2, MultiMediaContentTypeAudio, s_iAP2Info.currentNum, s_iAP2Info.totalNum);
    }
}

static void TMPlayeriAP2_TotalTrackChange(uint32_t totalnum)
{
    DEBUG_TMPLAYER_IAP2_PRINTF("totalnum:%u\n", totalnum);
   	s_iAP2Info.totalNum = totalnum;
    if(TMPlayeriAP2_isiAP2Device() && (totalnum > 0))
    {
		SendDBusTotalNumberChanged(DeviceSourceiAP2, MultiMediaContentTypeAudio, s_iAP2Info.totalNum);
    }
}

static void TMPlayeriAP2_RepeatMode(uint32_t repeatMode)
{
	TMPlayer_SetRepeatMode(DeviceSourceiAP2, MultiMediaContentTypeAudio, repeatMode);

	if(TMPlayeriAP2_isiAP2Device())
	{
		SendDBusRepeatModeChanged(repeatMode);
	}
}

static void TMPlayeriAP2_ShuffleMode(uint32_t shuffleMode)
{
	TMPlayer_SetShuffleMode(DeviceSourceiAP2, MultiMediaContentTypeAudio, shuffleMode);

	if(TMPlayeriAP2_isiAP2Device())
	{
		SendDBusShuffleModeChanged(shuffleMode);
	}
}

static void TMPlayeriAP2_RepeatChange(uint32_t repeat)
{
    DEBUG_TMPLAYER_IAP2_PRINTF("Repeat(%lu)\n", repeat);

	s_iAP2Info.repeat = repeat;
	switch(repeat)
	{
		case iAP2RepeatOff:
			s_iAP2Info.repeat = RepeatModeOff;
			break;
		case iAP2RepeatOneTrack:
			s_iAP2Info.repeat = RepeatModeTrack;
			break;
		case iAP2RepeatAllTracks:
			s_iAP2Info.repeat = RepeatModeAll;
			break;
		default :
			break;
	}

	TMPlayer_SetRepeatMode(DeviceSourceiAP2, MultiMediaContentTypeAudio, s_iAP2Info.repeat);

	if(TMPlayeriAP2_isiAP2Device())
	{
		SendDBusRepeatModeChanged(s_iAP2Info.repeat);
	}
}

static void TMPlayeriAP2_ShuffleChange(uint32_t shuffle)
{
    DEBUG_TMPLAYER_IAP2_PRINTF("Shuffle(%lu)\n", shuffle);

	s_iAP2Info.shuffle = shuffle;
	switch(shuffle)
	{
		case iAP2ShuffleOff:
			s_iAP2Info.shuffle = ShuffleModeOff;
			break;
		case iAP2ShuffleTracks:
		case iAP2ShuffleAlbums:
			s_iAP2Info.shuffle = ShuffleModeOn;
			break;
		default :
			break;
	}

	TMPlayer_SetShuffleMode(DeviceSourceiAP2, MultiMediaContentTypeAudio, s_iAP2Info.shuffle);

	if(TMPlayeriAP2_isiAP2Device())
	{
		SendDBusShuffleModeChanged(s_iAP2Info.shuffle);
	}
}

void TMPlayeriAP2_CategoryIndexChange(unsigned int currentnum, unsigned int totalnum)
{
    DEBUG_TMPLAYER_IAP2_PRINTF(" currentnum:%u, totalnum:%u\n",  currentnum, totalnum);
    MetaCategoryIndexChange(DeviceSourceiAP2, totalnum, currentnum);
}

void TMPlayeriAP2_CategoryMenuChange(unsigned int mode, const char *menuname, unsigned int length)
{
    DEBUG_TMPLAYER_IAP2_PRINTF(" mode:%u, menu:%s\n",  mode, menuname);
    MetaCategoryMenuName(DeviceSourceiAP2, mode, menuname);
}

void TMPlayeriAP2_CategoryInfoChange(int32_t index, const char *name, unsigned int length)
{
    DEBUG_TMPLAYER_IAP2_PRINTF(" index:%d, name:%s, length:%u\n", index, name, length);
    MetaCategoryInfo(DeviceSourceiAP2, index, name, 0);
}

void TMPlayeriAP2_CategoryInfoComplete(void)
{
    DEBUG_TMPLAYER_IAP2_PRINTF(" \n");
}

void TMPlayeriAP2_AddNewLibrary(int32_t type)
{
	DEBUG_TMPLAYER_IAP2_PRINTF(" \n");
	// Add D-Bus (Start Progress)
}

void TMPlayeriAP2_UpdateNewLibraryProgress(int32_t type, int32_t progress)
{
	DEBUG_TMPLAYER_IAP2_PRINTF(" \n");
	// Add D-Bus (Update Progress)
	if(progress == 100)
	{
		SendDBusFileDBCompleted(DeviceSourceiAP2);
	}
}

