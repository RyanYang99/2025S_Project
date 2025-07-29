#include "leak.h"
#include "Inventory.h"

#include <conio.h>

Inventory* g_inv;
ItemDB* g_db;

void gotoxy(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, pos);
}

void InitInventory(Inventory* inv) {
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

void PrintItemDetails(const Player_Item* pItem, const ItemDB* db) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    //이전에 있던 글 제거 밑 공백으로 덮어씌우기
    printf("                                                                                \r");

    if (pItem->Item_Index == 0) {
        // 빈 칸일 경우 아무것도 출력하지 않고 함수 종료
        return;
    }

    Item_Info* itemInfo = FindItemByIndex(db, pItem->Item_Index);
    if (!itemInfo) return;

    // 이제 새로운 내용을 출력합니다.
    printf("  > ");
    SetConsoleTextAttribute(hConsole, COLOR_INFO);
    printf("%s", itemInfo->name);
    if (pItem->isEquipped) {
        SetConsoleTextAttribute(hConsole, COLOR_EQUIPPED);
        printf(" (장착중)");
    }
    SetConsoleTextAttribute(hConsole, COLOR_INFO);
    printf(": ");
    switch (itemInfo->type) {
    case ITEM_CONSUMABLE: printf("사용 시 효과가 발동됩니다."); break;
    case ITEM_WEAPON: case ITEM_TOOL: case ITEM_ARMOR:
        printf("내구도: %d/%d", pItem->durability, itemInfo->BaseDurability); break;
    case ITEM_MATERIAL: printf("제작에 사용되는 재료입니다."); break;
    case ITEM_MISC: printf("특별한 용도가 있는 기타 아이템입니다."); break;
    }
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
}

void RedrawLine(const Inventory* inv, const ItemDB* db, int y, int slotIndex, bool isSelected, bool isBlinkOn) {
    gotoxy(0, y); // 해당 줄의 시작으로 커서 이동

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    const Player_Item* pItem = &inv->item[slotIndex];

    // 선택 여부에 따른 색상 및 커서 마커(>) 설정
    if (isSelected) {
        SetConsoleTextAttribute(hConsole, isBlinkOn ? COLOR_SELECT_BRIGHT : COLOR_SELECT_DARK);
        printf(" > ");
    }
    else {
        SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
        printf("   ");
    }

    // 아이템 정보 출력
    if (pItem->Item_Index != 0) {
        Item_Info* itemInfo = FindItemByIndex(db, pItem->Item_Index);
        if (!itemInfo) return; // DB에 없으면 그냥 넘어감

        printf("[%s]", itemInfo->name);
        if (itemInfo->maxStack > 1) printf(" (x%d)", pItem->quantity);
        if (pItem->isEquipped) {
            SetConsoleTextAttribute(hConsole, COLOR_EQUIPPED);
            printf(" [E]");
        }
    }
    else {
        printf("[ 비어있음 ]");
    }

    // 이전에 있던 글자 찌꺼기를 지우기 위해 줄 끝까지 공백으로 덮어씀
    printf("                                        ");
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
}


void DrawInventory(const Inventory* inv, const ItemDB* db, int currentPage, int totalPages) {

    // 헤더 및 페이지 정보 출력
    gotoxy(0, 0);
    printf("================= 인벤토리 (%d/%d 페이지) =================\n", currentPage + 1, totalPages);

    // 아이템 목록 전체 그리기
    int start_index_inv = currentPage * ITEMS_PER_PAGE;
    for (int i = 0; i < ITEMS_PER_PAGE; ++i) {
        int y = 1 + i; // 헤더가 1줄 있으므로 y좌표는 1부터 시작
        int slotIndex = start_index_inv + i;
        // 선택되지 않은 상태(isSelected=false)로 모든 줄을 그림
        RedrawLine(inv, db, y, slotIndex, false, false);
    }

    // 하단 메뉴 출력
    gotoxy(0, 1 + ITEMS_PER_PAGE + 2); // 목록 아래에 그리기
    printf("\n=========================================================\n");
    printf("  (W/S: 이동, A/D: 페이지, E: 사용/장착, I/ESC: 닫기)\n");
}

