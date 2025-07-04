#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"

#define INVENTORY_SIZE 40

typedef struct Item_Info {
	int index; //아이템 인덱스
	char name[32]; //아이템 이름
	int maxStack; //아이템 최대갯수(장비는 1로 고정)
	int type; // 아이템 종류
	int BaseDurability; // 아이템 내구도
}Item_Info;

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