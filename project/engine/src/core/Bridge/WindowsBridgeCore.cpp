#include "engine/include/core/Bridge/WindowsBridgeCore.h"

#include "engine/include/WindowsEngineCore.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"

#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/assets/Particle/Data/ParticleComponent.h"
#include "engine/include/assets/Sprite/Data/SpriteData.h"
#include "engine/include/camera/Data/CameraData.h"
#include "engine/include/physics/Force.h"
#include "engine/include/collider/Data/SphereColliderData.h"
#include "engine/include/collider/Data/AABBColliderData.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"
#include "engine/include/assets/Script/Data/CsharpComponent.h"
#include "engine/include/camera/Data/BillboardComponent.h"
#include "engine/include/core/Math/ParentData.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif

namespace QFE {

	std::string WindowsBridgeCore::GetModelDirectoryPath() {
		return engineCore_->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Model");
	}

	std::string WindowsBridgeCore::GetImageDirectoryPath() {
		return engineCore_->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Image");
	}

	std::string WindowsBridgeCore::GetEntityTemplateDirectoryPath() {
		return engineCore_->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
	}

	std::string WindowsBridgeCore::GetScriptDirectoryPath() {
		return engineCore_->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	}

	std::vector<uint32_t> WindowsBridgeCore::GetAllEntityIds() {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			return sceneManager->GetEntityManager()->GetActiveEntityIds();
		}
		return {};
	}

	std::string WindowsBridgeCore::GetEntityName(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			if (sceneManager->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				return sceneManager->GetEntityManager()->GetComponent<SceneObjectData>(entityId).name;
			}
		}
		return "";
	}

	void WindowsBridgeCore::SetEntityName(uint32_t entityId, const std::string& name) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			if (sceneManager->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				sceneManager->GetEntityManager()->GetComponent<SceneObjectData>(entityId).name = name;
			}
		}
	}

	std::string WindowsBridgeCore::GetEntityTag(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			if (sceneManager->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				return sceneManager->GetEntityManager()->GetComponent<SceneObjectData>(entityId).tag;
			}
		}
		return "";
	}

	void WindowsBridgeCore::SetEntityTag(uint32_t entityId, const std::string& tag) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			if (sceneManager->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				sceneManager->GetEntityManager()->GetComponent<SceneObjectData>(entityId).tag = tag;
			}
		}
	}

	bool WindowsBridgeCore::HasComponent(uint32_t entityId, ComponentType type) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (!sceneManager) return false;
		EntityManager* em = sceneManager->GetEntityManager();
		switch (type) {
		case ComponentType::Transform: return em->HasComponent<Transform>(entityId);
		case ComponentType::SceneObjectData: return em->HasComponent<SceneObjectData>(entityId);
		case ComponentType::ModelHandle: return em->HasComponent<ModelHandle>(entityId);
		case ComponentType::SpriteData: return em->HasComponent<SpriteData>(entityId);
		case ComponentType::CameraData: return em->HasComponent<CameraData>(entityId);
		case ComponentType::Billboard: return em->HasComponent<Component::BillboardComponent>(entityId);
		case ComponentType::Particle: return em->HasComponent<ParticleComponent>(entityId);
		case ComponentType::PhysicsForce: return em->HasComponent<Force>(entityId);
		case ComponentType::SphereCollider: return em->HasComponent<SphereColliderData>(entityId);
		case ComponentType::AABBCollider: return em->HasComponent<AABBColliderData>(entityId);
		case ComponentType::CsharpScript: return em->HasComponent<CsharpComponent>(entityId);
		case ComponentType::ParentData: return em->HasComponent<ParentData>(entityId);
		}
		return false;
	}

	void WindowsBridgeCore::AddComponent(uint32_t entityId, ComponentType type) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (!sceneManager) return;
		EntityManager* em = sceneManager->GetEntityManager();
		switch (type) {
		case ComponentType::Transform: em->EmplaceComponent<Transform>(entityId); break;
		case ComponentType::SceneObjectData: em->EmplaceComponent<SceneObjectData>(entityId); break;
		case ComponentType::Billboard: em->EmplaceComponent<Component::BillboardComponent>(entityId); break;
		case ComponentType::PhysicsForce: em->EmplaceComponent<Force>(entityId); break;
		case ComponentType::SphereCollider: em->EmplaceComponent<SphereColliderData>(entityId); break;
		case ComponentType::AABBCollider: em->EmplaceComponent<AABBColliderData>(entityId); break;
		case ComponentType::ParentData: em->EmplaceComponent<ParentData>(entityId); break;
			// 他のコンポーネントは必要に応じて対応
		}
	}

	void WindowsBridgeCore::RemoveComponent(uint32_t entityId, ComponentType type) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (!sceneManager) return;
		EntityManager* em = sceneManager->GetEntityManager();
		switch (type) {
		case ComponentType::Transform: em->RemoveComponent<Transform>(entityId); break;
		case ComponentType::SceneObjectData: em->RemoveComponent<SceneObjectData>(entityId); break;
		case ComponentType::ModelHandle: em->RemoveComponent<ModelHandle>(entityId); break;
		case ComponentType::SpriteData: em->RemoveComponent<SpriteData>(entityId); break;
		case ComponentType::CameraData: em->RemoveComponent<CameraData>(entityId); break;
		case ComponentType::Billboard: em->RemoveComponent<Component::BillboardComponent>(entityId); break;
		case ComponentType::Particle: em->RemoveComponent<ParticleComponent>(entityId); break;
		case ComponentType::PhysicsForce: em->RemoveComponent<Force>(entityId); break;
		case ComponentType::SphereCollider: em->RemoveComponent<SphereColliderData>(entityId); break;
		case ComponentType::AABBCollider: em->RemoveComponent<AABBColliderData>(entityId); break;
		case ComponentType::CsharpScript: em->RemoveComponent<CsharpComponent>(entityId); break;
		case ComponentType::ParentData: em->RemoveComponent<ParentData>(entityId); break;
		}
	}

	TransformData WindowsBridgeCore::GetTransform(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		TransformData data = {};
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<Transform>(entityId)) {
			Transform& t = sceneManager->GetEntityManager()->GetComponent<Transform>(entityId);
			data.translate[0] = t.translate.x; data.translate[1] = t.translate.y; data.translate[2] = t.translate.z;
			data.rotate[0] = t.rotate.x; data.rotate[1] = t.rotate.y; data.rotate[2] = t.rotate.z;
			data.scale[0] = t.scale.x; data.scale[1] = t.scale.y; data.scale[2] = t.scale.z;
		}
		return data;
	}

	void WindowsBridgeCore::SetTransform(uint32_t entityId, const TransformData& data) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<Transform>(entityId)) {
			Transform& t = sceneManager->GetEntityManager()->GetComponent<Transform>(entityId);
			t.translate = { data.translate[0], data.translate[1], data.translate[2] };
			t.rotate = { data.rotate[0], data.rotate[1], data.rotate[2] };
			t.scale = { data.scale[0], data.scale[1], data.scale[2] };
		}
	}

	ModelRenderInfo WindowsBridgeCore::GetModelRenderInfo(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		AssetManager* assetManager = engineCore_->GetAssetManager();
		ModelRenderInfo info = {};
		if (sceneManager && assetManager && sceneManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
			ModelHandle& handle = sceneManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
			info.modelName = handle.modelName;
			ModelRenderData* modelData = assetManager->GetModelRenderData(handle.handle);
			if (modelData) {
				for (auto& mesh : modelData->meshRenderDataHandles) {
					MeshInfo meshInfo = {};
					Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(mesh.materialHandle);
					DirectionalLight* light = assetManager->GetGpuBufferPool()->GetConstantBufferData<DirectionalLight>(mesh.lightBufferHandle);
					if (material) {
						meshInfo.materialColor[0] = material->color.x; meshInfo.materialColor[1] = material->color.y;
						meshInfo.materialColor[2] = material->color.z; meshInfo.materialColor[3] = material->color.w;
						meshInfo.materialShininess = material->shininess;
					}
					if (light) {
						meshInfo.lightColor[0] = light->color.x; meshInfo.lightColor[1] = light->color.y;
						meshInfo.lightColor[2] = light->color.z; meshInfo.lightColor[3] = light->color.w;
						meshInfo.lightDirection[0] = light->direction.x; meshInfo.lightDirection[1] = light->direction.y;
						meshInfo.lightDirection[2] = light->direction.z;
					}
					info.meshes.push_back(meshInfo);
				}
			}
		}
		return info;
	}

	void WindowsBridgeCore::SetMeshMaterial(uint32_t entityId, int meshIndex, const float color[4], float shininess) {
		AssetManager* assetManager = engineCore_->GetAssetManager();
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && assetManager && sceneManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
			ModelHandle& handle = sceneManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
			ModelRenderData* modelData = assetManager->GetModelRenderData(handle.handle);
			if (modelData && meshIndex < modelData->meshRenderDataHandles.size()) {
				Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(modelData->meshRenderDataHandles[meshIndex].materialHandle);
				if (material) {
					material->color = { color[0], color[1], color[2], color[3] };
					material->shininess = shininess;
				}
			}
		}
	}

	void WindowsBridgeCore::SetMeshLight(uint32_t entityId, int meshIndex, const float color[4], const float direction[3]) {
		AssetManager* assetManager = engineCore_->GetAssetManager();
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && assetManager && sceneManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
			ModelHandle& handle = sceneManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
			ModelRenderData* modelData = assetManager->GetModelRenderData(handle.handle);
			if (modelData && meshIndex < modelData->meshRenderDataHandles.size()) {
				DirectionalLight* light = assetManager->GetGpuBufferPool()->GetConstantBufferData<DirectionalLight>(modelData->meshRenderDataHandles[meshIndex].lightBufferHandle);
				if (light) {
					light->color = { color[0], color[1], color[2], color[3] };
					light->direction = Vector3::Normalize({ direction[0], direction[1], direction[2] });
				}
			}
		}
	}

	void WindowsBridgeCore::ChangeModel(uint32_t entityId, const std::string& modelName) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->ChangeEntityModel(entityId, modelName);
		}
	}

	ParticleInfo WindowsBridgeCore::GetParticleInfo(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		ParticleInfo info = {};
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<ParticleComponent>(entityId)) {
			ParticleComponent& p = sceneManager->GetEntityManager()->GetComponent<ParticleComponent>(entityId);
			info.modelName = p.modelName;
			info.maxParticleCount = p.maxParticleCount;
		}
		return info;
	}

	SpriteInfo WindowsBridgeCore::GetSpriteInfo(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		SpriteInfo info = {};
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
			SpriteData& s = sceneManager->GetEntityManager()->GetComponent<SpriteData>(entityId);
			info.fileName = s.textureName;
			info.isBillboard = sceneManager->GetEntityManager()->HasComponent<Component::BillboardComponent>(entityId);
			info.width = s.width;
			info.height = s.height;
			info.pivot[0] = s.pivot.x; info.pivot[1] = s.pivot.y;
		}
		return info;
	}

	void WindowsBridgeCore::SetSpriteInfo(uint32_t entityId, const SpriteInfo& info) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
			SpriteData& s = sceneManager->GetEntityManager()->GetComponent<SpriteData>(entityId);
			s.textureName = info.fileName;
			bool hasBillboard = sceneManager->GetEntityManager()->HasComponent<Component::BillboardComponent>(entityId);
			if (info.isBillboard && !hasBillboard) {
				sceneManager->GetEntityManager()->EmplaceComponent<Component::BillboardComponent>(entityId);
			}
			else if (!info.isBillboard && hasBillboard) {
				sceneManager->GetEntityManager()->RemoveComponent<Component::BillboardComponent>(entityId);
			}
			s.width = info.width;
			s.height = info.height;
			s.pivot = { info.pivot[0], info.pivot[1] };
		}
	}

	CameraInfo WindowsBridgeCore::GetCameraInfo(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		CameraInfo info = {};
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<CameraData>(entityId)) {
			CameraData& c = sceneManager->GetEntityManager()->GetComponent<CameraData>(entityId);
			info.fov = c.fovY_;
			info.nearZ = c.nearZ_;
			info.farZ = c.farZ_;
		}
		return info;
	}

	void WindowsBridgeCore::SetCameraInfo(uint32_t entityId, const CameraInfo& info) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<CameraData>(entityId)) {
			CameraData& c = sceneManager->GetEntityManager()->GetComponent<CameraData>(entityId);
			c.fovY_ = info.fov;
			c.nearZ_ = info.nearZ;
			c.farZ_ = info.farZ;
		}
	}

	ForceData WindowsBridgeCore::GetForceData(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		ForceData data = {};
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<Force>(entityId)) {
			Force& f = sceneManager->GetEntityManager()->GetComponent<Force>(entityId);
			data.velocity[0] = f.velocity.x; data.velocity[1] = f.velocity.y; data.velocity[2] = f.velocity.z;
			data.acceleration[0] = f.acceleration.x; data.acceleration[1] = f.acceleration.y; data.acceleration[2] = f.acceleration.z;
			data.mass = f.mass;
			data.friction = f.friction;
			data.gravityStrength = f.gravityStrength;
			data.isGravity = f.isGravity;
		}
		return data;
	}

	void WindowsBridgeCore::SetForceData(uint32_t entityId, const ForceData& data) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<Force>(entityId)) {
			Force& f = sceneManager->GetEntityManager()->GetComponent<Force>(entityId);
			f.velocity = { data.velocity[0], data.velocity[1], data.velocity[2] };
			f.acceleration = { data.acceleration[0], data.acceleration[1], data.acceleration[2] };
			f.mass = data.mass;
			f.friction = data.friction;
			f.gravityStrength = data.gravityStrength;
			f.isGravity = data.isGravity;
		}
	}

	SphereColliderInfo WindowsBridgeCore::GetSphereColliderInfo(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		SphereColliderInfo info = {};
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<SphereColliderData>(entityId)) {
			SphereColliderData& c = sceneManager->GetEntityManager()->GetComponent<SphereColliderData>(entityId);
			info.center[0] = c.sphere.center.x; info.center[1] = c.sphere.center.y; info.center[2] = c.sphere.center.z;
			info.radius = c.sphere.radius;
			info.isTrigger = c.isTrigger;
			info.isStatic = c.isStatic;
			info.colliderLayer = c.colliderLayer;
			info.eventColliderLayer = c.eventColliderLayer;
			info.isDraw = c.isDraw;
		}
		return info;
	}

	void WindowsBridgeCore::SetSphereColliderInfo(uint32_t entityId, const SphereColliderInfo& info) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<SphereColliderData>(entityId)) {
			SphereColliderData& c = sceneManager->GetEntityManager()->GetComponent<SphereColliderData>(entityId);
			c.sphere.center = { info.center[0], info.center[1], info.center[2] };
			c.sphere.radius = info.radius;
			c.isTrigger = info.isTrigger;
			c.isStatic = info.isStatic;
			c.colliderLayer = info.colliderLayer;
			c.eventColliderLayer = info.eventColliderLayer;
			c.isDraw = info.isDraw;
		}
	}

	AABBColliderInfo WindowsBridgeCore::GetAABBColliderInfo(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		AABBColliderInfo info = {};
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<AABBColliderData>(entityId)) {
			AABBColliderData& c = sceneManager->GetEntityManager()->GetComponent<AABBColliderData>(entityId);
			info.center[0] = c.aabb.center.x; info.center[1] = c.aabb.center.y; info.center[2] = c.aabb.center.z;
			info.size[0] = c.aabb.size.x; info.size[1] = c.aabb.size.y; info.size[2] = c.aabb.size.z;
			info.isTrigger = c.isTrigger;
			info.isStatic = c.isStatic;
			info.colliderLayer = c.colliderLayer;
			info.eventColliderLayer = c.eventColliderLayer;
			info.isDraw = c.isDraw;
		}
		return info;
	}

	void WindowsBridgeCore::SetAABBColliderInfo(uint32_t entityId, const AABBColliderInfo& info) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<AABBColliderData>(entityId)) {
			AABBColliderData& c = sceneManager->GetEntityManager()->GetComponent<AABBColliderData>(entityId);
			c.aabb.center = { info.center[0], info.center[1], info.center[2] };
			c.aabb.size = { info.size[0], info.size[1], info.size[2] };
			c.isTrigger = info.isTrigger;
			c.isStatic = info.isStatic;
			c.colliderLayer = info.colliderLayer;
			c.eventColliderLayer = info.eventColliderLayer;
			c.isDraw = info.isDraw;
		}
	}

	std::vector<std::string> WindowsBridgeCore::GetCsharpClassNames(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		std::vector<std::string> names;
		if (sceneManager && sceneManager->GetEntityManager()->HasComponent<CsharpComponent>(entityId)) {
			CsharpComponent& cs = sceneManager->GetEntityManager()->GetComponent<CsharpComponent>(entityId);
			for (auto& handle : cs.csharpHandles_) {
				names.push_back(handle.className_);
			}
		}
		return names;
	}

	void WindowsBridgeCore::RemoveCsharpScript(uint32_t entityId, const std::string& className) {
		(void)className;
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->GetEntityManager()->RemoveComponent<CsharpComponent>(entityId);
		}
	}

	void WindowsBridgeCore::AddCsharpScript(uint32_t entityId, const std::string& className) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->AddCsharpScript(entityId, className);
		}
	}

	std::vector<std::string> WindowsBridgeCore::GetAvailableCsharpClasses() {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager && sceneManager->GetCsharpScriptExecutor()) {
			return sceneManager->GetCsharpScriptExecutor()->GetAvailableScriptClasses();
		}
		return {};
	}

	void WindowsBridgeCore::AddEmptyEntity() {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->AddEmptyObject();
		}
	}

	void WindowsBridgeCore::AddEntityFromFile(const std::string& filepath) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->AddEntity(filepath);
		}
	}

	void WindowsBridgeCore::AddModelEntity(const std::string& filepath) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->AddModel(filepath);
		}
	}

	void WindowsBridgeCore::AddSpriteEntity(const std::string& filepath) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->AddSprite(filepath);
		}
	}

	void WindowsBridgeCore::AddParticleEmitterEntity(const std::string& filepath, uint32_t particleCount) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->AddParticleEmitter(filepath, particleCount);
		}
	}

	void WindowsBridgeCore::AddCameraEntity() {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Can not add Camera.");
#endif
	}

	void WindowsBridgeCore::CopyEntity(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->CopyEntity(entityId);
		}
	}

	void WindowsBridgeCore::SaveEntity(uint32_t entityId, std::string filename) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->SaveEntity(entityId, filename);
		}
	}

	void WindowsBridgeCore::DeleteEntity(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->DeleteEntity(entityId);
		}
	}

	void WindowsBridgeCore::ParentChild(uint32_t parentEntityId, uint32_t childEntityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->ParentChild(parentEntityId, childEntityId);
		}
	}

	void WindowsBridgeCore::Unparent(uint32_t entityId) {
		SceneManager* sceneManager = engineCore_->GetSceneManager();
		if (sceneManager) {
			sceneManager->Unparent(entityId);
		}
	}

	uint32_t WindowsBridgeCore::GetDebugCameraEntityId() {
		if (!CameraManager::GetInstance()) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("CameraManager is not initialized.");
#endif
			return UINT32_MAX;
		}
		return CameraManager::GetInstance()->GetCamera(0).GetBindEntityId();
	}

}
