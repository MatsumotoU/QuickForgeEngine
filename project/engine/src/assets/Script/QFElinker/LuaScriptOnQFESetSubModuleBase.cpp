#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSubModuleBase.h"
#include "engine/include/input/InputInterface.h"
#include "engine/include/audio/AudioInterface.h"
#include "engine/include/assets/AudioSource/AudioSourceManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"

void QFE::Script::Base::LuaScriptOnQFESetSubModuleBase(sol::state* luaState, EntityManager* entityManager) {
	sol::table qfe = luaState->get<sol::table>("QFE");
	sol::table input = qfe.create_named("Input");
	sol::table audio = qfe.create_named("Audio");

	// 蜈･蜉・
	InputInterface* inputManager = InputInterface::GetInstance();
	input.set_function("GetKeyMoveDir", [inputManager]() {
		return inputManager->GetKeyMoveDir();
		});
	// 繧ｭ繝ｼ繝懊・繝・
	input.set_function("GetKeyPress", [inputManager](std::string actionName) {
		return inputManager->GetKeyPress(actionName);
		});
	input.set_function("GetKeyTrigger", [inputManager](std::string actionName) {
		return inputManager->GetKeyTrigger(actionName);
		});
	input.set_function("GetKeyRelease", [inputManager](std::string actionName) {
		return inputManager->GetKeyRelease(actionName);
		});
	// 繝槭え繧ｹ
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
	// 繧ｲ繝ｼ繝繝代ャ繝・
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

	// 繧ｪ繝ｼ繝・ぅ繧ｪ
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
