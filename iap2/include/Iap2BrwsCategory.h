#define ITEMSPERPAGE         6
#define TOTALMENULIST        9

using namespace std;
class Iap2BrwsCategory {
    private:
         enum{
             playback_type_invalid = -1,
             playback_type_items = 0,
             playback_type_collection,
             playback_type_current_selection,
             playback_type_special
         };
        int mTotalItemCount;
        unsigned int mCurrentPage;
        unsigned int mCurrentIndex;
        unsigned int mParentIndex;
        unsigned int mCategoryType;
        unsigned int mLibraryType;
        unsigned int mMediaType;
        unsigned int mPlayType;
        string mCategoryName;
        Iap2BrwsCategory *mParent;
        vector<Iap2BrwsItem *> mItems;

    public:
        Iap2BrwsCategory();
        Iap2BrwsCategory(unsigned int libraryType,
                         unsigned int categoryType,
                         unsigned int mediaType,
                         unsigned int playType,
                         Iap2BrwsCategory *parent,
                         int index);
        ~Iap2BrwsCategory();
        Iap2BrwsCategory *start();
        Iap2BrwsCategory *select(int index);
        Iap2BrwsCategory *undo();
        Iap2BrwsCategory *clear(Iap2BrwsCategory *category);
        Iap2BrwsCategory *getParent();
        void move(int index);
        void deleteParents();

    private:
        Iap2BrwsCategory *selectRootCategory(int index);
        Iap2BrwsCategory *selectNewCategory(unsigned int libraryType,
                                            unsigned int categoryType,
                                            unsigned int mediaType,
                                            unsigned int playType,
                                            int index);
        void playSelectedItem(int index);
        unsigned int databaseGetCount();
        void databaseGetName(bool updatePage, bool move, bool undo);
        Iap2BrwsCategory *showMenuList(bool updatePage);
        void makeMenuCategoryList();
        string getCategoryName(int index);
        void parseSelectedItems(uint8_t *payload);
        Iap2BrwsItem *parseIdAndName(Iap2ObjGroup *group);
        void setCategoryName(unsigned int index);
        vector<Iap2BrwsItem *> getItems();
        unsigned int categoryTypeForMenu(); 
        void playbackWithItems(int index);
        void playbackCollection(int index);
        uint64_t *persistentIdList();
        int getCollectionType();
        unsigned int getCategoryType();
};
