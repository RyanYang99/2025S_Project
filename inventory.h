#pragma once


#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "ItemDB.h"
#include "time.h"

#define ITEMS_PER_PAGE 10   // �� �������� ������ ������ ��
#define MAX_PAGES 5         // �ִ� ������ ��
#define INVENTORY_SIZE (ITEMS_PER_PAGE * MAX_PAGES) // �� �κ��丮 ĭ ��

#define COLOR_DEFAULT 7
#define COLOR_SELECT_DARK 8
#define COLOR_SELECT_BRIGHT 15
#define COLOR_INFO 14
#define COLOR_EQUIPPED 10

typedef struct Player_Item {
	int Item_Index;  //DB�� �����ϴ� index
	int quantity; //���� ����
	int durability; //���� ������
	bool isEquipped; //��������   
}Player_Item;

typedef struct Inventory {
	Player_Item item[INVENTORY_SIZE];
	int selectedIndex; //�κ��丮 ���� ���� �� ĭ
}Inventory;

void InitInventory(Inventory* inv);
bool AddItemToInventory(Inventory* inv, ItemDB* db, int itemIndex, int quantity);
void ShowInventory(Inventory* playerInventory, ItemDB* db);
void gotoxy(int x, int y);
void HandleInventoryKeyInput();