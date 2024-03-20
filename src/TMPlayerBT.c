/****************************************************************************************
 *	 FileName	 : TMPlayerBT.c
 *	 Description : TMPlayerBT.c
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
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <dbus/dbus.h>
#include <inttypes.h>

#include "TMPlayerDBusBTManager.h"
#include "TMPlayerType.h"
#include "TMPlayerDBus.h"
#include "TMPlayer.h"
#include "TMPlayerBT.h"

extern int32_t g_tc_debug;
#define DEBUG_TMPLAYER_BT_PRINTF(format, arg...) \
	if (g_tc_debug != 0) \
	{ \
		(void)fprintf(stderr, "[TM PLAYER BT] %s: " format "", __FUNCTION__, ##arg); \
	}

#define MAX_QUEUE_SIZE		16
#define MAX_STACK_DEPTH		64

#define FIRST_ITEM			1

#define BRWS_UP				0x00
#define BRWS_DOWN			0x01

typedef enum {
	BTRepeatNone,
	BTRepeatOff,
	BTRepeatSingle,
	BTRepeatAll,
	BTRepeatGroup,
	TotalBTRepeatModes
} BTRepeatMode;

typedef enum {
	BTShuffleNone,
	BTShuffleOff,
	BTShuffleAll,
	BTShuffleGroup,
	TotalBTShuffleModes
} BTShuffleMode;

typedef struct {
	uint32_t	number;
	uint32_t	count;
	uint32_t	browsingType;
} BTQueue;

typedef struct {
	uint32_t	uid1; //uint8_t uid[0], uid[1], uid[2], uid[3]
	uint32_t	uid2; //uint8_t uid[4], uid[5], uid[6], uid[7]
} BTUid;

typedef struct {
	uint32_t	playStatus;
	uint32_t	currentTime;
	uint32_t	totalTime;
	uint32_t	currentNum;
	uint32_t	totalNum;
	uint32_t	shuffle;
	uint32_t	repeat;
	char		title[512];
	char		artist[512];
	char		album[512];
	char		albumArt[512];
	uint32_t	albumArtLength;
	bool		albumArtSupport;
	bool		browsingSupport;
	uint32_t	browsingType;

	BTQueue		browsingQueue[MAX_QUEUE_SIZE];
	char		browsingFront;
	char		browsingRear;

	BTUid		*playListUid;
	BTUid		*folderListUid;
	uint32_t	*itemType;
	uint32_t	itemPerPage;
	uint32_t	itemIndex;
	uint32_t	itemCount;
	uint32_t	itemStack[MAX_STACK_DEPTH];
	uint32_t	itemStackIndex;
} BTInfo;

static int32_t TMPlayerBT_isBTDevice(void);

static void TMPlayerBTUid_Create(BTUid **uidTable, uint32_t size);
static void TMPlayerBTUid_Release(BTUid **uidTable);
static void TMPlayerBTUid_SetValue(BTUid **uidTable, uint32_t *uid1, uint32_t *uid2, uint32_t idx, uint32_t cnt);

static bool		TMPlayerBTQueue_isEmpty(void);
static bool		TMPlayerBTQueue_isFull(void);
static bool		TMPlayerBTQueue_push(BTQueue *q);
static bool		TMPlayerBTQueue_pop(BTQueue *q);

static BTInfo  s_BTInfo;

void TMPlayerBT_Release(void)
{
	TMPlayerBTUid_Release(&s_BTInfo.playListUid);
	TMPlayerBTUid_Release(&s_BTInfo.folderListUid);
	if (s_BTInfo.itemType != NULL)
	{
		free(s_BTInfo.itemType);
		s_BTInfo.itemType = NULL;
	}
}

/****************************/
/*     Command Function     */
/****************************/
void TMPlayerBT_PlayStart(void)
{
	SendDBusBluetoothTrackStart();
}

void TMPlayerBT_PlayResume(void)
{
	SendDBusBluetoothTrackResume();
}

