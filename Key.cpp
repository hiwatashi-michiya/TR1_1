#include "Key.h"
#include<Novice.h>

Key::Key()
{
}

void Key::Update() {

	// キー入力を受け取る
	memcpy(preKeys, keys, 256);//ひとつ前のキー入力に移す
	Novice::GetHitKeyStateAll(keys);//keys(現在のキー入力の配列)を更新する

}


bool Key::IsTrigger(int key) {

	if (preKeys[key] == 0 && keys[key] != 0) {
		return true;
	}

	return false;

}

bool Key::IsRelease(int key) {

	if (preKeys[key] != 0 && keys[key] == 0) {
		return true;
	}

	return false;

}

bool Key::IsPress(int key) {

	if (keys[key] != 0) {
		return true;
	}

	return false;

}

char Key::keys[256] = { 0 };
char Key::preKeys[256] = { 0 };
