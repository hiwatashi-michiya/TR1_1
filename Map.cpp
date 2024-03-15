#include "Map.h"
#include <Novice.h>
#include <stdio.h>
#include <cassert>
#include "Key.h"
#include <imgui.h>
#include <sstream>
#include <iostream>
#include <filesystem>

Map::Map()
{

	mouseX_ = 0;
	mouseY_ = 0;
	mouseXGrid_ = 0;
	mouseYGrid_ = 0;

	tmpArrayX_ = 0;
	tmpArrayY_ = 0;
	tmpArrayType_ = 0;

	blockNum_ = 0;

	isEdit_ = false;
	isRangeFill_ = false;
	isSelect_ = false;
	isSave_ = true;

	startRangeFillX_ = 0;
	endRangeFillX_ = 0;
	startRangeFillY_ = 0;
	endRangeFillY_ = 0;

	drawX_ = 0;
	drawY_ = 0;

	ImGuiPosX_ = 0;
	ImGuiPosY_ = 0;
	ImGuiWidth_ = 0;
	ImGuiHeight_ = 0;

	selectX_ = 0;
	selectY_ = 0;

	keyCount_ = kMaxKeyCount_;

	borderRight_ = 1280 + kMapChipSize;
	borderLeft_ = -kMapChipSize;
	borderTop_ = -kMapChipSize;
	borderDown_ = 720 + kMapChipSize;

	tool_ = BRUSH;

	textureHandle_ = Novice::LoadTexture("./Resources/Texture/block.png");
	frameTexture_ = Novice::LoadTexture("./Resources/Texture/frameborder.png");
	bgTexture_ = Novice::LoadTexture("./Resources/Texture/background.png");
	groundTexture_ = Novice::LoadTexture("./Resources/Texture/ground.png");
	blockTexture_ = Novice::LoadTexture("./Resources/Texture/block.png");
	wonderBlockTexture_ = Novice::LoadTexture("./Resources/Texture/wonderblock.png");
	fixedBlockTexture_ = Novice::LoadTexture("./Resources/Texture/fixedblock.png");

	color_ = 0xFFFFFFFF;

}

Map::~Map()
{
}

void Map::Update() {

	//コントロールを押していない時
	if (!Key::IsPress(DIK_LCONTROL)) {

		if (Key::IsPress(DIK_W)) {
			
			if (scrollY_ > 0) {

				scrollY_ -= scrollValue_;

				if (scrollY_ < 0) {
					scrollY_ = 0;
				}

			}

		}

		else if (Key::IsPress(DIK_S)) {

			if (scrollY_ < kScrollLimitY_) {

				scrollY_ += scrollValue_;

				if (scrollY_ > kScrollLimitY_) {
					scrollY_ = kScrollLimitY_;
				}

			}

		}

		if (Key::IsPress(DIK_A)) {

			if (scrollX_ > 0) {

				scrollX_ -= scrollValue_;

				if (scrollX_ < 0) {
					scrollX_ = 0;
				}

			}

		}

		else if (Key::IsPress(DIK_D)) {

			if (scrollX_ < kScrollLimitX_) {
				
				scrollX_ += scrollValue_;

				if (scrollX_ > kScrollLimitX_) {
					scrollX_ = kScrollLimitX_;
				}

			}

		}

	}


	borderRight_ = scrollX_ + 1280 + kMapChipSize;
	borderLeft_ = scrollX_ - kMapChipSize;
	borderTop_ = scrollY_ - kMapChipSize;
	borderDown_ = scrollY_ + 720 + kMapChipSize;

	if (isEdit_) {

		ImGui::Begin("Editor");
		ImGuiPosX_ = ImGui::GetWindowPos().x;
		ImGuiPosY_ = ImGui::GetWindowPos().y;
		ImGuiWidth_ = ImGui::GetWindowSize().x;
		ImGuiHeight_ = ImGui::GetWindowSize().y;

		if (isOpenFile_) {

			ImGui::DragInt("blockNum", &blockNum_, 0.05f, 0, kMaxBlockType - 1);
			ImGui::InputText("tool", &toolString_[tool_][0], ImGuiInputTextFlags_ReadOnly);

			if (ImGui::Button("Save")) {
				Save();
			}

			if (ImGui::Button("Close")) {
				Close();
			}

			if (ImGui::Button("Undo")) {
				Undo();
			}

			ImGui::SameLine();

			if (ImGui::Button("Redo")) {
				Redo();
			}

		}
		else {

			ImGui::InputText(".csv", fileName_, sizeof(fileName_));

			if (ImGui::Button("Create")) {

				//ファイル名が空の場合スキップ
				if (!CheckIsEmpty(fileName_)) {
					Create();
				}
				else {
					MessageBox(nullptr, L"ファイル名を入力してください。", L"Map Editor - Create", 0);
				}

			}

			if (ImGui::Button("Load")) {

				//ファイル名が空の場合スキップ
				if (!CheckIsEmpty(fileName_)) {
					Load();
				}
				else {
					MessageBox(nullptr, L"ファイル名を入力してください。", L"Map Editor - Load", 0);
				}

			}

		}

		ImGui::End();

	}

	Novice::GetMousePosition(&mouseX_, &mouseY_);

	mouseXGrid_ = (mouseX_) / kMapChipSize;
	mouseYGrid_ = (mouseY_) / kMapChipSize;

	Edit();

}