void TMPlayerBT_PlayRestart(void)
{
	SendDBusBluetoothTrackStart();
	TMPlayerBT_RefreshInfomation();
}

void TMPlayerBT_PlayStop(void)
{
	s_BTInfo.playStatus = PlayStatusStop;
	SendDBusBluetoothTrackStop();
}

void TMPlayerBT_PlayPause(void)
{
	if (s_BTInfo.playStatus == PlayStatusStop)
	{
		s_BTInfo.playStatus = PlayStatusPause;
	}
	SendDBusBluetoothTrackPause();
}

void TMPlayerBT_TrackUp(void)
{
	SendDBusBluetoothTrackUp();
}

void TMPlayerBT_TrackDown(void)
{
	SendDBusBluetoothTrackDown();
}

void TMPlayerBT_TrackMove(int number)
{
	if ((s_BTInfo.browsingSupport) && (s_BTInfo.playListUid != NULL) && (number > 0))
	{
		number--;

		DEBUG_TMPLAYER_BT_PRINTF("UID for Track %d Move: %x %x\n", number + 1, s_BTInfo.playListUid[number].uid1, s_BTInfo.playListUid[number].uid2);
		if ((s_BTInfo.playListUid[number].uid1 != 0) || (s_BTInfo.playListUid[number].uid2 != 0))
		{
			SendDBusBluetoothBrowsingPlay(number + 1, s_BTInfo.playListUid[number].uid1, s_BTInfo.playListUid[number].uid2);
		}
	}
}

void TMPlayerBT_TrackSeek(unsigned char hour, unsigned char min, unsigned char sec)
{
//	BT can't support this function
}

void TMPlayerBT_TrackForward(void)
{
	SendDBusBluetoothForward();
}

void TMPlayerBT_TrackRewind(void)
{
	SendDBusBluetoothRewind();
}

void TMPlayerBT_TrackFFREWEnd(void)
{
	SendDBusBluetoothFFREWEnd();
}

void TMPlayerBT_ChangeRepeat(void)
{
	SendDBusBluetoothChangeRepeat();
}

void TMPlayerBT_ChangeShuffle(void)
{
	SendDBusBluetoothChangeShuffle();
}

void TMPlayerBT_RequestList(int number, unsigned int count)
{
	BTQueue q;

	if (s_BTInfo.browsingSupport && (s_BTInfo.browsingType == BRWS_TYPE_PLAYLIST))
	{
		if (s_BTInfo.totalNum >= number)
		{
			q.number = number;
			q.count = count;
			q.browsingType = s_BTInfo.browsingType;

			if(!SendDBusBluetoothBrowsingList((unsigned int)number, &count))
			{
				DEBUG_TMPLAYER_BT_PRINTF("Queue Push: %d %d\n", q.number, q.count);
				TMPlayerBTQueue_push(&q);
			}
		}
	}
}

void TMPlayerBT_MetaBrowsingStart(unsigned int itemPerPage)
{
	uint32_t count = itemPerPage;

	DEBUG_TMPLAYER_BT_PRINTF("Item per Page: %d\n", itemPerPage);

	if ((s_BTInfo.browsingSupport) && (s_BTInfo.browsingType == BRWS_TYPE_FOLDER))
	{
		if (itemPerPage > 0)
		{
			s_BTInfo.itemPerPage = itemPerPage;
			TMPlayerBTUid_Release(&s_BTInfo.folderListUid);
			TMPlayerBTUid_Create(&s_BTInfo.folderListUid, itemPerPage);
			if (s_BTInfo.itemType != NULL)
			{
				free(s_BTInfo.itemType);
			}
			s_BTInfo.itemType = (uint32_t *)malloc(sizeof(uint32_t) * itemPerPage);
			if (s_BTInfo.itemType != NULL)
			{
				memset(s_BTInfo.itemType, 0x00, sizeof(uint32_t) * itemPerPage);
				DEBUG_TMPLAYER_BT_PRINTF("Memory allocation Success: %d byte\n", sizeof(BTUid) * itemPerPage);
			}
			else
			{
				(void)fprintf(stderr, "Memory allocation failed\n");
			}

			if (SendDBusBluetoothBrowsingList(s_BTInfo.itemIndex, &count))
			{
				if (s_BTInfo.itemCount != count)
				{
					s_BTInfo.itemCount = count;
					MetaCategoryIndexChange(DeviceSourceBluetooth, count, 0);
				}
			}

			MetaCategoryMenuName(DeviceSourceBluetooth, META_FOLDER, "Folder Browsing");
		}
	}
}

