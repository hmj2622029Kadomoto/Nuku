#include "DxLib.h"
#include "sub.h"
#include <stdlib.h>

// 定数の定義
const int WIDTH = 1200, HEIGHT = 720; // ウィンドウの幅と高さのピクセル数
const int FPS = 60; // フレームレート
const int ATTACK_MAX = 100; // 自分が出す攻撃の最大数
const int ENEMY_MAX = 1000; // 敵の数の最大数
const int STAGE_DISTANCE = FPS * 60; // ステージの長さ（非表示）
const int PLAYER_HP_MAX = 10; // 自分の最大HP
enum { ENE_ZAKO1,ENE_ZAKO2,ENE_ZAKO3,ENE_ZAKO4,ENE_ZAKO5};