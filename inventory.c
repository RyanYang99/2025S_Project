#include "Inventory.h"

Inventory* g_inv;
ItemDB* g_db;

void gotoxy(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hConsole, pos);
}

void InitInventory(Inventory* inv) {
    for (int i = 0; i < INVENTORY_SIZE; ++i) {
        inv->item[i].Item_Index = 0; // 0�� �� ĭ�� �ǹ�
        inv->item[i].quantity = 0;
        inv->item[i].durability = 0;
        inv->item[i].isEquipped = false;
    }
}

// �κ��丮�� �������� �߰��ϴ� �Լ�
// (�̹� �ִ� �������̸� ���� �߰�, ������ �� ĭ�� �߰�)
bool AddItemToInventory(Inventory* inv, ItemDB* db, int itemIndex, int quantityToAdd) {
    Item_Info* itemInfo = FindItemByIndex(db, itemIndex);
    assert(itemInfo != NULL && "DB�� �������� �ʴ� �������� �߰��Ϸ��� �õ��߽��ϴ�!");

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
            printf("�κ��丮�� ���� ���� ������ [%s] %d���� ���� ���� �� �����ϴ�.\n", itemInfo->name, quantityToAdd);
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

    //������ �ִ� �� ���� �� �������� ������
    printf("                                                                                \r");

    if (pItem->Item_Index == 0) {
        // �� ĭ�� ��� �ƹ��͵� ������� �ʰ� �Լ� ����
        return;
    }

    Item_Info* itemInfo = FindItemByIndex(db, pItem->Item_Index);
    if (!itemInfo) return;

    // ���� ���ο� ������ ����մϴ�.
    printf("  > ");
    SetConsoleTextAttribute(hConsole, COLOR_INFO);
    printf("%s", itemInfo->name);
    if (pItem->isEquipped) {
        SetConsoleTextAttribute(hConsole, COLOR_EQUIPPED);
        printf(" (������)");
    }
    SetConsoleTextAttribute(hConsole, COLOR_INFO);
    printf(": ");
    switch (itemInfo->type) {
    case ITEM_CONSUMABLE: printf("��� �� ȿ���� �ߵ��˴ϴ�."); break;
    case ITEM_WEAPON: case ITEM_TOOL: case ITEM_ARMOR:
        printf("������: %d/%d", pItem->durability, itemInfo->BaseDurability); break;
    case ITEM_MATERIAL: printf("���ۿ� ���Ǵ� ����Դϴ�."); break;
    case ITEM_MISC: printf("Ư���� �뵵�� �ִ� ��Ÿ �������Դϴ�."); break;
    }
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
}

void RedrawLine(const Inventory* inv, const ItemDB* db, int y, int slotIndex, bool isSelected, bool isBlinkOn) {
    gotoxy(0, y); // �ش� ���� �������� Ŀ�� �̵�

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    const Player_Item* pItem = &inv->item[slotIndex];

    // ���� ���ο� ���� ���� �� Ŀ�� ��Ŀ(>) ����
    if (isSelected) {
        SetConsoleTextAttribute(hConsole, isBlinkOn ? COLOR_SELECT_BRIGHT : COLOR_SELECT_DARK);
        printf(" > ");
    }
    else {
        SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
        printf("   ");
    }

    // ������ ���� ���
    if (pItem->Item_Index != 0) {
        Item_Info* itemInfo = FindItemByIndex(db, pItem->Item_Index);
        if (!itemInfo) return; // DB�� ������ �׳� �Ѿ

        printf("[%s]", itemInfo->name);
        if (itemInfo->maxStack > 1) printf(" (x%d)", pItem->quantity);
        if (pItem->isEquipped) {
            SetConsoleTextAttribute(hConsole, COLOR_EQUIPPED);
            printf(" [E]");
        }
    }
    else {
        printf("[ ������� ]");
    }

    // ������ �ִ� ���� ��⸦ ����� ���� �� ������ �������� ���
    printf("                                        ");
    SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
}


