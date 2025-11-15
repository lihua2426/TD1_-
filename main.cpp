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
	STOP,
	WIN,  // 勝利
	LOSE, // 負け
} GAME_STATE;
GAME_STATE game_state = START;

// 攻撃の種類
enum AttackType {
	MELEE, // 近接攻撃
	RANGE, // 弾発射
};
bool attack_type = false;

enum ParticleType {
	PRT_BLODD,
	PRT_DASH,
};

struct Vector2 {
	float x;
	float y;
};

// マウス
struct Mouse {
	Vector2 pos;
	Vector2 dir;
	bool isMouse;
};

// カメラ宣言
struct Camera {
	Vector2 offset;
	int shankeTime;
	float shakeProw;
};

// UI宣言
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
	float speed;      // 移動のスピード
	float dash_speed; // 瞬間加速
	float dash_time;
	float normal_speed;
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

// 粒子システム
struct Particle {
	Vector2 pos;
	Vector2 vec;
	float width;
	float height;
	int lifeTime;
	bool isAlive;
	ParticleType type;
};

const int prtmax = 100;
Particle blood[prtmax]{};

//=================================================================
// 初期化
//===============================================================--

// プレイヤ
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
	p.base.speed = 0;
	p.base.dash_speed = 10;
	p.base.dash_time = 0;
	p.base.normal_speed = 5;
	p.base.isDash = false;
	p.base.isAlive = true;
	p.base.isHit = false;
	p.base.isInvincible = false;
	return p;
}

// ボース
Boss InitBoss(float x, float y) {
	Boss b{};
	b.base.pos = {x, y};
	b.base.vec = {0.0f, 0.0f};
	b.base.dir = {1.0f, 0.0f};
	b.base.hp = 300;
	b.base.damage = 10;
	b.base.invincible_time = 60;
	b.base.shootCooldown = 0;
	b.base.width = 75.0f;
	b.base.height = 75.0f;
	b.base.speed = 0;
	b.base.dash_speed = 10;
	b.base.dash_time = 0;
	b.base.normal_speed = 5;
	b.base.isDash = false;
	b.base.isAlive = true;
	b.base.isHit = false;
	b.base.isInvincible = false;
	return b;
}

// 共有近接攻撃
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

