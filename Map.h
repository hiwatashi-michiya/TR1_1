#pragma once
#include <list>

//横最大幅
const int kMaxWidth = 50;
//縦最大幅
const int kMaxHeight = 25;

//ブロックの種類
const int kMaxBlockType = 5;

//マップチップのサイズ
const int kMapChipSize = 32;

enum BlockType {
	NONE,
	BLOCK,
	FIRE,
	WATER,
	WOOD
};

//マップの操作を行うクラス
class Map
{
public:
	Map();
	~Map();

	//更新
	void Update();
	//描画
	void Draw();
	//csvロード
	void Load();
	//csvセーブ
	void Save();

private:

	//書き換え機能
	void Edit();

	//配列の数字によってブロックの持つ情報を変える
	void SetState(int mapNum);

	//Undo
	void Undo();

	//Redo
	void Redo();

	//csv書き換えに使う文字列
	char string[kMaxBlockType][2] = { "0", "1", "2", "3", "4" };

	//書き換える配列の要素を格納するリスト。mapの要素、要素の行数、要素の列数の順に格納する
	std::list<int> undoArrayList;

	//手戻りしたときの要素保管場所
	std::list<int> redoArrayList;

	//リストの要素を参照する変数
	int tmpArrayX;
	int tmpArrayY;
	int tmpArrayType;

	//マップチップの数
	int map[kMaxHeight][kMaxWidth];

	//マウスX座標
	int mouseX;
	//マップチップ上のマウスの位置X
	int mouseXGrid;
	//マウスY座標
	int mouseY;
	//マップチップ上のマウスの位置Y
	int mouseYGrid;

	//設置するブロックのナンバー
	int blockNum;

	//マップ書き換え可能かどうか
	bool isEdit;

	//画面外の表示を制限
	int borderRight;
	int borderLeft;
	int borderTop;
	int borderDown;

	//画像
	int textureHandle;
	int frameTexture;

	//色
	int color;

};


