#define _CRT_SECURE_NO_WARNINGS

#include "leak.h"
#include "save.h"

#include <string.h>
#include <ShlObj_core.h>
#include "game.h"
#include "input.h"

static bool should_render_save_menu = false;
static bool pUsed[MAX_SAVE_SPOTS] = { 0 };
static char *pMessage = "";

save_t *pCurrent_save = NULL;

void initialize_save(void) {
    should_render_save_menu = false;
    pMessage = "";
}

void instantiate_save(void) {
    pCurrent_save = calloc(1, sizeof(save_t));
}

static void write_save(LPCWSTR const pPath) {
    FILE *pFile = _wfopen(pPath, L"wb");

    fwrite(&pCurrent_save->game_time, sizeof(pCurrent_save->game_time), 1, pFile);
    fwrite(&pCurrent_save->x, sizeof(pCurrent_save->x), 1, pFile);
    fwrite(&pCurrent_save->y, sizeof(pCurrent_save->y), 1, pFile);
    fwrite(&pCurrent_save->hp, sizeof(pCurrent_save->hp), 1, pFile);
    fwrite(pCurrent_save->pInventory, sizeof(player_item_t), INVENTORY_SIZE, pFile);
    fwrite(pCurrent_save->pHotbar_linked_index, sizeof(int), HOTBAR_COUNT, pFile);
    fwrite(pCurrent_save->pPermuation_table, sizeof(int), PERLIN_SIZE, pFile);
    fwrite(&pCurrent_save->map_x, sizeof(pCurrent_save->map_x), 1, pFile);
    fwrite(&pCurrent_save->map_y, sizeof(pCurrent_save->map_y), 1, pFile);
    fwrite(pCurrent_save->pBlocks, sizeof(block_info_t), pCurrent_save->map_x * pCurrent_save->map_y, pFile);
    fwrite(&pCurrent_save->mob_count, sizeof(pCurrent_save->mob_count), 1, pFile);
    fwrite(&pCurrent_save->mob_level, sizeof(pCurrent_save->mob_level), 1, pFile);
    fwrite(pCurrent_save->pMobs, sizeof(Mob), pCurrent_save->mob_count, pFile);

    fclose(pFile);
}

const static bool directory_exists(LPCWSTR const pPath) {
    const DWORD attribute = GetFileAttributes(pPath);
    return attribute != INVALID_FILE_ATTRIBUTES && (attribute & FILE_ATTRIBUTE_DIRECTORY);
}

static LPCWSTR get_file_path(const int index) {
    static WCHAR pPath[MAX_PATH] = { 0 };

    wcscpy_s(pPath, MAX_PATH, get_save_folder());
    wcscat_s(pPath, MAX_PATH, L"\\");

    WCHAR number[2] = { 0 };
    swprintf_s(number, 2, L"%d", index);
    wcscat_s(pPath, MAX_PATH, number);
    return pPath;
}

void save_input(void) {
    if (!keyboard_pressed)
        return;

    if (input_character == VK_ESCAPE) {
        if (!should_render_save_menu)
            get_save_spots();

        should_render_save_menu = !should_render_save_menu;
    } else if (should_render_save_menu) {
        if (tolower(input_character) == 'q')
            game_exit = true;
        else {
            const int number = input_character - '0';
            if (number >= 1 && number <= 3) {
                save_date_time();
                save_player();
                save_inventory();
                save_map();
                save_mob();

                LPCWSTR pFolder = get_save_folder();
                if (!directory_exists(pFolder))
                    CreateDirectory(pFolder, NULL);

                LPCWSTR pFile = get_file_path(number - 1);
                write_save(pFile);
                free_save();
            }
        }
    }
}

void render_save_menu(void) {
    if (!should_render_save_menu)
        return;

    int y = console.size.Y / 2 - 5 / 2;
    print_center("=============== Save ===============", y++, BACKGROUND_T_BLACK, FOREGROUND_T_BLUE);
    print_center("1 ~ 3: Save, ESC: Close Q: Main Menu", y++, BACKGROUND_T_BLACK, FOREGROUND_T_BLUE);

    for (int i = 0; i < MAX_SAVE_SPOTS; ++i)
        print_center("%d. %s", y++, BACKGROUND_T_BLACK, FOREGROUND_T_BLUE, i + 1, pUsed[i] ? "In Use (Overwrite)" : "Empty");
}

save_t *load_save(LPCWSTR const pPath) {
    FILE *pFile = _wfopen(pPath, L"rb");
    save_t *pSave = malloc(sizeof(save_t));

    fread(&pSave->game_time, sizeof(pSave->game_time), 1, pFile);
    fread(&pSave->x, sizeof(pSave->x), 1, pFile);
    fread(&pSave->y, sizeof(pSave->y), 1, pFile);
    fread(&pSave->hp, sizeof(pSave->hp), 1, pFile);
    fread(pSave->pInventory, sizeof(player_item_t), INVENTORY_SIZE, pFile);
    fread(pSave->pHotbar_linked_index, sizeof(int), HOTBAR_COUNT, pFile);
    fread(pSave->pPermuation_table, sizeof(int), PERLIN_SIZE, pFile);
    fread(&pSave->map_x, sizeof(pSave->map_x), 1, pFile);
    fread(&pSave->map_y, sizeof(pSave->map_y), 1, pFile);

    const int size = pSave->map_x * pSave->map_y;
    pSave->pBlocks = malloc(sizeof(block_info_t) * size);
    fread(pSave->pBlocks, sizeof(block_info_t), size, pFile);

    fread(&pSave->mob_count, sizeof(pSave->mob_count), 1, pFile);
    fread(&pSave->mob_level, sizeof(pSave->mob_level), 1, pFile);

    pSave->pMobs = malloc(sizeof(Mob) * pSave->mob_count);
    fread(pSave->pMobs, sizeof(Mob), pSave->mob_count, pFile);

    fclose(pFile);
    return pSave;
}

void load_save_index(const int index) {
    LPCWSTR pFile = get_file_path(index);
    pCurrent_save = load_save(pFile);
}

LPCWSTR const get_save_folder(void) {
    static WCHAR pPath[MAX_PATH] = { 0 };
    SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, pPath);

    wcscat_s(pPath, MAX_PATH, L"\\TerraCraft");
    return pPath;
}

const static bool file_exists(LPCWSTR const pPath) {
    const DWORD attribute = GetFileAttributes(pPath);
    return attribute != INVALID_FILE_ATTRIBUTES && !(attribute & FILE_ATTRIBUTE_DIRECTORY);
}

bool *get_save_spots(void) {
    memset(pUsed, 0, sizeof(bool) * MAX_SAVE_SPOTS);

    if (!directory_exists(get_save_folder()))
        return pUsed;

    for (int i = 0; i < MAX_SAVE_SPOTS; ++i)
        pUsed[i] = file_exists(get_file_path(i));

    return pUsed;
}

void free_save(void) {
    if (!pCurrent_save)
        return;

    free(pCurrent_save->pBlocks);
    free(pCurrent_save->pMobs);
    free(pCurrent_save);
    
    pCurrent_save = NULL;
}