void TMPlayerBT_MetaBrowsingMove(int index)
{
	unsigned int count = s_BTInfo.itemPerPage;

	DEBUG_TMPLAYER_BT_PRINTF("Index: %d\n", index);

	if ((s_BTInfo.browsingSupport) && (s_BTInfo.browsingType == BRWS_TYPE_FOLDER))
	{
		if (index > 0)
		{
			TMPlayerBTUid_Release(&s_BTInfo.folderListUid);
			TMPlayerBTUid_Create(&s_BTInfo.folderListUid, s_BTInfo.itemPerPage);

			if (SendDBusBluetoothBrowsingList(index, &count))
			{
				s_BTInfo.itemIndex = index;
			}
		}
	}
}

void TMPlayerBT_MetaBrowsingSelect(int index)
{
	unsigned int count = s_BTInfo.itemPerPage;

	DEBUG_TMPLAYER_BT_PRINTF("Index: %d\n", index);

	if ((s_BTInfo.browsingSupport) && (s_BTInfo.folderListUid != NULL) && (index > 0))
	{
		index--;

		DEBUG_TMPLAYER_BT_PRINTF("UID for Browsing %d Selected: %x %x\n", index + 1, s_BTInfo.folderListUid[index].uid1, s_BTInfo.folderListUid[index].uid2);
		if ((s_BTInfo.folderListUid[index].uid1 != 0) || (s_BTInfo.folderListUid[index].uid2 != 0))
		{
			if (s_BTInfo.itemType[index] == META_FOLDER)
			{
				SendDBusBluetoothBrowsingChange(BRWS_DOWN, index + 1, s_BTInfo.folderListUid[index].uid1, s_BTInfo.folderListUid[index].uid2);

				TMPlayerBTUid_Release(&s_BTInfo.folderListUid);
				TMPlayerBTUid_Create(&s_BTInfo.folderListUid, s_BTInfo.itemPerPage);
				if (SendDBusBluetoothBrowsingList(FIRST_ITEM, &count))
				{
					DEBUG_TMPLAYER_BT_PRINTF("Parent index: %d, Depth: %d\n", s_BTInfo.itemIndex, s_BTInfo.itemStackIndex + 1);

					s_BTInfo.itemStackIndex++;
					s_BTInfo.itemStack[s_BTInfo.itemStackIndex] = s_BTInfo.itemIndex;
					s_BTInfo.itemIndex = FIRST_ITEM;
					s_BTInfo.itemCount = count;
					MetaCategoryIndexChange(DeviceSourceBluetooth, count, 0);
				}
			}
			else if (s_BTInfo.itemType[index] == META_TRACK)
			{
				SendDBusBluetoothBrowsingPlay(index + 1, s_BTInfo.folderListUid[index].uid1, s_BTInfo.folderListUid[index].uid2);
			}

			MetaCategoryMenuName(DeviceSourceBluetooth, META_FOLDER, "Folder Browsing");
		}
	}
}

void TMPlayerBT_MetaBrowsingHome(unsigned int listcount)
{
	DEBUG_TMPLAYER_BT_PRINTF("List Count: %d\n", listcount);
	MetaCategoryMenuName(DeviceSourceBluetooth, META_FOLDER, "Folder Browsing");
	//TODO Change to root folder in A2DP borwsing
}

