#include "engine/include/core/Bridge/EditorEngineBridge.h"
#ifdef  QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif //  QFE_OPTIMIZE_OFF

namespace QFE {
	std::function<void(void)> EditorEngineBridge::AddEmptyEntity = nullptr;
	std::function<void(const std::string&)> EditorEngineBridge::AddEntityFromFile = nullptr;
	std::function<void(const std::string&)> EditorEngineBridge::AddModelEntity = nullptr;
	std::function<void(const std::string&)> EditorEngineBridge::AddSpriteEntity = nullptr;
	std::function<void(const std::string&, unsigned int)> EditorEngineBridge::AddParticleEmitterEntity = nullptr;
	std::function<void(void)> EditorEngineBridge::AddCameraEntity = nullptr;
	std::function<void(unsigned int)> EditorEngineBridge::CopyEntity = nullptr;
	std::function<void(unsigned int, std::string)> EditorEngineBridge::SaveEntity = nullptr;
	std::function<void(unsigned int)> EditorEngineBridge::DeleteEntity = nullptr;
	std::function<void(unsigned int, unsigned int)> EditorEngineBridge::ParentChild = nullptr;
}

void QFE::EditorEngineBridge::CheckFunctionRegistration() {
	if (!AddEmptyEntity) {
		DebugLog("EditorEngineBridge::AddEmptyEntity is not registered.");
	}
	if (!AddEntityFromFile) {
		DebugLog("EditorEngineBridge::AddEntityFromFile is not registered.");
	}
	if (!AddModelEntity) {
		DebugLog("EditorEngineBridge::AddModelEntity is not registered.");
	}
	if (!AddSpriteEntity) {
		DebugLog("EditorEngineBridge::AddSpriteEntity is not registered.");
	}
	if (!AddParticleEmitterEntity) {
		DebugLog("EditorEngineBridge::AddParticleEmitterEntity is not registered.");
	}
	if (!AddCameraEntity) {
		DebugLog("EditorEngineBridge::AddCameraEntity is not registered.");
	}
	if (!CopyEntity) {
		DebugLog("EditorEngineBridge::CopyEntity is not registered.");
	}
	if (!SaveEntity) {
		DebugLog("EditorEngineBridge::SaveEntity is not registered.");
	}
	if (!DeleteEntity) {
		DebugLog("EditorEngineBridge::DeleteEntity is not registered.");
	}
	if (!ParentChild) {
		DebugLog("EditorEngineBridge::ParentChild is not registered.");
	}
}

void QFE::EditorEngineBridge::ClearFunctionRegistration() {
	AddEmptyEntity = nullptr;
	AddEntityFromFile = nullptr;
	AddModelEntity = nullptr;
	AddSpriteEntity = nullptr;
	AddParticleEmitterEntity = nullptr;
	AddCameraEntity = nullptr;
	CopyEntity = nullptr;
	SaveEntity = nullptr;
	DeleteEntity = nullptr;
	ParentChild = nullptr;
}
