/****************************************************************************************
 *   FileName    : TMPlayer.c
 *   Description : 
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
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <glib.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <dbus/dbus.h>
#include <vector>
#include <limits>

#include "nls720.h"
#include "TMPlayerDBus.h"
#include "TMPlayerDBusMediaPlayback.h"
#include "TMPlayerType.h"
#include "TMPlayerConnect.h"
#include "TCDBDefine.h"
#include "TCDBGen.h"
#include "TMPlayerControl.h"
#include "MetaParser.h"
#include "UI_META_browsing.h"
#include "TMPlayerDB.h"
#include "TMPlayer.h"
#include "TMPlayerDBusModeManager.h"
#include "TMPlayerAux.h"
#ifdef HAVE_IAP2
#include "TMPlayeriAP2.h"
#endif
#ifdef HAVE_BT
#include "TMPlayerBT.h"
#endif

static int s_playIDCount;
static int s_playID;
extern int g_tc_debug;
#define DEBUG_TMPLAYER_PRINTF(format, arg...) \
	if (g_tc_debug) \
	{ \
		fprintf(stderr, "[TMPLAYER] %s: " format "", __FUNCTION__, ##arg); \
	}


#define INVALID_NUMBER			((unsigned int)-1)
#define INTRO_TIME				10
#define MUSIC_DIR_NAME			"/music"
#define ALBUM_ART_WIDTH		260
#define ALBUM_ART_HEIGHT		260

typedef struct _MediaInfo
{
	bool _haveContent;
	MultiMediaContentType	_contentType;
	char _mntPath[4096];
	bool _fileDBReady;
	bool _metaDBReady;
	DataBaseInfo _db[TotalMultiMediaContentTypes];
} MediaInfo;

typedef struct _DeviceInfo
{
	char _key[KEY_STRING_SIZE];
	bool _used;
} DeviceInfo;

static PlayInfo					*_playInfo = NULL;
static DataBaseInfo			*_dbInfo = NULL;
static MediaInfo				_totalMediaInfo[TotalDeviceSources];
static DeviceSource			_device = TotalDeviceSources;
static DeviceInfo				_connectedDevice[TotalDeviceSources];
static char					_myMusicPath[MAX_PATH_LENGTH];
static char					_tempFilePath[MAX_PATH_LENGTH];
static int						_initStart;

bool _displayResource;
bool _audioResource;
bool _avOff = true;
bool _videoPlay = false;

static bool s_detect = false;

static pthread_mutex_t connectMutex;

static DeviceSource GetConnectedDevice(char *name);
static DeviceSource DeleteConnectedDevice(char *name);

static void DeviceMountErrorProcess(char *key);

static void FileDBResult(unsigned int dbNum, int result);
static void FileDBUpdate(unsigned int dbNum, ContentsType type);
static void MetaDBResult(unsigned int dbNum, int result);
static void MetaDBUpdate(unsigned int dbNum);

static void UpdateFileNumber(DeviceSource device, MultiMediaContentType content, unsigned int totalNumber);
static bool HaveDeviceContent(DeviceSource device, MultiMediaContentType content);
static bool IsKeepMount(const char *path);
static MultiMediaContentType ConversionTablebyMediaType(ContentsType type);
static int ChangeContent(MultiMediaContentType content);
static bool ChangeDevice(DeviceSource device, bool play);
static bool PlayStart(int number, PlayTime playTime);
static bool GetFileName(std::string &fileName, std::string &filePath);
static bool GetFolderName(std::string &folderName, std::string &filePath);
static void RefreshFileNumber(DeviceSource device, MultiMediaContentType content, bool isclearList);
static void IncreasePlayID(void);
static void PlayStartDevice(DeviceSource device);
static void PlayStopDevice(DeviceSource device);
static int IsStorageDevice (DeviceSource device);

int TMPlayer_Initialize()
{
	DEBUG_TMPLAYER_PRINTF("\n");
	pid_t pid;
	int ret, i, j;

	struct passwd *pw = getpwuid(getuid());
	std::string _myDBPath;
	memset(_myMusicPath, 0x00, MAX_PATH_LENGTH);

	(void)pthread_mutex_init(&connectMutex, NULL);

	_initStart = -1;

	if (pw != NULL)
	{
		_myDBPath = pw->pw_dir;
	}
	else
	{
		fprintf(stderr, "get user home directory failed.\n set default path\n");
		_myDBPath = "/home/invite";
	}

	errno =0;
	_myDBPath.append("/.telechips");
	if( mkdir(_myDBPath.data(), S_IRWXU|S_IRGRP|S_IXGRP) == -1)
	{
		if(errno != 17)
		{
			fprintf(stderr, "%s : mkdir error (%s), error(%d) - %s\n",__FUNCTION__, _myDBPath.data(), errno, strerror (errno));
		}
	}

	strncpy(_myMusicPath, _myDBPath.data(), MAX_PATH_LENGTH);
	strcat(_myMusicPath, MUSIC_DIR_NAME);
	if(mkdir(_myMusicPath, S_IRWXU|S_IRGRP|S_IXGRP)== -1)
	{
		if(errno != 17)
		{
			fprintf(stderr, "%s : mkdir error (%s), error(%d) - %s\n",__FUNCTION__, _myDBPath.data(), errno, strerror (errno));
		}
	}

	_myDBPath.append("/database");
	if( mkdir(_myDBPath.data(), S_IRWXU|S_IRGRP|S_IXGRP) == -1)
	{
		if(errno != 17)
		{
			fprintf(stderr, "%s : mkdir error (%s), error(%d) - %s\n",__FUNCTION__, _myDBPath.data(), errno, strerror (errno));
		}
	}

	s_playIDCount = 0;
	s_playID = 0;

	_displayResource = false;
	_audioResource = false;

	for (i = DeviceSourceMyMusic; i < TotalDeviceSources; i++)
	{

		memset(_connectedDevice[i]._key, 0, KEY_STRING_SIZE);
		_connectedDevice[i]._used = false;

		_totalMediaInfo[i]._haveContent = false;

		_totalMediaInfo[i]._contentType = TotalMultiMediaContentTypes;
		_totalMediaInfo[i]._fileDBReady= false;
		_totalMediaInfo[i]._metaDBReady= false;

		for (j = MultiMediaContentTypeAudio; j < TotalMultiMediaContentTypes; j++)
		{
			_totalMediaInfo[i]._db[j]._device = (DeviceSource)i;
			_totalMediaInfo[i]._db[j]._contentType = (MultiMediaContentType)j;
			_totalMediaInfo[i]._db[j]._shuffle = false;
			_totalMediaInfo[i]._db[j]._isMeta = false;
			//_totalMediaInfo[i]._db[j]._folderCount = 0;
			//_totalMediaInfo[i]._db[j]._fileCount = 0;
			_totalMediaInfo[i]._db[j]._totalNumber = 0;
			_totalMediaInfo[i]._db[j]._playInfo._playStatus = PlayStatusStop;
			_totalMediaInfo[i]._db[j]._playInfo._repeatMode = RepeatModeOff;
			_totalMediaInfo[i]._db[j]._playInfo._shuffleMode = ShuffleModeOff;
			_totalMediaInfo[i]._db[j]._playInfo._playTime._hour = 0;
			_totalMediaInfo[i]._db[j]._playInfo._playTime._min = 0;
			_totalMediaInfo[i]._db[j]._playInfo._playTime._sec = 0;
			_totalMediaInfo[i]._db[j]._playInfo._fileNumber = INVALID_NUMBER;
			_totalMediaInfo[i]._db[j]._playInfo._listIndex = INVALID_INDEX;
			_totalMediaInfo[i]._db[j]._playInfo._dbIndex = INVALID_INDEX;
		}
	}

	/*Init language for id3 tag string conveting */
	InitNLS();

	/* Init Db config */
	TMPlayerDB_EventCB dbEventcallback;
	dbEventcallback._TMPlayerDBFileDBResult = FileDBResult;
	dbEventcallback._TMPlayerDBFileDBUpdate = NULL;
	dbEventcallback._TMPlayerDBMetaDBResult = MetaDBResult;
	dbEventcallback._TMPlayerDBMetaDBUpdate = NULL;
	TMPlayerDB_SetEventCallback(&dbEventcallback);

	ret = TMPlayerDB_Initialize(_myDBPath.data());

	/* Init Connect */
	TMPlayerConnect_DeviceEventCB connectEventcallback;
	connectEventcallback.TMPlayerConnectDeviceConnected = DeviceConnectProcess;
	connectEventcallback.TMPlayerConnectDeviceDisConnected = DeviceDisconnectProcess;
	connectEventcallback.TMPlayerConnectDeviceMounted = DeviceMountProcess;
	connectEventcallback.TMPlayerConnectDeviceMountErrorOccured = DeviceMountErrorProcess;
	TMPlayerConnect_SetEventCallback(&connectEventcallback);
	TMPlayerConnect_Initialize();

	/* Init MetaBrws */
	TCDB_MetaBrwsEventCB metaCallBacks;
	metaCallBacks.MetaCategoryMenuName = MetaCategoryMenuName;
	metaCallBacks.MetaCategoryIndexChange = MetaCategoryIndexChange;
	metaCallBacks.MetaCategoryInfo = MetaCategoryInfo;
	metaCallBacks.MetaSelectTrack = MetaSelectTrack;
	UI_META_SetEventCallbackFunctions(&metaCallBacks);

	/* Init Aux */
	if(AuxPlayInit(DeviceConnectProcess, DeviceDisconnectProcess) != 0)
	{
		fprintf(stderr, "%s: AuxPlayInit error.\n", __func__);
	}

	pid = fork();

	if (pid == 0) // -1: error, 0: child, others: parent
	{
		execl("/bin/rm", "rm", "-rf", _myMusicPath, (char *)0);
	}
	else if (pid == -1)
	{
		fprintf(stderr, "%s: fork for remove off failed\n", __func__);
	}

	return ret;
}

void TMPlayer_Release()
{
	DEBUG_TMPLAYER_PRINTF("\n");
	AuxPlayDeinit();
	TMPlayerDB_Release();
	TMPlayerConnect_Release();
	pthread_mutex_destroy(&connectMutex);
}