void TMPlayerBT_MetaBrowsingUndo(void)
{
	unsigned int count = s_BTInfo.itemPerPage;

	DEBUG_TMPLAYER_BT_PRINTF("\n");

	if (s_BTInfo.itemStackIndex > 0)
	{
		SendDBusBluetoothBrowsingChange(BRWS_UP, 0, 0, 0);

		TMPlayerBTUid_Release(&s_BTInfo.folderListUid);
		TMPlayerBTUid_Create(&s_BTInfo.folderListUid, s_BTInfo.itemPerPage);

		if (SendDBusBluetoothBrowsingList(s_BTInfo.itemStack[s_BTInfo.itemStackIndex], &count))
		{
			DEBUG_TMPLAYER_BT_PRINTF("Parent index: %d, Depth: %d\n", s_BTInfo.itemStack[s_BTInfo.itemStackIndex], s_BTInfo.itemStackIndex);

			s_BTInfo.itemIndex = s_BTInfo.itemStack[s_BTInfo.itemStackIndex];
			s_BTInfo.itemStackIndex--;
			s_BTInfo.itemCount = count;
			MetaCategoryIndexChange(DeviceSourceBluetooth, count, 0);
		}

		MetaCategoryMenuName(DeviceSourceBluetooth, META_FOLDER, "Folder Browsing");
	}
}

void TMPlayerBT_MetaBrowsingEnd(int index)
{
	DEBUG_TMPLAYER_BT_PRINTF("Index: %d\n", index);

	s_BTInfo.itemPerPage = 0;
	TMPlayerBTUid_Release(&s_BTInfo.folderListUid);
	if (s_BTInfo.itemType != NULL)
	{
		free(s_BTInfo.itemType);
		s_BTInfo.itemType = NULL;
	}
}

void TMPlayerBT_RefreshInfomation(void)
{
	TMPlayerBT_TitleInfo(s_BTInfo.title, strnlen(s_BTInfo.title, 512));
	TMPlayerBT_ArtistInfo(s_BTInfo.artist, strnlen(s_BTInfo.artist, 512));
	TMPlayerBT_AlbumInfo(s_BTInfo.album, strnlen(s_BTInfo.album, 512));
	TMPlayerBT_AlbumArt(s_BTInfo.albumArtLength);
	TMPlayerBT_RepeatMode(s_BTInfo.repeat);
	TMPlayerBT_ShuffleMode(s_BTInfo.shuffle);
	TMPlayerBT_PlayTimeChange(s_BTInfo.currentTime);
	TMPlayerBT_TotalTimeChange(s_BTInfo.totalTime);
	TMPlayerBT_TotalTrackChange(s_BTInfo.totalNum);
}

void TMPlayerBT_ClearInformation(void)
{
	(void)memset(s_BTInfo.title, 0x00, 512);
	(void)memset(s_BTInfo.artist, 0x00, 512);
	(void)memset(s_BTInfo.album, 0x00, 512);

	s_BTInfo.albumArtLength = 0;
	s_BTInfo.repeat = RepeatModeOff;
	s_BTInfo.shuffle = ShuffleModeOff;
	s_BTInfo.currentTime = 0;
	s_BTInfo.totalTime = 0;
	s_BTInfo.totalNum = 0;
}

/***************************/
/*     Utility Function    */
/***************************/
static int32_t TMPlayerBT_isBTDevice(void)
{
	int32_t isBT = false;

	if(TMPlayer_GetDevice() == DeviceSourceBluetooth)
	{
		isBT = true;
	}
	return isBT;
}

static void TMPlayerBTUid_Create(BTUid **uidTable, uint32_t size)
{
	*uidTable = (BTUid *)malloc(sizeof(BTUid) * size);
	if (*uidTable != NULL)
	{
		memset(*uidTable, 0x00, sizeof(BTUid) * size);
		DEBUG_TMPLAYER_BT_PRINTF("Memory allocation Success: %d byte\n", sizeof(BTUid) * size);
	}
	else
	{
		(void)fprintf(stderr, "Memory allocation failed\n");
	}
}

