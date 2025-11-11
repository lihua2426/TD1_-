#include <Novice.h>
#include <math.h>
const char kWindowTitle[] = "GC1B_01_カ_アン";
const int kWindowWitch = 1280;
const int kWindowHeight = 720;
bool debug = true;

//
typedef enum {
	START, // タイトル
	FIGHT, // 戦い
	WIN,   // 勝利
	LOSE,  // 負け
} GAME_STATE;
GAME_STATE game_state = START;

// 攻撃の種類
enum AttackType {
	MELEE, // 近接攻撃
	RANGE, // 弾発射
};

struct Vector2 {
	float x;
	float y;
};

struct Mouse {
	Vector2 pos;
	Vector2 dir;
	bool isMouse;
};

struct Camera {
	Vector2 offset;
	int shankeTime;
	float shakeProw;
};

struct UI {
	int x, y, w, h;
	const char* label;
	bool isHover;
	bool isClicked;
};

// playerとbossの共有宣言
struct Character {
	Vector2 pos;         // 座標
	Vector2 vec;         //
	Vector2 dir;         // これ、もしマウスを使う場合は、dir使う
	int hp;              // 体力
	int damage;          // ダメージ
	int invincible_time; // 無敵時間
	int shootCooldown;   //
	float width;         // サイズ
	float height;
	float speed;       // 移動のスピード
	float dash_speed;  // 瞬間加速
	bool isDash;       // 加速かどうか
	bool isAlive;      // 存在かどうか
	bool isHit;        // 被弾中かどうか//今まだ使わない
	bool isInvincible; // 無敵かどうか
};

// player
struct Player {
	Character base;
};

// Boss
struct Boss {
	Character base;
};

// playerとplayerとbossの攻撃の共有宣言
struct Attack {
	Vector2 pos; // 座標
	Vector2 vec;
	AttackType type; // 種類
	int damage;
	int lifeTime; // 存在時間
	int waitTime; // 攻撃前の準備時間
	float width;  // サイズ
	float height;
	float speed;
	bool isAlive; // 存在かどうか
	bool hasHit;  // ビットしたかどうか
};

