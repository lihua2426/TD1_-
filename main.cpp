#include <Novice.h>
#include <math.h>
const char kWindowTitle[] = "GC1B_01_カ_アン";
const int kWindowWitch = 1280;
const int kWindowHeight = 720;
bool debug = true;
 
//
typedef enum {
	START,
	FIGHT,
	WIN,
	LOSE,
} GAME_STATE;
GAME_STATE game_state = START;

enum AttackType {
	MELEE,
	RANGE,
};
struct Vector2 {
	float x;
	float y;
};

// playerとbossの共有宣言
struct Character {
	Vector2 pos; // 座標
	Vector2 vec; //
	Vector2 dir;
	int hp;     //
	int damage; //
	int invincible_time;
	float width;
	float height;
	float speed;
	float dash_speed;
	bool isDash;
	bool isAlive;
	bool isHit;
	bool isInvincible;
};

// player
struct Player {
	Character base;
};

// Boss
struct Boss {
	Character base;
};

struct Attack {
	Vector2 pos;
	Vector2 vec;
	AttackType type;
	int damage;
	int lifeTime;
	int waitTime; 
	float width;
	float height;
	float speed;
	bool isAlive;
	bool hasHit;
};

Player InitPlayer(float x, float y) {
	Player p{};
	p.base.pos = {x, y};
	p.base.vec = {0.0f, 0.0f};
	p.base.dir = {1.0f, 0.0f};
	p.base.hp = 100;
	p.base.damage = 10;
	p.base.invincible_time = 60;
	p.base.width = 50.0f;
	p.base.height = 50.0f;
	p.base.speed = 5;
	p.base.dash_speed = 10;
	p.base.isDash = false;
	p.base.isAlive = true;
	p.base.isHit = false;
	p.base.isInvincible = false;
	return p;
}

Boss InitBoss(float x, float y) {
	Boss b{};
	b.base.pos = {x, y};
	b.base.vec = {0.0f, 0.0f};
	b.base.dir = {1.0f, 0.0f};
	b.base.hp = 100;
	b.base.damage = 10;
	b.base.invincible_time = 60;
	b.base.width = 75.0f;
	b.base.height = 75.0f;
	b.base.speed = 5;
	b.base.dash_speed = 10;
	b.base.isDash = false;
	b.base.isAlive = true;
	b.base.isHit = false;
	b.base.isInvincible = false;
	return b;
}

Attack Attack_Melee(Vector2 pos, Vector2 dir) {
	Attack m{};
	m.pos = pos;
	m.vec = dir;
	m.type = MELEE;
	m.damage = 10;
	m.lifeTime = 30;
	m.waitTime = 15;
	m.width = 50;
	m.height = 50;
	m.speed = 0.0f;
	m.isAlive = false;
	m.hasHit = false;

	m.pos.x += dir.x * m.width;
	m.pos.y += dir.y * m.height;
	return m;
}

Attack Attack_Range(Vector2 pos, Vector2 dir) {
	Attack r{};
	r.pos = pos;
	r.vec = dir;
	r.type = RANGE;
	r.damage = 10;
	r.lifeTime = 90;
	r.width = 25;
	r.height = 25;
	r.speed = 10.0f;
	r.isAlive = false;
	r.hasHit = false;

	return r;

}

// 当たり判定の
bool isHit(Vector2& a, float aw, float ah, Vector2& b, float bw, float bh) { return (a.x < b.x + bw && a.x + aw > b.x && a.y < b.y + bh && a.y + ah > b.y); }

// 斜め移動の
void Move(Vector2& pos, Vector2& vec, float width, float height, float speed) {
	float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
	if (length > 0) {
		vec.x /= length;
		vec.y /= length;
	}
	pos.x += vec.x * speed;
	pos.y += vec.y * speed;

	if (pos.x < 0) {
		pos.x = 0;
	}
	if (pos.x + width > 1280) {
		pos.x = 1280 - width;
	}
	if (pos.y < 0) {
		pos.y = 0;
	}
	if (pos.y + height > 720) {
		pos.y = 720 - height;
	}
}

