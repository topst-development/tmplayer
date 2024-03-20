/****************************************************************************************
 *	 FileName	 : Iap2BrwsManager.h
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

#ifndef TC_IAP2_BRWS_MANAGER_H
#define TC_IAP2_BRWS_MANAGER_H

#include "iAP2NativeMsg.h"
#include "iAP2DbHandler.h"
#include "Iap2BrwsItem.h"
#include "iAP2BrwsLibrary.h"
#include "Iap2BrwsCategory.h"

#define DEFAULT_ITEM_COUNT_PER_PAGE (6)

class Iap2BrwsManager {

public:
    Iap2BrwsCategory *mRootCategory;
    Iap2BrwsCategory *mCurrentCategory;


    bool m3rdPartyApp;
    pthread_mutex_t mCurrentCategoryMutex;
    pthread_mutex_t mProgressCategoryMutex;
    pthread_mutex_t mRootCategoryMutex;
    int mItemCountPerPage;
    static Iap2BrwsManager *instance;
public:
    Iap2BrwsManager();
    ~Iap2BrwsManager();
    static Iap2BrwsManager *getInstance();  
    void selectItem(int uiSelectIndex);
    void undoitem();
    void move(int uiMoveIndex);
    void homeBrws(int itemCountPerPage);
    void exitBrws(int uiExitIndex);
    void addLibraryInformation(void *obj,int length);
    void updateProgress(void *obj,int length);
    void dispatchNotify(int what,int arg1,int arg2,void *obj,int length);
};


#endif

