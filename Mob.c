#include "leak.h"
#include "Mob.h"

#include "save.h"
#include "player.h"
#include "map.h"
#include "astar.h"
#include "delta.h"
#include "BlockCtrl.h"

#define GRAVITY 25.0f
#define BG_BLACK BACKGROUND_T_BLACK
#define FG_WHITE FOREGROUND_T_WHITE
#define FG_YELLOW FOREGROUND_T_YELLOW
#define FG_CYAN FOREGROUND_T_CYAN
#define FG_MAGENTA FOREGROUND_T_MAGENTA
#define FG_RED FOREGROUND_T_RED
#define FG_GREEN FOREGROUND_T_GREEN
#define FG_DARKYELLOW FOREGROUND_T_DARKYELLOW
#define FG_DARKGREEN FOREGROUND_T_DARKGREEN
#define FG_BLACK FOREGROUND_T_BLACK
#define FG_GRAY FOREGROUND_T_GRAY

#define MOB_SPRITE_WIDTH 5
#define MOB_SPRITE_HEIGHT 5
#define MOB_ATK_COOLTIME 2.0f

#define PLAYER_SPRITE_WIDTH 5
#define PLAYER_SPRITE_HEIGHT 5

#define MOB_SPD 0.2f
#define JUMP_SPD -12.0f

typedef struct {
	int damage_value, mob_x, mob_y;
	float precise_y;
	float timer;
	bool active;
} MobDamageText;

char mob_debug_message[MAX_MOB_DEBUG_MESSAGE_LEN] = "";

static int mob_count = 0, mob_level = 1;
static Mob mobs[MAX_MOB] = { 0 };

//idle 상태
const static color_tchar_t zombie_sprite_data[MOB_SPRITE_HEIGHT][MOB_SPRITE_WIDTH] = {
	{ {L' ',0,0}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L' ',0,0} },
	{ {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0} },
	{ {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN} },
	{ {L' ',0,0}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L' ',0,0} },
	{ {L' ',0,0}, {L'█', BG_BLACK, FG_GREEN}, {L' ',0,0}, {L'█', BG_BLACK, FG_GREEN}, {L' ',0,0} }
};

//걷는 상태
const static color_tchar_t zombie_sprite_walk[2][MOB_SPRITE_HEIGHT][MOB_SPRITE_WIDTH] = {
	// 프레임 0: 
	{
		{ {L' ',0,0}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L' ',0,0} }, // 머리
		{ {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0} }, // 팔과 눈
		{ {L'█', BG_BLACK, FG_GREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L' ',0,0} },
		{ {L' ',0,0}, {L'▓', BG_BLACK, FG_DARKYELLOW}, {L'▓', BG_BLACK, FG_DARKYELLOW}, {L' ',0,0}, {L' ',0,0} },
		{ {L' ',0,0}, {L'█', BG_BLACK, FG_GREEN},{L' ',0,0}, {L' ',0,0}, {L' ',0,0} }
	},
	// 프레임 1 
	{
		{ {L' ',0,0}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L'▀', BG_BLACK, FG_RED}, {L' ',0,0} }, // 머리
		{ {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0}, {L'o', BG_BLACK, FG_WHITE}, {L' ',0,0} }, // 팔과 눈
		{ {L' ',0,0}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKGREEN},{L'█', BG_BLACK, FG_GREEN}},
		{ {L' ',0,0}, {L'▓', BG_BLACK, FG_DARKYELLOW}, {L'▓', BG_BLACK, FG_DARKYELLOW}, {L' ',0,0}, {L' ',0,0} },
		{ {L'█', BG_BLACK, FG_GREEN}, {L' ',0,0}, {L'█', BG_BLACK, FG_GREEN}, {L' ',0,0}, {L' ',0,0} }
	}

};


//죽었을때
const static color_tchar_t dead_zombie_sprite_data[MOB_SPRITE_HEIGHT][MOB_SPRITE_WIDTH] = {
	{ {L' ',0,0}, {L' ',0,0}, {L'█', BG_BLACK, FG_DARKGREEN}, {L' ',0,0}, {L' ',0,0} },
	{ {L'▀', BG_BLACK, FG_RED}, {L'o', BG_BLACK, FG_WHITE}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L'█', BG_BLACK, FG_GREEN} },
	{ {L'▀', BG_BLACK, FG_RED}, {L' ',0,0}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L' ',0,0} },
	{ {L'▀', BG_BLACK, FG_RED}, {L'o', BG_BLACK, FG_WHITE}, {L'█', BG_BLACK, FG_DARKGREEN}, {L'█', BG_BLACK, FG_DARKYELLOW}, {L'█', BG_BLACK, FG_GREEN} },
	{ {L' ',0,0}, {L' ',0,0}, {L'█', BG_BLACK, FG_DARKGREEN}, {L' ',0,0}, {L' ',0,0} }
};

