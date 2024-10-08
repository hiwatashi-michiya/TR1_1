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

			//スタート地点付近作成
			if (y < 5) {

				isWrite_[y][x] = false;

				

			}
			else if (y == 5) {

				if ((x >= 8 && x <= 13) || (x >= 26 && x <= 31)) {
					isWrite_[y][x] = false;
				}
				else if (x <= 5 || (x >= 16 && x <= 23) || x >= 34) {
					isWrite_[y][x] = false;
				}

			}
			else if (y == 6) {

				if ((x >= 9 && x <= 12) || (x >= 27 && x <= 30)) {
					isWrite_[y][x] = false;
				}

			}
			//一番下を破壊できないブロックにする
			else if (y == kMaxHeight - 1) {
				isWrite_[y][x] = false;
			}
			//左右を破壊できないブロックにする
			if (x == 0 || x == kMaxWidth - 1) {
				isWrite_[y][x] = false;
			}

		}

	}

	textureHandle_ = Novice::LoadTexture("./Resources/textures/blocks/defaultRocks.png");
	frameTexture_ = Novice::LoadTexture("./Resources/textures/frameborder.png");
	bgTexture_ = Novice::LoadTexture("./Resources/textures/backGround/background.png");
	unBreakBlockTex_ = Novice::LoadTexture("./Resources/textures/blocks/defaultRocks.png");
	coldBlockTex_ = Novice::LoadTexture("./Resources/textures/blocks/rocks.png");
	hotBlockTex_ = Novice::LoadTexture("./Resources/textures/blocks/rocks.png");
	iceBlockTex_ = Novice::LoadTexture("./Resources/textures/blocks/rocks.png");
	speedBlockTex_ = Novice::LoadTexture("./Resources/textures/blocks/blue.png");
	digBlockTex_ = Novice::LoadTexture("./Resources/textures/blocks/green.png");
	saunaBlockTex_ = Novice::LoadTexture("./Resources/textures/blocks/red.png");
	downBlockTex_ = Novice::LoadTexture("./Resources/textures/blocks/down.png");
	needleTex_ = Novice::LoadTexture("./Resources/textures/blocks/needle.png");
	TNTTex_ = Novice::LoadTexture("./Resources/textures/blocks/bomb.png");

	color_ = 0xFFFFFFFF;

	undoArrayList_.clear();
	redoArrayList_.clear();
	undoFillArrayList_.clear();
	redoFillArrayList_.clear();

}

Map::~Map()
{
}

