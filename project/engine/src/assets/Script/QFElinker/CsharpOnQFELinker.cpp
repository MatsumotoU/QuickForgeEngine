#include "engine/include/assets/Script/QFElinker/CsharpOnQFELinker.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/input/InputInterface.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/Math/Transform.h"

using namespace QFE;

void QFE::CsharpOnQFELinker::GetTransformTranslate(uint32_t entityId, Vector3* outTranslate) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<Transform>(entityId)) {
        *outTranslate = entityManager->GetComponent<Transform>(entityId).translate;
    }
}

void QFE::CsharpOnQFELinker::SetTransformTranslate(uint32_t entityId, Vector3* inTranslate) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<Transform>(entityId)) {
        entityManager->GetComponent<Transform>(entityId).translate = *inTranslate;
    }
}

void QFE::CsharpOnQFELinker::GetTransformRotate(uint32_t entityId, Vector3* outRotate) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<Transform>(entityId)) {
        *outRotate = entityManager->GetComponent<Transform>(entityId).rotate;
    }
}

void QFE::CsharpOnQFELinker::SetTransformRotate(uint32_t entityId, Vector3* inRotate) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<Transform>(entityId)) {
        entityManager->GetComponent<Transform>(entityId).rotate = *inRotate;
    }
}

void QFE::CsharpOnQFELinker::GetTransformScale(uint32_t entityId, Vector3* outScale) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<Transform>(entityId)) {
        *outScale = entityManager->GetComponent<Transform>(entityId).scale;
    }
}

void QFE::CsharpOnQFELinker::SetTransformScale(uint32_t entityId, Vector3* inScale) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<Transform>(entityId)) {
        entityManager->GetComponent<Transform>(entityId).scale = *inScale;
    }
}

void QFE::CsharpOnQFELinker::Translate(uint32_t entityId, Vector3* translation) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<Transform>(entityId)) {
        entityManager->GetComponent<Transform>(entityId).translate += *translation;
    }
}

void QFE::CsharpOnQFELinker::Rotate(uint32_t entityId, Vector3* eulerAngles) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<Transform>(entityId)) {
        entityManager->GetComponent<Transform>(entityId).rotate += *eulerAngles;
    }
}

float QFE::CsharpOnQFELinker::GetDeltaTime() {
	return QFE::EngineGlobalValue::deltaTime;
}

bool QFE::CsharpOnQFELinker::IsKeyTrigger(MonoString* actionName) {
	bool result = false;
    char* utf8_message = mono_string_to_utf8(actionName);
	result = InputInterface::GetInstance()->GetKeyTrigger(utf8_message);
	mono_free(utf8_message);
	return result;
}

bool QFE::CsharpOnQFELinker::IsKeyPress(MonoString* actionName) {
    bool result = false;
    char* utf8_message = mono_string_to_utf8(actionName);
    result = InputInterface::GetInstance()->GetKeyPress(utf8_message);
    mono_free(utf8_message);
    return result;
}

bool QFE::CsharpOnQFELinker::IsKeyRelease(MonoString* actionName) {
    bool result = false;
    char* utf8_message = mono_string_to_utf8(actionName);
    result = InputInterface::GetInstance()->GetKeyRelease(utf8_message);
    mono_free(utf8_message);
    return result;
}

uint32_t QFE::CsharpOnQFELinker::CreateEntity(MonoString* className) {
	char* utf8_className = mono_string_to_utf8(className);
	uint32_t entityId = SceneManager::GetInstance()->RunTimeAddEntity(utf8_className);
	mono_free(utf8_className);
	return entityId;
}

void QFE::CsharpOnQFELinker::Native_Debug_Log(MonoString* message) {
    message;
#ifdef QFE_OPTIMIZE_OFF
    char* utf8_message = mono_string_to_utf8(message);
	DebugLogCsharp(utf8_message);
    mono_free(utf8_message);
#endif // QFE_OPTIMIZE_OFF
}
