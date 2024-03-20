/****************************************************************************************
 *   FileName    : TMPlayerDB.c
 *   Description : TmPlayerDB.c
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
#include <iostream>
#include <string>
#include <string.h>
#include <time.h>
#include <vector>
#include "sqlite3.h"
#include "MetaParser.h"
#include "TCDBDefine.h"
#include "TCDBGen.h"
#include "TMPlayerType.h"
#include "TMPlayerDB.h"

extern int g_tc_debug;
#define DEBUG_TMPLAYER_DB_PRINTF(format, arg...) \
	if (g_tc_debug) \
	{ \
		fprintf(stderr, "[TMPLAYER DB] %s: " format "", __FUNCTION__, ##arg); \
	}
static TMPlayerDBFileDBResult_cb			_TMPlayerDBFileDBResult = NULL;
static TMPlayerDBFileDBUpdate_cb		_TMPlayerDBFileDBUpdate = NULL;
static TMPlayerDBMetaDBResult_cb		_TMPlayerDBMetaDBResult = NULL;
static TMPlayerDBMetaDBUpdate_cb		_TMPlayerDBMetaDBUpdate = NULL;

static ContentsType ConversionTablebyContentType(MultiMediaContentType content);
static int AddList_cb(void *args, int rowCount, int Index);
static bool CreateFileList_All(DeviceSource device, ContentsType contentType, DataBaseInfo *db);
static bool CreateFileList_Folder(DeviceSource device, ContentsType contentType, unsigned int folderIndex, DataBaseInfo *db);
static bool CreateMetaList(DeviceSource device, DataBaseInfo *db);
static void CreateShuffleList(DeviceSource device, bool meta, DataBaseInfo *db);

int TMPlayerDB_Initialize(const char* _myDBPath)
{
	DEBUG_TMPLAYER_DB_PRINTF("DB Path : %s\n", _myDBPath);
	int ret;
	TCDB_EventCB dbEventcallback;
	TCDB_UserCB dbUsercallback;
	TCDB_SetDebug(g_tc_debug);
	ret =  TCDB_Initialize(_myDBPath, TotalDeviceSources, "/usr/share/tc-dbgen/tc-dbconf.xml");
	/* regist callback fucntion for Check Extention */
	dbEventcallback._fileDBResult = _TMPlayerDBFileDBResult;
	dbEventcallback._metaDBResult = _TMPlayerDBMetaDBResult;
	dbEventcallback._fileDBUpdate = _TMPlayerDBFileDBUpdate;
	dbEventcallback._metaDBUpdate = _TMPlayerDBMetaDBUpdate;

	TCDB_SetEventCallbackFunctions(&dbEventcallback);

	dbUsercallback._compareFunction = NULL;
	dbUsercallback._gatheringTag = TagParsingCB;
	TCDB_SetUserCallbackFunctions(&dbUsercallback);

	return ret;
}

void TMPlayerDB_Release(void)
{
	DEBUG_TMPLAYER_DB_PRINTF("\n");
	TCDB_Release();
}

void TMPlayerDB_Debug(int state)
{
	DEBUG_TMPLAYER_DB_PRINTF("%s \n", state ? "ON" : "OFF");
	TCDB_SetDebug(state);
}

void TMPlayerDB_SetEventCallback(TMPlayerDB_EventCB *cb)
{
	DEBUG_TMPLAYER_DB_PRINTF("\n");
	_TMPlayerDBFileDBResult = cb->_TMPlayerDBFileDBResult;
	_TMPlayerDBFileDBUpdate = cb->_TMPlayerDBFileDBUpdate;
	_TMPlayerDBMetaDBResult = cb->_TMPlayerDBMetaDBResult;
	_TMPlayerDBMetaDBUpdate= cb->_TMPlayerDBMetaDBUpdate;
}

