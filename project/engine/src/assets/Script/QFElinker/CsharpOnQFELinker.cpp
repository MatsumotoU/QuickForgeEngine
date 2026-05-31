#include "engine/include/assets/Script/QFElinker/CsharpOnQFELinker.h"

#include "engine/include/core/EngineDefines.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/input/InputInterface.h"
#include "engine/include/collider/ColliderManager.h"
#include "engine/include/graphic/PostEffect/RenderingPostprocess.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/Math/Transform.h"
#include "engine/include/audio/AudioInterface.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/renderer/ModelRenderer.h"

#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/core/Math/TransformComponent.h"
#include "engine/include/assets/Script/Data/CsharpComponent.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"

using namespace QFE;

void QFE::CsharpOnQFELinker::GetTransforms(MonoArray* entityIds, MonoArray* transforms, uint32_t* count)
{
	try
	{
		EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();

		if (entityManager->HasComponentStrage<TransformComponent>()) {
			auto& transformStorage = entityManager->GetComponentStrage<TransformComponent>();

			// 1. 実際にコピーする数を決定（C#側の配列サイズを超えないように！）
			uint32_t csharpBufLen = static_cast<uint32_t>(mono_array_length(entityIds));
			uint32_t nativeCount = static_cast<uint32_t>(transformStorage.size());
			uint32_t copyCount = (csharpBufLen < nativeCount) ? csharpBufLen : nativeCount;

			// 2. C#配列の「生ポインタ」を取得
			uint32_t* dstIds = mono_array_addr(entityIds, uint32_t, 0);
			EulerTransform* dstTrans = mono_array_addr(transforms, EulerTransform, 0);

			// 3. C++側のデータをC#のメモリへコピー
			std::vector<uint32_t> entityIdsVec = transformStorage.GetEntityIds();
			std::vector<TransformComponent> transformsVec = transformStorage.GetComponents();

			for(uint32_t i = 0; i < copyCount; ++i) {
				dstIds[i] = entityIdsVec[i];

				dstTrans[i].translate = transformsVec[i].transform.translate;
				dstTrans[i].rotate = transformsVec[i].transform.rotate;
				dstTrans[i].scale = transformsVec[i].transform.scale;
			}

			// 4. C#側に「何個書いたか」を教える
			*count = copyCount;
		}
		else {
			*count = 0;
		}
	}
	catch (const std::exception& e)
	{
		QFE_REPORT_SYSTEM_ERROR(std::string("Exception in GetTransforms: ") + e.what(), SystemError::Abort);
	}
}

void QFE::CsharpOnQFELinker::SetTransforms(MonoArray* entityIds, MonoArray* transforms, uint32_t count)
{
	try
	{
		EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
		if (entityManager->HasComponentStrage<TransformComponent>()) {
			// 1. C#側の配列サイズを確認（C++側のデータ数を超えないように！）
			uint32_t csharpBufLen = static_cast<uint32_t>(mono_array_length(entityIds));
			uint32_t copyCount = (csharpBufLen < count) ? csharpBufLen : count;

			// 2. C#配列の「生ポインタ」を取得
			uint32_t* srcIds = mono_array_addr(entityIds, uint32_t, 0);
			EulerTransform* srcTrans = mono_array_addr(transforms, EulerTransform, 0);

			// 3. C#のデータをC++側へコピー
			for (uint32_t i = 0; i < copyCount; ++i) {
				uint32_t entityId = srcIds[i];
				const EulerTransform& transform = srcTrans[i];
				if (entityManager->HasComponent<TransformComponent>(entityId)) {
					entityManager->GetComponent<TransformComponent>(entityId).transform = transform;
				}
			}
		}
	}
	catch (const std::exception& e)
	{
		QFE_REPORT_SYSTEM_ERROR(std::string("Exception in SetTransforms: ") + e.what(), SystemError::Abort);
	}
}

void QFE::CsharpOnQFELinker::GetTransformTranslate(uint32_t entityId, Vector3* outTranslate) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<TransformComponent>(entityId)) {
        *outTranslate = entityManager->GetComponent<TransformComponent>(entityId).transform.translate;
    }
}

void QFE::CsharpOnQFELinker::SetTransformTranslate(uint32_t entityId, Vector3* inTranslate) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<TransformComponent>(entityId)) {
        entityManager->GetComponent<TransformComponent>(entityId).transform.translate = *inTranslate;
    }
}

