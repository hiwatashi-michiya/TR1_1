#pragma once

class Key
{
public:
	Key();

	static void Update();

	/// <summary>
	/// キーがトリガーされているかどうか返す
	/// </summary>
	/// <param name="key">キーコード([DIK_...])</param>
	/// <returns>されていたら : true　されていなかったら : false</returns>
	static bool IsTrigger(int key);

	/// <summary>
	/// キーが離されたかどうか返す
	/// </summary>
	/// <param name="key">キーコード([DIK_...])</param>
	/// <returns>されていたら : true　されていなかったら : false</returns>
	static bool IsRelease(int key);

	/// <summary>
	/// キーが押されているか(長押し)
	/// </summary>
	/// <param name="key">キーコード([DIK_...])</param>
	/// <returns>されていたら : true　されていなかったら : false</returns>
	static bool IsPress(int key);

private:

	// キー入力結果を受け取る箱
	static char keys[256];
	static char preKeys[256];

};
