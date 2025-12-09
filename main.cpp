#define _USE_MATH_DEFINES
#include <Novice.h>
#include <math.h>
const char kWindowTitle[] = "GC1B_01_カ_アン";

#pragma region
#pragma endregion

const int kWindowWitch = 1280;
const int kWindowHeight = 720;
int loadingTimer = 150;
int introTimer = 90;
int hitTimer = 15;
bool debug = true;
bool fullscreen = false;
bool bossRoomEntered = false;

int deadX = 250;
int deadY = 150;
int deadW = 1280 - 500;
int deadH = 720 - 300;

typedef enum {
	START, // タイトル
	FIGHT, // 戦い
	INTRO,
	STOP,
	WIN,     // 勝利
	LOSE,    // 負け
	MANUAL,  //
	LOADING, //
	HIT,
} GAME_STATE;
GAME_STATE game_state = START;

//======================================================================
// boss
//=====================================================================

//
enum BOSS_PHASE {
	PHASE_1,
	PHASE_2,
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
	CHARGE,     // ダシュル
};
BOSS_SKILL boss_shill = SHOOT;

enum BossDir { LEFT, RIGHT, MAX };

enum BossAnim {
	BOSS_ANIM_IDLE,       // 待機
	BOSS_ANIM_SHOOT,      // 散弾
	BOSS_ANIM_LASER,      // レーザー
	BOSS_ANIM_GROUNDSLAM, // 地面攻撃
	BOSS_ANIM_CHARGE,     // ダッシュ
	BOSS_ANIM_INTRO,
	BOSS_ANIM_DIE,
	BOSS_ANIM_MAX
};

//=========================================================================================================

enum PlayerDir { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_MAX };

enum PlayerState { PST_IDLE, PST_RUN, PST_ATTACK, PST_DASH, PST_HIT, PST_DIE, PST_MAX };

//================================================================================
//
// // player攻撃の種類
enum AttackType {
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
	int tex1;
	int tex2;
	bool isHover;
	bool isClicked;
	bool winEnd;
	bool dieEnd;
	int tex; // 対応画像
	float frameTime;
	int frameCount;   // アニメーション何枚
	int currentFrame; // 今の画像何枚
	float animTimer;
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
	int maxHP;
	int hpTex;
	int tex[DIR_MAX][PST_MAX]; // 対応画像
	float frameTime[PST_MAX];
	int frameCount[DIR_MAX][PST_MAX]; // アニメーション何枚
	int currentFrame;                 // 今の画像何枚
	float animTimer;
	bool animEnd;
	PlayerDir dir;
	PlayerState state;
};
// Boss
struct Boss {
	Character base;
	int maxHP;
	int skillCooldown;
	int skillWaitTime;
	Vector2 bossLockedDir;
	bool bossDirLocked;
	BOSS_SKILL currentSkill;
	//
	int tex[DIR_MAX][BOSS_ANIM_MAX];
	float frameTime[BOSS_ANIM_MAX];
	int frameCount[DIR_MAX][BOSS_ANIM_MAX];
	int currentFrame;
	float animTimer;
	bool animEnd;
	BossDir dir;
	BossAnim anim;
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

struct ScatterShot {
	skillCharacter base;
	Vector2 dir;
	float speed;
	float angle;
};

struct Laser {
	skillCharacter base;
	float angle;
	float rotateSpeed;
	float rotateLimit;
	float startAngle;

	//
	int tex1;
	int tex2;
	int frameCount;
	float frameTime;
	int currentFrame;
	float animTimer;
};

struct GroundSlam {
	skillCharacter base;
	int tex1;
	int tex2;
	int frameCount;
	float frameTime;
	int currentFrame;
	float animTimer;
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
	p.base.hp = 5;
	p.maxHP = p.base.hp;
	p.base.damage = 10;
	p.base.invincible_time = 60;
	p.base.shootCooldown = 0;
	p.base.width = 32.0f;
	p.base.height = 48.0f;
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
	//
	p.state = PST_IDLE;
	p.dir = DIR_RIGHT;
	p.currentFrame = 0;
	p.animEnd = false;
	p.animTimer = 0.0f;

	p.frameTime[PST_IDLE] = 0.25f;
	p.frameTime[PST_RUN] = 0.12f;
	p.frameTime[PST_ATTACK] = 0.08f;
	p.frameTime[PST_DASH] = 0.05f;
	p.frameTime[PST_DIE] = 0.1f;

