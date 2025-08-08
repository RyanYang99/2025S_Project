#include "leak.h"
#include "delta.h"
#include "BossMalakh.h"
#include "console.h"
#include "input.h"
#include "map.h"
#include "player.h"
#include "astar.h"
#include "Mob.h"
#include <math.h>
#include <stdlib.h>
#include "BlockCtrl.h"

// 전역 변수와 구조체 선언
BossMalakh boss;
Missile missiles[MAX_MISSILES];
BossDamageText boss_damage_texts[MAX_BOSS_DAMAGE_TEXTS];

// 보스 데미지 텍스트 관련
#define MAX_BOSS_DAMAGE_TEXTS 10
#define BOSS_DAMAGE_TEXT_DURATION 1.0f

// 전역 변수 외부 선언
extern map_t map;
extern player_t player;
extern console_t console;
extern block_info_t get_block_info_at(int x, int y);
extern void player_take_damage(int damage);
extern float delta_time;
extern int selected_block_x;
extern int selected_block_y;
extern bool is_boss_spawned;

// 가독성을 위한 매크로
#define FG_WHITE FOREGROUND_T_WHITE
#define FG_YELLOW FOREGROUND_T_YELLOW
#define FG_CYAN FOREGROUND_T_CYAN
#define FG_MAGENTA FOREGROUND_T_MAGENTA
#define FG_RED FOREGROUND_T_RED
#define FG_BLUE FOREGROUND_T_BLUE
#define BG_BLUE BACKGROUND_T_BLUE
#define BG_WHITE BACKGROUND_T_WHITE
#define BG_RED BACKGROUND_T_RED
#define BG_YELLOW BACKGROUND_T_YELLOW
#define BG_BLACK BACKGROUND_T_BLACK

#define BOSS_SPRITE_WIDTH 20
#define BOSS_SPRITE_HEIGHT 20
#define BOSS_DRAW_SCALE 3


// 보스 스프라이트 데이터 (생략)
static const color_tchar_t boss_malakh_sprite_data[BOSS_SPRITE_HEIGHT][BOSS_SPRITE_WIDTH] = {
	// 0        1          2	      3		     4		    5		   6		  7			 8			9		   10		  11		 12			13		   14		  15		 16			17		   18		  19 
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//1	
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_YELLOW,FG_YELLOW},{L'█',FG_YELLOW,FG_YELLOW},{L'█',FG_YELLOW,FG_YELLOW},{L'█',FG_YELLOW,FG_YELLOW},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//2
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_YELLOW,FG_YELLOW},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_YELLOW,FG_YELLOW},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//3
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_BLUE,FG_BLUE},{L'█',FG_YELLOW,FG_YELLOW},{L'█',FG_YELLOW,FG_YELLOW},{L'█',FG_BLUE,FG_BLUE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//4
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//5
	{{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L'█',FG_CYAN,FG_CYAN},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{ L'█',BG_BLACK,BG_BLACK },{ L'█', FG_RED, FG_RED },{ L'█',BG_BLACK,BG_BLACK },{ L'█',BG_BLACK,BG_BLACK },{ L'█', FG_RED, FG_RED },{ L'█',BG_BLACK,BG_BLACK },{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L'█',FG_CYAN,FG_CYAN},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0}},
	//6	
	{{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{ L'█',BG_BLACK,BG_BLACK}, { L'█',BG_BLACK,BG_BLACK },{ L'█',BG_BLACK,BG_BLACK },{ L'█',BG_BLACK,BG_BLACK },{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE} ,{L' ',0,0},{L' ',0,0},{L' ',0,0} ,{L'█',FG_WHITE,FG_WHITE},{L' ',0,0}},
	//7	
	{{L'█',FG_WHITE,FG_WHITE},{L'█',FG_WHITE,FG_WHITE},{L'█',FG_CYAN,FG_CYAN},{L'█',FG_WHITE,FG_WHITE},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{ L'█',BG_BLACK,BG_BLACK},{ L'█',BG_BLACK,BG_BLACK}, { L'█',FG_YELLOW,FG_YELLOW },{ L'█',FG_YELLOW,FG_YELLOW },{ L'█',BG_BLACK,BG_BLACK },{ L'█',BG_BLACK,BG_BLACK },{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L'█',FG_WHITE,FG_WHITE},{L'█',FG_CYAN,FG_CYAN},{L'█',FG_WHITE,FG_WHITE},{L'█',FG_WHITE,FG_WHITE}},
	//8
	{{L' ',0,0},{L' ',0,0},{L'█',BG_WHITE,BG_WHITE},{L'█',FG_CYAN,FG_CYAN},{L'█',BG_WHITE,BG_WHITE},{L'█',BG_WHITE,BG_WHITE},{L' ',0,0},{ L'█',BG_BLACK,BG_BLACK},{ L'█',BG_BLACK,BG_BLACK}, { L'█',FG_RED,FG_RED },{ L'█',FG_RED,FG_RED },{ L'█',BG_BLACK,BG_BLACK },{ L'█',BG_BLACK,BG_BLACK },{L' ',0,0},{L'█',BG_WHITE,BG_WHITE},{L'█',FG_WHITE,FG_WHITE} ,{L'█',FG_CYAN,FG_CYAN},{L'█',FG_WHITE,FG_WHITE} ,{L' ',0,0} ,{L' ',0,0}},
	//9
	{{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{ L'█',BG_BLACK,BG_BLACK}, { L'█',BG_BLACK,BG_BLACK },{ L'█',BG_BLACK,BG_BLACK },{ L'█',BG_BLACK,BG_BLACK },{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE} ,{L' ',0,0},{L' ',0,0},{L' ',0,0} ,{L'█',FG_WHITE,FG_WHITE},{L' ',0,0}},
	//10	
	{{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{ L'█',BG_BLACK,BG_BLACK },{ L'█', FG_RED, FG_RED },{ L'█',BG_BLACK,BG_BLACK },{ L'█',BG_BLACK,BG_BLACK },{ L'█', FG_RED, FG_RED },{ L'█',BG_BLACK,BG_BLACK },{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}, { L'█',FG_WHITE,FG_WHITE }},
	//11
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//12
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//13
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//14
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//15
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L'█',FG_WHITE,FG_WHITE},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//16
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//17
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//18
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}},
	//19
	{{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0},{L' ',0,0}}
};
;


