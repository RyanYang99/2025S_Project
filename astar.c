#include "leak.h"
#include "astar.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

// A* 알고리즘을 위한 노드 구조체
typedef struct PathNode {
    int x, y, g, h, f;
    struct PathNode *parent;
} PathNode;

static PathNode *all_allocated_nodes = NULL, *open_list_heap = NULL;
static bool** close_list_visited = NULL, **in_open_list_check = NULL;
static int **open_list_index = NULL, allocated_x = 0;

static const int Calculate_Heuristic(const int x1, const int y1, const int x2, const int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

static void swap_nodes(PathNode * const a, PathNode * const b) {
    PathNode temp = *a;
    *a = *b;
    *b = temp;

    if (open_list_index) {
        int temp_index = open_list_index[a->x][a->y];
        open_list_index[a->x][a->y] = open_list_index[b->x][b->y];
        open_list_index[b->x][b->y] = temp_index;
    }
}

static void heapify_up(int index) {
    while (index > 0) {
        int parent_index = (index - 1) / 2;
        if (open_list_heap[index].f < open_list_heap[parent_index].f) {
            swap_nodes(&open_list_heap[index], &open_list_heap[parent_index]);
            index = parent_index;
        }
        else
            break;
    }
}

static void heapify_down(const int heap_size, const int index) {
    const int left_child = 2 * index + 1, right_child = 2 * index + 2;
    int min_index = index;

    if (left_child < heap_size && open_list_heap[left_child].f < open_list_heap[min_index].f)
        min_index = left_child;

    if (right_child < heap_size && open_list_heap[right_child].f < open_list_heap[min_index].f)
        min_index = right_child;

    if (min_index != index) {
        swap_nodes(&open_list_heap[index], &open_list_heap[min_index]);
        heapify_down(heap_size, min_index);
    }
}

static void heap_push(int * const heap_size, const PathNode * const node) {
    if (*heap_size < MAX_HEAP_SIZE) {
        open_list_heap[*heap_size] = *node;
        (*heap_size)++;

        open_list_index[node->x][node->y] = *heap_size - 1;
        heapify_up(*heap_size - 1);
    }
}

static PathNode * const heap_pop(int * const heap_size) {
    if (*heap_size <= 0)
        return NULL;

    PathNode * const min_node = &open_list_heap[0];
    (*heap_size)--;
    if (*heap_size > 0) {
        open_list_heap[0] = open_list_heap[*heap_size];

        open_list_index[open_list_heap[0].x][open_list_heap[0].y] = 0;
        heapify_down(*heap_size, 0);
    }
    return min_node;
}

static void *recalloc(void *pBuffer, const size_t old, const size_t new) {
    void *pNew = realloc(pBuffer, new);
    if (new > old && pNew)
        memset((void *)(((char *)pNew) + old), 0, new - old);

    return pNew;
}

path_t find_path(int start_x, int start_y, int target_x, int target_y, IsMovableFunc is_movable) {
    if (!all_allocated_nodes)
        all_allocated_nodes = malloc(sizeof(PathNode) * MAX_NODES_FOR_ASTAR_SEARCH);

    if (!open_list_heap)
        open_list_heap = malloc(sizeof(PathNode) * MAX_HEAP_SIZE);

    int allocated_node_count = 0;
    int open_list_size = 0;

    if (map.size.x != allocated_x) {
        int size = map.size.x * sizeof(bool *), old = allocated_x * sizeof(bool *);
        close_list_visited = recalloc(close_list_visited, old, size);
        in_open_list_check = recalloc(in_open_list_check, old, size);
        open_list_index = recalloc(open_list_index, allocated_x * sizeof(int *), map.size.x * sizeof(int *));
    }

    path_t result_path = { 0 };
    bool path_found = false;

    if (allocated_x != map.size.x) {
        allocated_x = map.size.x;

        for (int i = 0; i < allocated_x; i++) {
            int size = map.size.y * sizeof(bool);
            close_list_visited[i] = realloc(close_list_visited[i], size);
            memset(close_list_visited[i], false, size);

            in_open_list_check[i] = realloc(in_open_list_check[i], size);
            memset(in_open_list_check[i], false, size);

            size = map.size.y * sizeof(int);
            open_list_index[i] = realloc(open_list_index[i], size);
            memset(open_list_index[i], 0, size);
        }
    }

    const int size = sizeof(bool) * map.size.y;
    for (int i = 0; i < map.size.x; i++) {
        memset(close_list_visited[i], false, size);
        memset(in_open_list_check[i], false, size);
    }

    PathNode start_node = {
        .x = start_x,
        .y = start_y,
        .g = 0,
        .h = Calculate_Heuristic(start_x, start_y, target_x, target_y),
        .parent = NULL
    };
    start_node.f = start_node.g + start_node.h;

    heap_push(&open_list_size, &start_node);
    in_open_list_check[start_x][start_y] = true;
    all_allocated_nodes[allocated_node_count++] = start_node;

    while (open_list_size > 0 && allocated_node_count < MAX_NODES_FOR_ASTAR_SEARCH) {
        PathNode *current_node = heap_pop(&open_list_size);

        in_open_list_check[current_node->x][current_node->y] = false;

        if (current_node->x == target_x && current_node->y == target_y) {
            PathNode *temp = current_node;
            int path_len = 0;
            while (temp != NULL && path_len < MAX_PATH_LENGTH) {
                result_path.path[path_len].X = (SHORT)temp->x;
                result_path.path[path_len].Y = (SHORT)temp->y;
                path_len++;
                temp = temp->parent;
            }
            result_path.count = path_len;

            for (int i = 0; i < result_path.count / 2; ++i) {
                COORD temp_coord = result_path.path[i];
                result_path.path[i] = result_path.path[result_path.count - 1 - i];
                result_path.path[result_path.count - 1 - i] = temp_coord;
            }
            path_found = true;
            break;
        }

        close_list_visited[current_node->x][current_node->y] = true;

        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                if (dx == 0 && dy == 0) continue;

                int neighbor_x = current_node->x + dx;
                int neighbor_y = current_node->y + dy;

                if (neighbor_x < 0 || neighbor_x >= map.size.x || neighbor_y < 0 || neighbor_y >= map.size.y) continue;
                if (close_list_visited[neighbor_x][neighbor_y]) continue;
                if (!is_movable(neighbor_x, neighbor_y)) continue;

                int new_g = current_node->g + 1;

                if (in_open_list_check[neighbor_x][neighbor_y]) {
                    int existing_index = open_list_index[neighbor_x][neighbor_y];
                    PathNode *existing_node = &open_list_heap[existing_index];
                    if (new_g < existing_node->g) {
                        existing_node->g = new_g;
                        existing_node->f = existing_node->g + existing_node->h;
                        existing_node->parent = current_node;
                        heapify_up(existing_index);
                    }
                }
                else {
                    PathNode neighbor_node = {
                        .x = neighbor_x,
                        .y = neighbor_y,
                        .g = new_g,
                        .h = Calculate_Heuristic(neighbor_x, neighbor_y, target_x, target_y),
                        .parent = current_node
                    };
                    neighbor_node.f = neighbor_node.g + neighbor_node.h;

                    if (open_list_size < MAX_HEAP_SIZE && allocated_node_count < MAX_NODES_FOR_ASTAR_SEARCH) {
                        heap_push(&open_list_size, &neighbor_node);
                        in_open_list_check[neighbor_x][neighbor_y] = true;
                        all_allocated_nodes[allocated_node_count++] = neighbor_node;
                    }
                }
            }
        }
    }

    return result_path;
}

void destroy_astar(void) {
    free(all_allocated_nodes);
    all_allocated_nodes = NULL;

    free(open_list_heap);
    open_list_heap = NULL;

    for (int i = 0; i < allocated_x; ++i)
        free(close_list_visited[i]);
    free(close_list_visited);
    close_list_visited = NULL;
    
    for (int i = 0; i < allocated_x; ++i)
        free(in_open_list_check[i]);
    free(in_open_list_check);
    in_open_list_check = NULL;

    for (int i = 0; i < allocated_x; ++i)
        free(open_list_index[i]);
    free(open_list_index);
    open_list_index = NULL;
}