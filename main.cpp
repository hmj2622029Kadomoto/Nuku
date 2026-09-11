#include "DxLib.h"
#include "sub.h"
#include <stdlib.h>

// 定数の定義
const int WIDTH = 1200, HEIGHT = 720; // ウィンドウの幅と高さのピクセル数
const int FPS = 60; // フレームレート
const int IMG_ENEMY_MAX = 5;
const int ATTACK_MAX = 100; // 自分が出す攻撃の最大数
const int ENEMY_MAX = 1000; // 敵の数の最大数
const int STAGE_DISTANCE = FPS * 60; // ステージの長さ（非表示）
const int PLAYER_HP_MAX = 10; // 自分の最大HP
enum { ENE_ZAKO1, ENE_ZAKO2, ENE_ZAKO3, ENE_ZAKO4, ENE_ZAKO5 }; // 敵の種類
enum { TITLE, PLAY, OVER, CLEAR }; // シーンを分けるための列挙定数

// グローバル変数
int imgSakura, imgGround, imgMoon, imgMountain, imgSky, imgGra; // 背景画像
int imgSamurai, imgAttack,imgAttack2; // 自分と攻撃の画像
int imgEnemy[IMG_ENEMY_MAX]; // 敵の画像
int bgm, jinOver, jinClear, seAttack; // 音の読み込み用
int distance = 0; // ステージ終端までの距離
int stage = 1; // ステージ
int score = 0; // スコア
int hisco = 0; // ハイスコア
int losco = 0; // ロースコア
int noDamageFrame; // 無敵状態
int scene = TITLE; // シーンを管理
int timer = 0; // 時間の進行を管理
bool isAttacking = false;
bool isAttacking2 = false;