// 미사일이 이동 가능한 블록인지 체크하는 함수
static bool is_missile_movable(int x, int y) {
	if (x < 0 || x >= map.size.x || y < 0 || y >= map.size.y) {
		return false;
	}
	block_info_t block = get_block_info_at(x, y);
	return block.type != BLOCK_STONE && block.type != BLOCK_WATER;
}

// 보스 전용 문자열 출력 함수
static void Boss_Print_String_Color_W(const wchar_t* wstr, COORD position, BACKGROUND_color_t background_color, FOREGROUND_color_t foreground_color) {
	COORD current_pos = position;
	for (int i = 0; wstr[i] != L'\0'; ++i) {
		print_color_tchar((color_tchar_t) { wstr[i], background_color, foreground_color }, current_pos);
		current_pos.X++;
	}
}

// 보스 데미지 텍스트 생성 함수
static void create_boss_damage_text(const int damage_value) {
	for (int i = 0; i < MAX_BOSS_DAMAGE_TEXTS; ++i) {
		if (!boss_damage_texts[i].active) {
			boss_damage_texts[i].active = true;
			boss_damage_texts[i].damage_value = damage_value;
			boss_damage_texts[i].precise_x = (float)boss.x + (float)(rand() % (BOSS_SPRITE_WIDTH - 1));
			boss_damage_texts[i].precise_y = (float)boss.y + (float)(rand() % (BOSS_SPRITE_HEIGHT - 1));
			boss_damage_texts[i].timer = BOSS_DAMAGE_TEXT_DURATION;
			break;
		}
	}
}

// 보스 대미지 텍스트 업데이트 함수
static void update_boss_damage_texts(void) {
	for (int i = 0; i < MAX_BOSS_DAMAGE_TEXTS; ++i) {
		if (boss_damage_texts[i].active) {
			boss_damage_texts[i].precise_y -= delta_time * 5.0f;
			boss_damage_texts[i].timer -= delta_time;
			if (boss_damage_texts[i].timer <= 0.0f) {
				boss_damage_texts[i].active = false;
			}
		}
	}
}

