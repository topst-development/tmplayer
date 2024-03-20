using namespace std;
class Iap2BrwsItem {
    private:                                                                          
        bool isFolder;
        uint64_t persistentId;
        string itemName;                                                              
    public:
        Iap2BrwsItem();
        Iap2BrwsItem(uint64_t id, string name, bool folder);                                       
        ~Iap2BrwsItem();
        
        void setIsFolder(bool folder);                                                
        void setPersistentId(uint64_t id);                                            
        void setItemName(string name);
        bool getIsFolder();
        uint64_t getPersistentId();
        string getItemName();
};
