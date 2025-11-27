#include <Novice.h>
#include <math.h>
const char kWindowTitle[] = "GC1B_01_カ_アン";

#pragma region
#pragma endregion

const int kWindowWitch = 1280;
const int kWindowHeight = 720;
bool debug = true;
bool fullscreen = false;
int deadX = 250;
int deadY = 150;
int deadW = 1280 - 500;
int deadH = 720 - 300;

typedef enum {
	START, // タイトル
	FIGHT, // 戦い
	STOP,
	WIN,  // 勝利
	LOSE, // 負け
} GAME_STATE;
GAME_STATE game_state = START;

//======================================================================
// boss
//=====================================================================

//
enum BOSS_PHASE {
	PHASE_1,
	PhASE_2,
};
BOSS_PHASE boss_phase = PHASE_1;

// boss状態
enum BOSS_STATE {
	IDLE,
	SELECT_SKILL,
	CAST_SKILL,
};
BOSS_STATE boss_start = IDLE;

// スキル
enum BOSS_SKILL {
	SHOOT,      // 弾発射
	LASER,      // レーザー
	GROUNDSLAM, // 地面攻撃
	EYELASER,   // 目からレーザー
	CHARGE,     // ダシュル
};
BOSS_SKILL boss_shill = SHOOT;

//=========================================================================================================

// player攻撃の種類
enum AttackType {

	MELEE, // 近接攻撃
	RANGE, // 弾発射

};
bool attack_type = false;

// 粒子
enum ParticleType {
	PRT_BLODD,
	PRT_DASH,
};

// 座標
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
	Vector2 worldOffset;
	Vector2 shakeOffset;
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
	Vector2 vel;         //
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

	// 弾管理
	float bulletSezi;
	int bulletTime;
	float bulletSpeed;
	int bulletdamege;
};

// player
struct Player {
	Character base;
};

// Boss
struct Boss {
	Character base;
	int skillCooldown = 0;
	int skillWaitTime = 60;
	BOSS_SKILL currentSkill = SHOOT;
};