// 보스 대미지 텍스트 렌더링 함수
static void render_boss_damage_texts(void) {
	const COORD center_pos = {
		.X = console.size.X / 2,
		.Y = console.size.Y / 2
	};

	for (int i = 0; i < MAX_BOSS_DAMAGE_TEXTS; ++i) {
		if (boss_damage_texts[i].active) {
			const COORD draw_pos = {
				.X = (SHORT)(center_pos.X + (boss_damage_texts[i].precise_x - player.x)),
				.Y = (SHORT)(center_pos.Y + (boss_damage_texts[i].precise_y - player.y))
			};

			fprint_string("Attack! -%d", draw_pos, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE, boss_damage_texts[i].damage_value);
		}
	}
}

// 보스 업데이트
void Boss_update()
{
	Boss_Update_Ai();
	Update_Missiles();
	update_boss_damage_texts();
}


// 보스 초기화
void Boss_Init(int start_x, int start_y, int init_hp, int attack_power)
{
	boss.x = start_x;
	boss.y = start_y;

	boss.hp = init_hp;
	boss.Max_hp = init_hp;
	boss.atk = attack_power;
	boss.state = E_BOSS_STATE_PHASE1;

	boss.action_timer = 0.0f;
	boss.missile_timer = 0.0f;
	boss.horizontal_laser_timer = 0.0f;
	boss.vertical_laser_timer = 0.0f;

	boss.missile_attack_cooltime = 4.0f;
	boss.horizontal_laser_cooltime = 3.0f;
	boss.vertical_laser_cooltime = 5.0f;

	boss.is_horizontal_laser_active = false;
	boss.is_vertical_laser_active = false;


	for (int y = 0; y < BOSS_SPRITE_HEIGHT; y++) {
		for (int x = 0; x < BOSS_SPRITE_WIDTH; x++) {
			boss.sprite_data[y][x] = boss_malakh_sprite_data[y][x];
		}
	}

	for (int i = 0; i < MAX_MISSILES; i++) {
		missiles[i].is_active = false;
		missiles[i].move_timer = 0.0f;
	}

	for (int i = 0; i < MAX_BOSS_DAMAGE_TEXTS; ++i) {
		boss_damage_texts[i].active = false;
	}
	subscribe_mouse_click(handle_boss_click);
}

// 보스 렌더링
void Boss_Render()
{
	if (boss.state == E_BOSS_STATE_DEFEATED) return;

	COORD center_m = { console.size.X / 2 , console.size.Y / 2 };

	int boss_screen_base_x = center_m.X + (boss.x - player.x) * BOSS_DRAW_SCALE - (BOSS_SPRITE_WIDTH * BOSS_DRAW_SCALE / 2);
	int boss_screen_base_y = center_m.Y + (boss.y - player.y) * BOSS_DRAW_SCALE - (BOSS_SPRITE_HEIGHT * BOSS_DRAW_SCALE / 2);

	// 스프라이트 렌더링
	for (int y_offset = 0; y_offset < BOSS_SPRITE_HEIGHT; ++y_offset) {
		for (int x_offset = 0; x_offset < BOSS_SPRITE_WIDTH; ++x_offset) {
			for (int py = 0; py < BOSS_DRAW_SCALE; ++py) {
				for (int px = 0; px < BOSS_DRAW_SCALE; ++px) {
					COORD char_pos = { (SHORT)(boss_screen_base_x + (x_offset * BOSS_DRAW_SCALE) + px), (SHORT)(boss_screen_base_y + (y_offset * BOSS_DRAW_SCALE) + py) };
					if (char_pos.X >= 0 && char_pos.X < console.size.X &&
						char_pos.Y >= 0 && char_pos.Y < console.size.Y) {
						color_tchar_t char_to_print = boss.sprite_data[y_offset][x_offset];
						if (char_to_print.character != L' ') {
							print_color_tchar(char_to_print, char_pos);
						}
					}
				}
			}
		}
	}

	// 보스 체력 바 렌더링
	wchar_t hp_text[50];
	swprintf(hp_text, sizeof(hp_text) / sizeof(wchar_t), L"HP: %d/%d", boss.hp, boss.Max_hp);
	COORD hp_pos = { (SHORT)(boss_screen_base_x + (BOSS_SPRITE_WIDTH * BOSS_DRAW_SCALE / 2) - (int)(wcslen(hp_text) / 2.0)), (SHORT)(boss_screen_base_y - 1) };
	if (hp_pos.X < 0) hp_pos.X = 0;
	if (hp_pos.Y < 0) hp_pos.Y = 0;
	Boss_Print_String_Color_W(hp_text, hp_pos, BACKGROUND_T_BLACK, FOREGROUND_T_WHITE);

	// 패턴 렌더링
	Boss_Render_Pattern();
	render_boss_damage_texts();
}

