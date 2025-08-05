#include "astar.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "map.h"


extern map_t map;

PathNode* Create_node(int x, int y, int g, int h, PathNode* parent) {
    PathNode* newNode = (PathNode*)malloc(sizeof(PathNode));
    if (newNode == NULL) {
        fprintf(stderr, "메모리 할당 실패 (PathNode)\n");
        exit(EXIT_FAILURE);
    }
    newNode->x = x;
    newNode->y = y;
    newNode->g = g;
    newNode->h = h;
    newNode->f = g + h;
    newNode->parent = parent;
    return newNode;
}

int Calculate_Heuristic(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}


void swap_nodes(PathNode** a, PathNode** b, int** open_list_index) {
    PathNode* temp = *a;
    *a = *b;
    *b = temp;

    
    if (open_list_index) {
        int temp_index = open_list_index[(*a)->x][(*a)->y];
        open_list_index[(*a)->x][(*a)->y] = open_list_index[(*b)->x][(*b)->y];
        open_list_index[(*b)->x][(*b)->y] = temp_index;
    }
}
void heapify_up(PathNode** heap, int heap_size, int index, int** open_list_index) {
    while (index > 0) {
        int parent_index = (index - 1) / 2;
        if (heap[index]->f < heap[parent_index]->f) {
            swap_nodes(&heap[index], &heap[parent_index], open_list_index);
            index = parent_index;
        }
        else {
            break;
        }
    }
}
void heapify_down(PathNode** heap, int heap_size, int index, int** open_list_index) {
    int min_index = index;
    int left_child = 2 * index + 1;
    int right_child = 2 * index + 2;

    if (left_child < heap_size && heap[left_child]->f < heap[min_index]->f) {
        min_index = left_child;
    }
    if (right_child < heap_size && heap[right_child]->f < heap[min_index]->f) {
        min_index = right_child;
    }

    if (min_index != index) {
        swap_nodes(&heap[index], &heap[min_index], open_list_index);
        heapify_down(heap, heap_size, min_index, open_list_index);
    }
}
void heap_push(PathNode** heap, int* heap_size, PathNode* node, int** open_list_index) {
    if (*heap_size < MAX_HEAP_SIZE) {
        heap[*heap_size] = node;
        (*heap_size)++;
      
        open_list_index[node->x][node->y] = *heap_size - 1;
        heapify_up(heap, *heap_size, *heap_size - 1, open_list_index);
    }
}
PathNode* heap_pop(PathNode** heap, int* heap_size, int** open_list_index) {
    if (*heap_size <= 0) return NULL;
    PathNode* min_node = heap[0];
    (*heap_size)--;
    if (*heap_size > 0) {
        heap[0] = heap[*heap_size];
     
        open_list_index[heap[0]->x][heap[0]->y] = 0;
        heapify_down(heap, *heap_size, 0, open_list_index);
    }
    return min_node;
}


path_t find_path(int start_x, int start_y, int target_x, int target_y, IsMovableFunc is_movable) {
    PathNode** all_allocated_nodes = (PathNode**)malloc(sizeof(PathNode*) * MAX_NODES_FOR_ASTAR_SEARCH);
    PathNode** open_list_heap = (PathNode**)malloc(sizeof(PathNode*) * MAX_HEAP_SIZE);

    int allocated_node_count = 0;
    int open_list_size = 0;

    bool** close_list_visited = (bool**)malloc(map.size.x * sizeof(bool*));
    bool** in_open_list_check = (bool**)malloc(map.size.x * sizeof(bool*));
    int** open_list_index = (int**)malloc(map.size.x * sizeof(int*));

    path_t result_path = { .count = 0, .current_index = 0 };
    bool path_found = false;

    if (!close_list_visited || !in_open_list_check || !open_list_index || !all_allocated_nodes || !open_list_heap) {
        fprintf(stderr, "A* 배열 메모리 할당 실패!\n");
        return result_path;
    }

    for (int i = 0; i < map.size.x; i++) {
        close_list_visited[i] = (bool*)malloc(map.size.y * sizeof(bool));
        in_open_list_check[i] = (bool*)malloc(map.size.y * sizeof(bool));
        open_list_index[i] = (int*)malloc(map.size.y * sizeof(int));
    }

    for (int i = 0; i < map.size.x; i++) {
        memset(close_list_visited[i], 0, map.size.y * sizeof(bool));
        memset(in_open_list_check[i], 0, map.size.y * sizeof(bool));
        memset(open_list_index[i], 0, map.size.y * sizeof(int));
    }

    PathNode* start_node = Create_node(start_x, start_y, 0, Calculate_Heuristic(start_x, start_y, target_x, target_y), NULL);
    heap_push(open_list_heap, &open_list_size, start_node, open_list_index);
    in_open_list_check[start_x][start_y] = true;
    all_allocated_nodes[allocated_node_count++] = start_node;

    while (open_list_size > 0 && allocated_node_count < MAX_NODES_FOR_ASTAR_SEARCH) {
        PathNode* current_node = heap_pop(open_list_heap, &open_list_size, open_list_index);

       
        in_open_list_check[current_node->x][current_node->y] = false;

        if (current_node->x == target_x && current_node->y == target_y) {
            PathNode* temp = current_node;
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
                    PathNode* existing_node = open_list_heap[existing_index];
                    if (new_g < existing_node->g) {
                        existing_node->g = new_g;
                        existing_node->f = existing_node->g + existing_node->h;
                        existing_node->parent = current_node;
                        heapify_up(open_list_heap, open_list_size, existing_index, open_list_index);
                    }
                }
                else {
                    PathNode* neighbor_node = Create_node(
                        neighbor_x, neighbor_y, new_g,
                        Calculate_Heuristic(neighbor_x, neighbor_y, target_x, target_y),
                        current_node
                    );

                    if (open_list_size < MAX_HEAP_SIZE && allocated_node_count < MAX_NODES_FOR_ASTAR_SEARCH) {
                        heap_push(open_list_heap, &open_list_size, neighbor_node, open_list_index);
                        in_open_list_check[neighbor_x][neighbor_y] = true;
                        all_allocated_nodes[allocated_node_count++] = neighbor_node;
                    }
                    else {
                        free(neighbor_node);
                    }
                }
            }
        }
    }

    for (int i = 0; i < allocated_node_count; ++i) {
        free(all_allocated_nodes[i]);
    }
    for (int i = 0; i < map.size.x; i++) {
        free(close_list_visited[i]);
        free(in_open_list_check[i]);
        free(open_list_index[i]);
    }
    free(all_allocated_nodes);
    free(open_list_heap);
    free(close_list_visited);
    free(in_open_list_check);
    free(open_list_index);

    return result_path;
}