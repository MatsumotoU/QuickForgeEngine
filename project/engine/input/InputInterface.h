/**
 * @file InputInterface.h
 * @brief 入力（キーボード、マウス、ゲームパッド）を一括管理するクラス
 */

#pragma once
#include "math/MathInclude.h"
#include <string>
#include <cstdint>

namespace QFE::INPUT {
	/**
	 * @class InputInterface
	 * @brief 直観的な入力取得APIを提供し、内部でDirectInputやXInputを制御するシングルトンクラス
	 */
	class InputInterface {
	public:
		/// @brief 更新処理
		virtual void Update() = 0;
		/// @brief フレーム終了時の処理
		virtual void EndFrame() = 0;

		/// @brief 今フレームで押されたキーコードを取得（任意の一つのキー）
		virtual uint32_t GetKeyCodeTrigger() = 0;
		/// @brief 何かキーが押されているか判定
		virtual bool IsAnyKeyPressed() = 0;

		// --- キーボード入力 ---
		/// @brief キーコードに対応するキーが押されているか
		virtual bool GetKeyPress(uint32_t keyCode) = 0;
		/// @brief キーコードに対応するキーが押された瞬間か
		virtual bool GetKeyTrigger(uint32_t keyCode) = 0;
		/// @brief キーコードに対応するキーが離された瞬間か
		virtual bool GetKeyRelease(uint32_t keyCode) = 0;
		/// @brief アクション名に対応するキーが押されているか
		virtual bool GetKeyPress(const std::string& actionName) = 0;
		/// @brief アクション名に対応するキーが押された瞬間か
		virtual bool GetKeyTrigger(const std::string& actionName) = 0;
		/// @brief アクション名に対応するキーが離された瞬間か
		virtual bool GetKeyRelease(const std::string& actionName) = 0;
		/// @brief 移動操作（WASD等）の方向ベクトルを計算して取得
		virtual MATH::Vector2 GetKeyMoveDir() = 0;

		// --- マウス入力 ---
		/// @brief マウスボタンが押されているか
		virtual bool GetMousePress(int8_t button) = 0;
		/// @brief マウスボタンが押された瞬間か
		virtual bool GetMouseTrigger(int8_t button) = 0;
		/// @brief マウスボタンが離された瞬間か
		virtual bool GetMouseRelease(int8_t button) = 0;
		/// @brief マウスの移動量を取得
		virtual MATH::Vector2 GetMouseMove() = 0;
		/// @brief マウスのスクリーン座標を取得
		virtual MATH::Vector2 GetMouseScreenPos() = 0;
		/// @brief マウスホイールの回転量を取得
		virtual float GetMouseWheelDir() = 0;

		// --- ゲームパッド入力 ---
		/// @brief パッドのボタンが押されているか
		virtual bool GetGamePadPress(uint16_t button) = 0;
		/// @brief パッドのボタンが押された瞬間か
		virtual bool GetGamePadTrigger(uint16_t button) = 0;
		/// @brief パッドのボタンが離された瞬間か
		virtual bool GetGamePadRelease(uint16_t button) = 0;
		/// @brief 左スティックの入力を取得
		virtual MATH::Vector2 GetGamePadLeftStickDir() = 0;
		/// @brief 右スティックの入力を取得
		virtual MATH::Vector2 GetGamePadRightStickDir() = 0;
	};
}  // namespace QFE::INPUT