// 보스 AI 업데이트
void Boss_Update_Ai()
{
	if (boss.state == E_BOSS_STATE_DEFEATED) return;

	// 페이즈 전환 로직
	if (boss.hp <= boss.Max_hp * 0.3 && boss.state < E_BOSS_STATE_PHASE3)
	{
		boss.state = E_BOSS_STATE_PHASE3;
		boss.action_timer = 0.0f;
		boss.atk = boss.atk * 2;
		boss.missile_attack_cooltime = 1.5f;
		boss.vertical_laser_cooltime = 4.0f;
	}
	else if (boss.hp <= boss.Max_hp * 0.6 && boss.state < E_BOSS_STATE_PHASE2)
	{
		boss.state = E_BOSS_STATE_PHASE2;
		boss.action_timer = 0.0f;
		boss.atk = (int)(boss.atk * 1.5f);
		boss.missile_attack_cooltime = 2.0f;
		boss.horizontal_laser_cooltime = 3.0f;
	}
	else if (boss.hp > boss.Max_hp * 0.6 && boss.state != E_BOSS_STATE_PHASE1)
	{
		boss.state = E_BOSS_STATE_PHASE1;
		boss.action_timer = 0.0f;
		boss.missile_attack_cooltime = 4.0f;
	}

	if (boss.is_horizontal_laser_active && abs(boss.y - player.y) <= 1) {
		player_take_damage(boss.atk * 2);
	}
	if (boss.is_vertical_laser_active && abs(boss.x - player.x) <= 1) {
		player_take_damage(boss.atk * 2);
	}


	Boss_Update_Pattern();


	switch (boss.state) {
	case E_BOSS_STATE_DAMAGED: {
		boss.action_timer += delta_time;
		if (boss.action_timer >= 0.2f) {
			if (boss.hp > boss.Max_hp * 0.6) boss.state = E_BOSS_STATE_PHASE1;
			else if (boss.hp > boss.Max_hp * 0.3) boss.state = E_BOSS_STATE_PHASE2;
			else boss.state = E_BOSS_STATE_PHASE3;
		}
		break;
	}
	case E_BOSS_STATE_DEFEATED:
		if (is_boss_spawned) {
			is_boss_spawned = false;
		}
		break;
	default:
		break;
	}
}

// 보스 패턴 업데이트 및 발동 로직
void Boss_Update_Pattern() {
	if (boss.state == E_BOSS_STATE_DEFEATED) return;

	// 타이머 누적
	boss.missile_timer += delta_time;
	boss.horizontal_laser_timer += delta_time;
	boss.vertical_laser_timer += delta_time;

	// 미사일 패턴 (모든 페이즈)
	if (boss.missile_timer >= boss.missile_attack_cooltime) {
		int missile_count_to_launch = 1;
		if (boss.state == E_BOSS_STATE_PHASE1) missile_count_to_launch = 3;
		else if (boss.state == E_BOSS_STATE_PHASE2) missile_count_to_launch = 5;
		else if (boss.state == E_BOSS_STATE_PHASE3) missile_count_to_launch = 10;

		Boss_Launch_Missile(missile_count_to_launch);
		boss.missile_timer = 0.0f; // 타이머 리셋
	}

	// 페이즈 2: 가로 레이저 패턴
	if (boss.state == E_BOSS_STATE_PHASE2) {
		if (boss.horizontal_laser_timer >= boss.horizontal_laser_cooltime) {
			Boss_Launch_New_Horizontal_Laser(); // 새로운 가로 레이저 패턴 호출
			boss.horizontal_laser_timer = 0.0f;
		}
	}

	// 페이즈 3: 세로 레이저 패턴
	if (boss.state == E_BOSS_STATE_PHASE3) {
		if (boss.vertical_laser_timer >= boss.vertical_laser_cooltime) {
			Boss_Launch_New_Vertical_Laser(); // 새로운 세로 레이저 패턴 호출
			boss.vertical_laser_timer = 0.0f;
		}
	}
}

