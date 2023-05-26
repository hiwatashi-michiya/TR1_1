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
	isRangeFill = false;

	startRangeFillX = 0;
	endRangeFillX = 0;
	startRangeFillY = 0;
	endRangeFillY = 0;

	drawX = 0;
	drawY = 0;

	keyCount = kMaxKeyCount;

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

		Novice::ScreenPrintf(0, 0, "undoArrayList size : %d", undoArrayList.size());
		Novice::ScreenPrintf(0, 20, "redoArrayList size : %d", redoArrayList.size());
		Novice::ScreenPrintf(0, 40, "blockNumber : %d", blockNum);

		if (Novice::IsPressMouse(1)) {
			Novice::DrawBox(drawX, drawY, mouseX - drawX, mouseY - drawY, 0.0f, 0x000000FF, kFillModeWireFrame);
		}

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

		//単選択
		if (Novice::IsPressMouse(0) && !Novice::IsPressMouse(1)) {

			//マウスを押している間書き換える要素をリストに追加
			Novice::GetMousePosition(&mouseX, &mouseY);

			mouseXGrid = (mouseX) / kMapChipSize;
			mouseYGrid = (mouseY) / kMapChipSize;

			//配列外参照を起こさない
			mouseXGrid = Clamp(mouseXGrid, 0, kMaxWidth);
			mouseYGrid = Clamp(mouseYGrid, 0, kMaxHeight);

			//画面外だったら書き換えない
			if (mouseX < 1280 && mouseX > 0 &&
				mouseY < 720 && mouseY > 0) {

				//書き換える前と後の要素が同じ場合スルー
				if (map[mouseYGrid][mouseXGrid] != blockNum) {

					//redoのリストに要素があった場合、空にする
					if (redoArrayList.empty() != true) {
						redoArrayList.clear();
					}

					//redoのリスト(範囲塗りつぶし)に要素があった場合、空にする
					if (redoFillArrayList.empty() != true) {
						redoFillArrayList.clear();
					}

					//リストの最後尾に変更前の要素を追加
					//マップのナンバーを最初に格納
					undoArrayList.push_back(map[mouseYGrid][mouseXGrid]);
					//行の数字を追加
					undoArrayList.push_back(mouseYGrid);
					//列の数字を追加
					undoArrayList.push_back(mouseXGrid);

					//サイズが一定値を超えたら古い順に削除
					if (undoArrayList.size() > kMaxListSize) {
						for (int i = 0; i < 3; i++) {
							undoArrayList.pop_front();
						}
					}

					//配列の要素を変更
					map[mouseYGrid][mouseXGrid] = blockNum;

				}

			}

		}
		
		//範囲塗りつぶし、範囲選択
		if (Novice::IsPressMouse(1) && !isRangeFill) {

			//塗りつぶし開始座標を決める
			startRangeFillX = (mouseX) / kMapChipSize;
			startRangeFillY = (mouseY) / kMapChipSize;

			//四角形表示座標を決める
			drawX = mouseX;
			drawY = mouseY;

			//フラグを立たせる
			isRangeFill = true;

		}

		//範囲塗りつぶし
		if (isRangeFill && !Novice::IsPressMouse(1)) {

			RangeFill();
			
		}

		if (Key::IsPress(DIK_LCONTROL)) {

			//ctrl + Zで手戻り、ctrl + Yで元に戻す
			if (Key::IsPress(DIK_Z)) {

				//操作性向上の為に入力を管理
				if (keyCount == kMaxKeyCount || (keyCount < 3 && keyCount % 3 == 0)) {
					Undo();
				}

				if (keyCount == 0) {
					keyCount = 3;
				}

				keyCount--;

			}
			else if (Key::IsPress(DIK_Y)) {

				//操作性向上の為に入力を管理
				if (keyCount == kMaxKeyCount || (keyCount < 3 && keyCount % 3 == 0)) {
					Redo();
				}

				if (keyCount == 0) {
					keyCount = 3;
				}

				keyCount--;

			}
			
			//ctrl + S でセーブ
			if (Key::IsTrigger(DIK_S)) {
				Save();
			}

		}

		//Z、Yどっちも押さずキーカウントが最大値でなければキーカウントリセット
		if (keyCount != kMaxKeyCount && !Key::IsPress(DIK_Z) && !Key::IsPress(DIK_Y)) {
			keyCount = kMaxKeyCount;
		}

	}

}

//マップチップのステータス設定
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

//マップの読み込み
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

