#include <stdint.h>
#include <string>
#include "Iap2BrwsItem.h"

Iap2BrwsItem::Iap2BrwsItem() {
}

Iap2BrwsItem::Iap2BrwsItem(uint64_t id, string name, bool folder) {
    persistentId = id;
    itemName = name;
    isFolder = folder; 
}

Iap2BrwsItem::~Iap2BrwsItem() {
}

void Iap2BrwsItem::setIsFolder(bool folder) {
    isFolder = folder;
}

void Iap2BrwsItem::setPersistentId(uint64_t id) {
    persistentId = id;
}

void Iap2BrwsItem::setItemName(string name) {
    itemName = name;
}

bool Iap2BrwsItem::getIsFolder() {
    return isFolder;
}

uint64_t Iap2BrwsItem::getPersistentId() {
    return persistentId;
}

string Iap2BrwsItem::getItemName() {
    return itemName;
}
