#include "leak.hpp"
#include "astar.hpp"

#include <cmath>
#include <vector>

#include "map.hpp"

namespace astar {
    struct node {
        int x, y, g, h, f;
        struct node *pParent;
    };

    static int open_count{}, closed_count{};
    static std::vector<node *> open_list{}, closed_list{};

    static int get_h(const int x1, const int y1, const int x2, const int y2) noexcept {
        return abs(x1 - x2) + abs(y1 - y2);
    }

    direction find_next_direction(const int start_x,
                                  const int start_y,
                                  const int target_x,
                                  const int target_y,
                                  const is_movable is_movable_) noexcept {
        if (start_x == target_x && start_y == target_y)
            return direction::none;

        if (static_cast<size_t>(map.size.x) != open_list.size()) {
            open_list.resize(map.size.x);
            closed_list.resize(map.size.x);
        }
        open_count = closed_count = 0;

        node *pStart{ new node }, *pFinal{};
        pStart->x = start_x;
        pStart->y = start_y;
        pStart->g = 0;
        pStart->h = get_h(start_x, start_y, target_x, target_y);
        pStart->f = pStart->h;
        pStart->pParent = nullptr;

        open_list[open_count++] = pStart;

        int work = 0;
        while (open_count > 0) {
            int best_f_index = 0;
            for (int i{}; i < open_count; ++i)
                if (open_list[i]->f < open_list[best_f_index]->f)
                    best_f_index = i;

            node *pCurrent = open_list[best_f_index];
            open_list[best_f_index] = open_list[--open_count];
            closed_list[closed_count++] = pCurrent;

            if (pCurrent->x == target_x && pCurrent->y == target_y) {
                pFinal = pCurrent;
                break;
            }

            if (++work > 30 * map.size.y) {
                pFinal = pCurrent;
                break;
            }

            for (int i{}; i < 4; ++i) {
                int new_x = pCurrent->x, new_y = pCurrent->y;
                if (i == 0)
                    ++new_x;
                else if (i == 1)
                    --new_x;
                else if (i == 2)
                    ++new_y;
                else
                    --new_y;

                if (!is_movable_ || !is_movable_(new_x, new_y))
                    continue;

                bool in_closed = false;
                for (int j{}; j < closed_count; ++j)
                    if (closed_list[j]->x == new_x && closed_list[j]->y == new_y) {
                        in_closed = true;
                        break;
                    }
                if (in_closed)
                    continue;

                node *pNeighbor{};
                for (int j{}; j < open_count; ++j)
                    if (open_list[j]->x == new_x && open_list[j]->y == new_y) {
                        pNeighbor = open_list[j];
                        break;
                    }

                const int tentative_g = pCurrent->g + 1;
                if (!pNeighbor) {
                    pNeighbor = new node;
                    pNeighbor->x = new_x;
                    pNeighbor->y = new_y;
                    pNeighbor->g = tentative_g;
                    pNeighbor->h = get_h(new_x, new_y, target_x, target_y);
                    pNeighbor->f = pNeighbor->g + pNeighbor->h;
                    pNeighbor->pParent = pCurrent;
                    open_list[open_count++] = pNeighbor;
                } else if (pNeighbor->g > tentative_g) {
                    pNeighbor->g = tentative_g;
                    pNeighbor->h = get_h(pNeighbor->x, pNeighbor->y, target_x, target_y);
                    pNeighbor->f = pNeighbor->g + pNeighbor->h;
                    pNeighbor->pParent = pCurrent;
                }
            }
        }

        direction direction{};
        if (pFinal) {
            node *pPath = pFinal;
            while (pPath->pParent && !(start_x == pPath->pParent->x && start_y == pPath->pParent->y))
                pPath = pPath->pParent;

            if (pPath->x > start_x)
                direction = direction::right;
            else if (pPath->x < start_x)
                direction = direction::left;
            else if (pPath->y > start_y)
                direction = direction::down;
            else if (pPath->y < start_y)
                direction = direction::up;
        }

        for (int i{}; i < open_count; ++i)
            delete open_list[i];

        for (int i{}; i < closed_count; ++i)
            delete closed_list[i];
        return direction;
    }
}