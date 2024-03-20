/****************************************************************************************
 *	 FileName	 : iAP2BrwsLibrary.h
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

#ifndef TC_IAP2_MEDIA_LIBRARY_MANAGER_H
#define TC_IAP2_MEDIA_LIBRARY_MANAGER_H

using namespace std;

typedef bool (*dbInitializeFinished_callback)();
typedef void (*dbUpdateStart_callback)();
typedef void (*dbUpdateStop_callback)();
typedef struct {
    int type;
    int progress;
} current_progress_t;

class iAP2BrwsLibrary{
private:
    bool mHide;
    int mUpdateFsm;
    vector<Iap2DbHandler *> mDbHandler;
    vector<int> mLibraryType;
    dbInitializeFinished_callback mDbInitializeFinished_cb;
    dbUpdateStart_callback mUpdateStart_cb;
    dbUpdateStop_callback mUpdateStop_cb;

    void startUpdate();
    void stopUpdate();
    void updateFsmByHide(bool hide);
    bool updateFsmByProgress(bool initializeFinished);
    static iAP2BrwsLibrary *instance;
    
public:
    iAP2BrwsLibrary();
    ~iAP2BrwsLibrary();
    static iAP2BrwsLibrary *getInstance(); 
    void cleanup();
    void setCallbackFunc(dbInitializeFinished_callback dbInitFinished_cb,
                                            dbUpdateStart_callback updateStart_cb,
                                            dbUpdateStop_callback updateStop_cb);
    int pargeLibraryGroup(Iap2ObjGroup * group);
    void addLibrary(void *obj,int length);
    current_progress_t updateLibrary(void *obj,int length);
    Iap2DbHandler *getDbHandler(int libraryType);
    int getDbHandlerCount();
    int getDbHandlerTypeByIndex(int index);
    int getProgressByIndex(int index);
    bool isAllReady();
    void setMusicApplicationHide(bool hide);
    bool initFilter(Iap2DbHandler *dbHandler,int categoryType);
    bool addFilter(Iap2DbHandler *dbHandler,int subCategory, uint64_t persistentId);
    bool addFilterForAll(Iap2DbHandler *dbHandler,int subCategory);
    bool moveUp(Iap2DbHandler *dbHandler);
    int getLibraryCount();
    int getLibraryType(int index);
};


#endif
