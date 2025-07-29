#include "ItemDB.h"

Item_Info* FindItemByIndex(const ItemDB* db, int index) {
    for (size_t i = 0; i < db->count; ++i) {
        if (db->item[i].index == index) {
            return &db->item[i]; // �������� ã�����Ƿ� �ּҸ� ��ȯ
        }
    }
    return NULL; // ������ ã�Ƶ� ������ NULL ��ȯ
}

void InitItemDB(ItemDB* db) {
    db->item = NULL;
    db->count = 0;
}

int AddItemToDB(ItemDB* db, int index, const char* name, int maxStack, int type, int baseDurability, int toolkind, int materialTier, int isplaceable, int blockID) {
    // ũ�� ������ ���� ���Ҵ�
    Item_Info* resized = realloc(db->item, sizeof(Item_Info) * (db->count + 1));
    if (resized == NULL) {
        return -1; // �޸� ����
    }

    db->item = resized;

    // ������ ����
    Item_Info* item = &db->item[db->count];
    item->index = index;
    strncpy(item->name, name, sizeof(item->name) - 1);
    item->name[sizeof(item->name) - 1] = '\0'; // �� ���� ����
    item->maxStack = maxStack;
    item->type = type;
    item->BaseDurability = baseDurability;
    item->toolkind = toolkind;
    item->materialTier = materialTier;
    item->isplaceable = isplaceable;
    item->blockID = blockID;

    db->count++;
    return 0;
}

void InputItemFromUser(ItemDB* db) {
    int index, maxStack, type, baseDurability, toolkind, materialTier, isplaceable, blockID;
    char name[32];

    printf("�� �������� �߰��մϴ�.\n");

    // ===== �ε��� �Է� �� �ߺ� �˻� ���� (������ �κ�) =====
    while (1) {
        printf("�ε��� : ");

        // ����ڰ� ���ڰ� �ƴ� ���� �Է��ϴ� ��쿡 ���� ��� �ڵ�
        if (scanf("%d", &index) != 1) {
            printf("����: ��ȿ�� ���ڸ� �Է����ּ���.\n");
            // �Է� ���۸� ��� ���� ���� ����
            while (getchar() != '\n');
            continue; // ������ ó������ ���ư�
        }

        getchar(); // scanf�� ���� ���� ���� ����

        // FindItemByIndex �Լ��� �ߺ� �˻�
        if (FindItemByIndex(db, index) != NULL) {
            printf("����: �̹� ��� ���� �ε���(%d)�Դϴ�. �ٸ� �ε����� �Է����ּ���.\n\n", index);
        }
        else {
            // �ߺ����� ���� ��ȿ�� �ε����̹Ƿ� ���� Ż��
            break;
        }
    }
    // =======================================================

    printf("�̸� : ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0'; // ���� ���� ����

    printf("�ִ� ���� : ");
    scanf("%d", &maxStack);

    printf("Ÿ�� (0:�Ҹ�ǰ, 1:����, 2:����, 3:��, 4:���, 5:��Ÿ) : ");
    scanf("%d", &type);

    printf("�⺻ ������ : ");
    scanf("%d", &baseDurability);

    if (type == 2)
    {
        printf("���� ���� (0:���, 1:����, 2:��): ");
        scanf("%d", &toolkind);

    }
    else
        toolkind = -1;

    if (toolkind != -1)
    {
        printf("��� Ƽ�� (1:����, 2:��, 3:ö): ");
        scanf("%d", &materialTier);
    }
    else
        materialTier = -1;

    printf("��ġ ���� ���� (1:��ġ ����, 0:��ġ �Ұ�): ");
    scanf("%d", &isplaceable);

    if (isplaceable)
    {
        printf("��ġ�� ��� ID (block_t enum��ȣ): ");
        scanf("%d", &blockID);
    }
    else
        blockID = -1;

    if (AddItemToDB(db, index, name, maxStack, type, baseDurability, toolkind, materialTier, isplaceable, blockID) == 0) {
        printf("�߰� �Ϸ�!\n\n");
    }
    else {
        printf("������ �߰� ���� (�޸� ����)\n");
    }
}

void PrintItemDB(const ItemDB* db) {
    printf("=== Item Database (%zu items) ===\n", db->count);
    for (size_t i = 0; i < db->count; ++i) {
        printf("[%d] �̸� : %s | �ִ� ����: %d | ������ ���� : %d | ������: %d\n",
            db->item[i].index,
            db->item[i].name,
            db->item[i].maxStack,
            db->item[i].type,
            db->item[i].BaseDurability,
            db->item[i].toolkind,
            db->item[i].materialTier,
            db->item[i].isplaceable,
            db->item[i].blockID);
    }
    printf("\n");
}

void FreeItemDB(ItemDB* db) {
    free(db->item);
    db->item = NULL;
    db->count = 0;
}

void SaveItemDBToCSV(const ItemDB* db, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("���� ���� ����");
        return;
    }

    // UTF-8 BOM �߰� (������ �̰� ���� �ѱ� �ν� ����)
    unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    fwrite(bom, sizeof(bom), 1, fp);

    // ���
    fprintf(fp, "index,name,maxStack,type,baseDurability,toolkind,materialTier,isplaceable,blockID\n");

    // ����
    for (size_t i = 0; i < db->count; ++i) {
        const Item_Info* item = &db->item[i];
        fprintf(fp, "%d,%s,%d,%d,%d,%d,%d,%d,%d\n",
            item->index,
            item->name,
            item->maxStack,
            item->type,
            item->BaseDurability,
            item->toolkind,
            item->materialTier,
            item->isplaceable,
            item->blockID);
    }

    fclose(fp);
}