// playerとplayerとbossの攻撃の共有宣言
struct Attack {
	Vector2 pos; // 座標
	Vector2 vel;
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

struct HitEvent {
	bool hit;       // 当たり判定成功かどうか
	Vector2 hitPos; // 当たった場所（血用）
};

// 粒子システム
struct Particle {
	Vector2 pos;
	Vector2 vel;
	float width;
	float height;
	int lifeTime;
	bool isAlive;
	ParticleType type;
};

const int prtmax = 100;
Particle blood[prtmax]{};

// 弾リロードする
struct Reload {
	float reload_time;
	int nowBullet;
	int bulletMax;
	bool isReload;
};

// bossスキル共有宣言
struct skillCharacter {
	Vector2 pos;
	Vector2 vel;
	BOSS_SKILL type;
	float width;
	float height;
	int damege;
	int waitTime;
	bool isWait;
	bool isAlive;
	int aliveTime;
};

struct Shoot {
	skillCharacter base;
};

struct Laser {
	skillCharacter base;
	float angle;
	float rotateSpeed;
	float rotateLimit;
	float startAngle;
};

struct GroundSlam {
	skillCharacter base;
};

struct EyeLaser {
	skillCharacter base;
};

struct Charge {
	skillCharacter base;
};

//=================================================================
// 初期化
//===============================================================

// プレイヤ
Player InitPlayer(float x, float y) {
	Player p{};
	p.base.pos = {x, y};
	p.base.vel = {0.0f, 0.0f};
	p.base.dir = {1.0f, 0.0f};
	p.base.hp = 100;
	p.base.damage = 10;
	p.base.invincible_time = 60;
	p.base.shootCooldown = 0;
	p.base.width = 64.0f;
	p.base.height = 64.0f;
	p.base.speed = 0;
	p.base.dash_speed = 15;
	p.base.dash_time = 0;
	p.base.normal_speed = 8;
	p.base.isDash = false;
	p.base.isAlive = true;
	p.base.isHit = false;
	p.base.isInvincible = false;
	p.base.bulletSpeed = 15.0f;
	p.base.bulletdamege = 15;
	p.base.bulletTime = 90;
	p.base.bulletSezi = 16.0f;

	return p;
}

// ボース
Boss InitBoss(float x, float y) {
	Boss b{};
	b.base.pos = {x, y};
	b.base.vel = {0.0f, 0.0f};
	b.base.dir = {1.0f, 0.0f};
	b.base.hp = 300;
	b.base.damage = 10;
	b.base.invincible_time = 60;
	b.base.shootCooldown = 90;
	b.base.width = 128.0f;
	b.base.height = 128.0f;
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

// Player近接攻撃
Attack Attack_Melee(Vector2 pos, Vector2 dir) {
	Attack m{};
	m.pos = pos;
	m.vel = dir;
	m.type = MELEE;
	m.damage = 10;
	m.lifeTime = 15;
	m.waitTime = 15;
	m.width = 50;
	m.height = 50;
	m.speed = 0.0f;
	m.isAlive = false;
	m.hasHit = false;
	return m;
}

// Player弾攻撃
Attack Attack_Range(Character attacker) {
	Attack r{};

	// 斜め移動の処理//正規化
	float length = sqrtf(attacker.dir.x * attacker.dir.x + attacker.dir.y * attacker.dir.y);
	if (length > 0) {
		attacker.dir.x /= length;
		attacker.dir.y /= length;
	}

	r.pos = {attacker.pos.x + attacker.width / 2, attacker.pos.y + attacker.height / 2};
	r.vel = attacker.dir;
	r.type = RANGE;
	r.damage = 10;
	r.lifeTime = attacker.bulletTime;
	r.width = attacker.bulletSezi;
	r.height = attacker.bulletSezi;
	r.speed = attacker.bulletSpeed;
	r.isAlive = false;
	r.hasHit = false;
	return r;
}

// 粒子システム
Particle InitPrt(Vector2 pos, ParticleType type) {
	Particle p{};
	p.pos = pos;
	p.vel = {0.0f, 0.0f};
	p.width = 5.0f;
	p.height = 5.0f;
	p.isAlive = true;
	p.lifeTime = 0;
	p.type = type;
	return p;
}

// boss skill




Laser InitLaser(Character& boss, BOSS_SKILL type, Character& player) {
	Laser l{};
	l.base.pos = {boss.pos.x + boss.width * 0.5f, boss.pos.y + boss.height * 0.5f};
	l.base.vel = {0.0f, 0.0f};
	l.base.width = 500.0f;
	l.base.height = 50.0f;
	l.base.damege = 5;
	l.base.type = type;
	l.base.aliveTime = 90;
	l.base.waitTime = 60;
	l.base.isAlive = false;
	l.base.isWait = false;

	float px = player.pos.x + player.width * 0.5f;
	float py = player.pos.y + player.height * 0.5f;

	float bx = boss.pos.x + boss.width * 0.5f;
	float by = boss.pos.y + boss.height * 0.5f;

	float angle = atan2f(py - by, px - bx);

	l.angle = angle;
	l.startAngle = angle;
	l.rotateSpeed = 0.02f;
	l.rotateLimit = 0.6f;

	return l;
}

EyeLaser InitEyeLaser(Vector2 pos, BOSS_SKILL type) {
	EyeLaser e{};
	e.base.pos = pos;
	e.base.vel = {0.0f, 0.0f};
	e.base.width = 50.0f;
	e.base.height = 50.0f;
	e.base.damege = 5;
	e.base.type = type;
	e.base.aliveTime = 90;
	e.base.waitTime = 30;
	e.base.isAlive = false;
	e.base.isWait = false;

	return e;
}

GroundSlam InitgroundSlam(Vector2 pos, BOSS_SKILL type) {
	GroundSlam g{};
	g.base.pos = pos;
	g.base.vel = {0.0f, 0.0f};
	g.base.width = 160.0f;
	g.base.height = 160.0f;
	g.base.damege = 5;
	g.base.type = type;
	g.base.aliveTime = 60;
	g.base.waitTime = 30;
	g.base.isAlive = false;
	g.base.isWait = false;

	return g;
}

Charge InitCharge(BOSS_SKILL type) {
	Charge c{};
	// c.base.pos = pos;
	c.base.vel = {0.0f, 0.0f};
	// c.base.width = 50.0f;
	// c.base.height = 50.0f;
	c.base.damege = 15;
	c.base.type = type;
	c.base.aliveTime = 30;
	c.base.waitTime = 30;
	c.base.isAlive = false;
	c.base.isWait = false;
	return c;
}

//===============================================================================
// 関数
//================================================================================
// マップ
int tile = 32;
int map[25][50] = {
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
};

void TileHit(Vector2* pos, Vector2* vel, float width, float height, int Map[25][50], int tile_sezi) {

	{
		int L = (int)pos->x;
		int R = (int)pos->x + (int)width;
		int T = (int)pos->y;
		int B = (int)pos->y + (int)height;

		int txL = L / tile_sezi;
		int txR = (R - 1) / tile_sezi;
		int tyT = T / tile_sezi;
		int tyB = (B - 1) / tile_sezi;

		if (txL < 1) {

			txL = 1;
		}

		if (txR > 48) {
			txR = 48;
		}

		if (vel->y < 0 && (Map[tyT][txL] == 2 || Map[tyT][txR] == 2)) {
			pos->y = (tyT + 1) * (float)tile_sezi;
		}
		if (vel->y > 0 && (Map[tyB][txL] == 2 || Map[tyB][txR] == 2)) {
			pos->y = tyB * tile_sezi - height;
		}
	}

	{
		int L = (int)pos->x;
		int R = (int)pos->x + (int)width;
		int T = (int)pos->y;
		int B = (int)pos->y + (int)height;

		int txL = L / tile_sezi;
		int txR = (R - 1) / tile_sezi;
		int tyT = T / tile_sezi;
		int tyB = (B - 1) / tile_sezi;

		if (vel->x < 0 && (Map[tyT][txL] == 2 || Map[tyB][txL] == 2)) {
			pos->x = (txL + 1) * (float)tile_sezi;
		}
		if (vel->x > 0 && (Map[tyT][txR] == 2 || Map[tyB][txR] == 2)) {
			pos->x = txR * tile_sezi - width;
		}
	}
}

bool BulletTileHit(Attack atk, int Map[25][50], int tile_sezi) {
	bool isHit = false;
	float px = atk.pos.x + atk.width * 0.5f;
	float py = atk.pos.y + atk.height * 0.5f;

	int tx = (int)px / tile_sezi;
	int ty = (int)py / tile_sezi;

	if (tx < 0 || tx >= 50 || ty < 0 || ty >= 25) {
		return true;
	}

	if (Map[ty][tx] == 2) {
		isHit = true;
	}
	return isHit;
}

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

			prt[i].vel.x = cosf(angle) * speed;
			prt[i].vel.y = sinf(angle) * speed;
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

	prt.pos.x += prt.vel.x;
	prt.pos.y += prt.vel.y;

	prt.vel.x *= 0.9f;
	prt.vel.y *= 0.9f;

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

	float w = 50 * (float)tile;

	float h = 25 * (float)tile;

	if (pos.x < 0) {
		pos.x = 0;
	}
	if (pos.x + width > w) {
		pos.x = w - width;
	}
	if (pos.y < 0) {
		pos.y = 0;
	}
	if (pos.y + height > h) {
		pos.y = h - height;
	}
}

// 共有の当たり判定の関数（aabb判定）
bool isHit(const Character& a, const Character& b) { return (a.pos.x < b.pos.x + b.width && a.pos.x + a.width > b.pos.x && a.pos.y < b.pos.y + b.height && a.pos.y + a.height > b.pos.y); }

bool isSkillHit(const skillCharacter& a, const Character& b) { return (a.pos.x < b.pos.x + b.width && a.pos.x + a.width > b.pos.x && a.pos.y < b.pos.y + b.height && a.pos.y + a.height > b.pos.y); }

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
	if (cooldown > 0) {
		return;
	}