// 미사일 발사 함수
void Boss_Launch_Missile(int count) {
	int launched_count = 0;
	for (int i = 0; i < MAX_MISSILES && launched_count < count; i++) {
		if (!missiles[i].is_active) {
			missiles[i].x = boss.x;
			missiles[i].y = boss.y;
			missiles[i].is_active = true;
			launched_count++;
		}
	}
}

// 미사일 업데이트 함수 
void Update_Missiles() {
	float missile_speed = 5.0f; // 초당 5타일 이동
	for (int i = 0; i < MAX_MISSILES; i++) {
		if (missiles[i].is_active) {
			missiles[i].move_timer += delta_time;

			// 일정한 속도로 타일 이동
			if (missiles[i].move_timer >= 1.0f / missile_speed) {

				direction_t next_direction = find_next_direction(missiles[i].x, missiles[i].y, player.x, player.y, is_missile_movable);

				switch (next_direction) {
				case DIRECTION_UP: missiles[i].y--; break;
				case DIRECTION_DOWN: missiles[i].y++; break;
				case DIRECTION_LEFT: missiles[i].x--; break;
				case DIRECTION_RIGHT: missiles[i].x++; break;
				case DIRECTION_NONE: break;
				}

				missiles[i].move_timer -= 1.0f / missile_speed;
			}

			// 플레이어와 충돌 체크
			if (missiles[i].x == player.x && missiles[i].y == player.y) {
				player_take_damage(boss.atk);
				missiles[i].is_active = false;
			}

			// 블록과 충돌 체크
			if (!is_missile_movable(missiles[i].x, missiles[i].y)) {
				missiles[i].is_active = false;
			}
		}
	}
}

// 미사일 렌더링
void Render_Missiles() {
	for (int i = 0; i < MAX_MISSILES; i++) {
		if (missiles[i].is_active) {
			int missile_screen_x = console.size.X / 2 + (missiles[i].x - player.x) * BOSS_DRAW_SCALE;
			int missile_screen_y = console.size.Y / 2 + (missiles[i].y - player.y) * BOSS_DRAW_SCALE;


			COORD missile_pos = {
				(SHORT)missile_screen_x,
				(SHORT)missile_screen_y
			};
			if (missile_pos.X >= 0 && missile_pos.X < console.size.X &&
				missile_pos.Y >= 0 && missile_pos.Y < console.size.Y) {
				print_color_tchar((color_tchar_t) { L'*', BG_BLACK, FG_RED }, missile_pos);
			}
		}
	}
}

// 가로 레이저 발사 함수
void Boss_Launch_Horizontal_Laser() {
	boss.is_horizontal_laser_active = true;
	boss.action_timer = 0.0f;
}

// 하늘 레이저 발사 함수
void Boss_Launch_Vertical_Laser() {
	boss.is_vertical_laser_active = true;
	boss.action_timer = 0.0f;
}


void Boss_Launch_New_Horizontal_Laser() {
	boss.is_horizontal_laser_active = true;
	boss.action_timer = 0.0f;
	// 보스 머리 위부터 시작
	boss.current_horizontal_laser_y = boss.y - BOSS_SPRITE_HEIGHT / 2;
}

// 세로 레이저 패턴 시작 함수
void Boss_Launch_New_Vertical_Laser() {
	boss.is_vertical_laser_active = true;
	boss.action_timer = 0.0f;
	// 방향을 랜덤으로 결정
	boss.is_vertical_laser_from_left = (rand() % 2 == 0);
	// 시작 위치 설정
	if (boss.is_vertical_laser_from_left) {
		// 왼쪽에서 시작
		boss.current_vertical_laser_x = 0;
	}
	else {
		// 오른쪽에서 시작
		boss.current_vertical_laser_x = map.size.x - 1;
	}
}