void DeviceConnectProcess(char *key)
{
	(void)pthread_mutex_lock(&connectMutex);

	if (key != NULL)
	{
		DeviceInfo deviceInfo;
		DeviceSource device;
		bool inserted = false;

		memcpy(deviceInfo._key, key, KEY_STRING_SIZE);

		device = GetConnectedDevice(deviceInfo._key);
		if (device == TotalDeviceSources)
		{
			if (strncmp(key, "sd", 2) == 0)		//sda, sdb, sdc
			{
				if (!_connectedDevice[DeviceSourceUSB1]._used)
				{
					device = DeviceSourceUSB1;
				}
				else if (!_connectedDevice[DeviceSourceUSB2]._used)
				{
					device = DeviceSourceUSB2;
				}
				else if (!_connectedDevice[DeviceSourceUSB3]._used)
				{
					device = DeviceSourceUSB3;
				}
				else
				{
					fprintf(stderr, "%s: usb device full(%s)\n", __func__, key);
				}
			}
			else if (strncmp(key, "mmc", 3) == 0)
			{
				device = DeviceSourceSDMMC;
			}
			else if (strncmp(key, "iAP", 3) == 0)
			{
				device = DeviceSourceiAP2;
			}
			else if(strncmp(key, "aux", 3) == 0)
			{
				device = DeviceSourceAUX;
			}
			else if(strncmp(key, "bt", 2) == 0)
			{
				device = DeviceSourceBluetooth;
			}			
			else
			{
				fprintf(stderr, "%s: unknown device(%s)\n", __func__, key);
			}

			if (device >= DeviceSourceMyMusic && device < TotalDeviceSources)
			{
				if (!_connectedDevice[device]._used)
				{
					_connectedDevice[device] = deviceInfo;
					_connectedDevice[device]._used = true;
					inserted = true;
				}
				else
				{
					fprintf(stderr, "%s: exist block device(%s), device enum (%d)\n", __func__, key, device);
				}
			}
		}
		else
		{
			fprintf(stderr, "%s: exist block device(%s), device enum (%d)\n", __func__, key, device);
		}

		if (inserted)
		{
			DEBUG_TMPLAYER_PRINTF("CONNECT DEVICE(%s) SUCCEDDED\n", key);
			if (s_detect)
			{
				SendDBusDeviceConnect(device);

				if(device == DeviceSourceAUX)
				{
					TMPlayer_NotifyAuxReady();
				}
			}
		}
		else
		{
			DEBUG_TMPLAYER_PRINTF("CONNECT DEVICE(%s) FAILED\n", key);
		}
	}
	else
	{
		fprintf(stderr, "%s: buffer is null\n", __func__);
	}
	(void)pthread_mutex_unlock(&connectMutex);
}

void DeviceDisconnectProcess(char *key)
{
	DeviceSource device;

	(void)pthread_mutex_lock(&connectMutex);

	device = DeleteConnectedDevice(key);
	if (device != TotalDeviceSources)
	{
		if (device >= DeviceSourceMyMusic && device < TotalDeviceSources)
		{
			if((device == DeviceSourceAUX) || (device == DeviceSourceiAP2))
			{
				_totalMediaInfo[device]._haveContent = false;
			}
			else
			{
				DataBaseInfo *db;
				TMPlayerDB_DeleteDateBase(device);
				(void)UI_META_NAVI_DeleteNaviInfo(device);
				//_totalMediaInfo[device]._contentType = TotalMultiMediaContentTypes;
				_totalMediaInfo[device]._haveContent = false;
				for (int i = MultiMediaContentTypeAudio; i < TotalMultiMediaContentTypes; i++)
				{
					db = &_totalMediaInfo[device]._db[i];
					//db->_device = TotalDeviceSources;
					db->_shuffle = false;
					//db->_folderCount = 0;
					//db->_fileCount = 0;
					db->_totalNumber = 0;
					db->_list.clear();
					db->_shuffleList.clear();

					db->_playInfo._playStatus = PlayStatusStop;
					db->_playInfo._repeatMode = RepeatModeOff;
					db->_playInfo._shuffleMode = ShuffleModeOff;
					db->_playInfo._playTime._hour = 0;
					db->_playInfo._playTime._min = 0;
					db->_playInfo._playTime._sec = 0;
					db->_playInfo._fileNumber = INVALID_NUMBER;
					db->_playInfo._listIndex = INVALID_INDEX;
					db->_playInfo._dbIndex = INVALID_INDEX;

				}
			}

			_connectedDevice[device]._used = false;
			DEBUG_TMPLAYER_PRINTF("DISCONNECT DEVICE(%s)\n", key);
			if (s_detect)
			{
				SendDBusDeviceDisconnect(device);

				if(_device == device)
				{
					PlayStopDevice(_device);
					TMPlayer_ChangeNextDeviceProcess(false);
				}
			}
			/* have not gui only hmi */
			/*_device = TotalDeviceSources;*/
		}
	}
	else
	{
		fprintf(stderr, "%s: not inserted key (%s), device(%d)\n", __func__, key, device);
	}
	(void)pthread_mutex_unlock(&connectMutex);
}

void DeviceMountProcess(char *key, char *mountPath)
{
	char disk_key[KEY_STRING_SIZE];

	if (key != NULL)
	{
		DeviceSource device;

		if (strncmp(key, "sd", 2) == 0) // sda, sdb, ...
		{
			memcpy(disk_key, key, 3);
			disk_key[3] = '\0';
		}
		else if (strncmp(key, "mmc", 3) == 0) // mmcblk2 ...
		{
			memcpy(disk_key, key, 7);
			disk_key[7] = '\0';
		}
		else //aux, iAP2, A2DP
		{
			strncpy(disk_key, key, strlen(key) + 1);
		}

		device = GetConnectedDevice(disk_key);
		if (device != TotalDeviceSources)
		{
			if (device >= DeviceSourceMyMusic && device < DeviceSourceAUX)
			{
				_totalMediaInfo[device]._fileDBReady = false;
				_totalMediaInfo[device]._metaDBReady= false;
				strcpy(_totalMediaInfo[device]._mntPath, mountPath);
				if (s_detect)
				{
					if(TMPlayerDB_AddDataBase(device, mountPath)==false)
					{
						DEBUG_TMPLAYER_PRINTF("MOUNT DEVICE(%s) FAILED\n", disk_key);
						SendDBusDeviceMountStop((DeviceSource)device);
					}
				}
			}
#ifdef HAVE_IAP2
			else if(device == DeviceSourceiAP2)
			{
				TMPlayer_NotifyiAP2Ready();
			}
#endif
#ifdef HAVE_BT
			else if(device == DeviceSourceBluetooth)
			{
				TMPlayer_NotifyBTReady();
			}
#endif
			else
			{
				; /* TODO: Other device */
			}
		}
		else
		{
			fprintf(stderr, "%s: not inserted key (%s), device(%d)\n", __func__, disk_key, device);
		}
	}
	else
	{
		fprintf(stderr, "%s: buffer is null\n", __func__);
	}
}

static void DeviceMountErrorProcess(char *key)
{
	DeviceSource device;

	device = DeleteConnectedDevice(key);
	if (device != TotalDeviceSources)
	{
		if (device >= DeviceSourceMyMusic && device < TotalDeviceSources)
		{
			if (_device == device)
			{
				if(s_playID != 0)
				{
					TMPlayerControl_Stop(s_playID);
					s_playID = 0;
				}
			}
			SendDBusDeviceMountStop((DeviceSource)device);
		}
		else
		{
			fprintf(stderr, "%s: invalid device(%d)\n", __func__, device);
		}
	}
	else
	{
		fprintf(stderr, "%s: not connected device(%s)\n", __func__, key);
	}
}

void TMPlayer_SetBrowsingMode(unsigned int device, unsigned char content, bool meta)
{
	DEBUG_TMPLAYER_PRINTF("Device: %u, Content: %d, Meta: %s\n",
							device, content, meta ? "Meta" : "PlayList");
	_totalMediaInfo[device]._db[content]._isMeta = meta;

#ifdef HAVE_BT
	if (device == DeviceSourceBluetooth)
	{
		TMPlayerBT_SetBrowsingMode(meta);
	}
#endif
}

void TMPlayer_SetRepeatMode(unsigned int device, unsigned char content, unsigned int mode)
{
	DEBUG_TMPLAYER_PRINTF("Device: %u, Repeat: %d\n", device, mode);
	_totalMediaInfo[device]._db[content]._playInfo._repeatMode = RepeatModeOff;
}

void TMPlayer_SetShuffleMode(unsigned int device, unsigned char content, unsigned int mode)
{
	DEBUG_TMPLAYER_PRINTF("Device: %u, Shuffle: %d\n", device, mode);
	_totalMediaInfo[device]._db[content]._playInfo._shuffleMode = ShuffleModeOff;
}

void TMPlayer_ChangeContentTypeProcess(void)
{
	bool exist = false;
	int contentType, ret;

	DEBUG_TMPLAYER_PRINTF("\n");

	contentType = (int)_totalMediaInfo[_device]._contentType;

	for (int i = 0; (i < TotalMultiMediaContentTypes) && (!exist); i++)
	{
		contentType++;
		if (contentType >= TotalMultiMediaContentTypes)
		{
			contentType = MultiMediaContentTypeAudio;
		}

		exist = HaveDeviceContent(_device, (MultiMediaContentType)contentType);
	}

	if (exist && (_totalMediaInfo[_device]._contentType != contentType))
	{
		ret = ChangeContent((MultiMediaContentType)contentType);

		if (ret > 0)
		{
			if (contentType == MultiMediaContentTypeVideo)
			{
				DEBUG_TMPLAYER_PRINTF("Video Go Go\n");
				ret = SendDBusModeManagerChangeMode(VIDEOPLAY);
			}
			else
			{
				DEBUG_TMPLAYER_PRINTF("Audio Go Go\n");
				ret = SendDBusModeManagerChangeMode(AUDIOPLAY);
			}

			if (ret <= 0)
			{
				DEBUG_TMPLAYER_PRINTF("Resource Request is denied\n");
			}
		}
	}
	else
	{
		fprintf (stderr, "%s: Content Change Failed\n", __PRETTY_FUNCTION__);
	}
}

