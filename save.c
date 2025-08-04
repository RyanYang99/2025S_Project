#include "leak.h"
#include "save.h"

save_t *pCurrent_save = NULL;

void instantiate_save(void) {
    pCurrent_save = calloc(1, sizeof(save_t));
}

void write_save(const char * const pPath) {
    return;

    FILE *pFile = fopen(pPath, "wb");

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

    fclose(pFile);
}

save_t *load_save(const char * const pPath) {
    FILE *pFile = fopen(pPath, "rb");
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

    fclose(pFile);
    return pSave;
}