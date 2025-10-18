#include "LuaScriptOnQFESetSubModuleBase.h"
#include "Input/InputInterface.h"

void QFE::Script::Base::LuaScriptOnQFESetSubModuleBase(sol::state* luaState) {
	sol::table qfe = luaState->get<sol::table>("QFE");
	sol::table input = qfe.create_named("Input");

	// 入力
	InputInterface* inputManager = InputInterface::GetInstance();
	input.set_function("GetKeyMoveDir", [inputManager]() {
		return inputManager->GetKeyMoveDir();
		});
	// キーボード
	input.set_function("GetKeyPress", [inputManager](std::string actionName) {
		return inputManager->GetKeyPress(actionName);
		});
	input.set_function("GetKeyTrigger", [inputManager](std::string actionName) {
		return inputManager->GetKeyTrigger(actionName);
		});
	input.set_function("GetKeyRelease", [inputManager](std::string actionName) {
		return inputManager->GetKeyRelease(actionName);
		});
	// マウス
	input.set_function("GetMousePress", [inputManager](int8_t button) {
		return inputManager->GetMousePress(button);
		});
	input.set_function("GetMouseTrigger", [inputManager](int8_t button) {
		return inputManager->GetMouseTrigger(button);
		});
	input.set_function("GetMouseRelease", [inputManager](int8_t button) {
		return inputManager->GetMouseRelease(button);
		});
	input.set_function("GetMouseScreenPos", [inputManager]() {
		return inputManager->GetMouseScreenPos();
		});
	input.set_function("GetMouseMoveDir", [inputManager]() {
		return inputManager->GetMouseMove();
		});
	input.set_function("GetMouseWheelDir", [inputManager]() {
		return inputManager->GetMouseWheelDir();
		});
}