// 共有弾攻撃
Attack Attack_Range(Vector2 pos, Vector2 dir) {
	Attack r{};

	// 斜め移動の処理//正規化
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

// 粒子システム
Particle InitPrt(Vector2 pos, ParticleType type) {
	Particle p{};
	p.pos = pos;
	p.vec = {0.0f, 0.0f};
	p.width = 5.0f;
	p.height = 5.0f;
	p.isAlive = true;
	p.lifeTime = 0;
	p.type = type;

	return p;
}

//===============================================================================
// 関数
//================================================================================

//============================================================================================
// 粒子システムについて関数
//=====================================================================================================
// 粒子の生成
void SpawPrt(Vector2 hitPos, ParticleType type, Particle prt[], int prtMax) {

	for (int i = 0; i < prtMax; i++) {
		float angle = 0.0f;
		float speed = 0.0f;
		if (!prt[i].isAlive) {

			// Particle p{};
			prt[i] = InitPrt(hitPos, type);

			switch (type) {
			case PRT_BLODD:
				angle = (float)(rand() % 360) * 3.14159f / 180.0f;
				speed = (float)(rand() % 15 + 5) / 10.0f;

				prt[i].width = prt[i].height = 4.0f + rand() % 3;
				prt[i].lifeTime = 30 + rand() % 30;
				break;
			case PRT_DASH:
				// prt[i].width = prt[i].height = 5;
				// prt[i].lifeTime = 20;
				break;
			}

			if (type == PRT_DASH) {
				speed *= 0.4f;
			}

			prt[i].vec.x = cosf(angle) * speed;
			prt[i].vec.y = sinf(angle) * speed;

			// prt[i].lifeTime = 20 + rand() % 120;

			prt[i].isAlive = true;
		}
	}
}

// 粒子の存在時間関数
void UpdatePrt(Particle& prt) {
	if (!prt.isAlive) {
		return;
	}
	prt.lifeTime--;
	if (prt.lifeTime <= 0) {
		prt.isAlive = false;
	}

	prt.pos.x += prt.vec.x;
	prt.pos.y += prt.vec.y;

	prt.vec.x *= 0.9f;
	prt.vec.y *= 0.9f;

	if (prt.pos.x < 0 || prt.pos.x > 1280 || prt.pos.y < 0 || prt.pos.y > 720) {
		prt.isAlive = false;
	}
}

// 生成条件
void Particle_UpdateAll(Particle prt[], int prtMax) {
	for (int i = 0; i < prtMax; i++) {
		if (prt[i].isAlive) {
			UpdatePrt(prt[i]);
		}
	}
}
//========================================================================================================================================================================================================
//========================================================================================================================================================================================================
//========================================================================================================================================================================================================

// 共有のダシュル関数
void Dash(Character& dasher) { // dasherは、ダシュルする人
	// もしダシュルしない場合は、処理/終わり
	if (!dasher.isDash) {
		return;
	}
	// スピード変更、時間始まる、
	dasher.speed = dasher.dash_speed;
	dasher.dash_time++;

	// もし、ダシュル中、時間20かかる、ダシュル終わる
	if (dasher.isDash) {
		SpawPrt(dasher.pos, PRT_DASH, blood, prtmax);
		if (dasher.dash_time >= 20) {
			dasher.isDash = false;
			dasher.dash_time = 0;

			dasher.speed = dasher.normal_speed;
		}
	}
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
bool isHit(const Character& a, const Character& b) { return (a.pos.x < b.pos.x + b.width && a.pos.x + a.width > b.pos.x && a.pos.y < b.pos.y + b.height && a.pos.y + a.height > b.pos.y); }

//============================================================================================
// 攻撃について関数
// ==============================================================================================
// 近接攻撃を生成する関数
void SpawnMelee(Character& attacker, Attack melee[], int meleeMax) {
	for (int i = 0; i < meleeMax; i++) {
		if (!melee[i].isAlive) {
			melee[i] = Attack_Melee(attacker.pos, attacker.dir);
			melee[i].isAlive = true;
			return;
		}
	}
}

// 弾攻撃を生成する関数
void SpawnRange(Character& attacker, Attack range[], int rangeMax, int& cooldown) {
	if (cooldown > 0)
		return;

	for (int i = 0; i < rangeMax; i++) {
		if (!range[i].isAlive) {
			range[i] = Attack_Range(attacker.pos, attacker.dir);
			range[i].isAlive = true;
			cooldown = 5;
			return;
		}
	}
}

// 共有攻撃存在時間関数
void UpdateAttack(Attack& atk) {
	if (!atk.isAlive)
		return;

	atk.lifeTime--;
	if (atk.lifeTime <= 0) {
		atk.isAlive = false;
		return;
	}

	// 攻撃の移動（弾）
	atk.pos.x += atk.vec.x * atk.speed;
	atk.pos.y += atk.vec.y * atk.speed;

	// 画面外出てない
	if (atk.pos.x < 0 || atk.pos.x > 1280 || atk.pos.y < 0 || atk.pos.y > 720) {
		atk.isAlive = false;
	}
}

// 共有無敵関数
void ApplyDamage(Character& target, int damage) {
	if (target.isInvincible)
		return;

	target.hp -= damage;
	target.isInvincible = true;
	target.invincible_time = 30;
}

// 画面揺れる時間
void ApplyCameraShake(Camera& camera, float power, int time) {
	camera.shankeTime = time;
	camera.shakeProw = power;
}

// 共有攻撃の関数（前の当たり判定関数と違う、攻撃の宣言Characterを使ってないので）
bool CheckHit(const Attack& atk, const Character& target) {
	return (atk.pos.x < target.pos.x + target.width && atk.pos.x + atk.width > target.pos.x && atk.pos.y < target.pos.y + target.height && atk.pos.y + atk.height > target.pos.y);
}

// 全ての攻撃の更新関数
void AttackSystem_UpdateAll(Character& attacker, Character& target, Attack attackArray[], int attackMax, bool isAttacking, int& shootCooldown, Camera& camera) {
	// 生成攻击
	if (isAttacking) {
		if (attackArray[0].type == MELEE)
			SpawnMelee(attacker, attackArray, attackMax); // 近接攻撃
		else
			SpawnRange(attacker, attackArray, attackMax, shootCooldown); // 弾
	}

	for (int i = 0; i < attackMax; i++) {
		Attack& atk = attackArray[i];

		if (!atk.isAlive)
			continue;

		// 近接攻撃の場所
		if (atk.type == MELEE) {
			atk.pos.x = attacker.pos.x + attacker.dir.x * attacker.width;
			atk.pos.y = attacker.pos.y + attacker.dir.y * attacker.height;
		}

		// 更新存在時間関数読み込み
		UpdateAttack(atk);

		// もし、攻撃判定が成功したら，
		if (CheckHit(atk, target)) {
			if (atk.type == RANGE) {
				atk.isAlive = false;
			}

			ApplyDamage(target, atk.damage); // ダメージの関数読み込み
			ApplyCameraShake(camera, 5, 10);

			Vector2 hitPos;
			hitPos.x = atk.pos.x + atk.width * 0.5f;
			hitPos.y = atk.pos.y + atk.height * 0.5f;
			SpawPrt(hitPos, PRT_BLODD, blood, prtmax);
		}
	}
}
//================================================================================================================================================================================================================
//================================================================================================================================================================================================================
//================================================================================================================================================================================================================

// 画面が揺れるの強さ
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

// マオスとUI　BOXの当たり判定
void UpdateButton(UI& box) {
	int mouseX, mouseY;
	Novice::GetMousePosition(&mouseX, &mouseY);

	box.isHover = (mouseX > box.x && mouseX < box.x + box.w && mouseY > box.y && mouseY < box.y + box.h); // マウスがボタンの範囲に入っているかどうか
	box.isClicked = box.isHover && Novice::IsTriggerMouse(0);                                             // BoX中に左クリック
}

// BOXを描画する
void DrawButton(const UI& box) {
	int color = box.isHover ? BLACK : GREEN;
	Novice::DrawBox(box.x, box.y, box.w, box.h, 0.0f, color, kFillModeSolid);
	Novice::ScreenPrintf(box.x + box.w / 2 - 30, box.y + box.h / 2 - 5, box.label);
}

// もう一回遊び、数値をリセットする
void ALL(Player& player, Boss& boss, Camera& camera, Attack melee[], Attack range[], int meleeMax, int rangeMax) {
	attack_type = false;

	player = InitPlayer(640.0f, 600.0f);
	//
	boss = InitBoss(640.0f, 320.0f);

	//
	// mouse = {
	//  .pos = {0.0f, 0.0f},
	//  .dir = {0.0f, 0.0f},
	// .isMouse = false,
	//};
	//
	camera = {
	    .offset = {0.0f, 0.0f},
	    .shankeTime = 0,
	    .shakeProw = 0.0f,
	};
	//
	meleeMax = 1;
	rangeMax = 32;
	for (int i = 0; i < meleeMax; i++) {
		melee[i].pos = player.base.pos;
		melee[i].vec = player.base.dir;
		melee[i].type = MELEE;
		melee[i].damage = 10;
		melee[i].lifeTime = 15;
		melee[i].waitTime = 15;
		melee[i].width = 50;
		melee[i].height = 50;
		melee[i].speed = 0.0f;
		melee[i].isAlive = false;
		melee[i].hasHit = false;

		melee[i].pos.x += player.base.dir.x * melee[i].width;
		melee[i].pos.y += player.base.dir.y * melee[i].height;
	}
	for (int i = 0; i < rangeMax; i++) {
		range[i].pos = player.base.pos;
		range[i].vec = player.base.dir;
		range[i].type = RANGE;
		range[i].damage = 10;
		range[i].lifeTime = 90;
		range[i].width = 25;
		range[i].height = 25;
		range[i].speed = 20.0f;
		range[i].isAlive = false;
		range[i].hasHit = false;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	Player player = InitPlayer(640.0f, 600.0f);
	Boss boss = InitBoss(640.0f, 320.0f);

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

	// タイトル
	UI startBtn{540, 300, 200, 60, "START"};

	// ゲーム時、止まる
	UI stopBtn{0, 0, 30, 30, "STOP"};
	// 止まるmenu
	// 続行
	UI menuBtn_countinou{540, 100, 200, 100, "COUNTINOU"};
	// やり直
	UI menuBtn_return{540, 300, 200, 100, "RETURN"};
	// タイトル戻す
	UI menuBtn_return_title{540, 500, 200, 100, "RETURN_TITLE"};

	// WIN
	// もう一回遊び
	UI scoreBtn_return{540, 100, 200, 100, "RETURN"};
	// タイトルに戻す
	UI scoreBtn_return_title{540, 520, 200, 100, "RETURN_TITLE"};

	//////////////////////////////////////////////////

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

		// マウスの正規化
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
			ALL(player, boss, camera, player_melee, player_range, meleeMax, rangeMax);

			UpdateButton(startBtn);

			if (startBtn.isClicked) {
				game_state = FIGHT;
				ALL(player, boss, camera, player_melee, player_range, meleeMax, rangeMax);
			}

			break;
		}

			// 戦い
		case FIGHT: {

			UpdateButton(stopBtn);
			if (stopBtn.isClicked) {
				game_state = STOP;
			}

			if (player.base.hp <= 0 && boss.base.isAlive) {
				game_state = LOSE;
			}

			if (boss.base.hp <= 0 && player.base.isAlive) {
				game_state = WIN;
			}

			if (camera.shankeTime > 0) {
				Shak(camera);
			} else {
				camera.offset = {0.0f, 0.0f};
			}

			if (player.base.shootCooldown > 0) {
				player.base.shootCooldown--;
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
				player.base.speed = player.base.normal_speed;
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

				if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {
					player.base.isDash = true;
					player.base.dash_time = 0;
				}
				// 移動関数使う
				Dash(player.base);
				Move(player.base.pos, player.base.vec, player.base.width, player.base.height, player.base.speed);

				bool isAttacking = false;
				if (keys[DIK_E] && !preKeys[DIK_E]) {
					attack_type = !attack_type;
				}
				if (attack_type == false) {
					isAttacking = Novice::IsPressMouse(0);
					AttackSystem_UpdateAll(player.base, boss.base, player_range, rangeMax, isAttacking, player.base.shootCooldown, camera);

				} else if (attack_type == true) {
					isAttacking = Novice::IsTriggerMouse(0);
					AttackSystem_UpdateAll(player.base, boss.base, player_melee, meleeMax, isAttacking, player.base.shootCooldown, camera);
				}

				Particle_UpdateAll(blood, prtmax);
				// playerとボースの当たり判定
				if (!player.base.isInvincible) {
					if (isHit(player.base, boss.base)) {
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

			UpdateButton(scoreBtn_return);
			if (scoreBtn_return.isClicked) {
				ALL(player, boss, camera, player_melee, player_range, meleeMax, rangeMax);
				game_state = FIGHT;
			}
			UpdateButton(scoreBtn_return_title);
			if (scoreBtn_return_title.isClicked) {
				ALL(player, boss, camera, player_melee, player_range, meleeMax, rangeMax);
				game_state = START;
			}

			break;
		}

			// 負け
		case LOSE: {

			UpdateButton(scoreBtn_return);
			if (scoreBtn_return.isClicked) {
				ALL(player, boss, camera, player_melee, player_range, meleeMax, rangeMax);
				game_state = FIGHT;
			}
			UpdateButton(scoreBtn_return_title);
			if (scoreBtn_return_title.isClicked) {
				ALL(player, boss, camera, player_melee, player_range, meleeMax, rangeMax);
				game_state = START;
			}
			break;
		}
		case STOP: {
			UpdateButton(menuBtn_countinou);
			if (menuBtn_countinou.isClicked) {
				game_state = FIGHT;
			}

			UpdateButton(menuBtn_return);
			if (menuBtn_return.isClicked) {
				ALL(player, boss, camera, player_melee, player_range, meleeMax, rangeMax);
				game_state = FIGHT;
			}
			UpdateButton(menuBtn_return_title);
			if (menuBtn_return_title.isClicked) {
				ALL(player, boss, camera, player_melee, player_range, meleeMax, rangeMax);
				game_state = START;
			}
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
			// UpdateButton(startBtn);
			DrawButton(startBtn);

			break;
		}

		case FIGHT: {

			// 背景
			Novice::DrawBox(0 - (int)camera.offset.x, 0 - (int)camera.offset.y, kWindowWitch, kWindowHeight, 0.0f, BLACK, kFillModeSolid);

			// boss
			Novice::DrawBox((int)boss.base.pos.x - (int)camera.offset.x, (int)boss.base.pos.y - (int)camera.offset.y, (int)boss.base.width, (int)boss.base.height, 0.0f, WHITE, kFillModeWireFrame);
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

			// 血

			for (int i = 0; i < prtmax; i++) {
				if (blood[i].isAlive) {
					Novice::DrawBox((int)blood[i].pos.x - (int)camera.offset.x, (int)blood[i].pos.y - (int)camera.offset.y, (int)blood[i].width, (int)blood[i].height, 0.0f, RED, kFillModeSolid);
				}
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
				Novice::ScreenPrintf(0, Y += H, "shakTime: %d", camera.shankeTime);
				Novice::ScreenPrintf(0, Y += H, "dashTime: %f", player.base.dash_time);
				Novice::ScreenPrintf(0, Y += H, "nowSpeed: %f", player.base.speed);
				Novice::ScreenPrintf(0, Y += H, "bossHit: %d", boss.base.isHit);
				Novice::ScreenPrintf(0, Y += H, "prtIsAlive: %d", blood->isAlive);
			}

			Novice::DrawLine((int)px, (int)py, (int)mouse.pos.x, (int)mouse.pos.y, RED);

			DrawButton(stopBtn);
			break;
		}

		case WIN: {
			Novice::DrawBox(0, 0, kWindowWitch, kWindowHeight, 0.0f, 0xFFD700, kFillModeSolid);
			Novice::ScreenPrintf(kWindowWitch / 2, kWindowHeight / 2, "WIN");

			DrawButton(scoreBtn_return);
			DrawButton(scoreBtn_return_title);
			break;
		}

		case LOSE: {
			Novice::DrawBox(0, 0, kWindowWitch, kWindowHeight, 0.0f, RED, kFillModeSolid);
			Novice::ScreenPrintf(kWindowWitch / 2, kWindowHeight / 2, "LOSE");

			DrawButton(scoreBtn_return);
			DrawButton(scoreBtn_return_title);
			break;
		}
		case STOP: {
			DrawButton(menuBtn_countinou);
			DrawButton(menuBtn_return);
			DrawButton(menuBtn_return_title);
		}
		}
		Novice::DrawEllipse((int)mouse.pos.x, (int)mouse.pos.y, 10, 10, 0.0f, GREEN, kFillModeSolid);
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