static MobDamageText mob_damage_texts[MAX_MOB_DAMAGE_TEXTS];

static bool is_walkable_block(const block_t block) {
	switch (block) {
	case BLOCK_AIR:
	case BLOCK_STAR:
	case BLOCK_WATER:
	case BLOCK_LOG:
	case BLOCK_LEAF:
		return false;
	}
	return true;
}

 void MobSpawn(void) {
	srand((unsigned int)time(NULL));

	const int random = rand() % 20;
	const bool positive = rand() % 2;

	int mob_x = player.x;
	if (positive)
		mob_x += random;
	else
		mob_x -= random;

	int mob_y = player.y;
	for (int y = 0; y < map.size.y; ++y)
		if (is_walkable_block(map.ppBlocks[y][mob_x].type)) {
			mob_y = y;
			break;
		}

	if (mob_count >= MAX_MOB) {
		return;
	}

	mobs[mob_count].x = mob_x;
	mobs[mob_count].y = mob_y - 1;
	mobs[mob_count].HP = mob_level * 10;
	mobs[mob_count].atk = mob_level * 2;
	mobs[mob_count].precise_x = (float)mobs[mob_count].x;
	mobs[mob_count].precise_y = (float)mobs[mob_count].y;
	mobs[mob_count].velocity_y = 0;
	mobs[mob_count].is_on_ground = true;
	mobs[mob_count].is_dead = false; 
	mobs[mob_count].dying_timer = 0.0f; 
	mobs[mob_count].animation_timer = 0.0f; 
	mobs[mob_count].current_frame = 0;     

	mobs[mob_count].ai_timer = 0.0f;
	mobs[mob_count].despawn_timer = 0.0f;

	++mob_count;
}

 static float mob_spawn_timer = 0.0f;
 const float Mob_Spawn_Cooltime = 2.0f;

 void mob_spawn_manager()
 {
	 if (is_night_time()) {
		 mob_spawn_timer += delta_time;
		 if (mob_spawn_timer >= Mob_Spawn_Cooltime) {
			 MobSpawn();
			 mob_spawn_timer = 0.0f;
		 }
	 }
 }



// 몬스터 피격 시 대미지 텍스트 생성 함수
static void create_mob_damage_text(const int mob_index, const int damage_value) {
	for (int i = 0; i < MAX_MOB_DAMAGE_TEXTS; ++i)
		if (!mob_damage_texts[i].active) {
			mob_damage_texts[i].active = true;
			mob_damage_texts[i].damage_value = damage_value;
			mob_damage_texts[i].mob_x = mobs[mob_index].x;
			mob_damage_texts[i].mob_y = mobs[mob_index].y;
			mob_damage_texts[i].precise_y = (float)mobs[mob_index].y;
			mob_damage_texts[i].timer = 1.0f; // 1.0초 동안 화면에 표시
			break;
		}
}

// 몬스터 대미지 텍스트 업데이트 함수 (매 프레임 호출)
static void update_mob_damage_texts(void) {
	for (int i = 0; i < MAX_MOB_DAMAGE_TEXTS; ++i)
		if (mob_damage_texts[i].active) {
			// 위로 움직이는 효과
			mob_damage_texts[i].precise_y -= delta_time * 5.0f;
			// 타이머 감소
			mob_damage_texts[i].timer -= delta_time;
			if (mob_damage_texts[i].timer <= 0.0f)
				mob_damage_texts[i].active = false;
		}
}

// 몬스터 대미지 텍스트 렌더링 함수
static void render_mob_damage_texts(void) {
	const COORD center_pos = {
		.X = console.size.X / 2,
		.Y = console.size.Y / 2
	};

	for (int i = 0; i < MAX_MOB_DAMAGE_TEXTS; ++i)
		if (mob_damage_texts[i].active) {
			const COORD draw_pos = {
				.X = center_pos.X + 8,
				.Y = (SHORT)(center_pos.Y - (PLAYER_SPRITE_HEIGHT / 2) - 1 - (player.precise_y - mob_damage_texts[i].precise_y))
			};

			fprint_string("Attack %d HP!", draw_pos, BACKGROUND_T_BLACK, FOREGROUND_T_YELLOW, mob_damage_texts[i].damage_value);
		}
}

