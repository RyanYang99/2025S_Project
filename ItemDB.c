#include "ItemDB.h"

Item_Info* FindItemByIndex(const ItemDB* db, int index) {
    for (size_t i = 0; i < db->count; ++i) {
        if (db->item[i].index == index) {
            return &db->item[i]; // 아이템을 찾았으므로 주소를 반환
        }
    }
    return NULL; // 끝까지 찾아도 없으면 NULL 반환
}

void InitItemDB(ItemDB* db) {
    db->item = NULL;
    db->count = 0;
}

int AddItemToDB(ItemDB* db, int index, const char* name, int maxStack, int type, int baseDurability, int toolkind, int materialTier, int isplaceable, int blockID) {
    // 크기 증가를 위한 재할당
    Item_Info* resized = realloc(db->item, sizeof(Item_Info) * (db->count + 1));
    if (resized == NULL) {
        return -1; // 메모리 부족
    }

    db->item = resized;

    // 아이템 설정
    Item_Info* item = &db->item[db->count];
    item->index = index;
    strncpy(item->name, name, sizeof(item->name) - 1);
    item->name[sizeof(item->name) - 1] = '\0'; // 널 종결 보장
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

    printf("새 아이템을 추가합니다.\n");

    // ===== 인덱스 입력 및 중복 검사 루프 (수정된 부분) =====
    while (1) {
        printf("인덱스 : ");

        // 사용자가 숫자가 아닌 값을 입력하는 경우에 대한 방어 코드
        if (scanf("%d", &index) != 1) {
            printf("오류: 유효한 숫자를 입력해주세요.\n");
            // 입력 버퍼를 비워 무한 루프 방지
            while (getchar() != '\n');
            continue; // 루프의 처음으로 돌아감
        }

        getchar(); // scanf가 남긴 개행 문자 제거

        // FindItemByIndex 함수로 중복 검사
        if (FindItemByIndex(db, index) != NULL) {
            printf("오류: 이미 사용 중인 인덱스(%d)입니다. 다른 인덱스를 입력해주세요.\n\n", index);
        }
        else {
            // 중복되지 않은 유효한 인덱스이므로 루프 탈출
            break;
        }
    }
    // =======================================================

    printf("이름 : ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0'; // 개행 문자 제거

    printf("최대 갯수 : ");
    scanf("%d", &maxStack);

    printf("타입 (0:소모품, 1:무기, 2:도구, 3:방어구, 4:재료, 5:기타) : ");
    scanf("%d", &type);

    printf("기본 내구도 : ");
    scanf("%d", &baseDurability);

    if (type == 2)
    {
        printf("도구 종류 (0:곡괭이, 1:도끼, 2:삽): ");
        scanf("%d", &toolkind);

    }
    else
        toolkind = -1;

    if (toolkind != -1)
    {
        printf("재료 티어 (1:나무, 2:돌, 3:철): ");
        scanf("%d", &materialTier);
    }
    else
        materialTier = -1;

    printf("설치 가능 여부 (1:설치 가능, 0:설치 불가): ");
    scanf("%d", &isplaceable);

    if (isplaceable)
    {
        printf("설치할 블록 ID (block_t enum번호): ");
        scanf("%d", &blockID);
    }
    else
        blockID = -1;

    if (AddItemToDB(db, index, name, maxStack, type, baseDurability, toolkind, materialTier, isplaceable, blockID) == 0) {
        printf("추가 완료!\n\n");
    }
    else {
        printf("아이템 추가 실패 (메모리 부족)\n");
    }
}

void PrintItemDB(const ItemDB* db) {
    printf("=== Item Database (%zu items) ===\n", db->count);
    for (size_t i = 0; i < db->count; ++i) {
        printf("[%d] 이름 : %s | 최대 갯수: %d | 아이템 종류 : %d | 내구도: %d\n",
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
        perror("파일 저장 실패");
        return;
    }

    // UTF-8 BOM 추가 (엑셀이 이걸 봐야 한글 인식 가능)
    unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
    fwrite(bom, sizeof(bom), 1, fp);

    // 헤더
    fprintf(fp, "index,name,maxStack,type,baseDurability,toolkind,materialTier,isplaceable,blockID\n");

    // 내용
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
        perror("파일 불러오기 실패");
        return;
    }

    char line[128];
    fgets(line, sizeof(line), fp); // 헤더 스킵

    while (fgets(line, sizeof(line), fp)) {
        int index, maxStack, type, baseDurability, toolkind, materialTier, isplaceable, blockID;
        char name[32];

        // 문자열 파싱 (CSV 형식: 정수,문자열,정수,정수,정수)
        if (sscanf(line, "%d,%31[^,],%d,%d,%d,%d,%d,%d,%d",
            &index, name, &maxStack, &type, &baseDurability, &toolkind, &materialTier, &isplaceable, &blockID) == 9) {
            AddItemToDB(db, index, name, maxStack, type, baseDurability, toolkind, materialTier, isplaceable, blockID);
            printf("\n");
        }
    }

    fclose(fp);
}

void SelectMode(ItemDB* db) // main으로부터 받은 주소(포인터) db
{
    int mode = -1;

    // 무한 루프. case 0의 'return'으로 탈출.
    while (1)
    {
        printf("===========================================\n");
        printf("모드를 선택하세요 (1: 아이템 추가, 2: DB 목록 열람, 0: 종료 및 저장)\n");
        printf("입력: ");

        if (scanf("%d", &mode) != 1) {
            printf("오류: 유효한 숫자를 입력해주세요.\n");
            while (getchar() != '\n');
            continue;
        }
        while (getchar() != '\n');
        printf("\n");

        switch (mode)
        {
        case 1:
            // 포인터 변수 db를 그대로 전달
            InputItemFromUser(db);
            break;
        case 2:
            // 포인터 변수 db를 그대로 전달
            PrintItemDB(db);
            break;
        case 0:
            printf("모드 선택을 종료합니다.\n");
            // void 함수이므로 값 없이 return;
            return;
        default:
            printf("오류: 0, 1, 2 중에서 선택해주세요.\n");
            break;
        }
    }
}

void CallItemDB(ItemDB* db)
{
    // UTF-8 환경 설정
    //setlocale(LC_ALL, ".UTF8");
    //SetConsoleOutputCP(CP_UTF8);
    //SetConsoleCP(CP_UTF8);

    // 1. 프로그램 시작 시 DB 초기화 및 데이터 로드 (딱 한 번!)
    InitItemDB(db); // main의 db 변수 주소를 넘김
    printf("기존 아이템 정보를 'items.csv'에서 불러옵니다...\n");
    LoadItemDBFromCSV(db, "items.csv");
    printf("로드 완료! 현재 아이템 수: %zu\n\n", db->count);

    // 2. 사용자 요청 처리 루프 실행
    SelectMode(db);

    // 3. 루프가 끝나고 프로그램이 종료되기 직전, 최종 데이터를 저장 (딱 한 번!)
    printf("\n변경된 아이템 정보를 'items.csv'에 저장합니다.\n");
    SaveItemDBToCSV(db, "items.csv");

    // 4. 메모리 해제
    FreeItemDB(db);

    printf("프로그램을 종료합니다.\n");
}