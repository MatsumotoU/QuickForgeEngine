#include "framework/input/InputFrameWork.h"
#include "components/InputControlComponent.h"

#include <cassert>
#include <array>
#include <fstream>
#include <chrono>

// 実際の機器を使わず、同じ操作に割り当てた複数入力の切り替えを再現する。
class FakeInput final : public QFE::INPUT::InputInterface {
public:
	std::array<bool, 256> keys{}, previousKeys{};
	std::array<bool, 4> mouse{}, previousMouse{};
	uint16_t pad = 0, previousPad = 0;
	void Update() override {}
	void EndFrame() override { previousKeys = keys; previousMouse = mouse; previousPad = pad; }
	uint32_t GetKeyCodeTrigger() override { return 0; }
	bool IsAnyKeyPressed() override { return false; }
	bool GetKeyPress(uint32_t key) override { return keys.at(key); }
	bool GetKeyTrigger(uint32_t key) override { return keys.at(key) && !previousKeys.at(key); }
	bool GetKeyRelease(uint32_t key) override { return !keys.at(key) && previousKeys.at(key); }
	bool GetKeyPress(const std::string&) override { return false; }
	bool GetKeyTrigger(const std::string&) override { return false; }
	bool GetKeyRelease(const std::string&) override { return false; }
	QFE::MATH::Vector2 GetKeyMoveDir() override { return {}; }
	bool GetMousePress(int8_t button) override { return mouse.at(button); }
	bool GetMouseTrigger(int8_t button) override { return mouse.at(button) && !previousMouse.at(button); }
	bool GetMouseRelease(int8_t button) override { return !mouse.at(button) && previousMouse.at(button); }
	QFE::MATH::Vector2 GetMouseMove() override { return {}; }
	QFE::MATH::Vector2 GetMouseScreenPos() override { return {}; }
	float GetMouseWheelDir() override { return 0.0f; }
	bool GetGamePadPress(uint16_t button) override { return (pad & button) != 0; }
	bool GetGamePadTrigger(uint16_t button) override { return (pad & button) != 0 && (previousPad & button) == 0; }
	bool GetGamePadRelease(uint16_t button) override { return (pad & button) == 0 && (previousPad & button) != 0; }
	QFE::MATH::Vector2 GetGamePadLeftStickDir() override { return {}; }
	QFE::MATH::Vector2 GetGamePadRightStickDir() override { return {}; }
};

int main() {
	using namespace QFE::FRAMEWORK;
	FakeInput input;
	const std::vector<GameInputAction> actions = { {"Test", {"A", "B"}, 1, 0x1000} };
	assert(!GetActionPress(&input, actions, "Test"));
	input.keys[0x1e] = true; // DirectInput の A
	assert(GetActionTrigger(&input, actions, "Test"));
	input.EndFrame();
	input.mouse[1] = true;
	input.keys[0x1e] = false;
	assert(GetActionPress(&input, actions, "Test"));
	assert(!GetActionTrigger(&input, actions, "Test") && !GetActionRelease(&input, actions, "Test"));
	input.EndFrame();
	input.mouse[1] = false;
	input.pad = 0x1000;
	assert(GetActionPress(&input, actions, "Test") && !GetActionRelease(&input, actions, "Test"));
	input.EndFrame();
	input.pad = 0;
	assert(GetActionRelease(&input, actions, "Test"));
	assert(!GetActionPress(&input, actions, "Missing"));

	QFE::COMPONENTS::InputControlComponent control;
	control.actions = {"Shot"};
	assert(!control.BlocksAction("Shot"));
	control.inputEnabled = false;
	assert(control.BlocksAction("Shot") && control.BlocksAction("Move"));
	control.affectAllActions = false;
	assert(control.BlocksAction("Shot") && !control.BlocksAction("Move"));

	// 古いプロジェクトの初期割り当てと、保存した操作設定の読み込みを確認する。
	const auto suffix = std::chrono::steady_clock::now().time_since_epoch().count();
	const auto path = std::filesystem::temp_directory_path() /
		("qfe-game-input-actions-test-" + std::to_string(suffix) + ".json");
	{ std::ofstream file(path); file << R"({"game_config":{}})"; }
	std::vector<GameInputAction> loaded;
	assert(LoadGameInputActions(path, loaded) && loaded.size() == DefaultGameInputActions().size());
	{ std::ofstream file(path); file << R"({"game_config":{"inputActions":[{"name":"Jump","keys":["A","Space"],"mouseButton":1,"gamepadButton":4096}]}})"; }
	assert(LoadGameInputActions(path, loaded));
	assert(loaded.size() == 1 && loaded[0].name == "Jump" && loaded[0].keys.size() == 2 &&
		loaded[0].mouseButton == 1 && loaded[0].gamepadButton == 4096);
	{ std::ofstream file(path); file << R"({"game_config":{"inputActions":[{"name":"Jump","keys":["Unknown"]}]}})"; }
	assert(!LoadGameInputActions(path, loaded) && loaded.size() == DefaultGameInputActions().size());
	std::filesystem::remove(path);
}
