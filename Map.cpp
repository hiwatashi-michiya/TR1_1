#include "Map.h"
#include <Novice.h>
#include <stdio.h>
#include <cassert>
#include "Key.h"
#include <imgui.h>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <json.hpp>
#include <fstream>

Map::Map()
{

	isTouchGui_ = false;

	mouseX_ = 0;
	mouseY_ = 0;
	mouseXGrid_ = 0;
	mouseYGrid_ = 0;

	tmpArrayX_ = 0;
	tmpArrayY_ = 0;
	tmpArrayType_ = 0;

	blockNum_ = 0;
	preBlockNum_ = 0;

	isEdit_ = true;
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

	//一部の場所は固定ブロックに変更
	for (uint32_t y = 0; y < kMaxHeight; y++) {

		for (uint32_t x = 0; x < kMaxWidth; x++) {

			isWrite_[y][x] = true;

		}

	}

	textureFileNames_.push_back("./Resources/Textures/Blocks/block0.png");
	textureFileNames_.push_back("./Resources/Textures/gimmick/cloud_shadow2.png");
	textureFileNames_.push_back("./Resources/Textures/player/player_stop.png");
	textureFileNames_.push_back("./Resources/Textures/gimmick/key_light.png");
	textureFileNames_.push_back("./Resources/Textures/gimmick/goal_close_doing.png");
	textureFileNames_.push_back("./Resources/Textures/gimmick/coin.png");
	textureFileNames_.push_back("./Resources/Textures/enemy/enemy_eye_down.png");
	textureFileNames_.push_back("./Resources/Textures/Blocks/block7.png");
	textureFileNames_.push_back("./Resources/Textures/Blocks/block8.png");
	textureFileNames_.push_back("./Resources/Textures/Blocks/block9.png");
	textureFileNames_.push_back("./Resources/Textures/Blocks/block10.png");

	textureHandle_ = Novice::LoadTexture("./Resources/Textures/gimmick/cloud_shadow2.png");
	frameTexture_ = Novice::LoadTexture("./Resources/Textures/frameborder.png");
	bgTexture_ = Novice::LoadTexture("./Resources/Textures/backGround/backGround_back.png");
	block1Tex_ = Novice::LoadTexture("./Resources/Textures/gimmick/cloud_shadow2.png");
	block2Tex_ = Novice::LoadTexture("./Resources/Textures/player/player_stop.png");
	block3Tex_ = Novice::LoadTexture("./Resources/Textures/gimmick/key_light.png");
	block4Tex_ = Novice::LoadTexture("./Resources/Textures/gimmick/goal_close_doing.png");
	block5Tex_ = Novice::LoadTexture("./Resources/Textures/gimmick/coin.png");
	block6Tex_ = Novice::LoadTexture("./Resources/Textures/enemy/enemy_eye_down.png");
	block7Tex_ = Novice::LoadTexture("./Resources/Textures/Blocks/block7.png");
	block8Tex_ = Novice::LoadTexture("./Resources/Textures/Blocks/block8.png");
	block9Tex_ = Novice::LoadTexture("./Resources/Textures/Blocks/block9.png");
	block10Tex_ = Novice::LoadTexture("./Resources/Textures/Blocks/block10.png");

	color_ = 0xFFFFFFFF;

	undoArrayList_.clear();
	redoArrayList_.clear();
	undoFillArrayList_.clear();
	redoFillArrayList_.clear();

	LoadAllMaps();

}

Map::~Map()
{
}