static void TMPlayerBTUid_Release(BTUid **uidTable)
{
	if (*uidTable != NULL)
	{
		free(*uidTable);
		*uidTable = NULL;
	}
}

static void TMPlayerBTUid_SetValue(BTUid **uidTable, uint32_t *uid1, uint32_t *uid2, uint32_t idx, uint32_t cnt)
{
	int i;
	BTUid *pUid = *uidTable + idx;

	if (*uidTable != NULL)
	{
		pUid = *uidTable + idx;
		for (i = 0; i < cnt; i++)
		{
			pUid->uid1 = uid1[i];
			pUid->uid2 = uid2[i];
			pUid++;

			DEBUG_TMPLAYER_BT_PRINTF("%d's UID is Updated: %x %x\n", idx + i + 1, uid1[i], uid2[i]);
		}
	}
	else
	{
		(void)fprintf(stderr, "[TM PLAYER BT] %s: UID Table is NULL\n", __FUNCTION__);
	}
}

static bool TMPlayerBTQueue_isEmpty(void)
{
	bool ret = false;

	if (s_BTInfo.browsingRear == s_BTInfo.browsingFront)
	{
		ret = true;
	}

	return ret;
}

static bool TMPlayerBTQueue_isFull(void)
{
	bool ret = false;

	if ((s_BTInfo.browsingRear + 1) % MAX_QUEUE_SIZE == s_BTInfo.browsingFront)
	{
		ret = true;
	}

	return ret;
}

static bool TMPlayerBTQueue_push(BTQueue *q)
{
	bool ret = false;

	if (!TMPlayerBTQueue_isFull())
	{
		s_BTInfo.browsingQueue[s_BTInfo.browsingRear].number = q->number;
		s_BTInfo.browsingQueue[s_BTInfo.browsingRear].count = q->count;
		s_BTInfo.browsingQueue[s_BTInfo.browsingRear].browsingType = q->browsingType;

		s_BTInfo.browsingRear = (s_BTInfo.browsingRear + 1) % MAX_QUEUE_SIZE;
		ret = true;
	}
	else
	{
		(void)fprintf(stderr, "[TM PLAYER BT] %s: Queue is full\n", __FUNCTION__);
	}

	return ret;
}

static bool TMPlayerBTQueue_pop(BTQueue *q)
{
	bool ret = false;

	if (!TMPlayerBTQueue_isEmpty())
	{
		q->number = s_BTInfo.browsingQueue[s_BTInfo.browsingFront].number;
		q->count = s_BTInfo.browsingQueue[s_BTInfo.browsingFront].count;
		q->browsingType = s_BTInfo.browsingQueue[s_BTInfo.browsingFront].browsingType;
		s_BTInfo.browsingFront = (s_BTInfo.browsingFront + 1) % MAX_QUEUE_SIZE;

		ret = true;
	}
	else
	{
		(void)fprintf(stderr, "[TM PLAYER BT] %s: Queue is empty\n", __FUNCTION__);
	}

	return ret;
}

/***************************/
/*     Event Function      */
/***************************/
void TMPlayerBT_Connected(void)
{
	DEBUG_TMPLAYER_BT_PRINTF("\n");
	DeviceConnectProcess("bt");
	DeviceMountProcess("bt", NULL);
}

void TMPlayerBT_Disconnected(void)
{
	DEBUG_TMPLAYER_BT_PRINTF("\n");
	DeviceDisconnectProcess("bt");
	TMPlayerBTUid_Release(&s_BTInfo.playListUid);
	TMPlayerBTUid_Release(&s_BTInfo.folderListUid);
	if (s_BTInfo.itemType != NULL)
	{
		free(s_BTInfo.itemType);
		s_BTInfo.itemType = NULL;
	}
	(void)memset(s_BTInfo.itemStack, 0x00, sizeof(uint32_t) * MAX_STACK_DEPTH);
	s_BTInfo.itemStackIndex = 0;
	s_BTInfo.totalNum = 0;
	TMPlayerBT_ClearInformation();
}

