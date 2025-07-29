#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "windows.h"
#include "assert.h"
#include <locale.h> 
#include "stdbool.h"

typedef enum ItemType {
    ITEM_CONSUMABLE = 0, // 0: 소모품
    ITEM_WEAPON,     // 1: 무기
    ITEM_TOOL,       // 2: 도구
    ITEM_ARMOR,      // 3: 방어구
    ITEM_MATERIAL,   // 4: 제작 재료
    ITEM_MISC        // 5: 소모품
} ItemType;

typedef struct Item_Info {
    int index; //아이템 인덱스
    char name[32]; //아이템 이름
    int maxStack; //아이템 최대갯수(장비는 1로 고정)
    ItemType type; // 아이템 종류
    int BaseDurability; // 아이템 내구도

    int toolkind;
    int materialTier;
    int blockID;
    bool isplaceable;
}Item_Info;

typedef struct ItemDB {
    Item_Info* item;
    size_t count;
}ItemDB;

extern ItemDB g_db;

void InitItemDB(ItemDB* db);
int AddItemToDB(ItemDB* db, int index, const char* name, int maxStack, int type, int baseDurability, int toolkind, int materialTier, int isplaceable, int blockID);
void PrintItemDB(const ItemDB* db);
void FreeItemDB(ItemDB* db);
void SaveItemDBToCSV(const ItemDB* db, const char* filename);
void LoadItemDBFromCSV(ItemDB* db, const char* filename);
Item_Info* FindItemByIndex(const ItemDB* db, int index);
void CallItemDB(ItemDB* db);