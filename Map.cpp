#include "Map.h"
#include <Novice.h>
#include <stdio.h>
#include <cassert>
#include "Key.h"
#include <imgui.h>

Map::Map()
{

	Load();

	mouseX = 0;
	mouseY = 0;
	mouseXGrid = 0;
	mouseYGrid = 0;

	tmpArrayX = 0;
	tmpArrayY = 0;
	tmpArrayType = 0;

	blockNum = 0;

	isEdit = false;

	borderRight = 1280 + kMapChipSize;
	borderLeft = -kMapChipSize;
	borderTop = -kMapChipSize;
	borderDown = 720 + kMapChipSize;

	textureHandle = Novice::LoadTexture("./Resources/Texture/map.png");
	frameTexture = Novice::LoadTexture("./Resources/Texture/frameborder.png");

	color = 0xFFFFFFFF;

}

Map::~Map()
{
}

void Map::Update() {

	Novice::GetMousePosition(&mouseX, &mouseY);

	mouseXGrid = (mouseX) / kMapChipSize;
	mouseYGrid = (mouseY) / kMapChipSize;

	Edit();

}

void Map::Draw() {

	for (int y = 0; y < kMaxHeight; y++) {
		for (int x = 0; x < kMaxWidth; x++) {

			SetState(map[y][x]);

			//画面範囲内なら表示
			if (x * kMapChipSize < borderRight &&
				x * kMapChipSize > borderLeft &&
				y * kMapChipSize > borderTop &&
				y * kMapChipSize < borderDown) {
				
				Novice::DrawQuad(x * kMapChipSize, y * kMapChipSize, 
					x * kMapChipSize + kMapChipSize, y * kMapChipSize,
					x * kMapChipSize, y * kMapChipSize + kMapChipSize,
					x * kMapChipSize + kMapChipSize, y * kMapChipSize + kMapChipSize,
					0, 0, 32, 32, textureHandle, color);

			}
		}
	}

	if (isEdit) {

		Novice::DrawQuad(0, 0, 1280, 0, 0, 800, 1280, 800, 0, 0, 1280, 800, frameTexture, 0xFFFFFF66);

		Novice::ScreenPrintf(0, 0, "now size %d", undoArrayList.size());

	}

}

void Map::Edit() {

	//書き換えオンオフ設定
	if (Key::IsTrigger(DIK_E)) {

		if (isEdit) {
			isEdit = false;
		}
		else {
			isEdit = true;
		}

	}

	if (isEdit == true) {

		//ブロック切り替え
		if (Key::IsTrigger(DIK_W)) {

			if (blockNum < kMaxBlockType - 1) {
				blockNum++;
			}

		}

		if (Key::IsTrigger(DIK_S)) {

			if (blockNum > 0) {
				blockNum--;
			}

		}

		if (Novice::IsPressMouse(0)) {

			//マウスを押している間書き換える要素をリストに追加
			Novice::GetMousePosition(&mouseX, &mouseY);

			mouseXGrid = (mouseX) / kMapChipSize;
			mouseYGrid = (mouseY) / kMapChipSize;

			//画面外だったら書き換えない
			if (mouseX < 1280 && mouseX > 0 &&
				mouseY < 720 && mouseY > 0) {

				//書き換える前と後の要素が同じ場合スルー
				if (map[mouseYGrid][mouseXGrid] != blockNum) {

					//リストの最後尾に変更前の要素を追加
					//マップのナンバーを最初に格納
					undoArrayList.push_back(map[mouseYGrid][mouseXGrid]);
					//行の数字を追加
					undoArrayList.push_back(mouseYGrid);
					//列の数字を追加
					undoArrayList.push_back(mouseXGrid);

					//サイズが一定値を超えたら古い順に削除
					if (undoArrayList.size() > 900) {
						for (int i = 0; i < 3; i++) {
							undoArrayList.pop_front();
						}
					}

					//配列の要素を変更
					map[mouseYGrid][mouseXGrid] = blockNum;

				}

			}

		}
		
		if (Key::IsPress(DIK_LCONTROL)) {

			//ctrl + Zで手戻り、ctrl + Yで元に戻す、ctrl + Sでセーブ
			if (Key::IsTrigger(DIK_Z)) {

				Undo();

			}
			else if (Key::IsTrigger(DIK_Y)) {

				Redo();

			}
			else if (Key::IsTrigger(DIK_S)) {

				Save();

			}

		}

	}

}

void Map::SetState(int mapNum) {

	switch (mapNum)
	{
	case NONE:
	default:
		color = 0x00000000;
		break;
	case BLOCK:
		color = 0xFFFFFFFF;
		break;
	case FIRE:
		color = 0xFF0000FF;
		break;
	case WATER:
		color = 0x0000FFFF;
		break;
	case WOOD:
		color = 0x00FF00FF;
		break;
	}

}

void Map::Load() {

	FILE* fp = NULL;

	fopen_s(&fp, "./Resources/Maps/map.csv", "rt");

	assert(fp != NULL);

	for (int y = 0; y < kMaxHeight; y++) {
		for (int x = 0; x < kMaxWidth; x++) {
			fscanf_s(fp, "%d,", &map[y][x]);
		}
	}

	fclose(fp);

}

//セーブ機能
void Map::Save() {

	FILE* fp = NULL;

	fopen_s(&fp, "./Resources/Maps/map.csv", "r+b");

	assert(fp != NULL);

	for (int y = 0; y < kMaxHeight; y++) {
		for (int x = 0; x < kMaxWidth; x++) {
			//ファイルの書き込む場所を探す
			fseek(fp, (y * (kMaxWidth * 2 + 2)) + (x * 2), SEEK_SET);
			//指定した場所の値を書き換える
			fputs(string[map[y][x]], fp);
		}
	}

	fclose(fp);

}

//手戻り
void Map::Undo() {

	//リストが空でないときに処理
	if (undoArrayList.empty() != true) {

		//要素を一つずつ取り出して削除、現在のマップを保管用リストに追加
		tmpArrayX = undoArrayList.back();
		redoArrayList.push_back(undoArrayList.back());
		undoArrayList.pop_back();

		tmpArrayY = undoArrayList.back();
		redoArrayList.push_back(undoArrayList.back());
		undoArrayList.pop_back();

		//直前に変更した場所の要素を保管し、前の要素に入れ替える
		tmpArrayType = undoArrayList.back();
		redoArrayList.push_back(map[tmpArrayY][tmpArrayX]);
		undoArrayList.pop_back();

		//取り出した要素を使い書き換える前のマップに戻す
		map[tmpArrayY][tmpArrayX] = tmpArrayType;

	}

}

//元に戻す
void Map::Redo() {

	//リストが空でないときに処理
	if (redoArrayList.empty() != true) {

		//保管用のリストから要素を取り出して元のリストに戻し削除
		tmpArrayType = redoArrayList.back();
		undoArrayList.push_back(redoArrayList.back());
		redoArrayList.pop_back();

		tmpArrayY = redoArrayList.back();
		undoArrayList.push_back(redoArrayList.back());
		redoArrayList.pop_back();

		tmpArrayX = redoArrayList.back();
		undoArrayList.push_back(map[tmpArrayY][tmpArrayX]);
		redoArrayList.pop_back();

		//取り出した要素を使い元のマップに戻す
		map[tmpArrayY][tmpArrayX] = tmpArrayType;

	}

}