void TMPlayerBT_PlayTimeChange(uint32_t time)
{
//	DEBUG_TMPLAYER_BT_PRINTF("time:%u\n", time);

    unsigned char hour, min, sec;

	s_BTInfo.currentTime = time;
    hour = (unsigned char)(time/3600);
    min  = (unsigned char)((time/60)%60);
    sec  = (unsigned char)(time%60);

	if(TMPlayerBT_isBTDevice())
	{
	    SendDBusPlayTimeChanged(hour, min, sec);
	}
}

void TMPlayerBT_TotalTimeChange(uint32_t totaltime)
{
//	DEBUG_TMPLAYER_BT_PRINTF("totaltime:%u\n", totaltime);

    unsigned char hour, min, sec;

	s_BTInfo.totalTime = totaltime;
    hour = (unsigned char)(totaltime/3600);
    min =  (unsigned char)((totaltime/60)%60);
    sec =  (unsigned char)(totaltime%60);

	if(TMPlayerBT_isBTDevice())
	{
	    SendDBusTotalTimeChanged(hour, min, sec);
	}
}

void TMPlayerBT_TitleInfo(const char *title, uint32_t length)
{
    DEBUG_TMPLAYER_BT_PRINTF("title:%s\n", title);

	memcpy(s_BTInfo.title, title, length);
	if(TMPlayerBT_isBTDevice() && (length != 0))
	{
		SendDBusFileNameChanged(title, length);
		SendDBusiD3Infomation(MetaCategoryTitle, title);
	}
}

void TMPlayerBT_ArtistInfo(const char *artist, uint32_t length)
{
    DEBUG_TMPLAYER_BT_PRINTF("artist:%s\n", artist);

	memcpy(s_BTInfo.artist, artist, length);
	if(TMPlayerBT_isBTDevice() && (length != 0))
	{
		SendDBusiD3Infomation(MetaCategoryArtist, artist);
	}
}

void TMPlayerBT_AlbumInfo(const char *album, uint32_t length)
{
	DEBUG_TMPLAYER_BT_PRINTF("album:%s\n", album);

	memcpy(s_BTInfo.album, album, length);
	if(TMPlayerBT_isBTDevice() && (length != 0))
	{
		SendDBusiD3Infomation(MetaCategoryAlbum, album);
	}
}

void TMPlayerBT_AlbumArtSupport(bool support, const char *uri)
{
    DEBUG_TMPLAYER_BT_PRINTF("album art support: %s, uri : %s\n", support ? "true" : "false", uri);

	s_BTInfo.albumArtSupport = support;
	if (support)
	{
		strncpy(s_BTInfo.albumArt, uri, sizeof(s_BTInfo.albumArt) - 1);
		s_BTInfo.albumArt[512] = '\0';
	}
	else
	{
		s_BTInfo.albumArt[0] = '\0';
		s_BTInfo.albumArtLength = 0;
	}
}

void TMPlayerBT_AlbumArt(uint32_t length)
{
	if(TMPlayerBT_isBTDevice())
	{
		if(s_BTInfo.albumArtSupport)
		{
			s_BTInfo.albumArtLength = length;
			SendDBusAlbumArtShmData((const char *)s_BTInfo.albumArt, length);
		}
		else
		{
			DEBUG_TMPLAYER_BT_PRINTF("AlbumArt Not Supported\n");
		}
	}
}

void TMPlayerBT_PlayStatusChange(uint32_t playstatus)
{
	int ret;
    DEBUG_TMPLAYER_BT_PRINTF("play status(%lu)\n", playstatus);

	if ((s_BTInfo.playStatus == PlayStatusStop) && (playstatus == PlayStatusPause))
	{
		playstatus = PlayStatusStop;
	}

	s_BTInfo.playStatus = playstatus;
	if (s_BTInfo.playStatus == PlayStatusPlaying)
	{
		ret = TMPlayer_RequestChangeDevice(DeviceSourceBluetooth, true);
		if (ret)
		{
			TMPlayer_SetPlayStatus_BT((int32_t)playstatus);
		}
		else
		{
			TMPlayerBT_PlayStop();
		}
	}
	else
	{
		if(TMPlayerBT_isBTDevice())
		{
			TMPlayer_SetPlayStatus_BT((int32_t)playstatus);
		}
	}
}