void Map::Draw() {

	Novice::DrawQuad(0, 0, 1280, 0, 0, 720, 1280, 720, 0, 0, 1280, 720, bgTexture_, 0xFFFFFFFF);

	for (int y = 0; y < kMaxHeight; y++) {
		for (int x = 0; x < kMaxWidth; x++) {

			SetState(map_[y][x]);

			//画面範囲内なら表示
			if (x * kMapChipSize < borderRight_ &&
				x * kMapChipSize > borderLeft_ &&
				y * kMapChipSize > borderTop_ &&
				y * kMapChipSize < borderDown_) {
				
				//ブロックがあったら表示
				if (map_[y][x] != NONE) {
					Novice::DrawQuad(x * kMapChipSize - scrollX_, y * kMapChipSize - scrollY_,
						x * kMapChipSize + kMapChipSize - scrollX_, y * kMapChipSize - scrollY_,
						x * kMapChipSize - scrollX_, y * kMapChipSize + kMapChipSize - scrollY_,
						x * kMapChipSize + kMapChipSize - scrollX_, y * kMapChipSize + kMapChipSize - scrollY_,
						0, 0, 32, 32, textureHandle_, color_);
				}

			}
		}
	}


	if (isEdit_) {

		for (uint32_t y = 0; y < 4; y++) {

			for (uint32_t x = 0; x < 5; x++) {

				Novice::DrawQuad(0 + x * 1280 - scrollX_, 0 + y * 800 - scrollY_,
					1280 + x * 1280 - scrollX_, 0 + y * 800 - scrollY_,
					0 + x * 1280 - scrollX_, 800 + y * 800 - scrollY_,
					1280 + x * 1280 - scrollX_, 800 + y * 800 - scrollY_,
					0, 0, 1280, 800, frameTexture_, 0xFFFFFF66);

			}

		}

		if (Novice::IsPressMouse(0)) {
			Novice::DrawEllipse(mouseX_, mouseY_, 5, 5, 0.0f, 0xFF0000FF, kFillModeSolid);
		}
		else {
			Novice::DrawEllipse(mouseX_, mouseY_, 5, 5, 0.0f, 0xFFFFFFFF, kFillModeSolid);
		}

		Novice::ScreenPrintf(0, 0, "undoArrayList size : %d", undoArrayList_.size());
		Novice::ScreenPrintf(0, 20, "redoArrayList size : %d", redoArrayList_.size());
		Novice::ScreenPrintf(0, 40, "blockNumber : %d", blockNum_);

		Novice::DrawBox(selectX_ * kMapChipSize, selectY_ * kMapChipSize, kMapChipSize, kMapChipSize, 0.0f, 0xAA0000FF, kFillModeWireFrame);

		if (isRangeFill_ && tool_ == RANGEFILL) {
			Novice::DrawBox(drawX_, drawY_, mouseX_ - drawX_, mouseY_ - drawY_, 0.0f, 0x000000FF, kFillModeWireFrame);
		}
		else if (isSelect_ && tool_ == SELECT) {
			Novice::DrawBox(drawX_, drawY_, mouseX_ - drawX_, mouseY_ - drawY_, 0.0f, 0x0000FFFF, kFillModeWireFrame);
		}

		if (isSave_) {
			Novice::DrawEllipse(1260, 20, 10, 10, 0.0f, 0x00FF00AA, kFillModeSolid);
		}
		else {
			Novice::DrawEllipse(1260, 20, 10, 10, 0.0f, 0xFF0000AA, kFillModeSolid);
		}

	}

}

