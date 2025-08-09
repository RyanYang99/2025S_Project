#include "leak.h"
#include "item_database.h"

#include <stdio.h>
#include <locale.h>
#include <string.h>

#include <Windows.h>

item_database_t database = { 0 };

static const char pFile_name[] = "items.csv";

static const bool database_add_item(const int index,
                                    const char * const pName,
                                    const int max_stack,
                                    const item_type_t type,
                                    const int base_durability,
                                    const tool_kind_t tool_kind,
                                    const material_tier_t material_tier,
                                    const bool is_placeable) {
    // 크기 증가를 위한 재할당
    item_information_t *pResized = realloc(database.pItem_information, sizeof(item_information_t) * (database.count + 1));
    if (!pResized) {
        return false; //메모리 부족
    }

    database.pItem_information = pResized;

    //아이템 설정
    item_information_t *pItem_information = &database.pItem_information[database.count];
    pItem_information->index = index;

    const size_t name_length = sizeof(pItem_information->name) - 1;
    strncpy_s(pItem_information->name, sizeof(char) * name_length, pName, name_length);
    pItem_information->name[name_length] = '\0'; //널 종결 보장

    pItem_information->max_stack = max_stack;
    pItem_information->type = type;
    pItem_information->base_durability = base_durability;
    pItem_information->tool_kind = tool_kind;
    pItem_information->material_tier = material_tier;
    pItem_information->is_placeable = is_placeable;

    ++database.count;
    return true;
}

static void database_load(void) {
    FILE *pFile = fopen(pFile_name, "r");
    if (!pFile) {
        puts("Failed to load file.");
        return;
    }

    char line[128] = { 0 };
    fgets(line, sizeof(line), pFile); //헤더 스킵

    while (fgets(line, sizeof(line), pFile)) {
        int index = 0, max_stack = 0, type = 0, base_durability = 0, tool_kind = 0, material_tier = 0, is_placeable = false;
        char name[32] = { 0 };

        //문자열 파싱 (CSV 형식: 정수,문자열,정수,정수,정수)
        if (sscanf_s(line, "%d,%31[^,],%d,%d,%d,%d,%d,%d",
            &index, name, (unsigned int)sizeof(name), &max_stack, &type, &base_durability, &tool_kind, &material_tier, &is_placeable) == 8)
            database_add_item(index, name, max_stack, type, base_durability, tool_kind, material_tier, is_placeable);
    }

    fclose(pFile);
}

static void clear_input(void) {
    while (getchar() != '\n');
}

