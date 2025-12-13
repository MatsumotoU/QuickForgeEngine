#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include "KeyConfig.h"
#include "DirectInput/DirectInputManager.h"
#include "XInput/XInputController.h"

class InputInterface final :public Singleton<InputInterface> {
	friend class Singleton<InputInterface>;
public:
	InputInterface() = default;
	~InputInterface() override = default;
	// 初期化
	void Initialize(const HWND& hwnd, const HINSTANCE& hInstance);
	// 終了
	void Finalize();
	// 更新
	void Update();

	// 今回のフレームで押されたキーコードを取得する
	uint32_t GetKeyCodeTrigger();
	// 何かのキーが押されたか
	bool IsAnyKeyPressed();

	// アクション名に対応するキーが押されているか
	bool GetKeyPress(const std::string& actionName);
	// アクション名に対応するキーが押された瞬間か
	bool GetKeyTrigger(const std::string& actionName);
	// アクション名に対応するキーが離された瞬間か
	bool GetKeyRelease(const std::string& actionName);
	// ゲームでありがちな移動操作の方向を返します
	Vector2 GetKeyMoveDir();

	// マウスのボタンが押されているか
	bool GetMousePress(int8_t button);
	// マウスのボタンが押された瞬間か
	bool GetMouseTrigger(int8_t button);
	// マウスのボタンが離された瞬間か
	bool GetMouseRelease(int8_t button);
	// マウスの移動量を返します
	Vector2 GetMouseMove();
	// マウスのスクリーン上の座標を返します
	Vector2 GetMouseScreenPos();
	// マウスのホイールの回転量を返します
	float GetMouseWheelDir();

	// ゲームパッドのボタンが押されているか
	bool GetGamePadPress(uint16_t button);
	// ゲームパッドのボタンが押された瞬間か
	bool GetGamePadTrigger(uint16_t button);
	// ゲームパッドのボタンが離された瞬間か
	bool GetGamePadRelease(uint16_t button);
	// ゲームパッドの左スティックの方向を返します
	Vector2 GetGamePadLeftStickDir();
	// ゲームパッドの右スティックの方向を返します
	Vector2 GetGamePadRightStickDir();

	// アクション名に対応するキーを追加します
	void AddKeyConfig(const std::string& actionName, uint32_t keyCorde);
	// アクション名に対応するキーをすべてクリアします
	void ClearKeyConfig(const std::string& actionName);
	// アクション名に対応するキーをすべて返します
	const std::vector<uint32_t>& GetKeyConfig(const std::string& actionName);

	// Editor用
	DirectInputManager& GetDirectInputManager() { return directInputManager_; }
	KeyConfig& GetKeyConfigManager() { return keyConfig_; }

private:
	KeyConfig keyConfig_;
	DirectInputManager directInputManager_;
	XInputController xInputController_;
};