struct OBJECT imgTitle1, imgTitle2; // タイトルの画像
struct OBJECT player; // 自分用の構造体変数
struct OBJECT attack[ATTACK_MAX]; // 自分用の構造体変数
struct OBJECT attack2[ATTACK_MAX]; // 自分用の構造体変数
struct OBJECT enemy[ENEMY_MAX]; // 自分用の構造体変数

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	SetWindowText("Nuku"); // ウィンドウのタイトル
	SetGraphMode(WIDTH, HEIGHT, 32); // ウィンドウの大きさとカラービット数の指定
	ChangeWindowMode(TRUE); // ウィンドウモードで起動
	if (DxLib_Init() == -1) { return -1; } // ライブラリ初期化　エラーが起きたら終了
	SetBackgroundColor(0, 0, 0); // 背景色の指定
	SetDrawScreen(DX_SCREEN_BACK); // 描画面を裏画面にする

	InitGame(); // 初期化用の関数を呼び出す
	InitVariable();
	distance = STAGE_DISTANCE; // ステージの長さを代入

	while (1) // メインループ
	{
		ClearDrawScreen(); // 画面をクリアする
		
		DrawGraph(0, 0, imgSky, FALSE);
		DrawGraph(0, 0, imgGra, TRUE);
		DrawGraph(0, 0, imgMoon, TRUE);
		DrawGraph(0, 0, imgMountain, TRUE);
		DrawGraph(0, 0, imgGround, TRUE);
		MoveEnemy(); // 敵機の制御
		DrawGraph(0, 0, imgSakura, TRUE);
		DrawFormatString(0, 0, 0xffffff, "distance = %d",distance);

		timer++; // タイマーをカウント
		switch (scene) // シーンごとに処理を分岐
		{
		case TITLE: // タイトル画面
			DrawTextC(WIDTH * 0.5, HEIGHT * 0.3, "Nuku Game", 0xffffff, 80);
			DrawTextC(WIDTH * 0.5, HEIGHT * 0.7, "Press SPACE to start.", 0xffffff, 30);
			if (CheckHitKey(KEY_INPUT_SPACE))
			{
				InitVariable();
				scene = PLAY;
			}
			break;
		case PLAY: // ゲームプレイ画面
			MovePlayer(); // 自分の操作
			MoveAttack(); // 攻撃の制御
			MoveAttack2(); // 攻撃の制御
			if (distance == STAGE_DISTANCE)
			{
				srand(stage); // ステージのパターンを決める
				PlaySoundMem(bgm,DX_PLAYTYPE_LOOP); // BGMループを出力
			}
			if (distance > 0 && player.hp) { distance--; }
			if (300 < distance && distance % 20 == 0) // ザコ1と2の出現
			{
				int x = WIDTH;
				int y = HEIGHT * 3 / 5;
				int e = rand() % 2;
				if (e == ENE_ZAKO1) { SetEnemy(x, y, -3, 0, 1); }
				if (e == ENE_ZAKO2) { SetEnemy(x, -100, 0, 5, 1); }
			}
			if (300 < distance && distance < 900 && distance % 30 == 0) // ザコ3の出現
			{
				int x = 100 + rand() % (WIDTH - 200);
				int y = -50;
				int vy = 40 + rand() % 20;
				SetEnemy(x, -100, 0, vy, 1);
			}
			if (player.hp == 0)
			{
				StopSoundMem(bgm); // BGM停止
				scene = OVER;
				timer = 0;
				break;
			}
			if (distance <= 0)
			{
				scene = CLEAR;
				timer = 0;
				break;
			}
			break;
		case OVER: // ゲームオーバー
			if (timer == FPS * 3)
			{
				PlaySoundMem(jinOver, DX_PLAYTYPE_BACK); // ジングル出力
			}
			else
			{
				DrawTextC(WIDTH * 0.5, HEIGHT * 0.3, "GAME OVER", 0xff0000, 80);
			}
			if (timer > FPS * 10) { scene = TITLE; } // タイトルへ遷移
			break;

		case CLEAR: // ステージクリア
			if (timer == FPS * 3)
			{
				PlaySoundMem(jinClear, DX_PLAYTYPE_BACK); // ジングル出力
			}
			else
			{
				DrawTextC(WIDTH * 0.5, HEIGHT * 0.3, "STAGE CLEAR!", 0x00ffff, 80);
			}
			if (timer > FPS * 10 || CheckHitKey(KEY_INPUT_RETURN)) // タイトルへ遷移
			{
				stage++;
				distance = STAGE_DISTANCE;
				scene = TITLE;
			}
			break;
		}
		

		// スコア、ハイスコア、ステージ数の表示
		DrawText(10, 10, "SCORE %07d", score, 0xffffff, 30);
		DrawText(WIDTH - 220, 10, "HI-SC %4d", hisco, 0xffffff, 30);
		DrawText(WIDTH - 220, 40, "LO-SC %4d", losco, 0xffffff, 30);
		DrawText(WIDTH - 145, HEIGHT - 40, "STAGE %2d", stage, 0xffffff, 30);

		ScreenFlip(); // 裏画面の内容を表画面に反映させる
		WaitTimer(1000 / FPS); // 一定時間待つ
		if (ProcessMessage() == -1) { break; } // Windowsから情報を受け取りエラーが起きたら終了
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) { break; } // ESCキーが押されたら終了
	}
	DxLib_End(); // DXライブラリ使用の終了処理
	return 0; // ソフトの終了
}

// 初期化用の関数
void InitGame(void)
{
	// 背景用の画像の読み込み
	imgSakura = LoadGraphWithCheck("素材フォルダー/Sakura.png");
	imgMountain = LoadGraphWithCheck("素材フォルダー/Mountain.png");
	imgGround = LoadGraphWithCheck("素材フォルダー/Ground.png");
	imgGra = LoadGraphWithCheck("素材フォルダー/グラデーション.png");
	imgSky = LoadGraphWithCheck("素材フォルダー/Sky.png");
	imgMoon = LoadGraphWithCheck("素材フォルダー/Moon.png");
	imgTitle1 = LoadGraphWithCheck("素材フォルダー/TITLE.png");
	imgTitle2 = LoadGraphWithCheck("素材フォルダー/TITLE2.png");
	// 自分とその攻撃の画像の読み込み
	imgSamurai = LoadGraphWithCheck("素材フォルダー/Samurai.png");
	imgAttack = LoadGraphWithCheck("素材フォルダー/Attack.png");
	imgAttack2 = LoadGraphWithCheck("素材フォルダー/Attack2.png");
	// 敵の画像の読み込み
	
	imgEnemy[ENE_ZAKO1] = LoadGraphWithCheck("素材フォルダー/Enemy0.png");
	imgEnemy[ENE_ZAKO2] = LoadGraphWithCheck("素材フォルダー/Enemy1.png");
	imgEnemy[ENE_ZAKO3] = LoadGraphWithCheck("素材フォルダー/Enemy2.png");
	imgEnemy[ENE_ZAKO4] = LoadGraphWithCheck("素材フォルダー/Enemy3.png");
	imgEnemy[ENE_ZAKO5] = LoadGraphWithCheck("素材フォルダー/Enemy4.png");

	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
}