void Map::Update() {

	preBlockNum_ = blockNum_;

	for (int32_t i = 0; i < kMaxBlock; i++) {
		blockCounts_[i] = 0;
	}

	for (int32_t y = 0; y < kMaxHeight; y++) {

		for (int32_t x = 0; x < kMaxWidth; x++) {

			if (map_[y][x] != kNone) {
				blockCounts_[map_[y][x]]++;
			}

		}

	}

	//コントロールを押していない時
	if (!Key::IsPress(DIK_LCONTROL) && isOpenFile_) {

		if (Key::IsPress(DIK_W)) {
			
			if (scrollY_ > 0) {

				scrollY_ -= scrollValue_;

				if (isRangeFill_) {
					drawHeightScrollY_ -= scrollValue_;
				}

				if (scrollY_ < 0) {
					scrollY_ = 0;
				}

			}

		}

		else if (Key::IsPress(DIK_S)) {

			if (scrollY_ < kScrollLimitY_) {

				scrollY_ += scrollValue_;

				if (isRangeFill_) {
					drawHeightScrollY_ += scrollValue_;
				}

				if (scrollY_ > kScrollLimitY_) {
					scrollY_ = kScrollLimitY_;
				}

			}

		}

		if (Key::IsPress(DIK_A)) {

			if (scrollX_ > 0) {

				scrollX_ -= scrollValue_;

				if (isRangeFill_) {
					drawWidthScrollX_ -= scrollValue_;
				}

				if (scrollX_ < 0) {
					scrollX_ = 0;
				}

			}

		}

		else if (Key::IsPress(DIK_D)) {

			if (scrollX_ < kScrollLimitX_) {
				
				scrollX_ += scrollValue_;

				if (isRangeFill_) {
					drawWidthScrollX_ += scrollValue_;
				}

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

			ImGui::DragInt("blockNum", &blockNum_, 0.05f, 0, BlockType::kMaxBlock - 1);

			ImGui::Text(blockNames_[blockNum_].c_str());

			{

				int toolNum = tool_;

				std::vector<const char*> toolStr;

				for (uint32_t i = 0; i < 2; i++) {
					toolStr.push_back(tools_[i].c_str());
				}

				ImGui::Combo("Tool", &toolNum, toolStr.data(), int(toolStr.size()));

				tool_ = static_cast<TOOL>(toolNum);

			}

			/*ImGui::InputText("tool", &toolString_[tool_][0], ImGuiInputTextFlags_ReadOnly);*/

			if (ImGui::Button("Save")) {
				Save();
				SaveJson();
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

			ImGui::Text(" cloud : %d\n player : %d\n key : %d\n goal : %d\n coin : %d\n enemy : %d",
				blockCounts_[kCloud], blockCounts_[kPlayerPoint], blockCounts_[kKey], blockCounts_[kGoal], blockCounts_[kCollection], blockCounts_[kEye]);

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

			ImGui::Separator();
			ImGui::Text("Map List");

			for (int32_t i = 0; i < mapNames_.size(); i++) {

				if (ImGui::Button(mapNames_[i].c_str())) {

					for (int32_t k = 0; k < mapNames_[i].size(); k++) {
						fileName_[k] = mapNames_[i][k];
					}

					Load();
				}

			}

		}

		/*ImGui::Text("Scroll X : %d", scrollX_);
		ImGui::Text("Scroll Y : %d", scrollY_);*/

		ImGui::End();

	}

	ImGui::Begin("Manual");
	ImGui::Text("wasd : scroll");
	ImGui::Text("left or right arrow : change block");
	ImGui::Text(" ctrl + s : save \n ctrl + z : undo \n ctrl + y : redo ");
	ImGui::End();

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
				if (map_[y][x] != kNone) {
					Novice::DrawQuad(x * kMapChipSize - scrollX_, y * kMapChipSize - scrollY_,
						x * kMapChipSize + kMapChipSize - scrollX_, y * kMapChipSize - scrollY_,
						x * kMapChipSize - scrollX_, y * kMapChipSize + kMapChipSize - scrollY_,
						x * kMapChipSize + kMapChipSize - scrollX_, y * kMapChipSize + kMapChipSize - scrollY_,
						0, 0, 512, 512, textureHandle_, color_);
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
					0, 0, 1280, 800, frameTexture_, 0xFFFFFF22);

			}

		}

		SetState(blockNum_);

		if (blockNum_ != 0) {
			Novice::DrawQuad(mouseX_ - 16, mouseY_ - 16, mouseX_ + 16, mouseY_ - 16,
				mouseX_ - 16, mouseY_ + 16, mouseX_ + 16, mouseY_ + 16,
				0, 0, 512, 512, textureHandle_, color_);
		}

		Novice::ScreenPrintf(0, 0, "undoArrayList size : %d", undoArrayList_.size());
		Novice::ScreenPrintf(0, 20, "redoArrayList size : %d", redoArrayList_.size());
		Novice::ScreenPrintf(0, 40, "blockNumber : %d", blockNum_);

		Novice::DrawBox(selectX_ * kMapChipSize - scrollX_, selectY_ * kMapChipSize - scrollY_, kMapChipSize, kMapChipSize, 0.0f, 0xAA0000FF, kFillModeWireFrame);

		Novice::DrawLine(0, 32 * 4 + 16, 1280, 32 * 4 + 16, 0xAA0000FF);
		Novice::DrawLine(0, 32 * 18, 1280, 32 * 18, 0xAA0000FF);

		if (isRangeFill_ && tool_ == RANGEFILL) {
			Novice::DrawBox(drawX_ - drawWidthScrollX_, drawY_ - drawHeightScrollY_,
				mouseX_ - (drawX_ - drawWidthScrollX_), mouseY_ - (drawY_ - drawHeightScrollY_), 0.0f, 0x000000FF, kFillModeWireFrame);
		}
		else if (isSelect_ && tool_ == SELECT) {
			Novice::DrawBox(drawX_ - drawWidthScrollX_, drawY_ - drawHeightScrollY_,
				mouseX_ - (drawX_ - drawWidthScrollX_), mouseY_ - (drawY_ - drawHeightScrollY_), 0.0f, 0x0000FFFF, kFillModeWireFrame);
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

	if (isEdit_ && isOpenFile_) {

		//コントロールを押していない時
		if (!Key::IsPress(DIK_LCONTROL)) {

			//ブロック切り替え
			if (Key::IsTrigger(DIK_E) || Key::IsTrigger(DIK_RIGHT)) {

				if (blockNum_ < BlockType::kMaxBlock - 1) {
					blockNum_++;
				}

			}

			if (Key::IsTrigger(DIK_Q) || Key::IsTrigger(DIK_LEFT)) {

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

		//ImGuiウィンドウ内で触っていたらマップの塗りつぶしをしないようにする
		if (Novice::IsTriggerMouse(0) && (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemHovered())) {
			isTouchGui_ = true;
		}

		if (isTouchGui_ && !Novice::IsPressMouse(0)) {
			isTouchGui_ = false;
		}

		//ImGuiウィンドウの範囲内を反応させない
		if (!isTouchGui_) {

			//ボタンを押したらセレクト位置を決める
			if (Novice::IsPressMouse(0) || Novice::IsPressMouse(1)) {
				selectX_ = (mouseX_ + scrollX_) / kMapChipSize;
				selectY_ = (mouseY_ + scrollY_) / kMapChipSize;
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

						//書き換える前と後の要素が同じ場合か書き換え不可の場合スルー
						if (map_[mouseYGrid_][mouseXGrid_] != blockNum_ && isWrite_[mouseYGrid_][mouseXGrid_]) {

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

						drawWidthScrollX_ = 0;
						drawHeightScrollY_ = 0;

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
				SaveJson();
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
	case kNone:
	default:
		color_ = 0x00000000;
		break;
	case kCloud:
		color_ = 0xFFFFFFFF;
		textureHandle_ = block1Tex_;
		break;
	case kPlayerPoint:
		color_ = 0xFFFFFFFF;
		textureHandle_ = block2Tex_;
		break;
	case kKey:
		color_ = 0xFFFFFFFF;
		textureHandle_ = block3Tex_;
		break;
	case kGoal:
		color_ = 0xFFFFFFFF;
		textureHandle_ = block4Tex_;
		break;
	case kCollection:
		color_ = 0xFFFFFFFF;
		textureHandle_ = block5Tex_;
		break;
	case kEye:
		color_ = 0xFFFFFFFF;
		textureHandle_ = block6Tex_;
		break;
	}

}

//マップの読み込み
void Map::Load() {

	FILE* fp = NULL;

	std::string str = "Resources/Maps/";

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
			
			fscanf_s(fp, "%x,", &map_[y][x]);

			if (map_[y][x] >= kMaxBlock) {
				map_[y][x] = 1;
			}

		}
	}

	fclose(fp);

	isOpenFile_ = true;

}

void Map::LoadJson() {

	

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

			//書き換え不可の場合スキップ
			if (isWrite_[y][x]) {

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

	std::string str = "Resources/Maps/";

	//ディレクトリが無ければ作成する
	std::filesystem::path dir(str);
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}

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

void Map::SaveJson() {

	//csvのパス
	std::string csvStr = "Resources/Maps/";
	csvStr += fileName_;
	csvStr += ".csv";


	nlohmann::json root;

	root = nlohmann::json::object();

	root["objects"] = nlohmann::json::array();

	//カメラの追加
	{

		//カメラ2Dコンポーネント追加
		root["objects"].push_back(nlohmann::json::object());
		root["objects"].back()["Comps"] = nlohmann::json::array();

		root["objects"].back()["Comps"].push_back(nlohmann::json::object());
		root["objects"].back()["Comps"].back()["CompName"] = "class CameraComp";

		root["objects"].back()["Comps"].push_back(nlohmann::json::object());
		root["objects"].back()["Comps"].back()["CompName"] = "class TransformComp";
		root["objects"].back()["Comps"].back()["scale"] =
			nlohmann::json::array({ 0.6f, 0.6f, 1.0f });
		root["objects"].back()["Comps"].back()["rotate"] =
			nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
		root["objects"].back()["Comps"].back()["translate"] =
			nlohmann::json::array({ 624.0f, -344.0f, -10.0f });

		root["objects"].back()["Comps"].push_back(nlohmann::json::object());
		root["objects"].back()["Comps"].back()["CompName"] = "class Camera2DComp";
		root["objects"].back()["Comps"].back()["width"] = 1280;
		root["objects"].back()["Comps"].back()["height"] = 720;
		root["objects"].back()["Comps"].back()["farClip"] = 1000.0f;
		root["objects"].back()["Comps"].back()["nearClip"] = 0.1f;

		root["objects"].back()["Comps"].push_back(nlohmann::json::object());
		root["objects"].back()["Comps"].back()["CompName"] = "class FollowCamera2DComp";

		root["objects"].back()["type"] = "Object";

	}

	root["objects"].push_back(nlohmann::json::object());
	root["objects"].back()["Comps"] = nlohmann::json::array();

	//スプライトレンダーデータコンポーネント追加
	root["objects"].back()["Comps"].push_back(nlohmann::json::object());
	root["objects"].back()["Comps"].back()["BlendType"] = "kNormal";
	root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
	root["objects"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,1.0f,1.0f,1.0f });
	root["objects"].back()["Comps"].back()["fileName"] = "./Resources/Textures/backGround/backGround_back.png";
	root["objects"].back()["Comps"].back()["offsetType"] = "kMiddle";

	//スプライトレンダーコンポーネント追加
	root["objects"].back()["Comps"].push_back(nlohmann::json::object());
	root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

	//トランスフォームコンポーネント追加
	root["objects"].back()["Comps"].push_back(nlohmann::json::object());
	root["objects"].back()["Comps"].back()["CompName"] = "class TransformComp";
	root["objects"].back()["Comps"].back()["scale"] =
		nlohmann::json::array({ 768.0f,432.0f,16.0f });
	root["objects"].back()["Comps"].back()["rotate"] =
		nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
	root["objects"].back()["Comps"].back()["translate"] =
		nlohmann::json::array({ 640.0f - 16.0f, -360.0f + 16.0f, 100.0f });

	//バックグラウンドコンポーネント追加
	root["objects"].back()["Comps"].push_back(nlohmann::json::object());
	root["objects"].back()["Comps"].back()["CompName"] = "class BackGroundComp";

	root["scene"] = fileName_;

	for (int32_t y = 0; y < kMaxHeight; y++) {

		for (int32_t x = 0; x < kMaxWidth; x++) {

			//2Dコンポーネント追加
			if (map_[y][x] == kCloud) {

				root["objects"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"] = nlohmann::json::array();

				//スプライトレンダーデータコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["BlendType"] = "kNormal";
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
				root["objects"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,1.0f,1.0f,1.0f });
				root["objects"].back()["Comps"].back()["fileName"] = textureFileNames_[map_[y][x]];
				root["objects"].back()["Comps"].back()["offsetType"] = "kMiddle";

				//スプライトレンダーコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

				//トランスフォームコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class TransformComp";
				root["objects"].back()["Comps"].back()["scale"] =
					nlohmann::json::array({ float(kMapChipSize), float(kMapChipSize),  float(kMapChipSize) });
				root["objects"].back()["Comps"].back()["rotate"] =
					nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
				root["objects"].back()["Comps"].back()["translate"] =
					nlohmann::json::array({ float(x * kMapChipSize), -float(y * kMapChipSize), 5.0f });
				
				//雲コンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class CloudComp";

				//OBBコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class ObbComp";
				root["objects"].back()["Comps"].back()["center"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });
				root["objects"].back()["Comps"].back()["collisiionTags"] = nlohmann::json::array();
				root["objects"].back()["Comps"].back()["scale"] = nlohmann::json::array({ 32.5f,32.5f,32.5f });
				root["objects"].back()["Comps"].back()["isScaleEffect"] = false;

				//OBB押し出しコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class ObbPushComp";
				root["objects"].back()["Comps"].back()["pushTags"] = nlohmann::json::array({});

				//AABB2Dコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Aabb2DComp";
				root["objects"].back()["Comps"].back()["scale"] = nlohmann::json::array({ 32.0f,32.0f,32.0f });
				root["objects"].back()["Comps"].back()["isScaleEffect"] = false;

				//フラグコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class FlagComp";

				//マスコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Mass2DComp";
				root["objects"].back()["Comps"].back()["mass"] = nlohmann::json::array({ x,y });
				root["objects"].back()["Comps"].back()["offset"] = nlohmann::json::array({ 0.0f,0.0f });
				root["objects"].back()["Comps"].back()["size"] = 32;

				//クラウドレンダーコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class CloudRenderComp";
				root["objects"].back()["Comps"].back()["scaleMax"] = 1.2f;
				root["objects"].back()["Comps"].back()["startTime_Max"] = 1.0f;
				root["objects"].back()["Comps"].back()["startTime_Min"] = 0.0f;

				//イージングコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class EaseingComp";
				root["objects"].back()["Comps"].back()["isLoop"] = true;
				root["objects"].back()["Comps"].back()["spdT"] = 2.0f;
				root["objects"].back()["Comps"].back()["type"] = "InOutSine";

				//インスタンスタイムコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class InstanceTimeComp";

				root["objects"].back()["type"] = "Object";

			}
			else if (map_[y][x] == kPlayerPoint) {

				root["objects"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"] = nlohmann::json::array();

				//プレイヤーコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class PlayerComp";

				//スプライトレンダーデータコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["BlendType"] = "kNormal";
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
				root["objects"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,1.0f,1.0f,1.0f });
				root["objects"].back()["Comps"].back()["fileName"] = textureFileNames_[map_[y][x]];
				root["objects"].back()["Comps"].back()["offsetType"] = "kMiddle";

				//スプライトレンダーコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

				//トランスフォームコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class TransformComp";
				root["objects"].back()["Comps"].back()["scale"] =
					nlohmann::json::array({ float(kMapChipSize) * 1.5f, float(kMapChipSize) * 1.5f,  float(kMapChipSize) * 1.5f });
				root["objects"].back()["Comps"].back()["rotate"] =
					nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
				root["objects"].back()["Comps"].back()["translate"] =
					nlohmann::json::array({ float(x * kMapChipSize), -float(y * kMapChipSize), 1.0f });

				//落下コンポーネント追加追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class FallComp";
				root["objects"].back()["Comps"].back()["gravity"] = -9.8f;

				//OBBコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class ObbComp";
				root["objects"].back()["Comps"].back()["center"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });
				root["objects"].back()["Comps"].back()["collisiionTags"] = nlohmann::json::array();
				root["objects"].back()["Comps"].back()["scale"] = nlohmann::json::array({ 0.5f,0.5f,0.5f });

				//OBB押し出しコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class ObbPushComp";
				root["objects"].back()["Comps"].back()["pushTags"] = nlohmann::json::array();

				//AABB2Dコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Aabb2DComp";
				root["objects"].back()["Comps"].back()["scale"] = nlohmann::json::array({ 24.0f,24.0f,24.0f });
				root["objects"].back()["Comps"].back()["isScaleEffect"] = false;

				//インプットコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class InputMoveComp";
				root["objects"].back()["Comps"].back()["speed"] = 1.0f;

				//フラグコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class FlagComp";

				//方向コンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Direction2DComp";

				//csvDataのコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class CsvDataComp";
				root["objects"].back()["Comps"].back()["fileName"] = csvStr;

				//雲食べのコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class EatCloudComp";

				//雲吐きのコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class RemoveCloudComp";

				//カウントコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class CountComp";

				//マスコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Mass2DComp";
				root["objects"].back()["Comps"].back()["mass"] = nlohmann::json::array({ x,y });
				root["objects"].back()["Comps"].back()["offset"] = nlohmann::json::array({ 16.0f,16.0f });
				root["objects"].back()["Comps"].back()["size"] = 32;

				//アニメーションコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteAnimatorComp";
				root["objects"].back()["Comps"].back()["startPos"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });
				root["objects"].back()["Comps"].back()["isLoop"] = true;
				root["objects"].back()["Comps"].back()["animationNumber"] = 6;
				root["objects"].back()["Comps"].back()["duration"] = 0.1666f;

				//テクスチャハンドルコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class TextureHandlesComp";
				root["objects"].back()["Comps"].back()["fileNames"] = nlohmann::json::array({
					"./Resources/Textures/player/player_stop.png",
					"./Resources/Textures/player/player_walk.png",
					"./Resources/Textures/player/player_eat.png",
					"./Resources/Textures/player/player_out.png",
					"./Resources/Textures/player/player_stop_transparent.png",
					"./Resources/Textures/player/player_walk_transparent.png",
					"./Resources/Textures/player/player_eat_transparent.png",
					"./Resources/Textures/player/player_out_transparent.png"
					});

			}
			else if (map_[y][x] == kKey) {

				root["objects"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"] = nlohmann::json::array();

				//スプライトレンダーデータコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["BlendType"] = "kNormal";
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
				root["objects"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,1.0f,1.0f,1.0f });
				root["objects"].back()["Comps"].back()["fileName"] = textureFileNames_[map_[y][x]];
				root["objects"].back()["Comps"].back()["offsetType"] = "kMiddle";

				//スプライトレンダーコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

				//トランスフォームコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class TransformComp";
				root["objects"].back()["Comps"].back()["scale"] =
					nlohmann::json::array({ float(kMapChipSize) * 0.8f, float(kMapChipSize) * 0.8f,  float(kMapChipSize) * 0.8f });
				root["objects"].back()["Comps"].back()["rotate"] =
					nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
				root["objects"].back()["Comps"].back()["translate"] =
					nlohmann::json::array({ float(x * kMapChipSize), -float(y * kMapChipSize), 4.0f });

				//鍵コンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class KeyComp";

				//マスコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Mass2DComp";
				root["objects"].back()["Comps"].back()["mass"] = nlohmann::json::array({ x,y });
				root["objects"].back()["Comps"].back()["offset"] = nlohmann::json::array({ 0.0f,0.0f });
				root["objects"].back()["Comps"].back()["size"] = 32;

				//AABB2Dコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Aabb2DComp";
				root["objects"].back()["Comps"].back()["scale"] = nlohmann::json::array({ 0.8f,0.8f,0.8f });

				//アニメーションコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteAnimatorComp";
				root["objects"].back()["Comps"].back()["startPos"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });
				root["objects"].back()["Comps"].back()["isLoop"] = true;
				root["objects"].back()["Comps"].back()["animationNumber"] = 6;
				root["objects"].back()["Comps"].back()["duration"] = 0.1666f;

				root["objects"].back()["type"] = "Object";

			}
			else if (map_[y][x] == kGoal) {

				root["objects"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"] = nlohmann::json::array();

				//スプライトレンダーデータコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["BlendType"] = "kNormal";
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
				root["objects"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,1.0f,1.0f,1.0f });
				root["objects"].back()["Comps"].back()["fileName"] = textureFileNames_[map_[y][x]];
				root["objects"].back()["Comps"].back()["offsetType"] = "kMiddle";

				//スプライトレンダーコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

				//トランスフォームコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class TransformComp";
				root["objects"].back()["Comps"].back()["scale"] =
					nlohmann::json::array({ 64.0f,45.33f,32.0f });
				root["objects"].back()["Comps"].back()["rotate"] =
					nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
				root["objects"].back()["Comps"].back()["translate"] =
					nlohmann::json::array({ float(x * kMapChipSize), -float(y * kMapChipSize), 2.0f });

				//ゴールコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class GoalComp";

				//マスコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Mass2DComp";
				root["objects"].back()["Comps"].back()["mass"] = nlohmann::json::array({ x,y });
				root["objects"].back()["Comps"].back()["offset"] = nlohmann::json::array({ 0.0f,0.0f });
				root["objects"].back()["Comps"].back()["size"] = 32;

				//AABB2Dコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Aabb2DComp";
				root["objects"].back()["Comps"].back()["scale"] = nlohmann::json::array({ 0.35f,0.55f,1.0f });

				//アニメーションコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteAnimatorComp";
				root["objects"].back()["Comps"].back()["startPos"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });
				root["objects"].back()["Comps"].back()["isLoop"] = true;
				root["objects"].back()["Comps"].back()["animationNumber"] = 8;
				root["objects"].back()["Comps"].back()["duration"] = 0.125f;

				//テクスチャハンドルコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class TextureHandlesComp";
				root["objects"].back()["Comps"].back()["fileNames"] = nlohmann::json::array({
					"./Resources/Textures/gimmick/goal_open_doing.png",
					"./Resources/Textures/gimmick/goal_close_doing.png",
					"./Resources/Textures/gimmick/goal_open_anime.png",
					"./Resources/Textures/gimmick/goal_close_anime.png"
					});

				root["objects"].back()["type"] = "Object";

			}
			else if (map_[y][x] == kCollection) {

				root["objects"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"] = nlohmann::json::array();

				//スプライトレンダーデータコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["BlendType"] = "kNormal";
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
				root["objects"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,1.0f,1.0f,1.0f });
				root["objects"].back()["Comps"].back()["fileName"] = textureFileNames_[map_[y][x]];
				root["objects"].back()["Comps"].back()["offsetType"] = "kMiddle";

				//スプライトレンダーコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

				//トランスフォームコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class TransformComp";
				root["objects"].back()["Comps"].back()["scale"] =
					nlohmann::json::array({ float(kMapChipSize), float(kMapChipSize),  float(kMapChipSize) });
				root["objects"].back()["Comps"].back()["rotate"] =
					nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
				root["objects"].back()["Comps"].back()["translate"] =
					nlohmann::json::array({ float(x * kMapChipSize), -float(y * kMapChipSize), 4.0f });

				//収集アイテムコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class CollectionComp";

				//マスコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Mass2DComp";
				root["objects"].back()["Comps"].back()["mass"] = nlohmann::json::array({ x,y });
				root["objects"].back()["Comps"].back()["offset"] = nlohmann::json::array({ 0.0f,0.0f });
				root["objects"].back()["Comps"].back()["size"] = 32;

				//AABB2Dコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class Aabb2DComp";
				root["objects"].back()["Comps"].back()["scale"] = nlohmann::json::array({ 0.8f,0.8f,0.8f });

				//アニメーションコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteAnimatorComp";
				root["objects"].back()["Comps"].back()["startPos"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });
				root["objects"].back()["Comps"].back()["isLoop"] = true;
				root["objects"].back()["Comps"].back()["animationNumber"] = 7;
				root["objects"].back()["Comps"].back()["duration"] = 0.142857f;

				root["objects"].back()["type"] = "Object";

			}
			else if (map_[y][x] == kEye) {

				root["objects"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"] = nlohmann::json::array();

				//敵コンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class EyeComp";

				//トランスフォームコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class TransformComp";
				root["objects"].back()["Comps"].back()["scale"] =
					nlohmann::json::array({ 100.0f,100.0f,100.0f });
				root["objects"].back()["Comps"].back()["rotate"] =
					nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
				root["objects"].back()["Comps"].back()["translate"] =
					nlohmann::json::array({ float(x * kMapChipSize), -float(y * kMapChipSize), 10.0f });

				//敵ステータスコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class EyeStateComp";
				root["objects"].back()["Comps"].back()["aimFixedTime"] = 1.0f;
				root["objects"].back()["Comps"].back()["aimeTime"] = 1.5f;
				root["objects"].back()["Comps"].back()["fireTime"] = 0.5f;

				//線当たり判定コンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class LineCollisionComp";
				root["objects"].back()["Comps"].back()["CollisionTags"] = nlohmann::json::array({ "class CloudComp" });

				//線コンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class LineComp";
				root["objects"].back()["Comps"].back()["end"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });
				root["objects"].back()["Comps"].back()["start"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });

				//線描画データコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class LineRenderDataComp";
				root["objects"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,0.0f,0.0f,1.0f });
				root["objects"].back()["Comps"].back()["isDepth"] = true;

				//イージングコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class EaseingComp";
				root["objects"].back()["Comps"].back()["isLoop"] = false;
				root["objects"].back()["Comps"].back()["spdT"] = 0.5f;
				root["objects"].back()["Comps"].back()["type"] = "InOutSine";

				//子のコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class ChildrenObjectComp";
				root["objects"].back()["Comps"].back()["Children"] = nlohmann::json::array();
				root["objects"].back()["Comps"].back()["Children"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["Children"].back()["Comps"] = nlohmann::json::array();

				{
					//スプライトレンダーデータコンポーネント追加
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].push_back(nlohmann::json::object());
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["BlendType"] = "kNormal";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,1.0f,1.0f,1.0f });
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["fileName"] = "./Resources/Textures/enemy/enemy_eye_up.png";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["offsetType"] = "kMiddle";

					//スプライトレンダーコンポーネント追加
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].push_back(nlohmann::json::object());
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

					//トランスフォームコンポーネント追加
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].push_back(nlohmann::json::object());
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["CompName"] = "class TransformComp";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["scale"] =
						nlohmann::json::array({ 1.0f, 1.0f, 1.0f });
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["rotate"] =
						nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["translate"] =
						nlohmann::json::array({ 0.0f,0.0f,-0.1f });

					root["objects"].back()["Comps"].back()["Children"].back()["type"] = "Object";

				}

				//子のコンポーネント追加
				root["objects"].back()["Comps"].back()["Children"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["Children"].back()["Comps"] = nlohmann::json::array();

				//ビーム
				{

					//スプライトレンダーデータコンポーネント追加
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].push_back(nlohmann::json::object());
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["BlendType"] = "kNormal";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,0.0f,0.0f,1.0f });
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["fileName"] = "./Resources/EngineResources/white2x2.png";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["offsetType"] = "kMiddle";

					//スプライトレンダーコンポーネント追加
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].push_back(nlohmann::json::object());
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

					//トランスフォームコンポーネント追加
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].push_back(nlohmann::json::object());
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["CompName"] = "class TransformComp";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["scale"] =
						nlohmann::json::array({ 0.0f, 10.0f, 10.0f });
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["rotate"] =
						nlohmann::json::array({ 0.0f, 0.0f, 0.0f, 1.0f });
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["translate"] =
						nlohmann::json::array({ 0.0f,0.0f,0.0f });

					//線のコンバートトランスフォームコンポーネント追加
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].push_back(nlohmann::json::object());
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["CompName"] = "class LineConvertTransformComp";

					//トランスフォームコンポーネント追加
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].push_back(nlohmann::json::object());
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["CompName"] = "class LineComp";
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["end"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });
					root["objects"].back()["Comps"].back()["Children"].back()["Comps"].back()["start"] = nlohmann::json::array({ 0.0f,0.0f,0.0f });

					root["objects"].back()["Comps"].back()["Children"].back()["type"] = "Object";

				}

				//線描画コンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class LineRenderComp";

				//スプライトレンダーコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderComp";

				//スプライトレンダーデータコンポーネント追加
				root["objects"].back()["Comps"].push_back(nlohmann::json::object());
				root["objects"].back()["Comps"].back()["BlendType"] = "kNormal";
				root["objects"].back()["Comps"].back()["CompName"] = "class SpriteRenderDataComp";
				root["objects"].back()["Comps"].back()["color"] = nlohmann::json::array({ 1.0f,1.0f,1.0f,1.0f });
				root["objects"].back()["Comps"].back()["fileName"] = "./Resources/Textures/enemy/enemy_eye_down.png";
				root["objects"].back()["Comps"].back()["offsetType"] = "kMiddle";

				root["objects"].back()["type"] = "Object";

			}

		}

	}

	std::string str = "SceneData/";

	//ディレクトリが無ければ作成する
	std::filesystem::path dir(str);
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}

	//書き込むJSONファイルのフルパスを合成する
	std::string filePath = str + fileName_ + ".json";
	//書き込み用ファイルストリーム
	std::ofstream ofs;
	//ファイルを書き込み用に開く
	ofs.open(filePath);

	//ファイルオープン失敗したら表示
	if (ofs.fail()) {
		MessageBox(nullptr, L"ファイルを開くのに失敗しました。", L"Map SaveJson", 0);
		return;
	}

	//ファイルにjson文字列を書き込む(インデント幅4)
	ofs << std::setw(4) << root << std::endl;
	//ファイルを閉じる
	ofs.close();

	MessageBox(nullptr, L"セーブしました。", L"Map SaveJson", 0);

	isSave_ = true;

}