void Map::Edit() {

	//書き換えオンオフ設定
	if (Key::IsTrigger(DIK_RETURN)) {

		if (isEdit_) {
			isEdit_ = false;
		}
		else {
			isEdit_ = true;
		}

	}

	if (isEdit_ == true) {

		//コントロールを押していない時
		if (!Key::IsPress(DIK_LCONTROL)) {

			//ブロック切り替え
			if (Key::IsTrigger(DIK_E)) {

				if (blockNum_ < kMaxBlockType - 1) {
					blockNum_++;
				}

			}

			if (Key::IsTrigger(DIK_Q)) {

				if (blockNum_ > 0) {
					blockNum_--;
				}

			}

			//機能切り替え
			if (Key::IsTrigger(DIK_1)) {
				tool_ = BRUSH;
			}
			else if (Key::IsTrigger(DIK_2)) {
				tool_ = RANGEFILL;
			}
			else if (Key::IsTrigger(DIK_3)) {
				tool_ = SELECT;
			}

		}

		//ImGuiウィンドウの範囲内を反応させない
		if (!(ImGuiPosX_ <= mouseX_ && mouseX_ <= ImGuiPosX_ + ImGuiWidth_ &&
			ImGuiPosY_ <= mouseY_ && mouseY_ <= ImGuiPosY_ + ImGuiHeight_)) {

			//ボタンを押したらセレクト位置を決める
			if (Novice::IsPressMouse(0) || Novice::IsPressMouse(1)) {
				selectX_ = (mouseX_) / kMapChipSize;
				selectY_ = (mouseY_) / kMapChipSize;
			}

			//左クリックした場合
			if (Novice::IsPressMouse(0)) {

				//現在の機能によって行うことを変える
				switch (tool_)
				{
				case BRUSH:
				default:

					//単選択

					//座標を決める
					mouseXGrid_ = (mouseX_ + scrollX_) / kMapChipSize;
					mouseYGrid_ = (mouseY_ + scrollY_) / kMapChipSize;

					//配列外参照を起こさない
					mouseXGrid_ = Clamp(mouseXGrid_, 0, kMaxWidth - 1);
					mouseYGrid_ = Clamp(mouseYGrid_, 0, kMaxHeight - 1);

					//画面外だったら書き換えない
					if (mouseX_ < 1280 && mouseX_ > 0 &&
						mouseY_ < 720 && mouseY_ > 0) {

						//書き換える前と後の要素が同じ場合スルー
						if (map_[mouseYGrid_][mouseXGrid_] != blockNum_) {

							//未セーブ状態を知らせる
							if (isSave_) {
								isSave_ = false;
							}

							//redoのリストに要素があった場合、空にする
							if (redoArrayList_.empty() != true) {
								redoArrayList_.clear();
							}

							//redoのリスト(範囲塗りつぶし)に要素があった場合、空にする
							if (redoFillArrayList_.empty() != true) {
								redoFillArrayList_.clear();
							}

							//リストの最後尾に変更前の要素を追加
							//マップのナンバーを最初に格納
							undoArrayList_.push_back(map_[mouseYGrid_][mouseXGrid_]);
							//行の数字を追加
							undoArrayList_.push_back(mouseYGrid_);
							//列の数字を追加
							undoArrayList_.push_back(mouseXGrid_);

							//サイズが一定値を超えたら古い順に削除
							if (undoArrayList_.size() > kMaxListSize) {
								for (int i = 0; i < 3; i++) {
									undoArrayList_.pop_front();
								}
							}

							//配列の要素を変更
							map_[mouseYGrid_][mouseXGrid_] = blockNum_;

						}

					}

					break;
				case RANGEFILL:

					//範囲塗りつぶし
					if (isRangeFill_ == false) {
						//塗りつぶし開始座標を決める
						startRangeFillX_ = (mouseX_ + scrollX_) / kMapChipSize;
						startRangeFillY_ = (mouseY_ + scrollY_) / kMapChipSize;

						//四角形表示座標を決める
						drawX_ = mouseX_;
						drawY_ = mouseY_;

						//フラグを立たせる
						isRangeFill_ = true;
					}

					

					break;
				case SELECT:

					//範囲選択
					if (isSelect_ == false) {
						//選択開始座標を決める
						startRangeFillX_ = (mouseX_ + scrollX_) / kMapChipSize;
						startRangeFillY_ = (mouseY_ + scrollY_) / kMapChipSize;

						//四角形表示座標を決める
						drawX_ = mouseX_;
						drawY_ = mouseY_;

						isSelect_ = true;
					}

					

					break;
				
				}

			}

		}

		

		//範囲塗りつぶし
		if (isRangeFill_ && !Novice::IsPressMouse(0) && tool_ == RANGEFILL) {

			//未セーブ状態を知らせる
			if (isSave_) {
				isSave_ = false;
			}

			RangeFill();
			
		}

		if (isSelect_ && !Novice::IsPressMouse(0) && tool_ == SELECT) {

			//未セーブ状態を知らせる
			if (isSave_) {
				isSave_ = false;
			}

			Select();

		}

		if (Key::IsPress(DIK_LCONTROL)) {

			//ctrl + Zで手戻り、ctrl + Yで元に戻す
			if (Key::IsPress(DIK_Z)) {

				//操作性向上の為に入力を管理
				if (keyCount_ == kMaxKeyCount_ || (keyCount_ < 3 && keyCount_ % 3 == 0)) {
					Undo();
				}

				if (keyCount_ == 0) {
					keyCount_ = 3;
				}

				keyCount_--;

			}
			else if (Key::IsPress(DIK_Y)) {

				//操作性向上の為に入力を管理
				if (keyCount_ == kMaxKeyCount_ || (keyCount_ < 3 && keyCount_ % 3 == 0)) {
					Redo();
				}

				if (keyCount_ == 0) {
					keyCount_ = 3;
				}

				keyCount_--;

			}
			
			//ctrl + S でセーブ
			if (Key::IsTrigger(DIK_S)) {
				Save();
			}

		}

		//Z、Yどっちも押さずキーカウントが最大値でなければキーカウントリセット
		if (keyCount_ != kMaxKeyCount_ && !Key::IsPress(DIK_Z) && !Key::IsPress(DIK_Y)) {
			keyCount_ = kMaxKeyCount_;
		}

	}

}