	for (int i = 0; i < rangeMax; i++) {
		if (!range[i].isAlive) {
			range[i] = Attack_Range(attacker);
			range[i].isAlive = true;
			cooldown = 5;

			return;
		}
	}
}

// リロードの時間関数
void UpdateReload(Reload& reload, int rangMax) {
	if (!reload.isReload) {
		return;
	}

	reload.reload_time--;
	if (reload.reload_time <= 0) {
		reload.isReload = false;
		int need = rangMax - reload.nowBullet;
		if (need < 0)
			need = 0;

		int canLoad = reload.bulletMax; // 予備弾
		if (canLoad > need) {
			canLoad = need;
		}

		// ---- 実際に装填 ----
		reload.nowBullet += canLoad;
		reload.bulletMax -= canLoad;
		reload.reload_time = 90;
		if (reload.bulletMax == 0) {
			reload.bulletMax = 200;
			reload.reload_time = 90;
		}
		return;
	}
}

// 共有攻撃存在時間関数
void UpdateAttackTime(Attack& atk) {
	if (!atk.isAlive)
		return;

	atk.lifeTime--;
	if (atk.lifeTime <= 0) {
		atk.isAlive = false;
		return;
	}

	// 攻撃の移動（弾）
	atk.pos.x += atk.vel.x * atk.speed;
	atk.pos.y += atk.vel.y * atk.speed;

	// 画面外出てない
	// if (atk.pos.x < 0 || atk.pos.x > kWindowWitch || atk.pos.y < 0 || atk.pos.y > kWindowHeight) {
	//	atk.isAlive = false;
	//}
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
void Attack_Fire(Character& attacker, Attack attackArray[], int attackMax, bool isAttacking, int& shootCooldown) {
	if (!isAttacking)
		return;

	// 近接
	if (attackArray[0].type == MELEE) {
		SpawnMelee(attacker, attackArray, attackMax);
		return;
	}

	// 遠距離
	if (attackArray[0].type == RANGE) {
		if (shootCooldown <= 0) {
			SpawnRange(attacker, attackArray, attackMax, shootCooldown);
		}
	}
}

HitEvent Attack_Update(Character& attacker, Character& target, Attack attackArray[], int attackMax) {
	HitEvent result{};
	result.hit = false;

	for (int i = 0; i < attackMax; i++) {

		Attack& atk = attackArray[i];
		if (!atk.isAlive)
			continue;

		// 近接攻撃Pos
		if (atk.type == MELEE) {
			atk.pos.x = attacker.pos.x + attacker.dir.x * attacker.width;
			atk.pos.y = attacker.pos.y + attacker.dir.y * attacker.height;
		}

		// 存在時間、
		UpdateAttackTime(atk);

		// 弾とタイル
		if (atk.type == RANGE) {
			if (BulletTileHit(atk, map, tile)) {
				atk.isAlive = false;
				continue;
			}
		}

		// 当たり判定
		if (CheckHit(atk, target)) {
			result.hit = true;

			result.hitPos = {atk.pos.x + atk.width * 0.5f, atk.pos.y + atk.height * 0.5f};

			if (atk.type == RANGE) {
				atk.isAlive = false;
			}
		}
	}

	return result;
}

//================================================================================================================================================================================================================
//================================================================================================================================================================================================================
//================================================================================================================================================================================================================

// 画面が揺れるの強さ
void Shak(Camera& camera) {
	if (camera.shankeTime > 0) {
		camera.shankeTime--;
		camera.shakeOffset.x = (float)(rand() % 3 - 1) * camera.shakeProw;
		camera.shakeOffset.y = (float)(rand() % 3 - 1) * camera.shakeProw;
	} else {
		camera.shakeOffset.x = 0.0f;
		camera.shakeOffset.y = 0.0f;
	}
}

void UpdateCamera(Camera& camera, Player& player) {

	float px = player.base.pos.x + player.base.width * 0.5f;
	float py = player.base.pos.y + player.base.height * 0.5f;

	float screenX = px - camera.worldOffset.x;
	float screenY = py - camera.worldOffset.y;

	// int deadX = 200;
	// int deadY = 150;
	// int deadW = 1280 - 400;
	// int deadH = 720 - 300;

	// 左
	if (screenX < deadX) {
		camera.worldOffset.x = px - deadX;
	}

	// 右
	if (screenX > deadX + deadW) {
		camera.worldOffset.x = px - (deadX + deadW);
	}
	// 上
	if (screenY < deadY) {
		camera.worldOffset.y = py - deadY;
	}

	// 下
	if (screenY > deadY + deadH) {
		camera.worldOffset.y = py - (deadY + deadH);
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

//=====================================================================================================================================================================================================================
// bossの攻撃関数
// ====================================================================================================================================================================================================================

// wait	関数
bool UpdateSkillWait(skillCharacter& skill) {
	if (!skill.isWait) {
		return true;
	}

	skill.waitTime--;
	if (skill.waitTime > 0) {
		return false;
	}

	if (skill.waitTime <= 0) {
		skill.isAlive = true;
	}

	skill.isWait = false;

	return true;
}

// スキル１



// スキル2　　　レーザー
// レーザーの生成
void SpawLaser(Laser& laser, Character& boss, Character& player) {
	if (!laser.base.isAlive && !laser.base.isWait) {
		laser = InitLaser(boss, LASER, player);
		// laser.base.isAlive = false;
		laser.base.isWait = true;
	}
}

// レーザーの更新

void UpdateLaser(Laser& laser) {

	if (laser.base.isWait) {
		if (!UpdateSkillWait(laser.base)) {
			return;
		}

		laser.base.isAlive = true;
		laser.base.isWait = false;
		return;
	}

	if (!laser.base.isAlive) {
		return;
	}

	laser.angle += laser.rotateSpeed;

	// 超出角度限制 → 翻转方向
	if (laser.angle > laser.startAngle + laser.rotateLimit || laser.angle < laser.startAngle - laser.rotateLimit) {
		laser.rotateSpeed *= -1.0f;
	}

	laser.base.aliveTime--;
	if (laser.base.aliveTime <= 0) {
		laser.base.isAlive = false;
	}
}

// レーザーの描画
void DrawLaser(Laser& laser, float camX, float camY, int tex) {
	if (!laser.base.isAlive && !laser.base.isWait)
		return;

	float cx = laser.base.pos.x - camX;
	float cy = laser.base.pos.y - camY;

	float w = laser.base.width;
	float h = laser.base.height;

	//===== 等待阶段：预兆线 =====
	if (laser.base.isWait) {
		float ex = cx + cosf(laser.angle) * w;
		float ey = cy + sinf(laser.angle) * w;

		Novice::DrawLine((int)cx, (int)cy, (int)ex, (int)ey, RED);
		return;
	}

	//===== 计算旋转矩形顶点 =====
	float dx = cosf(laser.angle); // 方向单位向量
	float dy = sinf(laser.angle);

	float fx = dx * w;
	float fy = dy * w;

	float nx = -dy * (h * 0.5f); // 法线方向（半高度）
	float ny = dx * (h * 0.5f);

	Vector2 baseL = {cx, cy};           // 左边中点
	Vector2 baseR = {cx + fx, cy + fy}; // 右边中点

	Vector2 pLT = {baseL.x + nx, baseL.y + ny}; // 左上
	Vector2 pLB = {baseL.x - nx, baseL.y - ny}; // 左下
	Vector2 pRB = {baseR.x - nx, baseR.y - ny}; // 右下
	Vector2 pRT = {baseR.x + nx, baseR.y + ny}; // 右上

	Novice::DrawQuad(
	    (int)pLT.x, (int)pLT.y, // 左上
	    (int)pRT.x, (int)pRT.y, // 右上
	    (int)pLB.x, (int)pLB.y, // 左下
	    (int)pRB.x, (int)pRB.y, // 右下
	    0, 0, (int)laser.base.width, (int)laser.base.height, tex, WHITE);
}

// 目からレーザー
// 生成
void SpawEyeLaser(EyeLaser eyelaser[], int max, int Map[25][50], int Tile) {

	for (int i = 0; i < max; i++) {
		if (!eyelaser[i].base.isAlive && !eyelaser[i].base.isWait) {

			while (true) {

				int tx = rand() % 50;
				int ty = rand() % 25;

				if (Map[ty][tx] == 1 && Map[ty + 5][tx + 5] == 1 && Map[ty - 5][tx - 5] == 1) {

					Vector2 v{};

					v.x = (float)(tx * Tile + Tile);
					v.y = (float)(ty * Tile + Tile);

					eyelaser[i] = InitEyeLaser(v, EYELASER);

					eyelaser[i].base.isWait = true;
					eyelaser[i].base.isAlive = false;
					break;
				}
			}

			// return;
		}
	}
}

// 目からレーザー更新
void UpdateEyelaser(EyeLaser eyelaser[], int max) {
	for (int i = 0; i < max; i++) {

		if (eyelaser[i].base.isWait) {
			if (UpdateSkillWait(eyelaser[i].base)) {
				eyelaser[i].base.isAlive = true;
				eyelaser[i].base.isWait = false;
			}

			continue;
		}

		if (!eyelaser[i].base.isAlive) {
			continue;
		}

		eyelaser[i].base.aliveTime--;
		if (eyelaser[i].base.aliveTime <= 0) {
			eyelaser[i].base.isAlive = false;
		}

		// 更新
	}
}

// 目からレーザー生成
void DrawEyeLaser(EyeLaser eyelaser[], int max, float camX, float camY) {
	for (int i = 0; i < max; i++) {
		if (!eyelaser[i].base.isAlive && !eyelaser[i].base.isWait) {
			continue;
		}

		if (eyelaser[i].base.isWait) {
			Novice::DrawBox((int)eyelaser[i].base.pos.x - (int)camX, (int)eyelaser[i].base.pos.y - (int)camY, (int)eyelaser[i].base.width, (int)eyelaser[i].base.height, 0.0f, RED, kFillModeWireFrame);

		} else if (eyelaser[i].base.isAlive) {
			Novice::DrawBox((int)eyelaser[i].base.pos.x - (int)camX, (int)eyelaser[i].base.pos.y - (int)camY, (int)eyelaser[i].base.width, (int)eyelaser[i].base.height, 0.0f, RED, kFillModeSolid);
		}
	}
}

// 地面攻撃生成
void SpawGroundSlam(GroundSlam groundSlam[], int max, int Map[25][50], int Tile) {

	for (int i = 0; i < max; i++) {
		if (!groundSlam[i].base.isAlive && !groundSlam[i].base.isWait) {

			while (true) {

				int tx = rand() % 50;
				int ty = rand() % 25;

				if (tx <= 5 || tx >= 44 || ty <= 5 || ty >= 19) {
					continue;
				}

				if (Map[ty][tx] == 1 && Map[ty + 3][tx + 3] == 1 && Map[ty - 3][tx - 3] == 1) {

					Vector2 v{};

					v.x = (float)(tx * Tile + Tile);
					v.y = (float)(ty * Tile + Tile);

					groundSlam[i] = InitgroundSlam(v, GROUNDSLAM);

					groundSlam[i].base.isWait = true;
					groundSlam[i].base.isAlive = false;
					break;
				}
			}
		}
	}
}

// 地面攻撃更新
void UpdateGroundSlam(GroundSlam groundSlam[], int max) {
	for (int i = 0; i < max; i++) {

		if (groundSlam[i].base.isWait) {
			if (UpdateSkillWait(groundSlam[i].base)) {
				groundSlam[i].base.isAlive = true;
				groundSlam[i].base.isWait = false;
			}

			continue;
		}

		if (!groundSlam[i].base.isAlive) {
			continue;
		}

		groundSlam[i].base.aliveTime--;
		if (groundSlam[i].base.aliveTime <= 0) {
			groundSlam[i].base.isAlive = false;
		}

		// 更新
	}
}

// 地面攻撃描画
void DrawGroundSlam(GroundSlam groundSlam[], int max, float camX, float camY) {
	for (int i = 0; i < max; i++) {
		if (!groundSlam[i].base.isAlive && !groundSlam[i].base.isWait) {
			continue;
		}

		if (groundSlam[i].base.isWait) {
			Novice::DrawBox(
			    (int)groundSlam[i].base.pos.x - (int)camX, (int)groundSlam[i].base.pos.y - (int)camY, (int)groundSlam[i].base.width, (int)groundSlam[i].base.height, 0.0f, RED, kFillModeWireFrame);

		} else if (groundSlam[i].base.isAlive) {
			Novice::DrawBox(
			    (int)groundSlam[i].base.pos.x - (int)camX, (int)groundSlam[i].base.pos.y - (int)camY, (int)groundSlam[i].base.width, (int)groundSlam[i].base.height, 0.0f, RED, kFillModeSolid);
		}
	}
}

//=====================================================================================================================================================================================================================
//
// ====================================================================================================================================================================================================================

// もう一回遊び、数値をリセットする
void ALL(Player& player, Boss& boss, Camera& camera, Attack melee[], Attack range[], int meleeMax, int rangeMax) {
	attack_type = false;

	player = InitPlayer(640.0f, 600.0f);
	//
	boss = InitBoss(640.0f, 320.0f);

	camera = {
	    .worldOffset = {0.0f, 0.0f},
	    .shakeOffset = {0.0f, 0.0f},

	    .shankeTime = 0,
	    .shakeProw = 0.0f,
	};
	//
	meleeMax = 1;
	rangeMax = 32;
	for (int i = 0; i < meleeMax; i++) {
		melee[i].pos = player.base.pos;
		melee[i].vel = player.base.dir;
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
		range[i].vel = player.base.dir;
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
	    .worldOffset = {0.0f, 0.0f},
	    .shakeOffset = {0.0f, 0.0f},
	    .shankeTime = 0,
	    .shakeProw = 0.0f,
	};

	// レーザー
	Laser laser = InitLaser(boss.base, LASER, player.base);

	// 目からレーザー
	const int eyelaserMax = 3;
	EyeLaser eyeLaser[eyelaserMax]{};

	// 地面攻撃
	const int groundSlamMax = 4;
	GroundSlam groundSlam[groundSlamMax];

	// 弾発射
	
	bool laserSkillStarted = false;
	bool eyeSkillStarted = false;
	bool groundSkillStarted = false;
	bool bossShootStarted = false;

	int skillCountMax = 4; //

	const int meleeMax = 1;
	const int rangeMax = 40;
	Attack player_melee[meleeMax]{};
	Attack player_range[rangeMax]{};

	// リロード
	Reload reload{
	    .reload_time = 90,
	    .nowBullet = rangeMax,
	    .bulletMax = 200,
	    .isReload = false,
	};

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
	Novice::Initialize(kWindowTitle, kWindowWitch, kWindowHeight);

	int laserImager = Novice::LoadTexture("./imager/laser.png");
	int tileMapImager = Novice::LoadTexture("./imager/tileMap.png");
	int playerImager = Novice::LoadTexture("./imager/player.png");

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

		float px = player.base.pos.x + player.base.width / 2.0f;
		float py = player.base.pos.y + player.base.height / 2.0f;

		float camX = camera.worldOffset.x + camera.shakeOffset.x;
		float camY = camera.worldOffset.y + camera.shakeOffset.y;

		Vector2 mouseWorld{mouse.pos.x + camX, mouse.pos.y + camY};

		player.base.dir.x = mouseWorld.x - px;
		player.base.dir.y = mouseWorld.y - py;

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

			UpdateCamera(camera, player);
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
				camera.shakeOffset = {0.0f, 0.0f};
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
				player.base.vel = {0.0f, 0.0f};

				if (keys[DIK_W]) {
					player.base.vel.y = -1.0f;
				}
				if (keys[DIK_S]) {
					player.base.vel.y = 1.0f;
				}
				if (keys[DIK_A]) {
					player.base.vel.x = -1.0f;
				}
				if (keys[DIK_D]) {
					player.base.vel.x = 1.0f;
				}

				if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {
					player.base.isDash = true;
					player.base.dash_time = 0;
				}
				// 移動関数使う
				Dash(player.base);
				Move(player.base.pos, player.base.vel, player.base.width, player.base.height, player.base.speed);

				TileHit(&player.base.pos, &player.base.vel, player.base.width, player.base.height, map, tile);

				// 手動リロード（Rキー）
				if (keys[DIK_R] && !preKeys[DIK_R]) {
					if (!reload.isReload && reload.nowBullet < rangeMax) {
						reload.isReload = true;
						reload.reload_time = 90;
					}
				}

				// 自動リロード
				if (reload.nowBullet <= 0 && !reload.isReload) {
					reload.isReload = true;
					reload.reload_time = 90;
				}

				// リロード更新
				UpdateReload(reload, rangeMax);

				if (keys[DIK_E] && !preKeys[DIK_E]) {
					attack_type = !attack_type;
				}

				if (attack_type == false) { // 弾発射
					bool wantFire = Novice::IsPressMouse(0);
					HitEvent e = Attack_Update(player.base, boss.base, player_range, rangeMax);

					if (!reload.isReload && reload.nowBullet > 0) {
						Attack_Fire(player.base, player_range, rangeMax, wantFire, player.base.shootCooldown);

						// 発射した瞬間だけ弾を1発消費
						if (wantFire && player.base.shootCooldown == 5) {
							reload.nowBullet--;
						}

						// 命中イベント
						if (e.hit) {
							ApplyDamage(boss.base, player.base.bulletdamege);
							ApplyCameraShake(camera, 5, 10);
							SpawPrt(e.hitPos, PRT_BLODD, blood, prtmax);
						}
					}

				} else { // 近接

					bool wantFire = Novice::IsTriggerMouse(0);

					HitEvent e = Attack_Update(player.base, boss.base, player_melee, meleeMax);
					Attack_Fire(player.base, player_melee, meleeMax, wantFire, player.base.shootCooldown);

					if (e.hit) {
						ApplyDamage(boss.base, player.base.damage);
						ApplyCameraShake(camera, 5, 10);
						SpawPrt(e.hitPos, PRT_BLODD, blood, prtmax);
					}
				}
				Particle_UpdateAll(blood, prtmax);

				// playerとボースの当たり判定
				if (!player.base.isInvincible) {
					if (isHit(player.base, boss.base)) {
						ApplyDamage(player.base, boss.base.damage);
					}
				}
			}

			//==============================================================================================================================================================================================================================
			// bossの攻撃
			//===============================================================================================================================================================================================================================

			float PX = player.base.pos.x + player.base.width / 2;
			float PY = player.base.pos.y + player.base.height / 2;

			float BX = boss.base.pos.x + boss.base.width / 2;
			float BY = boss.base.pos.y + boss.base.height / 2;

			boss.base.dir.x = PX - BX;
			boss.base.dir.y = PY - BY;

			float LEN = sqrtf(boss.base.dir.x * boss.base.dir.x + boss.base.dir.y * boss.base.dir.y);
			if (LEN > 0) {
				boss.base.dir.x /= LEN;
				boss.base.dir.y /= LEN;
			}

			if (boss.base.shootCooldown > 0) {
				boss.base.shootCooldown--;
			}

			boss.skillCooldown--;
			if (boss.skillCooldown < 0)
				boss.skillCooldown = 0;

			switch (boss_start) {

			case IDLE: // ===============================
				if (boss.skillCooldown <= 0) {
					boss_start = SELECT_SKILL;
				}
				break;

			case SELECT_SKILL: // =========================
			{
				int r = rand() % skillCountMax;

				if (r == 0) {
					boss.currentSkill = SHOOT;
				}
				if (r == 1) {
					boss.currentSkill = LASER;
				}
				if (r == 2) {
					boss.currentSkill = EYELASER;
				}
				if (r == 3) {
					boss.currentSkill = GROUNDSLAM;
				}

				// 重置技能开始标志
				laserSkillStarted = false;
				eyeSkillStarted = false;
				groundSkillStarted = false;

				boss_start = CAST_SKILL;
			} break;

			case CAST_SKILL: // ==========================
				switch (boss.currentSkill) {

				// ---------------------------------------
				case SHOOT:
					
							boss_start = IDLE;
							boss.skillCooldown = 120;
							bossShootStarted = false;
					
					break;

				// ---------------------------------------
				case LASER:
					if (!laserSkillStarted) {
						SpawLaser(laser, boss.base, player.base);
						laserSkillStarted = true;
					}

					if (!laser.base.isAlive && !laser.base.isWait && laserSkillStarted) {
						boss_start = IDLE;
						boss.skillCooldown = 160;
						laserSkillStarted = false;
					}
					break;

				// ---------------------------------------
				case EYELASER: {
					if (!eyeSkillStarted) {
						SpawEyeLaser(eyeLaser, eyelaserMax, map, tile);
						eyeSkillStarted = true;
					}

					bool allDone = true;
					for (int i = 0; i < eyelaserMax; i++) {
						if (eyeLaser[i].base.isAlive || eyeLaser[i].base.isWait) {
							allDone = false;
							break;
						}
					}

					if (allDone && eyeSkillStarted) {
						boss_start = IDLE;
						boss.skillCooldown = 200;
						eyeSkillStarted = false;
					}
				} break;

				// ---------------------------------------
				case GROUNDSLAM: {
					if (!groundSkillStarted) {
						SpawGroundSlam(groundSlam, groundSlamMax, map, tile);
						groundSkillStarted = true;
					}

					bool allDone = true;
					for (int i = 0; i < groundSlamMax; i++) {
						if (groundSlam[i].base.isAlive || groundSlam[i].base.isWait) {
							allDone = false;
							break;
						}
					}

					if (allDone && groundSkillStarted) {
						boss_start = IDLE;
						boss.skillCooldown = 180;
						groundSkillStarted = false;
					}
				} break;
				}
				break;
			}

			// レーザー
			// UpdateSkillWait(laser.base);
			UpdateLaser(laser);

			// 目からレーザー
			UpdateEyelaser(eyeLaser, eyelaserMax);

			// 地面攻撃
			UpdateGroundSlam(groundSlam, groundSlamMax);
			// 弾発射
			

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

		if (keys[DIK_F11] && !preKeys[DIK_F11]) {
			fullscreen = !fullscreen;
			if (fullscreen) {
				Novice::SetWindowMode(kFullscreen);
			} else {

				Novice::SetWindowMode(kWindowed);
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
			Novice::DrawBox(0 - (int)camX, 0 - (int)camY, 1600, 800, 0.0f, BLACK, kFillModeSolid);

			Novice::DrawBox((int)boss.base.pos.x - (int)camX, (int)boss.base.pos.y - (int)camY, (int)boss.base.width, (int)boss.base.height, 0.0f, WHITE, kFillModeWireFrame);
			// player
			Novice::DrawBox((int)player.base.pos.x - (int)camX, (int)player.base.pos.y - (int)camY, (int)player.base.width, (int)player.base.height, 0.0f, WHITE, kFillModeSolid);
			Novice::DrawSprite((int)player.base.pos.x - (int)camX, (int)player.base.pos.y - (int)camY, playerImager, 1, 1, 0.0f, WHITE);
			for (int i = 0; i < meleeMax; i++) {
				if (player_melee[i].isAlive) {

					Novice::DrawBox(
					    (int)player_melee[i].pos.x - (int)camX, (int)player_melee[i].pos.y - (int)camY, (int)player_melee[i].width, (int)player_melee[i].height, 0.0f, WHITE, kFillModeSolid);
				}
			}

			for (int i = 0; i < rangeMax; i++) {
				if (player_range[i].isAlive) {

					Novice::DrawBox(
					    (int)player_range[i].pos.x - (int)camX, (int)player_range[i].pos.y - (int)camY, (int)player_range[i].width, (int)player_range[i].height, 0.0f, WHITE, kFillModeSolid);
				}
			}

			// レーザー
			DrawLaser(laser, camX, camY, laserImager);

			for (int y = 0; y < 25; y++) {
				for (int x = 0; x < 50; x++) {
					if (map[y][x] == 2) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMapImager, 1, 1, 0.0f, WHITE);
					}
				}
			}

			// boss

			// boss　のスキル　描画

			// 目からレーザー
			DrawEyeLaser(eyeLaser, eyelaserMax, camX, camY);

			// 地面攻撃
			DrawGroundSlam(groundSlam, groundSlamMax, camX, camY);

			// 弾発射
			

			// 血

			for (int i = 0; i < prtmax; i++) {
				if (blood[i].isAlive) {
					Novice::DrawBox((int)blood[i].pos.x - (int)camX, (int)blood[i].pos.y - (int)camY, (int)blood[i].width, (int)blood[i].height, 0.0f, RED, kFillModeSolid);
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
				Novice::ScreenPrintf(0, Y += H, "nowBullet: %d", reload.nowBullet);
				Novice::ScreenPrintf(0, Y += H, "ReloadMax: %d", reload.bulletMax);
				Novice::ScreenPrintf(0, Y += H, "ReloadTime: %f", reload.reload_time);
				Novice::ScreenPrintf(0, Y += H, "isReload: %d", reload.isReload);
				Novice::ScreenPrintf(0, Y += H, "playerDir_X: %f", player.base.dir.x);
				Novice::ScreenPrintf(0, Y += H, "playerDir_Y: %f", player.base.dir.y);
				Novice::ScreenPrintf(0, Y += H, "BossShootTime: %d", boss.base.shootCooldown);
				Novice::ScreenPrintf(0, Y += H, "BOSSDir_X: %f", boss.base.dir.x);
				Novice::ScreenPrintf(0, Y += H, "BOSSDir_Y: %f", boss.base.dir.y);
				Novice::ScreenPrintf(0, Y += H, "laser_waitTime:  %d", laser.base.waitTime);
				Novice::ScreenPrintf(0, Y += H, "laser_Iswait:  %d", laser.base.isWait);
				Novice::ScreenPrintf(0, Y += H, "laser_aliveTime:  %d", laser.base.aliveTime);
				Novice::ScreenPrintf(0, Y += H, "laser_IsAliveTime:  %d", laser.base.isAlive);
				Novice::ScreenPrintf(0, Y += H, "eyeLaser_IsAliveTime:  %d", eyeLaser->base.aliveTime);
				Novice::ScreenPrintf(0, Y += H, "eyeLaser_waitTime:  %d", eyeLaser->base.waitTime);
				Novice::ScreenPrintf(0, Y += H, " groundPos:  %0.2f,%0.2f", groundSlam->base.pos.x, groundSlam->base.pos.y);
				Novice::ScreenPrintf(0, Y += H, "Boss_State: %d", boss_start);
				Novice::ScreenPrintf(0, Y += H, "Boss_Skill: %d", boss.currentSkill);
				Novice::ScreenPrintf(0, Y += H, "Skill_CD: %d", boss.skillCooldown);
			}

			Novice::DrawLine((int)px - (int)camX, (int)py - (int)camY, (int)mouse.pos.x, (int)mouse.pos.y, RED);

			Novice::DrawBox(deadX, deadY, deadW, deadH, 0.0f, RED, kFillModeWireFrame);

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