void Map::Update() {

	preBlockNum_ = blockNum_;

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

			if (blockNum_ == kMagma) {

				if (preBlockNum_ < kMagma) {
					blockNum_++;
				}
				else {
					blockNum_--;
				}
			}

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

		ImGui::Text("Scroll X : %d", scrollX_);
		ImGui::Text("Scroll Y : %d", scrollY_);

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
				if (map_[y][x] != kNone) {
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
					0, 0, 1280, 800, frameTexture_, 0xFFFFFF22);

			}

		}

		SetState(blockNum_);

		if (blockNum_ != 0) {
			Novice::DrawQuad(mouseX_ - 16, mouseY_ - 16, mouseX_ + 16, mouseY_ - 16,
				mouseX_ - 16, mouseY_ + 16, mouseX_ + 16, mouseY_ + 16,
				0, 0, 32, 32, textureHandle_, color_);
		}

		Novice::ScreenPrintf(0, 0, "undoArrayList size : %d", undoArrayList_.size());
		Novice::ScreenPrintf(0, 20, "redoArrayList size : %d", redoArrayList_.size());
		Novice::ScreenPrintf(0, 40, "blockNumber : %d", blockNum_);

		Novice::DrawBox(selectX_ * kMapChipSize - scrollX_, selectY_ * kMapChipSize - scrollY_, kMapChipSize, kMapChipSize, 0.0f, 0xAA0000FF, kFillModeWireFrame);

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

	Novice::SetBlendMode(kBlendModeAdd);
	Novice::DrawBox(0, 0, 1280, 720, 0.0f, 0xFFFFFF11, kFillModeSolid);

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
			if (Key::IsTrigger(DIK_E)) {

				if (blockNum_ < BlockType::kMaxBlock - 1) {
					blockNum_++;

					//マグマ飛ばす
					if (blockNum_ == kMagma) {
						blockNum_++;
					}

				}

			}

			if (Key::IsTrigger(DIK_Q)) {

				if (blockNum_ > 0) {

					blockNum_--;

					//マグマ飛ばす
					if (blockNum_ == kMagma) {
						blockNum_--;
					}

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
		if (Novice::IsTriggerMouse(0) && (ImGuiPosX_ <= mouseX_ && mouseX_ <= ImGuiPosX_ + ImGuiWidth_ &&
			ImGuiPosY_ <= mouseY_ && mouseY_ <= ImGuiPosY_ + ImGuiHeight_)) {
			isTouchGui_ = true;
		}

		if (isTouchGui_ && !Novice::IsPressMouse(0)) {
			isTouchGui_ = false;
		}

		//ImGuiウィンドウの範囲内を反応させない
		if (!(ImGuiPosX_ <= mouseX_ && mouseX_ <= ImGuiPosX_ + ImGuiWidth_ &&
			ImGuiPosY_ <= mouseY_ && mouseY_ <= ImGuiPosY_ + ImGuiHeight_) && !isTouchGui_) {

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

							//配列の要素を変更

							//マグマなら極寒に切り替え
							if (blockNum_ != kMagma) {
								map_[mouseYGrid_][mouseXGrid_] = blockNum_;
							}
							else {
								
							}


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
		color_ = 0x000000FF;
		break;
	case kUnbreakable:
		color_ = 0xBBBBBBFF;
		textureHandle_ = unBreakBlockTex_;
		break;
	case kSnow:
		color_ = 0xAAAAFFFF;
		textureHandle_ = coldBlockTex_;
		break;
	case kMagma:
		color_ = 0xFFAAAAFF;
		textureHandle_ = hotBlockTex_;
		break;
	case kIceBlock:
		color_ = 0xDDDDFFFF;
		textureHandle_ = unBreakBlockTex_;
		break;
	case kSpeedBlock:
		color_ = 0x8888AAFF;
		textureHandle_ = saunaBlockTex_;
		break;
	case kDigerBlock:
		color_ = 0x88AA88FF;
		textureHandle_ = digBlockTex_;
		break;
	case kSaunnerBlock:
		color_ = 0xAA8888FF;
		textureHandle_ = speedBlockTex_;
		break;
	case kDownMagma:
		color_ = 0xFFFFFFFF;
		textureHandle_ = downBlockTex_;
		break;
	case kGoldBlock:
		color_ = 0xFFFF00FF;
		textureHandle_ = unBreakBlockTex_;
		break;
	case kFlag:
		color_ = 0xFFFFFFFF;
		textureHandle_ = unBreakBlockTex_;
		break;
	case kCollapse:
		color_ = 0xFFFFFFFF;
		textureHandle_ = coldBlockTex_;
		break;
	case kEnemyBlock:
		color_ = 0xFF0000FF;
		textureHandle_ = coldBlockTex_;
		break;
	case kNeedleBlock:
		color_ = 0xDDDDFFFF;
		textureHandle_ = needleTex_;
		break;
	case kTNTBlock:
		color_ = 0xFFFFFFFF;
		textureHandle_ = TNTTex_;
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
			fscanf_s(fp, "%x,", &map_[y][x]);
		}
	}

	//一部の場所は固定ブロックに変更
	for (uint32_t y = 0; y < kMaxHeight; y++) {

		for (uint32_t x = 0; x < kMaxWidth; x++) {

			//スタート地点付近作成
			if (y < 5) {

				map_[y][x] = 1;

				//一部は空白に変更
				if (y == 0) {

					if ((x >= 10 && x <= 11) || (x >= 28 && x <= 29)) {
						map_[y][x] = 0;
					}

				}
				else {

					if ((x >= 8 && x <= 13) || (x >= 26 && x <= 31)) {
						map_[y][x] = 0;
					}

					if (y == 1 || y == 4) {

						if ((x >= 17 && x <= 18) || (x >= 21 && x <= 22)) {
							map_[y][x] = 0;
						}

					}
					else {

						if ((x >= 14 && x <= 18) || (x >= 21 && x <= 25)) {
							map_[y][x] = 0;
						}

					}

				}

			}
			else if (y == 5) {

				if ((x >= 8 && x <= 13) || (x >= 26 && x <= 31)) {
					map_[y][x] = 0;
				}
				else if (x <= 5 || (x >= 16 && x <= 23) || x >= 34) {
					map_[y][x] = 1;
				}

			}
			else if (y == 6) {

				if ((x >= 9 && x <= 12) || (x >= 27 && x <= 30)) {
					map_[y][x] = 1;
				}

			}
			//一番下を破壊できないブロックにする
			else if (y == kMaxHeight - 1) {
				map_[y][x] = 1;
			}
			//左右を破壊できないブロックにする
			if (x == 0 || x == kMaxWidth - 1) {
				map_[y][x] = 1;
			}

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

			//スタート地点付近作成
			if (y < 5) {

				map_[y][x] = 1;

				//一部は空白に変更
				if (y == 0) {

					if ((x >= 10 && x <= 11) || (x >= 28 && x <= 29)) {
						map_[y][x] = 0;
					}

				}
				else {

					if ((x >= 8 && x <= 13) || (x >= 26 && x <= 31)) {
						map_[y][x] = 0;
					}

					if (y == 1 || y == 4) {

						if ((x >= 17 && x <= 18) || (x >= 21 && x <= 22)) {
							map_[y][x] = 0;
						}

					}
					else {

						if ((x >= 14 && x <= 18) || (x >= 21 && x <= 25)) {
							map_[y][x] = 0;
						}

					}

				}

			}
			else if (y == 5) {

				if ((x >= 8 && x <= 13) || (x >= 26 && x <= 31)) {
					map_[y][x] = 0;
				}
				else if (x <= 5 || (x >= 16 && x <= 23) || x >= 34) {
					map_[y][x] = 1;
				}

			}
			else if (y == 6) {

				if ((x >= 9 && x <= 12) || (x >= 27 && x <= 30)) {
					map_[y][x] = 1;
				}

			}
			//一番下を破壊できないブロックにする
			else if (y == kMaxHeight - 1) {
				map_[y][x] = 1;
			}
			//左右を破壊できないブロックにする
			if (x == 0 || x == kMaxWidth - 1) {
				map_[y][x] = 1;
			}

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