//攻撃
void isAttack(Attack& atk) {
	if (!atk.isAlive) {
		return;
	}
	atk.lifeTime--;
	if (atk.lifeTime <= 0) {
		atk.isAlive = false;
	}
	if (atk.type == RANGE) {
		atk.pos.x += atk.vec.x * atk.speed;
		atk.pos.y += atk.vec.y * atk.speed;
	}
	if (atk.pos.x < 0 || atk.pos.x + atk.width>1280 || atk.pos.y < 0 || atk.pos.y + atk.height>720) {
		atk.isAlive = false;

	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Player player = InitPlayer(640.0f, 600.0f);

	Boss boss = InitBoss(640.0f, 320.0f);
	Attack attack_player{};
	//Attack attack_boss{};

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		switch (game_state) {
			// タイトル
		case START: {
			if (keys[DIK_DOWN] && !preKeys[DIK_DOWN]) {
				game_state = FIGHT;
			}

			break;
		}

			// 戦い
		case FIGHT: {
			if (keys[DIK_DOWN] && !preKeys[DIK_DOWN]) {
				game_state = WIN;
			}
			if (keys[DIK_DOWN] && !preKeys[DIK_DOWN]) {
				game_state = LOSE;
			}

			//
			if (player.base.isInvincible) {
				player.base.invincible_time--;

				if (player.base.invincible_time <= 0) {
					player.base.isInvincible = false;
					player.base.invincible_time = 0;
				}
			}

			if (boss.base.isInvincible) {
				boss.base.invincible_time--;

				if (boss.base.invincible_time <= 0) {
					boss.base.isInvincible = false;
					boss.base.invincible_time = 0;
				}
			}

			// player
			//  移動処理
			if (player.base.isAlive) {
				player.base.vec = {0.0f, 0.0f};
				
				if (keys[DIK_W]) {
					player.base.vec.y = -1.0f;
				}

				if (keys[DIK_S]) {
					player.base.vec.y = 1.0f;
				}

				if (keys[DIK_A]) {
					player.base.vec.x = -1.0f;
				}

				if (keys[DIK_D]) {
					player.base.vec.x = 1.0f;
				}
				if (player.base.vec.x != 0 || player.base.vec.y != 0) {
					player.base.dir = player.base.vec;
				}

				if (keys[DIK_J] && !preKeys[DIK_J]) {
					attack_player = Attack_Melee(player.base.pos, player.base.dir);
					attack_player.isAlive = true;
				}
				
				if (keys[DIK_K] && !preKeys[DIK_K]) {
					attack_player = Attack_Range(player.base.pos, player.base.dir);
					attack_player.isAlive = true;
				

				Move(player.base.pos, player.base.vec, player.base.width, player.base.height, player.base.speed);

				isAttack(attack_player);


				// playerとボースの当たり判定
				if (!player.base.isInvincible) {
					if (isHit(player.base.pos, player.base.width, player.base.height, boss.base.pos, boss.base.width, boss.base.height)) {

						player.base.hp -= boss.base.damage;
						player.base.isInvincible = true;
						player.base.invincible_time = 60;
					}
				}

				if (attack_player.isAlive && isHit(attack_player.pos, attack_player.width, attack_player.height, boss.base.pos, boss.base.width, boss.base.height)) {
					attack_player.isAlive = false;
					boss.base.hp -= attack_player.damage;
					boss.base.isInvincible = true;
					boss.base.invincible_time = 60;
				}
			}

			break;
		}
			// 勝利
		case WIN: {
			if (keys[DIK_DOWN] && !preKeys[DIK_DOWN]) {
				game_state = START;
			}
			if (keys[DIK_UP] && !preKeys[DIK_UP]) {
				game_state = FIGHT;
			}
			break;
		}

			// 負け
		case LOSE: {
			if (keys[DIK_DOWN] && !preKeys[DIK_DOWN]) {
				game_state = START;
			}
			if (keys[DIK_UP] && !preKeys[DIK_UP]) {
				game_state = FIGHT;
			}
			break;
		}
		}
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		switch (game_state) {
		case START: {
			Novice::DrawBox(0, 0, kWindowWitch, kWindowHeight, 0.0f, BLUE, kFillModeSolid);
			Novice::ScreenPrintf(kWindowWitch / 2, kWindowHeight / 2, "START");
			break;
		}

		case FIGHT: {
			// 背景
			Novice::DrawBox(0, 0, kWindowWitch, kWindowHeight, 0.0f, BLACK, kFillModeSolid);

			// boss
			Novice::DrawBox((int)boss.base.pos.x, (int)boss.base.pos.y, (int)boss.base.width, (int)boss.base.height, 0.0f, RED, kFillModeSolid);
			// player
			Novice::DrawBox((int)player.base.pos.x, (int)player.base.pos.y, (int)player.base.width, (int)player.base.height, 0.0f, WHITE, kFillModeSolid);
			
			if (attack_player.isAlive) {
				Novice::DrawBox((int)attack_player.pos.x, (int)attack_player.pos.y, (int)attack_player.width, (int)attack_player.height, 0.0f, WHITE, kFillModeSolid);
			}
		

			// 调试
			int Y = 0;
			int H = 20;

			if (keys[DIK_F1] && !preKeys[DIK_F1]) {
				debug = !debug;
			}
			if (debug) {
				Novice::ScreenPrintf(0, Y += H, "FIGHT");
				Novice::ScreenPrintf(0, Y += H, "PLAYER HP: %d", player.base.hp);
				Novice::ScreenPrintf(0, Y += H, "PLAYER isInvincible: %d", player.base.isInvincible);
				Novice::ScreenPrintf(0, Y += H, "PLAYER invincible_time: %d", player.base.invincible_time);
				Novice::ScreenPrintf(0, Y += H, "BOSS HP: %d", boss.base.hp);
				Novice::ScreenPrintf(0, Y += H, "BOSS isInvincible: %d", boss.base.isInvincible);
				Novice::ScreenPrintf(0, Y += H, "BOSS invincible_time: %d", boss.base.invincible_time);
			}

			break;
		}

		case WIN: {
			Novice::DrawBox(0, 0, kWindowWitch, kWindowHeight, 0.0f, 0xFFD700, kFillModeSolid);
			Novice::ScreenPrintf(kWindowWitch / 2, kWindowHeight / 2, "WIN");
			break;
		}

		case LOSE: {
			Novice::DrawBox(0, 0, kWindowWitch, kWindowHeight, 0.0f, RED, kFillModeSolid);
			Novice::ScreenPrintf(kWindowWitch / 2, kWindowHeight / 2, "LOSE");
			break;
		}
		}
		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