// 패턴 렌더링 함수
void Boss_Render_Pattern() {


	// 미사일 렌더링
	Render_Missiles();


	// 페이즈 2: 가로 레이저 (보스 머리부터 차례대로)
	if (boss.is_horizontal_laser_active) {
		// 0.2초마다 한 줄씩 아래로 이동
		if (boss.action_timer > 0.2f) {
			boss.current_horizontal_laser_y++;
			boss.action_timer = 0.0f;
		}
		boss.action_timer += delta_time;

		// 보스 스프라이트를 벗어나면 패턴 종료
		if (boss.current_horizontal_laser_y > boss.y + BOSS_SPRITE_HEIGHT / 2) {
			boss.is_horizontal_laser_active = false;
		}

		// 3줄 레이저를 한번에 출력
		for (int i = 0; i < 3; ++i) {
			int laser_y = console.size.Y / 2 + ((boss.current_horizontal_laser_y + i) - player.y) * BOSS_DRAW_SCALE;
			for (int x = 0; x < console.size.X; ++x) {
				COORD laser_pos = { (SHORT)x, (SHORT)laser_y };
				print_color_tchar((color_tchar_t) { L'═', BG_BLACK, FG_RED }, laser_pos);
			}
		}
	}

	// 페이즈 3: 세로 레이저 (랜덤 방향에서 3줄씩 한번에)
	if (boss.is_vertical_laser_active) {
		// 0.2초마다 한 줄씩 이동
		if (boss.action_timer > 0.2f) {
			if (boss.is_vertical_laser_from_left) {
				boss.current_vertical_laser_x++;
			}
			else {
				boss.current_vertical_laser_x--;
			}
			boss.action_timer = 0.0f;
		}
		boss.action_timer += delta_time;

		// 화면을 벗어나면 패턴 종료
		if (boss.is_vertical_laser_from_left && boss.current_vertical_laser_x > map.size.x ||
			!boss.is_vertical_laser_from_left && boss.current_vertical_laser_x < 0) {
			boss.is_vertical_laser_active = false;
		}

		// 3줄 레이저를 한번에 출력
		for (int i = 0; i < 3; ++i) {
			int laser_x = console.size.X / 2 + ((boss.current_vertical_laser_x + i) - player.x) * BOSS_DRAW_SCALE;
			if (!boss.is_vertical_laser_from_left) {
				laser_x = console.size.X / 2 + ((boss.current_vertical_laser_x - i) - player.x) * BOSS_DRAW_SCALE;
			}
			for (int y = 0; y < console.size.Y; ++y) {
				COORD laser_pos = { (SHORT)laser_x, (SHORT)y };
				print_color_tchar((color_tchar_t) { L'║', BG_BLACK, FG_RED }, laser_pos);
			}
		}
	}
}



static void handle_boss_click(const bool left_click)
{
	// 보스가 패배했거나, 왼쪽 클릭이 아니면 리턴
	if (boss.state == E_BOSS_STATE_DEFEATED || !left_click)
	{
		return;
	}

	// 보스가 생성되지 않았으면 리턴
	if (!is_boss_spawned) {
		return;
	}

	// 마우스 클릭 좌표와 보스 스프라이트 범위 비교
	// 보스의 좌표(boss.x, boss.y)가 중심점이라고 가정하고 충돌 판정 영역을 재계산합니다.
	int boss_left = boss.x - (BOSS_SPRITE_WIDTH / 2);
	int boss_top = boss.y - (BOSS_SPRITE_HEIGHT / 2);
	int boss_right = boss.x + (BOSS_SPRITE_WIDTH / 2);
	int boss_bottom = boss.y + (BOSS_SPRITE_HEIGHT / 2);

	if (selected_block_x >= boss_left && selected_block_x < boss_right &&
		selected_block_y >= boss_top && selected_block_y < boss_bottom)
	{
		// 보스에게 데미지 입히는 부분
		Boss_Take_Damage(player.atk_power);
		return;
	}
}



// 보스 데미지 입히는 함수
void Boss_Take_Damage(int damage)
{
	if (boss.state == E_BOSS_STATE_DEFEATED) return;

	boss.hp -= damage;
	create_boss_damage_text(damage);
	if (boss.hp <= 0)
	{
		boss.hp = 0;
		boss.state = E_BOSS_STATE_DEFEATED;
	}
	else
	{
		// 보스가 데미지를 입었을 때 상태 변경 (피격 애니메이션 등을 위해)
		boss.state = E_BOSS_STATE_DAMAGED;
		boss.action_timer = 0.0f;
	}
}