//マップチップのステータス設定
void Map::SetState(int mapNum) {

	switch (mapNum)
	{
	case NONE:
	default:
		color_ = 0xFFFFFFFF;
		break;
	case GROUND:
		color_ = 0xFFFFFFFF;
		textureHandle_ = groundTexture_;
		break;
	case BLOCK:
		color_ = 0xFFFFFFFF;
		textureHandle_ = blockTexture_;
		break;
	case WONDERBLOCK:
		color_ = 0xFFFFFFFF;
		textureHandle_ = wonderBlockTexture_;
		break;
	case FIXEDBLOCK:
		color_ = 0xFFFFFFFF;
		textureHandle_ = fixedBlockTexture_;
		break;
	}

}

//マップの読み込み
void Map::Load() {

	FILE* fp = NULL;

	std::string str = "./Resources/Maps/";

	str += fileName_;

	str += ".csv";

	fopen_s(&fp, str.c_str(), "rb");

	//指定したファイルが無かったらリターン
	if (fp == NULL) {

		MessageBox(nullptr, L"指定したファイルは存在しません", L"Map Load", 0);

		return;

	}

	assert(fp != NULL);

	for (int y = 0; y < kMaxHeight; y++) {
		for (int x = 0; x < kMaxWidth; x++) {
			fscanf_s(fp, "%d,", &map_[y][x]);
		}
	}

	fclose(fp);

	isOpenFile_ = true;

}

