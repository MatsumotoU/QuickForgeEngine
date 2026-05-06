#include "engine/include/assets/Script/QFElinker/CsharpOnQFELinker.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/input/InputInterface.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/audio/AudioInterface.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/renderer/ModelRenderer.h"

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
	return EngineGlobalValue::deltaTime;
}

void QFE::CsharpOnQFELinker::GetKeyMoveDir(Vector2* outDir) {
	if (outDir) {
		*outDir = InputInterface::GetInstance()->GetKeyMoveDir();
	}
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

bool QFE::CsharpOnQFELinker::GetMousePress(int8_t button) {
	return InputInterface::GetInstance()->GetMousePress(button);
}

bool QFE::CsharpOnQFELinker::GetMouseTrigger(int8_t button) {
	return InputInterface::GetInstance()->GetMouseTrigger(button);
}

bool QFE::CsharpOnQFELinker::GetMouseRelease(int8_t button) {
	return InputInterface::GetInstance()->GetMouseRelease(button);
}

void QFE::CsharpOnQFELinker::GetMouseScreenPos(Vector2* outPos) {
	if (outPos) {
		*outPos = InputInterface::GetInstance()->GetMouseScreenPos();
	}
}

void QFE::CsharpOnQFELinker::GetMouseMoveDir(Vector2* outDir) {
	if (outDir) {
		*outDir = InputInterface::GetInstance()->GetMouseMove();
	}
}

float QFE::CsharpOnQFELinker::GetMouseWheelDir() {
	return InputInterface::GetInstance()->GetMouseWheelDir();
}

uint32_t QFE::CsharpOnQFELinker::LoadSound(MonoString* soundName) {
	char* utf8_soundName = mono_string_to_utf8(soundName);
	uint32_t handle = AssetManager::GetInstance()->LoadAudio(utf8_soundName);
	mono_free(utf8_soundName);
	return handle;
}

uint32_t QFE::CsharpOnQFELinker::PlayQFESound(uint32_t soundHandle, bool isLoop, float volume) {
	return AudioInterface::GetInstance()->PlaySoundForAudioData(soundHandle, isLoop, volume);
}

void QFE::CsharpOnQFELinker::StopSound(uint32_t playHandle) {
	AudioInterface::GetInstance()->StopSound(playHandle);
}

uint32_t QFE::CsharpOnQFELinker::GetEntity(MonoString* entityName)
{
	return SceneManager::GetInstance()-
}

uint32_t QFE::CsharpOnQFELinker::CreateEntity(MonoString* entityName) {
	char* utf8_className = mono_string_to_utf8(entityName);
	uint32_t entityId = SceneManager::GetInstance()->RunTimeAddEntity(utf8_className);
	mono_free(utf8_className);
	return entityId;
}

void QFE::CsharpOnQFELinker::LoadScene(MonoString* sceneName) {
	char* utf8_sceneName = mono_string_to_utf8(sceneName);
	SceneManager::GetInstance()->RunTimeSwapScene(utf8_sceneName);
	mono_free(utf8_sceneName);
}

void QFE::CsharpOnQFELinker::ChangeModel(uint32_t entityId, MonoString* modelName) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<ModelHandle>(entityId)) { return; }
	
	char* utf8_modelName = mono_string_to_utf8(modelName);
	ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
	modelHandle.modelName = utf8_modelName;
	modelHandle.handle = AssetManager::GetInstance()->LoadModel(utf8_modelName);
	mono_free(utf8_modelName);
}

void QFE::CsharpOnQFELinker::ChangeMesh(uint32_t entityId, MonoString* meshName) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	if (!entityManager->HasComponent<ModelHandle>(entityId)) { return; }

	AssetManager* assetManager = AssetManager::GetInstance();
	ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
	ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
	
	if (modelData->meshRenderDataHandles.size() == 0) { return; }
	
	char* utf8_meshName = mono_string_to_utf8(meshName);
	modelData->meshRenderDataHandles[0].vertexBufferHandle = assetManager->LoadModelMesh(utf8_meshName);
	mono_free(utf8_meshName);
}

void QFE::CsharpOnQFELinker::Native_Debug_Log(MonoString* message) {
    message;
#ifdef QFE_OPTIMIZE_OFF
    char* utf8_message = mono_string_to_utf8(message);
	DebugLogCsharp(utf8_message);
    mono_free(utf8_message);
#endif // QFE_OPTIMIZE_OFF
}
