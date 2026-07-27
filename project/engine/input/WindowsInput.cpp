/**
 * @file WindowsInput.cpp
 * @brief 各種入力デバイス(キーボード、マウス、ゲームパッド)の窓口となるクラスの実装
 */

#include "WindowsInput.h"

namespace QFE::INPUT {

	/** @brief 初期化 */
	void WindowsInput::Initialize(const HWND& hwnd, const HINSTANCE& hInstance) {
		keyConfig_.Initialize();
		directInputManager_.Initialize(hwnd, hInstance);
	}

	/** @brief 終了処理 */
	void WindowsInput::Finalize() {
		keyConfig_.Finalize();
		directInputManager_.Finalize();
	}

	/** @brief 更新 */
	void WindowsInput::Update() {
		inputLogger_.StartNewFrame();

		directInputManager_.Update();
		xInputController_.Update();
	}

	/** @brief フレーム終了時の処理 (ログ記録等) */
	void WindowsInput::EndFrame() {
		// 押されたキーをログに記録
		for (auto& pressKey : directInputManager_.keyboard_.GetPressedKeys()) {
			inputLogger_.RecordKeyPress(pressKey);
		}

		// フレーム終了処理
		inputLogger_.EndFrame();
	}

	/** @brief 最初に押されたキーコードを取得 */
	uint32_t WindowsInput::GetKeyCodeTrigger() {
		for (uint32_t keyCode = 0; keyCode < 256; keyCode++) {
			if (directInputManager_.keyboard_.GetPress(keyCode)) {
				return keyCode;
			}
		}
		return 0;
	}

	/** @brief 何かキーが押されているか判定 */
	bool WindowsInput::IsAnyKeyPressed() {
		for (uint32_t keyCode = 0; keyCode < 256; keyCode++) {
			if (directInputManager_.keyboard_.GetPress(keyCode)) {
				return true;
			}
		}
		return false;
	}

	bool WindowsInput::GetKeyPress(uint32_t keyCode) {
		return directInputManager_.keyboard_.GetPress(keyCode);
	}

	bool WindowsInput::GetKeyTrigger(uint32_t keyCode) {
		return directInputManager_.keyboard_.GetTrigger(keyCode);
	}

	bool WindowsInput::GetKeyRelease(uint32_t keyCode) {
		return directInputManager_.keyboard_.GetRelease(keyCode);
	}

	/**
	 * @brief 特定のアクション名のキーが押されているか
	 * @param actionName アクション名(例: "Jump")
	 */
	bool WindowsInput::GetKeyPress(const std::string& actionName) {
		bool isPressed = false;
		for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
			isPressed |= directInputManager_.keyboard_.GetPress(keyCode);
		}
		return isPressed;
	}

	/** @brief 特定のアクション名のキーがトリガーされたか */
	bool WindowsInput::GetKeyTrigger(const std::string& actionName) {
		bool isTriggered = false;
		for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
			isTriggered |= directInputManager_.keyboard_.GetTrigger(keyCode);
		}
		return isTriggered;
	}

	/** @brief 特定のアクション名のキーが離されたか */
	bool WindowsInput::GetKeyRelease(const std::string& actionName) {
		bool isReleased = false;
		for (const auto& keyCode : keyConfig_.GetKeys(actionName)) {
			isReleased |= directInputManager_.keyboard_.GetRelease(keyCode);
		}
		return isReleased;
	}

	/** @brief キーによる移動方向を取得 */
	MATH::Vector2 WindowsInput::GetKeyMoveDir() {
		return directInputManager_.GetKeyMoveDir();
	}

	/** @brief マウスボタンが押されているか */
	bool WindowsInput::GetMousePress(int8_t button) {
		return directInputManager_.mouse_.GetPress(button);
	}

	/** @brief マウスボタンがトリガーされたか */
	bool WindowsInput::GetMouseTrigger(int8_t button) {
		return directInputManager_.mouse_.GetTrigger(button);
	}

	/** @brief マウスボタンが離されたか */
	bool WindowsInput::GetMouseRelease(int8_t button) {
		return directInputManager_.mouse_.GetRelease(button);
	}

	/** @brief マウスの移動量を取得 */
	MATH::Vector2 WindowsInput::GetMouseMove() {
		return directInputManager_.mouse_.mouseMoveDir_;
	}

	/** @brief マウスのスクリーン座標を取得 */
	MATH::Vector2 WindowsInput::GetMouseScreenPos() {
		return directInputManager_.mouse_.mouseScreenPos_;
	}

	/** @brief マウスホイールの回転量(方向)を取得 */
	float WindowsInput::GetMouseWheelDir() {
		return directInputManager_.mouse_.wheelDir_;
	}

	/** @brief ゲームパッド各ボタンの入力状態を取得 */
	bool WindowsInput::GetGamePadPress(uint16_t button) {
		return xInputController_.GetPressButton(static_cast<WORD>(button), 0);
	}

	bool WindowsInput::GetGamePadTrigger(uint16_t button) {
		return xInputController_.GetTriggerButton(static_cast<WORD>(button), 0);
	}

	bool WindowsInput::GetGamePadRelease(uint16_t button) {
		return xInputController_.GetReleaseButton(static_cast<WORD>(button), 0);
	}

	/** @brief ゲームパッドの左スティック方向を取得 */
	MATH::Vector2 WindowsInput::GetGamePadLeftStickDir() {
		return xInputController_.GetLeftStick(0);
	}

	/** @brief ゲームパッドの右スティック方向を取得 */
	MATH::Vector2 WindowsInput::GetGamePadRightStickDir() {
		return xInputController_.GetRightStick(0);
	}

	/** @brief キーコンフィグの追加 */
	void WindowsInput::AddKeyConfig(const std::string& actionName, uint32_t keyCorde) {
		keyConfig_.AddKey(actionName, keyCorde);
	}

	/** @brief キーコンフィグのクリア */
	void WindowsInput::ClearKeyConfig(const std::string& actionName) {
		keyConfig_.RemoveKey(actionName);
	}

	/** @brief キーコンフィグの取得 */
	const std::vector<uint32_t>& WindowsInput::GetKeyConfig(const std::string& actionName) {
		return keyConfig_.GetKeys(actionName);
	}

}
