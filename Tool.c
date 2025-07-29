#include "ItemDB.h"
#include "blockctrl.h"
#include <stdbool.h>
#include "inventory.h"
#include "player.h"

extern Inventory* g_inv;
extern ItemDB* g_db;

// ���� ���� ��� (toolkind ���� ����)
#define TOOL_KIND_NONE     -1
#define TOOL_KIND_PICKAXE  0
#define TOOL_KIND_AXE      1
#define TOOL_KIND_SHOVEL   2

// ��� Ƽ�� ��� (materialTier ���� ����)
#define TIER_HAND   0  // �Ǽ�
#define TIER_WOOD   1
#define TIER_STONE  2
#define TIER_IRON   3


// ���� ���õ� �κ��丮 ĭ�� ������ �����͸� ��ȯ
Player_Item* GetEquippedItem(Inventory* inv, int selectedSlotIndex)
{
    if (selectedSlotIndex < 0 || selectedSlotIndex >= INVENTORY_SIZE) return NULL;

    Player_Item* item = &inv->item[selectedSlotIndex];
    if (item->Item_Index == 0 || item->quantity == 0) return NULL; // �� ĭ

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
        return true; //�Ǽ�,��� ����

    case BLOCK_DIRT:
        return (toolKind == TOOL_KIND_NONE || toolKind == TOOL_KIND_SHOVEL || toolKind == TOOL_KIND_PICKAXE); //�Ǽ�,��,��̷θ� ����

    case BLOCK_SNOW:
    case BLOCK_SAND:
        return (toolKind == TOOL_KIND_SHOVEL || toolKind == TOOL_KIND_NONE); //�Ǽ�,�����θ� ����

    case BLOCK_LOG:
        return (toolKind == TOOL_KIND_AXE || toolKind == TOOL_KIND_NONE); //�Ǽ�,�����θ� ����

    case BLOCK_STONE:
        return (toolKind == TOOL_KIND_PICKAXE && toolTier >= TIER_WOOD); //��������̻� and ��̷θ� ����

    case BLOCK_IRON_ORE:
        return (toolKind == TOOL_KIND_PICKAXE && toolTier >= TIER_STONE); //������̻� and ��̷θ� ����

    case BLOCK_BEDROCK:
    case BLOCK_AIR:
    case BLOCK_WATER:
        return false; //� �����ε� �ı��Ұ�

    default:
        return false; //� �����ε� �ı��Ұ�
    }
}




// ������ �ش� ��Ͽ� �ִ� �������� ����ϴ� �Լ�
int GetToolDamageToBlock(const Item_Info* tool, int blockType)
{
    const int baseDamage = 3;         // �Ǽ� �⺻ ������
    const int bonusPerTier = 6;       // ���� Ƽ�� 1�ܰ�� �߰� ������

    // ������ ���ų� �ش� ����� �μ� �� ������ �Ǽ� ������
    if (tool == NULL || !CanToolBreakBlock(tool, blockType))
    {
        return baseDamage;
    }

    int toolTier = tool->materialTier;

    return baseDamage + (bonusPerTier * toolTier);
}

// ����ı��� �κ��丮�� �˸��� ������ ȹ�� 
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
        // ��, �����, ���� ���� ��� ����
    default: return -1;
    }
}


////////////////

bool CanPlaceBlock(int x, int y)
{
    // 1. �÷��̾� ��ġ���� ��ġ �Ұ�
    if (x == player.x && y == player.y)
        return false;

    // 2. �ش� ��ġ�� ��� ���� ��������
    block_info_t target = get_block_info_at(x, y);

    // 3. �̹� ����� �����ϸ� ��ġ �Ұ�
    if (target.type != BLOCK_AIR)
        return false;

    // 4. (����) ��ġ �Ұ� ��� ������ ��ġ ���� (��: BEDROCK)
    // ��ġ�� ����� ������ �� �� �Լ��� �Ǵ����� ����
    // �ʿ� �� �ֺ� ��ϱ��� �˻� ����

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

    // ���� ����
    equipped->quantity--;

    // 0�� �Ǹ� ����
    if (equipped->quantity == 0)
    {
        equipped->Item_Index = -1;
        equipped->durability = 0;
        equipped->isEquipped = false;
    }

    return true;
}

// item_index�κ��� �����ϴ� block_t ��ȯ (������ BLOCK_AIR)
block_t GetBlockTypeFromItem(const ItemDB* db, int item_index)
{
    const Item_Info* info = FindItemByIndex(db, item_index);
    if (!info || !info->isplaceable) return BLOCK_AIR;

    // ������ �ε��� == ��� Ÿ���̶�� ���� (���� ��Ŀ� ���� ���� ����)
    return (block_t)(info->blockID);
}

