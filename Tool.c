#include "leak.h"
#include "Tool.h"

#include <stdbool.h>
#include "ItemDB.h"
#include "player.h"
#include "blockctrl.h"
#include "inventory.h"

const color_tchar_t pWooden_pickaxe[TEXTURE_SIZE][TEXTURE_SIZE] = {
    { { ' ', BACKGROUND_T_DARKYELLOW, 0 }, { ' ', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_BLACK }, { ' ', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_BLACK } },
    { { ' ', BACKGROUND_T_TRANSPARENT, 0 }, { ' ', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_BLACK }, { ' ', BACKGROUND_T_TRANSPARENT, 0 } },
    { { ' ', BACKGROUND_T_TRANSPARENT, 0 }, { ' ', BACKGROUND_T_DARKYELLOW, FOREGROUND_T_BLACK }, { ' ', BACKGROUND_T_TRANSPARENT, 0 } }
};

const bool can_tool_break_block(const item_information_t * const pTool, const block_t block) {
    int tool = TOOL_KIND_NONE, material = MATERIAL_TIER_NONE;
    
    if (pTool)
    {
        tool = pTool->tool_kind;
        material = pTool->material_tier;
    }

    switch (block) {
        case BLOCK_GRASS:
        case BLOCK_LEAF:
            return true; //맨손, 모든 도구

        case BLOCK_DIRT:
            return (tool == TOOL_KIND_NONE || tool == TOOL_KIND_SHOVEL || tool == TOOL_KIND_PICKAXE); //맨손, 삽, 곡괭이로만 가능

        case BLOCK_SNOW:
        case BLOCK_SAND:
            return (tool == TOOL_KIND_SHOVEL || tool == TOOL_KIND_NONE); //맨손, 삽으로만 가능

        case BLOCK_LOG:
            return (tool == TOOL_KIND_AXE || tool == TOOL_KIND_NONE); //맨손, 도끼로만 가능

        case BLOCK_STONE:
            return (tool == TOOL_KIND_PICKAXE && material >= MATERIAL_TIER_WOOD); //나무등급이상 and 곡괭이로만 가능

        case BLOCK_IRON_ORE:
            return (tool == TOOL_KIND_PICKAXE && material >= MATERIAL_TIER_STONE); //돌등급이상 and 곡괭이로만 가능

        case BLOCK_BEDROCK:
        case BLOCK_AIR:
        case BLOCK_WATER:
            return false; //어떤 도구로도 파괴불가
    }

    return false;
}

//도구가 해당 블록에 주는 데미지를 계산하는 함수
const int get_tool_damage_to_block(const item_information_t * const tool, const block_t block) {
    const int base_damage = 3; //맨손 기본 데미지

    //도구가 없거나 해당 블록을 부술 수 없으면 맨손 데미지
    if (!tool || !can_tool_break_block(tool, block))
        return base_damage;
    
    const int bonus_per_tier = 6; //도구 티어 1단계당 추가 데미지
    return base_damage + (bonus_per_tier * tool->material_tier);
}

//블록파괴시 인벤토리에 알맞은 아이템 획득 
const int get_drop_from_block(const block_t block) {
    switch (block) {
        // 물, 베드락, 공기 등은 드롭 없음
        case BLOCK_AIR:
        case BLOCK_BEDROCK:
        case BLOCK_WATER:
            return -1;
    }

    return block;
}

const bool can_place_block(const int x, const int y) {
    //1. 플레이어 위치에는 설치 불가
    if (x == player.x && y == player.y)
        return false;

    //2. 해당 위치의 블록 정보 가져오기
    const block_info_t target = get_block_info_at(x, y);

    //3. 이미 블록이 존재하면 설치 불가
    if (target.type != BLOCK_AIR)
        return false;

    /*
        4. (선택) 설치 불가 블록 위에는 설치 제한 (예: BEDROCK)
        설치할 블록이 공기인 건 이 함수가 판단하지 않음
        필요 시 주변 블록까지 검사 가능
    */
    return true;
}

const color_tchar_t get_tool_texture(const tool_t tool, const int x, const int y) {
    switch (tool) {
        case TOOL_WOODEN_PICKAXE:
            return pWooden_pickaxe[y][x];
    }

    return (color_tchar_t){ 0 };
}