	return p;
}

// ボース
Boss InitBoss(float x, float y) {
	Boss b{};
	b.base.pos = {x, y};
	b.base.vel = {0.0f, 0.0f};
	b.base.dir = {1.0f, 0.0f};
	b.base.hp = 1000;
	b.base.damage = 1;
	b.base.invincible_time = 60;
	b.base.shootCooldown = 90;
	b.base.width = 96.0f;
	b.base.height = 136.0f;
	b.base.speed = 0;
	b.base.dash_time = 0;
	b.base.isDash = false;
	b.base.isAlive = true;
	b.base.isHit = false;
	b.base.isInvincible = false;
	b.maxHP = b.base.hp;
	b.skillCooldown = 0;
	b.skillWaitTime = 60;
	b.currentSkill = SHOOT;
	b.bossDirLocked = false;
	boss_start = IDLE;

	b.dir = RIGHT;
	b.currentFrame = 0;
	b.animTimer = 0.0f;
	b.animEnd = false;
	b.anim = BOSS_ANIM_IDLE;

	b.frameTime[BOSS_ANIM_IDLE] = 0.25f;
	b.frameTime[BOSS_ANIM_SHOOT] = 0.12f;
	b.frameTime[BOSS_ANIM_GROUNDSLAM] = 0.12f;
	b.frameTime[BOSS_ANIM_LASER] = 0.1f;
	b.frameTime[BOSS_ANIM_CHARGE] = 0.09f;
	b.frameTime[BOSS_ANIM_INTRO] = 0.08f;
	b.frameTime[BOSS_ANIM_DIE] = 0.08f;

	return b;
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

ScatterShot InitScatterShot(Vector2 pos, Vector2 dir, BOSS_SKILL type) {
	ScatterShot s{};
	s.base.pos = pos;
	s.dir = dir;
	s.speed = 10.0f;
	s.base.width = 25.f;
	s.base.height = 25.f;
	s.base.type = type;
	s.base.isAlive = false;
	s.base.isWait = false;
	s.base.waitTime = 20;
	s.base.aliveTime = 120;
	s.base.damege = 1;
	return s;
}

Laser InitLaser(Laser& l, Character& boss, BOSS_SKILL type, Character& player, BOSS_PHASE phase) {
	l.base.pos = {boss.pos.x + boss.width * 0.5f, boss.pos.y + boss.height * 0.75f};
	l.base.vel = {0.0f, 0.0f};
	l.base.type = type;
	l.base.isAlive = false;
	l.base.isWait = false;

	float px = player.pos.x + player.width * 0.5f;
	float py = player.pos.y + player.height * 0.5f;
	float bx = boss.pos.x + boss.width * 0.5f;
	float by = boss.pos.y + boss.height * 0.5f;

	float angle = atan2f(py - by, px - bx);
	l.angle = angle;
	l.startAngle = angle;

	float velY = by - py;
	float dirY = velY > 0 ? 1.0f : -1.0f;

	if (phase == PHASE_1) {
		l.base.width = 500.0f;
		l.base.height = 50.0f;
		l.rotateSpeed = dirY * 0.02f;
		l.rotateLimit = 0.6f;
		l.base.aliveTime = 90;
		l.base.damege = 1;
		l.base.waitTime = 60;
	} else { // PHASE 2
		l.base.width = 750.0f;
		l.base.height = 80.0f;

		l.rotateSpeed = dirY * 0.015f;
		l.rotateLimit = 3.14159f * 2.0f;
		l.base.aliveTime = 90;
		l.base.damege = 1;

		l.base.waitTime = 60;
	}

	return l;
}
void InitgroundSlam(GroundSlam& g, Vector2 pos, BOSS_SKILL type, BOSS_PHASE phase) {
	g.base.pos = pos;
	g.base.vel = {0.0f, 0.0f};
	g.base.type = type;

	g.base.isAlive = false;
	g.base.isWait = false;
	g.base.waitTime = 40;
	g.base.aliveTime = 60;

	if (phase == PHASE_1) {
		g.base.width = 144.0f;
		g.base.height = 144.0f;
		g.base.damege = 1;
	} else {
		g.base.width = 2.0f;
		g.base.height = 2.0f;
		g.base.damege = 1;
	}
	g.currentFrame = 0;
	g.animTimer = 0.0f;
	
}

//===============================================================================
// 関数
//================================================================================
// マップ
int tile = 32;
int map[45][40] = {
    {6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5},
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 1, 1, 1, 1, 6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, //
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 1, 1, 1, 1, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 6, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 7, 1, 1, 1, 1, 1, 1, 1, 1}, //  38
    {1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1},
};
int initialMap[45][40];

void TileHit(Vector2* pos, Vector2* vel, float width, float height, int Map[45][40], int tile_sezi) {

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

		if (vel->y < 0 && ((Map[tyT][txL] == 2 || Map[tyT][txR] == 2) || (Map[tyT][txL] == 4 || Map[tyT][txR] == 4) || (Map[tyT][txL] == 5 || Map[tyT][txR] == 5))) {
			pos->y = (tyT + 1) * (float)tile_sezi;
		}
		if (vel->y > 0 && ((Map[tyB][txL] == 2 || Map[tyB][txR] == 2) || (Map[tyB][txL] == 4 || Map[tyB][txR] == 4) || (Map[tyB][txL] == 5 || Map[tyB][txR] == 5))) {
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

		if (vel->x < 0 && ((Map[tyT][txL] == 2 || Map[tyB][txL] == 2) || (Map[tyT][txL] == 4 || Map[tyB][txL] == 4) || (Map[tyT][txL] == 5 || Map[tyB][txL] == 5))) {
			pos->x = (txL + 1) * (float)tile_sezi;
		}
		if (vel->x > 0 && ((Map[tyT][txR] == 2 || Map[tyB][txR] == 2) || (Map[tyT][txR] == 4 || Map[tyB][txR] == 4) || (Map[tyT][txR] == 5 || Map[tyB][txR] == 5))) {
			pos->x = txR * tile_sezi - width;
		}
	}
}

bool BulletTileHit(Attack atk, int Map[45][40], int tile_sezi) {
	bool isHit = false;
	float px = atk.pos.x + atk.width * 0.5f;
	float py = atk.pos.y + atk.height * 0.5f;

	int tx = (int)px / tile_sezi;
	int ty = (int)py / tile_sezi;

	if (tx < 0 || tx >= 40 || ty < 0 || ty >= 45) {
		return true;
	}

	if (Map[ty][tx] == 2 || Map[ty][tx] == 4 || Map[ty][tx] == 5) {
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

	float w = 40 * (float)tile;

	float h = 45 * (float)tile;

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
}

// 共有無敵関数
void ApplyDamage(Character& target, int damage, bool isPlayer) {
	if (target.isInvincible)
		return;

	target.hp -= damage;
	target.isInvincible = true;
	target.invincible_time = 30;

	if (isPlayer) {
		game_state = HIT;
		hitTimer = 15;
	}
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

	// 遠距離
	if (attackArray[0].type == RANGE) {
		if (shootCooldown <= 0) {
			SpawnRange(attacker, attackArray, attackMax, shootCooldown);
		}
	}
}

HitEvent Attack_Update(Character& target, Attack attackArray[], int attackMax) {
	HitEvent result{};
	result.hit = false;

	for (int i = 0; i < attackMax; i++) {

		Attack& atk = attackArray[i];
		if (!atk.isAlive)
			continue;

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
void DrawButton(UI& box) {
	int tex = box.isHover ? box.tex1 : box.tex2;
	Novice::DrawSprite(box.x, box.y, tex, 1, 1, 0.0f, WHITE);
}

// playerのアニメーション
void UpdatePlayerAnim(Player& p, float deltaTime) {

	p.animTimer += deltaTime;

	int frames = p.frameCount[p.dir][p.state];
	float speed = p.frameTime[p.state];
	if (p.animTimer >= speed) {
		p.animTimer = 0;
		p.currentFrame++;
		if (p.currentFrame >= frames) {
			p.currentFrame = 0;
		}
	}
}

// player死ぬアニメーショ
void UpdatePlayerDieAnim(Player& p, UI& dieBox, float deltaTime) {

	p.animTimer += deltaTime;

	int frames = p.frameCount[p.dir][p.state];
	float speed = p.frameTime[p.state];
	if (p.animTimer >= speed) {
		p.animTimer = 0;
		p.currentFrame++;
		if (p.currentFrame >= frames) {
			p.currentFrame = 10;
			p.animEnd = true;
			dieBox.winEnd = true;
		}
	}
}

void DrawPlayerHp(Player& player, float x, float y) {

	int hp = player.base.hp;
	int maxHp = player.maxHP;

	
	//int frameCount = maxHp + 1;
	int frameW = 339;
	int frameH = 96;

	int frameIndex = maxHp - hp;

	// 计算 UV
	int u = 0;
	int v = frameIndex * frameH;

	int tex = player.hpTex;

	Novice::DrawQuad((int)x, (int)y, (int)(x + frameW), (int)y, (int)x, (int)(y + frameH), (int)(x + frameW), (int)(y + frameH), u, v, frameW, frameH, tex, WHITE);
}




// bossアニメーション
void UpdateBossAnim(Boss& b, float deltaTime) {

	b.animTimer += deltaTime;

	int frames = b.frameCount[b.dir][b.anim];
	float speed = b.frameTime[b.anim];
	if (b.animTimer >= speed) {
		b.animTimer = 0;
		b.currentFrame++;
		if (b.currentFrame >= frames) {
			b.currentFrame = 0;
		}
	}
}

// bossHP出てくるアニメーション
void UpdateBossHpAnim(UI& b, float deltaTime) {

	b.animTimer += deltaTime;

	int frames = b.frameCount;
	float speed = b.frameTime;
	if (b.animTimer >= speed) {
		b.animTimer = 0;
		b.currentFrame++;
		if (b.currentFrame >= frames) {
			b.currentFrame = frames - 1;
		}
	}
}

// bossHP描画
void DrawBossHp(Boss& b, UI& hpBox) {
	float nowHp = (float)b.base.hp / (float)b.maxHP;
	float nowHpW = (float)hpBox.w * nowHp;
	Novice::DrawBox(hpBox.x + 9, hpBox.y + 6, (int)nowHpW - 18, hpBox.h - 12, 0.0f, RED, kFillModeSolid);
}

// boss死ぬアニメーション
void UpdateBossDieAnim(Boss& b, UI& winBox, float deltaTime) {

	b.animTimer += deltaTime;

	int frames = b.frameCount[b.dir][b.anim];
	float speed = b.frameTime[b.anim];
	if (b.animTimer >= speed) {
		b.animTimer = 0;
		b.currentFrame++;
		if (b.currentFrame >= frames) {
			b.currentFrame = 29;
			b.animEnd = true;
			winBox.dieEnd = true;
		}
	}
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
// 弾の生成
void SpawnScatterShot(ScatterShot scatter[], int max, Character& boss, int bulletCount, float spread) {

	for (int i = 0; i < bulletCount; i++) {

		for (int j = 0; j < max; j++) {
			if (!scatter[j].base.isAlive && !scatter[j].base.isWait) {

				float startAngle = atan2f(boss.dir.y, boss.dir.x);
				float angle = startAngle + spread * ((i / (float)(bulletCount - 1)) - 0.5f);

				Vector2 dir = {cosf(angle), sinf(angle)};

				Vector2 pos = {boss.pos.x + boss.width * 0.5f, boss.pos.y + boss.height * 0.5f};

				scatter[j] = InitScatterShot(pos, dir, SHOOT);
				scatter[j].base.isWait = true;
				break;
			}
		}
	}
}

// 弾の更新
void UpdateScatterShot(ScatterShot scatter[], int max) {

	for (int i = 0; i < max; i++) {

		if (scatter[i].base.isWait) {
			if (UpdateSkillWait(scatter[i].base)) {
				scatter[i].base.isAlive = true;
			}
			continue;
		}

		if (!scatter[i].base.isAlive)
			continue;

		scatter[i].base.aliveTime--;
		if (scatter[i].base.aliveTime <= 0) {
			scatter[i].base.isAlive = false;
			continue;
		}

		scatter[i].base.pos.x += scatter[i].dir.x * scatter[i].speed;
		scatter[i].base.pos.y += scatter[i].dir.y * scatter[i].speed;

		Attack temp{};
		temp.pos = scatter[i].base.pos;
		temp.width = scatter[i].base.width;
		temp.height = scatter[i].base.height;

		if (BulletTileHit(temp, map, tile)) {
			scatter[i].base.isAlive = false;
			continue;
		}
	}
}

// 弾の更新
void DrawScatterShot(ScatterShot scatter[], int max, float camX, float camY, int tex) {
	for (int i = 0; i < max; i++) {
		if (!scatter[i].base.isAlive && !scatter[i].base.isWait)
			continue;

		Vector2 p = {scatter[i].base.pos.x - camX, scatter[i].base.pos.y - camY};

		if (scatter[i].base.isAlive) {
			Novice::DrawSprite((int)scatter[i].base.pos.x - (int)camX, (int)scatter[i].base.pos.y - (int)camY, tex, 1, 1, 0.0f, WHITE);
		}
	}
}

// スキル2　　　レーザー
// レーザーの生成
void SpawLaser(Laser& laser, Character& boss, Character& player) {
	if (!laser.base.isAlive && !laser.base.isWait) {
		InitLaser(laser, boss, LASER, player, boss_phase);
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
		laser.currentFrame = 0;
		laser.animTimer = 0.0f;
	}

	if (!laser.base.isAlive) {
		return;
	}

	laser.angle += laser.rotateSpeed;

	if (laser.angle > laser.startAngle + laser.rotateLimit || laser.angle < laser.startAngle - laser.rotateLimit) {
		laser.rotateSpeed *= -1.0f;
	}

	laser.base.aliveTime--;
	if (laser.base.aliveTime <= 0) {
		laser.base.isAlive = false;
	}
}

// レーザのアニメーション
void UpdateLaserAnim(Laser& l, float deltaTime) {

	if (!l.base.isAlive)
		return;

	l.animTimer += deltaTime;
	if (l.animTimer >= l.frameTime) {
		l.animTimer = 0.0f;
		l.currentFrame++;
		if (l.currentFrame >= l.frameCount) {
			l.currentFrame = 0;
		}
	}
}

// レーザーの描画
void DrawLaser(Laser& laser, float camX, float camY) {
	if (!laser.base.isAlive && !laser.base.isWait)
		return;

	float cx = laser.base.pos.x - camX;
	float cy = laser.base.pos.y - camY;

	float w = laser.base.width;
	float h = laser.base.height;

	if (laser.base.isWait) {
		float ex = cx + cosf(laser.angle) * w;
		float ey = cy + sinf(laser.angle) * w;

		Novice::DrawLine((int)cx, (int)cy, (int)ex, (int)ey, RED);
		return;
	}

	float dx = cosf(laser.angle);
	float dy = sinf(laser.angle);

	float fx = dx * w;
	float fy = dy * w;

	float nx = -dy * (h * 0.5f);
	float ny = dx * (h * 0.5f);

	Vector2 baseL = {cx, cy};
	Vector2 baseR = {cx + fx, cy + fy};

	Vector2 pLT = {baseL.x + nx, baseL.y + ny}; // 左上
	Vector2 pLB = {baseL.x - nx, baseL.y - ny}; // 左下
	Vector2 pRB = {baseR.x - nx, baseR.y - ny}; // 右下
	Vector2 pRT = {baseR.x + nx, baseR.y + ny}; // 右上

	int tex = (boss_phase == PHASE_1) ? laser.tex1 : laser.tex2;

	int frameW = boss_phase == PHASE_1 ? 500 : 750;
	int frameH = boss_phase == PHASE_1 ? 50 : 80;

	int u = 0;
	int v = frameH * laser.currentFrame;

	Novice::DrawQuad(
	    (int)pLT.x, (int)pLT.y, // 左上
	    (int)pRT.x, (int)pRT.y, // 右上
	    (int)pLB.x, (int)pLB.y, // 左下
	    (int)pRB.x, (int)pRB.y, // 右下
	    u, v, frameW, frameH, tex, WHITE);
}

// 地面攻撃生成
void SpawGroundSlam(GroundSlam groundSlam[], int max, int Map[45][40], int Tile) {

	int slamSize = (boss_phase == PHASE_1) ? 160 : 220;
	int radius = slamSize;

	int mapWidth = 40 * Tile;
	int mapHeight = 26 * Tile;

	for (int i = 0; i < max; i++) {

		if (!groundSlam[i].base.isAlive && !groundSlam[i].base.isWait) {

			while (true) {

				int tx = rand() % 40;
				int ty = rand() % 26;

				float cx = float(tx * Tile + Tile);
				float cy = float(ty * Tile + Tile);

				if (cx < radius) {
					continue;
				}
				if (cy < radius) {
					continue;
				}
				if (cx > mapWidth - radius) {
					continue;
				}
				if (cy > mapHeight - radius) {
					continue;
				}

				if (Map[ty][tx] == 1) {
					Vector2 v{cx, cy};
					InitgroundSlam(groundSlam[i], v, GROUNDSLAM, boss_phase);
					groundSlam[i].base.isWait = true;
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
				groundSlam[i].currentFrame = 0;
				groundSlam[i].animTimer = 0.0f;
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
// スキルのアニメーション
// 地面攻撃
void UpdateGroundAnim(GroundSlam gs[], int max, float deltaTime) {
	for (int i = 0; i < max; i++) {
		if (!gs[i].base.isAlive)
			continue;

		gs[i].animTimer += deltaTime;
		if (gs[i].animTimer >= gs[i].frameTime) {
			gs[i].animTimer = 0.0f;
			gs[i].currentFrame++;
			if (gs[i].currentFrame >= gs[i].frameCount) {
				gs[i].currentFrame = gs[i].frameCount - 1;
			}
		}
	}
}

void DrawGroundSlamAnim(GroundSlam gs[], int max, float camX, float camY) {

	for (int i = 0; i < max; i++) {
		if (!gs[i].base.isAlive && !gs[i].base.isWait)
			continue;

		int tex = (boss_phase == PHASE_1) ? gs[i].tex1 : gs[i].tex2;

		int frameSize = boss_phase == PHASE_1 ? 144 : 216;
		int u = frameSize * gs[i].currentFrame;
		int v = 0;

		int drawX = (int)(gs[i].base.pos.x - camX);
		int drawY = (int)(gs[i].base.pos.y - camY);

		Novice::DrawQuad(
		    drawX, drawY,                         // 左上 LT
		    drawX + frameSize, drawY,             // 右上 RT
		    drawX, drawY + frameSize,             // 左下 LB
		    drawX + frameSize, drawY + frameSize, // 右下 RB
		    u, v, frameSize, frameSize, tex, WHITE);
	}
}

//=====================================================================================================================================================================================================================
//
// ====================================================================================================================================================================================================================

// もう一回遊び、数値をリセットする
void ALL(
    Player& player, Boss& boss, Camera& camera, Attack player_range[], int rangeMax, Laser& laser, ScatterShot scatter[], int scatterMax, GroundSlam groundSlam[], int groundSlamMax, Reload& reload,
    Particle bloods[], int prtMax, bool& scatterStarted, bool& laserSkillStarted, bool& groundSkillStarted, bool& bossChargeStarted, bool& bossChargeWindup, int& bossChargeWindupTimer,
    int& bossChargeCount, Vector2& bossChargeDir, Vector2& bossChargeAimDir) {

	memcpy(map, initialMap, sizeof(map));

	bossRoomEntered = false;
	introTimer = 120;
	boss_phase = PHASE_1;
	boss_start = IDLE;

	player.base.pos = {20.0f * tile, 40.0f * tile};
	player.base.hp = 5;
	player.base.isInvincible = false;
	player.state = PST_IDLE;
	player.dir = DIR_RIGHT;
	player.currentFrame = 0;
	player.animTimer = 0.0f;
	player.base.isAlive = true;
	player.animEnd = false;

	boss.base.pos = {640, 360};
	boss.base.hp = 1000;
	boss.base.isInvincible = false;
	boss.dir = RIGHT;
	boss.currentFrame = 0;
	boss.animTimer = 0.0f;
	boss.anim = BOSS_ANIM_IDLE;
	boss.base.isInvincible = false;
	boss.base.isAlive = true;
	boss.base.vel = {0.0f, 0.0f};
	boss.base.isDash = false;

	boss.bossDirLocked = false;
	boss.bossLockedDir = {1.0f, 0.0f};
	boss.currentSkill = SHOOT;
	boss.animEnd = false;

	camera.worldOffset.x = player.base.pos.x + player.base.width * 0.5f - kWindowWitch * 0.5f;
	camera.worldOffset.y = player.base.pos.y + player.base.height * 0.5f - kWindowHeight * 0.5f;

	camera.shakeOffset = {0.0f, 0.0f};
	camera.shankeTime = 0;
	camera.shakeProw = 0.0f;

	InitLaser(laser, boss.base, LASER, player.base, boss_phase);

	for (int i = 0; i < scatterMax; i++) {
		scatter[i].base.isAlive = false;
		scatter[i].base.isWait = false;
		scatter[i].base.aliveTime = 0;
	}
	for (int i = 0; i < groundSlamMax; i++) {
		groundSlam[i].base.isAlive = false;
		groundSlam[i].base.isWait = false;
		groundSlam[i].base.aliveTime = 0;
		groundSlam[i].currentFrame = 0;
		groundSlam[i].animTimer = 0;
	}

	for (int i = 0; i < rangeMax; i++) {
		player_range[i].isAlive = false;
		player_range[i].lifeTime = 0;
		player_range[i].pos = player.base.pos;
		player_range[i].vel = {0, 0};
	}

	for (int i = 0; i < prtMax; i++) {
		bloods[i].isAlive = false;
		bloods[i].lifeTime = 0;
	}

	reload.nowBullet = rangeMax;
	reload.reload_time = 0;
	reload.isReload = false;
	reload.bulletMax = 999;

	player.base.isDash = false;
	player.base.dash_time = 0;
	player.base.speed = player.base.normal_speed;

	scatterStarted = false;
	laserSkillStarted = false;
	groundSkillStarted = false;
	bossChargeStarted = false;

	bossChargeWindup = false;
	bossChargeWindupTimer = 0;
	bossChargeCount = 0;

	bossChargeDir = {0.0f, 0.0f};
	bossChargeAimDir = {0.0f, 0.0f};

	boss.skillCooldown = 90;
	loadingTimer = 150;
	introTimer = 90;
}
// ui
void UI_ALL(
    UI& startBtn, UI& manualBtn, UI& backBtn, UI& stopBtn, UI& menuBtn_countinou, UI& menuBtn_return, UI& menuBtn_return_title, UI& scoreBtn_return, UI& scoreBtn_return_title, UI& bossHp,
    Laser& laser) {

	startBtn.isHover = false;
	startBtn.isClicked = false;

	manualBtn.isHover = false;
	manualBtn.isClicked = false;

	backBtn.isHover = false;
	backBtn.isClicked = false;

	stopBtn.isHover = false;
	stopBtn.isClicked = false;

	menuBtn_countinou.isHover = false;
	menuBtn_countinou.isClicked = false;

	menuBtn_return.isHover = false;
	menuBtn_return.isClicked = false;

	menuBtn_return_title.isHover = false;
	menuBtn_return_title.isClicked = false;

	scoreBtn_return.isHover = false;
	scoreBtn_return.isClicked = false;

	scoreBtn_return_title.isHover = false;
	scoreBtn_return_title.isClicked = false;

	bossHp.animTimer = 0;
	bossHp.currentFrame = 0;

	laser.currentFrame = 0;
	laser.animTimer = 0;

	
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	memcpy(initialMap, map, sizeof(map));

	Player player = InitPlayer(43.0f * tile, 21.0f * tile);
	Boss boss = InitBoss(640.0f, 320.0f);

	Mouse mouse{
	    .pos = {0.0f, 0.0f},
	    .dir = {0.0f, 0.0f},
	    .isMouse = false,
	};

	Camera camera{
	    .worldOffset = {player.base.pos.x + player.base.width / 2 - kWindowWitch / 2, player.base.pos.y + player.base.height / 2 - kWindowHeight / 2},
	    .shakeOffset = {0.0f,	                                                     0.0f	                                                      },
	    .shankeTime = 0,
	    .shakeProw = 0.0f,
	};

	// レーザー
	Laser laser = InitLaser(laser, boss.base, LASER, player.base, boss_phase);
	// 地面攻撃
	const int groundSlamMax = 8;
	GroundSlam groundSlam[groundSlamMax];

	// 弾発射
	const int scatterMax = 30;
	ScatterShot scatter[scatterMax]{};
	// ダシュル

	int bossChargeCount = 0;
	Vector2 bossChargeDir{0.0f, 0.0f};
	Vector2 bossChargeAimDir{0.0f, 0.0f};
	bool bossChargeWindup = false;
	int bossChargeWindupTimer = 0;

	//
	bool scatterStarted = false;
	bool laserSkillStarted = false;
	bool groundSkillStarted = false;
	bool bossChargeStarted = false;

	int skillCountMax = 4;

	const int rangeMax = 40;

	Attack player_range[rangeMax]{};
	BossAnim prevAnim = BOSS_ANIM_IDLE;

	// リロード
	Reload reload{
	    .reload_time = 90,
	    .nowBullet = rangeMax,
	    .bulletMax = 999,
	    .isReload = false,
	};

	// タイトル
	UI startBtn{340, 550, 200, 100};
	UI manualBtn{740, 550, 200, 100};
	UI backBtn{20, 20, 120, 50};

	// ゲーム時、止まる
	UI stopBtn{1240, 0, 40, 40};
	// 止まるmenu
	// 続行
	UI menuBtn_countinou{540, 100, 200, 100};
	// やり直し
	UI menuBtn_return{540, 300, 200, 100};
	// タイトル戻る
	UI menuBtn_return_title{540, 500, 200, 100};

	// WIN
	// もう一回遊び
	UI scoreBtn_return{340, 450, 200, 100};
	// タイトルに戻す
	UI scoreBtn_return_title{740, 450, 200, 100};

	UI winBox = {
	    .x = 340,
	    .y = 150,
	    .w = 600,
	    .h = 200,
	    .winEnd = false,
	};
	UI dieBox = {
	    .x = 340,
	    .y = 150,
	    .w = 600,
	    .h = 200,
	    .dieEnd = false,
	};

	UI bossHp = {
	    .x = 100,
	    .y = 650,
	    .w = 1098,
	    .h = 32,
	};

	//////////////////////////////////////////////////

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, kWindowWitch, kWindowHeight);

	// int laserImager = Novice::LoadTexture("./imager/laser.png");
	int tileMap_2 = Novice::LoadTexture("./imager/tileMap_2.png");
	int tileMap_4 = Novice::LoadTexture("./imager/tileMap_4.png");
	int tileMap_5 = Novice::LoadTexture("./imager/tileMap_5.png");
	int tileMap_6 = Novice::LoadTexture("./imager/tileMap_6.png");
	int tileMap_7 = Novice::LoadTexture("./imager/tileMap_7.png");

	int bg1 = Novice::LoadTexture("./imager/bg.png");
	int bg2 = Novice::LoadTexture("./imager/bg2.png");
	int bg3 = Novice::LoadTexture("./imager/bg3.png");

	//
	int title = Novice::LoadTexture("./imager/title.png");

	// 待機
	player.tex[DIR_LEFT][PST_IDLE] = Novice::LoadTexture("./imager/player_idle_left.png");
	player.tex[DIR_RIGHT][PST_IDLE] = Novice::LoadTexture("./imager/player_idle_right.png");
	player.tex[DIR_UP][PST_IDLE] = Novice::LoadTexture("./imager/player_idle_left.png");
	player.tex[DIR_DOWN][PST_IDLE] = Novice::LoadTexture("./imager/player_idle_right.png");

	player.frameCount[DIR_LEFT][PST_IDLE] = 4;
	player.frameCount[DIR_RIGHT][PST_IDLE] = 4;
	player.frameCount[DIR_UP][PST_IDLE] = 4;
	player.frameCount[DIR_DOWN][PST_IDLE] = 4;

	// 足る
	player.tex[DIR_LEFT][PST_RUN] = Novice::LoadTexture("./imager/player_run_left.png");
	player.tex[DIR_RIGHT][PST_RUN] = Novice::LoadTexture("./imager/player_run_right.png");
	player.tex[DIR_UP][PST_RUN] = Novice::LoadTexture("./imager/player_run_up.png");
	player.tex[DIR_DOWN][PST_RUN] = Novice::LoadTexture("./imager/player_run_down.png");

	player.frameCount[DIR_LEFT][PST_RUN] = 6;
	player.frameCount[DIR_RIGHT][PST_RUN] = 6;
	player.frameCount[DIR_UP][PST_RUN] = 6;
	player.frameCount[DIR_DOWN][PST_RUN] = 6;

	// shoot
	player.tex[DIR_LEFT][PST_ATTACK] = Novice::LoadTexture("./imager/player_shoot_left.png");
	player.tex[DIR_RIGHT][PST_ATTACK] = Novice::LoadTexture("./imager/player_shoot_right.png");
	player.tex[DIR_UP][PST_ATTACK] = Novice::LoadTexture("./imager/player_shoot_up.png");
	player.tex[DIR_DOWN][PST_ATTACK] = Novice::LoadTexture("./imager/player_shoot_down.png");

	player.frameCount[DIR_LEFT][PST_ATTACK] = 6;
	player.frameCount[DIR_RIGHT][PST_ATTACK] = 6;
	player.frameCount[DIR_UP][PST_ATTACK] = 6;
	player.frameCount[DIR_DOWN][PST_ATTACK] = 6;

	// dash
	player.tex[DIR_LEFT][PST_DASH] = Novice::LoadTexture("./imager/player_dash_left.png");
	player.tex[DIR_RIGHT][PST_DASH] = Novice::LoadTexture("./imager/player_dash_right.png");
	player.tex[DIR_UP][PST_DASH] = Novice::LoadTexture("./imager/player_dash_up.png");
	player.tex[DIR_DOWN][PST_DASH] = Novice::LoadTexture("./imager/player_dash_down.png");

	player.frameCount[DIR_RIGHT][PST_DASH] = 6;
	player.frameCount[DIR_LEFT][PST_DASH] = 6;
	player.frameCount[DIR_UP][PST_DASH] = 6;
	player.frameCount[DIR_DOWN][PST_DASH] = 6;

	int playerHit = Novice::LoadTexture("./imager/player_hit.png");

	player.tex[DIR_LEFT][PST_DIE] = Novice::LoadTexture("./imager/player_die.png");
	player.tex[DIR_RIGHT][PST_DIE] = Novice::LoadTexture("./imager/player_die.png");
	player.tex[DIR_UP][PST_DIE] = Novice::LoadTexture("./imager/player_die.png");
	player.tex[DIR_DOWN][PST_DIE] = Novice::LoadTexture("./imager/player_die.png");

	player.frameCount[DIR_RIGHT][PST_DIE] = 11;
	player.frameCount[DIR_LEFT][PST_DIE] = 11;
	player.frameCount[DIR_UP][PST_DIE] = 11;
	player.frameCount[DIR_DOWN][PST_DIE] = 11;

	//==================================================================
	// int bossImager = Novice::LoadTexture("./imager/boss.png");
	// idle
	boss.tex[RIGHT][BOSS_ANIM_IDLE] = Novice::LoadTexture("./imager/boss_idle_right.png");
	boss.tex[LEFT][BOSS_ANIM_IDLE] = Novice::LoadTexture("./imager/boss_idle_left.png");
	boss.frameCount[RIGHT][BOSS_ANIM_IDLE] = 8;
	boss.frameCount[LEFT][BOSS_ANIM_IDLE] = 8;

	// 地面攻撃
	boss.tex[RIGHT][BOSS_ANIM_GROUNDSLAM] = Novice::LoadTexture("./imager/boss_ground_right.png");
	boss.tex[LEFT][BOSS_ANIM_GROUNDSLAM] = Novice::LoadTexture("./imager/boss_ground_left.png");
	boss.frameCount[RIGHT][BOSS_ANIM_GROUNDSLAM] = 13;
	boss.frameCount[LEFT][BOSS_ANIM_GROUNDSLAM] = 13;

	// ダシュル
	boss.tex[RIGHT][BOSS_ANIM_CHARGE] = Novice::LoadTexture("./imager/boss_dash_right.png");
	boss.tex[LEFT][BOSS_ANIM_CHARGE] = Novice::LoadTexture("./imager/boss_dash_left.png");
	boss.frameCount[RIGHT][BOSS_ANIM_CHARGE] = 18;
	boss.frameCount[LEFT][BOSS_ANIM_CHARGE] = 18;

	// 弾の発射
	boss.tex[RIGHT][BOSS_ANIM_SHOOT] = Novice::LoadTexture("./imager/boss_idle_right.png");
	boss.tex[LEFT][BOSS_ANIM_SHOOT] = Novice::LoadTexture("./imager/boss_idle_left.png");
	boss.frameCount[RIGHT][BOSS_ANIM_SHOOT] = 8;
	boss.frameCount[LEFT][BOSS_ANIM_SHOOT] = 8;

	// 出现
	boss.tex[RIGHT][BOSS_ANIM_INTRO] = Novice::LoadTexture("./imager/boss_intor.png");
	boss.tex[LEFT][BOSS_ANIM_INTRO] = Novice::LoadTexture("./imager/boss_intor.png");
	boss.frameCount[RIGHT][BOSS_ANIM_INTRO] = 26;
	boss.frameCount[LEFT][BOSS_ANIM_INTRO] = 26;
	// レーザ
	boss.tex[RIGHT][BOSS_ANIM_LASER] = Novice::LoadTexture("./imager/boss_laser_right.png");
	boss.tex[LEFT][BOSS_ANIM_LASER] = Novice::LoadTexture("./imager/boss_laser_left.png");
	boss.frameCount[RIGHT][BOSS_ANIM_LASER] = 28;
	boss.frameCount[LEFT][BOSS_ANIM_LASER] = 28;

	// die
	boss.tex[RIGHT][BOSS_ANIM_DIE] = Novice::LoadTexture("./imager/boss_die.png");
	boss.tex[LEFT][BOSS_ANIM_DIE] = Novice::LoadTexture("./imager/boss_die.png");
	boss.frameCount[RIGHT][BOSS_ANIM_DIE] = 30;
	boss.frameCount[LEFT][BOSS_ANIM_DIE] = 30;

	// 弾
	int bossBullet = Novice::LoadTexture("./imager/bossbullet.png");
	int playerbullet = Novice::LoadTexture("./imager/playerBullet.png");

	// スキル

	for (int i = 0; i < groundSlamMax; i++) {
		InitgroundSlam(groundSlam[i], {0, 0}, GROUNDSLAM, boss_phase);

		groundSlam[i].tex1 = Novice::LoadTexture("./imager/ground.png");

		groundSlam[i].tex2 = Novice::LoadTexture("./imager/ground2.png");

	groundSlam[i].frameCount = 25;
	groundSlam[i].frameTime = 0.05f;
	groundSlam[i].currentFrame = 0;
	groundSlam[i].animTimer = 0.0f;
	}

	laser.tex1 = Novice::LoadTexture("./imager/laser.png");
	laser.tex2 = Novice::LoadTexture("./imager/laser2.png");

	laser.frameCount = 23;
	laser.frameTime = 0.065f;
	laser.currentFrame = 0;
	laser.animTimer = 0.0f;

	// ui
	int explanation = Novice::LoadTexture("./imager/explanation.png");

	startBtn.tex1 = Novice::LoadTexture("./imager/ui_start_on.png");
	startBtn.tex2 = Novice::LoadTexture("./imager/ui_start_off.png");

	manualBtn.tex1 = Novice::LoadTexture("./imager/ui_manual_on.png");
	manualBtn.tex2 = Novice::LoadTexture("./imager/ui_manual_off.png");

	int winImager = Novice::LoadTexture("./imager/win.png");
	int dieImager = Novice::LoadTexture("./imager/die.png");

	int stopBg = Novice::LoadTexture("./imager/stopBg.png");

	int particle = Novice::LoadTexture("./imager/particle.png");

	// boss,HP
	bossHp.tex = Novice::LoadTexture("./imager/boss_hp_gif.png");
	int bossHpImager = Novice::LoadTexture("./imager/boss_hp.png");
	if (laser.base.isAlive) {
	}
	bossHp.animTimer = 0.0f;
	bossHp.currentFrame = 0;
	bossHp.frameTime = 0.1f;
	bossHp.frameCount = 14;
	//player,Hp
	player.hpTex = Novice::LoadTexture("./imager/playerHp.png");


	// ゲーム止まる
	stopBtn.tex1 = Novice::LoadTexture("./imager/ui_stop_on.png");
	stopBtn.tex2 = Novice::LoadTexture("./imager/ui_stop_off.png");

	menuBtn_countinou.tex1 = Novice::LoadTexture("./imager/ui_menu_countinou_on.png");
	menuBtn_countinou.tex2 = Novice::LoadTexture("./imager/ui_menu_countinou_off.png");

	menuBtn_return.tex1 = Novice::LoadTexture("./imager/ui_menu_return_on.png");
	menuBtn_return.tex2 = Novice::LoadTexture("./imager/ui_menu_return_off.png");

	menuBtn_return_title.tex1 = Novice::LoadTexture("./imager/ui_menu_Title_return_on.png");
	menuBtn_return_title.tex2 = Novice::LoadTexture("./imager/ui_menu_Title_return_off.png");

	// クリア
	scoreBtn_return.tex1 = Novice::LoadTexture("./imager/ui_score_return_on.png");
	scoreBtn_return.tex2 = Novice::LoadTexture("./imager/ui_score_return_off.png");

	scoreBtn_return_title.tex1 = Novice::LoadTexture("./imager/ui_menu_Title_return_on.png");
	scoreBtn_return_title.tex2 = Novice::LoadTexture("./imager/ui_menu_Title_return_off.png");

	// 説明
	backBtn.tex1 = Novice::LoadTexture("./imager/ui_back_on.png");
	backBtn.tex2 = Novice::LoadTexture("./imager/ui_back_off.png");

	/// bgm
	int titleBgm = Novice::LoadAudio("./Sounds/title.wav");
	int titleHandle = -1;

	int winBgm = Novice::LoadAudio("./Sounds/win.wav");
	int winHandle = -1;
	bool winPlay = false;

	int dieBgm = Novice::LoadAudio("./Sounds/die.wav");
	int dieHandle = -1;
	bool dieplay = false;

	int fightBgm = Novice::LoadAudio("./Sounds/fight.wav");
	int fightHandle = -1;

	int fight2Bgm = Novice::LoadAudio("./Sounds/fight2.wav");
	int fight2Handle = -1;

	int boss_introBgm = Novice::LoadAudio("./Sounds/boss_intro.wav");
	int boss_introHandle = -1;

	int laserBgm  = Novice::LoadAudio("./Sounds/laser.wav");
	int laserHandle = -1;

	int groundBgm = Novice::LoadAudio("./Sounds/ground.wav");
	int groundHandle = -1;

	int playerHitBgm = Novice::LoadAudio("./Sounds/playerHit.wav");
	int playerHitHandle = -1;


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
			winPlay = false;
			dieplay = false;
			Novice::StopAudio(winHandle);
			Novice::StopAudio(dieHandle);
			if (!Novice::IsPlayingAudio(titleHandle)) {
				titleHandle = Novice::PlayAudio(titleBgm, true, 0.3f);
			}
			UpdateButton(startBtn);
			UpdateButton(manualBtn);

			if (manualBtn.isClicked) {
				game_state = MANUAL;
			}

			if (startBtn.isClicked) {
				game_state = LOADING;
			}
			break;
		}

		case MANUAL: {
			Novice::DrawSprite(0, 0, explanation, 1, 1, 0.0f, WHITE);
			UpdateButton(backBtn);
			DrawButton(backBtn);

			if (backBtn.isClicked) {
				game_state = START;
			}
			break;
		}

		case LOADING: {
			Novice::StopAudio(titleHandle);
			loadingTimer--;
			if (loadingTimer <= 0) {
				camX = player.base.pos.x - kWindowWitch / 2;
				camY = player.base.pos.y - kWindowHeight / 2;

				bossRoomEntered = false;
				loadingTimer = 150;
				ALL(player, boss, camera, player_range, rangeMax, laser, scatter, scatterMax, groundSlam, groundSlamMax, reload, blood, prtmax, scatterStarted, laserSkillStarted, groundSkillStarted,
				    bossChargeStarted, bossChargeWindup, bossChargeWindupTimer, bossChargeCount, bossChargeDir, bossChargeAimDir);
				UI_ALL(startBtn, manualBtn, backBtn, stopBtn, menuBtn_countinou, menuBtn_return, menuBtn_return_title, scoreBtn_return, scoreBtn_return_title, bossHp, laser);

				game_state = FIGHT;
			}

			break;
		}

			// 戦い
		case FIGHT: {
			winPlay = false;
			dieplay = false;
			Novice::StopAudio(winHandle);
			Novice::StopAudio(dieHandle);
			Novice::StopAudio(titleHandle);
			if (!Novice::IsPlayingAudio(fight2Handle)) {
				fight2Handle = Novice::PlayAudio(fight2Bgm, true, 0.3f);
			}

			if (boss.anim != prevAnim) {
				boss.currentFrame = 0;
				boss.animTimer = 0.0f;
				prevAnim = boss.anim;
			}

			float deltaTime = 1.0f / 60.0f;
			UpdatePlayerAnim(player, deltaTime);
			UpdateBossAnim(boss, deltaTime);

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

			if (player.base.isDash) {
				player.state = PST_DASH;
			} else if (Novice::IsPressMouse(0)) {
				player.state = PST_ATTACK;
			} else if (player.base.vel.x != 0 || player.base.vel.y != 0) {
				player.state = PST_RUN;
			} else {
				player.state = PST_IDLE;
			}

			float angle = atan2f(player.base.dir.y, player.base.dir.x);

			if (angle > -M_PI / 4 && angle <= M_PI / 4) {
				player.dir = DIR_RIGHT;
			} else if (angle > M_PI / 4 && angle <= 3 * M_PI / 4) {
				player.dir = DIR_DOWN;
			} else if (angle <= -M_PI / 4 && angle > -3 * M_PI / 4) {
				player.dir = DIR_UP;
			} else {
				player.dir = DIR_LEFT;
			}

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

				float playerCenterY = player.base.pos.y + player.base.height * 0.5f;
				if (!bossRoomEntered) {
					if (playerCenterY + player.base.height > 25 * tile) {
						Novice::StopAudio(fight2Handle);
						if (!Novice::IsPlayingAudio(fightHandle)) {
							fightHandle = Novice::PlayAudio(fightBgm, true, 0.4f);
						}
					}
				}

				if (!bossRoomEntered && playerCenterY + player.base.height < 25 * tile) {
					Novice::StopAudio(fightHandle);

					bossRoomEntered = true;
					game_state = INTRO;
					introTimer = 90;

					boss.skillCooldown = 120;
					boss_start = IDLE;
					ApplyCameraShake(camera, 10, 30);

					for (int x = 0; x < 40; x++) {
						map[25][x] = 2;
					}
				}

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

				bool wantFire = Novice::IsPressMouse(0);

				HitEvent e = Attack_Update(boss.base, player_range, rangeMax);
				if (bossRoomEntered) {
					if (!reload.isReload && reload.nowBullet > 0) {
						Attack_Fire(player.base, player_range, rangeMax, wantFire, player.base.shootCooldown);

						if (wantFire && player.base.shootCooldown == 5) {
							reload.nowBullet--;
						}

						if (e.hit && boss.base.isAlive) {
							ApplyDamage(boss.base, player.base.bulletdamege, false);
							ApplyCameraShake(camera, 5, 10);
							SpawPrt(e.hitPos, PRT_BLODD, blood, prtmax);
						}
					}
				} else {
					Attack_Fire(player.base, player_range, rangeMax, wantFire, player.base.shootCooldown);
				}

				Particle_UpdateAll(blood, prtmax);

				// playerとボースの当たり判定
				if (!player.base.isInvincible) {
					if (isHit(player.base, boss.base)) {
						ApplyDamage(player.base, boss.base.damage, true);
					}
				}
			}

			//==============================================================================================================================================================================================================================
			// bossの攻撃
			//===============================================================================================================================================================================================================================
			if (boss.base.dir.x > 0) {
				boss.dir = RIGHT;
			} else if (boss.base.dir.x < 0) {
				boss.dir = LEFT;
			}

			if (!bossRoomEntered) {
				break;
			}

			{
				if (!boss.bossDirLocked) {
					float PX = player.base.pos.x + player.base.width * 0.5f;
					float PY = player.base.pos.y + player.base.height * 0.5f;
					float BX = boss.base.pos.x + boss.base.width * 0.5f;
					float BY = boss.base.pos.y + boss.base.height * 0.5f;

					boss.base.dir.x = PX - BX;
					boss.base.dir.y = PY - BY;

					float L = sqrtf(boss.base.dir.x * boss.base.dir.x + boss.base.dir.y * boss.base.dir.y);
					if (L > 0) {
						boss.base.dir.x /= L;
						boss.base.dir.y /= L;
					}
				} else {
					boss.base.dir = boss.bossLockedDir;
				}
			}
			if (boss.base.isDash) {
				boss.base.vel = bossChargeDir;
				Move(boss.base.pos, boss.base.vel, boss.base.width, boss.base.height, boss.base.speed);

				Vector2 oldPos = boss.base.pos;
				TileHit(&boss.base.pos, &boss.base.vel, boss.base.width, boss.base.height, map, tile);
				if (oldPos.x != boss.base.pos.x || oldPos.y != boss.base.pos.y) {
					boss.base.isDash = false;
				}
			}

			boss.skillCooldown--;
			if (boss.skillCooldown < 0)
				boss.skillCooldown = 0;

			if (boss.base.hp <= boss.maxHP / 2 && boss_phase == PHASE_1) {
				boss_phase = PHASE_2;
				boss.skillCooldown = 120;
			}

			switch (boss_start) {

			case IDLE: {

				boss.anim = BOSS_ANIM_IDLE;
				boss.bossDirLocked = false;

				if (boss.skillCooldown <= 0) {
					boss_start = SELECT_SKILL;
				}

				// 计算 Boss 到玩家的方向
				float PX = player.base.pos.x + player.base.width * 0.5f;
				float PY = player.base.pos.y + player.base.height * 0.5f;
				float BX = boss.base.pos.x + boss.base.width * 0.5f;
				float BY = boss.base.pos.y + boss.base.height * 0.5f;

				Vector2 moveDir = {PX - BX, PY - BY};
				float Len = sqrtf(moveDir.x * moveDir.x + moveDir.y * moveDir.y);

				if (Len > 0) {
					moveDir.x /= Len;
					moveDir.y /= Len;
				}

				float minDist = 220.0f;
				if (Len > minDist) {
					float walkSpeed = 2.0f;
					boss.base.vel = moveDir;
					Move(boss.base.pos, boss.base.vel, boss.base.width, boss.base.height, walkSpeed);
					TileHit(&boss.base.pos, &boss.base.vel, boss.base.width, boss.base.height, map, tile);
				}

			} break;

			case SELECT_SKILL: // =========================
			{

				BOSS_SKILL lastSkill = boss.currentSkill;

				int r;
				do {
					r = rand() % skillCountMax;
				} while (r == lastSkill);

				boss.currentSkill = (BOSS_SKILL)r;

				scatterStarted = false;
				laserSkillStarted = false;
				groundSkillStarted = false;

				boss_start = CAST_SKILL;
				if (r == 0) {
					boss.currentSkill = SHOOT;
				}
				if (r == 1) {
					boss.currentSkill = LASER;
				}
				if (r == 2) {
					boss.currentSkill = GROUNDSLAM;
				}
				if (r == 3) {
					boss.currentSkill = CHARGE;
				}

			} break;

			case CAST_SKILL: //
				boss.bossLockedDir = boss.base.dir;
				boss.bossDirLocked = true;
				switch (boss.currentSkill) {

				// ---------------------------------------
				case SHOOT: {

					boss.anim = BOSS_ANIM_SHOOT;

					int bulletCount = (boss_phase == PHASE_1) ? 6 : 10;
					int shotTimes = (boss_phase == PHASE_1) ? 1 : 2;

					if (!scatterStarted) {
						SpawnScatterShot(scatter, scatterMax, boss.base, bulletCount, 1.2f);
						scatterStarted = true;
						boss.skillWaitTime = shotTimes == 2 ? 60 : 60;
					}

					// 2连发逻辑
					if (shotTimes == 2 && boss.skillWaitTime > 0) {
						boss.skillWaitTime--;
						if (boss.skillWaitTime == 0) {
							SpawnScatterShot(scatter, scatterMax, boss.base, bulletCount, 1.2f);
						}
					}

					bool allDone = true;
					for (int i = 0; i < scatterMax; i++) {
						if (scatter[i].base.isAlive || scatter[i].base.isWait) {
							allDone = false;
							break;
						}
					}

					if (allDone) {
						scatterStarted = false;
						boss_start = IDLE;
						boss.skillCooldown = 120;
						boss.bossDirLocked = false;
						boss.currentFrame = 0;
					}

				} break;
				// ---------------------------------------
				case LASER:

					boss.anim = BOSS_ANIM_LASER;
					if (!laserSkillStarted) {
						SpawLaser(laser, boss.base, player.base);
						laserSkillStarted = true;
					}

					if (!laser.base.isAlive && !laser.base.isWait && laserSkillStarted) {
						boss_start = IDLE;
						boss.skillCooldown = 120;
						laserSkillStarted = false;
						boss.bossDirLocked = false;
						boss.currentFrame = 0;
					}
					break;

				// ---------------------------------------
				case GROUNDSLAM: {
					boss.anim = BOSS_ANIM_GROUNDSLAM;
					int groundSlamCount = (boss_phase == PHASE_1) ? 4 : 8;
					if (!groundSkillStarted) {
						SpawGroundSlam(groundSlam, groundSlamCount, map, tile);
						groundSkillStarted = true;
						
					}

					bool allDone = true;
					for (int i = 0; i < groundSlamCount; i++) {
						if (groundSlam[i].base.isAlive || groundSlam[i].base.isWait) {
							allDone = false;
							break;
						}
					}

					if (allDone && groundSkillStarted) {
						boss_start = IDLE;
						boss.skillCooldown = 120;
						groundSkillStarted = false;
						boss.bossDirLocked = false;
						boss.currentFrame = 0;
					}
				} break;

				case CHARGE: {
					boss.anim = BOSS_ANIM_CHARGE;
					int maxCount = (boss_phase == PHASE_1) ? 1 : 3;

					if (!bossChargeWindup && !boss.base.isDash && !bossChargeStarted) {

						bossChargeStarted = true;
						bossChargeCount = 0;

						bossChargeWindup = true;
						bossChargeWindupTimer = (boss_phase == PHASE_1) ? 45 : 45;
					}

					if (bossChargeWindup) {

						float pX = player.base.pos.x + player.base.width * 0.5f;
						float pY = player.base.pos.y + player.base.height * 0.5f;
						float bX = boss.base.pos.x + boss.base.width * 0.5f;
						float bY = boss.base.pos.y + boss.base.height * 0.5f;

						bossChargeAimDir.x = pX - bX;
						bossChargeAimDir.y = pY - bY;

						float lenD = sqrtf(bossChargeAimDir.x * bossChargeAimDir.x + bossChargeAimDir.y * bossChargeAimDir.y);
						if (lenD > 0) {
							bossChargeAimDir.x /= lenD;
							bossChargeAimDir.y /= lenD;
						}

						bossChargeWindupTimer--;
						if (bossChargeWindupTimer <= 0) {
							bossChargeDir = bossChargeAimDir;
							if (bossChargeDir.x >= 0) {
								boss.dir = RIGHT;
							} else {
								boss.dir = LEFT;
							}
							boss.bossLockedDir = bossChargeDir;
							boss.bossDirLocked = true;

							boss.base.vel = bossChargeDir;
							boss.base.speed = 30.0f;
							boss.base.isDash = true;
							bossChargeWindup = false;
						}
					}

					if (!boss.base.isDash && !bossChargeWindup && bossChargeStarted) {
						bossChargeCount++;
						if (bossChargeCount < maxCount) {
							bossChargeWindup = true;
							bossChargeWindupTimer = (boss_phase == PHASE_1) ? 45 : 45;
							boss.bossDirLocked = false;
							boss.currentFrame = 0;
						} else {
							boss_start = IDLE;
							boss.skillCooldown = 120;
							bossChargeStarted = false;
							boss.bossDirLocked = false;
							boss.currentFrame = 0;
						}
					}

				} break;
				}
				break;
			}

			// レーザー
			// UpdateSkillWait(laser.base);
			UpdateLaser(laser);
			UpdateLaserAnim(laser, deltaTime);
			// 地面攻撃
			UpdateGroundSlam(groundSlam, groundSlamMax);
			UpdateGroundAnim(groundSlam, groundSlamMax, deltaTime);
			// 弾発射
			UpdateScatterShot(scatter, scatterMax);

		
			

			for (int i = 0; i < scatterMax; i++) {
				if (scatter[i].base.isAlive && isSkillHit(scatter[i].base, player.base)) {
					ApplyDamage(player.base, scatter[i].base.damege, true);
					SpawPrt(player.base.pos, PRT_BLODD, blood, prtmax);
					scatter[i].base.isAlive = false;
				}
			}

			//
			for (int i = 0; i < groundSlamMax; i++) {
				if (groundSlam[i].base.isAlive) {
					if (!Novice::IsPlayingAudio(groundHandle)) {
						groundHandle = Novice::PlayAudio(groundBgm, false, 0.6f);
					}
					if (isSkillHit(groundSlam[i].base, player.base)) {
						ApplyDamage(player.base, groundSlam[i].base.damege, true);
						SpawPrt(player.base.pos, PRT_BLODD, blood, prtmax);
					}
				}

				if (groundSlam[i].base.isAlive && groundSlam[i].base.aliveTime == groundSlam[i].base.aliveTime - 1) {
					ApplyCameraShake(camera, 8, 5);
				}
			}

			if (boss.base.isDash && isHit(boss.base, player.base)) {
				ApplyDamage(player.base, boss.base.damage, true);
				ApplyCameraShake(camera, 3, 6);
			}

			if (laser.base.isAlive) {
				if (!Novice::IsPlayingAudio(laserHandle)) {
					laserHandle = Novice::PlayAudio(laserBgm, false, 0.6f);
				}
				

				float BX = laser.base.pos.x + laser.base.width * 0.0f;
				float BY = laser.base.pos.y + laser.base.height * 0.5f;

				float DX = cosf(laser.angle);
				float DY = sinf(laser.angle);

				float PX = player.base.pos.x + player.base.width * 0.5f;
				float PY = player.base.pos.y + player.base.height * 0.5f;

				float t = ((PX - BX) * DX + (PY - BY) * DY);
				t = fmaxf(0.0f, fminf(laser.base.width, t));

				float CX = BX + DX * t;
				float CY = BY + DY * t;

				float dist = sqrtf((PX - CX) * (PX - CX) + (PY - CY) * (PY - CY));

				float laserThickness = laser.base.height * 0.5f;

				if (dist < laserThickness) {
					ApplyDamage(player.base, laser.base.damege, true);
					ApplyCameraShake(camera, 3, 8);
					SpawPrt(player.base.pos, PRT_BLODD, blood, prtmax);
				}
			} else {
				if (Novice::IsPlayingAudio(laserHandle)) {
					Novice::StopAudio(laserHandle);
				}
			}

			break;
		}
		case INTRO: {

			Novice::StopAudio(fight2Handle);

			if (!Novice::IsPlayingAudio(boss_introHandle)) {
				boss_introHandle = Novice::PlayAudio(boss_introBgm, false, 1.0f);
			}
			float deltaTime = 1.0f / 60.0f;

			boss.anim = BOSS_ANIM_INTRO;
			player.state = PST_IDLE;

			UpdatePlayerAnim(player, deltaTime);
			UpdateBossAnim(boss, deltaTime);
			UpdateBossHpAnim(bossHp, deltaTime);

			player.base.vel = {0, 0};

			float targetX = boss.base.pos.x + boss.base.width * 0.5f - kWindowWitch * 0.5f;
			float targetY = boss.base.pos.y + boss.base.height * 0.5f - kWindowHeight * 0.5f;

			camera.worldOffset.x += (targetX - camera.worldOffset.x) * 0.05f;
			camera.worldOffset.y += (targetY - camera.worldOffset.y) * 0.05f;

			introTimer--;
			if (introTimer <= 0) {
				game_state = FIGHT;
				Novice::StopAudio(boss_introHandle);
			}

			break;
		}
		case HIT: {

			float deltaTime = 1.0f / 60.0f;
			player.state = PST_HIT;
			UpdatePlayerAnim(player, deltaTime);
			if (!Novice::IsPlayingAudio(playerHitHandle)) {
				playerHitHandle = Novice::PlayAudio(playerHitBgm, false, 0.7f);
			}
			hitTimer--;
			if (hitTimer <= 0) {
				game_state = FIGHT;
				hitTimer = 15;
			}
			break;
		}

			// 勝利
		case WIN: {
			Novice::StopAudio(fight2Handle);
			if (!winPlay) {
				winHandle = Novice::PlayAudio(winBgm, false, 0.3f);
				winPlay = true;
			}

			float deltaTime = 1.0f / 60.0f;
			boss.anim = BOSS_ANIM_DIE;
			player.state = PST_IDLE;
			UpdatePlayerAnim(player, deltaTime);
			UpdateBossDieAnim(boss, winBox, deltaTime);
			if (boss.animEnd) {
				UpdateButton(scoreBtn_return);
				UpdateButton(scoreBtn_return_title);
			}

			if (scoreBtn_return.isClicked) {
				ALL(player, boss, camera, player_range, rangeMax, laser, scatter, scatterMax, groundSlam, groundSlamMax, reload, blood, prtmax, scatterStarted, laserSkillStarted, groundSkillStarted,
				    bossChargeStarted, bossChargeWindup, bossChargeWindupTimer, bossChargeCount, bossChargeDir, bossChargeAimDir);
				UI_ALL(startBtn, manualBtn, backBtn, stopBtn, menuBtn_countinou, menuBtn_return, menuBtn_return_title, scoreBtn_return, scoreBtn_return_title, bossHp, laser);

				game_state = FIGHT;
				Novice::StopAudio(winHandle);
			}

			if (scoreBtn_return_title.isClicked) {
				ALL(player, boss, camera, player_range, rangeMax, laser, scatter, scatterMax, groundSlam, groundSlamMax, reload, blood, prtmax, scatterStarted, laserSkillStarted, groundSkillStarted,
				    bossChargeStarted, bossChargeWindup, bossChargeWindupTimer, bossChargeCount, bossChargeDir, bossChargeAimDir);
				UI_ALL(
				    startBtn, manualBtn, backBtn, stopBtn, menuBtn_countinou, menuBtn_return, menuBtn_return_title, scoreBtn_return, scoreBtn_return_title, bossHp, laser);

				game_state = START;
				Novice::StopAudio(winHandle);
			}

			break;
		}

			// 負け
		case LOSE: {
			Novice::StopAudio(fight2Handle);

			if (!dieplay) {
				dieHandle = Novice::PlayAudio(dieBgm, false, 0.2f);
				dieplay = true;
			}

			float deltaTime = 1.0f / 60.0f;
			player.state = PST_DIE;

			UpdatePlayerDieAnim(player, dieBox, deltaTime);

			if (player.animEnd) {
				UpdateButton(scoreBtn_return);
				UpdateButton(scoreBtn_return_title);
			}

			if (scoreBtn_return.isClicked) {
				ALL(player, boss, camera, player_range, rangeMax, laser, scatter, scatterMax, groundSlam, groundSlamMax, reload, blood, prtmax, scatterStarted, laserSkillStarted, groundSkillStarted,
				    bossChargeStarted, bossChargeWindup, bossChargeWindupTimer, bossChargeCount, bossChargeDir, bossChargeAimDir);
				UI_ALL(
				    startBtn, manualBtn, backBtn, stopBtn, menuBtn_countinou, menuBtn_return, menuBtn_return_title, scoreBtn_return, scoreBtn_return_title, bossHp, laser);

				game_state = FIGHT;
			}

			if (scoreBtn_return_title.isClicked) {
				ALL(player, boss, camera, player_range, rangeMax, laser, scatter, scatterMax, groundSlam, groundSlamMax, reload, blood, prtmax, scatterStarted, laserSkillStarted, groundSkillStarted,
				    bossChargeStarted, bossChargeWindup, bossChargeWindupTimer, bossChargeCount, bossChargeDir, bossChargeAimDir);
				UI_ALL(
				    startBtn, manualBtn, backBtn, stopBtn, menuBtn_countinou, menuBtn_return, menuBtn_return_title, scoreBtn_return, scoreBtn_return_title, bossHp, laser);

				game_state = START;
			}
			break;
		}
		case STOP: {
			Novice::StopAudio(titleHandle);
			Novice::StopAudio(fightHandle);

			if (Novice::IsPlayingAudio(fight2Handle)) {
				Novice::SetAudioVolume(fight2Handle, 0.0f);
			}

			UpdateButton(menuBtn_countinou);
			if (menuBtn_countinou.isClicked) {
				game_state = FIGHT;
				Novice::SetAudioVolume(fight2Handle, 0.7f);
			}

			UpdateButton(menuBtn_return);
			if (menuBtn_return.isClicked) {
				ALL(player, boss, camera, player_range, rangeMax, laser, scatter, scatterMax, groundSlam, groundSlamMax, reload, blood, prtmax, scatterStarted, laserSkillStarted, groundSkillStarted,
				    bossChargeStarted, bossChargeWindup, bossChargeWindupTimer, bossChargeCount, bossChargeDir, bossChargeAimDir);
				UI_ALL(
				    startBtn, manualBtn, backBtn, stopBtn, menuBtn_countinou, menuBtn_return, menuBtn_return_title, scoreBtn_return, scoreBtn_return_title, bossHp, laser);
				game_state = FIGHT;
			}
			UpdateButton(menuBtn_return_title);
			if (menuBtn_return_title.isClicked) {
				ALL(player, boss, camera, player_range, rangeMax, laser, scatter, scatterMax, groundSlam, groundSlamMax, reload, blood, prtmax, scatterStarted, laserSkillStarted, groundSkillStarted,
				    bossChargeStarted, bossChargeWindup, bossChargeWindupTimer, bossChargeCount, bossChargeDir, bossChargeAimDir);
				UI_ALL(
				    startBtn, manualBtn, backBtn, stopBtn, menuBtn_countinou, menuBtn_return, menuBtn_return_title, scoreBtn_return, scoreBtn_return_title, bossHp, laser);
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

			Novice::DrawSprite(0, 0, title, 1, 1, 0.0f, WHITE);
			// UpdateButton(startBtn);
			DrawButton(startBtn);
			DrawButton(manualBtn);
			break;
		}
		case LOADING: {
			Novice::DrawBox(0, 0, kWindowWitch, kWindowHeight, 0.0f, BLACK, kFillModeSolid);
			Novice::ScreenPrintf(500, 350, "LOADING");

			int dotCount = (loadingTimer / 20) % 4;
			for (int i = 0; i < dotCount; i++) {
				Novice::ScreenPrintf(600 + i * 15, 350, ".");
			}
			break;
		}
		case FIGHT: {

			// 背景
			Novice::DrawBox(-500, -500, 3000, 3000, 0.0f, BLACK, kFillModeSolid);
			Novice::DrawSprite(0 - (int)camX, 0 - (int)camY, bg1, 1, 1, 0.0f, WHITE);
			Novice::DrawSprite(18 * tile - (int)camX, 25 * tile - (int)camY, bg2, 1, 1, 0.0f, WHITE);
			Novice::DrawSprite(8 * tile - (int)camX, 38 * tile - (int)camY, bg3, 1, 1, 0.0f, WHITE);

			// 弾の画像
			for (int i = 0; i < rangeMax; i++) {
				if (player_range[i].isAlive) {
					Novice::DrawSprite((int)player_range[i].pos.x - (int)camX, (int)player_range[i].pos.y - (int)camY, playerbullet, 1, 1, 0.0f, WHITE);
				}
			}

			// boss　のスキル　描画
			// レーザー
			DrawLaser(laser, camX, camY);
			// 地面攻撃
			DrawGroundSlamAnim(groundSlam, groundSlamMax, camX, camY);
			//  弾発射
			DrawScatterShot(scatter, scatterMax, camX, camY, bossBullet);

			for (int y = 0; y < 45; y++) {
				for (int x = 0; x < 40; x++) {
					if (map[y][x] == 2) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_2, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 3) {
						Novice::DrawBox((int)tile * x - (int)camX, (int)tile * y - (int)camY, tile, tile, 0.0f, BLACK, kFillModeSolid);
					} else if (map[y][x] == 4) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_4, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 5) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_5, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 6) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_6, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 7) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_7, 1, 1, 0.0f, WHITE);
					}
				}
			}

			// player
			if (player.base.isAlive) {

				// playerの画像
				int tex = player.tex[player.dir][player.state];
				int frameW = 64;
				int frameH = 64;

				int u = frameW * player.currentFrame;
				int v = 0;

				int drawX = (int)(player.base.pos.x - camX);
				int drawY = (int)(player.base.pos.y - camY);

				Novice::DrawQuad(
				    drawX - 16, drawY - 15,                   // 左上 LT
				    drawX + frameW - 16, drawY - 15,          // 右上 RT
				    drawX - 16, drawY + frameH - 15,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 15, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			if (boss.base.isAlive && bossRoomEntered) {
				int tex = boss.tex[boss.dir][boss.anim];
				int frameW = 128;
				int frameH = 168;

				int u = frameW * boss.currentFrame;
				int v = 0;

				int drawX = (int)(boss.base.pos.x - camX);
				int drawY = (int)(boss.base.pos.y - camY);
				Novice::DrawQuad(
				    drawX - 16, drawY - 16,                   // 左上 LT
				    drawX + frameW - 16, drawY - 16,          // 右上 RT
				    drawX - 16, drawY + frameH - 16,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 16, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			// 血
			for (int i = 0; i < prtmax; i++) {
				if (blood[i].isAlive) {
					Novice::DrawSprite((int)blood[i].pos.x - (int)camX, (int)blood[i].pos.y - (int)camY, particle, 1, 1, 0.0f, WHITE);
				}
			}

			if (bossRoomEntered && boss.base.isAlive) {
				DrawBossHp(boss, bossHp);
				Novice::DrawSprite(bossHp.x, bossHp.y, bossHpImager, 1, 1, 0.0f, WHITE);
				Novice::DrawSprite(bossHp.x, bossHp.y, bossHpImager, 1, 1, 0.0f, WHITE);
			}

			DrawPlayerHp(player, 0, 0);
			DrawButton(stopBtn);
			break;
		}

		case WIN: {

			Novice::DrawBox(-500, -500, 3000, 3000, 0.0f, BLACK, kFillModeSolid);
			Novice::DrawSprite(0 - (int)camX, 0 - (int)camY, bg1, 1, 1, 0.0f, WHITE);
			Novice::DrawSprite(18 * tile - (int)camX, 25 * tile - (int)camY, bg2, 1, 1, 0.0f, WHITE);
			Novice::DrawSprite(8 * tile - (int)camX, 38 * tile - (int)camY, bg3, 1, 1, 0.0f, WHITE);

			for (int y = 0; y < 45; y++) {
				for (int x = 0; x < 40; x++) {
					if (map[y][x] == 2) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_2, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 3) {
						Novice::DrawBox((int)tile * x - (int)camX, (int)tile * y - (int)camY, tile, tile, 0.0f, BLACK, kFillModeSolid);
					} else if (map[y][x] == 4) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_4, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 5) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_5, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 6) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_6, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 7) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_7, 1, 1, 0.0f, WHITE);
					}
				}
			}

			{
				// playerの画像
				int tex = player.tex[player.dir][player.state];
				int frameW = 64;
				int frameH = 64;
				int u = frameW * player.currentFrame;
				int v = 0;

				int drawX = (int)(player.base.pos.x - camX);
				int drawY = (int)(player.base.pos.y - camY);
				Novice::DrawQuad(
				    drawX - 16, drawY - 15,                   // 左上 LT
				    drawX + frameW - 16, drawY - 15,          // 右上 RT
				    drawX - 16, drawY + frameH - 15,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 15, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			{
				int tex = boss.tex[boss.dir][boss.anim];
				int frameW = 128;
				int frameH = 168;

				int u = frameW * boss.currentFrame;
				int v = 0;

				int drawX = (int)(boss.base.pos.x - camX);
				int drawY = (int)(boss.base.pos.y - camY);

				// Novice::DrawBox(drawX, drawY, (int)boss.base.width, (int)boss.base.height, 0.0f, WHITE, kFillModeSolid);
				Novice::DrawQuad(
				    drawX - 16, drawY - 16,                   // 左上 LT
				    drawX + frameW - 16, drawY - 16,          // 右上 RT
				    drawX - 16, drawY + frameH - 16,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 16, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			if (boss.animEnd) {
				DrawButton(scoreBtn_return);
				DrawButton(scoreBtn_return_title);
				Novice::DrawSprite(winBox.x, winBox.y, winImager, 1, 1, 0.0f, WHITE);
			}

			break;
		}

		case LOSE: {

			Novice::DrawBox(-500, -500, 3000, 3000, 0.0f, BLACK, kFillModeSolid);
			{
				// playerの画像
				int tex = player.tex[player.dir][player.state];
				int frameW = 64;
				int frameH = 64;

				int u = frameW * player.currentFrame;
				int v = 0;

				int drawX = (int)(player.base.pos.x - camX);
				int drawY = (int)(player.base.pos.y - camY);

				Novice::DrawQuad(
				    drawX - 16, drawY - 15,                   // 左上 LT
				    drawX + frameW - 16, drawY - 15,          // 右上 RT
				    drawX - 16, drawY + frameH - 15,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 15, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			if (player.animEnd) {
				DrawButton(scoreBtn_return);
				DrawButton(scoreBtn_return_title);
				Novice::DrawSprite(dieBox.x, dieBox.y, dieImager, 1, 1, 0.0f, WHITE);
			}

			break;
		}
		case STOP: {

			Novice::DrawBox(-500, -500, 3000, 3000, 0.0f, BLACK, kFillModeSolid);
			Novice::DrawSprite(0 - (int)camX, 0 - (int)camY, bg1, 1, 1, 0.0f, WHITE);
			Novice::DrawSprite(18 * tile - (int)camX, 25 * tile - (int)camY, bg2, 1, 1, 0.0f, WHITE);
			Novice::DrawSprite(8 * tile - (int)camX, 38 * tile - (int)camY, bg3, 1, 1, 0.0f, WHITE);

			// 弾の画像
			for (int i = 0; i < rangeMax; i++) {
				if (player_range[i].isAlive) {

					Novice::DrawSprite((int)player_range[i].pos.x - (int)camX, (int)player_range[i].pos.y - (int)camY, playerbullet, 1, 1, 0.0f, WHITE);
				}
			}

			// boss　のスキル　描画
			// レーザー
			DrawLaser(laser, camX, camY);
			// 地面攻撃
			DrawGroundSlamAnim(groundSlam, groundSlamMax, camX, camY);
			//  弾発射
			DrawScatterShot(scatter, scatterMax, camX, camY, bossBullet);

			for (int y = 0; y < 45; y++) {
				for (int x = 0; x < 40; x++) {
					if (map[y][x] == 2) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_2, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 3) {
						Novice::DrawBox((int)tile * x - (int)camX, (int)tile * y - (int)camY, tile, tile, 0.0f, BLACK, kFillModeSolid);
					} else if (map[y][x] == 4) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_4, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 5) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_5, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 6) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_6, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 7) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_7, 1, 1, 0.0f, WHITE);
					}
				}
			}

			{
				// playerの画像
				int tex = player.tex[player.dir][player.state];
				int frameW = 64;
				int frameH = 64;

				int u = frameW * player.currentFrame;
				int v = 0;

				int drawX = (int)(player.base.pos.x - camX);
				int drawY = (int)(player.base.pos.y - camY);

				Novice::DrawQuad(
				    drawX - 16, drawY - 15,                   // 左上 LT
				    drawX + frameW - 16, drawY - 15,          // 右上 RT
				    drawX - 16, drawY + frameH - 15,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 15, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			{
				int tex = boss.tex[boss.dir][boss.anim];
				int frameW = 128;
				int frameH = 168;

				int u = frameW * boss.currentFrame;
				int v = 0;

				int drawX = (int)(boss.base.pos.x - camX);
				int drawY = (int)(boss.base.pos.y - camY);

				// Novice::DrawBox(drawX, drawY, (int)boss.base.width, (int)boss.base.height, 0.0f, WHITE, kFillModeSolid);
				Novice::DrawQuad(
				    drawX - 16, drawY - 16,                   // 左上 LT
				    drawX + frameW - 16, drawY - 16,          // 右上 RT
				    drawX - 16, drawY + frameH - 16,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 16, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			// 血
			for (int i = 0; i < prtmax; i++) {
				if (blood[i].isAlive) {
					Novice::DrawBox((int)blood[i].pos.x - (int)camX, (int)blood[i].pos.y - (int)camY, (int)blood[i].width, (int)blood[i].height, 0.0f, RED, kFillModeSolid);
				}
			}

			Novice::DrawSprite(0, 0, stopBg, 1, 1, 0.0f, WHITE);
			DrawButton(menuBtn_countinou);
			DrawButton(menuBtn_return);
			DrawButton(menuBtn_return_title);
			break;
		}

		case INTRO: {

			Novice::DrawBox(-500, -500, 3000, 3000, 0.0f, BLACK, kFillModeSolid);
			Novice::DrawSprite(0 - (int)camX, 0 - (int)camY, bg1, 1, 1, 0.0f, WHITE);
			Novice::DrawSprite(18 * tile - (int)camX, 25 * tile - (int)camY, bg2, 1, 1, 0.0f, WHITE);
			Novice::DrawSprite(8 * tile - (int)camX, 38 * tile - (int)camY, bg3, 1, 1, 0.0f, WHITE);

			for (int y = 0; y < 45; y++) {
				for (int x = 0; x < 40; x++) {
					if (map[y][x] == 2) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_2, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 3) {
						Novice::DrawBox((int)tile * x - (int)camX, (int)tile * y - (int)camY, tile, tile, 0.0f, BLACK, kFillModeSolid);
					} else if (map[y][x] == 4) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_4, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 5) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_5, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 6) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_6, 1, 1, 0.0f, WHITE);
					} else if (map[y][x] == 7) {
						Novice::DrawSprite((int)tile * x - (int)camX, (int)tile * y - (int)camY, tileMap_7, 1, 1, 0.0f, WHITE);
					}
				}
			}

