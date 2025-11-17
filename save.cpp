#define _CRT_SECURE_NO_WARNINGS

#include "leak.hpp"
#include "save.hpp"

#include <stdio.h>
#include <string.h>
#include <string>
#include <format>

#include <ShlObj_core.h>
#include "game.hpp"
#include "input.hpp"
#include "delta_time.hpp"
#include "player.hpp"

static bool should_render_save_menu = false, pUsed[MAX_SAVE_SPOTS] = { 0 };
static std::string pMessage{};

save_t *pSave_current = NULL;

void save_initialize(void) {
    should_render_save_menu = false;
    pMessage = "";
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

static void write_save(LPCWSTR const pPath) {
    FILE *pFile = _wfopen(pPath, L"wb");

    fwrite(&pSave_current->game_time, sizeof(pSave_current->game_time), 1, pFile);
    fwrite(&pSave_current->x, sizeof(pSave_current->x), 1, pFile);
    fwrite(&pSave_current->y, sizeof(pSave_current->y), 1, pFile);
    fwrite(&pSave_current->HP, sizeof(pSave_current->HP), 1, pFile);
    fwrite(&pSave_current->max_HP, sizeof(pSave_current->max_HP), 1, pFile);
    fwrite(pSave_current->pInventory, sizeof(player_item_t), INVENTORY_SIZE, pFile);
    fwrite(pSave_current->pHotbar_linked_index, sizeof(int), HOTBAR_COUNT, pFile);
    fwrite(pSave_current->pPermuation_table, sizeof(int), PERLIN_SIZE, pFile);
    fwrite(&pSave_current->map_x, sizeof(pSave_current->map_x), 1, pFile);
    fwrite(&pSave_current->map_y, sizeof(pSave_current->map_y), 1, pFile);
    fwrite(pSave_current->pBlocks, sizeof(block_info_t), pSave_current->map_x * pSave_current->map_y, pFile);
    fwrite(&pSave_current->mob_count, sizeof(pSave_current->mob_count), 1, pFile);
    fwrite(&pSave_current->mob_level, sizeof(pSave_current->mob_level), 1, pFile);
    fwrite(pSave_current->pMobs, sizeof(mob_t), pSave_current->mob_count, pFile);

    fclose(pFile);
}

void save_input(void) {
    if (!Input::keyboard_pressed())
        return;

    const char input_character{ Input::input_character() };
    if (input_character == VK_ESCAPE) {
        if (!should_render_save_menu)
            get_save_spots();

        should_render_save_menu = !should_render_save_menu;
        pMessage = "";
    } else if (should_render_save_menu) {
        if (tolower(input_character) == 'q')
            Game::instance()->exit(true);
        else {
            const int number = input_character - '0';

            if (number >= 1 && number <= MAX_SAVE_SPOTS) {
                Game::instance()->elapsed_since_start().save();
                player_save();
                inventory_save();
                map_save();
                mob_save();

                LPCWSTR pFolder = get_save_folder();
                if (!directory_exists(pFolder))
                    CreateDirectory(pFolder, NULL);

                LPCWSTR pFile = get_file_path(number - 1);
                write_save(pFile);
                save_free();

                pMessage = "Saved";
            }
        }
    }
}

void save_render(void) {
    static float timer = 0.0f;

    if (!should_render_save_menu)
        return;

    const bool has_message = strcmp(pMessage.c_str(), "");
    if (has_message)
        timer += delta_time_t::delta_time;
    if (timer >= 2.0f) {
        timer = 0.0f;
        pMessage = "";
    }

    int y = Console::size().Y / 2 - (MAX_SAVE_SPOTS + 2 + has_message) / 2;
    Console::print_center("=== Save ===", y++, BG::black, FG::blue);
    if (has_message)
        Console::print_center(pMessage, y++, BG::black, FG::cyan);
    Console::print_center("[1 ~ 3]: Save, [ESC]: Close [Q]: Main Menu", y++, BG::black, FG::blue);

    for (int i = 0; i < MAX_SAVE_SPOTS; ++i)
        Console::print_center(std::format("{}. {}", i + 1, pUsed[i] ? "In Use (Overwrite)" : "Empty"), y++, BG::black, FG::blue);
}

void save_instantiate(void) {
    pSave_current = static_cast<save_t *>(calloc(1, sizeof(save_t)));
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

const bool * const get_save_spots(void) {
    memset(pUsed, 0, sizeof(bool) * MAX_SAVE_SPOTS);

    if (!directory_exists(get_save_folder()))
        return pUsed;

    for (int i = 0; i < MAX_SAVE_SPOTS; ++i)
        pUsed[i] = file_exists(get_file_path(i));

    return pUsed;
}

static save_t *load_save(LPCWSTR const pPath) {
    FILE *pFile = _wfopen(pPath, L"rb");
    save_t *pSave = static_cast<save_t *>(malloc(sizeof(save_t)));

    fread(&pSave->game_time, sizeof(pSave->game_time), 1, pFile);
    fread(&pSave->x, sizeof(pSave->x), 1, pFile);
    fread(&pSave->y, sizeof(pSave->y), 1, pFile);
    fread(&pSave->HP, sizeof(pSave->HP), 1, pFile);
    fread(&pSave->max_HP, sizeof(pSave->max_HP), 1, pFile);
    fread(pSave->pInventory, sizeof(player_item_t), INVENTORY_SIZE, pFile);
    fread(pSave->pHotbar_linked_index, sizeof(int), HOTBAR_COUNT, pFile);
    fread(pSave->pPermuation_table, sizeof(int), PERLIN_SIZE, pFile);
    fread(&pSave->map_x, sizeof(pSave->map_x), 1, pFile);
    fread(&pSave->map_y, sizeof(pSave->map_y), 1, pFile);

    const int size = pSave->map_x * pSave->map_y;
    pSave->pBlocks = static_cast<block_info_t *>(malloc(sizeof(block_info_t) * size));
    fread(pSave->pBlocks, sizeof(block_info_t), size, pFile);

    fread(&pSave->mob_count, sizeof(pSave->mob_count), 1, pFile);
    fread(&pSave->mob_level, sizeof(pSave->mob_level), 1, pFile);

    pSave->pMobs = static_cast<mob_t *>(malloc(sizeof(mob_t) * pSave->mob_count));
    fread(pSave->pMobs, sizeof(mob_t), pSave->mob_count, pFile);

    fclose(pFile);
    return pSave;
}

void load_save_index(const int index) {
    LPCWSTR pFile = get_file_path(index);
    pSave_current = load_save(pFile);
}

void save_free(void) {
    if (!pSave_current)
        return;

    free(pSave_current->pBlocks);
    free(pSave_current->pMobs);
    free(pSave_current);

    pSave_current = NULL;
}