//範囲塗りつぶし
void Map::RangeFill() {

	//塗りつぶし終了座標を決める
	endRangeFillX_ = (mouseX_ + scrollX_) / kMapChipSize;
	endRangeFillY_ = (mouseY_ + scrollY_) / kMapChipSize;

	//塗りつぶし範囲の左右、上下
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;

	//開始座標と終了座標のどっちが左かを確認する
	if (startRangeFillX_ <= endRangeFillX_) {
		left = startRangeFillX_;
		right = endRangeFillX_;
	}
	else {
		left = endRangeFillX_;
		right = startRangeFillX_;
	}

	//開始座標と終了座標のどっちが上かを確認する
	if (startRangeFillY_ <= endRangeFillY_) {
		top = startRangeFillY_;
		bottom = endRangeFillY_;
	}
	else {
		top = endRangeFillY_;
		bottom = startRangeFillY_;
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
			if (redoArrayList_.empty() != true) {
				redoArrayList_.clear();
			}

			//redoのリスト(範囲塗りつぶし)に要素があった場合、空にする
			if (redoFillArrayList_.empty() != true) {
				redoFillArrayList_.clear();
			}

			//リストの最後尾に変更前の要素を追加
			//マップのナンバーを格納
			undoFillArrayList_.push_back(map_[y][x]);

			//サイズが一定値を超えたら古い順に削除
			if (undoFillArrayList_.size() > kMaxFillListSIze) {
				for (int i = 0; i < 3; i++) {
					undoFillArrayList_.pop_front();
				}
			}

			//配列の要素を変更
			map_[y][x] = blockNum_;

		}
	}

	//範囲を格納
	undoFillArrayList_.push_back(left);
	undoFillArrayList_.push_back(right);
	undoFillArrayList_.push_back(top);
	undoFillArrayList_.push_back(bottom);

	//範囲塗りつぶし用の値をundoリストに格納。値がずれないように三回行う
	for (int i = 0; i < 3; i++) {
		undoArrayList_.push_back(-1);
	}

	//フラグを下げる
	isRangeFill_ = false;

}

//セーブ機能
void Map::Save() {

	FILE* fp = NULL;

	std::string str = "./Resources/Maps/";

	str += fileName_;

	str += ".csv";

	fopen_s(&fp, str.c_str(), "w+b");

	if (fp == NULL) {

		MessageBox(nullptr, L"ファイルが存在しません。", L"Map Load", 0);

		return;

	}

	assert(fp != NULL);

	for (int y = 0; y < kMaxHeight; y++) {
		for (int x = 0; x < kMaxWidth; x++) {

			//ファイルの書き込む場所を最後尾に設定
			fseek(fp, 0, SEEK_END);
			std::stringstream stream;
			//数字を16進数に変換
			stream << std::hex << map_[y][x];
			std::string hexString = stream.str();
			//カンマを追加
			hexString += ",";
			//x行が最大までいったら改行
			if (x == kMaxWidth - 1) {
				hexString += "\n";
			}
			//指定した場所の値を16進数で書き換える
			fputs(hexString.c_str(), fp);

		}
	}

	fclose(fp);

	//ファイルが保存されたことを伝える
	MessageBox(nullptr, L"ファイルを保存しました", L"Map Save", 0);

	//セーブが完了したことを伝える
	isSave_ = true;

}

void Map::Close() {

	//セーブしていなかったら、セーブするかどうか聞く
	if (!isSave_) {

		if (MessageBox(nullptr, L"ファイルが保存されていません。保存しますか？", L"Map - Close", MB_OKCANCEL) == IDOK) {

			Save();

		}

	}

	isOpenFile_ = false;

}

void Map::Create() {

	//読み込むJSONファイルのフルパスを合成する
	std::string filePath = "./Resources/Maps/";

	filePath += fileName_;

	filePath += ".csv";

	std::filesystem::path path(filePath);

	//ファイルパスが存在するか確認
	if (std::filesystem::exists(path)) {

		if (MessageBox(nullptr, L"同名ファイルが既にあります。上書きしますか？", L"Map - Create", MB_OKCANCEL) == IDCANCEL) {

			return;

		}

	}

	FILE* fp = NULL;

	fopen_s(&fp, filePath.c_str(), "w");

	if (fp == NULL) {

		MessageBox(nullptr, L"ファイルの作成に失敗しました。", L"Map Create", 0);

		return;

	}

	assert(fp != NULL);

	for (int y = 0; y < kMaxHeight; y++) {
		for (int x = 0; x < kMaxWidth; x++) {

			//ファイルの書き込む場所を最後尾に設定
			fseek(fp, 0, SEEK_END);
			std::stringstream stream;
			//数字を16進数に変換
			stream << std::hex << 0;
			std::string hexString = stream.str();
			//カンマを追加
			hexString += ",";
			//x行が最大までいったら改行
			if (x == kMaxWidth - 1) {
				hexString += "\n";
			}
			//指定した場所の値を16進数で書き換える
			fputs(hexString.c_str(), fp);

		}
	}

	fclose(fp);

	isOpenFile_ = true;

}