int TMPlayerDB_AddDataBase(DeviceSource device, const char *mountPath)
{
	DEBUG_TMPLAYER_DB_PRINTF("DeviceSource : %d path : %s\n", device, mountPath);
	bool ret = false;
	if(device < TotalDeviceSources && strlen(mountPath) > 0)
	{
		if(TCDB_GenerateFileDB(device, mountPath) == DB_SUCCESS)
		{
			ret = true;
		}
		else 
		{
			fprintf(stderr, "%s: SQLite_AddDevice failed in %s\n", __func__, mountPath);
		}
	}
	return ret;	
}

int TMPlayerDB_CreateMetaDB(unsigned int dbNum)
{
	DEBUG_TMPLAYER_DB_PRINTF("dbNum : %d \n", dbNum);
	bool ret = false;
	if(TCDB_Meta_MakeDB(dbNum) == DB_SUCCESS)
	{
		ret = true;
	}
	else
	{
		fprintf(stderr, "%s: Meta DB Start Error, Device[%d]\n", __func__, dbNum);
	}
	return ret;
}

void TMPlayerDB_DeleteDateBase(DeviceSource device)
{
	DEBUG_TMPLAYER_DB_PRINTF("DeviceSource : %d\n", device);
	TCDB_DeleteDB(device);
}

bool TMPlayerDB_CreatePlayList(DeviceSource device, MultiMediaContentType content, bool meta, bool folder, unsigned int dbIndex, DataBaseInfo *db)
{
	bool result = false;
	DEBUG_TMPLAYER_DB_PRINTF("Device(%d), Content(%d), Meta(%s), Folder(%s), DB Index(%u)\n", 
							device, content, meta ? "TRUE" : "FALSE", folder ? "TRUE" : "FALSE", dbIndex);
	ContentsType contentType;
	contentType = ConversionTablebyContentType(content);		//convert int to ContentsType
	
	if (meta)
	{
		result = CreateMetaList(device, db);
	}
	else if(folder)
	{
		int folderIndex;
		folderIndex = TCDB_GetParentFolderIndexofFile(device, dbIndex, contentType);
		result = CreateFileList_Folder(device, contentType, folderIndex, db);
	}
	else
	{
		result = CreateFileList_All(device, contentType, db);		
	}

	if(result)
	{
		CreateShuffleList(device, meta, db);
	}
	return result;
}

int TMPlayerDB_GetTotalFileCount(unsigned int dbNum, MultiMediaContentType content)
{
	ContentsType contentType;
	contentType = ConversionTablebyContentType(content);
	return TCDB_GetTotalFileCount(dbNum, contentType);
}

bool TMPlayerDB_GetFilePath(DeviceSource device, MultiMediaContentType content, int dbIndex, std::string &filePath)
{
	bool found = false;
	ContentsType contentType;	//libdbgen type
	char _path[MAX_PATH_LENGTH];
	DEBUG_TMPLAYER_DB_PRINTF("DEVICE(%d), CONTENT(%d), DB INDEX(%d)\n",
						   device, content, dbIndex); 

	if (dbIndex != INVALID_INDEX)
	{
		memset(_path, 0x00, MAX_PATH_LENGTH);
		contentType = ConversionTablebyContentType(content);		//convert int to ContentsType
		if (TCDB_GetFileFullPath(device, dbIndex, _path, MAX_PATH_LENGTH, contentType) == DB_SUCCESS)
		{
			filePath = (const char *)_path;
			found = true;
		}
		else
		{
			fprintf(stderr, "%s: Get_FullPathName failed. device (%d), db index (%d), file path (%s), content type (%d)\n",
				__func__, device, dbIndex, _path, contentType);
		}
	}
	return found;
}

