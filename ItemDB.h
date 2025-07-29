#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "windows.h"
#include "assert.h"
#include <locale.h> 
#include "stdbool.h"

typedef enum ItemType {
	ITEM_CONSUMABLE = 0, // 0: �Ҹ�ǰ
	ITEM_WEAPON,     // 1: ����
	ITEM_TOOL,       // 2: ����
	ITEM_ARMOR,      // 3: ��
	ITEM_MATERIAL,   // 4: ���� ���
	ITEM_MISC        // 5: �Ҹ�ǰ
} ItemType;

typedef struct Item_Info {
	int index; //������ �ε���
	char name[32]; //������ �̸�
	int maxStack; //������ �ִ밹��(���� 1�� ����)
	ItemType type; // ������ ����
	int BaseDurability; // ������ ������

	int toolkind;
	int materialTier;
	int blockID;
	bool isplaceable;
}Item_Info;

typedef struct ItemDB {
	Item_Info* item;
	size_t count;
}ItemDB;

void InitItemDB(ItemDB* db);
int AddItemToDB(ItemDB* db, int index, const char* name, int maxStack, int type, int baseDurability);
void PrintItemDB(const ItemDB* db);
void FreeItemDB(ItemDB* db);
void SaveItemDBToCSV(const ItemDB* db, const char* filename);
void LoadItemDBFromCSV(ItemDB* db, const char* filename);
Item_Info* FindItemByIndex(const ItemDB* db, int index);
void SelectMode();
void CallItemDB(ItemDB* db);