// 背景を描画する


// ゲーム開始時の初期値を代入する関数

void InitVariable(void)
{
	player.x = WIDTH / 10;
	player.y = HEIGHT * 3 / 5;
	player.hp = PLAYER_HP_MAX;
	player.wid = 64 * 2;
	player.hei = 52;
	for (int i = 0; i < ENEMY_MAX; i++) { enemy[i].state = 0; } // 全ての敵を存在しない状態にする
	score = 0;
	noDamageFrame = 0;
	distance = STAGE_DISTANCE;
}

// 中心座標を指定して画像を表示する関数

void DrawImage(int img, int x, int y)
{
	int w, h;
	GetGraphSize(img, &w, &h);
	DrawGraph(x - w / 2, y - h / 2, img, TRUE);
}

// 自機を動かす関数

void MovePlayer(void)
{
	static int count = 0;
	static int oldAKey = 0;
	static int oldSKey = 0;
	int aKey;
	int sKey;
	count++;
	if (noDamageFrame > 0) { noDamageFrame--; } // 無敵時間のカウント
	
	aKey = CheckHitKey(KEY_INPUT_A);
	sKey = CheckHitKey(KEY_INPUT_S);

	if (aKey == TRUE && oldAKey == FALSE)
	{
		isAttacking = TRUE;
		player.attackTimer = 0;
		score -= 50;
		if (score < losco) { losco = score; } // ロースコアの更新

	
		SetAttack();
	}
	oldAKey = aKey;
	if (isAttacking)
	{
		int ix;
		ix = (player.attackTimer+2) * 64;			
		if (noDamageFrame % 4 < 2) { DrawRectExtendGraph(player.x - 64, player.y - 52, player.x + 64, player.y + 52, ix, 0, 64, 52, imgSamurai, TRUE); }
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
		DrawBox(0, 0, WIDTH, HEIGHT, 0x000000, TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		if (count % 10 == 0)
		{
			player.attackTimer++;
			if (player.attackTimer >= 3)
			{
				player.attackTimer = 0;
				isAttacking = FALSE;
			}
		}
		return;
	}
	if (sKey == TRUE && oldSKey == FALSE&&score>=1000)
	{
		isAttacking2 = TRUE;
		player.attackTimer = 0;
		score -= 1000;
	
		SetAttack2();
	}
	oldSKey = sKey;
	if (isAttacking2)
	{
		int ix;
		ix = (player.attackTimer+5) * 64;			
		if (noDamageFrame % 4 < 2) { DrawRectExtendGraph(player.x - 64, player.y - 52, player.x + 64, player.y + 52, ix, 0, 64, 52, imgSamurai, TRUE); }

		if (count % 90 == 0)
		{
			player.attackTimer++;
			if (player.attackTimer >= 2)
			{
				player.attackTimer = 0;
				isAttacking2 = FALSE;
			}
		}
		return;
	}
	if (player.hp > 0 && !isAttacking)
	{
		int ix;
		ix = player.idleTimer*64;
		if (noDamageFrame % 4 < 2) { DrawRectExtendGraph(player.x - 64, player.y - 52, player.x + 64, player.y + 52, ix, 0, 64, 52, imgSamurai, TRUE); } // 自分の描画
		if (count % 30 == 0)
		{
			player.idleTimer++;
			if (player.idleTimer >= 2) { player.idleTimer = 0; }
		}
	}
}

// 攻撃のセット

void SetAttack(void)
{
	int x = player.x;
	int y = player.y;
	for (int i = 0; i < ATTACK_MAX; i++) {
		if (attack[i].state == 0)
		{
			attack[i].x = x;
			attack[i].y = y;
			attack[i].state = 1;
			attack[i].timer = 0;
			attack[i].wid = 128*2;
			attack[i].hei = 76*2;
			break;
		}
	}
	PlaySoundMem(seAttack, DX_PLAYTYPE_BACK); // 効果音
}

void SetAttack2(void)
{
	for (int i = 0; i < ATTACK_MAX; i++) {
		if (attack2[i].state == 0)
		{
			attack2[i].x = WIDTH / 2;
			attack2[i].y = HEIGHT / 2;
			attack2[i].state = 1;
			attack2[i].timer = 0;
			attack2[i].wid = WIDTH;
			attack2[i].hei = HEIGHT;
			break;
		}
	}
	PlaySoundMem(seAttack, DX_PLAYTYPE_BACK); // 効果音
}

// 攻撃の移動

void MoveAttack(void)
{
	static int count = 0;
	count++;
	for (int i = 0; i < ATTACK_MAX; i++)
	{
		int ix = attack[i].timer * 128;
		if (attack[i].state == 0) { continue; } // 空いている配列なら処理しない
		attack[i].x += attack[i].vx; // ┬座標を変化させる
		attack[i].y += attack[i].vy; // ┘
		DrawRectExtendGraph(attack[i].x - 128, attack[i].y - 76, attack[i].x + 128, attack[i].y + 76, ix, 0, 128, 76, imgAttack, TRUE); // 攻撃の描画
		if (count % 5 == 0)
		{
			attack[i].timer++;
			if (attack[i].timer >= 3) { attack[i].state = 0; }
		}
	}
}

void MoveAttack2(void)
{
	static int count = 0;
	count++;
	for (int i = 0; i < ATTACK_MAX; i++)
	{
		int ix = attack2[i].timer * WIDTH;
		if (attack2[i].state == 0) { continue; } // 空いている配列なら処理しない
		attack2[i].x += attack2[i].vx; // ┬座標を変化させる
		attack2[i].y += attack2[i].vy; // ┘
		DrawRectGraph(attack2[i].x - WIDTH/2, attack2[i].y - HEIGHT/2,ix, 0, WIDTH, HEIGHT, imgAttack2, TRUE); // 攻撃の描画
		if (count % 5 == 0)
		{
			attack2[i].timer++;
			if (attack2[i].timer >= 8) { attack2[i].state = 0; }
		}
	}
}

// 敵をセットする

int SetEnemy(int x, int y, int vx, int vy,int hp)
{
	for (int i = 0; i < ENEMY_MAX; i++) {
		if (enemy[i].state == 0) {
			enemy[i].x = x;
			enemy[i].y = y;
			enemy[i].vx = vx;
			enemy[i].vy = vy;
			enemy[i].state = 1;
			return i;
		}
	}
	return -1;
}

// 敵を動かす

void MoveEnemy(void)
{
	static int count = 0;
	count++;
	for (int i = 0; i < ENEMY_MAX; i++) {
		int ix = enemy[i].timer * 63;
		if (enemy[i].state == 0) { continue; } // 空いている配列なら処理しない
		if (enemy[i].pattern == ENE_ZAKO1)
		{
			DrawRectExtendGraph(enemy[i].x - 63, enemy[i].y - 57, enemy[i].x + 63, enemy[i].y + 57, ix, 0, 63, 57, imgEnemy[ENE_ZAKO1], TRUE); // 攻撃の描画
			if (count % 10 == 0)
			{
				enemy[i].timer++;
				if (enemy[i].timer >= 2) { enemy[i].timer = 0; }
			}
		}
		if (enemy[i].pattern == ENE_ZAKO3) // ザコ敵3
		{
			if (enemy[i].vy > 1) // 減速
			{
				enemy[i].vy *= 0.9;
			}
			else if (enemy[i].vy > 0)
			{
				enemy[i].vx = 8;
				enemy[i].vy = 0;
			}
		}
		enemy[i].x += enemy[i].vx; // ┬敵機の移動
		enemy[i].y += enemy[i].vy; // ┘
		DrawImage(enemy[i].image, enemy[i].x, enemy[i].y); // 敵機の描画
		// 画面外に出たか？
		if (enemy[i].x < -200 || WIDTH + 200 < enemy[i].x || enemy[i].y < -200 || HEIGHT + 200 < enemy[i].y) { enemy[i].state = 0; }
		// 当たり判定のアルゴリズム
		for (int j = 0; j < ATTACK_MAX; j++)
		{
			if (attack[j].state == 0) { continue; }
			int dx = abs((int)(enemy[i].x - attack[j].x)); // ┬中心座標間のピクセル数
			int dy = abs((int)(enemy[i].y - attack[j].y)); // ┘
			if (dx < enemy[i].wid/2 + attack[j].wid/2 && dy < enemy[i].hei/2 + attack[j].hei/2)
			{
				DamageEnemy(i,1);
			}
		}
		for (int j = 0; j < ATTACK_MAX; j++)
		{
			if (attack2[j].state == 0) { continue; }
			int dx = abs((int)(enemy[i].x - attack2[j].x)); // ┬中心座標間のピクセル数
			int dy = abs((int)(enemy[i].y - attack2[j].y)); // ┘
			if (dx < enemy[i].wid/2 + attack2[j].wid/2 && dy < enemy[i].hei/2 + attack2[j].hei/2)
			{
				DamageEnemy(i,10);
			}
		}
		if (noDamageFrame == 0) // 無敵状態でないとき、自分とヒットチェック
		{
			int dx = abs((int)(enemy[i].x - player.x)); // ┬中心座標間のピクセル数
			int dy = abs((int)(enemy[i].y - player.y)); // ┘
			if (dx < enemy[i].wid/2 + player.wid/2 && dy < enemy[i].hei/2 + player.hei/2)
			{
				if (player.hp > 0) { player.hp-=10; } // HPを減らす
				noDamageFrame = FPS * 2; // 無敵状態をセット
			}
		}
	}
}

// 敵のシールドを減らす（ダメージを与える）

void DamageEnemy(int n, int dmg)
{
	score += 100; // スコア加算
	if (score > hisco) { hisco = score; } // ハイスコアの更新
	enemy[n].hp -= dmg; // HPを減らす
	if (enemy[n].hp <= 0)
	{
		enemy[n].state = 0; // HP0以下で消す
		if (distance == STAGE_DISTANCE)
		{
			StopSoundMem(bgm); // BGM停止
			scene = CLEAR;
			timer = 0;
		}
	}
}

// 影を付けた文字列と値を表示する変数
void DrawText(int x, int y, const char* txt, int val, int col, int siz)
{
	SetFontSize(siz); // フォントの大きさを指定
	DrawFormatString(x + 1, y + 1, 0x000000, txt, val); // 黒で文字列を表示
	DrawFormatString(x, y, col, txt, val); // 引数の色で文字列を表示
}

// 文字列をセンタリングして表示する関数
void DrawTextC(int x, int y, const char* txt, int col, int siz)
{
	SetFontSize(siz);
	int strWidth = GetDrawStringWidth(txt, strlen(txt));
	x -= strWidth / 2;
	y -= siz / 2;
	DrawString(x + 1, y + 1,txt, 0x000000);
	DrawString(x, y, txt, col);
}


// 画像の読み込み、読み込み失敗時は通知
int LoadGraphWithCheck(const char* file)
{
	int res = LoadGraph(file);
	if (res == -1) { MessageBox(GetMainWindowHandle(), file, "画像の読み込みに失敗", MB_OK | MB_ICONSTOP); }
	return res;
}

int LoadSoundMemWithCheck(const char* file)
{
	int res = LoadSoundMem(file);
	if (res == -1) { MessageBox(GetMainWindowHandle(), file, "音の読み込みに失敗", MB_OK | MB_ICONSTOP); }
	return res;
}