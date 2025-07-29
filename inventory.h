#pragma once


#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "ItemDB.h"
#include "time.h"

#define ITEMS_PER_PAGE 10   // 한 페이지에 보여줄 아이템 수
#define MAX_PAGES 5         // 최대 페이지 수
#define INVENTORY_SIZE (ITEMS_PER_PAGE * MAX_PAGES) // 총 인벤토리 칸 수

#define COLOR_DEFAULT 7
#define COLOR_SELECT_DARK 8
#define COLOR_SELECT_BRIGHT 15
#define COLOR_INFO 14
#define COLOR_EQUIPPED 10

typedef struct Player_Item {
	int Item_Index;  //DB를 참조하는 index
	int quantity; //현재 수량
	int durability; //현재 내구도
	bool isEquipped; //장착여부   
}Player_Item;

typedef struct Inventory {
	Player_Item item[INVENTORY_SIZE];
	int selectedIndex; //인벤토리 현재 선택 된 칸
}Inventory;

void InitInventory(Inventory* inv);
bool AddItemToInventory(Inventory* inv, ItemDB* db, int itemIndex, int quantity);
void ShowInventory(Inventory* playerInventory, ItemDB* db);
void gotoxy(int x, int y);
void HandleInventoryKeyInput();