void TMPlayer_ChangeNextDeviceProcess(bool play)
{
	unsigned int device = _device;
	bool stop = false;
	unsigned int deviceCount = 0;
	int ret = 0;
	DEBUG_TMPLAYER_PRINTF("play(%d)\n", play);

	while (!stop && deviceCount < TotalDeviceSources)
	{
		device++;
		if (device >= TotalDeviceSources)
		{
			device = DeviceSourceMyMusic;
		}

		DEBUG_TMPLAYER_PRINTF("device (%d), hanve content(%d)\n", device, _totalMediaInfo[device]._haveContent);
		if (_totalMediaInfo[device]._haveContent)
		{
			if ((DeviceSource)device != _device)
			{
				stop = ChangeDevice((DeviceSource)device, play || _audioResource);
			}
			else
			{
				stop = true;
			}
		}

		deviceCount++;
	}

	if (!stop)
	{
		_device = TotalDeviceSources;

		SendDBusNoDevice();
		// No Device... releasing resources and go idle mode
		ReleaseAllResource(DO_IDLE);
	}
}

void TMPlayer_ChangeDeviceProcess(int device, bool play)
{
	bool changed = false;

	DEBUG_TMPLAYER_PRINTF("devcie : %d, play : %d \n", device, play);
	if (device < TotalDeviceSources)
	{
		if (_totalMediaInfo[device]._haveContent)
		{
			changed = ChangeDevice((DeviceSource)device, play || _audioResource);
		}
	}
	if (!changed)
	{
		TMPlayer_ChangeNextDeviceProcess(play);
	}
}

void TMPlayer_ChangeRepeatModeProcess(void)
{
	bool result = true;

	if (IsStorageDevice(_device))
	{
		if (_playInfo->_repeatMode == RepeatModeAll)
		{
			_playInfo->_repeatMode = RepeatModeTrack;

			if (_dbInfo->_isMeta)
			{
				_dbInfo->_metaList.clear();
				_dbInfo->_metaList.push_back(_playInfo->_dbIndex);
			}
			else
			{
				_dbInfo->_list.clear();
				_dbInfo->_list.push_back(_playInfo->_dbIndex);
			}
			_dbInfo->_shuffleList.clear();
			_dbInfo->_shuffleList.push_back(1);
			_dbInfo->_totalNumber = 1;
		}
		else
		{
			_playInfo->_repeatMode = RepeatModeAll;

			result = TMPlayerDB_CreatePlayList(_device, _dbInfo->_contentType, _dbInfo->_isMeta, false, INVALID_INDEX, _dbInfo);
		}

		if (result)
		{
			SendDBusRepeatModeChanged(_playInfo->_repeatMode);
			SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_totalNumber);
			RefreshFileNumber(_device, _dbInfo->_contentType, false);
		}
		else
		{
			SendDBusDeviceMountStop(_device);
		}
	}
#ifdef HAVE_IAP2
	else if(_device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_ChangeRepeat();
	}
#endif
#ifdef HAVE_BT
	else if(_device == DeviceSourceBluetooth)
	{
		TMPlayerBT_ChangeRepeat();
	}
#endif
}

void TMPlayer_ChangeShuffleModeProcess(void)
{
	bool result = true;

	if (IsStorageDevice(_device))
	{
		if (_playInfo->_shuffleMode == ShuffleModeOff)
		{
			_playInfo->_shuffleMode = ShuffleModeOn;
			_dbInfo->_shuffle = true;
		}
		else if (_playInfo->_shuffleMode == ShuffleModeOn)
		{
			_playInfo->_shuffleMode = ShuffleModeOff;
			_dbInfo->_shuffle = false;
		}

		if (_playInfo->_repeatMode == RepeatModeAll)
		{
			result = TMPlayerDB_CreatePlayList(_device, _dbInfo->_contentType, _dbInfo->_isMeta, false, INVALID_INDEX, _dbInfo);
		}

		if (result)
		{
			SendDBusShuffleModeChanged(_playInfo->_shuffleMode);
			SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_totalNumber);
			RefreshFileNumber(_device, _dbInfo->_contentType, false);
		}
		else
		{
			SendDBusDeviceMountStop(_device);
		}
	}
#ifdef HAVE_IAP2
	else if(_device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_ChangeShuffle();
	}
#endif
#ifdef HAVE_BT
	else if(_device == DeviceSourceBluetooth)
	{
		TMPlayerBT_ChangeShuffle();
	}
#endif
}

void TMPlayer_FileListRequestProcess(unsigned int device, unsigned char content, int number, unsigned int count)
{
	DeviceSource dev = (DeviceSource)device;
	DEBUG_TMPLAYER_PRINTF("device(%u), content(%u), number(%d), count(%u)\n",
							 device, content, number, count);
	if ((dev >= DeviceSourceMyMusic) && (dev < DeviceSourceAUX) && (number > 0))
	{
		DataBaseInfo *db;
		int index = number - 1;
		unsigned int dbIndex = INVALID_INDEX;
		db = &_totalMediaInfo[dev]._db[content];
		unsigned int totalLength = 0;
		std::string filePath, fileName;
		std::vector<std::string> fileList;

		for (unsigned int i = 0; i < count && number <= (int)db->_list.size(); i++)
		{
			if (_dbInfo->_isMeta)
			{
				if (index < (int)_totalMediaInfo[_device]._db[ _dbInfo->_contentType]._metaList.size())
				{
					dbIndex = _totalMediaInfo[_device]._db[ _dbInfo->_contentType]._metaList[index];
				}
			}
			else
			{
				if (index < (int)_totalMediaInfo[_device]._db[ _dbInfo->_contentType]._list.size())
				{
					dbIndex = _totalMediaInfo[_device]._db[ _dbInfo->_contentType]._list[index];
				}
			}
			if (TMPlayerDB_GetFilePath(dev, (MultiMediaContentType)content, dbIndex, filePath))
			{
				if (GetFileName(fileName, filePath))
				{
					fileList.push_back(fileName.data());
					totalLength += fileName.length() + 1;
				}
			}
			index++;
		}
		char **Buffer = (char**)malloc(sizeof(char*)*fileList.size());
		for(int i = 0; i < fileList.size(); i++)
		{
			Buffer[i] = (char*)malloc(fileList[i].size()+1);
			memset(Buffer[i], 0x00, fileList[i].size()+1);
			strncpy(Buffer[i], fileList[i].c_str(), fileList[i].size());
		}
		SendDBusFileList(device, content, number, Buffer, fileList.size());

		for(int i = 0; i < fileList.size(); i++)
		{
			free(Buffer[i]);
		}
		free(Buffer);
	}
#ifdef HAVE_BT
	else if (dev == DeviceSourceBluetooth)
	{
		TMPlayerBT_RequestList(number, count);
	}
#endif
}

void TMPlayer_MetaBrowsingStartBrowsing(unsigned int device, unsigned int count)
{
	if (IsStorageDevice((DeviceSource)device))
	{
		UI_META_NAVI_StartBrowse(device, count);
	}
#ifdef HAVE_IAP2
	else if(device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_MetaBrowsingStart(count);
	}
#endif
#ifdef HAVE_BT
	else if(device == DeviceSourceBluetooth)
	{
		TMPlayerBT_MetaBrowsingStart(count);
	}
#endif
}

void TMPlayer_MetaBrowsingRequestList(unsigned int device, int index)
{
	if (IsStorageDevice((DeviceSource)device))
	{
		UI_META_NAVI_Move(device, index);
	}
#ifdef HAVE_IAP2
	else if(device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_MetaBrowsingDown(index);
	}
#endif
#ifdef HAVE_BT
	else if(device == DeviceSourceBluetooth)
	{
		TMPlayerBT_MetaBrowsingMove(index);
	}
#endif
}

void TMPlayer_MetaBrowsingSelectItem(unsigned int device, int index)
{
	if (IsStorageDevice((DeviceSource)device))
	{
		UI_META_NAVI_SelectChildItem(device, index);
	}
#ifdef HAVE_IAP2
	else if(device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_MetaBrowsingSelect(index);
	}
#endif
#ifdef HAVE_BT
	else if(device == DeviceSourceBluetooth)
	{
		TMPlayerBT_MetaBrowsingSelect(index);
	}
#endif
}

void TMPlayer_MetaBrowsingMoveToHome(unsigned int device, unsigned int count)
{
	if (IsStorageDevice((DeviceSource)device))
	{
		UI_META_NAVI_HomeBrowse(device, count);
	}
#ifdef HAVE_IAP2
	else if(device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_MetaBrowsingHome(count);
	}
#endif
#ifdef HAVE_BT
	else if(device == DeviceSourceBluetooth)
	{
		TMPlayerBT_MetaBrowsingHome(count);
	}
#endif
}

void TMPlayer_MetaBrowsingMoveToPreviousMenu(unsigned int device)
{
	if (IsStorageDevice((DeviceSource)device))
	{
		UI_META_NAVI_SelectParentItem(device);
	}
#ifdef HAVE_IAP2
	else if(device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_MetaBrowsingUndo();
	}
#endif
#ifdef HAVE_BT
	else if(device == DeviceSourceBluetooth)
	{
		TMPlayerBT_MetaBrowsingUndo();
	}
#endif
}

void TMPlayer_MetaBrowsingEndBrowsing(unsigned int device, int index)
{
	if (IsStorageDevice((DeviceSource)device))
	{
		UI_META_NAVI_ExitBrowse(device, index);
	}
#ifdef HAVE_IAP2
	else if(device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_MetaBrowsingEnd(index);
	}
#endif
#ifdef HAVE_BT
	else if(device == DeviceSourceBluetooth)
	{
		TMPlayerBT_MetaBrowsingEnd(index);
	}
#endif
}

void TMPlayer_TrackStartProcess(void)
{
	DEBUG_TMPLAYER_PRINTF("\n");
	if ((_device >= DeviceSourceMyMusic) && (_device < TotalDeviceSources))
	{
		if (IsStorageDevice(_device))
		{
			DEBUG_TMPLAYER_PRINTF("INDEX(%d)\n", _playInfo->_listIndex);

			if (_dbInfo->_totalNumber > 0)
			{
				if (_playInfo->_listIndex == _dbInfo->_shuffleList.size())
				{
					_playInfo->_listIndex = 0;
				}

				_playInfo->_fileNumber = _dbInfo->_shuffleList[_playInfo->_listIndex];
				SendDBusFileNumberChanged(_playInfo->_fileNumber);
			}
			else
			{
				fprintf(stderr, "TM Player: %s Invalid Device\n", __func__);
			}
		}

		PlayStartDevice(_device);
	}
}

