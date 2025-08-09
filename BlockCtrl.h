#pragma once

#include <Windows.h>
#include "map.h"
#include "input.h"

extern int selected_block_x, selected_block_y;

//모듈 초기화: 반드시 main()에서 호출
void initialize_block_control(void);

void render_virtual_cursor(void);

//모듈 정리: 프로그램 종료 전 호출
void destroy_block_control(void);