			{
				// playerの画像
				int tex = player.tex[player.dir][player.state];
				int frameW = 64;
				int frameH = 64;

				int u = frameW * player.currentFrame;
				int v = 0;

				int drawX = (int)(player.base.pos.x - camX);
				int drawY = (int)(player.base.pos.y - camY);

				// Novice::DrawBox(drawX, drawY, (int)player.base.width, (int)player.base.height, 0.0f, WHITE, kFillModeSolid);
				Novice::DrawQuad(
				    drawX - 16, drawY - 15,                   // 左上 LT
				    drawX + frameW - 16, drawY - 15,          // 右上 RT
				    drawX - 16, drawY + frameH - 15,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 15, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			{
				int tex = boss.tex[boss.dir][boss.anim];
				int frameW = 128;
				int frameH = 168;

				int u = frameW * boss.currentFrame;
				int v = 0;

				int drawX = (int)(boss.base.pos.x - camX);
				int drawY = (int)(boss.base.pos.y - camY);

				// Novice::DrawBox(drawX, drawY, (int)boss.base.width, (int)boss.base.height, 0.0f, WHITE, kFillModeSolid);
				Novice::DrawQuad(
				    drawX - 16, drawY - 16,                   // 左上 LT
				    drawX + frameW - 16, drawY - 16,          // 右上 RT
				    drawX - 16, drawY + frameH - 16,          // 左下 LB
				    drawX + frameW - 16, drawY + frameH - 16, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			{
				// boss Hp
				int tex = bossHp.tex;
				int frameW = 1098;
				int frameH = 36;

				int u = 0;
				int v = frameH * bossHp.currentFrame;

				int drawX = bossHp.x;
				int drawY = bossHp.y;

				// Novice::DrawBox(drawX, drawY, (int)player.base.width, (int)player.base.height, 0.0f, WHITE, kFillModeSolid);
				Novice::DrawQuad(
				    drawX, drawY,                   // 左上 LT
				    drawX + frameW, drawY,          // 右上 RT
				    drawX, drawY + frameH,          // 左下 LB
				    drawX + frameW, drawY + frameH, // 右下 RB
				    u, v, frameW, frameH, tex, WHITE);
			}

			break;
		}

		case HIT: {
			int drawX = (int)(player.base.pos.x - camX);
			int drawY = (int)(player.base.pos.y - camY);

			Novice::DrawBox(-500, -500, 3000, 3000, 0.0f, BLACK, kFillModeSolid);
			Novice::DrawSprite(drawX - 16, drawY - 15, playerHit, 1.0f, 1.0f, 0.0f, WHITE);
			break;
		}
		}
		Novice::DrawEllipse((int)mouse.pos.x, (int)mouse.pos.y, 10, 10, 0.0f, GREEN, kFillModeSolid);

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
