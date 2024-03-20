/****************************************************************************************
 *	 FileName	 : Iap2BrwsManager.cpp
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
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <string> 
#include <string.h>
#include "iAP2Const.h"
#include "Iap2BrwsManager.h"
#include "TCDBusRawAPI.h"
#include "TCiAP2Manager.h"

extern int g_brws_debug;
#define DBG_BRWS(format, arg...) \
	if (g_brws_debug) \
	{ \
		fprintf(stderr, "[BRWS manager] [%s] : " format "\n", __FUNCTION__,##arg); \
	}

#define LOG_BRWS(format, arg...) \
		fprintf(stderr, "[BRWS manager] [%s] : " format "\n", __FUNCTION__,##arg); 

Iap2BrwsManager *Iap2BrwsManager::instance;

Iap2BrwsManager::Iap2BrwsManager()
{
    DBG_BRWS("constructing");
    mCurrentCategory = NULL;
    mRootCategory = new Iap2BrwsCategory();
    DBG_BRWS("constructed");
}

Iap2BrwsManager::~Iap2BrwsManager()
{
    if (mCurrentCategory != NULL)
    {
        mCurrentCategory->deleteParents();
        delete mCurrentCategory;
    }

    if (mRootCategory != NULL)
        delete mRootCategory;
}

Iap2BrwsManager *Iap2BrwsManager::getInstance() 
{
    if (instance == NULL)
    {
        instance = new Iap2BrwsManager();
    }
    return instance;
}

void Iap2BrwsManager::selectItem(int index)
{
    if (mCurrentCategory != NULL)
        mCurrentCategory = mCurrentCategory->select(index);
    else
        fprintf(stderr, "%s mCurrentCateogry is null", __FUNCTION__); 
}

void Iap2BrwsManager::undoitem()
{
    if (mCurrentCategory != NULL)
        mCurrentCategory = mCurrentCategory->undo()->clear(mCurrentCategory);
    else
        fprintf(stderr, "%s mCurrentCateogry is null", __FUNCTION__); 
}

void Iap2BrwsManager::move(int index)
{
    if (mCurrentCategory != NULL)
        mCurrentCategory->move(index);
    else
        fprintf(stderr, "%s mCurrentCateogry is null", __FUNCTION__); 
}

void Iap2BrwsManager::homeBrws(int itemCountPerPage)
{
    LOG_BRWS(" itemCountPerPage[%d]\n", itemCountPerPage);
    if (mRootCategory != NULL)
        mCurrentCategory = mRootCategory->start();
    else
        fprintf(stderr, "[ERR]%s RootCategory is null\n", __FUNCTION__);
}

void Iap2BrwsManager::exitBrws(int uiExitIndex)
{
    LOG_BRWS(" uiExitIndex[%d]\n", uiExitIndex);
    if (mCurrentCategory != NULL)
    {
        if (mCurrentCategory->getParent() != NULL)
        {
            mCurrentCategory->deleteParents();
            delete mCurrentCategory;
        }
        else
        {
            LOG_BRWS(" CurrentCategory is root\n");
        }
        mCurrentCategory = NULL;
    }
    else
        fprintf(stderr, "[ERR]%s CurrentCategory is null\n", __FUNCTION__);
        
}

void Iap2BrwsManager::addLibraryInformation(void *obj,int length)
{
    DBG_BRWS(" in length %d",length);
    iAP2BrwsLibrary *libraryManager = iAP2BrwsLibrary::getInstance();
    libraryManager->addLibrary(obj,length);
    for (int idx = 0; idx < libraryManager->getLibraryCount(); idx++)
        iAP2EventAddNewLibrary(libraryManager->getLibraryType(idx));
}

void Iap2BrwsManager::updateProgress(void *obj,int length)
{
    DBG_BRWS(" in");
    iAP2BrwsLibrary *libraryManager = iAP2BrwsLibrary::getInstance();
    current_progress_t currentProgress = libraryManager->updateLibrary(obj, length);
    iAP2EventUpdateNewLibraryProgress(currentProgress.type, currentProgress.progress);
}

void Iap2BrwsManager::dispatchNotify(int what,int arg1,int arg2,void *obj,int length)
{
    DBG_BRWS(" in");
    (void)arg1;
    (void)arg2;
    switch(what)
    {
        case IAP2_NOTI_MEDIA_LIB_NEW:
            {
                addLibraryInformation(obj,length);
            }
            break;
        case IAP2_NOTI_MEDIA_LIB_PROGRESS:
            {
                updateProgress(obj,length);
            }
            break;
        case IAP2_NOTI_MEDIA_LIB_CHANGED:
            {
                //if(mItemCountPerPage > 0)        
                //    homeBrws(mItemCountPerPage);
            }
            break;
    }
    DBG_BRWS(" out ");
}