//範囲塗りつぶし
void Map::RangeFill() {

	//塗りつぶし終了座標を決める
	endRangeFillX = (mouseX) / kMapChipSize;
	endRangeFillY = (mouseY) / kMapChipSize;

	//塗りつぶし範囲の左右、上下
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;

	//開始座標と終了座標のどっちが左かを確認する
	if (startRangeFillX <= endRangeFillX) {
		left = startRangeFillX;
		right = endRangeFillX;
	}
	else {
		left = endRangeFillX;
		right = startRangeFillX;
	}

	//開始座標と終了座標のどっちが上かを確認する
	if (startRangeFillY <= endRangeFillY) {
		top = startRangeFillY;
		bottom = endRangeFillY;
	}
	else {
		top = endRangeFillY;
		bottom = startRangeFillY;
	}

	//配列外参照にならないよう値を収める
	left = Clamp(left, 0, kMaxWidth - 1);
	right = Clamp(right, 0, kMaxWidth - 1);
	top = Clamp(top, 0, kMaxHeight - 1);
	bottom = Clamp(bottom, 0, kMaxHeight - 1);

	//範囲内塗りつぶしを開始する
	for (int y = top; y < bottom + 1; y++) {
		for (int x = left; x < right + 1; x++) {

			//redoのリストに要素があった場合、空にする
			if (redoArrayList.empty() != true) {
				redoArrayList.clear();
			}

			//redoのリスト(範囲塗りつぶし)に要素があった場合、空にする
			if (redoFillArrayList.empty() != true) {
				redoFillArrayList.clear();
			}

			//リストの最後尾に変更前の要素を追加
			//マップのナンバーを格納
			undoFillArrayList.push_back(map[y][x]);

			//サイズが一定値を超えたら古い順に削除
			if (undoFillArrayList.size() > kMaxFillListSIze) {
				for (int i = 0; i < 3; i++) {
					undoFillArrayList.pop_front();
				}
			}

			//配列の要素を変更
			map[y][x] = blockNum;

		}
	}

	//範囲を格納
	undoFillArrayList.push_back(left);
	undoFillArrayList.push_back(right);
	undoFillArrayList.push_back(top);
	undoFillArrayList.push_back(bottom);

	//範囲塗りつぶし用の値をundoリストに格納。値がずれないように三回行う
	for (int i = 0; i < 3; i++) {
		undoArrayList.push_back(-1);
	}

	//フラグを下げる
	isRangeFill = false;

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

		//範囲塗りつぶしをしていたかどうか
		if (undoArrayList.back() != -1) {

			//要素を一つずつ取り出して削除、現在のマップを保管用リストに追加
			tmpArrayX = undoArrayList.back();
			undoArrayList.pop_back();
			tmpArrayY = undoArrayList.back();
			undoArrayList.pop_back();
			tmpArrayType = undoArrayList.back();
			undoArrayList.pop_back();

			redoArrayList.push_back(tmpArrayX);
			redoArrayList.push_back(tmpArrayY);
			redoArrayList.push_back(map[tmpArrayY][tmpArrayX]);

			//取り出した要素を使い書き換える前のマップに戻す
			map[tmpArrayY][tmpArrayX] = tmpArrayType;

		}
		//範囲塗りつぶしの場合の処理
		else {

			//undoリストから値を削除
			for (int i = 0; i < 3; i++) {
				undoArrayList.pop_back();
			}

			//範囲の要素を取り出し代入
			int bottom = undoFillArrayList.back();
			undoFillArrayList.pop_back();
			int top = undoFillArrayList.back();
			undoFillArrayList.pop_back();
			int right = undoFillArrayList.back();
			undoFillArrayList.pop_back();
			int left = undoFillArrayList.back();
			undoFillArrayList.pop_back();

			//塗った時と逆の手順で塗りつぶしを行う
			for (int y = bottom; y >= top; y--) {
				for (int x = right; x >= left; x--) {
					//タイプを格納
					redoFillArrayList.push_back(map[y][x]);
					//要素を代入
					map[y][x] = undoFillArrayList.back();
					//undo塗りつぶしリストの要素を削除
					undoFillArrayList.pop_back();
				}
			}

			//範囲を格納
			redoFillArrayList.push_back(left);
			redoFillArrayList.push_back(right);
			redoFillArrayList.push_back(top);
			redoFillArrayList.push_back(bottom);

			//範囲塗りつぶし用の値をredoリストに格納。値がずれないように三回行う
			for (int i = 0; i < 3; i++) {
				redoArrayList.push_back(-1);
			}

		}

		

	}

}

//元に戻す
void Map::Redo() {

	//リストが空でないときに処理
	if (redoArrayList.empty() != true) {

		//範囲塗りつぶしをしていたかどうか
		if (redoArrayList.back() != -1) {

			//保管用のリストから要素を取り出して元のリストに戻し削除
			tmpArrayType = redoArrayList.back();
			redoArrayList.pop_back();
			tmpArrayY = redoArrayList.back();
			redoArrayList.pop_back();
			tmpArrayX = redoArrayList.back();
			redoArrayList.pop_back();

			undoArrayList.push_back(map[tmpArrayY][tmpArrayX]);
			undoArrayList.push_back(tmpArrayY);
			undoArrayList.push_back(tmpArrayX);

			//取り出した要素を使い元のマップに戻す
			map[tmpArrayY][tmpArrayX] = tmpArrayType;

		}
		else {

			//redoリストから値を削除
			for (int i = 0; i < 3; i++) {
				redoArrayList.pop_back();
			}

			//範囲の要素を取り出し代入
			int bottom = redoFillArrayList.back();
			redoFillArrayList.pop_back();
			int top = redoFillArrayList.back();
			redoFillArrayList.pop_back();
			int right = redoFillArrayList.back();
			redoFillArrayList.pop_back();
			int left = redoFillArrayList.back();
			redoFillArrayList.pop_back();

			for (int y = top; y < bottom + 1; y++) {
				for (int x = left; x < right + 1; x++) {
					//タイプを格納
					undoFillArrayList.push_back(map[y][x]);
					//要素を代入
					map[y][x] = redoFillArrayList.back();
					//undo塗りつぶしリストの要素を削除
					redoFillArrayList.pop_back();
				}
			}

			//範囲を格納
			undoFillArrayList.push_back(left);
			undoFillArrayList.push_back(right);
			undoFillArrayList.push_back(top);
			undoFillArrayList.push_back(bottom);

			//範囲塗りつぶし用の値をundoリストに格納。値がずれないように三回行う
			for (int i = 0; i < 3; i++) {
				undoArrayList.push_back(-1);
			}

		}

		

	}

}

int Map::Clamp(int x, int min, int max) {

	if (x < min) {
		x = min;
	}

	if (x > max) {
		x = max;
	}

	return x;

}
