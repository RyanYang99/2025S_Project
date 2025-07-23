#pragma once

#include <Windows.h>
#include "map.h"
#include "input.h"

// 모듈 초기화: 반드시 main()에서 호출
void BlockControl_Init(void);

// 모듈 정리: 프로그램 종료 전 호출
void BlockControl_Destroy(void);

void render_virtual_cursor(void);

