#include "CsharpOnQFELinker.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

#include "Assets/AssetManager.h"
#include "Core/Entity/EntityManager.h"
#include "Input/InputInterface.h"

#include "Core/Math/Transform.h"

void CsharpOnQFELinker::GetTransformTranslate(uint32_t entityId, Vector3* outTranslate) {
    if (AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(entityId)) {
        *outTranslate = AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(entityId).translate;
    }
}

void CsharpOnQFELinker::SetTransformTranslate(uint32_t entityId, Vector3* inTranslate) {
    if (AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(entityId)) {
        AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(entityId).translate = *inTranslate;
    }
}

void CsharpOnQFELinker::GetTransformRotate(uint32_t entityId, Vector3* outRotate) {
    if (AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(entityId)) {
        *outRotate = AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(entityId).rotate;
    }
}

void CsharpOnQFELinker::SetTransformRotate(uint32_t entityId, Vector3* inRotate) {
    if (AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(entityId)) {
        AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(entityId).rotate = *inRotate;
    }
}

void CsharpOnQFELinker::GetTransformScale(uint32_t entityId, Vector3* outScale) {
    if (AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(entityId)) {
        *outScale = AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(entityId).scale;
    }
}

void CsharpOnQFELinker::SetTransformScale(uint32_t entityId, Vector3* inScale) {
    if (AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(entityId)) {
        AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(entityId).scale = *inScale;
    }
}

void CsharpOnQFELinker::Translate(uint32_t entityId, Vector3* translation) {
    if (AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(entityId)) {
        AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(entityId).translate += *translation;
    }
}

void CsharpOnQFELinker::Rotate(uint32_t entityId, Vector3* eulerAngles) {
    if (AssetManager::GetInstance()->GetEntityManager()->HasComponent<Transform>(entityId)) {
        AssetManager::GetInstance()->GetEntityManager()->GetComponent<Transform>(entityId).rotate += *eulerAngles;
    }
}

bool CsharpOnQFELinker::IsKeyTrigger(MonoString* actionName) {
	bool result = false;
    char* utf8_message = mono_string_to_utf8(actionName);
	result = InputInterface::GetInstance()->GetKeyTrigger(utf8_message);
	mono_free(utf8_message);
	return result;
}

bool CsharpOnQFELinker::IsKeyPress(MonoString* actionName) {
    bool result = false;
    char* utf8_message = mono_string_to_utf8(actionName);
    result = InputInterface::GetInstance()->GetKeyPress(utf8_message);
    mono_free(utf8_message);
    return result;
}

bool CsharpOnQFELinker::IsKeyRelease(MonoString* actionName) {
    bool result = false;
    char* utf8_message = mono_string_to_utf8(actionName);
    result = InputInterface::GetInstance()->GetKeyRelease(utf8_message);
    mono_free(utf8_message);
    return result;
}

void CsharpOnQFELinker::Native_Debug_Log(MonoString* message) {
    char* utf8_message = mono_string_to_utf8(message);
	DebugLogCsharp(utf8_message);
    mono_free(utf8_message);
}
