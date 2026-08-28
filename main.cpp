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
			}
		}
	}
}