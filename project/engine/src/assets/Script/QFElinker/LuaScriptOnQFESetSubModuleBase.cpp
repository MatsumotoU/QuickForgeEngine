#include "LuaScriptOnQFESetSubModuleBase.h"
#include "Input/InputInterface.h"
#include "Audio/AudioInterface.h"
#include "Assets/AudioSource/AudioSourceManager.h"
#include "Assets/AssetManager.h"

void QFE::Script::Base::LuaScriptOnQFESetSubModuleBase(sol::state* luaState) {
	sol::table qfe = luaState->get<sol::table>("QFE");
	sol::table input = qfe.create_named("Input");
	sol::table audio = qfe.create_named("Audio");

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
	// ゲームパッド
	input.set_function("GetGamePadPress", [inputManager](uint16_t button) {
		return inputManager->GetGamePadPress(button);
		});
	input.set_function("GetGamePadTrigger", [inputManager](uint16_t button) {
		return inputManager->GetGamePadTrigger(button);
		});
	input.set_function("GetGamePadRelease", [inputManager](uint16_t button) {
		return inputManager->GetGamePadRelease(button);
		});
	input.set_function("GetGamePadLeftStickDir", [inputManager]() {
		return inputManager->GetGamePadLeftStickDir();
		});
	input.set_function("GetGamePadRightStickDir", [inputManager]() {
		return inputManager->GetGamePadRightStickDir();
		});

	// オーディオ
	audio.set_function("LoadSound", [](const std::string& soundName) {
		return AssetManager::GetInstance()->LoadAudio(soundName);
		});
	audio.set_function("PlaySound", [](const uint32_t& soundHandle, bool isLoop,float volume) {
		return AudioInterface::GetInstance()->PlaySoundForAudioData(soundHandle, isLoop, volume);
		});
	audio.set_function("StopSound", [](const uint32_t& playHandle) {
		AudioInterface::GetInstance()->StopSound(playHandle);
		});
}
