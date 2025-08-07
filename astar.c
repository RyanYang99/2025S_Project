#include "leak.h"
#include "astar.h"

#include "map.h"
#include "Mob.h"

typedef struct node_t {
    int x, y, g, h, f;
    struct node_t *pParent;
} node_t;

static int allocated_x = 0, open_count = 0, closed_count = 0;
static node_t **pOpen_list = NULL,
              **pClosed_list = NULL;

static int get_h(const int x1, const int y1, const int x2, const int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

const direction_t find_next_direction(const int start_x,
                                      const int start_y,
                                      const int target_x,
                                      const int target_y,
                                      const is_movable_t is_movable) {
    if (start_x == target_x && start_y == target_y)
        return DIRECTION_NONE;

    if (map.size.x != allocated_x) {
        allocated_x = map.size.x;
        
        const int size = sizeof(node_t*) * allocated_x * map.size.y;
        pOpen_list = realloc(pOpen_list, size);
        pClosed_list = realloc(pClosed_list, size);
    }
    open_count = closed_count = 0;

    node_t *pStart = malloc(sizeof(node_t)), *pFinal = NULL;
    pStart->x = start_x;
    pStart->y = start_y;
    pStart->g = 0;
    pStart->h = get_h(start_x, start_y, target_x, target_y);
    pStart->f = pStart->h;
    pStart->pParent = NULL;

    pOpen_list[open_count++] = pStart;

    int work = 0;
    while (open_count > 0) {
        int best_f_index = 0;
        for (int i = 0; i < open_count; ++i)
            if (pOpen_list[i]->f < pOpen_list[best_f_index]->f)
                best_f_index = i;

        node_t *pCurrent = pOpen_list[best_f_index];
        pOpen_list[best_f_index] = pOpen_list[--open_count];
        pClosed_list[closed_count++] = pCurrent;

        if (pCurrent->x == target_x && pCurrent->y == target_y) {
            pFinal = pCurrent;
            break;
        }

        if (++work > 30 * map.size.y) {
            pFinal = pCurrent;
            break;
        }

        for (int i = 0; i < 4; ++i) {
            int new_x = pCurrent->x, new_y = pCurrent->y;
            if (i == 0)
                ++new_x;
            else if (i == 1)
                --new_x;
            else if (i == 2)
                ++new_y;
            else
                --new_y;

            if (!is_movable(new_x, new_y))
                continue;

            bool in_closed = false;
            for (int j = 0; j < closed_count; ++j)
                if (pClosed_list[j]->x == new_x && pClosed_list[j]->y == new_y) {
                    in_closed = true;
                    break;
                }
            if (in_closed)
                continue;

            node_t *pNeighbor = NULL;
            for (int j = 0; j < open_count; ++j)
                if (pOpen_list[j]->x == new_x && pOpen_list[j]->y == new_y) {
                    pNeighbor = pOpen_list[j];
                    break;
                }

            const int tentative_g = pCurrent->g + 1;
            if (!pNeighbor) {
                pNeighbor = malloc(sizeof(node_t));
                pNeighbor->x = new_x;
                pNeighbor->y = new_y;
                pNeighbor->g = tentative_g;
                pNeighbor->h = get_h(new_x, new_y, target_x, target_y);
                pNeighbor->f = pNeighbor->g + pNeighbor->h;
                pNeighbor->pParent = pCurrent;
                pOpen_list[open_count++] = pNeighbor;
            } else if (pNeighbor->g > tentative_g) {
                pNeighbor->g = tentative_g;
                pNeighbor->h = get_h(pNeighbor->x, pNeighbor->y, target_x, target_y);
                pNeighbor->f = pNeighbor->g + pNeighbor->h;
                pNeighbor->pParent = pCurrent;
            }
        }
    }

    direction_t direction = DIRECTION_NONE;
    if (pFinal) {
        node_t *pPath = pFinal;
        while (pPath->pParent && !(start_x == pPath->pParent->x && start_y == pPath->pParent->y))
            pPath = pPath->pParent;

        if (pPath->x > start_x)
            direction = DIRECTION_RIGHT;
        else if (pPath->x < start_x)
            direction = DIRECTION_LEFT;
        else if (pPath->y > start_y)
            direction = DIRECTION_DOWN;
        else if (pPath->y < start_y)
            direction = DIRECTION_UP;
    }

    for (int i = 0; i < open_count; ++i)
        free(pOpen_list[i]);

    for (int i = 0; i < closed_count; ++i)
        free(pClosed_list[i]);
    return direction;
}

void destroy_astar(void) {
    free(pOpen_list);
    free(pClosed_list);
}