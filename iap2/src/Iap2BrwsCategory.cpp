#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dbus/dbus.h>

#include "iAP2Const.h"
#include "iAP2NativeMsg.h"
#include "iAP2DbHandler.h"
#include "TCiAP2Manager.h"
#include "Iap2BrwsItem.h"
#include "Iap2BrwsCategory.h"

extern int g_brws_debug;
#define DBG_BRWS(format, arg...) \
	if (g_brws_debug) \
	{ \
		fprintf(stderr, "[BRWS Category] [%s] : " format "\n", __FUNCTION__,##arg); \
	}

#define LOG_BRWS(format, arg...) \
		fprintf(stderr, "[BRWS Category] [%s] : " format "\n", __FUNCTION__,##arg); 

static Iap2DbHandler *mDbHandler = NULL;

Iap2BrwsCategory::Iap2BrwsCategory() {
    mTotalItemCount = 0;
    mCurrentPage = 0;
    mCurrentIndex = 0;
    mParentIndex = 0;
    mLibraryType = 0;
    mCategoryType = 0;
    mMediaType = 0;
    mPlayType = 0;
    mParent = NULL;
    mCategoryName = (const char *) "CATEGORY\0";
    mDbHandler = new Iap2DbHandler();
}

Iap2BrwsCategory::Iap2BrwsCategory(unsigned int libraryType,
                                   unsigned int categoryType,
                                   unsigned int mediaType,
                                   unsigned int playType,
                                   Iap2BrwsCategory *parent,
                                   int index)
{
    mTotalItemCount = 0;
    mCurrentPage = 0;
    mCurrentIndex = index;
    mParentIndex = index;
    mLibraryType = libraryType;
    mCategoryType = categoryType;
    mMediaType = mediaType;
    mPlayType = playType;
    mParent = parent;
}

Iap2BrwsCategory::~Iap2BrwsCategory() {
    Iap2BrwsItem *item = NULL;
    for (unsigned int index = 0; index < mItems.size(); index++)
    {
        item = mItems.at(index);
        if (item != NULL)
            delete item;        
    }

    if ((mParent == NULL) && (mDbHandler != NULL))
        delete mDbHandler;
}

Iap2BrwsCategory *Iap2BrwsCategory::start() {
    mCurrentPage = 0;
    mCurrentIndex = 0;
    mTotalItemCount = TOTALMENULIST;
    LOG_BRWS(" currentIndex[%u] currentPage[%u] categoryType[%u] " \
                    "libraryType[%u] mediaType[%u] playType[%u] parent[%p]\n",
                     mCurrentIndex, mCurrentPage, mCategoryType,
                     mLibraryType, mMediaType, mPlayType, mParent);
    makeMenuCategoryList();
    return showMenuList(true);
}

Iap2BrwsCategory *Iap2BrwsCategory::select(int index) {
    Iap2BrwsCategory *category = this; 
    LOG_BRWS(" currentIndex[%u] currentPage[%u] categoryType[%u] " \
                    "libraryType[%u] mediaType[%u] playType[%u] parent[%p]\n",
                     mCurrentIndex, mCurrentPage, mCategoryType,
                     mLibraryType, mMediaType, mPlayType, mParent);
    switch(mCategoryType) {
        case db_category_All:
            category = selectRootCategory(index); 
            break;
        case db_category_Artist:
            category = selectNewCategory(media_library_type_LocalDeviceLib,
                                         db_category_Album,
                                         nowplaying_media_type_Music,
                                         playback_type_invalid,
                                         index);
            break;
        case db_category_Album:
        if ((mMediaType == nowplaying_media_type_Podcast) ||
            (mMediaType == nowplaying_media_type_AudioBook))
        {
            category = selectNewCategory(media_library_type_LocalDeviceLib,
                                         db_category_Track,
                                         nowplaying_media_type_Music,
                                         playback_type_items,
                                         index);
        }
        else
        {
            category = selectNewCategory(media_library_type_LocalDeviceLib,
                                         db_category_Track,
                                         nowplaying_media_type_Music,
                                         playback_type_collection,
                                         index);
        }
            break;
        case db_category_Genre:
            category = selectNewCategory(media_library_type_LocalDeviceLib,
                                         db_category_Artist,
                                         nowplaying_media_type_Music,
                                         playback_type_invalid,
                                         index);
            break;
        case db_category_Composer:
            category = selectNewCategory(media_library_type_LocalDeviceLib,
                                         db_category_Genre,
                                         nowplaying_media_type_Music,
                                         playback_type_invalid,
                                         index);
            break;
        case db_category_Playlist:
        if (mMediaType == nowplaying_media_type_Music)
        {
            category = selectNewCategory(media_library_type_LocalDeviceLib,
                                         db_category_Track,
                                         nowplaying_media_type_Music,
                                         playback_type_collection,
                                         index);
        }
        else
        {
            playSelectedItem(index);
        }
            break;
        case db_category_Track:
            playSelectedItem(index);
            break;
        default:
            fprintf(stderr, "[ERR]%s Invalid category type\n", __FUNCTION__);
    }

    return category;
}

Iap2BrwsCategory *Iap2BrwsCategory::undo() {
    LOG_BRWS(" currentIndex[%u] currentPage[%u] categoryType[%u] " \
                     "libraryType[%u] mediaType[%u] playType[%u] parent[%p]\n",
                     mCurrentIndex, mCurrentPage, mCategoryType,
                     mLibraryType, mMediaType, mPlayType, mParent);
    Iap2BrwsCategory *rCategory = this;
    if ((mParent != NULL) && (mParent->getParent() != NULL))
    {
        rCategory = mParent;
        rCategory->databaseGetName(true, false, true);
    }
    else
    {
        rCategory = mParent;
        if (rCategory != NULL)
            rCategory->showMenuList(true);
        else
            fprintf(stderr, "[ERR]%s undo on root category\n", __FUNCTION__);
    }

    return rCategory;
}

Iap2BrwsCategory *Iap2BrwsCategory::clear(Iap2BrwsCategory *category) {
    Iap2BrwsCategory *rCategory = this;
    if ((category != NULL) && (category != this))
        delete category;
    else
        fprintf(stderr,"[ERR}%s Invalid cateogry[%p:%p]\n",
                       __FUNCTION__, category, this);

    return rCategory;
}

void Iap2BrwsCategory::move(int index) {
    mCurrentPage = (index - 1) / ITEMSPERPAGE;
    mCurrentIndex = index - 1;
    LOG_BRWS(" currentIndex[%u] currentPage[%u] categoryType[%u] " \
                    "libraryType[%u] mediaType[%u] playType[%u] parent[%p]\n",
                     mCurrentIndex, mCurrentPage, mCategoryType,
                     mLibraryType, mMediaType, mPlayType, mParent);
    if (mParent != NULL)
    {
        databaseGetName(false, true, false);
    }
    else
    {
        makeMenuCategoryList();
        showMenuList(false);
    }
}

void Iap2BrwsCategory::deleteParents() {
    Iap2BrwsCategory *grandParent = NULL;
    Iap2BrwsCategory *parent = mParent;
    LOG_BRWS(" currentIndex[%u] currentPage[%u] categoryType[%u] " \
                    "libraryType[%u] mediaType[%u] playType[%u] parent[%p]\n",
                     mCurrentIndex, mCurrentPage, mCategoryType,
                     mLibraryType, mMediaType, mPlayType, mParent);
    while (parent != NULL) {
        grandParent = parent->getParent();
        if (grandParent != NULL)
            delete parent;
        parent = grandParent; 
    }
}

Iap2BrwsCategory *Iap2BrwsCategory::selectRootCategory(int index) {
    int categoryIndex = (mCurrentPage * ITEMSPERPAGE) + index;
    Iap2BrwsCategory *category = NULL;
    if ((categoryIndex >= db_category_Playlist) &&
        (categoryIndex <= db_category_Composer) &&
        (categoryIndex != db_category_Track))
    {
        category = new Iap2BrwsCategory(media_library_type_LocalDeviceLib,
                                        categoryIndex,
                                        nowplaying_media_type_Music,
                                        playback_type_invalid,
                                        this,
                                        index);
    }
    else if (categoryIndex == db_category_Track)
    {
        category = new Iap2BrwsCategory(media_library_type_LocalDeviceLib,
                                        categoryIndex,
                                        nowplaying_media_type_Music,
                                        playback_type_items,
                                        this,
                                        index);
    }
    else if (categoryIndex == db_category_Audiobook)
    {
        category = new Iap2BrwsCategory(media_library_type_LocalDeviceLib,
                                        db_category_Album,
                                        nowplaying_media_type_AudioBook,
                                        playback_type_invalid,
                                        this,
                                        index);
    }
    else if (categoryIndex == db_category_Podcast)
    {
        category = new Iap2BrwsCategory(media_library_type_LocalDeviceLib,
                                        db_category_Album,
                                        nowplaying_media_type_Podcast,
                                        playback_type_invalid,
                                        this,
                                        index);
    }
    else
    {
        category = new Iap2BrwsCategory(media_library_type_iTunesRadioLib,
                                        db_category_Playlist,
                                        nowplaying_media_type_iTunesU,
                                        playback_type_collection,
                                        this,
                                        index);
    }
    
    if (category != NULL) 
    {
        category->setCategoryName(categoryIndex);
        category->databaseGetName(true, false, false);
    }
    else
        fprintf(stderr, "[ERR} %s category is null\n", __FUNCTION__);
    return category;
}

Iap2BrwsCategory *Iap2BrwsCategory::selectNewCategory(unsigned int libraryType,
                                                      unsigned int categoryType,
                                                      unsigned int mediaType,
                                                      unsigned int playType,
                                                      int index)
{
    Iap2BrwsCategory *category = new Iap2BrwsCategory(libraryType,
                                                      categoryType,
                                                      mediaType,
                                                      playType, 
                                                      this,
                                                      index);
    if (category != NULL)
    {
        category->setCategoryName(categoryType);
        category->databaseGetName(true, false, false);
    }
    else
        fprintf(stderr, "[ERR}%s category is null\n", __FUNCTION__);
    return category;
}

void Iap2BrwsCategory::playSelectedItem(int index) {
    LOG_BRWS(" playType[%u] index[%u]\n", mPlayType, index);
    switch (mPlayType) {
        case playback_type_items:
            playbackWithItems(index);
            break;
        case playback_type_collection:
            playbackCollection(index);
            break;
        default:
            fprintf(stderr, "[ERR]%s invalid playType[%u]\n", __FUNCTION__, mPlayType);
    }

    deleteParents();
}

void Iap2BrwsCategory::databaseGetName(bool updatePage, bool move, bool undo) {
    uint8_t *payload = NULL;
    uint64_t persistentId = 0;
    Iap2BrwsItem *item = NULL;
    vector<Iap2BrwsItem *> items;
    LOG_BRWS(" parent[%p] grandPatent[%p]\n",
                    mParent, mParent->getParent());

    if ((mParent != NULL) && (mParent->getParent() == NULL))
    {
        if (GetDBusDbInitHandler((void *) mDbHandler, mLibraryType) >= 0)
            mTotalItemCount = GetDBusDatabaseInitWithLibType((void *)mDbHandler,
                                                                    mCategoryType,
                                                                    mMediaType);
        else
            fprintf(stderr, "[ERR]%s dbHandler[%p] init fail:libraryType[%u]\n",
                            __FUNCTION__, mDbHandler, mLibraryType);
    }
    else
    {
        if ((mParent != NULL) && !move && !undo)
        {
            items = mParent->getItems();
            if ((mCurrentIndex - 1) < items.size())
                item = items.at(mParentIndex - 1);
            if (item != NULL)
            {
                persistentId = item->getPersistentId(); 
                if (persistentId != 0)
                    mTotalItemCount = GetDBusDatabaseSelect((void *)mDbHandler,
                                                                    mCategoryType,
                                                                    persistentId);
                else
                    fprintf(stderr, "[ERR]%s invalid persist id\n", __FUNCTION__);
            }
            else
            {
                fprintf(stderr, "[ERR]%s item is null index[%u]\n", __FUNCTION__,
                                                                   mCurrentIndex);
            }
        }
        else if (undo && !move)
        {
            mTotalItemCount = GetDBusDatabaseMoveUpper((void *) mDbHandler);
        }
        else
            fprintf(stderr, "%s parent[%p] move[%d] undo[%d]\n",
                            __FUNCTION__, mParent, move, undo);
    }

    if (mTotalItemCount > 0)
    {
        payload = GetDBusDatabaseGetName((void *)mDbHandler,
                                                 mCurrentPage * ITEMSPERPAGE,
                                                 ITEMSPERPAGE); 
        parseSelectedItems(payload);
    }
    else
        fprintf(stderr, "[ERR]%s Invalid ItemCount[%d] type[%u] id[%llu]\n",
                         __FUNCTION__, mTotalItemCount, mCategoryType,
                        (unsigned long long) persistentId);
    showMenuList(updatePage);
}

Iap2BrwsCategory *Iap2BrwsCategory::showMenuList(bool updatePage) {
    int length = 0;
    char name[1024];
    Iap2BrwsItem *item = NULL;
    fprintf(stderr, "%s currentPage[%u] currentIndex[%u] items[%d]\n",
                    __FUNCTION__, mCurrentPage, mCurrentIndex, mTotalItemCount);
   
    if (updatePage) 
    {
        if (mTotalItemCount > 0)
        {
            iAP2EventCategoryIndexChange(mCurrentIndex, mTotalItemCount);
        }
        else
        {
            iAP2EventCategoryIndexChange(0, 1);
        }
		iAP2EventCategoryMenuChange(categoryTypeForMenu(), mCategoryName.c_str(), mCategoryName.size());
    }

    if (mTotalItemCount > 0)
    {
        for (unsigned int index = 0; index < mItems.size(); index++)
        {
            item = mItems.at(index);
            if (item != NULL)
            {
                item->getItemName();
                memset(name, 0x00, 1024);
                strcpy(name, (char *) item->getItemName().c_str());
                //name = (char *) item->getItemName().c_str();
                length = item->getItemName().size();
            }
            else
            {
                fprintf(stderr, "[ERR]%s item is null\n", __FUNCTION__);
                continue;
            }
			iAP2EventCategoryNameChange(mCurrentPage * ITEMSPERPAGE + index, name, length);
        }
		iAP2EventCategoryNameComplete();
    }
    else
    {
        fprintf(stderr, "%s No Items on database category[%d]\n",
                        __FUNCTION__, mCategoryType);
    }

    return this;
}

Iap2BrwsCategory *Iap2BrwsCategory::getParent() {
    return mParent;
}

void Iap2BrwsCategory::makeMenuCategoryList() {
    Iap2BrwsItem *item = NULL;
    unsigned int remain = ((mCurrentPage * ITEMSPERPAGE)  + ITEMSPERPAGE) > 
                           (unsigned int) mTotalItemCount ?
                           (ITEMSPERPAGE - (((mCurrentPage * ITEMSPERPAGE) +
                           ITEMSPERPAGE) - (unsigned int) mTotalItemCount)) :
                           ITEMSPERPAGE;
    mItems.clear();
    for (unsigned int i = mCurrentIndex + 1; i <= mCurrentIndex + remain; i++)
    {
        item = new Iap2BrwsItem(0, getCategoryName(i), false);
        mItems.push_back(item);
    }
}

string Iap2BrwsCategory::getCategoryName(int index) {
    string name;
    switch (index) {
        case 0:
            name = "CATEGORY\0";
            break;
        case 1:
            name = "PLAYLIST\0";
            break;
        case 2:
            name = "ARTIST\0";
            break;
        case 3:
            name = "ALBUM\0";
            break;
        case 4:
            name = "GENRE\0";
            break;
        case 5:
            name = "SONGS\0";
            break;
        case 6:
            name = "COMPOSER\0";
            break;
        case 7:
            name = "AUDIOBOOK\0";
            break;
        case 8:
            name = "PODCAST\0";
            break;
        case 9:
            name = "ITUNESRADIO\0";
            break;
        default:
            fprintf(stderr, "[ERR]%s Invalid index[%d]\n",
                            __FUNCTION__, index);
    }

    return name;
}

void Iap2BrwsCategory::parseSelectedItems(uint8_t *payload) {
    iAP2NativeMsg *nativeMsg = NULL;
    Iap2ObjGroup *rootGroup = NULL;
    Iap2ObjGroup *group = NULL;
    Iap2ObjItem *item = NULL;
    Iap2BrwsItem *brwsItem = NULL;
    int itemCount = 0;
    LOG_BRWS(" payload[%p] mTotalItemCount[%d]\n",
                    payload, mTotalItemCount);
    if (payload != NULL)
    {
        nativeMsg = new iAP2NativeMsg(payload, true);
        if (nativeMsg != NULL)
        {
            rootGroup = nativeMsg->getRootGroup();
            if (rootGroup != NULL)
            {
                mItems.clear();
                itemCount = rootGroup->getItemCount();
                for (int index = 0; index < itemCount; index++)
                {
                    item = rootGroup->getObjItem(index);
                    if (item != NULL)
                    {
                        switch (item->mParamId) {
                            case db_get_name_ItemGroup:
                                group = item->getItemGroup();
                                brwsItem = parseIdAndName(group);
                                if (brwsItem != NULL)
                                    mItems.push_back(brwsItem);
                                else
                                    fprintf(stderr, "[ERR] %s brwsItem is null\n",
                                                    __FUNCTION__);
                                break;
                            default:
                                fprintf(stderr, "%s mParamId[%d]\n",
                                                __FUNCTION__, item->mParamId);
                        }
                    }   
                    else
                    {
                        fprintf(stderr, "[ERR]%s item is null\n", __FUNCTION__);
                        continue;
                    }
                }
            }
            else
                fprintf(stderr, "[ERR]%s rootGroup is null\n", __FUNCTION__);
        }
        else
            fprintf(stderr, "[ERR]%s nativeMSg is null\n", __FUNCTION__);
    }
    else
        fprintf(stderr, "[ERR]%s payload is null\n", __FUNCTION__);
} 

Iap2BrwsItem *Iap2BrwsCategory::parseIdAndName(Iap2ObjGroup *group) {
    int itemCount = 0;
    int strLength = 0;
    Iap2ObjItem *item = NULL;
    Iap2BrwsItem *brwsItem = NULL;
    if (group != NULL)
    {
        brwsItem = new Iap2BrwsItem();
        itemCount = group->getItemCount();
        for (int index = 0; index < itemCount; index++) 
        {
            item = group->getObjItem(index);
            if (item != NULL)
            {
                switch (item->mParamId) {
                    case db_item_Name:
                        if (item->mParamType == IAP2_PARAM_STRING)
                            brwsItem->setItemName((const char *)
                                                  item->getItemString(&strLength));
                        else
                            fprintf(stderr, "[ERR]%s Invalid type [%d]\n",
                                            __FUNCTION__, item->mParamType);
                        break;
                    case db_item_PersistentId:
                        if (item->mParamType == IAP2_PARAM_NUMU64)
                            brwsItem->setPersistentId(item->getItemLong());
                        else
                            fprintf(stderr, "[ERR]%s Invalid type [%d]\n",
                                            __FUNCTION__, item->mParamType);
                        break;
                    case playlist_item_IsFolder:
                        if (item->mParamType == IAP2_PARAM_BOOLEAN)
                            brwsItem->setIsFolder(item->getItemInt() > 0 ?
                                                  true:false);
                        else
                            fprintf(stderr, "[ERR]%s Invalid type [%d]\n",
                                            __FUNCTION__, item->mParamType);
                        break;
                    default:
                        fprintf(stderr, "%s mParamId[%d]\n",
                                        __FUNCTION__, item->mParamId);
                }
            }
            else
                fprintf(stderr, "%sitem is null index[%d]\n", __FUNCTION__, index);
        }
    }
    else
        fprintf(stderr, "[ERR]%s group is null\n", __FUNCTION__);

    return brwsItem;
}

void Iap2BrwsCategory::setCategoryName(unsigned int index) {
    mCategoryName = getCategoryName(index);
}

vector<Iap2BrwsItem *> Iap2BrwsCategory::getItems() {
    return mItems;
}

unsigned int Iap2BrwsCategory::categoryTypeForMenu() {
    unsigned int categoryType = 0;
    if (mCategoryType == db_category_Album)
    {
        if (mMediaType == nowplaying_media_type_AudioBook)
            categoryType = db_category_Audiobook;
        else if (mMediaType == nowplaying_media_type_Podcast)
            categoryType = db_category_Podcast;
        else
            categoryType = mCategoryType;
    }
    else if (mCategoryType == db_category_Playlist)
    {
        if (mMediaType == nowplaying_media_type_iTunesU)
            categoryType = 9;
        else
            categoryType = mCategoryType; 
    }
    else
    {
        categoryType = mCategoryType;
    }
    return categoryType;
}

void Iap2BrwsCategory::playbackWithItems(int index) {
    uint8_t *msgPayload = NULL;
    iAP2NativeMsg *msg = new iAP2NativeMsg();
    LOG_BRWS(" index[%d]\n", index);

    if ((msg != NULL) && (mTotalItemCount > 0))
    {
        msg->addStringItem(play_items_LibUid,
                           IAP2_PARAM_STRING,
                           (void *) mDbHandler->getLibUid().c_str());
        msg->addIntItem(play_items_ItemsStartingIndex,
                        IAP2_PARAM_NUMU32,
                        mCurrentPage * ITEMSPERPAGE + index - 1);
        msg->addBlobItem(play_items_ItemsPersistentIdentifiers,
                         IAP2_PARAM_BLOB,
                         (void *) persistentIdList(),
                         (mTotalItemCount * sizeof(uint64_t)));

        msgPayload = msg->getRawNativeMsg();
        if(msgPayload != NULL)
        {
            int length = 0;
            memcpy(&length, msgPayload, 4);
            SendDBusiAP2PlayMediaLibraryItems((void *) msgPayload);
            free(msgPayload);
        }
        else
            fprintf(stderr, "[ERR]%s msgPayload is null\n", __FUNCTION__);
    }
    else
        fprintf(stderr, "[ERR]%s msg[%p]\n", __FUNCTION__, msg);
}

void Iap2BrwsCategory::playbackCollection(int index) {
    uint64_t persistentId = 0;
    uint8_t *msgPayload =NULL;
    vector<Iap2BrwsItem> items;
    Iap2BrwsItem *item;
    iAP2NativeMsg *msg = new iAP2NativeMsg();
    LOG_BRWS(" index[%d]\n", index);

    if ((msg != NULL) && (mParent != NULL))
    {
        if ((mCategoryType == db_category_Playlist) &&
            (mMediaType == nowplaying_media_type_iTunesU))
            item = mItems.at(index - 1);
        else
            item = mParent->getItems().at(mParentIndex - 1);

        if (item != NULL)
            persistentId = item->getPersistentId();

        msg->addLongItem(play_collection_PersistentIdentifiers,
                         IAP2_PARAM_NUMU64,
                         persistentId);
        msg->addIntItem(play_collection_Type,
                        IAP2_PARAM_ENUM,
                        getCollectionType());
        msg->addIntItem(play_collection_StartingIndex,
                        IAP2_PARAM_NUMU32,
                        mCurrentPage * ITEMSPERPAGE + index - 1);
        msg->addStringItem(play_collection_LibUid,
                           IAP2_PARAM_STRING,
                           (void *) mDbHandler->getLibUid().c_str());
        msgPayload = msg->getRawNativeMsg();
        if(msgPayload != NULL)
        {
            SendDBusiAP2PlayMediaLibraryCollection((void *)msgPayload);
            free(msgPayload);
        }
        else
            fprintf(stderr, "[ERR]%s msgPayload is null\n", __FUNCTION__);
    }
    else    
        fprintf(stderr, "[ERR]%s msg[%p] parent[%p]\n",
                        __FUNCTION__, msg, mParent);
}

uint64_t *Iap2BrwsCategory::persistentIdList() {
    uint8_t *payload = NULL;
    uint64_t *persistentIds = NULL;
    Iap2BrwsItem *item = NULL;
    LOG_BRWS(" items[%d]\n", mTotalItemCount);
    
    if (mTotalItemCount > 0)
    {
        persistentIds = (uint64_t *) malloc(mTotalItemCount * sizeof(uint64_t));
        payload = GetDBusDatabaseGetName((void *)mDbHandler,
                                                 0,
                                                 mTotalItemCount);
        if ((payload != NULL) && (persistentIds != NULL))
        {
            memset(persistentIds, 0x00, mTotalItemCount); 
            parseSelectedItems(payload);
            for (unsigned int idx = 0; idx < mItems.size(); idx++) {
                item = mItems.at(idx);
                if (item != NULL)
                    persistentIds[idx] = item->getPersistentId();
            }
        }
        else
            fprintf(stderr, "[ERR]%s payload[%p] pids[%p]\n",
                            __FUNCTION__, payload, persistentIds);
    }
    else
        fprintf(stderr, "[ERR]%s mTotalItems[%d]\n",
                        __FUNCTION__, mTotalItemCount);

    return persistentIds; 
}

int Iap2BrwsCategory::getCollectionType() {
    int collectionType = 0;
    unsigned int categoryType = 0;
    if ((mCategoryType == db_category_Playlist) &&
        (mMediaType == nowplaying_media_type_iTunesU))
        categoryType = mCategoryType;
    else
        categoryType = mParent->getCategoryType();
 
    switch (categoryType) {
        case db_category_Playlist:
            collectionType = collection_category_Playlist;
            break;
        case db_category_Artist:
            collectionType = collection_category_Artist;
            break;
        case db_category_Album:
            collectionType = collection_category_Album;
            break;
        case db_category_Genre:
            collectionType = collection_category_Genre;
            break;
        case db_category_AlbumArtist:
            collectionType = collection_category_AlbumArtist;
            break;
        case db_category_Composer:
            collectionType = collection_category_Composer;
            break;
        default:
            fprintf(stderr, "[ERR]%s Invalid category[%u]\n",
                            __FUNCTION__, mCategoryType);
    }

    return collectionType;
}

unsigned int Iap2BrwsCategory::getCategoryType() {
    return mCategoryType;
}

