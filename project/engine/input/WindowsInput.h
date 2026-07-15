/**
 * @file WindowsInput.h
 * @brief 入力（キーボード、マウス、ゲームパッド）を一括管理するクラス
 */

#pragma once
#include "InputInterface.h"
#include "DirectInput/DirectInputManager.h"
#include "XInput/XInputController.h"

#include "KeyConfig.h"
#include "Logger/InputLogger.h"
namespace QFE::INPUT {
	/**
	 * @class WindowsInput
	 * @brief 直観的な入力取得APIを提供し、内部でDirectInputやXInputを制御するシングルトンクラス
	 */
	class WindowsInput final : public InputInterface {
	public:
		/// @brief Windows用入力受付窓口
		void Initialize(const HWND& hwnd, const HINSTANCE& hInstance);
		/// @brief 終了処理
		void Finalize();
		/// @brief 更新処理（毎フレーム呼び出し）
		void Update() override;
		/// @brief フレーム終了時の処理
		void EndFrame() override;

		/// @brief 今フレームで押されたキーコードを取得（任意の一つのキー）
		uint32_t GetKeyCodeTrigger() override;
		/// @brief 何かキーが押されているか判定
		bool IsAnyKeyPressed() override;

		// --- キーボード入力 ---
		/// @brief キーコードに対応するキーが押されているか
		bool GetKeyPress(uint32_t keyCode) override;
		/// @brief キーコードに対応するキーが押された瞬間か
		bool GetKeyTrigger(uint32_t keyCode) override;
		/// @brief キーコードに対応するキーが離された瞬間か
		bool GetKeyRelease(uint32_t keyCode) override;
		/// @brief アクション名に対応するキーが押されているか
		bool GetKeyPress(const std::string& actionName) override;
		/// @brief アクション名に対応するキーが押された瞬間か
		bool GetKeyTrigger(const std::string& actionName) override;
		/// @brief アクション名に対応するキーが離された瞬間か
		bool GetKeyRelease(const std::string& actionName) override;
		/// @brief 移動操作（WASD等）の方向ベクトルを計算して取得
		MATH::Vector2 GetKeyMoveDir() override;

		// --- マウス入力 ---
		/// @brief マウスボタンが押されているか
		bool GetMousePress(int8_t button) override;
		/// @brief マウスボタンが押された瞬間か
		bool GetMouseTrigger(int8_t button) override;
		/// @brief マウスボタンが離された瞬間か
		bool GetMouseRelease(int8_t button) override;
		/// @brief マウスの移動量を取得
		MATH::Vector2 GetMouseMove() override;
		/// @brief マウスのスクリーン座標を取得
		MATH::Vector2 GetMouseScreenPos() override;
		/// @brief マウスホイールの回転量を取得
		float GetMouseWheelDir() override;

		// --- ゲームパッド入力 ---
		/// @brief パッドのボタンが押されているか
		bool GetGamePadPress(uint16_t button) override;
		/// @brief パッドのボタンが押された瞬間か
		bool GetGamePadTrigger(uint16_t button) override;
		/// @brief パッドのボタンが離された瞬間か
		bool GetGamePadRelease(uint16_t button) override;
		/// @brief 左スティックの入力を取得
		MATH::Vector2 GetGamePadLeftStickDir() override;
		/// @brief 右スティックの入力を取得
		MATH::Vector2 GetGamePadRightStickDir() override;
		// --- キーコンフィグ ---
		/// @brief アクション名にキーコードを紐付け
		void AddKeyConfig(const std::string& actionName, uint32_t keyCorde);
		/// @brief 指定したアクションのキー設定をクリア
		void ClearKeyConfig(const std::string& actionName);
		/// @brief アクションに紐付けられたキーコードリストを取得
		const std::vector<uint32_t>& GetKeyConfig(const std::string& actionName);

	private:
		KeyConfig keyConfig_;
		InputLogger inputLogger_;
		DirectInputManager directInputManager_;
		XInputController xInputController_;
	};
}  // namespace QFE::INPUT