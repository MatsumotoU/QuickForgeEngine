/**
 * @file InputInterface.cpp
 * @brief 各種入力デバイス(キーボード、マウス、ゲームパッド)の窓口となるクラスの実装
 */

#include "engine/include/input/InputInterface.h"

namespace QFE {

	/** @brief 初期化 */
	void InputInterface::Initialize(const HWND& hwnd, const HINSTANCE& hInstance) {
		keyConfig_.Initialize();
		keyConfig_.LoadKeyConfig();

		directInputManager_.Initialize(hwnd, hInstance);
		microphoneDevice_.Initialize();
	}

	/** @brief 終了処理 */
	void InputInterface::Finalize() {
		microphoneDevice_.Finalize();
		keyConfig_.SaveKeyConfig();
		keyConfig_.Finalize();
		directInputManager_.Finalize();
	}

	/** @brief 更新 */
	void InputInterface::Update() {
		inputLogger_.StartNewFrame();

		directInputManager_.Update();
		xInputController_.Update();
	}

	/** @brief フレーム終了時の処理 (ログ記録等) */
	void InputInterface::EndFrame()
	{
		// 押されたキーをログに記録
		for (auto& pressKey : directInputManager_.keyboard_.GetPressedKeys()) {
			inputLogger_.RecordKeyPress(pressKey);
		}

		// フレーム終了処理
		inputLogger_.EndFrame();
	}

	/** @brief 最初に押されたキーコードを取得 */
	uint32_t InputInterface::GetKeyCodeTrigger() {
		for (uint32_t keyCode = 0; keyCode < 256; keyCode++) {
			if (directInputManager_.keyboard_.GetPress(keyCode)) {
				return keyCode;
			}
		}
		return 0;
	}

	/** @brief 何かキーが押されているか判定 */
	bool InputInterface::IsAnyKeyPressed() {
		for (uint32_t keyCode = 0; keyCode < 256; keyCode++) {
			if (directInputManager_.keyboard_.GetPress(keyCode)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief 特定のアクション名のキーが押されているか
	 * @param actionName アクション名(例: "Jump")
	 */
	bool InputInterface::GetKeyPress(const std::string& actionName) {
		bool isPressed = false;
		for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
			isPressed |= directInputManager_.keyboard_.GetPress(keyCode);
		}
		return isPressed;
	}

	/** @brief 特定のアクション名のキーがトリガーされたか */
	bool InputInterface::GetKeyTrigger(const std::string& actionName) {
		bool isTriggered = false;
		for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
			isTriggered |= directInputManager_.keyboard_.GetTrigger(keyCode);
		}
		return isTriggered;
	}

	/** @brief 特定のアクション名のキーが離されたか */
	bool InputInterface::GetKeyRelease(const std::string& actionName) {
		bool isReleased = false;
		for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
			isReleased |= directInputManager_.keyboard_.GetRelease(keyCode);
		}
		return isReleased;
	}

	/** @brief キーによる移動方向を取得 */
	Vector2 InputInterface::GetKeyMoveDir() {
		return directInputManager_.GetKeyMoveDir();
	}

	/** @brief マウスボタンが押されているか */
	bool InputInterface::GetMousePress(int8_t button) {
		return directInputManager_.mouse_.GetPress(button);
	}

	/** @brief マウスボタンがトリガーされたか */
	bool InputInterface::GetMouseTrigger(int8_t button) {
		return directInputManager_.mouse_.GetTrigger(button);
	}

	/** @brief マウスボタンが離されたか */
	bool InputInterface::GetMouseRelease(int8_t button) {
		return directInputManager_.mouse_.GetRelease(button);
	}

	/** @brief マウスの移動量を取得 */
	Vector2 InputInterface::GetMouseMove() {
		return directInputManager_.mouse_.mouseMoveDir_;
	}

	/** @brief マウスのスクリーン座標を取得 */
	Vector2 InputInterface::GetMouseScreenPos() {
		return directInputManager_.mouse_.mouseScreenPos_;
	}

	/** @brief マウスホイールの回転量(方向)を取得 */
	float InputInterface::GetMouseWheelDir() {
		return directInputManager_.mouse_.wheelDir_;
	}

	/** @brief ゲームパッド各ボタンの入力状態を取得 */
	bool InputInterface::GetGamePadPress(uint16_t button) {
		return xInputController_.GetPressButton(static_cast<WORD>(button), 0);
	}

	bool InputInterface::GetGamePadTrigger(uint16_t button) {
		return xInputController_.GetTriggerButton(static_cast<WORD>(button), 0);
	}

	bool InputInterface::GetGamePadRelease(uint16_t button) {
		return xInputController_.GetReleaseButton(static_cast<WORD>(button), 0);
	}

	/** @brief ゲームパッドの左スティック方向を取得 */
	Vector2 InputInterface::GetGamePadLeftStickDir() {
		return xInputController_.GetLeftStick(0);
	}

	/** @brief ゲームパッドの右スティック方向を取得 */
	Vector2 InputInterface::GetGamePadRightStickDir() {
		return xInputController_.GetRightStick(0);
	}

	/** @brief キーコンフィグの追加 */
	void InputInterface::AddKeyConfig(const std::string& actionName, uint32_t keyCorde) {
		keyConfig_.AddKey(actionName, keyCorde);
	}

	/** @brief キーコンフィグのクリア */
	void InputInterface::ClearKeyConfig(const std::string& actionName) {
		keyConfig_.RemoveKey(actionName);
	}

	/** @brief キーコンフィグの取得 */
	const std::vector<uint32_t>& InputInterface::GetKeyConfig(const std::string& actionName) {
		return keyConfig_.GetKeys(actionName);
	}

}
