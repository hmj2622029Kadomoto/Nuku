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
int imgSamurai, imgAttack; // 自分と攻撃の画像
int imgEnemy[IMG_ENEMY_MAX]; // 敵の画像
int bgm, jinOver, jinClear, seAttack; // 音の読み込み用
int distance = 0; // ステージ終端までの距離
int stage = 1; // ステージ
int score = 0; // スコア
int hisco = 0; // ハイスコア
int noDamageFrame; // 無敵状態
int scene = TITLE; // シーンを管理
int timer = 0; // 時間の進行を管理

struct OBJECT player; // 自分用の構造体変数
struct OBJECT attack[ATTACK_MAX]; // 自分用の構造体変数
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
		
		MoveEnemy(); // 敵機の制御
		MoveAttack(); // 攻撃の制御
		StageMap(); // ステージマップ
		DrawParameter(); // パラメーターを表示

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
			if (distance == STAGE_DISTANCE)
			{
				srand(stage); // ステージのパターンを決める
				PlaySoundMem(bgm,DX_PLAYTYPE_LOOP); // BGMループを出力
			}
			if (distance > 0) { distance; }
			if (300 < distance && distance % 20 == 0) // ザコ1と2の出現
			{
				int x = 100 + rand() % (WIDTH - 200);
				int y = -50;
				int e = rand() % 2;
				if (e == ENE_ZAKO1) { SetEnemy(x, y, 0, 3, ENE_ZAKO1, imgEnemy[ENE_ZAKO1], 1); }
				if (e == ENE_ZAKO2) {
					int vx = 0;
					if (player.x < x - 50) { vx = -3; }
					if (player.x > x + 50) { vx = 3; }
					SetEnemy(x, -100, vx, 5, ENE_ZAKO2, imgEnemy[ENE_ZAKO2], 1); 
				}
			}
			if (300 < distance && distance < 900 && distance % 30 == 0) // ザコ3の出現
			{
				int x = 100 + rand() % (WIDTH - 200);
				int y = -50;
				int vy = 40 + rand() % 20;
				SetEnemy(x, -100, 0, vy, ENE_ZAKO3, imgEnemy[ENE_ZAKO3], 1);
			}
			if (player.hp == 0)
			{
				StopSoundMem(bgm); // BGM停止
				scene = OVER;
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
			MovePlayer(); // 自分の処理
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
		DrawText(WIDTH - 220, 10, "HI-SC %07d", hisco, 0xffffff, 30);
		DrawText(WIDTH - 145, HEIGHT - 40, "STAGE %02d", stage, 0xffffff, 30);

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
	// 自分とその攻撃の画像の読み込み
	imgSamurai = LoadGraphWithCheck("素材フォルダー/.png");
	imgAttack = LoadGraphWithCheck("素材フォルダー/.png");
	// 敵の画像の読み込み
	for (int i = 0; i < IMG_ENEMY_MAX; i++)
	{
		char file[] = "image/enemy*.png";
		file[11] = (char)('0' + i);
		imgEnemy[i] = LoadGraphWithCheck(file);
	}
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
	bgm = LoadSoundMemWithCheck("sound/.mp3");
}

// ゲーム開始時の初期値を代入する関数
void InitVariable(void)
{
	player.x = WIDTH / 2;
	player.y = HEIGHT / 2;
	player.vx = 5;
	player.vy = 5;
	player.hp = PLAYER_HP_MAX;
	GetGraphSize(imgSamurai, &player.wid, &player.hei); // 自機の画像の幅と高さを代入
	for (int i = 0; i < ENEMY_MAX; i++) { enemy[i].state = 0; } // 全ての敵を存在しない状態にする
	score = 0;
	stage = 1;
	noDamageFrame = 0;
	distance = STAGE_DISTANCE;
}

// 中心座標を指定して画像を表示する関数
void drawImage(int img, int x, int y)
{
	int w, h;
	GetGraphSize(img, &w, &h);
	DrawGraph(x - w / 2, y - h / 2, img, TRUE);
}

// 自機を動かす関数
void movePlayer(void)
{
	if (CheckHitKey(KEY_INPUT_SPACE))
	{

	}
	if (noDamageFrame > 0) { noDamageFrame--; } // 無敵時間のカウント
	if (noDamageFrame % 4 < 2) { drawImage(imgSamurai, player.x, player.y); } // 自機の描画
}

// 攻撃のセット
void SetAttack(void)
{
	int x = player.x;
	int y = player.y-20;
	for (int i = 0; i < ATTACK_MAX; i++) {
		if (attack[i].state == 0)
		{
			attack[i].x = x;
			attack[i].y = y;
			attack[i].vx = 0;
			attack[i].vy = 0;
			attack[i].state = 1;
			break;
		}
	}
	PlaySoundMem(seAttack, DX_PLAYTYPE_BACK); // 効果音
}

// 攻撃の移動
void MoveAttack(void)
{
	for (int i = 0; i < ATTACK_MAX; i++)
	{
		if (attack[i].state == 0) { continue; } // 空いている配列なら処理しない
		attack[i].x += attack[i].vx; // ┬座標を変化させる
		attack[i].y += attack[i].vy; // ┘
		DrawImage(imgAttack, attack[i].x, attack[i].y); // 攻撃の描画
		if (attack[i].y < -100) { attack[i].state = 0; } // 画面外に出たら、存在しない状態にする
	}
}

// 敵をセットする
int SetEnemy(int x, int y, int vx, int vy, int ptn, int img, int hp)
{
	for (int i = 0; i < ENEMY_MAX; i++) {
		if (enemy[i].state == 0) {
			enemy[i].x = x;
			enemy[i].y = y;
			enemy[i].vx = vx;
			enemy[i].vy = vy;
			enemy[i].state = 1;
			enemy[i].pattern = ptn;
			enemy[i].image = img;
			GetGraphSize(img, &enemy[i].wid, &enemy[i].hei); // 画像の幅と高さを代入
			return i;
		}
	}
	return -1;
}

// 敵を動かす
void moveEnemy(void)
{
	for (int i = 0; i < ENEMY_MAX; i++) {
		if (enemy[i].state == 0) { continue; } // 空いている配列なら処理しない
		if (enemy[i].pattern == ENE_ZAKO3) // ザコ機3
		{
			if (enemy[i].vy > 1) // 減速
			{
				enemy[i].vy *= 0.9;
			}
			else if (enemy[i].vy > 0)
			{
				enemy[i].vx = 8;
				enemy[i].vy = -4;
			}
		}
	}
}