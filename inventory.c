#include "leak.h"
#include "Inventory.h"

#include <conio.h>
#include "input.h"
#include "console.h"
#include "formatter.h"

#define INVENTORY_BACKGROUND BACKGROUND_T_BLACK
#define INVENTORY_FOREGROUND FOREGROUND_T_WHITE
#define FOREGROUND_EQUIPPED FOREGROUND_T_DARKBLUE

Inventory g_inv = { 0 };

bool isInventoryOpen = false;
int current_selection_index = 0,
    max_selection_index = ITEMS_PER_PAGE - 1,
    current_page_index = 0,
    max_page_index = MAX_PAGES - 1;

void InitInventory(Inventory *inv) {
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        inv->item[i].Item_Index = 0; // 0은 빈 칸을 의미
        inv->item[i].quantity = 0;
        inv->item[i].durability = 0;
        inv->item[i].isEquipped = false;
    }
}

// 인벤토리에 아이템을 추가하는 함수
// (이미 있는 아이템이면 개수 추가, 없으면 빈 칸에 추가)
bool AddItemToInventory(Inventory* inv, ItemDB* db, int itemIndex, int quantityToAdd) {
    Item_Info* itemInfo = FindItemByIndex(db, itemIndex);
    assert(itemInfo != NULL && "DB에 존재하지 않는 아이템을 추가하려고 시도했습니다!");

    while (quantityToAdd > 0) {
        bool stacked = false;
        if (itemInfo->maxStack > 1) {
            for (int i = 0; i < INVENTORY_SIZE; ++i) {
                if (inv->item[i].Item_Index == itemIndex && inv->item[i].quantity < itemInfo->maxStack) { // items -> item (x2)
                    int freeSpace = itemInfo->maxStack - inv->item[i].quantity; // items -> item
                    if (quantityToAdd <= freeSpace) {
                        inv->item[i].quantity += quantityToAdd; // items -> item
                        quantityToAdd = 0;
                    }
                    else {
                        inv->item[i].quantity = itemInfo->maxStack; // items -> item
                        quantityToAdd -= freeSpace;
                    }
                    if (quantityToAdd == 0) {
                        stacked = true;
                        break;
                    }
                }
            }
        }
        if (stacked) break;

        int emptySlot = -1;
        for (int i = 0; i < INVENTORY_SIZE; ++i) {
            if (inv->item[i].Item_Index == 0) { // items -> item
                emptySlot = i;
                break;
            }
        }

        if (emptySlot == -1) {
            printf("인벤토리가 가득 차서 아이템 [%s] %d개를 더는 얻을 수 없습니다.\n", itemInfo->name, quantityToAdd);
            return false;
        }

        inv->item[emptySlot].Item_Index = itemIndex; // items -> item
        inv->item[emptySlot].durability = itemInfo->BaseDurability; // items -> item
        inv->item[emptySlot].isEquipped = false; // items -> item

        if (quantityToAdd <= itemInfo->maxStack) {
            inv->item[emptySlot].quantity = quantityToAdd; // items -> item
            quantityToAdd = 0;
        }
        else {
            inv->item[emptySlot].quantity = itemInfo->maxStack; // items -> item
            quantityToAdd -= itemInfo->maxStack;
        }
    }
    return true;
}

static void render_inventory_item(const Inventory * const pInventory,
                                  const ItemDB * const pDB,
                                  const int y,
                                  const int inventory_index,
                                  const bool selected)
{
    const Player_Item *pItem = &pInventory->item[inventory_index];
    COORD position =
    {
        .Y = (SHORT)y
    };

    if (selected)
        position.X += (SHORT)fprint_string("> ", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND);

    if (pItem->Item_Index)
    {
        const Item_Info *pItem_info = FindItemByIndex(pDB, pItem->Item_Index);
        if (!pItem_info)
            return;

        position.X += (SHORT)fprint_string("[ %s", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND, pItem_info->name);

        if (pItem_info->maxStack > 1)
            position.X += (SHORT)fprint_string(" (x%d) ]", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND, pItem->quantity);

        if (pItem_info->type == ITEM_WEAPON || pItem_info->type == ITEM_TOOL || pItem_info->type == ITEM_ARMOR)
            position.X += (SHORT)fprint_string(" (내구성: %d/%d) ]", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND, pItem->durability, pItem_info->BaseDurability);

        if (pItem->isEquipped)
            position.X += (SHORT)fprint_string(" [E] ", position, INVENTORY_BACKGROUND, FOREGROUND_EQUIPPED);

    }
    else
        fprint_string("[ 비어있음 ]", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND);
}

void RenderInventory(Inventory* inv, ItemDB* db) {
    if (!isInventoryOpen)
        return;

    COORD position = { 0 };
    fprint_string("=== 인벤토리 (%d / %d) ===", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND, current_page_index + 1, MAX_PAGES);

    const int start_index = current_page_index * ITEMS_PER_PAGE;
    for (int i = 0; i < ITEMS_PER_PAGE; ++i)
    {
        render_inventory_item(inv, db, ++position.Y, start_index + i, i == current_selection_index);
    }

    ++position.Y;
    fprint_string("=== (W / S: 선택, A / D: 페이지, E: 사용 / 장착, I: 닫기) ===", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND);

    const Player_Item *pItem = &inv->item[start_index + current_selection_index];
    if (!pItem->Item_Index)
        return;

    const Item_Info *pItem_info = FindItemByIndex(db, pItem->Item_Index);
    position.Y += 2;
    position.X += (SHORT)fprint_string("%s", position, INVENTORY_BACKGROUND, INVENTORY_FOREGROUND, pItem_info->name);

    if (pItem->isEquipped)
        position.X += (SHORT)fprint_string(" (장착중)", position, INVENTORY_BACKGROUND, FOREGROUND_EQUIPPED);

    char *pDescription = "";
    switch (pItem_info->type) {
        case ITEM_CONSUMABLE:
            pDescription = ": 사용 시 효과가 발동됩니다.";
            break;

        case ITEM_MATERIAL:
            pDescription = ": 제작에 사용되는 재료입니다.";
            break;

        case ITEM_MISC:
            pDescription = ": 특별한 용도가 있는 기타 아이템입니다.";
            break;
    }

    fprint_string("%s", position, INVENTORY_BACKGROUND, FOREGROUND_T_YELLOW, pDescription);
}

//I키 입력시 인벤토리 호출
void HandleInventoryKeyInput()
{
    if (!keyboard_pressed)
        return;

    const char character = (char)tolower(input_character);
    if (character == 'i')
        isInventoryOpen = !isInventoryOpen;

    if (!isInventoryOpen)
        return;

    if (character == 'w' && current_selection_index > 0)
        --current_selection_index;
    else if (character == 's' && current_selection_index < max_selection_index)
        ++current_selection_index;
    else if (character == 'a' && current_page_index > 0)
        --current_page_index;
    else if (character == 'd' && current_page_index < max_page_index)
        ++current_page_index;
    else if (character == 'e')
    {
        const Player_Item *pItem = &g_inv.item[(current_page_index * ITEMS_PER_PAGE) + current_selection_index];
        if (!pItem->Item_Index)
            return;

        const Item_Info *pItem_info = FindItemByIndex(&g_db, pItem->Item_Index);
        if (!pItem_info)
            return;

        //사용 / 장착 처리
    }
}