#pragma once

//横最大幅
const int kMaxWidth = 50;
//縦最大幅
const int kMaxHeight = 25;

//ブロックの種類
const int kMaxBlockType = 5;

//マップの操作を行うクラス
class Map
{
public:
	Map();
	~Map();

	void Update();
	void Draw();

private:

	//csv書き換えに使う文字列
	char string[kMaxBlockType][2] = { "0", "1", "2", "3", "4" };

	//マップチップの数
	int map[kMaxHeight][kMaxWidth];

	//マウスX座標
	int mouseX;
	//マウスY座標
	int mouseY;

	//設置するブロックのナンバー
	int blockNum;

};


