#pragma once

#include <Windows.h>
#include "map.h"
#include "input.h"

//�����, �׽�Ʈ �뵵; ����� �̿ܿ� �ʿ�� BlockCtrl.c�� ����� block_x/y ���.
#if _DEBUG
extern int selected_block_x, selected_block_y;
#endif

// ��� �ʱ�ȭ: �ݵ�� main()���� ȣ��
void BlockControl_Init(void);

// ��� ����: ���α׷� ���� �� ȣ��
void BlockControl_Destroy(void);

void render_virtual_cursor(void);