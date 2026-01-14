/**
 * @file InputInterface.h
 * @brief 入力（キーボード、マウス、ゲームパッド）を一括管理するクラス
 */

#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"

#include "DirectInput/DirectInputManager.h"
#include "XInput/XInputController.h"
#include "Engine/include/input/Mic/WASAPIMicrophoneDevice.h"

#include "KeyConfig.h"
#include "Logger/InputLogger.h"

/**
 * @class InputInterface
 * @brief 直観的な入力取得APIを提供し、内部でDirectInputやXInputを制御するシングルトンクラス
 */
class InputInterface final :public Singleton<InputInterface> {
	friend class Singleton<InputInterface>;
public:
	InputInterface() = default;
	~InputInterface() override = default;
    
    /**
     * @brief 初期化処理
     * @param hwnd ウィンドウハンドル
     * @param hInstance インスタンスハンドル
     */
	void Initialize(const HWND& hwnd, const HINSTANCE& hInstance);
    /** @brief 終了処理 */
	void Finalize();
    /** @brief 更新処理（毎フレーム呼び出し） */
	void Update();
    /** @brief フレーム終了時の処理 */
	void EndFrame();

    /** @brief 今フレームで押されたキーコードを取得（任意の一つのキー） */
	uint32_t GetKeyCodeTrigger();
    /** @brief 何らかのキーが押されているか判定 */
	bool IsAnyKeyPressed();

	// --- キーボード入力 ---
    /** @brief アクション名に対応するキーが押されているか */
	bool GetKeyPress(const std::string& actionName);
    /** @brief アクション名に対応するキーが押された瞬間か */
	bool GetKeyTrigger(const std::string& actionName);
    /** @brief アクション名に対応するキーが離された瞬間か */
	bool GetKeyRelease(const std::string& actionName);
    /** @brief 移動操作（WASD等）の方向ベクトルを計算して取得 */
	Vector2 GetKeyMoveDir();

	// --- マウス入力 ---
    /** @brief マウスボタンが押されているか */
	bool GetMousePress(int8_t button);
    /** @brief マウスボタンが押された瞬間か */
	bool GetMouseTrigger(int8_t button);
    /** @brief マウスボタンが離された瞬間か */
	bool GetMouseRelease(int8_t button);
    /** @brief マウスの移動量を取得 */
	Vector2 GetMouseMove();
    /** @brief マウスのスクリーン座標を取得 */
	Vector2 GetMouseScreenPos();
    /** @brief マウスホイールの回転量を取得 */
	float GetMouseWheelDir();

	// --- ゲームパッド入力 ---
    /** @brief パッドのボタンが押されているか */
	bool GetGamePadPress(uint16_t button);
    /** @brief パッドのボタンが押された瞬間か */
	bool GetGamePadTrigger(uint16_t button);
    /** @brief パッドのボタンが離された瞬間か */
	bool GetGamePadRelease(uint16_t button);
    /** @brief 左スティックの入力を取得 */
	Vector2 GetGamePadLeftStickDir();
    /** @brief 右スティックの入力を取得 */
	Vector2 GetGamePadRightStickDir();

	// --- キーコンフィグ ---
    /** @brief アクション名にキーコードを紐付け */
	void AddKeyConfig(const std::string& actionName, uint32_t keyCorde);
    /** @brief 指定したアクションのキー設定をクリア */
	void ClearKeyConfig(const std::string& actionName);
    /** @brief アクションに紐付けられたキーコードリストを取得 */
	const std::vector<uint32_t>& GetKeyConfig(const std::string& actionName);

	// Editor用
	DirectInputManager& GetDirectInputManager() { return directInputManager_; }
	XInputController& GetXInputController() { return xInputController_; }
	WASAPIMicrophoneDevice& GetMicrophoneDevice() { return microphoneDevice_; }

	KeyConfig& GetKeyConfigManager() { return keyConfig_; }
	InputLogger& GetInputLogger() { return inputLogger_; }

private:
	KeyConfig keyConfig_;
	InputLogger inputLogger_;
	DirectInputManager directInputManager_;
	XInputController xInputController_;
	WASAPIMicrophoneDevice microphoneDevice_;
};
