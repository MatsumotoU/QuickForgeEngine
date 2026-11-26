#include "framework/input/InputFrameWork.h"
#include "input/WindowsInput.h"
#include "window/GameWindowManager.h"
#include <dinput.h>

std::unique_ptr<QFE::INPUT::InputInterface> QFE::FRAMEWORK::CreateInputInterface(
	QFE::WINDOW::IGameWindowManager& windowManager) {
	// Windows 固有のハンドルはここで取得し、共通ヘッダーに公開しない。
	const auto* windowsManager = dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
	if (!windowsManager) return nullptr;
	const HWND window = windowsManager->GetMainWindow();
	if (!window) return nullptr;
	const auto instance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(window, GWLP_HINSTANCE));
	if (!instance) return nullptr;
	auto input = std::make_unique<QFE::INPUT::WindowsInput>();
	input->Initialize(window, instance);
	return input;
}

// 未生成の入力は押されていないものとして扱い、呼び出し側での分岐を揃える。
bool QFE::FRAMEWORK::GetKeyPress(INPUT::InputInterface* input, const std::string& action) { return input && input->GetKeyPress(action); }
bool QFE::FRAMEWORK::GetKeyTrigger(INPUT::InputInterface* input, const std::string& action) { return input && input->GetKeyTrigger(action); }
bool QFE::FRAMEWORK::GetKeyRelease(INPUT::InputInterface* input, const std::string& action) { return input && input->GetKeyRelease(action); }
bool QFE::FRAMEWORK::IsAnyKeyPressed(INPUT::InputInterface* input) { return input && input->IsAnyKeyPressed(); }
QFE::MATH::Vector2 QFE::FRAMEWORK::GetKeyMoveDir(INPUT::InputInterface* input) { return input ? input->GetKeyMoveDir() : MATH::Vector2{}; }
bool QFE::FRAMEWORK::GetKeyPress(INPUT::InputInterface* input, InputKey key) {
	if (!input) return false;
	// 既存のカメラ操作と同じ物理キーを使い、DirectInput の定数はここに閉じ込める。
	switch (key) {
	case InputKey::LeftControl: return input->GetKeyPress(DIK_LCONTROL);
	case InputKey::Space: return input->GetKeyPress(DIK_SPACE);
	default: return false;
	}
}
bool QFE::FRAMEWORK::GetMousePress(INPUT::InputInterface* input, int8_t button) { return input && input->GetMousePress(button); }
bool QFE::FRAMEWORK::GetMouseTrigger(INPUT::InputInterface* input, int8_t button) { return input && input->GetMouseTrigger(button); }
bool QFE::FRAMEWORK::GetMouseRelease(INPUT::InputInterface* input, int8_t button) { return input && input->GetMouseRelease(button); }
QFE::MATH::Vector2 QFE::FRAMEWORK::GetMouseMove(INPUT::InputInterface* input) { return input ? input->GetMouseMove() : MATH::Vector2{}; }
QFE::MATH::Vector2 QFE::FRAMEWORK::GetMouseScreenPos(INPUT::InputInterface* input) { return input ? input->GetMouseScreenPos() : MATH::Vector2{}; }
float QFE::FRAMEWORK::GetMouseWheelDir(INPUT::InputInterface* input) { return input ? input->GetMouseWheelDir() : 0.0f; }
bool QFE::FRAMEWORK::GetGamePadPress(INPUT::InputInterface* input, uint16_t button) { return input && input->GetGamePadPress(button); }
bool QFE::FRAMEWORK::GetGamePadTrigger(INPUT::InputInterface* input, uint16_t button) { return input && input->GetGamePadTrigger(button); }
bool QFE::FRAMEWORK::GetGamePadRelease(INPUT::InputInterface* input, uint16_t button) { return input && input->GetGamePadRelease(button); }
QFE::MATH::Vector2 QFE::FRAMEWORK::GetGamePadLeftStickDir(INPUT::InputInterface* input) { return input ? input->GetGamePadLeftStickDir() : MATH::Vector2{}; }
QFE::MATH::Vector2 QFE::FRAMEWORK::GetGamePadRightStickDir(INPUT::InputInterface* input) { return input ? input->GetGamePadRightStickDir() : MATH::Vector2{}; }