void TMPlayer_TrackRestartProcess(void)
{
	DEBUG_TMPLAYER_PRINTF("\n");
	if (_device >= DeviceSourceMyMusic &&
		_device < TotalDeviceSources)
	{
		if(_device == DeviceSourceAUX)
		{
			AuxPlayOn();
		}
#ifdef HAVE_IAP2
		else if(_device == DeviceSourceiAP2)
		{
			TMPlayeriAP2_PlayStart();
		}
#endif
#ifdef HAVE_BT
		else if(_device == DeviceSourceBluetooth)
		{
			TMPlayerBT_PlayRestart();
		}
#endif
		else
		{
			DEBUG_TMPLAYER_PRINTF("INDEX(%d)\n",
								   _playInfo->_listIndex);

			if (_dbInfo->_totalNumber > 0)
			{
				if (_playInfo->_listIndex == _dbInfo->_shuffleList.size())
				{
					_playInfo->_listIndex = 0;
				}

				_playInfo->_fileNumber = _dbInfo->_shuffleList[_playInfo->_listIndex];
				SendDBusFileNumberChanged(_playInfo->_fileNumber);
				PlayStart(_playInfo->_fileNumber, _playInfo->_playTime);
			}
			else
			{
				fprintf(stderr, "TM Player: %s Invalid Device\n", __func__);
			}
		}
	}
}

void TMPlayer_TrackUpProcess(void)
{
	DEBUG_TMPLAYER_PRINTF("\n");
	if (_device >= DeviceSourceMyMusic &&
		_device < TotalDeviceSources)
	{
#ifdef HAVE_IAP2
		if(_device == DeviceSourceiAP2)
		{
			TMPlayeriAP2_TrackUp();
		}
		else
#endif
#ifdef HAVE_BT
		if(_device == DeviceSourceBluetooth)
		{
			TMPlayerBT_TrackUp();
		}
		else
#endif
		if (IsStorageDevice(_device))
		{
			if (_dbInfo->_totalNumber > 0)
			{
				_playInfo->_listIndex++;
				if (_playInfo->_listIndex == _dbInfo->_shuffleList.size())
				{
					_playInfo->_listIndex = 0;
				}

				_playInfo->_fileNumber = _dbInfo->_shuffleList[_playInfo->_listIndex];
				_playInfo->_playTime._hour = 0;
				_playInfo->_playTime._min = 0;
				_playInfo->_playTime._sec = 0;
				SendDBusFileNumberChanged(_playInfo->_fileNumber);
				IncreasePlayID();
				PlayStart(_playInfo->_fileNumber, _playInfo->_playTime);
			}
			else
			{
				fprintf(stderr, "TM Player: %s Invalid Device\n", __func__);
			}
		}
	}
}

void TMPlayer_TrackDownProcess(void)
{
	DEBUG_TMPLAYER_PRINTF("\n");
	if (_device >= DeviceSourceMyMusic &&
		_device < TotalDeviceSources)
	{
		if (IsStorageDevice(_device))
		{
			if ((_playInfo->_playTime._hour == 0)
				&& (_playInfo->_playTime._min == 0)
				&& (_playInfo->_playTime._sec < 5))
			{
				if (_dbInfo->_totalNumber > 0)
				{
					_playInfo->_listIndex--;
					if (_playInfo->_listIndex == INVALID_INDEX)
					{
						_playInfo->_listIndex = _dbInfo->_shuffleList.size() - 1;
					}

					_playInfo->_fileNumber = _dbInfo->_shuffleList[_playInfo->_listIndex];
					_playInfo->_playTime._hour = 0;
					_playInfo->_playTime._min = 0;
					_playInfo->_playTime._sec = 0;
					SendDBusFileNumberChanged(_playInfo->_fileNumber);
					IncreasePlayID();
					PlayStart(_playInfo->_fileNumber, _playInfo->_playTime);
				}
				else
				{
					fprintf(stderr, "TM Player: %s Invalid Device\n", __func__);
				}
			}
			else
			{
				TMPlayer_TrackSeekProcess(0, 0, 0);
			}
		}
#ifdef HAVE_IAP2
		else if(_device == DeviceSourceiAP2)
		{
			TMPlayeriAP2_TrackDown();
		}
#endif
#ifdef HAVE_BT
		else if(_device == DeviceSourceBluetooth)
		{
			TMPlayerBT_TrackDown();
		}
#endif
	}
}

void TMPlayer_TrackMoveProcess(int number)
{
	if ((_device >= DeviceSourceMyMusic) && (_device < TotalDeviceSources))
	{
		if (IsStorageDevice(_device))
		{
			int count;

			_playInfo->_playTime._hour = 0;
			_playInfo->_playTime._min = 0;
			_playInfo->_playTime._sec = 0;

			if (_dbInfo->_isMeta)
			{
				count = (int)_dbInfo->_metaList.size();
			}
			else
			{
				count = (int)_dbInfo->_list.size();
			}

			if (number > 0 && number <= count)
			{
				unsigned int count = _dbInfo->_shuffleList.size();
				bool found = false;
				for (unsigned int index = 0; index < count && !found; index++)
				{
					if (number == _dbInfo->_shuffleList[index])
					{
						_playInfo->_listIndex = index;
						found = true;
					}
				}

				if (found)
				{
					_playInfo->_fileNumber = number;
					SendDBusFileNumberChanged(_playInfo->_fileNumber);
				}
				else
				{
					fprintf(stderr, "TC Player: %s not found track number(%d)\n", __func__, number);
				}
			}

			IncreasePlayID();
			PlayStart(_playInfo->_fileNumber, _playInfo->_playTime);
		}
		else if (_device == DeviceSourceAUX)
		{
			//No Track Move in AUX
		}
#ifdef HAVE_IAP2
		else if (_device == DeviceSourceiAP2)
		{
			//No Track Move in iAP2
		}
#endif
#ifdef HAVE_BT
		else if (_device == DeviceSourceBluetooth)
		{
			TMPlayerBT_TrackMove(number);
		}
#endif
	}
}

void TMPlayer_TrackStopProcess(void)
{
	PlayStopDevice(_device);
}

void TMPlayer_TrackResumeProcess(void)
{
	if (IsStorageDevice(_device))
	{
		if(s_playID != 0)
		{
			TMPlayerControl_Resume(s_playID);
		}
	}
#ifdef HAVE_IAP2
	else if(_device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_PlayResume();
	}
#endif
#ifdef HAVE_BT
	else if(_device == DeviceSourceBluetooth)
	{
		TMPlayerBT_PlayResume();
	}
#endif
}

void TMPlayer_TrackPauseProcess(void)
{
	if (IsStorageDevice(_device))
	{
		if(s_playID != 0)
		{
			TMPlayerControl_Pause(s_playID);
		}
	}
#ifdef HAVE_IAP2
	else if(_device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_PlayPause();
	}
#endif
#ifdef HAVE_BT
	else if(_device == DeviceSourceBluetooth)
	{
		TMPlayerBT_PlayPause();
	}
#endif
}

void TMPlayer_TrackSeekProcess(unsigned char hour, unsigned char min, unsigned char sec)
{
	if (IsStorageDevice(_device))
	{
		TMPlayerControl_Seek(hour, min, sec, s_playID);
	}
#ifdef HAVE_IAP2
	else if(_device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_TrackSeek(hour, min, sec);
	}
#endif
#ifdef HAVE_BT
	else if(_device == DeviceSourceBluetooth)
	{
		TMPlayerBT_TrackSeek(hour, min, sec);
	}
#endif
}

void TMPlayer_TrackForwardProcess(void)
{
	if (IsStorageDevice(_device))
	{
		DEBUG_TMPLAYER_PRINTF("No Forward\n");
	}
#ifdef HAVE_IAP2
	else if(_device == DeviceSourceiAP2)
	{
		DEBUG_TMPLAYER_PRINTF("iAP2 Forward\n");
	}
#endif
#ifdef HAVE_BT
	else if(_device == DeviceSourceBluetooth)
	{
		DEBUG_TMPLAYER_PRINTF("BT Forward\n");
		TMPlayerBT_TrackForward();
	}
#endif
}

void TMPlayer_TrackRewindProcess(void)
{
	if (IsStorageDevice(_device))
	{
		DEBUG_TMPLAYER_PRINTF("No Rewind\n");
	}
#ifdef HAVE_IAP2
	else if(_device == DeviceSourceiAP2)
	{
		DEBUG_TMPLAYER_PRINTF("iAP2 Rewind\n");
	}
#endif
#ifdef HAVE_BT
	else if(_device == DeviceSourceBluetooth)
	{
		DEBUG_TMPLAYER_PRINTF("BT Rewind\n");
		TMPlayerBT_TrackRewind();
	}
#endif
}

void TMPlayer_TrackFFREWEndProcess(void)
{
	if (IsStorageDevice(_device))
	{
		DEBUG_TMPLAYER_PRINTF("No Forward/Rewind End\n");
	}
#ifdef HAVE_IAP2
	else if(_device == DeviceSourceiAP2)
	{
		DEBUG_TMPLAYER_PRINTF("iAP2 Forward/Rewind End\n");
	}
#endif
#ifdef HAVE_BT
	else if(_device == DeviceSourceBluetooth)
	{
		DEBUG_TMPLAYER_PRINTF("BT Forward/Rewind End\n");
		TMPlayerBT_TrackFFREWEnd();
	}
#endif
}

void TMPlayer_UpdateTimeProcess(unsigned char hour, unsigned char min, unsigned char sec)
{
	_playInfo->_playTime._hour = hour;
	_playInfo->_playTime._min = min;
	_playInfo->_playTime._sec = sec;
}

int TMPlayer_GetPlayID(void)
{
	return s_playID;
}

int32_t TMPlayer_GetDevice(void)
{
	return _device;
}

void TMPlayer_DisplayOn(bool play)
{
	int ret = 0;
	bool stop;

	if (!_displayResource)
	{
		DEBUG_TMPLAYER_PRINTF("Display On (%d) (device: %d)\n", _displayResource, _device);

		if (_device == TotalDeviceSources)
		{
			TMPlayer_GoLauncherHome();
			fprintf(stderr, "%s: No Device, Go to Home\n", __func__);
		}
		else
		{
			_displayResource = true;
			SendDBusDisplayOn();
			SendDBusDeviceChanged(_device, _dbInfo->_contentType);

			if ((!_audioResource) && play)
			{
				if (IsStorageDevice(_device))
				{
					if (_dbInfo->_isMeta)
					{
						SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_metaList.size());
					}
					else
					{
						SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_list.size());
					}
					SendDBusRepeatModeChanged(_playInfo->_repeatMode);
					SendDBusShuffleModeChanged(_playInfo->_shuffleMode);
					SendDBusFileNumberChanged(_playInfo->_fileNumber);
				}

				if (_dbInfo->_contentType == MultiMediaContentTypeVideo)
				{
					DEBUG_TMPLAYER_PRINTF("Video Go Go\n");
					ret = SendDBusModeManagerChangeMode(VIDEOPLAY);
				}
				else
				{
					DEBUG_TMPLAYER_PRINTF("Audio Go Go\n");
					ret = SendDBusModeManagerChangeMode(AUDIOPLAY);
				}

				if (ret < 1)
				{
					SendDBusPlayStateChanged(PlayStatusStop);
				}
			}
		}
	}
}