Player InitPlayer(float x, float y) {
	Player p{};
	p.base.pos = {x, y};
	p.base.vec = {0.0f, 0.0f};
	p.base.dir = {1.0f, 0.0f};
	p.base.hp = 100;
	p.base.damage = 10;
	p.base.invincible_time = 60;
	p.base.shootCooldown = 0;
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
	b.base.shootCooldown = 0;
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

// 近接攻撃
Attack Attack_Melee(Vector2 pos, Vector2 dir) {
	Attack m{};
	m.pos = pos;
	m.vec = dir;
	m.type = MELEE;
	m.damage = 10;
	m.lifeTime = 15;
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

// 弾発
Attack Attack_Range(Vector2 pos, Vector2 dir) {
	Attack r{};

	float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
	if (length > 0) {
		dir.x /= length;
		dir.y /= length;
	}

	r.pos = pos;
	r.vec = dir;
	r.type = RANGE;
	r.damage = 10;
	r.lifeTime = 90;
	r.width = 25;
	r.height = 25;
	r.speed = 20.0f;
	r.isAlive = false;
	r.hasHit = false;
	return r;
}


// 共有の斜め移動スピード正規化と画面出てない制定
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

// 共有の当たり判定の関数（aabb判定）
bool isHit(Vector2& a, float aw, float ah, Vector2& b, float bw, float bh) { return (a.x < b.x + bw && a.x + aw > b.x && a.y < b.y + bh && a.y + ah > b.y); }

// 攻撃の存在時間と画面出てない設定
void isAttack(Attack& atk) {
	if (!atk.isAlive) {
		return;
	}
	atk.lifeTime--;
	if (atk.lifeTime <= 0) {
		atk.isAlive = false;
	}

	if (atk.pos.x < 0 || atk.pos.x + atk.width > 1280 || atk.pos.y < 0 || atk.pos.y + atk.height > 720) {
		atk.isAlive = false;
	}
}
// 共有の近接攻撃
void CharacterMelee(Character& attacker, Character& b, Attack melee[], int meleeMax, bool isAttacking) {

	if (isAttacking) {
		for (int i = 0; i < meleeMax; i++) {
			if (!melee[i].isAlive) {
				melee[i] = Attack_Melee(attacker.pos, attacker.dir);
				melee[i].isAlive = true;
				break;
			}
		}
	}
	for (int i = 0; i < meleeMax; i++) {
		if (!melee[i].isAlive) {
			continue;
		}
		isAttack(melee[i]);
		melee[i].pos.x = attacker.pos.x + attacker.dir.x * attacker.width;
		melee[i].pos.y = attacker.pos.y + attacker.dir.y * attacker.height;

		if (!b.isInvincible && isHit(melee[i].pos, melee[i].width, melee[i].height, b.pos, b.width, b.height)) {
			b.hp -= melee[i].damage;
			b.isInvincible = true;
			b.invincible_time = 60;
			// melee[i].isAlive = false;
		}
	}
}

// 共有の弾発射
void CharacterRange(Character& attacker, Character& target, Attack range[], int rangeMax, int& shootCooldown, bool isShooting, Camera& camera) {

	if (shootCooldown > 0) {
		shootCooldown--;
	}

	if (isShooting && shootCooldown == 0) {
		for (int i = 0; i < rangeMax; i++) {
			if (!range[i].isAlive) {
				range[i] = Attack_Range(attacker.pos, attacker.dir);
				range[i].isAlive = true;
				shootCooldown = 5;
				break;
			}
		}
	}

	for (int i = 0; i < rangeMax; i++) {
		if (!range[i].isAlive)
			continue;

		// 移动
		range[i].pos.x += range[i].vec.x * range[i].speed;
		range[i].pos.y += range[i].vec.y * range[i].speed;

		if (range[i].pos.x < 0 || range[i].pos.x > 1280 || range[i].pos.y < 0 || range[i].pos.y > 720) {
			range[i].isAlive = false;
			continue;
		}

		if (isHit(range[i].pos, range[i].width, range[i].height, target.pos, target.width, target.height)) {
			range[i].isAlive = false;
			camera.shankeTime = 10;
			camera.shakeProw = 5;
			if (!target.isInvincible) {
				target.hp -= range[i].damage;
				target.isInvincible = true;
				target.invincible_time = 60;
				
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Shak(Camera& camera) {
	if (camera.shankeTime > 0) {
		camera.shankeTime--;
		camera.offset.x = (float)(rand() % 3 - 1) * camera.shakeProw;
		camera.offset.y = (float)(rand() % 3 - 1) * camera.shakeProw;
	} else {
		camera.offset.x = 0.0f;
		camera.offset.y = 0.0f;
	}
}


void UpdateButton(UI& box) {
	int mouseX, mouseY;
	Novice::GetMousePosition(&mouseX, &mouseY);

	box.isHover = (mouseX > box.x && mouseX < box.x + box.w && mouseY > box.y && mouseY < box.y + box.h);

	box.isClicked = box.isHover && Novice::IsTriggerMouse(0);
}

void DrawButton(const UI& box) {
	int color = box.isHover ? WHITE : BLACK;
	Novice::DrawBox(box.x, box.y, box.w, box.h, 0.0f, color, kFillModeSolid);
	Novice::ScreenPrintf(box.x + box.w / 2 - 10, box.y + box.h / 2 - 5, box.label);
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Player player = InitPlayer(640.0f, 600.0f);

	Boss boss = InitBoss(640.0f, 320.0f);
	Attack attack_player{};

	Mouse mouse{
	    .pos = {0.0f, 0.0f},
	    .dir = {0.0f, 0.0f},
	    .isMouse = false,
	};

	Camera camera{
	    .offset = {0.0f, 0.0f},
	    .shankeTime = 0,
	    .shakeProw = 0.0f,
	};

	const int meleeMax = 1;
	const int rangeMax = 32;
	Attack player_melee[meleeMax]{};
	Attack player_range[rangeMax]{};
	// Attack boss_melee[meleeMax]{};
	// Attack boss_range[rangeMax]{};

	UI startBtn{540, 300, 200, 60, "START"};

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();
		Novice::SetMouseCursorVisibility(mouse.isMouse);
		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///
		///
		// マウス
		int mouseX = 0, mouseY = 0;
		Novice::GetMousePosition(&mouseX, &mouseY);
		mouse.pos.x = (float)mouseX;
		mouse.pos.y = (float)mouseY;

		float px = player.base.pos.x + player.base.width / 2;
		float py = player.base.pos.y + player.base.height / 2;

		player.base.dir.x = mouse.pos.x - px;
		player.base.dir.y = mouse.pos.y - py;

		float len = sqrtf(player.base.dir.x * player.base.dir.x + player.base.dir.y * player.base.dir.y);
		if (len > 0) {
			player.base.dir.x /= len;
			player.base.dir.y /= len;
		}

		switch (game_state) {
			// タイトル
		case START: {

			UpdateButton(startBtn);

			if (startBtn.isClicked) {
				game_state = FIGHT;
			}

			break;
		}

			// 戦い
		case FIGHT: {
			// 勝敗条件//
			if (keys[DIK_DOWN] && !preKeys[DIK_DOWN]) {
				game_state = WIN;
			}
			if (keys[DIK_DOWN] && !preKeys[DIK_DOWN]) {
				game_state = LOSE;
			}

			if (camera.shankeTime > 0) {
				Shak(camera);
			} else {
				camera.offset = {0.0f, 0.0f};
			}

			// player無敵時間の管理(無敵時間関数管理ずっとbug出てくる、まず、このまま使う）
			if (player.base.isInvincible) {
				player.base.invincible_time--;

				if (player.base.invincible_time <= 0) {
					player.base.isInvincible = false;
					player.base.invincible_time = 0;
				}
			}
			// ボース無敵時間の管理
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

				// 移動関数使う
				Move(player.base.pos, player.base.vec, player.base.width, player.base.height, player.base.speed);

				// 弾連続発射

				bool isShooting = Novice::IsPressMouse(0);
				bool isAttacking = keys[DIK_J] && !preKeys[DIK_J];
				CharacterMelee(player.base, boss.base, player_melee, meleeMax, isAttacking);
				CharacterRange(player.base, boss.base, player_range, rangeMax, player.base.shootCooldown, isShooting, camera);

				// playerとボースの当たり判定
				if (!player.base.isInvincible) {
					if (isHit(player.base.pos, player.base.width, player.base.height, boss.base.pos, boss.base.width, boss.base.height)) {
						player.base.hp -= boss.base.damage;
						player.base.isInvincible = true;
						player.base.invincible_time = 60;
					}
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
			UpdateButton(startBtn);
			DrawButton(startBtn);

			break;
		}

		case FIGHT: {

			// 背景
			Novice::DrawBox(0 - (int)camera.offset.x, 0 - (int)camera.offset.y, kWindowWitch, kWindowHeight, 0.0f, BLACK, kFillModeSolid);

			// boss
			Novice::DrawBox((int)boss.base.pos.x - (int)camera.offset.x, (int)boss.base.pos.y - (int)camera.offset.y, (int)boss.base.width, (int)boss.base.height, 0.0f, RED, kFillModeSolid);
			// player
			Novice::DrawBox((int)player.base.pos.x - (int)camera.offset.x, (int)player.base.pos.y - (int)camera.offset.y, (int)player.base.width, (int)player.base.height, 0.0f, WHITE, kFillModeSolid);

			for (int i = 0; i < meleeMax; i++) {
				if (player_melee[i].isAlive) {

					Novice::DrawBox((int)player_melee[i].pos.x, (int)player_melee[i].pos.y, (int)player_melee[i].width, (int)player_melee[i].height, 0.0f, WHITE, kFillModeSolid);
				}
			}

			for (int i = 0; i < rangeMax; i++) {
				if (player_range[i].isAlive) {

					Novice::DrawBox((int)player_range[i].pos.x, (int)player_range[i].pos.y, (int)player_range[i].width, (int)player_range[i].height, 0.0f, WHITE, kFillModeSolid);
				}
			}

			// UIポインタの絵

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
				Novice::ScreenPrintf(0, Y += H, "shakTime: %d", camera.shankeTime);
			}

			Novice::DrawLine((int)px, (int)py, (int)mouse.pos.x, (int)mouse.pos.y, RED);

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
		Novice::DrawEllipse((int)mouse.pos.x, (int)mouse.pos.y, 10, 10, 0.0f, RED, kFillModeSolid);
		Novice::ScreenPrintf(0, 0, "Mouse: (%.1f, %.1f)", mouse.pos.x, mouse.pos.y);

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