void QFE::CsharpOnQFELinker::GetTransformRotate(uint32_t entityId, Vector3* outRotate) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<TransformComponent>(entityId)) {
        *outRotate = entityManager->GetComponent<TransformComponent>(entityId).transform.rotate;
    }
}

void QFE::CsharpOnQFELinker::SetTransformRotate(uint32_t entityId, Vector3* inRotate) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<TransformComponent>(entityId)) {
        entityManager->GetComponent<TransformComponent>(entityId).transform.rotate = *inRotate;
    }
}

void QFE::CsharpOnQFELinker::GetTransformScale(uint32_t entityId, Vector3* outScale) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<TransformComponent>(entityId)) {
        *outScale = entityManager->GetComponent<TransformComponent>(entityId).transform.scale;
    }
}

void QFE::CsharpOnQFELinker::SetTransformScale(uint32_t entityId, Vector3* inScale) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<TransformComponent>(entityId)) {
        entityManager->GetComponent<TransformComponent>(entityId).transform.scale = *inScale;
    }
}

void QFE::CsharpOnQFELinker::Translate(uint32_t entityId, Vector3* translation) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<TransformComponent>(entityId)) {
        entityManager->GetComponent<TransformComponent>(entityId).transform.translate += *translation;
    }
}

void QFE::CsharpOnQFELinker::Rotate(uint32_t entityId, Vector3* eulerAngles) {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
    if (entityManager->HasComponent<TransformComponent>(entityId)) {
        entityManager->GetComponent<TransformComponent>(entityId).transform.rotate += *eulerAngles;
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

void QFE::CsharpOnQFELinker::GetEntityTags(MonoArray* entityIds, MonoArray* tags, uint32_t* count)
{
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	std::vector<std::string> entityTags;
	std::vector<uint32_t> entityIdsVec;
	entityManager->GetComponentStrage<SceneObjectData>().Each([&](uint32_t entityId, const SceneObjectData& data) {
		if (!data.tag.empty()) {
			entityIdsVec.push_back(entityId);
			entityTags.push_back(data.tag);
		}
		});
	// 1. C#側の配列サイズを確認（C++側のデータ数を超えないように！）
	uint32_t csharpBufLen = static_cast<uint32_t>(mono_array_length(entityIds));
	uint32_t nativeCount = static_cast<uint32_t>(entityIdsVec.size());
	uint32_t copyCount = (csharpBufLen < nativeCount) ? csharpBufLen : nativeCount;
	// 2. C#配列の「生ポインタ」を取得
	uint32_t* dstEntityIds = mono_array_addr(entityIds, uint32_t, 0);
	MonoString** dstTags = mono_array_addr(tags, MonoString*, 0);
	// 3. C++側のデータをC#のメモリへコピー
	for (uint32_t i = 0; i < copyCount; ++i) {
		dstEntityIds[i] = entityIdsVec[i];
		dstTags[i] = mono_string_new(mono_domain_get(), entityTags[i].c_str());
	}
	// 4. C#側に「何個書いたか」を教える
	*count = copyCount;
}

uint32_t QFE::CsharpOnQFELinker::GetEntityFromName(MonoString* entityName) {
	std::string utf8_entityName = mono_string_to_utf8(entityName);
	uint32_t entityId = SceneManager::GetInstance()->GetEntityByName(utf8_entityName.c_str());
	return entityId;
}

uint32_t QFE::CsharpOnQFELinker::CreateEntity(MonoString* entityName, EulerTransform transform, uint32_t changeCount)
{
	if (!entityName) {
		QFE_REPORT_USER_ERROR("Entity name is null.", UserError::DeveloperError);
		return UINT32_MAX;
	}

	char* utf8_className = mono_string_to_utf8(entityName);
	if (!utf8_className) {
		QFE_REPORT_USER_ERROR("Failed to convert entity name to UTF-8.", UserError::DeveloperError);
		return UINT32_MAX;
	}

	uint32_t entityId = SceneManager::GetInstance()->AddEntity(utf8_className);
	mono_free(utf8_className);

	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<TransformComponent>(entityId)) {
		TransformComponent& transformComp = entityManager->GetComponent<TransformComponent>(entityId);

		if (changeCount > 0) {
			transformComp.transform.translate = transform.translate;
		}
		if (changeCount > 1) {
			transformComp.transform.rotate = transform.rotate;
		}
		if (changeCount > 2) {
			transformComp.transform.scale = transform.scale;
		}
	}

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

void QFE::CsharpOnQFELinker::DeleteEntity(uint32_t entityId) {
	SceneManager::GetInstance()->DeleteEntity(entityId);
}

void QFE::CsharpOnQFELinker::GetCollisionEnterEntityIds(MonoArray* aIds, MonoArray* bIds, uint32_t* count)
{
	ColliderManager* colliderManager = ColliderManager::GetInstance();
	uint32_t collisionCount = static_cast<uint32_t>(colliderManager->collisionEnterEntityIds_.size());

	// 1. C#側の配列サイズを確認（C++側のデータ数を超えないように！）
	uint32_t csharpBufLen = static_cast<uint32_t>(mono_array_length(aIds));
	uint32_t copyCount = (csharpBufLen < collisionCount) ? csharpBufLen : collisionCount;

	// 2. C#配列の「生ポインタ」を取得
	uint32_t* dstAIds = mono_array_addr(aIds, uint32_t, 0);
	uint32_t* dstBIds = mono_array_addr(bIds, uint32_t, 0);

	// 3. C++側のデータをC#のメモリへコピー
	for (uint32_t i = 0; i < copyCount; ++i) {
		dstAIds[i] = colliderManager->collisionEnterEntityIds_[i].first;
		dstBIds[i] = colliderManager->collisionEnterEntityIds_[i].second;
	}

	// 4. C#側に「何個書いたか」を教える
	*count = copyCount;
}

void QFE::CsharpOnQFELinker::GetCollisionStayEntityIds(MonoArray* aIds, MonoArray* bIds, uint32_t* count)
{
	ColliderManager* colliderManager = ColliderManager::GetInstance();
	uint32_t collisionCount = static_cast<uint32_t>(colliderManager->collisionStayEntityIds_.size());

	// 1. C#側の配列サイズを確認（C++側のデータ数を超えないように！）
	uint32_t csharpBufLen = static_cast<uint32_t>(mono_array_length(aIds));
	uint32_t copyCount = (csharpBufLen < collisionCount) ? csharpBufLen : collisionCount;

	// 2. C#配列の「生ポインタ」を取得
	uint32_t* dstAIds = mono_array_addr(aIds, uint32_t, 0);
	uint32_t* dstBIds = mono_array_addr(bIds, uint32_t, 0);

	// 3. C++側のデータをC#のメモリへコピー
	for (uint32_t i = 0; i < copyCount; ++i) {
		dstAIds[i] = colliderManager->collisionStayEntityIds_[i].first;
		dstBIds[i] = colliderManager->collisionStayEntityIds_[i].second;
	}

	// 4. C#側に「何個書いたか」を教える
	*count = copyCount;
}

void QFE::CsharpOnQFELinker::Native_Debug_Log(MonoString* message) {
    message;
#ifdef QFE_OPTIMIZE_OFF
    char* utf8_message = mono_string_to_utf8(message);
	DebugLogCsharp(utf8_message);
    mono_free(utf8_message);
#endif // QFE_OPTIMIZE_OFF
}

void QFE::CsharpOnQFELinker::Native_Debug_SystemLog(MonoString* message)
{
	message;
#ifdef QFE_OPTIMIZE_OFF
	char* utf8_message = mono_string_to_utf8(message);
	QFE_LOG("QFE_C#System: " + std::string(utf8_message));
	mono_free(utf8_message);
#endif // QFE_OPTIMIZE_OFF
}

void QFE::CsharpOnQFELinker::SetGrayScaleOffset(float offset) {
	offset = std::clamp(offset, 0.0f, 1.0f);
	RenderingPostprocess::GetInstance()->GetGrayScaleOffset().offset.x = offset;
}

void QFE::CsharpOnQFELinker::SetColorCorrection(float exposure, float contrast, float saturation, float gamma, float hue) {
	ColorCorrectionOffset& offset = RenderingPostprocess::GetInstance()->GetColorCorrectionOffset();
	offset.exposure = exposure;
	offset.contrast = contrast;
	offset.saturation = saturation;
	offset.gamma = gamma;
	offset.hue = hue;
}