void ShowInventory(Inventory* playerInventory, ItemDB* db) {
    int currentPage = 0;
    int selectedIndexOnPage = 0;
    int totalPages = (INVENTORY_SIZE - 1) / ITEMS_PER_PAGE + 1;

    clock_t last_time = clock();
    bool isBlinkOn = true;
    int oldSelectionOnPage = -1; // 이전 선택 위치를 추적하기 위한 변수

    // 1. 인벤토리 진입 시 화면을 딱 한 번만 지우고 전체를 그림
    system("cls");
    DrawInventory(playerInventory, db, currentPage, totalPages);

    while (1) {
        // --- 점멸 효과 또는 키 입력에 따른 업데이트 ---
        // 이전 선택 위치와 현재 선택 위치가 다르거나, 점멸 상태가 바뀌었을 때만 그림
        if (oldSelectionOnPage != selectedIndexOnPage || (double)(clock() - last_time) / CLOCKS_PER_SEC > 0.4) {
            if ((double)(clock() - last_time) / CLOCKS_PER_SEC > 0.4) {
                isBlinkOn = !isBlinkOn;
                last_time = clock();
            }

            // 2. 이전에 선택됐던 줄을 일반 상태로 되돌림
            if (oldSelectionOnPage != -1) {
                int oldSlotIndex = currentPage * ITEMS_PER_PAGE + oldSelectionOnPage;
                RedrawLine(playerInventory, db, 1 + oldSelectionOnPage, oldSlotIndex, false, false);
            }

            // 3. 새로 선택된 줄을 강조해서 그림
            int currentSlotIndex = currentPage * ITEMS_PER_PAGE + selectedIndexOnPage;
            RedrawLine(playerInventory, db, 1 + selectedIndexOnPage, currentSlotIndex, true, isBlinkOn);

            // 4. 아이템 상세 정보 창 업데이트
            gotoxy(0, 1 + ITEMS_PER_PAGE); // 목록 바로 아래로 커서 이동
            PrintItemDetails(&playerInventory->item[currentSlotIndex], db);
            oldSelectionOnPage = selectedIndexOnPage; // 현재 선택 위치를 '이전 위치'로 기록
        }

        // --- 사용자 키 입력 처리 ---
        if (_kbhit()) {
            char input = (char)_getch();
            switch (input) {
            case 'w': case 'W': if (selectedIndexOnPage > 0) selectedIndexOnPage--; break;
            case 's': case 'S': if (selectedIndexOnPage < ITEMS_PER_PAGE - 1) selectedIndexOnPage++; break;

            case 'a': case 'A': case 'd': case 'D':
                if (input == 'a' || input == 'A') {
                    if (currentPage > 0) currentPage--;
                }
                else {
                    if (currentPage < totalPages - 1) currentPage++;
                }
                selectedIndexOnPage = 0;
                oldSelectionOnPage = -1;
                DrawInventory(playerInventory, db, currentPage, totalPages); // 페이지가 바뀌면 전체를 다시 그림
                break;

            case 'e': case 'E':
                oldSelectionOnPage = -1; // 상호작용 후 상태가 바뀌었을 수 있으므로 강제 업데이트
                DrawInventory(playerInventory, db, currentPage, totalPages);
                break;

            case 'i': case 'I': case 27:
                system("cls");
                return;
            }
        }
        else {
            Sleep(50); // 키 입력이 없으면 잠시 대기
        }
    }
}

//I키 입력시 인벤토리 호출
void HandleInventoryKeyInput()
{
    char input;
    if (_kbhit()) {
        input = (char)_getch();
        switch (input) {
        case 'i': case 'I':
            ShowInventory(g_inv, g_db);
            break;
        }
    }
}