void TMPlayer_DisplayOff()
{
	if (_displayResource)
	{
		DEBUG_TMPLAYER_PRINTF("Display Off (%d)\n", _displayResource);

		_displayResource = false;

		if (_dbInfo->_contentType == MultiMediaContentTypeVideo)
		{
			SendDBusResetInfomation();
		}

		SendDBusDisplayOff();
	}
}

void TMPlayer_AudioOn(bool background)
{
	int ret = -1;

	DEBUG_TMPLAYER_PRINTF("Audio On (%d) device: %d content: %d\n", _audioResource, _device, _totalMediaInfo[_device]._contentType);

	if (_device != TotalDeviceSources)
	{
		if (_dbInfo->_contentType == MultiMediaContentTypeAudio)
		{
			if (IsStorageDevice(_device))
			{
				if (_dbInfo->_isMeta)
				{
					SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_metaList.size());
				}
				else
				{
					SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_list.size());
				}
				SendDBusRepeatModeChanged(_playInfo->_repeatMode);
				SendDBusShuffleModeChanged(_playInfo->_shuffleMode);
				SendDBusFileNumberChanged(_playInfo->_fileNumber);
			}

			if (!_audioResource)
			{
				_audioResource = true;
				PlayStartDevice(_device);
			}
			else if (_videoPlay == true)
			{
				PlayStartDevice(_device);
			}
		}

		if ((!_displayResource) && (!background))
		{
			TMPlayer_DisplayOn(false);
		}
	}

	_avOff = false;
	_videoPlay = false;
}

void TMPlayer_AudioOff()
{
	if (_audioResource)
	{
		DEBUG_TMPLAYER_PRINTF("Audio Off (%d)\n", _audioResource);

		_audioResource = false;

		if (IsStorageDevice(_device))
		{
			TMPlayerControl_MediaRelease(s_playID);
			s_playID = 0;
		}
		else
		{
			PlayStopDevice(_device);
			SendDBusModeManagerRscReleased(RSC_AUDIO);
		}
	}
}

void TMPlayer_VideoOn(void)
{
	int ret = -1;

	DEBUG_TMPLAYER_PRINTF("Video On %d\n", _dbInfo->_contentType);

	if (_device != TotalDeviceSources)
	{
		if (_dbInfo->_contentType == MultiMediaContentTypeVideo)
		{
			if (IsStorageDevice(_device))
			{
				if (_dbInfo->_isMeta)
				{
					SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_metaList.size());
				}
				else
				{
					SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_list.size());
				}
				SendDBusRepeatModeChanged(_playInfo->_repeatMode);
				SendDBusShuffleModeChanged(_playInfo->_shuffleMode);
				SendDBusFileNumberChanged(_playInfo->_fileNumber);

				if (!_displayResource)
				{
					TMPlayer_DisplayOn(false);
				}
			}

			if (!_audioResource)
			{
				_audioResource = true;
			}
			PlayStartDevice(_device);
		}
	}

	_avOff = false;
	_videoPlay = true;
}

void TMPlayer_RequestDisplay(void)
{
	DEBUG_TMPLAYER_PRINTF("Request to Show AV Player\n");
	if (!_displayResource)
	{
		SendDBusModeManagerChangeMode(VIEW);
	}
}

void TMPlayer_GoLauncherHome(void)
{
	DEBUG_TMPLAYER_PRINTF("Request to Go Launcher Home\n");
	SendDBusModeManagerGoHome();
	if (_dbInfo != NULL)
	{
		if (_dbInfo->_contentType == MultiMediaContentTypeVideo)
		{
			s_playID = 0;
		}
	}
}

void TMPlayer_GoLauncherBack(void)
{
	DEBUG_TMPLAYER_PRINTF("Request to Go Launcher Home\n");
	SendDBusModeManagerGoBack();
	if (_dbInfo != NULL)
	{
		if (_dbInfo->_contentType == MultiMediaContentTypeVideo)
		{
			s_playID = 0;
		}
	}
}

void TMPlayer_GoAVOnOff(void)
{
	if (_avOff)
	{
		DEBUG_TMPLAYER_PRINTF("AV On\n");
		_avOff = false;
	}
	else
	{
		if (_audioResource || _displayResource)
		{
			DEBUG_TMPLAYER_PRINTF("AV Off, Go to Idle\n");
			_avOff = true;
			ReleaseAllResource(DO_IDLE);
		}
	}
}

void TMPlayer_NotifyAuxReady(void)
{
	if(_connectedDevice[DeviceSourceAUX]._used == true)
	{
		_totalMediaInfo[DeviceSourceAUX]._haveContent = true;
		_totalMediaInfo[DeviceSourceAUX]._contentType = MultiMediaContentTypeAudio;
		SendDBusDeviceReady(DeviceSourceAUX);
		if (_device == TotalDeviceSources)
		{
			_device = DeviceSourceAUX;
			_dbInfo = &_totalMediaInfo[_device]._db[_totalMediaInfo[_device]._contentType];
			_playInfo = &_dbInfo->_playInfo;
			SendDBusDeviceChanged(DeviceSourceAUX,MultiMediaContentTypeAudio);
		}
	}
}

void TMPlayer_Suspend(void)
{
	if (_displayResource)
	{
		ReleaseAllResource(DO_SUSPEND);
	}
	AuxPlayDeinit();
}

void TMPlayer_Resume(void)
{
	AuxPlayInit(DeviceConnectProcess, DeviceDisconnectProcess);
	AuxDetectEnable();
}

#ifdef HAVE_IAP2
void TMPlayer_NotifyiAP2Ready(void)
{
	if(_connectedDevice[DeviceSourceiAP2]._used == true)
	{
		_totalMediaInfo[DeviceSourceiAP2]._fileDBReady = false;
		_totalMediaInfo[DeviceSourceiAP2]._metaDBReady = true;
		_totalMediaInfo[DeviceSourceiAP2]._haveContent = true;
		_totalMediaInfo[DeviceSourceiAP2]._contentType = MultiMediaContentTypeAudio;
		SendDBusDeviceReady(DeviceSourceiAP2);
		if (_device == TotalDeviceSources)
		{
			_device = DeviceSourceiAP2;
			_dbInfo = &_totalMediaInfo[_device]._db[_totalMediaInfo[_device]._contentType];
			_playInfo = &_dbInfo->_playInfo;
			SendDBusDeviceChanged(DeviceSourceiAP2,MultiMediaContentTypeAudio);
		}
	}
}
#endif
#ifdef HAVE_BT
void TMPlayer_NotifyBTReady(void)
{
	if(_connectedDevice[DeviceSourceBluetooth]._used == true)
	{
	//	_totalMediaInfo[DeviceSourceBluetooth]._fileDBReady = false;
	//	_totalMediaInfo[DeviceSourceBluetooth]._metaDBReady = true;
		_totalMediaInfo[DeviceSourceBluetooth]._haveContent = true;
		_totalMediaInfo[DeviceSourceBluetooth]._contentType = MultiMediaContentTypeAudio;
		SendDBusDeviceReady(DeviceSourceBluetooth);
		if (_device == TotalDeviceSources)
		{
			_device = DeviceSourceBluetooth;
			_dbInfo = &_totalMediaInfo[_device]._db[_totalMediaInfo[_device]._contentType];
			_playInfo = &_dbInfo->_playInfo;
			SendDBusDeviceChanged(DeviceSourceBluetooth,MultiMediaContentTypeAudio);
		}
	}
}
#endif

void TMPlayer_DetectEnable(void)
{
	int device;

	if (s_detect == false)
	{
		s_detect = true;
	}

	for (device = 0; device < TotalDeviceSources; device++)
	{
		(void)pthread_mutex_lock(&connectMutex);
		if (_connectedDevice[device]._used)
		{
			SendDBusDeviceConnect(device);
		}
		(void)pthread_mutex_unlock(&connectMutex);

		if (GetConnectedDevice(_connectedDevice[device]._key) == device)
		{
			if (IsStorageDevice((DeviceSource)device))
			{
				if(TMPlayerDB_AddDataBase((DeviceSource)device, _totalMediaInfo[device]._mntPath)==false)
				{
					DEBUG_TMPLAYER_PRINTF("MOUNT DEVICE(%s) FAILED\n", _connectedDevice[device]._key);
					SendDBusDeviceMountStop((DeviceSource)device);
				}
			}
			else if (device == DeviceSourceAUX)
			{
				TMPlayer_NotifyAuxReady();
			}
#ifdef HAVE_IAP2
			else if(device == DeviceSourceiAP2)
			{
				TMPlayer_NotifyiAP2Ready();
			}
#endif
#ifdef HAVE_BT
			else if(device == DeviceSourceBluetooth)
			{
				TMPlayer_NotifyBTReady();
			}
#endif
		}
	}

	AuxDetectEnable();
}

int32_t TMPlayer_RequestChangeDevice(int device, bool play)
{
	int ret = 1;

	if(!_displayResource && !_audioResource)
	{
		ret = SendDBusModeManagerChangeMode(AUDIOPLAY);
	}

	if(_device != device)
	{
		if(ret)
		{
			TMPlayer_ChangeDeviceProcess(device, play);
		}
		else
		{
			TMPlayer_ChangeDeviceProcess(device, false);
		}
	}
	return ret;
}

void TMPlayer_SetPlayStatus(int32_t status)
{
	if (IsStorageDevice(_device))
	{
		if ((status == PlayStatusPlaying) || (status == PlayStatusPause))
		{
			_playInfo->_playStatus = (TCPlayStatus)status;
		}

		DEBUG_TMPLAYER_PRINTF("device: %d, status: %d\n", _device, status);
		SendDBusPlayStateChanged((TCPlayStatus)status);
	}
}

