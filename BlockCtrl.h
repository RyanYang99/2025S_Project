#pragma once

#include <Windows.h>
#include "map.h"
#include "input.h"

// ��� �ʱ�ȭ: �ݵ�� main()���� ȣ��
void BlockControl_Init(void);

// ��� ����: ���α׷� ���� �� ȣ��
void BlockControl_Destroy(void);

void render_virtual_cursor(void);