void LoadItemDBFromCSV(ItemDB* db, const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("���� �ҷ����� ����");
        return;
    }

    char line[128];
    fgets(line, sizeof(line), fp); // ��� ��ŵ

    while (fgets(line, sizeof(line), fp)) {
        int index, maxStack, type, baseDurability, toolkind, materialTier, isplaceable, blockID;
        char name[32];

        // ���ڿ� �Ľ� (CSV ����: ����,���ڿ�,����,����,����)
        if (sscanf(line, "%d,%31[^,],%d,%d,%d,%d,%d,%d,%d",
            &index, name, &maxStack, &type, &baseDurability, &toolkind, &materialTier, &isplaceable, &blockID) == 9) {
            AddItemToDB(db, index, name, maxStack, type, baseDurability, toolkind, materialTier, isplaceable, blockID);
            printf("\n");
        }
    }

    fclose(fp);
}

void SelectMode(ItemDB* db) // main���κ��� ���� �ּ�(������) db
{
    int mode = -1;

    // ���� ����. case 0�� 'return'���� Ż��.
    while (1)
    {
        printf("===========================================\n");
        printf("��带 �����ϼ��� (1: ������ �߰�, 2: DB ��� ����, 0: ���� �� ����)\n");
        printf("�Է�: ");

        if (scanf("%d", &mode) != 1) {
            printf("����: ��ȿ�� ���ڸ� �Է����ּ���.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        printf("\n");

        switch (mode)
        {
        case 1:
            // ������ ���� db�� �״�� ����
            InputItemFromUser(db);
            break;
        case 2:
            // ������ ���� db�� �״�� ����
            PrintItemDB(db);
            break;
        case 0:
            printf("��� ������ �����մϴ�.\n");
            // void �Լ��̹Ƿ� �� ���� return;
            return;
        default:
            printf("����: 0, 1, 2 �߿��� �������ּ���.\n");
            break;
        }
    }
}

void CallItemDB(ItemDB* db)
{
    // UTF-8 ȯ�� ����
    //setlocale(LC_ALL, ".UTF8");
    //SetConsoleOutputCP(CP_UTF8);
    //SetConsoleCP(CP_UTF8);

    // 1. ���α׷� ���� �� DB �ʱ�ȭ �� ������ �ε� (�� �� ��!)
    InitItemDB(db); // main�� db ���� �ּҸ� �ѱ�
    printf("���� ������ ������ 'items.csv'���� �ҷ��ɴϴ�...\n");
    LoadItemDBFromCSV(db, "items.csv");
    printf("�ε� �Ϸ�! ���� ������ ��: %zu\n\n", db->count);

    // 2. ����� ��û ó�� ���� ����
    SelectMode(db);

    // 3. ������ ������ ���α׷��� ����Ǳ� ����, ���� �����͸� ���� (�� �� ��!)
    printf("\n����� ������ ������ 'items.csv'�� �����մϴ�.\n");
    SaveItemDBToCSV(db, "items.csv");

    // 4. �޸� ����
    FreeItemDB(db);

    printf("���α׷��� �����մϴ�.\n");
}