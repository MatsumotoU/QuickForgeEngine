#pragma once
#include "input/interface/InputInterface.h"
#include <memory>
#include <string>
#include <cstdint>

namespace QFE::WINDOW { class IGameWindowManager; }

namespace QFE::FRAMEWORK {
	// ゲーム操作は従来どおりアクション名で指定する。
	// エディターの固定キーだけは設定に左右されないため、OS 固有のキーコードと分離する。
	enum class InputKey { LeftControl, Space };

	/// ウィンドウに対応する入力を生成する。非対応のウィンドウでは nullptr を返す。
	std::unique_ptr<QFE::INPUT::InputInterface> CreateInputInterface(QFE::WINDOW::IGameWindowManager& windowManager);

	// nullptr の場合は入力なしとして扱う。NetworkFramework と同様に型を共有し、
	// 入力の操作はこの窓口を通して OS ごとの実装に委ねる。
	bool GetKeyPress(QFE::INPUT::InputInterface* input, const std::string& action);
	// Trigger / Release はそれぞれ押した瞬間 / 離した瞬間だけ true になる。
	bool GetKeyTrigger(QFE::INPUT::InputInterface* input, const std::string& action);
	bool GetKeyRelease(QFE::INPUT::InputInterface* input, const std::string& action);
	bool GetKeyPress(QFE::INPUT::InputInterface* input, InputKey key);
	bool IsAnyKeyPressed(QFE::INPUT::InputInterface* input);
	QFE::MATH::Vector2 GetKeyMoveDir(QFE::INPUT::InputInterface* input);
	// ボタン番号は既存の入力設定と同じ値を受け取る。
	bool GetMousePress(QFE::INPUT::InputInterface* input, int8_t button);
	bool GetMouseTrigger(QFE::INPUT::InputInterface* input, int8_t button);
	bool GetMouseRelease(QFE::INPUT::InputInterface* input, int8_t button);
	QFE::MATH::Vector2 GetMouseMove(QFE::INPUT::InputInterface* input);
	QFE::MATH::Vector2 GetMouseScreenPos(QFE::INPUT::InputInterface* input);
	float GetMouseWheelDir(QFE::INPUT::InputInterface* input);
	// ゲームパッドのボタン値も既存の設定データとの互換性を維持する。
	bool GetGamePadPress(QFE::INPUT::InputInterface* input, uint16_t button);
	bool GetGamePadTrigger(QFE::INPUT::InputInterface* input, uint16_t button);
	bool GetGamePadRelease(QFE::INPUT::InputInterface* input, uint16_t button);
	QFE::MATH::Vector2 GetGamePadLeftStickDir(QFE::INPUT::InputInterface* input);
	QFE::MATH::Vector2 GetGamePadRightStickDir(QFE::INPUT::InputInterface* input);
}