void Mob_render(void) {
	const COORD center_m = {
		.X = console.size.X / 2,
		.Y = console.size.Y / 2,
	};

	for (int i = 0; i < mob_count; i++) {
		const int screen_x = center_m.X + (mobs[i].x * TEXTURE_SIZE - player.x * TEXTURE_SIZE),
			screen_y = center_m.Y + (mobs[i].y * TEXTURE_SIZE - player.y * TEXTURE_SIZE) - MOB_SPRITE_HEIGHT / 2;

		if (screen_x < 0 || screen_x + MOB_SPRITE_WIDTH >= console.size.X ||
			screen_y < 0 || screen_y >= console.size.Y)
			continue;

		// 몬스터가 죽어가는 상태가 아니면 체력바 렌더링
		if (!mobs[i].is_dead) {
			const COORD hp_pos = {
				.X = (SHORT)screen_x,
				.Y = (SHORT)(screen_y - 1)
			};
			fprint_string("HP: %d", hp_pos, BACKGROUND_T_BLACK, FOREGROUND_T_RED, mobs[i].HP);
		}

		// 몬스터 상태에 따라 스프라이트 선택
		const color_tchar_t(*sprite_to_render)[MOB_SPRITE_WIDTH];

		if (mobs[i].is_dead) {
			sprite_to_render = dead_zombie_sprite_data;
		}
		else if (mobs[i].velocity_x != 0.0f) {
			sprite_to_render = zombie_sprite_walk[mobs[i].current_frame];
		}
		else {
			sprite_to_render = zombie_sprite_data;
		}

		for (int j = 0; j < MOB_SPRITE_HEIGHT; ++j)
			for (int k = 0; k < MOB_SPRITE_WIDTH; ++k) {
				const color_tchar_t mob_pixel = sprite_to_render[j][k];
				const COORD current_pos = {
					.X = (SHORT)(screen_x + k),
					.Y = (SHORT)(screen_y + j)
				};

				if (current_pos.X >= 0 && current_pos.X < console.size.X &&
					current_pos.Y >= 0 && current_pos.Y < console.size.Y &&
					mob_pixel.character != L' ')
					print_color_tchar(mob_pixel, current_pos);
			}
	}

	render_mob_damage_texts();
}

static void DespawnMob(void) {
	for (int i = 0; i < mob_count;) {
		// 죽어가는 몬스터는 즉시 사라지지 않도록
		if (mobs[i].is_dead) {
			++i;
			continue;
		}
		int x = abs(mobs[i].x - player.x), y = abs(mobs[i].y - player.y);
		if (x > 70 || y > 70) {
			mobs[i].despawn_timer += delta_time;
			if (mobs[i].despawn_timer >= 5.0f) {
				for (int j = i; j < mob_count - 1; j++)
					mobs[j] = mobs[j + 1];
				mob_count--;
				continue;
			}
		}
		else
			mobs[i].despawn_timer = 0.0f;
		++i;
	}
}


static void Mob_deadcheck(void) {
	for (int i = 0; i < mob_count;) {

		if (mobs[i].HP <= 0 && !mobs[i].is_dead) {
			mobs[i].is_dead = true;
			mobs[i].dying_timer = 1.0f; 
		}
		if (mobs[i].is_dead && mobs[i].dying_timer <= 0.0f) {
			for (int j = i; j < mob_count - 1; j++)
				mobs[j] = mobs[j + 1];
			mob_count--;
			continue;
		}
		++i;
	}
}

void save_mob(void) {
	if (!pCurrent_save)
		instantiate_save();

	pCurrent_save->mob_count = mob_count;
	pCurrent_save->mob_level = mob_level;

	const size_t size = sizeof(Mob) * mob_count;
	if (!pCurrent_save->pMobs)
		pCurrent_save->pMobs = malloc(size);
	else
		pCurrent_save->pMobs = realloc(pCurrent_save->pMobs, size);

	for (int i = 0; i < mob_count; ++i)
		pCurrent_save->pMobs[i] = mobs[i];
}

static void Mob_physics(void) {
	for (int i = 0; i < mob_count; ++i) {
		
		if (mobs[i].is_dead) {
			mobs[i].velocity_x = 0.0f;
			mobs[i].velocity_y += GRAVITY * delta_time;
			mobs[i].precise_y += mobs[i].velocity_y * delta_time;
			mobs[i].y = (int)mobs[i].precise_y;
			continue;
		}

		const int below = mobs[i].y + 1;
		if (below >= map.size.y)
			mobs[i].is_on_ground = true;
		else
			mobs[i].is_on_ground = is_walkable_block(map.ppBlocks[below][mobs[i].x].type);
		if (mobs[i].is_on_ground) {
			if (mobs[i].velocity_y > 0.0f)
				mobs[i].velocity_y = 0.0f;
			mobs[i].precise_y = (float)mobs[i].y;
		}
		else
			mobs[i].velocity_y += GRAVITY * delta_time;

		const float new_precise_x = mobs[i].precise_x + mobs[i].velocity_x * delta_time;
		const int new_x = (int)new_precise_x;

		bool can_move_horizontally = false;
		if (new_x >= 0 && new_x < map.size.x)
			can_move_horizontally = !is_walkable_block(map.ppBlocks[mobs[i].y][new_x].type);

		if (can_move_horizontally)
			mobs[i].precise_x = new_precise_x;
		else
			mobs[i].velocity_x = 0.0f;

		mobs[i].precise_y += mobs[i].velocity_y * delta_time;
		mobs[i].x = (int)mobs[i].precise_x;
		mobs[i].y = (int)mobs[i].precise_y;
	}
}

