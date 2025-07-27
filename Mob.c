#include "leak.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <stdbool.h>
#include <math.h>


#include "Mob.h"
#include "player.h"
#include "map.h"
#include "console.h"
#include "astar.h"
#include "delta.h"


Mob mobs[Max_Mob];

int mob_count = 0; // ���� ��ü�� Ȯ��
int mob_level = 1; // ���̵� (�ð���� ����)  

long startTime = 0; // ���� ���� �ð�
long totalElapsedTime = 0; // �� ��� �ð�
long last_spawn_time = 0;// ������ �����ð�


COORD console_c() // �ܼ� �߾� ã��
{
	COORD center_m;
	center_m.X = console.size.X / 2;
	center_m.Y = console.size.Y / 2;
	return center_m;
}

void Mob_Spawn_Time()
{
	static float spawnTimer = 0.0f, totalElapsedTime = 0.0f;
	spawnTimer += delta_time;
	totalElapsedTime += delta_time;

	if (spawnTimer >= 5.0f) // ��ȯ ���� ����(�� ����)
	{
		spawnTimer = 0;

		if (mob_count < Max_Mob) //������ �̻� ������ ����
		{
			MobSpawn(player.x, player.y); //�÷��̾� ��ǥ�� �޾� �÷��̾� �������� ����
		}
	}

	if (totalElapsedTime == 100 && mob_level < 10) // ���� ������ �ӽ� ����
	{
		mob_level++;
	}
}

void MobSpawn(int player_x, int player_y)
{
	srand((unsigned int)time(NULL)); // ������ �ʱ�ȭ 

	for (int check_x = player_x - 50; check_x <= player_x + 50; check_x++) //�÷��̾� �ֺ� x�� Ž��
	{
		for (int check_y = player_y - 50; check_y <= player_y + 50; check_y++) //�÷��̾� �ֺ� y�� Ž��
		{
			if (check_x >= 0 && check_x < map.size.x && check_y >= 0 && check_y + 1 < map.size.y) // �� �迭 �ʰ� Ȯ��(�ʰ��Ͽ� Ž���� ���ܹ߻� ������)
			{
				if (map.ppBlocks[check_y + 1][check_x].type == BLOCK_GRASS) //Ǯ ��ü�� ������ ���� �ǹǷ� Ǯ ���̱⸸ �ϸ� ���� �ǵ��� ����
				{
					int Spawn_r = rand() % 50; // ���� Ȯ��
					if (Spawn_r == 0)
					{
						mobs[mob_count].x = check_x; // Ž������ ������ �´� ��ǥ�� ����
						mobs[mob_count].y = check_y;
						mobs[mob_count].HP = mob_level * 5;
						mobs[mob_count].atk = mob_level * 2; //���� �߰� (���̵��� ���� ����)
						mobs[mob_count].last_move_time = clock();
						mob_count++;
					}
				}
			}

		}
	}

}



void Mob_render() // �� ������
{

	COORD center_m = console_c();

	for (int i = 0; i < mob_count; i++)
	{
		int mob_x = center_m.X + (mobs[i].x - player.x) * 3; // �÷��̾� ������ ���� �� ��ġ�� �ʿ� �°� ����
		int mob_y = center_m.Y + (mobs[i].y - player.y) * 3; // �÷��̾� �����Ӱ� �����ϰ� ���� ���� ������ ��ȯ��ġ�� �����߻�(���ǿ� ���� ������ ��ġ�� ����)

		// ȭ�� �� ���� �׸��� ����
		if (mob_x < 0 || mob_x >= console.size.X || mob_y < 0 || mob_y >= console.size.Y)
			continue;

		COORD Mobpos = { (SHORT)mob_x, (SHORT)mob_y }; // ������ ��ȯ���� ��ǥ�� ����
		color_tchar_t mob_c =
		{
			'@',
			BACKGROUND_T_BLACK,
			FOREGROUND_T_RED };
		print_color_tchar(mob_c, Mobpos);
	}

}

void update_mob_ai() {
	long current_time = clock();
	for (int i = 0; i < mob_count; ++i) {
		if ((current_time - mobs[i].last_move_time) / (double)CLOCKS_PER_SEC >= 0.5) {
			
			COORD next_pos = find_path_next_step(mobs[i].x, mobs[i].y, player.x, player.y, map.size.x, map.size.y);

			if (next_pos.X != -1 && next_pos.Y != -1) {
				mobs[i].x = next_pos.X;
				mobs[i].y = next_pos.Y;
			}
			mobs[i].last_move_time = current_time;
		}
	}
}
