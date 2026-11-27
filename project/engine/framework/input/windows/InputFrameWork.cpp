#include "framework/input/InputFrameWork.h"
#include "input/WindowsInput.h"
#include "window/GameWindowManager.h"
#include <dinput.h>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

namespace {
	// 保存するキー名は OS 固有のコードと切り離し、入力実装内で変換する。
	const std::vector<std::pair<std::string, uint32_t>>& GameKeys() {
		static const std::vector<std::pair<std::string, uint32_t>> keys = {
			{"A", DIK_A}, {"B", DIK_B}, {"C", DIK_C}, {"D", DIK_D}, {"E", DIK_E}, {"F", DIK_F},
			{"G", DIK_G}, {"H", DIK_H}, {"I", DIK_I}, {"J", DIK_J}, {"K", DIK_K}, {"L", DIK_L},
			{"M", DIK_M}, {"N", DIK_N}, {"O", DIK_O}, {"P", DIK_P}, {"Q", DIK_Q}, {"R", DIK_R},
			{"S", DIK_S}, {"T", DIK_T}, {"U", DIK_U}, {"V", DIK_V}, {"W", DIK_W}, {"X", DIK_X},
			{"Y", DIK_Y}, {"Z", DIK_Z}, {"0", DIK_0}, {"1", DIK_1}, {"2", DIK_2}, {"3", DIK_3},
			{"4", DIK_4}, {"5", DIK_5}, {"6", DIK_6}, {"7", DIK_7}, {"8", DIK_8}, {"9", DIK_9},
			{"Up", DIK_UP}, {"Down", DIK_DOWN}, {"Left", DIK_LEFT}, {"Right", DIK_RIGHT},
			{"Space", DIK_SPACE}, {"Enter", DIK_RETURN}, {"Escape", DIK_ESCAPE}, {"Backspace", DIK_BACK},
			{"LeftShift", DIK_LSHIFT}, {"RightShift", DIK_RSHIFT}, {"LeftCtrl", DIK_LCONTROL},
			{"RightCtrl", DIK_RCONTROL}, {"Tab", DIK_TAB}
		};
		return keys;
	}
	uint32_t KeyCode(const std::string& name) {
		const auto& keys = GameKeys();
		const auto it = std::find_if(keys.begin(), keys.end(), [&](const auto& entry) { return entry.first == name; });
		return it == keys.end() ? UINT32_MAX : it->second;
	}
	bool ActionInput(QFE::INPUT::InputInterface* input, const std::vector<QFE::FRAMEWORK::GameInputAction>& actions,
		const std::string& name, int mode) {
		if (!input || name.empty()) return false;
		const auto action = std::find_if(actions.begin(), actions.end(), [&](const auto& item) { return item.name == name; });
		if (action == actions.end()) return false;
		bool current = false;
		bool previous = false;
		// 複数の入力が同じ操作に割り当てられていても、操作全体の押下・解放を判定する。
		const auto addState = [&](bool pressed, bool triggered, bool released) {
			current |= pressed;
			previous |= (pressed && !triggered) || released;
		};
		for (const auto& key : action->keys) {
			const uint32_t code = KeyCode(key);
			if (code == UINT32_MAX) continue;
			addState(input->GetKeyPress(code), input->GetKeyTrigger(code), input->GetKeyRelease(code));
		}
		if (action->mouseButton >= 0 && action->mouseButton <= 3) {
			const auto button = static_cast<int8_t>(action->mouseButton);
			addState(input->GetMousePress(button), input->GetMouseTrigger(button), input->GetMouseRelease(button));
		}
		if (action->gamepadButton > 0 && action->gamepadButton <= UINT16_MAX) {
			const auto button = static_cast<uint16_t>(action->gamepadButton);
			addState(input->GetGamePadPress(button), input->GetGamePadTrigger(button), input->GetGamePadRelease(button));
		}
		if (mode == 1) return current && !previous;
		if (mode == 2) return !current && previous;
		return current;
	}
}

std::vector<QFE::FRAMEWORK::GameInputAction> QFE::FRAMEWORK::DefaultGameInputActions() {
	return { {"Right", {"Right", "D"}}, {"Left", {"Left", "A"}}, {"Up", {"Up", "W"}},
		{"Down", {"Down", "S"}}, {"Slow", {"LeftShift"}}, {"Shot", {"Space"}},
		{"Menu", {"Escape"}}, {"Enter", {"Enter", "Space"}}, {"Cancel", {"Backspace", "Escape"}} };
}
std::vector<std::string> QFE::FRAMEWORK::GameInputKeyNames() {
	std::vector<std::string> names;
	for (const auto& entry : GameKeys()) names.push_back(entry.first);
	return names;
}
bool QFE::FRAMEWORK::LoadGameInputActions(const std::filesystem::path& projectFile, std::vector<GameInputAction>& actions) {
	actions = DefaultGameInputActions();
	try {
		std::ifstream input(projectFile, std::ios::binary);
		if (!input) return false;
		const auto project = nlohmann::json::parse(input);
		if (!project.is_object()) return false;
		const auto config = project.find("game_config");
		if (config == project.end()) return true;
		if (!config->is_object()) return false;
		const auto entries = config->find("inputActions");
		if (entries == config->end()) return true;
		if (!entries->is_array()) return false;
		std::vector<GameInputAction> loaded;
		for (const auto& entry : *entries) {
			if (!entry.is_object()) return false;
			GameInputAction action;
			action.name = entry.value("name", std::string{});
			action.keys = entry.value("keys", std::vector<std::string>{});
			action.mouseButton = entry.value("mouseButton", int32_t{ -1 });
			action.gamepadButton = entry.value("gamepadButton", uint32_t{ 0 });
			if (action.name.empty() || action.mouseButton < -1 || action.mouseButton > 3 || action.gamepadButton > UINT16_MAX ||
				std::any_of(loaded.begin(), loaded.end(), [&](const auto& item) { return item.name == action.name; }) ||
				std::any_of(action.keys.begin(), action.keys.end(), [](const auto& key) { return KeyCode(key) == UINT32_MAX; })) return false;
			loaded.push_back(std::move(action));
		}
		actions = std::move(loaded);
		return true;
	} catch (const nlohmann::json::exception&) { return false; }
}
bool QFE::FRAMEWORK::GetActionPress(INPUT::InputInterface* input, const std::vector<GameInputAction>& actions, const std::string& name) { return ActionInput(input, actions, name, 0); }
bool QFE::FRAMEWORK::GetActionTrigger(INPUT::InputInterface* input, const std::vector<GameInputAction>& actions, const std::string& name) { return ActionInput(input, actions, name, 1); }
bool QFE::FRAMEWORK::GetActionRelease(INPUT::InputInterface* input, const std::vector<GameInputAction>& actions, const std::string& name) { return ActionInput(input, actions, name, 2); }

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