void TMPlayerBT_BrowsingSupport(bool support, uint32_t list_type)
{
    DEBUG_TMPLAYER_BT_PRINTF("support: %s, list_type: %s\n",
								support ? "true" : "false",
								(list_type == BRWS_TYPE_FOLDER) ? "Folder Browsing" : "List Browsing");

	s_BTInfo.browsingSupport = support;

	if (support)
	{
		SendDBusFileDBCompleted((DeviceSource)DeviceSourceBluetooth);
		SendDBusBluetoothBrowsingType(s_BTInfo.browsingType);

		s_BTInfo.itemIndex = FIRST_ITEM;
		s_BTInfo.itemStack[0] = 1;
		s_BTInfo.itemStackIndex = 0;

	}
}

void TMPlayerBT_SetBrowsingMode(bool folder)
{
	DEBUG_TMPLAYER_BT_PRINTF("Type to be changed: %s\n",
								(folder == true) ? "Folder Browsing" : "Play List Browsing");

	if (folder)
	{
		s_BTInfo.browsingType = BRWS_TYPE_FOLDER;
	}
	else
	{
		s_BTInfo.browsingType = BRWS_TYPE_PLAYLIST;
	}

	if (s_BTInfo.browsingSupport == true)
	{
		SendDBusBluetoothBrowsingType(s_BTInfo.browsingType);
	}
}

void TMPlayerBT_BrowsingList(uint32_t idx, uint32_t *uid1, uint32_t *uid2, uint32_t *types, const char **names, uint32_t cnt)
{
	unsigned int count, i;
	BTQueue q;

	DEBUG_TMPLAYER_BT_PRINTF("Start no: %u, Count: %u, %u\n", idx, cnt, s_BTInfo.totalNum);

	if ((s_BTInfo.browsingSupport) && (idx > 0) && (cnt > 0))
	{
		idx--;

		if (s_BTInfo.browsingType == BRWS_TYPE_PLAYLIST)
		{
			if (idx + cnt > s_BTInfo.totalNum)
			{
				cnt = s_BTInfo.totalNum - idx;
			}
			TMPlayerBTUid_SetValue(&s_BTInfo.playListUid, uid1, uid2, idx, cnt);
			SendDBusFileList(DeviceSourceBluetooth, MultiMediaContentTypeAudio, (int32_t)idx + 1, (char **)names, cnt);
		}
		else if (s_BTInfo.browsingType == BRWS_TYPE_FOLDER)
		{
			TMPlayerBTUid_SetValue(&s_BTInfo.folderListUid, uid1, uid2, 0, cnt);
			for (i = 0; i < cnt; i++)
			{
				s_BTInfo.itemType[i] = (types[i] == ITEM_TYPE_MEDIA) ? META_TRACK : META_FOLDER;
				MetaCategoryInfo(DeviceSourceBluetooth, s_BTInfo.itemIndex + i - 1, names[i], s_BTInfo.itemType[i]);
			}
		}

		if (TMPlayerBTQueue_pop(&q))
		{
			DEBUG_TMPLAYER_BT_PRINTF("Queue Pop: Idx: %d, Cnt: %d, Type: %s\n",
										q.number, q.count, (q.browsingType == BRWS_TYPE_FOLDER) ? "Folder" : "Play List");
			if (q.browsingType == s_BTInfo.browsingType)
			{
				if (SendDBusBluetoothBrowsingList((unsigned int)q.number, &q.count))
				{
					if (q.browsingType == BRWS_TYPE_FOLDER)
					{
						s_BTInfo.itemIndex = q.number;
					}
				}
				else
				{
					(void)fprintf(stderr, "[TM PLAYER BT] %s: Get list error\n", __FUNCTION__);
				}
			}
		}
	}
}

