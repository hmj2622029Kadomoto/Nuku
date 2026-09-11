#pragma once

// 構造体の宣言
struct OBJECT // 自分や敵用
{
	int x; // x座標
	int y; // y座標
	int vx; // x軸方向の速さ
	int vy; // y軸方向の速さ
	int state; // 存在するか
	int pattern; // 敵の動きの行動パターン
	int image; // 画像
	int wid; // 画像の幅（ピクセル数）
	int hei; // 画像の高さ
	int hp; // HP（耐久力）
	int timer; // タイマー
	int attackTimer; // タイマー
	int idleTimer; // タイマー
};

// 関数プロトタイプ宣言
void InitGame(void);
void InitVariable(void);
void DrawImage(int img, int x, int y);
void MovePlayer(void);
void SetAttack(void);
void MoveAttack(void);
void SetAttack2(void);
void MoveAttack2(void);
int SetEnemy(int x, int y, int vx, int vy, int hp);
void MoveEnemy(void);
void DamageEnemy(int n, int dmg);
void DrawText(int x, int y, const char* txt, int val, int col, int siz);
void DrawTextC(int x, int y, const char* txt, int col, int siz);


int LoadGraphWithCheck(const char* file);
int LoadSoundMemWithCheck(const char* file);