void Map::Close() {

	//セーブしていなかったら、セーブするかどうか聞く
	if (!isSave_) {

		if (MessageBox(nullptr, L"ファイルが保存されていません。保存しますか？", L"Map - Close", MB_OKCANCEL) == IDOK) {

			Save();
			SaveJson();

		}

	}

	for (uint32_t y = 0; y < kMaxHeight; y++) {

		for (uint32_t x = 0; x < kMaxWidth; x++) {
			map_[y][x] = 0;
		}

	}

	undoArrayList_.clear();
	redoArrayList_.clear();
	undoFillArrayList_.clear();
	redoFillArrayList_.clear();

	isOpenFile_ = false;

	isSave_ = true;

	std::memset(fileName_, 0, sizeof(fileName_));

	LoadAllMaps();

}

void Map::Create() {

	std::string filePath = "Resources/Maps/";

	//ディレクトリが無ければ作成する
	std::filesystem::path dir(filePath);
	if (!std::filesystem::exists(dir)) {
		std::filesystem::create_directory(dir);
	}

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

			map_[y][x] = 0;

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

	isOpenFile_ = true;

}

void Map::LoadAllMaps() {

	mapNames_.clear();

	std::filesystem::recursive_directory_iterator itr("./Resources/Maps/");

	//検索する拡張子
	std::string extension = ".csv";

	//マップ全検索
	for (const auto& entry : itr) {

		if (std::filesystem::is_regular_file(entry.path()) &&
			entry.path().extension() == extension) {
			std::string mapName = entry.path().stem().string();
			//最後尾に追加
			mapNames_.push_back(mapName);
		}

	}

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

					if (isWrite_[y][x]) {
						//タイプを格納
						redoFillArrayList_.push_back(map_[y][x]);
						//要素を代入
						map_[y][x] = undoFillArrayList_.back();
						//undo塗りつぶしリストの要素を削除
						undoFillArrayList_.pop_back();
					}

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

					if (isWrite_[y][x]) {

						//タイプを格納
						undoFillArrayList_.push_back(map_[y][x]);
						//要素を代入
						map_[y][x] = redoFillArrayList_.back();
						//undo塗りつぶしリストの要素を削除
						redoFillArrayList_.pop_back();

					}

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
