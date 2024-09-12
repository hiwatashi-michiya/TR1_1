#pragma once
#include <list>
#include <array>
#include <string>
#include <vector>

//横最大幅
const int kMaxWidth = 120;
//縦最大幅
const int kMaxHeight = 30;

//マップチップのサイズ
const int kMapChipSize = 32;

//リストの最大サイズ(Undo, Redo)
const int kMaxListSize = 900;

//範囲塗りつぶしリストの最大サイズ
const int64_t kMaxFillListSIze = kMaxWidth * kMaxHeight * kMaxListSize;

enum BlockType {
	
	kNone, //空白
	kCloud, //雲
	kPlayerPoint, //プレイヤースタート地点
	kKey, //鍵
	kGoal, //ゴール
	kCollection, //収集アイテム
	kEye, //目
	kMaxBlock,//ブロックの最大数

};

enum TOOL
{
	BRUSH,
	RANGEFILL,
	SELECT,
	MAXTOOL,
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

private:

	//書き換え機能
	void Edit();

	//指定した文字列が空かどうか判定
	bool CheckIsEmpty(const std::string& name);

	//配列の数字によってブロックの持つ情報を変える
	void SetState(int mapNum);

	//Undo
	void Undo();

	//Redo
	void Redo();

	//範囲塗りつぶし
	void RangeFill();

	//範囲選択
	void Select();

	//csvロード
	void Load();
	//csvセーブ
	void Save();

	//jsonロード
	void LoadJson();

	//jsonセーブ
	void SaveJson();

	//ファイル作成関数
	void Create();

	//ファイルを閉じる
	void Close();

	//全マップ検索
	void LoadAllMaps();

	//テキスト説明
	std::array<std::string, kMaxBlock> blockNames_ = { "None", "Cloud", "PlayerPoint", "Key", "Goal", "Collection", "Eye" };

	std::array<int32_t, kMaxBlock> blockCounts_{};

	//ImGuiをタッチしたか
	bool isTouchGui_ = false;

	//書き換える配列の要素を格納するリスト。mapの要素、要素の行数、要素の列数の順に格納する
	std::list<int> undoArrayList_;

	//手戻りしたときの要素保管場所
	std::list<int> redoArrayList_;

	//範囲選択時の同時塗りつぶし(undo)
	std::list<int> undoFillArrayList_;

	//範囲選択時の同時塗りつぶし(redo)
	std::list<int> redoFillArrayList_;

	//コピーリスト
	std::list<int> copyArrayList_;

	//リストの要素を参照する変数
	int tmpArrayX_;
	int tmpArrayY_;
	int tmpArrayType_;

	//マップチップの数
	int map_[kMaxHeight][kMaxWidth];
	//書き換え可能かどうか
	bool isWrite_[kMaxHeight][kMaxWidth];

	//マウスX座標
	int mouseX_;
	//マップチップ上のマウスの位置X
	int mouseXGrid_;
	//マウスY座標
	int mouseY_;
	//マップチップ上のマウスの位置Y
	int mouseYGrid_;

	//設置するブロックのナンバー
	int blockNum_;
	int preBlockNum_;

	//マップ書き換え可能かどうか
	bool isEdit_;

	//範囲塗りつぶしを行うかどうか
	bool isRangeFill_;

	//範囲塗りつぶし始点座標X
	int startRangeFillX_;
	//範囲塗りつぶし終点座標X
	int endRangeFillX_;
	//範囲塗りつぶし始点座標Y
	int startRangeFillY_;
	//範囲塗りつぶし終点座標Y
	int endRangeFillY_;

	//範囲選択時の開始座標表示用変数
	int drawX_;
	int drawY_;

	//範囲選択時のスクロール量調整
	int32_t drawWidthScrollX_;
	int32_t drawHeightScrollY_;

	//画面外の表示を制限
	int borderRight_;
	int borderLeft_;
	int borderTop_;
	int borderDown_;

	//スクロール
	int32_t scrollX_ = 0;
	int32_t scrollY_ = 0;

	int32_t kScrollLimitX_ = kMaxWidth * kMapChipSize - 1280;
	int32_t kScrollLimitY_ = kMaxHeight * kMapChipSize - 720;

	//スクロールの移動値
	uint32_t scrollValue_ = 20;

	//画像
	int textureHandle_;
	int frameTexture_;

	//色
	int color_;

	//キー入力を管理するためのカウント
	int keyCount_;
	const int kMaxKeyCount_ = 15;

	//値を範囲内に収める関数
	int Clamp(int x, int min, int max);

	//ImGuiの位置、サイズを格納する変数
	float ImGuiPosX_;
	float ImGuiPosY_;
	float ImGuiWidth_;
	float ImGuiHeight_;

	//現在選択しているマップの場所を表示するための座標
	int selectX_;
	int selectY_;

	//その他画像
	int bgTexture_;
	int block1Tex_;
	int block2Tex_;
	int block3Tex_;
	int block4Tex_;
	int block5Tex_;
	int block6Tex_;
	int block7Tex_;
	int block8Tex_;
	int block9Tex_;
	int block10Tex_;

	std::vector<std::string> textureFileNames_;
	std::vector<std::string> mapNames_;

	//現在の機能
	TOOL tool_ = BRUSH;
	std::array<std::string, 2> tools_ = { "Brush","RangeFill" };
	/*char toolString_[3][10] = { "Brush","RangeFill", "Select" };*/

	//範囲選択しているかどうか
	bool isSelect_ = false;

	//セーブしたかどうか
	bool isSave_ = true;

	//ファイルを開いているかどうか
	bool isOpenFile_ = false;

	//ファイル名
	char fileName_[256];

};