void DrawInventory(const Inventory* inv, const ItemDB* db, int currentPage, int totalPages) {

    // ��� �� ������ ���� ���
    gotoxy(0, 0);
    printf("================= �κ��丮 (%d/%d ������) =================\n", currentPage + 1, totalPages);

    // ������ ��� ��ü �׸���
    int start_index_inv = currentPage * ITEMS_PER_PAGE;
    for (int i = 0; i < ITEMS_PER_PAGE; ++i) {
        int y = 1 + i; // ����� 1�� �����Ƿ� y��ǥ�� 1���� ����
        int slotIndex = start_index_inv + i;
        // ���õ��� ���� ����(isSelected=false)�� ��� ���� �׸�
        RedrawLine(inv, db, y, slotIndex, false, false);
    }

    // �ϴ� �޴� ���
    gotoxy(0, 1 + ITEMS_PER_PAGE + 2); // ��� �Ʒ��� �׸���
    printf("\n=========================================================\n");
    printf("  (W/S: �̵�, A/D: ������, E: ���/����, I/ESC: �ݱ�)\n");
}

void ShowInventory(Inventory* playerInventory, ItemDB* db) {
    int currentPage = 0;
    int selectedIndexOnPage = 0;
    int totalPages = (INVENTORY_SIZE - 1) / ITEMS_PER_PAGE + 1;

    clock_t last_time = clock();
    bool isBlinkOn = true;
    int oldSelectionOnPage = -1; // ���� ���� ��ġ�� �����ϱ� ���� ����

    // 1. �κ��丮 ���� �� ȭ���� �� �� ���� ����� ��ü�� �׸�
    system("cls");
    DrawInventory(playerInventory, db, currentPage, totalPages);

    while (1) {
        // --- ���� ȿ�� �Ǵ� Ű �Է¿� ���� ������Ʈ ---
        // ���� ���� ��ġ�� ���� ���� ��ġ�� �ٸ��ų�, ���� ���°� �ٲ���� ���� �׸�
        if (oldSelectionOnPage != selectedIndexOnPage || (double)(clock() - last_time) / CLOCKS_PER_SEC > 0.4) {
            if ((double)(clock() - last_time) / CLOCKS_PER_SEC > 0.4) {
                isBlinkOn = !isBlinkOn;
                last_time = clock();
            }

            // 2. ������ ���õƴ� ���� �Ϲ� ���·� �ǵ���
            if (oldSelectionOnPage != -1) {
                int oldSlotIndex = currentPage * ITEMS_PER_PAGE + oldSelectionOnPage;
                RedrawLine(playerInventory, db, 1 + oldSelectionOnPage, oldSlotIndex, false, false);
            }

            // 3. ���� ���õ� ���� �����ؼ� �׸�
            int currentSlotIndex = currentPage * ITEMS_PER_PAGE + selectedIndexOnPage;
            RedrawLine(playerInventory, db, 1 + selectedIndexOnPage, currentSlotIndex, true, isBlinkOn);

            // 4. ������ �� ���� â ������Ʈ
            gotoxy(0, 1 + ITEMS_PER_PAGE); // ��� �ٷ� �Ʒ��� Ŀ�� �̵�
            PrintItemDetails(&playerInventory->item[currentSlotIndex], db);
            oldSelectionOnPage = selectedIndexOnPage; // ���� ���� ��ġ�� '���� ��ġ'�� ���
        }

        // --- ����� Ű �Է� ó�� ---
        if (_kbhit()) {
            char input = _getch();
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
                DrawInventory(playerInventory, db, currentPage, totalPages); // �������� �ٲ�� ��ü�� �ٽ� �׸�
                break;

            case 'e': case 'E':
                oldSelectionOnPage = -1; // ��ȣ�ۿ� �� ���°� �ٲ���� �� �����Ƿ� ���� ������Ʈ
                DrawInventory(playerInventory, db, currentPage, totalPages);
                break;

            case 'i': case 'I': case 27:
                system("cls");
                return;
            }
        }
        else {
            Sleep(50); // Ű �Է��� ������ ��� ���
        }
    }
}

//IŰ �Է½� �κ��丮 ȣ��
void HandleInventoryKeyInput()
{
    char input;
    if (_kbhit()) {
        input = _getch();
        switch (input) {
        case 'i': case 'I':
            ShowInventory(g_inv, g_db);
            break;
        }
    }
}