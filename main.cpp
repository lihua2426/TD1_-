#include <Novice.h>
#include <math.h>
const char kWindowTitle[] = "GC1B_01_カ_アン";
const int kWindowWitch = 1280;
const int kWindowHeight = 720;
//
typedef enum {
	START,
	FIGHT,
	WIN,
	LOSE,
} GAME_STATE;
GAME_STATE game_state = START;

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

// 当たり判定の
bool isHit(Vector2& a, float aw, float ah, Vector2& b, float bw, float bh) { 
	return (a.x < b.x + bw && a.x + aw > b.x && a.y < b.y + bh && a.y + ah > b.y);
}

//斜め移動の
void Length(Vector2& pos,Vector2& vec, float speed) {
	float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
	if (length > 0) {
		vec.x /= length;
		vec.y /= length;
	}
	pos.x += vec.x * speed;
	pos.y += vec.y * speed;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Player player = InitPlayer(640.0f, 600.0f);

	Boss boss = InitBoss(640.0f, 320.0f);

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
			// 移動処理
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
				 Length(player.base.pos, player.base.vec, player.base.speed);
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
			//背景
			Novice::DrawBox(0, 0, kWindowWitch, kWindowHeight, 0.0f, BLACK, kFillModeSolid);
			Novice::ScreenPrintf(kWindowWitch / 2, kWindowHeight / 2, "FIGHT");
			//player
			Novice::DrawBox((int)player.base.pos.x, (int)player.base.pos.y, (int)player.base.width, (int)player.base.height, 0.0f, WHITE, kFillModeSolid);
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
