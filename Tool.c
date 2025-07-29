#include "leak.h"

#include "ItemDB.h"
#include "blockctrl.h"
#include <stdbool.h>
#include "inventory.h"
#include "player.h"

extern Inventory* g_inv;
extern ItemDB* g_db;

// 도구 종류 상수 (toolkind 값에 대응)
#define TOOL_KIND_NONE     -1
#define TOOL_KIND_PICKAXE  0
#define TOOL_KIND_AXE      1
#define TOOL_KIND_SHOVEL   2

// 재료 티어 상수 (materialTier 값에 대응)
#define TIER_HAND   0  // 맨손
#define TIER_WOOD   1
#define TIER_STONE  2
#define TIER_IRON   3


// 현재 선택된 인벤토리 칸의 아이템 포인터를 반환
Player_Item* GetEquippedItem(Inventory* inv, int selectedSlotIndex)
{
    if (selectedSlotIndex < 0 || selectedSlotIndex >= INVENTORY_SIZE) return NULL;

    Player_Item* item = &inv->item[selectedSlotIndex];
    if (item->Item_Index == 0 || item->quantity == 0) return NULL; // 빈 칸

    return item;
}



bool CanToolBreakBlock(const Item_Info* tool, int blockType)
{
    int toolKind = (tool != NULL) ? tool->toolkind : TOOL_KIND_NONE;
    int toolTier = (tool != NULL) ? tool->materialTier : TIER_HAND;

    switch (blockType)
    {
    case BLOCK_GRASS:
    case BLOCK_LEAF:
        return true; //맨손,모든 도구

    case BLOCK_DIRT:
        return (toolKind == TOOL_KIND_NONE || toolKind == TOOL_KIND_SHOVEL || toolKind == TOOL_KIND_PICKAXE); //맨손,삽,곡괭이로만 가능

    case BLOCK_SNOW:
    case BLOCK_SAND:
        return (toolKind == TOOL_KIND_SHOVEL || toolKind == TOOL_KIND_NONE); //맨손,삽으로만 가능

    case BLOCK_LOG:
        return (toolKind == TOOL_KIND_AXE || toolKind == TOOL_KIND_NONE); //맨손,도끼로만 가능

    case BLOCK_STONE:
        return (toolKind == TOOL_KIND_PICKAXE && toolTier >= TIER_WOOD); //나무등급이상 and 곡괭이로만 가능

    case BLOCK_IRON_ORE:
        return (toolKind == TOOL_KIND_PICKAXE && toolTier >= TIER_STONE); //돌등급이상 and 곡괭이로만 가능

    case BLOCK_BEDROCK:
    case BLOCK_AIR:
    case BLOCK_WATER:
        return false; //어떤 도구로도 파괴불가

    default:
        return false; //어떤 도구로도 파괴불가
    }
}




// 도구가 해당 블록에 주는 데미지를 계산하는 함수
int GetToolDamageToBlock(const Item_Info* tool, int blockType)
{
    const int baseDamage = 3;         // 맨손 기본 데미지
    const int bonusPerTier = 6;       // 도구 티어 1단계당 추가 데미지

    // 도구가 없거나 해당 블록을 부술 수 없으면 맨손 데미지
    if (tool == NULL || !CanToolBreakBlock(tool, blockType))
    {
        return baseDamage;
    }

    int toolTier = tool->materialTier;

    return baseDamage + (bonusPerTier * toolTier);
}

// 블록파괴시 인벤토리에 알맞은 아이템 획득 
int GetDropItemFromBlockType(block_t blockType)
{
    switch (blockType)
    {
    case BLOCK_GRASS: return 502;
    case BLOCK_DIRT: return 402;
    case BLOCK_STONE: return 403;
    case BLOCK_IRON_ORE: return 503;
    case BLOCK_LOG: return 405;
    case BLOCK_LEAF: return 504;
    case BLOCK_SNOW: return 505;
    case BLOCK_SAND: return 408;
        // 물, 베드락, 공기 등은 드롭 없음
    default: return -1;
    }
}


////////////////

bool CanPlaceBlock(int x, int y)
{
    // 1. 플레이어 위치에는 설치 불가
    if (x == player.x && y == player.y)
        return false;

    // 2. 해당 위치의 블록 정보 가져오기
    block_info_t target = get_block_info_at(x, y);

    // 3. 이미 블록이 존재하면 설치 불가
    if (target.type != BLOCK_AIR)
        return false;

    // 4. (선택) 설치 불가 블록 위에는 설치 제한 (예: BEDROCK)
    // 설치할 블록이 공기인 건 이 함수가 판단하지 않음
    // 필요 시 주변 블록까지 검사 가능

    return true;
}

bool ConsumeEquippedBlockItem(Inventory* inv, const ItemDB* db)
{
    Player_Item* equipped = GetEquippedItem(g_inv, g_inv->selectedIndex);
    if (!equipped || equipped->quantity <= 0)
        return false;

    const Item_Info* info = FindItemByIndex(db, equipped->Item_Index);
    if (!info || !info->isplaceable)
        return false;

    // 수량 감소
    equipped->quantity--;

    // 0개 되면 제거
    if (equipped->quantity == 0)
    {
        equipped->Item_Index = -1;
        equipped->durability = 0;
        equipped->isEquipped = false;
    }

    return true;
}

// item_index로부터 대응하는 block_t 반환 (없으면 BLOCK_AIR)
block_t GetBlockTypeFromItem(const ItemDB* db, int item_index)
{
    const Item_Info* info = FindItemByIndex(db, item_index);
    if (!info || !info->isplaceable) return BLOCK_AIR;

    // 아이템 인덱스 == 블록 타입이라고 가정 (연동 방식에 따라 수정 가능)
    return (block_t)(info->blockID);
}

