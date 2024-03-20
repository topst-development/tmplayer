/****************************************************************************************
 *	 FileName	 : iAP2BrwsLibrary.cpp
 *	 Description : 
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
#include <unistd.h>
#include <vector>
#include <string> 
#include <string.h>
//#include <QProgressDialog>
#include "iAP2Const.h"
#include "iAP2NativeMsg.h"
#include "iAP2DbHandler.h"
//#include "iAPPlayer.h"
#include "iAP2BrwsLibrary.h"
#include "TCDBusRawAPI.h"
#include "TCiAP2Manager.h"

int g_brws_library_debug = 0;
#define DBG_BRWS(format, arg...) \
	if (g_brws_library_debug) \
	{ \
		fprintf(stderr, "[BRWS Library] [%s] : " format "\n", __FUNCTION__,##arg); \
	}

#define LOG_BRWS(format, arg...) \
		fprintf(stderr, "[BRWS Library] [%s] : " format "\n", __FUNCTION__,##arg); 

iAP2BrwsLibrary *iAP2BrwsLibrary::instance;

enum{
    DB_UPDATE_FSM_INITIALIZE = 1,
    DB_UPDATE_FSM_STARTED,
    DB_UPDATE_FSM_STOP_AFTER_INITIALIZE,
    DB_UPDATE_FSM_STOPED
};

iAP2BrwsLibrary::iAP2BrwsLibrary()
{
    LOG_BRWS("in");
    cleanup();
}

iAP2BrwsLibrary::~iAP2BrwsLibrary()
{
    LOG_BRWS("in");
    cleanup();
}

iAP2BrwsLibrary *iAP2BrwsLibrary::getInstance() 
{
    if (instance == NULL)
    {
        instance = new iAP2BrwsLibrary();
    }
    return instance;
}

void iAP2BrwsLibrary::cleanup()
{
    int count = mDbHandler.size();
    int i;
    for(i =0; i < count;i++)
    {
        Iap2DbHandler *dbHandler = mDbHandler.at(i);
        delete dbHandler;
    }
    mDbHandler.clear();
    mDbInitializeFinished_cb = NULL;
    mUpdateStart_cb = NULL;
    mUpdateStop_cb = NULL;
    mUpdateFsm = DB_UPDATE_FSM_INITIALIZE;
}

void iAP2BrwsLibrary::setCallbackFunc(dbInitializeFinished_callback dbInitFinished_cb,
                                                                                dbUpdateStart_callback updateStart_cb,
                                                                                dbUpdateStop_callback updateStop_cb)
{
    mDbInitializeFinished_cb = dbInitFinished_cb;
    mUpdateStart_cb = updateStart_cb;
    mUpdateStop_cb = updateStop_cb;
}

int iAP2BrwsLibrary::pargeLibraryGroup(Iap2ObjGroup * group)
{
    Iap2ObjItem *item = NULL;
    int cnt;
    int libraryType = -1;
    unsigned char *pLibraryUid = NULL;

    if(group == NULL)
    {
        return libraryType;
    }

    LOG_BRWS("in getItemCount %d",group->getItemCount());
    for(cnt=0; cnt < group->getItemCount(); cnt++)
    {
        item = group->getObjItem(cnt);
        DBG_BRWS(" id %d type %d",item->mParamId,item->mParamType);
        switch(item->mParamType)
        {
            case IAP2_PARAM_STRING:
                {
                    unsigned char *pStr;
                    int strLen = 0;
                    pStr = item->getItemString(&strLen);
                    
                    switch(item->mParamId)
                    {
                        case media_library_InfoUID:
                            if(strLen > 0)
                            {
                                pLibraryUid = pStr;
                                DBG_BRWS(" pLibraryUid %s strLen %d",pLibraryUid,strLen);
                            }
                            break;
                        case media_library_InfoName:
                             if(strLen > 0)
                            {
                                DBG_BRWS(" Library Name %s strLen %d",pLibraryUid,strLen);
                            }
                            break;
                            break;
                    }
                }
                break;
            case IAP2_PARAM_ENUM:
                {
                    unsigned int data32 = item->getItemInt();
                    switch(item->mParamId)
                    {
                        case media_library_InfoType:
                            libraryType = (int)data32;
                            DBG_BRWS(" libraryType %d",libraryType);
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }

    return libraryType;
    //if(pLibraryUid != NULL && libraryType >= 0 )
    //{
    //    DBG_BRWS(" library UID %s type %d",pLibraryUid,libraryType);
    //    Iap2DbHandler *dbHandler = getDbHandler(libraryType);
    //    if(dbHandler == NULL)
    //    {
    //        dbHandler = new Iap2DbHandler(libraryType);
    //        dbHandler->setLibUid((const char *)pLibraryUid);
    //        mDbHandler.push_back(dbHandler);
    //        DBG_BRWS(" create new db handler (dbHandlerList size %d)", (int) mDbHandler.size());
    //    }
    //    else
    //    {
    //        dbHandler->setProgress(0);
    //    }
    //}
}

void iAP2BrwsLibrary::addLibrary(void *obj,int length)
{
   iAP2NativeMsg *msg = NULL;
    Iap2ObjGroup *rootGroup = NULL;
    Iap2ObjItem *item = NULL;
    int cnt = 0;
    int libraryType = -1;

    if(obj == NULL)
    {
        return;
    }

    mLibraryType.clear();
    msg = new iAP2NativeMsg();//(uint8_t *)obj,length,false);
    rootGroup = msg->getRootGroup();
    rootGroup->setPayloadData((uint8_t *)obj, length);
    LOG_BRWS("in getItemCount %d",rootGroup->getItemCount());
    for(cnt=0; cnt < rootGroup->getItemCount(); cnt++)
    {
        item = rootGroup->getObjItem(cnt);
        DBG_BRWS(" id %d type %d",item->mParamId,item->mParamType);
        switch(item->mParamType)
        {
            case IAP2_PARAM_GROUP:
                {
                    Iap2ObjGroup * group;
                    group = item->getItemGroup();
                    
                    switch(item->mParamId)
                    {
                        case media_library_Information:
                            libraryType = pargeLibraryGroup(group);
                            mLibraryType.push_back(libraryType);
                            break;
                        default:
                            break;
                    }
                }
                break;
            
                break;
            default:
                break;
        }
    }
}

current_progress_t iAP2BrwsLibrary::updateLibrary(void *obj,int length)
{
    iAP2NativeMsg *msg = NULL;
    Iap2ObjGroup *rootGroup = NULL;
    Iap2ObjItem *item = NULL;
    int cnt;
    int libraryType = -1;
    unsigned char *pLibraryUid = NULL;
    int progress = -1;
    current_progress_t currentProgress;
    currentProgress.progress = progress;
    currentProgress.type = libraryType;
    

    LOG_BRWS("in updateLibrary %d",length);

    if(obj == NULL)
    {
        return currentProgress;
    }

    msg = new iAP2NativeMsg();//(uint8_t *)obj,length,false);
    rootGroup = msg->getRootGroup();
    rootGroup->setPayloadData((uint8_t *)obj, length);

    for(cnt=0; cnt < rootGroup->getItemCount(); cnt++)
    {
        item = rootGroup->getObjItem(cnt);
        DBG_BRWS(" id %d type %d",item->mParamId,item->mParamType);
        switch(item->mParamType)
        {
            case IAP2_PARAM_STRING:
                {
                    unsigned char *pStr;
                    int strLen = 0;
                    pStr = item->getItemString(&strLen);
                    switch(item->mParamId)
                    {
                        case media_update_Uid:
                            if(strLen > 0)
                            {
                                pLibraryUid = pStr;
                                DBG_BRWS(" pLibraryUid %s strLen %d",pLibraryUid,strLen);
                            }
                            break;
                    }
                }
                break;
            case IAP2_PARAM_ENUM:
                {
                    unsigned int data32 = item->getItemInt();
                    switch(item->mParamId)
                    {
                        case media_update_Type:
                            libraryType = (int)data32;
                            DBG_BRWS(" libraryType %d",libraryType);
                            break;
                    }
                }
                break;
            case IAP2_PARAM_NUMU32:
                {
                    unsigned int data32 = item->getItemInt();
                    switch(item->mParamId)
                    {
                        case media_update_Progress:
                            progress = (int)data32;
                            break;
                    }
                }
                break;
            default:
                break;
        }
    }

    currentProgress.progress = progress;
    currentProgress.type = libraryType;
    return currentProgress;
    //if(pLibraryUid != NULL && libraryType >= 0 && progress >= 0)
    //{
    //    DBG_BRWS(" library UID %s type %d progress %d",pLibraryUid,libraryType,progress);
    //    Iap2DbHandler *dbHandler = getDbHandler(libraryType);
    //    if(dbHandler != NULL)
    //    {
    //        dbHandler->setProgress(progress);
    //        if(isAllReady())
    //        {
    //            if(updateFsmByProgress(true))
    //            {
    //                
    //            }
    //        }
    //    }
    //    else
    //    {
    //         LOG_BRWS(" can not find the proper db handler type %d",libraryType);
    //    }
    //}
}

Iap2DbHandler *iAP2BrwsLibrary::getDbHandler(int libraryType)
{
    Iap2DbHandler *dbHandler = NULL;
    int count = mDbHandler.size();
    int i;
    for(i = 0;i < count;i++)
    {
        dbHandler = mDbHandler.at(i);
        if(dbHandler->getLibType() == libraryType)
        {
            break;
        }
        dbHandler = NULL;
    }
    if(dbHandler == NULL)
    {
        LOG_BRWS(" can not find the proper db handler size %d i %d type %d",(int) mDbHandler.size(),i,libraryType);
    }
    
    return dbHandler;
}

int iAP2BrwsLibrary::getDbHandlerCount()
{
    return mDbHandler.size();
}

int iAP2BrwsLibrary::getDbHandlerTypeByIndex(int index)
{
    int libraryType = media_library_type_Invalid;
    if(index < (int)mDbHandler.size())
    {
        Iap2DbHandler *dbHandler = mDbHandler.at(index);
        if(dbHandler != NULL)
        {
            libraryType = dbHandler->getLibType();
        }
    }
    return libraryType;
}

int iAP2BrwsLibrary::getProgressByIndex(int index)
{
    int progress = 0;
    if(index < (int)mDbHandler.size())
    {
        Iap2DbHandler *dbHandler = mDbHandler.at(index);
        if(dbHandler != NULL)
        {
            progress = dbHandler->getProgress();
        }
    }
    return progress;
}

bool iAP2BrwsLibrary::isAllReady()
{
    bool ret = false;
    Iap2DbHandler *dbHandler = NULL;
    int count = mDbHandler.size();
    int i;
    int progress = 0;
    
    DBG_BRWS(" in mcount %d",count);
    for(i = 0;i < count;i++)
    {
        dbHandler = mDbHandler.at(i);
        progress = dbHandler->getProgress();

        if ((progress >= 0) && (progress <= 100))
        {
        }
        else
        {
            DBG_BRWS(" ERROR: progress is %d", progress);
        }
        
        if(progress < 100)
        {
            ret = false;
            break;
        }
        else
        {
            ret = true;
        }
    }
    DBG_BRWS(" ret %d",ret);
    if (ret)
    {
        updateFsmByProgress(true);
    }
    return ret;
}

void iAP2BrwsLibrary::startUpdate()
{
    if(mUpdateStart_cb != NULL)
    {
        mUpdateStart_cb();
    }
}

void iAP2BrwsLibrary::stopUpdate()
{
    if(mUpdateStop_cb != NULL)
    {
        mUpdateStop_cb();
    }
}

void iAP2BrwsLibrary::updateFsmByHide(bool hide)
{
    DBG_BRWS(" in mUpdateFsm %d hide %d",mUpdateFsm,hide);
    switch(mUpdateFsm)
    {
        case DB_UPDATE_FSM_INITIALIZE:
            if(hide == true)
            {
                mUpdateFsm = DB_UPDATE_FSM_STOP_AFTER_INITIALIZE;
            }
            break;
        case DB_UPDATE_FSM_STARTED:
            if(hide == true)
            {
                stopUpdate();
                mUpdateFsm = DB_UPDATE_FSM_STOPED;
            }
            break;
        case DB_UPDATE_FSM_STOP_AFTER_INITIALIZE:
            if(hide == false)
            {
                mUpdateFsm = DB_UPDATE_FSM_INITIALIZE;
            }
            break;
        case DB_UPDATE_FSM_STOPED:
            if(hide == false)
            {
                startUpdate();
                mUpdateFsm = DB_UPDATE_FSM_STARTED;
            }
            break;
    }
    DBG_BRWS(" out mUpdateFsm %d ",mUpdateFsm);
}

bool iAP2BrwsLibrary::updateFsmByProgress(bool initializeFinished)
{
    bool isInitializeFinished = false;
    DBG_BRWS(" in mUpdateFsm %d initializeFinished %d",mUpdateFsm,initializeFinished);
    switch(mUpdateFsm)
    {
        case DB_UPDATE_FSM_INITIALIZE:
            if(initializeFinished == true)
            {
                mUpdateFsm = DB_UPDATE_FSM_STARTED;
                isInitializeFinished = true;
            }
            break;
        case DB_UPDATE_FSM_STARTED:
            break;
        case DB_UPDATE_FSM_STOP_AFTER_INITIALIZE:
            if(initializeFinished == true)
            {
                stopUpdate();
                mUpdateFsm = DB_UPDATE_FSM_STOPED;
            }
            break;
        case DB_UPDATE_FSM_STOPED:
            break;
    }
    DBG_BRWS(" out mUpdateFsm %d ",mUpdateFsm);
    return isInitializeFinished;
}

void iAP2BrwsLibrary::setMusicApplicationHide(bool hide)
{
    mHide = hide;
    updateFsmByHide(hide);
}

bool iAP2BrwsLibrary::initFilter(Iap2DbHandler *dbHandler,int categoryType)
{
    bool ret = false;
    ret = dbHandler->initFilter(categoryType, dbHandler->getLibType());
    return ret;
}

bool iAP2BrwsLibrary::addFilter(Iap2DbHandler *dbHandler,int subCategory, uint64_t persistentId)
{
    bool ret = false;
    ret = dbHandler->addFilter(subCategory, persistentId);
    return ret;
}

bool iAP2BrwsLibrary::addFilterForAll(Iap2DbHandler *dbHandler,int subCategory)
{
    bool ret = false;
    ret = dbHandler->addFilterForAll(subCategory);
    return ret;
}

bool iAP2BrwsLibrary::moveUp(Iap2DbHandler *dbHandler)
{
    bool ret = false;
    ret = dbHandler->moveUp();
    return ret;
}

int iAP2BrwsLibrary::getLibraryCount() 
{
    return (int) mLibraryType.size();
}

int iAP2BrwsLibrary::getLibraryType(int index)
{
    int type = -1;
    if (index < mLibraryType.size())
        type = mLibraryType.at(index);
    return type;
}
