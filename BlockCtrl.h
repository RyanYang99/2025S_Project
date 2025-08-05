#pragma once

#include <Windows.h>
#include "map.h"
#include "input.h"

//디버깅, 테스트 용도; 디버깅 이외에 필요시 BlockCtrl.c에 선언된 block_x/y 사용
#if _DEBUG
extern int selected_block_x, selected_block_y;
#endif

//모듈 초기화: 반드시 main()에서 호출
void initialize_block_control(void);

void render_virtual_cursor(void);


//모듈 정리: 프로그램 종료 전 호출
void destroy_block_control(void);