int TMPlayerDB_InitMetaPlayList(unsigned int dbNum)
{
	int ret;

	ret = TCDB_Meta_ResetBrowser(dbNum);
	if (ret == DB_SUCCESS)
	{
		ret = TCDB_Meta_GetNumberCategory(dbNum, "Title\0");
		if (ret > 0)
		{
			ret = TCDB_Meta_MakePlayList(dbNum, 0);

			if (ret != DB_SUCCESS)
			{
				fprintf(stderr, "%s: Meta DB Total Count Initialize - Make Play List Fail (%d)\n", __func__, ret);
			}
		}
		else
		{
			fprintf(stderr, "%s: Meta DB Total Count Initialize - Get Number Category Fail (%d)\n", __func__, ret);
		}
	}
	else
	{
		fprintf(stderr, "%s: No Meta DB (%d)\n", __func__, ret);
	}

	return ret;
}

static ContentsType ConversionTablebyContentType(MultiMediaContentType content)	//convert MultiMediaContentType to ContentsType
{
	ContentsType contentType;

	switch(content)
	{
		case MultiMediaContentTypeAudio:
			contentType = AudioContents;
			break;
		case MultiMediaContentTypeVideo:
			contentType = VideoContents;
			break;
		default:
			contentType = AudioContents;
			break;
	}
	return contentType;
}

static int AddList_cb(void *args, int rowCount, int Index)
{
	int ret =-1;
	DataBaseInfo *db;

	if(args != NULL)
	{
		db = (DataBaseInfo *)args;
		if((unsigned int)rowCount <= db->_totalNumber )
		{
			db->_list.push_back(Index);
		}
		ret =0;
	}
	return ret;
}

static bool CreateFileList_All(DeviceSource device, ContentsType contentType, DataBaseInfo *db)
{
	DataBaseInfo *tempdb;
	bool result = false;
	tempdb = db;
	
	tempdb->_list.clear();
	tempdb->_totalNumber =TCDB_GetTotalFileCount(device, contentType);
	
	if(TCDB_AddAllFileList(device, contentType, AddList_cb, (void *)tempdb) ==0)
	{
		result = true;
	}
	return result;	
}

static bool CreateFileList_Folder(DeviceSource device, ContentsType contentType, unsigned int folderIndex, DataBaseInfo *db)
{
	DataBaseInfo *tempdb;
	bool result = false;
	tempdb = db;

	tempdb->_list.clear();
	tempdb->_totalNumber = TCDB_GetSubFileCount(device, folderIndex, contentType);

	if(TCDB_AddSubFileList(device,folderIndex, contentType, AddList_cb, (void *)tempdb) ==0 )
	{
		result = true;
	}

	return result;
}

static bool CreateMetaList(DeviceSource device,  DataBaseInfo *db)
{
	DataBaseInfo *tempdb;
	unsigned int i;
	int index;
	bool result = false;
	tempdb = db;

	tempdb->_metaList.clear();
	tempdb->_totalNumber = TCDB_Meta_GetTotalTrack((unsigned int)tempdb->_device);
	if(tempdb->_totalNumber != INVALID_INDEX)
	{
		for (i = 0; i < tempdb->_totalNumber; i++)
		{
			index = TCDB_Meta_GetTrackList(tempdb->_device, i);
			tempdb->_metaList.push_back(index);
		}
		result = true;
	}
	return result;
}

static void CreateShuffleList(DeviceSource device, bool meta, DataBaseInfo *db)
{
	if (device < TotalDeviceSources)
	{
		DataBaseInfo *tempdb;
		int index;
		int count;

		tempdb = db;

		if (meta)
		{
			count = tempdb->_metaList.size();
		}
		else
		{
			count = tempdb->_list.size();
		}

		tempdb->_shuffleList.clear();
		for (index = 0; index < count; index++)
		{
			tempdb->_shuffleList.push_back(index + 1);
		}

		if (tempdb->_shuffle)
		{
			int temp;
			srand(time(NULL));
			for (int i = count; i > 1; i--)
			{
				index = rand() % i;
				temp = tempdb->_shuffleList[index];
				tempdb->_shuffleList[index] = tempdb->_shuffleList[i - 1];
				tempdb->_shuffleList[i - 1] = temp;
			}
		}
	}
}