void TMPlayerBT_BrowsingChanged(void)
{
    DEBUG_TMPLAYER_BT_PRINTF("\n");

	SendDBusFileListChanged(DeviceSourceBluetooth, MultiMediaContentTypeAudio, s_BTInfo.currentNum, s_BTInfo.totalNum);
}

void TMPlayerBT_TrackChange(uint32_t tracknum)
{
    DEBUG_TMPLAYER_BT_PRINTF("tracknum:%u\n", tracknum);

	s_BTInfo.currentNum = tracknum;
    if(TMPlayerBT_isBTDevice() && (tracknum > 0))
    {
		SendDBusFileNumberUpdated(DeviceSourceBluetooth, MultiMediaContentTypeAudio, s_BTInfo.currentNum, s_BTInfo.totalNum);
    }
}

void TMPlayerBT_TotalTrackChange(uint32_t totalnum)
{
    DEBUG_TMPLAYER_BT_PRINTF("totalnum: %u (prev totalnum: %u)\n", totalnum, s_BTInfo.totalNum);

	if (totalnum > 0)
	{
		if (s_BTInfo.totalNum != totalnum)
		{
			TMPlayerBTUid_Release(&s_BTInfo.playListUid);
			TMPlayerBTUid_Create(&s_BTInfo.playListUid, totalnum);

			SendDBusTotalNumberChanged(DeviceSourceBluetooth, MultiMediaContentTypeAudio, totalnum);
		}
	}

	s_BTInfo.totalNum = totalnum;
}

void TMPlayerBT_RepeatMode(uint32_t repeatMode)
{
	TMPlayer_SetRepeatMode(DeviceSourceBluetooth, MultiMediaContentTypeAudio, repeatMode);

	if(TMPlayerBT_isBTDevice())
	{
		SendDBusRepeatModeChanged(repeatMode);
	}
}

void TMPlayerBT_ShuffleMode(uint32_t shuffleMode)
{
	TMPlayer_SetShuffleMode(DeviceSourceBluetooth, MultiMediaContentTypeAudio, shuffleMode);

	if(TMPlayerBT_isBTDevice())
	{
		SendDBusShuffleModeChanged(shuffleMode);
	}
}

void TMPlayerBT_RepeatChange(uint32_t repeat)
{
    DEBUG_TMPLAYER_BT_PRINTF("Repeat(%lu)\n", repeat);

	s_BTInfo.repeat = repeat;
	switch(repeat)
	{
		case BTRepeatOff:
			s_BTInfo.repeat = RepeatModeOff;
			break;
		case BTRepeatSingle:
			s_BTInfo.repeat = RepeatModeTrack;
			break;
		case BTRepeatAll:
		case BTRepeatGroup:
			s_BTInfo.repeat = RepeatModeAll;
			break;
		default :
			break;
	}

	TMPlayer_SetRepeatMode(DeviceSourceBluetooth, MultiMediaContentTypeAudio, s_BTInfo.repeat);

	if(TMPlayerBT_isBTDevice())
	{
		SendDBusRepeatModeChanged(s_BTInfo.repeat);
	}
}

void TMPlayerBT_ShuffleChange(uint32_t shuffle)
{
    DEBUG_TMPLAYER_BT_PRINTF("Shuffle(%lu)\n", shuffle);

	s_BTInfo.shuffle = shuffle;
	switch(shuffle)
	{
		case BTShuffleOff:
			s_BTInfo.shuffle = ShuffleModeOff;
			break;
		case BTShuffleAll:
		case BTShuffleGroup:
			 s_BTInfo.shuffle= ShuffleModeOn;
			break;
		default :
			break;
	}

	TMPlayer_SetShuffleMode(DeviceSourceBluetooth, MultiMediaContentTypeAudio, s_BTInfo.shuffle);

	if(TMPlayerBT_isBTDevice())
	{
		SendDBusShuffleModeChanged(s_BTInfo.shuffle);
	}
}