#ifdef HAVE_IAP2
void TMPlayer_SetPlayStatus_iAP2(int32_t status)
{
	if ((status == PlayStatusPlaying) || (status == PlayStatusPause))
	{
		_totalMediaInfo[DeviceSourceiAP2]._db[MultiMediaContentTypeAudio]._playInfo._playStatus = (TCPlayStatus)status;
	}

	if (_device == DeviceSourceiAP2)
	{
		SendDBusPlayStateChanged((TCPlayStatus)status);
	}

	DEBUG_TMPLAYER_PRINTF("device: %d, status: %d\n", DeviceSourceiAP2, status);
}
#endif

#ifdef HAVE_BT
void TMPlayer_SetPlayStatus_BT(int32_t status)
{
	if ((status == PlayStatusPlaying) || (status == PlayStatusPause))
	{
		_totalMediaInfo[DeviceSourceBluetooth]._db[MultiMediaContentTypeAudio]._playInfo._playStatus = (TCPlayStatus)status;
	}

	if (_device == DeviceSourceBluetooth)
	{
		SendDBusPlayStateChanged((TCPlayStatus)status);
	}

	DEBUG_TMPLAYER_PRINTF("device: %d, status: %d\n", DeviceSourceBluetooth, status);
}
#endif

static DeviceSource GetConnectedDevice(char *key)
{
	DeviceSource device = TotalDeviceSources;

	if (key != NULL)
	{
		int index;

		for (index = 0; index < TotalDeviceSources && device == TotalDeviceSources; index++)
		{
			if (_connectedDevice[index]._used &&
				strncmp(key, _connectedDevice[index]._key, KEY_STRING_SIZE) == 0)
			{
				device = (DeviceSource)index;
			}
		}
	}
	else
	{
		fprintf(stderr, "%s: key is null\n", __func__);
	}

	return device;
}

static DeviceSource DeleteConnectedDevice(char *key)
{
	DeviceSource device = TotalDeviceSources;

	if (key != NULL)
	{
		int index;

		for (index = 0; index < TotalDeviceSources && device == TotalDeviceSources; index++)
		{
			if (_connectedDevice[index]._used &&
				strncmp(key, _connectedDevice[index]._key, KEY_STRING_SIZE) == 0)
			{
				device = (DeviceSource)index;
				memset(_connectedDevice[index]._key, 0, KEY_STRING_SIZE);
			}
		}
	}
	else
	{
		fprintf(stderr, "%s: key is null\n", __func__);
	}

	return device;
}

static void FileDBResult(unsigned int dbNum, int result)
{
	DataBaseInfo *db;
	bool ret;

	DEBUG_TMPLAYER_PRINTF("File Gathering End: device(%d), result (%d)\n", dbNum, result);

	if(!result)
	{
		DEBUG_TMPLAYER_PRINTF("_totalMediaInfo[dbNum]._fileDBReady (%d))\n", _totalMediaInfo[dbNum]._fileDBReady);

		/* Create New DB */
		if(_totalMediaInfo[dbNum]._fileDBReady == false)
		{
			ret = TMPlayerDB_CreatePlayList((DeviceSource)dbNum, MultiMediaContentTypeAudio, false, false, INVALID_INDEX, &_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio]);
			db = &_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio];
			db->_shuffle = false;
			if (db->_list.size() > 0)
			{
				db->_playInfo._fileNumber = 1;
				db->_playInfo._listIndex = 0;
			}
			db->_playInfo._repeatMode = RepeatModeAll;
			db->_playInfo._shuffleMode = ShuffleModeOff;
			db->_playInfo._playTime._hour = 0;
			db->_playInfo._playTime._min = 0;
			db->_playInfo._playTime._sec = 0;
		}
		else if(_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio]._totalNumber != TMPlayerDB_GetTotalFileCount(dbNum, MultiMediaContentTypeAudio))
		{

			db = &_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio];

			ret = TMPlayerDB_CreatePlayList((DeviceSource)dbNum, db->_contentType, false, false, INVALID_INDEX, db);
			if (ret)
			{
				UpdateFileNumber((DeviceSource)dbNum, db->_contentType,db->_totalNumber);
			}
			else
			{
				SendDBusDeviceMountStop((DeviceSource)dbNum);
			}
		}
		if (_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio]._isMeta == false)
		{
			SendDBusFileDBCompleted((DeviceSource)dbNum);
		}

		/* Create Video List */
		for (int i = MultiMediaContentTypeVideo; i < TotalMultiMediaContentTypes; i++)
		{
			ret = TMPlayerDB_CreatePlayList((DeviceSource)dbNum, (MultiMediaContentType)i, false, false, INVALID_INDEX, &_totalMediaInfo[dbNum]._db[i]);
			if(ret == false)
			{
				break;
			}

			db = &_totalMediaInfo[dbNum]._db[i];
			db->_shuffle = false;
			db->_isMeta = false;
			if (db->_list.size() > 0)
			{
				db->_playInfo._fileNumber = 1;
				db->_playInfo._listIndex = 0;
			}
			db->_playInfo._repeatMode = RepeatModeAll;
			db->_playInfo._shuffleMode = ShuffleModeOff;
			db->_playInfo._playTime._hour = 0;
			db->_playInfo._playTime._min = 0;
			db->_playInfo._playTime._sec = 0;
		}
		if(ret == true)
		{
			bool haveContent = false;

			for (int i = MultiMediaContentTypeAudio; i < TotalMultiMediaContentTypes && !haveContent; i++)
			{
				if (HaveDeviceContent((DeviceSource)dbNum, (MultiMediaContentType)i))
				{
					haveContent = true;
					_totalMediaInfo[dbNum]._contentType = (MultiMediaContentType)i;
				}
			}

			_totalMediaInfo[dbNum]._haveContent = haveContent;
			if (haveContent)
			{
				DEBUG_TMPLAYER_PRINTF("MOUNT DEVICE(%s) SUCCEDDED\n", _totalMediaInfo[dbNum]._mntPath);
				// Only have video file
				if(_totalMediaInfo[dbNum]._fileDBReady == false)
				{
					/*have not gui hmi*/
					/*_device = (DeviceSource)dbNum;*/
					if(_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio]._isMeta == false)
					{
						SendDBusDeviceReady((DeviceSource)dbNum);

						if (_device == TotalDeviceSources)
						{
							_device = (DeviceSource)dbNum;
							_dbInfo = &_totalMediaInfo[_device]._db[_totalMediaInfo[_device]._contentType];
							_playInfo = &_dbInfo->_playInfo;
							SendDBusDeviceChanged(_device, _dbInfo->_contentType);
						}
					}
				}
			}
			else
			{
				if (IsKeepMount(_totalMediaInfo[dbNum]._mntPath))
				{
					SendDBusNoSong((DeviceSource)dbNum);
				}
				else
				{
					fprintf(stderr, "%s: device not mounted\n", __func__);
				}

				_totalMediaInfo[dbNum]._contentType = TotalMultiMediaContentTypes;
			}
			_totalMediaInfo[dbNum]._fileDBReady = true;
		}
		else
		{
			DEBUG_TMPLAYER_PRINTF("MOUNT DEVICE(%s) FAILED\n", _totalMediaInfo[dbNum]._mntPath);
			SendDBusDeviceMountStop((DeviceSource)dbNum);
		}

		if(ret && _totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio]._isMeta)
		{
			(void)TMPlayerDB_CreateMetaDB((DeviceSource)dbNum);
		}
	}
	else
	{
		//senddbus filedberror
		fprintf(stderr, "%s: File DB Create Error, Device[%d], result [%d] \n", __func__, dbNum, result);
	}
}

static void FileDBUpdate(unsigned int dbNum, ContentsType type)
{
	DataBaseInfo *db;
	MultiMediaContentType mediaType;
	bool ret;
	mediaType = ConversionTablebyMediaType(type);

	DEBUG_TMPLAYER_PRINTF("File update : device(%d), Contents Type (%d), totalCount (%d)\n",
		dbNum, type, TMPlayerDB_GetTotalFileCount(dbNum, mediaType));

	if(type == AudioContents)
	{

		if(_totalMediaInfo[dbNum]._fileDBReady == false)
		{
			ret = TMPlayerDB_CreatePlayList((DeviceSource)dbNum, (MultiMediaContentType)mediaType, false, false, INVALID_INDEX, &_totalMediaInfo[dbNum]._db[mediaType]);
			db = &_totalMediaInfo[dbNum]._db[mediaType];
			db->_shuffle = false;
			if (db->_list.size() > 0)
			{
				db->_playInfo._fileNumber = 1;
				db->_playInfo._listIndex = 0;
			}
			db->_playInfo._repeatMode = RepeatModeAll;
			db->_playInfo._shuffleMode = ShuffleModeOff;
			db->_playInfo._playTime._hour = 0;
			db->_playInfo._playTime._min = 0;
			db->_playInfo._playTime._sec = 0;

			if(ret == true)
			{
				bool haveContent = false;

				if (HaveDeviceContent((DeviceSource)dbNum, (MultiMediaContentType)mediaType))
				{
					haveContent = true;
					_totalMediaInfo[dbNum]._contentType = (MultiMediaContentType)mediaType;
				}

				_totalMediaInfo[dbNum]._haveContent = haveContent;
				if (haveContent)
				{
					if(_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio]._isMeta == false)
					{
						SendDBusDeviceReady((DeviceSource)dbNum);
					}
				}
				else
				{
					if (IsKeepMount(_totalMediaInfo[dbNum]._mntPath))
					{
						SendDBusNoSong((DeviceSource)dbNum);
					}
					else
					{
						fprintf(stderr, "%s: device not mounted\n", __func__);
					}

					_totalMediaInfo[dbNum]._contentType = TotalMultiMediaContentTypes;
				}
			}

			_totalMediaInfo[dbNum]._fileDBReady = true;
		}
		else
		{
			db = &_totalMediaInfo[dbNum]._db[mediaType];

			ret = TMPlayerDB_CreatePlayList((DeviceSource)dbNum, db->_contentType, false, false, INVALID_INDEX, db);
			if (ret)
			{
				UpdateFileNumber((DeviceSource)dbNum, db->_contentType,db->_totalNumber);
			}
			else
			{
				SendDBusDeviceMountStop((DeviceSource)dbNum);
			}
		}
	}
}

