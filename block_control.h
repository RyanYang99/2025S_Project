#pragma once

extern int block_control_selected_x, block_control_selected_y;

//모듈 초기화: 반드시 main에서 호출
void block_control_initialize(void);
void block_control_render(void);
//모듈 정리: 프로그램 종료 전 호출
void block_control_destroy(void);