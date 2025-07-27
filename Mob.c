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

int mob_count = 0; // 몹의 개체수 확인
int mob_level = 1; // 난이도 (시간비례 증가)  

long startTime = 0; // 게임 시작 시간
long totalElapsedTime = 0; // 총 경과 시간
long last_spawn_time = 0;// 마지막 스폰시간


COORD console_c() // 콘솔 중앙 찾기
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

	if (spawnTimer >= 5.0f) // 소환 간격 설정(초 단위)
	{
		spawnTimer = 0;

		if (mob_count < Max_Mob) //일정수 이상 몹생성 제한
		{
			MobSpawn(player.x, player.y); //플레이어 좌표를 받아 플레이어 기준으로 생성
		}
	}

	if (totalElapsedTime == 100 && mob_level < 10) // 난도 조정용 임시 생성
	{
		mob_level++;
	}
}

void MobSpawn(int player_x, int player_y)
{
	srand((unsigned int)time(NULL)); // 랜덤값 초기화 

	for (int check_x = player_x - 50; check_x <= player_x + 50; check_x++) //플레이어 주변 x값 탐색
	{
		for (int check_y = player_y - 50; check_y <= player_y + 50; check_y++) //플레이어 주변 y값 탐색
		{
			if (check_x >= 0 && check_x < map.size.x && check_y >= 0 && check_y + 1 < map.size.y) // 맵 배열 초과 확인(초과하여 탐색시 예외발생 방지용)
			{
				if (map.ppBlocks[check_y + 1][check_x].type == BLOCK_GRASS) //풀 자체가 땅위에 생성 되므로 풀 위이기만 하면 생성 되도록 설정
				{
					int Spawn_r = rand() % 50; // 스폰 확률
					if (Spawn_r == 0)
					{
						mobs[mob_count].x = check_x; // 탐색으로 조건이 맞는 좌표값 저장
						mobs[mob_count].y = check_y;
						mobs[mob_count].HP = mob_level * 5;
						mobs[mob_count].atk = mob_level * 2; //이후 추가 (난이도에 따라 증가)
						mobs[mob_count].last_move_time = clock();
						mob_count++;
					}
				}
			}

		}
	}

}



void Mob_render() // 몹 렌더링
{

	COORD center_m = console_c();

	for (int i = 0; i < mob_count; i++)
	{
		int mob_x = center_m.X + (mobs[i].x - player.x) * 3; // 플레이어 움직임 따라 몹 위치도 맵에 맞게 변경
		int mob_y = center_m.Y + (mobs[i].y - player.y) * 3; // 플레이어 움직임과 동일하게 설정 하지 않으면 소환위치에 문제발생(조건에 맞지 ㅇ낳는 위치에 생성)

		// 화면 밖 몹은 그리지 않음
		if (mob_x < 0 || mob_x >= console.size.X || mob_y < 0 || mob_y >= console.size.Y)
			continue;

		COORD Mobpos = { (SHORT)mob_x, (SHORT)mob_y }; // 저장한 소환가능 좌표값 받음
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