static bool is_mob_movable(const int x, const int y) {
	return (x >= 0 && x < map.size.x && y >= 0 && y < map.size.y && !is_walkable_block(map.ppBlocks[y][x].type));
}

static void update_mob_ai(void) {
	for (int i = 0; i < mob_count; ++i)
		
		if (!mobs[i].is_dead) {
			if (mobs[i].ai_timer >= MOB_SPD) {
				mobs[i].ai_timer = 0.0f;
				const direction_t direction = find_next_direction(mobs[i].x, mobs[i].y, player.x, player.y, is_mob_movable);
				if (direction == DIRECTION_UP && mobs[i].is_on_ground)
					mobs[i].velocity_y = JUMP_SPD;
				else if (direction == DIRECTION_RIGHT)
					mobs[i].velocity_x = 1.0f;
				else if (direction == DIRECTION_LEFT)
					mobs[i].velocity_x = -1.0f;
				else if (direction == DIRECTION_NONE)
					mobs[i].velocity_x = 0;
			}
			else
				mobs[i].ai_timer += delta_time;
		}
}

//몬스터 피격 함수
static void handle_mob_click(const bool left_click) {
	// 왼쪽 클릭이 아니면 리턴
	if (!left_click)
		return;

	for (int i = 0; i < mob_count; ++i)
		if (!mobs[i].is_dead && selected_block_x >= mobs[i].x && selected_block_x < mobs[i].x + MOB_SPRITE_WIDTH &&
			selected_block_y >= mobs[i].y && selected_block_y < mobs[i].y + MOB_SPRITE_HEIGHT) {
			//데미지 입히는 부분
			mobs[i].HP -= player.atk_power;
			create_mob_damage_text(i, player.atk_power);
			return;
		}
}

static void check_mob_player_collision(void) {
	for (int i = 0; i < mob_count; ++i) {

		if (mobs[i].is_dead) continue;

		const bool collision_x = (mobs[i].x < player.x + PLAYER_SPRITE_WIDTH) && (mobs[i].x + MOB_SPRITE_WIDTH > player.x),
			collision_y = (mobs[i].y < player.y + PLAYER_SPRITE_HEIGHT) && (mobs[i].y + MOB_SPRITE_HEIGHT > player.y);

		if (collision_x && collision_y) {
			// 충돌했을 때만 쿨타임
			mobs[i].atk_cooltime_timer += delta_time;

			// 쿨타임이 지났는지 확인
			if (mobs[i].atk_cooltime_timer >= MOB_ATK_COOLTIME) {
				player_take_damage(mobs[i].atk);
				mobs[i].atk_cooltime_timer = 0.0f; // 공격 후 타이머 리셋
			}
		}
	}
}

static void update_mob_offset(void) {
	for (int i = 0; i < mob_count; ++i)
		mobs[i].x += map.offset_x;
}

void mob_init() {
	if (pCurrent_save) {
		mob_count = pCurrent_save->mob_count;
		mob_level = pCurrent_save->mob_level;
		for (int i = 0; i < mob_count; ++i)
			mobs[i] = pCurrent_save->pMobs[i];
	}
	else
		mob_count = 0;

	subscribe_mouse_click(handle_mob_click);
	subscribe_offset_change(update_mob_offset);
}

// 몬스터 업데이트
void mob_update() {
	update_mob_damage_texts();
	
	static float levelUpTimer = 0.0f;
	levelUpTimer += delta_time;
	if (levelUpTimer >= 100.0f && mob_level < 10) {
		++mob_level;
		levelUpTimer = 0.0f;
	}

	update_mob_ai();
	Mob_physics();
	check_mob_player_collision();

	//죽어갈때
	for (int i = 0; i < mob_count; ++i) {
		if (mobs[i].is_dead) {
			mobs[i].dying_timer -= delta_time;
		}

		// 걷기 애니메이션 
		if (mobs[i].velocity_x != 0.0f && !mobs[i].is_dead) {
			mobs[i].animation_timer += delta_time;
			if (mobs[i].animation_timer >= 0.5f) { 
				mobs[i].animation_timer = 0.0f;
				mobs[i].current_frame = (mobs[i].current_frame + 1) % 2;
			}
		}
		else {
			mobs[i].animation_timer = 0.0f;
			mobs[i].current_frame = 0;
		}
	}

	Mob_deadcheck();
	DespawnMob();
}


void destroy_mob(void) {
	unsubscribe_mouse_click(handle_mob_click);
	unsubscribe_offset_change(update_mob_offset);
}