static void MetaDBResult(unsigned int dbNum, int result)
{
	DataBaseInfo *db;
	bool ret;

	DEBUG_TMPLAYER_PRINTF("Meta Gathering End: device(%d), result (%d)\n", dbNum, result);
	DEBUG_TMPLAYER_PRINTF("_totalMediaInfo[dbNum]._metaDBReady (%d))\n", _totalMediaInfo[dbNum]._metaDBReady );

	if(!result)
	{
		TMPlayerDB_InitMetaPlayList(dbNum);

		/* Craete New Meta DB */
		if(_totalMediaInfo[dbNum]._metaDBReady == false)
		{
			ret = TMPlayerDB_CreatePlayList((DeviceSource)dbNum, MultiMediaContentTypeAudio, true, false, INVALID_INDEX, &_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio]);
			db = &_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio];
			if(db->_list.size() > 0)
			{
				db->_playInfo._fileNumber = 1;
				db->_playInfo._listIndex = 0;
			}
			db->_playInfo._repeatMode = RepeatModeAll;
			db->_playInfo._shuffleMode = ShuffleModeOff;
			db->_playInfo._playTime._hour = 0;
			db->_playInfo._playTime._min = 0;
			db->_playInfo._playTime._sec = 0;
		}
		SendDBusFileDBCompleted((DeviceSource)dbNum);

		if(ret == true)
		{
			bool haveContent = false;

			if (HaveDeviceContent((DeviceSource)dbNum, MultiMediaContentTypeAudio))
			{
				haveContent = true;
				_totalMediaInfo[dbNum]._contentType = MultiMediaContentTypeAudio;
			}

			_totalMediaInfo[dbNum]._haveContent = haveContent;
			if (haveContent)
			{
				DEBUG_TMPLAYER_PRINTF("MOUNT DEVICE(%s) IS READY TO PLAY\n", _totalMediaInfo[dbNum]._mntPath);
				if(_totalMediaInfo[dbNum]._metaDBReady == false)
				{
					SendDBusDeviceReady((DeviceSource)dbNum);

					if (_device == TotalDeviceSources)
					{
						_dbInfo = &_totalMediaInfo[_device]._db[_totalMediaInfo[_device]._contentType];
						_playInfo = &_dbInfo->_playInfo;
						_device = (DeviceSource)dbNum;
						SendDBusDeviceChanged(_device, _dbInfo->_contentType);
					}
				}
			}
			else
			{
				if (IsKeepMount(_totalMediaInfo[dbNum]._mntPath))
				{
					SendDBusNoSong((DeviceSource)dbNum);
				}
				else
				{
					fprintf(stderr, "%s: device not mounted\n", __func__);
				}

				_totalMediaInfo[dbNum]._contentType = TotalMultiMediaContentTypes;
			}
			_totalMediaInfo[dbNum]._metaDBReady = true;
		}
		else
		{
			DEBUG_TMPLAYER_PRINTF("MOUNT DEVICE(%s) FAILED\n", _totalMediaInfo[dbNum]._mntPath);
			SendDBusDeviceMountStop((DeviceSource)dbNum);
		}
	}
	else
	{
		fprintf(stderr, "%d: Meta DB Create Error, Device[%d], result [%d] \n", __func__, dbNum, result);
	}
}

static void MetaDBUpdate(unsigned int dbNum)
{
}

void MetaCategoryMenuName(unsigned int dbNum, unsigned char mode, const char * menu)
{
	SendDBusMetaMenuChanged(dbNum, mode, menu);
}

void MetaCategoryIndexChange(unsigned int dbNum, unsigned int totalNum, unsigned int currentNum)
{
	SendDBusMetaCountChagned( dbNum, totalNum);
}

void MetaCategoryInfo(unsigned int dbNum, unsigned short index, const char * name, unsigned int type)
{
	SendDBusMetaAddInfo(dbNum, (int)index + 1, name, type);
}

void MetaSelectTrack(unsigned int dbNum, unsigned int selectIndex)
{
	DataBaseInfo *db;
	int count = 0;

	DEBUG_TMPLAYER_PRINTF("DEVICE(%d), INDEX(%d)\n",
						   dbNum, selectIndex);

	db = &_totalMediaInfo[dbNum]._db[MultiMediaContentTypeAudio];

	if (TMPlayerDB_CreatePlayList((DeviceSource)dbNum, MultiMediaContentTypeAudio, db->_isMeta, false, INVALID_INDEX, db))
	{
		SendDBusResetInfomation();
		SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_totalNumber);
		RefreshFileNumber(_device, _dbInfo->_contentType, true);
	}
	else
	{
		fprintf(stderr, "%s: CreatePlayList Fail\n", __func__);
	}

	db->_playInfo._playTime._hour = 0;
	db->_playInfo._playTime._min = 0;
	db->_playInfo._playTime._sec = 0;

	count = (int)db->_shuffleList.size();
	if (count > 0)
	{
		bool found = false;
		selectIndex++;
		for (int index = 0; index < count && !found; index++)
		{
			if ((int)selectIndex == _dbInfo->_shuffleList[index])
			{
				DEBUG_TMPLAYER_PRINTF("CHANGE PLAY LIST INDEX(%d)\n",
									   db->_playInfo._listIndex);
				db->_playInfo._listIndex = index;
				found = true;
			}
		}

		if (!found)
		{
			fprintf(stderr, "%s: selected file number(%d) not matched\n", __func__, selectIndex);
			db->_playInfo._listIndex = INVALID_INDEX;
		}
	}
	SendDBusMetaCreateTrackList(dbNum,count);
}

static void UpdateFileNumber(DeviceSource device, MultiMediaContentType content, unsigned int totalNumber)
{
	unsigned int index = 0;
	unsigned int count;
	bool found = false;
	bool toPlay = false;
	DataBaseInfo *db;

	if (device < TotalDeviceSources)
	{
		db = &_totalMediaInfo[device]._db[content];

		if (db->_isMeta)
		{
			count = db->_metaList.size();
			for (index = 0; index < count && !found; index++)
			{
				if (db->_playInfo._dbIndex == db->_metaList[index])
				{
					db->_playInfo._fileNumber = index + 1;
					found = true;
				}
			}
		}
		else
		{
			count = db->_list.size();
			for (index = 0; index < count && !found; index++)
			{
				if (db->_playInfo._dbIndex == db->_list[index])
				{
					db->_playInfo._fileNumber = index + 1;
					found = true;
				}
			}
		}
			
		if (!found)
		{
			db->_playInfo._fileNumber = 1;
			db->_playInfo._playTime._hour = 0;
			db->_playInfo._playTime._min = 0;
			db->_playInfo._playTime._sec = 0;
			toPlay = true;
		}
		count = db->_shuffleList.size();
		found = false;
		for (index = 0; index < count && !found; index++)
		{
			if (db->_playInfo._fileNumber == (int)db->_shuffleList[index])
			{
				db->_playInfo._listIndex = index;
				found = true;
			}
		}

		if (_device == device)
		{
			SendDBusFileNumberUpdated(device, MultiMediaContentTypeAudio,_playInfo->_fileNumber,(int)totalNumber);

			if (toPlay)
			{
				IncreasePlayID();
				PlayStart(_playInfo->_fileNumber, _playInfo->_playTime);
			}
		}
		if (!found)
		{
			fprintf(stderr, "%s: can not found file number(%d)\n",
					__func__, _playInfo->_fileNumber);
		}
	}
}

static bool HaveDeviceContent(DeviceSource device, MultiMediaContentType content)
{
	bool have = false;
	if (device < TotalDeviceSources)
	{	
		have = (_totalMediaInfo[device]._db[content]._totalNumber > 0);	
	}

	return have;
}

static bool IsKeepMount(const char *path)
{
	bool keep = false;
	FILE *pipe;

	pipe = popen("mount", "r");
	if (pipe != NULL)
	{
		char buffer[256];
		char *bufp;
		while (!feof(pipe) && !keep)
		{
			if (fgets(buffer, 256, pipe) != NULL)
			{
				bufp = strstr(buffer, path);
				if (bufp != NULL)
				{
					keep = true;
				}
			}
		}
		pclose(pipe);
	}
	else
	{
		fprintf(stderr, "%s: popen failed\n", __func__);
	}

	return keep;
}

static MultiMediaContentType ConversionTablebyMediaType(ContentsType type)	//convert MultiMediaContentType to ContentsType
{
	MultiMediaContentType mediaType;

	switch(type)
	{
		case AudioContents:
			mediaType = MultiMediaContentTypeAudio;
			break;
		case VideoContents:
			mediaType = MultiMediaContentTypeVideo;
			break;
		default:
			mediaType = MultiMediaContentTypeAudio;
			break;
	}
	return mediaType;
}

static int ChangeContent(MultiMediaContentType content)
{
	int mode, ret = -1;
	bool exist;

	DEBUG_TMPLAYER_PRINTF("\n");

	PlayStopDevice(_device);

	exist = HaveDeviceContent(_device, content);

	if ((_totalMediaInfo[_device]._contentType != content) && exist)
	{
		DEBUG_TMPLAYER_PRINTF("%d %d\n", _totalMediaInfo[_device]._contentType, content);
		if (content < TotalMultiMediaContentTypes)
		{
			_totalMediaInfo[_device]._contentType = content;
			_dbInfo = &_totalMediaInfo[_device]._db[content];
			_playInfo = &_dbInfo->_playInfo;

			SendDBusContentChanged(_dbInfo->_contentType);

			if (_dbInfo->_isMeta)
			{
				SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_metaList.size());
			}
			else
			{
				SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_list.size());
			}
			SendDBusRepeatModeChanged(_playInfo->_repeatMode);
			SendDBusShuffleModeChanged(_playInfo->_shuffleMode);
			SendDBusFileNumberChanged(_playInfo->_fileNumber);
			ret = 1;
		}
	}

	return ret;
}