static void database_add_item_from_user(void) {
    int index = 0;

    printf("Adding a new item.\n");

    //===== 인덱스 입력 및 중복 검사 루프 (수정된 부분) =====
    while (true) {
        printf("Index : ");

        //사용자가 숫자가 아닌 값을 입력하는 경우에 대한 방어 코드
        if (scanf_s("%d", &index) != 1) {
            printf("Error: Enter a valid number.\n");
            //입력 버퍼를 비워 무한 루프 방지
            clear_input();
            continue; //루프의 처음으로 돌아감
        }

        (void)getchar(); //scanf가 남긴 개행 문자 제거

        //FindItemByIndex 함수로 중복 검사
        if (database_find_item_by_index(index))
            printf("Error: Index %d is already in use.\n", index);
        else 
            break; //중복되지 않은 유효한 인덱스이므로 루프 탈출
    }
    //=======================================================

    char name[32] = { 0 };

    printf("name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0'; //개행 문자 제거

    int max_stack = 0;
    printf("max_stack: ");
    scanf_s("%d", &max_stack);

    int type = 0;
    printf("type (1: Material, 2: Tool, 3: Armor, 4: Misc): ");
    scanf_s("%d", &type);

    int base_durability = 0;
    printf("base_durability: ");
    scanf_s("%d", &base_durability);

    int tool_kind = 0;
    if (type == 2) {
        printf("tool_kind (1: Sword, 2: Pickaxe, 3: Axe, 4: Shovel): ");
        scanf_s("%d", &tool_kind);
    }

    int material_tier = 0;
    if (tool_kind) {
        printf("material_tier (1: Wood, 2: Stone, 3: Iron): ");
        scanf_s("%d", &material_tier);
    }

    int is_placeable = false;
    printf("is_placeable (0: false, 1: true): ");
    scanf_s("%d", &is_placeable);

    if (database_add_item(index, name, max_stack, type, base_durability, tool_kind, material_tier, is_placeable))
        printf("Item added.\n");
    else
        printf("Failed to add an item.\n");
}

static void database_print(void) {
    printf("=== Item Database (%zu Items) ===\n", database.count);
    for (size_t i = 0; i < database.count; ++i)
        printf("[%d] name: %s | max_stack: %d | type: %d | base_durability: %d | tool_kind: %d | material_tier: %d | is_placeable: %d\n",
               database.pItem_information[i].index,
               database.pItem_information[i].name,
               database.pItem_information[i].max_stack,
               database.pItem_information[i].type,
               database.pItem_information[i].base_durability,
               database.pItem_information[i].tool_kind,
               database.pItem_information[i].material_tier,
               database.pItem_information[i].is_placeable);
}

static void select_mode(void) {
    int mode = -1;

    //무한 루프. case 0의 'return'으로 탈출.
    while (true) {
        printf("=== Database Editor ===\n" \
               "[1]: Add item, [2]: View items, [0]: Save and close\n" \
               "Input: ");

        if (scanf_s("%d", &mode) != 1) {
            printf("Error: Invalid input.\n");
            clear_input();
            continue;
        }

        clear_input();
        printf("\n");

        switch (mode) {
            case 1:
                database_add_item_from_user();
                break;

            case 2:
                database_print();
                break;
        
            case 0:
                printf("Exiting.\n");
                //void 함수이므로 값 없이 return;
                return;

            default:
                printf("Please select a correct option.\n");
                break;
        }
    }
}

static void database_save(void) {
    FILE *pFile = fopen(pFile_name, "w");
    if (!pFile) {
        perror("파일 저장 실패");
        return;
    }

    //UTF-8 BOM 추가 (엑셀이 이걸 봐야 한글 인식 가능)
    const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    fwrite(bom, sizeof(bom), 1, pFile);

    //헤더
    fprintf(pFile, "index,name,max_stack,type,base_durability,tool_kind,material_tier,is_placeable\n");

    //내용
    for (size_t i = 0; i < database.count; ++i) {
        const item_information_t * const pItem_information = &database.pItem_information[i];
        fprintf(pFile,
                "%d,%s,%d,%d,%d,%d,%d,%d\n",
                pItem_information->index,
                pItem_information->name,
                pItem_information->max_stack,
                pItem_information->type,
                pItem_information->base_durability,
                pItem_information->tool_kind,
                pItem_information->material_tier,
                pItem_information->is_placeable);
    }

    fclose(pFile);
}

void database_initialize(const bool edit) {
    setlocale(LC_ALL, ".UTF8");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    //1. 프로그램 시작 시 DB 초기화 및 데이터 로드 (딱 한 번!)
    printf_s("Loading items from %s.\n", pFile_name);
    database_load();
    printf_s("Load complete.\n" \
             "Current item count: %zu\n", database.count);

    if (!edit)
        return;

    //2. 사용자 요청 처리 루프 실행
    select_mode();

    //3. 루프가 끝나고 프로그램이 종료되기 직전, 최종 데이터를 저장 (딱 한 번!)
    printf("Saving changes to %s.\n", pFile_name);
    database_save();
}

void database_destroy(void) {
    free(database.pItem_information);
    database.pItem_information = NULL;
    database.count = 0;
}

item_information_t *database_find_item_by_index(const int index) {
    for (size_t i = 0; i < database.count; ++i)
        if (database.pItem_information[i].index == index)
            return &database.pItem_information[i]; //아이템을 찾았으므로 주소를 반환

    return NULL; //끝까지 찾아도 없으면 NULL 반환
}