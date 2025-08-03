#include "leak.h"
#include "ItemDB.h"

item_database_t database = { 0 };

void initialize_database(void) {
    database.pItem_information = NULL;
    database.count = 0;
}

static bool add_item_to_database(const int index,
                          const char* const name,
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
    strncpy_s(pItem_information->name, sizeof(char) * name_length, name, name_length);
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

static void load_database_from_csv(const char * const pFile_name) {
    FILE *pFile = fopen(pFile_name, "r");
    if (!pFile) {
        perror("파일 불러오기 실패");
        return;
    }

    char line[128] = { 0 };
    fgets(line, sizeof(line), pFile); //헤더 스킵

    while (fgets(line, sizeof(line), pFile)) {
        int index = 0, max_stack = 0, type = 0, base_durability = 0, tool_kind = 0, material_tier = 0, is_placeable = false;
        char name[32] = { 0 };

        // 문자열 파싱 (CSV 형식: 정수,문자열,정수,정수,정수)
        if (sscanf_s(line, "%d,%31[^,],%d,%d,%d,%d,%d,%d",
            &index, name, (unsigned int)sizeof(name), &max_stack, &type, &base_durability, &tool_kind, &material_tier, &is_placeable) == 8) {
            add_item_to_database(index, name, max_stack, type, base_durability, tool_kind, material_tier, is_placeable);
            printf("\n");
        }
    }

    fclose(pFile);
}

static void clear_input(void) {
    while (getchar() != '\n');
}

static void input_item_from_user(void) {
    int index = 0;

    printf("새 아이템을 추가합니다.\n");

    //===== 인덱스 입력 및 중복 검사 루프 (수정된 부분) =====
    while (true) {
        printf("인덱스 : ");

        //사용자가 숫자가 아닌 값을 입력하는 경우에 대한 방어 코드
        if (scanf_s("%d", &index) != 1) {
            printf("오류: 유효한 숫자를 입력해주세요.\n");
            //입력 버퍼를 비워 무한 루프 방지
            clear_input();
            continue; //루프의 처음으로 돌아감
        }

        (void)getchar(); //scanf가 남긴 개행 문자 제거

        //FindItemByIndex 함수로 중복 검사
        if (find_item_by_index(index))
            printf("오류: 이미 사용 중인 인덱스(%d)입니다. 다른 인덱스를 입력해주세요.\n\n", index);
        else {
            //중복되지 않은 유효한 인덱스이므로 루프 탈출
            break;
        }
    }
    //=======================================================

    char name[32] = { 0 };

    printf("이름: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0'; //개행 문자 제거

    int maxStack = 0;
    printf("최대 갯수: ");
    scanf_s("%d", &maxStack);

    int type = 0;
    printf("타입 (1: 재료 (블록), 2: 도구, 3: 갑옷, 4: 소모품): ");
    scanf_s("%d", &type);

    int base_durability = 0;
    printf("기본 내구도: ");
    scanf_s("%d", &base_durability);

    int tool_kind = 0;
    if (type == 2) {
        printf("도구 종류 (1: 칼, 2: 곡괭이, 3: 도끼, 4: 삽): ");
        scanf_s("%d", &tool_kind);
    }

    int material_tier = 0;
    if (tool_kind) {
        printf("재료 티어 (1:나무, 2:돌, 3:철): ");
        scanf_s("%d", &material_tier);
    }

    int is_placeable = false;
    printf("설치 가능 여부 (1:설치 가능, 0:설치 불가): ");
    scanf_s("%d", &is_placeable);

    if (add_item_to_database(index, name, maxStack, type, base_durability, tool_kind, material_tier, is_placeable))
        printf("추가 완료!\n\n");
    else
        printf("아이템 추가 실패 (메모리 부족)\n");
}

static void print_item(void) {
    printf("=== Item Database (%zu items) ===\n", database.count);
    for (size_t i = 0; i < database.count; ++i)
        printf("[%d] 이름 : %s | 최대 갯수: %d | 아이템 종류 : %d | 내구도: %d | 종류: %d | 티어: %d | 설치 가능: %d\n",
               database.pItem_information[i].index,
               database.pItem_information[i].name,
               database.pItem_information[i].max_stack,
               database.pItem_information[i].type,
               database.pItem_information[i].base_durability,
               database.pItem_information[i].tool_kind,
               database.pItem_information[i].material_tier,
               database.pItem_information[i].is_placeable);

    printf("\n");
}

static void select_mode(void) {
    int mode = -1;

    //무한 루프. case 0의 'return'으로 탈출.
    while (true) {
        printf("===========================================\n" \
               "모드를 선택하세요(1: 아이템 추가, 2 : DB 목록 열람, 0 : 종료 및 저장)\n" \
               "입력: ");

        if (scanf_s("%d", &mode) != 1) {
            printf("오류: 유효한 숫자를 입력해주세요.\n");
            clear_input();
            continue;
        }

        clear_input();
        printf("\n");

        switch (mode) {
            case 1:
                input_item_from_user();
                break;

            case 2:
                print_item();
                break;
        
            case 0:
                printf("모드 선택을 종료합니다.\n");
                //void 함수이므로 값 없이 return;
                return;

            default:
                printf("오류: 0, 1, 2 중에서 선택해주세요.\n");
                break;
        }
    }
}

static void save_database_as_csv(const char * const pFile_name) {
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
        const item_information_t *pItem_information = &database.pItem_information[i];
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

void call_database(const bool edit) {
    setlocale(LC_ALL, ".UTF8");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    //1. 프로그램 시작 시 DB 초기화 및 데이터 로드 (딱 한 번!)
    initialize_database();
    printf("기존 아이템 정보를 'items.csv'에서 불러옵니다...\n");
    load_database_from_csv("items.csv");
    printf("로드 완료! 현재 아이템 수: %zu\n\n", database.count);

    if (!edit)
        return;

    //2. 사용자 요청 처리 루프 실행
    select_mode();

    //3. 루프가 끝나고 프로그램이 종료되기 직전, 최종 데이터를 저장 (딱 한 번!)
    printf("\n변경된 아이템 정보를 'items.csv'에 저장합니다.\n");
    save_database_as_csv("items.csv");

    printf("프로그램을 종료합니다.\n");
}

item_information_t *find_item_by_index(const int index) {
    for (size_t i = 0; i < database.count; ++i)
        if (database.pItem_information[i].index == index)
            return &database.pItem_information[i]; //아이템을 찾았으므로 주소를 반환

    return NULL; //끝까지 찾아도 없으면 NULL 반환
}

void destroy_database(void) {
    free(database.pItem_information);
    database.pItem_information = NULL;
    database.count = 0;
}