static bool ChangeDevice(DeviceSource device, bool play)
{
	int ret, contentType = (int)_totalMediaInfo[_device]._contentType;
	bool changed = false;
	DEBUG_TMPLAYER_PRINTF("device(%d), play(%d), pre device(%d), s_playID(%d)\n", device, play, _device, s_playID);

	if (device < TotalDeviceSources)
	{
		DEBUG_TMPLAYER_PRINTF("_totalMediaInfo[%d]._haveContent(%d)\n",device, _totalMediaInfo[device]._haveContent);

		if (_totalMediaInfo[device]._haveContent)
		{
			PlayStopDevice(_device);

			_device = device;
			_dbInfo = &_totalMediaInfo[_device]._db[_totalMediaInfo[_device]._contentType];
			_playInfo = &_dbInfo->_playInfo;
			SendDBusDeviceChanged(_device, _dbInfo->_contentType);

			if (contentType == _totalMediaInfo[_device]._contentType) // no need to change content type
			{

				if (IsStorageDevice(_device))
				{
					if (_dbInfo->_isMeta)
					{
						SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_metaList.size());
					}
					else
					{
						SendDBusTotalNumberChanged(_device, _dbInfo->_contentType, _dbInfo->_list.size());
					}
					SendDBusRepeatModeChanged(_playInfo->_repeatMode);
					SendDBusShuffleModeChanged(_playInfo->_shuffleMode);
					SendDBusFileNumberChanged(_playInfo->_fileNumber);
				}

				if (_audioResource || play)
				{
					PlayStartDevice(_device);
				}
			}
			else // need content change
			{
				if (_totalMediaInfo[_device]._contentType == MultiMediaContentTypeVideo)
				{
					if (_displayResource)
					{
						DEBUG_TMPLAYER_PRINTF("Next ContentType is Video, Change Mode\n");
						SendDBusModeManagerChangeMode(VIDEOPLAY);
					}
					else
					{
						ReleaseAllResource(DO_IDLE);
					}
				}
				else
				{
					if (_displayResource && _audioResource)
					{
						DEBUG_TMPLAYER_PRINTF("Next ContentType is Audio, Change Mode\n");
						SendDBusModeManagerChangeMode(AUDIOPLAY);
					}
					else if (_audioResource || play)
					{
						DEBUG_TMPLAYER_PRINTF("Next ContentType is Audio (BG), Change Mode\n");
						SendDBusModeManagerChangeMode(AUDIOBGPLAY);
					}
				}
			}

			changed = true;
		}
		else
		{
			fprintf(stderr, "%s: no available contents\n", __func__);
		}
	}

	return changed;
}

static bool PlayStart(int number, PlayTime playTime)
{
	bool start = false;
	if(_initStart == -1)
	{
		SendDBusMediaGetAlbumArtKey();
		_initStart = 1;
	}
	DEBUG_TMPLAYER_PRINTF("NUMBER(%d), HOUR(%u), MINUTE(%u), SECOND(%u)\n",
							 number, playTime._hour, playTime._min, playTime._sec);

	std::string uri, path, fileName, folderName;
	int index = number - 1;
	unsigned int dbIndex = INVALID_INDEX;

	if ((_audioResource && (_dbInfo->_contentType == MultiMediaContentTypeAudio)) ||
			(_displayResource && (_dbInfo->_contentType == MultiMediaContentTypeVideo)))
	{
		if (_dbInfo->_isMeta)
		{
			if (index < (int)_totalMediaInfo[_device]._db[ _dbInfo->_contentType]._metaList.size())
			{
				dbIndex = _totalMediaInfo[_device]._db[ _dbInfo->_contentType]._metaList[index];
			}
		}
		else
		{
			if (index < (int)_totalMediaInfo[_device]._db[ _dbInfo->_contentType]._list.size())
			{
				dbIndex = _totalMediaInfo[_device]._db[ _dbInfo->_contentType]._list[index];
			}
		}

		if (TMPlayerDB_GetFilePath(_device, _dbInfo->_contentType, dbIndex, path))
		{
			_playInfo->_dbIndex = dbIndex;
			if (GetFolderName(folderName, path))
			{
				if (GetFileName(fileName, path))
				{
					DEBUG_TMPLAYER_PRINTF("FOLDER NAME(%s), FILE NAME(%s)\n",
											 folderName.data(),
											 fileName.data());
					SendDBusFileNameChanged(fileName.c_str(), fileName.size());
					SendDBusFolderNameChanged(folderName.c_str(), folderName.size());
					uri.append("file://");
					uri.append(path.data());
					if (_playInfo->_playStatus == PlayStatusPause)
					{
						TMPlayerControl_Play(_dbInfo->_contentType, uri.data(), playTime._hour, playTime._min, playTime._sec, s_playID, 1);
					}
					else
					{
						TMPlayerControl_Play(_dbInfo->_contentType, uri.data(), playTime._hour, playTime._min, playTime._sec, s_playID, 0);
					}
					start = true;
				}
				else
				{
					fprintf(stderr, "%s: GetFolderName failed\n", __func__);
				}
			}
			else
			{
				fprintf(stderr, "%s: GetFolderName failed\n", __func__);
			}
		}
		else
		{
			fprintf(stderr, "%s: GetFilePath failed\n", __func__);
		}
	}
	else
	{
		if (_dbInfo->_contentType == MultiMediaContentTypeVideo)
		{
			ReleaseAllResource(DO_IDLE);
		}
		else if (_dbInfo->_contentType == MultiMediaContentTypeAudio)
		{
			DEBUG_TMPLAYER_PRINTF("Audio Go Go\n");

			SendDBusModeManagerChangeMode(AUDIOBGPLAY);
		}
	}

	return start;
}

static bool GetFileName(std::string &fileName, std::string &filePath)
{
	bool found = false;
	size_t pos = filePath.rfind('/');
	if (pos != std::string::npos)
	{
		fileName = filePath.substr(pos + 1);
		found = true;
	}
	else
	{
		fprintf(stderr, "%s: can not find file name\n", __func__);
	}

	return found;
}

static bool GetFolderName(std::string &folderName, std::string &filePath)
{
	bool found = false;
	size_t pos = filePath.rfind('/');
	if (pos != std::string::npos)
	{
		char *bufp;
		size_t length;
		length = filePath.copy(_tempFilePath, pos);
		_tempFilePath[length] = '\0';

		bufp = strrchr(_tempFilePath, '/');
		if (bufp != NULL)
		{
			bufp++;
			folderName = (const char *)bufp;
		}
		else
		{
			folderName = (const char *)_tempFilePath;
		}
		found = true;
	}
	else
	{
		fprintf(stderr, "%s: can not find file name\n", __func__);
	}

	return found;
}

static void RefreshFileNumber(DeviceSource device, MultiMediaContentType content, bool isclearList)
{
	unsigned int index = 0;
	unsigned int count;
	bool found = false;
	bool toPlay = false;
	DataBaseInfo *db;

	if (device < TotalDeviceSources)
	{
		db = &_totalMediaInfo[device]._db[content];

		if (db->_isMeta)
		{
			count = db->_metaList.size();
			for (index = 0; index < count && !found; index++)
			{
				if (db->_playInfo._dbIndex == db->_metaList[index])
				{
					db->_playInfo._fileNumber = index + 1;
					found = true;
				}
			}

		}
		else
		{
			count = db->_list.size();
			for (index = 0; index < count && !found; index++)
			{
				if (db->_playInfo._dbIndex == db->_list[index])
				{
					db->_playInfo._fileNumber = index + 1;
					found = true;
				}
			}
		}

		if (!found && !isclearList)
		{
			db->_playInfo._fileNumber = 1;
			db->_playInfo._playTime._hour = 0;
			db->_playInfo._playTime._min = 0;
			db->_playInfo._playTime._sec = 0;
			toPlay = true;
		}

		count = db->_shuffleList.size();
		found = false;
		for (index = 0; index < count && !found; index++)
		{
			if (db->_playInfo._fileNumber == (int)db->_shuffleList[index])
			{
				db->_playInfo._listIndex = index;
				found = true;
			}
		}

		if (_device == device)
		{
			SendDBusFileNumberChanged(_playInfo->_fileNumber);

			if (toPlay)
			{
				IncreasePlayID();
				PlayStart(_playInfo->_fileNumber, _playInfo->_playTime);
			}
		}

		if (!found)
		{
			fprintf(stderr, "%s: can not found file number(%d)\n",
					__func__, _playInfo->_fileNumber);
		}
	}
}

static void IncreasePlayID(void)
{
	int iMax = std::numeric_limits<int>::max();

	if (s_playIDCount == iMax)
	{
		s_playIDCount = 0;
	}

	s_playIDCount++;

	s_playID = s_playIDCount;

	DEBUG_TMPLAYER_PRINTF("Play ID is %d\n", s_playID);
}


static void PlayStartDevice(DeviceSource device)
{
	if (IsStorageDevice(device))
	{
		IncreasePlayID();
		PlayStart(_playInfo->_fileNumber, _playInfo->_playTime);
	}
	else
	{
		if (!_audioResource)
		{
			DEBUG_TMPLAYER_PRINTF("%d device requests background audio play\n", _device);
			SendDBusModeManagerChangeMode(AUDIOBGPLAY);
		}
		else
		{
			if (device == DeviceSourceAUX)
			{
				AuxPlayOn();
			}
#ifdef HAVE_IAP2
			else if (device == DeviceSourceiAP2)
			{
				if (_totalMediaInfo[DeviceSourceiAP2]._db[MultiMediaContentTypeAudio]._playInfo._playStatus == PlayStatusPause)
				{
					TMPlayeriAP2_RefreshInfomation();
					TMPlayeriAP2_PlayPause();
				}
				else
				{
					TMPlayeriAP2_PlayRestart();
				}
			}
#endif
#ifdef HAVE_BT
			else if (device == DeviceSourceBluetooth)
			{
				if (_totalMediaInfo[DeviceSourceBluetooth]._db[MultiMediaContentTypeAudio]._playInfo._playStatus == PlayStatusPause)
				{
					TMPlayerBT_RefreshInfomation();
					TMPlayerBT_PlayPause();
				}
				else
				{
					TMPlayerBT_PlayRestart();
				}
			}
#endif
		}
	}
}

static void PlayStopDevice(DeviceSource device)
{
	if(device == DeviceSourceAUX)
	{
		AuxPlayOff();
	}
#ifdef HAVE_IAP2
	else if(device == DeviceSourceiAP2)
	{
		TMPlayeriAP2_PlayStop();
	}
#endif
#ifdef HAVE_BT
	else if(device == DeviceSourceBluetooth)
	{
		TMPlayerBT_PlayStop();
	}
#endif
	else
	{
		if (s_playID != 0)
		{
			TMPlayerControl_Stop(s_playID);
			s_playID = 0;
		}
	}
}

static int IsStorageDevice (DeviceSource device)
{
	int ret = 0;

	if ((device >= DeviceSourceUSB1) && (device <= DeviceSourceSDMMC))
	{
		ret = 1;
	}

	return ret;
}