//手戻り
void Map::Undo() {

	//リストが空でないときに処理
	if (undoArrayList_.empty() != true) {

		//範囲塗りつぶしをしていたかどうか
		if (undoArrayList_.back() != -1) {

			//要素を一つずつ取り出して削除、現在のマップを保管用リストに追加
			tmpArrayX_ = undoArrayList_.back();
			undoArrayList_.pop_back();
			tmpArrayY_ = undoArrayList_.back();
			undoArrayList_.pop_back();
			tmpArrayType_ = undoArrayList_.back();
			undoArrayList_.pop_back();

			redoArrayList_.push_back(tmpArrayX_);
			redoArrayList_.push_back(tmpArrayY_);
			redoArrayList_.push_back(map_[tmpArrayY_][tmpArrayX_]);

			//取り出した要素を使い書き換える前のマップに戻す
			map_[tmpArrayY_][tmpArrayX_] = tmpArrayType_;

		}
		//範囲塗りつぶしの場合の処理
		else {

			//undoリストから値を削除
			for (int i = 0; i < 3; i++) {
				undoArrayList_.pop_back();
			}

			//範囲の要素を取り出し代入
			int bottom = undoFillArrayList_.back();
			undoFillArrayList_.pop_back();
			int top = undoFillArrayList_.back();
			undoFillArrayList_.pop_back();
			int right = undoFillArrayList_.back();
			undoFillArrayList_.pop_back();
			int left = undoFillArrayList_.back();
			undoFillArrayList_.pop_back();

			//塗った時と逆の手順で塗りつぶしを行う
			for (int y = bottom; y >= top; y--) {
				for (int x = right; x >= left; x--) {
					//タイプを格納
					redoFillArrayList_.push_back(map_[y][x]);
					//要素を代入
					map_[y][x] = undoFillArrayList_.back();
					//undo塗りつぶしリストの要素を削除
					undoFillArrayList_.pop_back();
				}
			}

			//範囲を格納
			redoFillArrayList_.push_back(left);
			redoFillArrayList_.push_back(right);
			redoFillArrayList_.push_back(top);
			redoFillArrayList_.push_back(bottom);

			//範囲塗りつぶし用の値をredoリストに格納。値がずれないように三回行う
			for (int i = 0; i < 3; i++) {
				redoArrayList_.push_back(-1);
			}

		}

		

	}

}

//元に戻す
void Map::Redo() {

	//リストが空でないときに処理
	if (redoArrayList_.empty() != true) {

		//範囲塗りつぶしをしていたかどうか
		if (redoArrayList_.back() != -1) {

			//保管用のリストから要素を取り出して元のリストに戻し削除
			tmpArrayType_ = redoArrayList_.back();
			redoArrayList_.pop_back();
			tmpArrayY_ = redoArrayList_.back();
			redoArrayList_.pop_back();
			tmpArrayX_ = redoArrayList_.back();
			redoArrayList_.pop_back();

			undoArrayList_.push_back(map_[tmpArrayY_][tmpArrayX_]);
			undoArrayList_.push_back(tmpArrayY_);
			undoArrayList_.push_back(tmpArrayX_);

			//取り出した要素を使い元のマップに戻す
			map_[tmpArrayY_][tmpArrayX_] = tmpArrayType_;

		}
		else {

			//redoリストから値を削除
			for (int i = 0; i < 3; i++) {
				redoArrayList_.pop_back();
			}

			//範囲の要素を取り出し代入
			int bottom = redoFillArrayList_.back();
			redoFillArrayList_.pop_back();
			int top = redoFillArrayList_.back();
			redoFillArrayList_.pop_back();
			int right = redoFillArrayList_.back();
			redoFillArrayList_.pop_back();
			int left = redoFillArrayList_.back();
			redoFillArrayList_.pop_back();

			for (int y = top; y < bottom + 1; y++) {
				for (int x = left; x < right + 1; x++) {
					//タイプを格納
					undoFillArrayList_.push_back(map_[y][x]);
					//要素を代入
					map_[y][x] = redoFillArrayList_.back();
					//undo塗りつぶしリストの要素を削除
					redoFillArrayList_.pop_back();
				}
			}

			//範囲を格納
			undoFillArrayList_.push_back(left);
			undoFillArrayList_.push_back(right);
			undoFillArrayList_.push_back(top);
			undoFillArrayList_.push_back(bottom);

			//範囲塗りつぶし用の値をundoリストに格納。値がずれないように三回行う
			for (int i = 0; i < 3; i++) {
				undoArrayList_.push_back(-1);
			}

		}

		

	}

}

void Map::Select() {

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

bool Map::CheckIsEmpty(const std::string& name) {

	if (name.empty()) {
		return true;
	}

	return false;

}
