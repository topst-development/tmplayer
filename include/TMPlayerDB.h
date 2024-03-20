/****************************************************************************************
 *   FileName    : TMPlayerDB.h
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
#ifndef _TMPLAYER_DB_H_
#define _TMPLAYER_DB_H_

#ifdef __cplusplus
extern "C" {
#endif	

typedef struct _PlayInfo
{
	TCPlayStatus _playStatus;
	RepeatMode _repeatMode;
	ShuffleMode _shuffleMode;
	PlayTime _playTime;
	int _fileNumber;
	unsigned int _listIndex;
	unsigned int _dbIndex;
} PlayInfo;

typedef struct _DataBaseInfo
{
	DeviceSource _device;
	MultiMediaContentType	_contentType;
	bool _shuffle;
	bool _isMeta;
	//unsigned int _folderCount; current not used
	//unsigned int _fileCount; current not use
	unsigned int _totalNumber;
	PlayInfo _playInfo;
	std::vector<unsigned int> _list;
	std::vector<unsigned int> _metaList;
	std::vector<int> _shuffleList;
} DataBaseInfo;

typedef void (*TMPlayerDBFileDBResult_cb)(unsigned int dbNum, int result);
typedef void (*TMPlayerDBFileDBUpdate_cb)(unsigned int dbNum, ContentsType type);
typedef void (*TMPlayerDBMetaDBResult_cb)(unsigned int dbNum, int result);
typedef void (*TMPlayerDBMetaDBUpdate_cb)(unsigned int dbNum);

typedef struct _TMPlayerDB_EventCB{
	TMPlayerDBFileDBResult_cb			_TMPlayerDBFileDBResult;
	TMPlayerDBFileDBUpdate_cb			_TMPlayerDBFileDBUpdate;
	TMPlayerDBMetaDBResult_cb		_TMPlayerDBMetaDBResult;
	TMPlayerDBMetaDBUpdate_cb		_TMPlayerDBMetaDBUpdate;
}TMPlayerDB_EventCB;

int TMPlayerDB_Initialize(const char* DBPath);
void TMPlayerDB_Release(void);
void TMPlayerDB_Debug(int state);
void TMPlayerDB_SetEventCallback(TMPlayerDB_EventCB *cb);
int TMPlayerDB_AddDataBase(DeviceSource device, const char *mountPath);
int TMPlayerDB_CreateMetaDB(unsigned int dbNum);
void TMPlayerDB_DeleteDateBase(DeviceSource device);
bool TMPlayerDB_CreatePlayList(DeviceSource device, MultiMediaContentType content, bool meta, bool folder, unsigned int dbIndex, DataBaseInfo *db);
int TMPlayerDB_GetTotalFileCount(unsigned int dbNum, MultiMediaContentType content);
bool TMPlayerDB_GetFilePath(DeviceSource device, MultiMediaContentType content, int dbIndex, std::string &filePath);
int TMPlayerDB_InitMetaPlayList(unsigned int dbNum);

#ifdef __cplusplus
}
#endif	

#endif
