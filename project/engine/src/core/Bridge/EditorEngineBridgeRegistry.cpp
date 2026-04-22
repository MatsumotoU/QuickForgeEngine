#include "engine/include/core/Bridge/EditorEngineBridgeRegistry.h"
#include "engine/include/WindowsEngineCore.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

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
#include "engine/include/assets/Script/Data/ScriptHandle.h"
#include "engine/include/assets/Script/LuaScriptExecutor.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"
#include "engine/include/assets/Script/Data/CsharpComponent.h"
#include "engine/include/camera/Data/BillboardComponent.h"
#include "engine/include/core/Math/ParentData.h"

void QFE::EditorEngineBridgeRegistry::RegisterFunctions(WindowsEngineCore* engineCore) {
	engineCore;
#ifdef QFE_OPTIMIZE_OFF
	EditorEngineBridge::GetModelDirectoryPath = [engineCore]() -> std::string {
		return engineCore->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Model");
		};
	EditorEngineBridge::GetImageDirectoryPath = [engineCore]() -> std::string {
		return engineCore->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Image");
		};
	EditorEngineBridge::GetEntityTemplateDirectoryPath = [engineCore]() -> std::string {
		return engineCore->GetAssetManager()->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
		};

	EditorEngineBridge::GetAllEntityIds = [engineCore]() -> std::vector<uint32_t> {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			return sceneManager_->GetEntityManager()->GetActiveEntityIds();
		}
		return {};
		};
	EditorEngineBridge::GetEntityName = [engineCore](uint32_t entityId) -> std::string {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			if (sceneManager_->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				return sceneManager_->GetEntityManager()->GetComponent<SceneObjectData>(entityId).name;
			}
		}
		return "";
		};
	EditorEngineBridge::SetEntityName = [engineCore](uint32_t entityId, const std::string& name) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			if (sceneManager_->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				sceneManager_->GetEntityManager()->GetComponent<SceneObjectData>(entityId).name = name;
			}
		}
		};
	EditorEngineBridge::GetEntityTag = [engineCore](uint32_t entityId) -> std::string {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			if (sceneManager_->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				return sceneManager_->GetEntityManager()->GetComponent<SceneObjectData>(entityId).tag;
			}
		}
		return "";
		};
	EditorEngineBridge::SetEntityTag = [engineCore](uint32_t entityId, const std::string& tag) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			if (sceneManager_->GetEntityManager()->HasComponent<SceneObjectData>(entityId)) {
				sceneManager_->GetEntityManager()->GetComponent<SceneObjectData>(entityId).tag = tag;
			}
		}
		};

	EditorEngineBridge::HasComponent = [engineCore](uint32_t entityId, ComponentType type) -> bool {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (!sceneManager_) return false;
		EntityManager* em = sceneManager_->GetEntityManager();
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
		case ComponentType::LuaScript: return em->HasComponent<ScriptHandles>(entityId);
		case ComponentType::CsharpScript: return em->HasComponent<CsharpComponent>(entityId);
		case ComponentType::ParentData: return em->HasComponent<ParentData>(entityId);
		}
		return false;
		};

	EditorEngineBridge::AddComponent = [engineCore](uint32_t entityId, ComponentType type) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (!sceneManager_) return;
		EntityManager* em = sceneManager_->GetEntityManager();
		switch (type) {
		case ComponentType::Transform: em->EmplaceComponent<Transform>(entityId); break;
		case ComponentType::SceneObjectData: em->EmplaceComponent<SceneObjectData>(entityId); break;
		case ComponentType::Billboard: em->EmplaceComponent<Component::BillboardComponent>(entityId); break;
		case ComponentType::PhysicsForce: em->EmplaceComponent<Force>(entityId); break;
		case ComponentType::SphereCollider: em->EmplaceComponent<SphereColliderData>(entityId); break;
		case ComponentType::AABBCollider: em->EmplaceComponent<AABBColliderData>(entityId); break;
		case ComponentType::ParentData: em->EmplaceComponent<ParentData>(entityId); break;
			// 他のコンポーネントは初期化に引数が必要な場合があるため、一旦保留
		}
		};

	EditorEngineBridge::RemoveComponent = [engineCore](uint32_t entityId, ComponentType type) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (!sceneManager_) return;
		EntityManager* em = sceneManager_->GetEntityManager();
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
		case ComponentType::LuaScript: em->RemoveComponent<ScriptHandles>(entityId); break;
		case ComponentType::CsharpScript: em->RemoveComponent<CsharpComponent>(entityId); break;
		case ComponentType::ParentData: em->RemoveComponent<ParentData>(entityId); break;
		}
		};

	EditorEngineBridge::GetTransform = [engineCore](uint32_t entityId) -> TransformData {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		TransformData data = {};
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<Transform>(entityId)) {
			Transform& t = sceneManager_->GetEntityManager()->GetComponent<Transform>(entityId);
			data.translate[0] = t.translate.x; data.translate[1] = t.translate.y; data.translate[2] = t.translate.z;
			data.rotate[0] = t.rotate.x; data.rotate[1] = t.rotate.y; data.rotate[2] = t.rotate.z;
			data.scale[0] = t.scale.x; data.scale[1] = t.scale.y; data.scale[2] = t.scale.z;
		}
		return data;
		};

	EditorEngineBridge::SetTransform = [engineCore](uint32_t entityId, const TransformData& data) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<Transform>(entityId)) {
			Transform& t = sceneManager_->GetEntityManager()->GetComponent<Transform>(entityId);
			t.translate = { data.translate[0], data.translate[1], data.translate[2] };
			t.rotate = { data.rotate[0], data.rotate[1], data.rotate[2] };
			t.scale = { data.scale[0], data.scale[1], data.scale[2] };
		}
		};

	EditorEngineBridge::GetModelRenderInfo = [engineCore](uint32_t entityId) -> ModelRenderInfo {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		AssetManager* assetManager = engineCore->GetAssetManager();
		ModelRenderInfo info = {};
		if (sceneManager_ && assetManager && sceneManager_->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
			ModelHandle& handle = sceneManager_->GetEntityManager()->GetComponent<ModelHandle>(entityId);
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
		};

	EditorEngineBridge::ChangeModel = [engineCore](uint32_t entityId, const std::string& modelName) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->ChangeEntityModel(entityId, modelName);
		}
		};

	EditorEngineBridge::SetMeshMaterial = [engineCore](uint32_t entityId, int meshIndex, const float color[4], float shininess) {
		AssetManager* assetManager = engineCore->GetAssetManager();
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && assetManager && sceneManager_->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
			ModelHandle& handle = sceneManager_->GetEntityManager()->GetComponent<ModelHandle>(entityId);
			ModelRenderData* modelData = assetManager->GetModelRenderData(handle.handle);
			if (modelData && meshIndex < modelData->meshRenderDataHandles.size()) {
				Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(modelData->meshRenderDataHandles[meshIndex].materialHandle);
				if (material) {
					material->color = { color[0], color[1], color[2], color[3] };
					material->shininess = shininess;
				}
			}
		}
		};

	EditorEngineBridge::SetMeshLight = [engineCore](uint32_t entityId, int meshIndex, const float color[4], const float direction[3]) {
		AssetManager* assetManager = engineCore->GetAssetManager();
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && assetManager && sceneManager_->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
			ModelHandle& handle = sceneManager_->GetEntityManager()->GetComponent<ModelHandle>(entityId);
			ModelRenderData* modelData = assetManager->GetModelRenderData(handle.handle);
			if (modelData && meshIndex < modelData->meshRenderDataHandles.size()) {
				DirectionalLight* light = assetManager->GetGpuBufferPool()->GetConstantBufferData<DirectionalLight>(modelData->meshRenderDataHandles[meshIndex].lightBufferHandle);
				if (light) {
					light->color = { color[0], color[1], color[2], color[3] };
					light->direction = Vector3::Normalize({ direction[0], direction[1], direction[2] });
				}
			}
		}
		};

	EditorEngineBridge::GetParticleInfo = [engineCore](uint32_t entityId) -> ParticleInfo {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		ParticleInfo info = {};
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<ParticleComponent>(entityId)) {
			ParticleComponent& p = sceneManager_->GetEntityManager()->GetComponent<ParticleComponent>(entityId);
			info.modelName = p.modelName;
			info.maxParticleCount = p.maxParticleCount;
		}
		return info;
		};

	EditorEngineBridge::GetSpriteInfo = [engineCore](uint32_t entityId) -> SpriteInfo {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		SpriteInfo info = {};
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
			SpriteData& s = sceneManager_->GetEntityManager()->GetComponent<SpriteData>(entityId);
			info.fileName = s.textureName;
			info.isBillboard = sceneManager_->GetEntityManager()->HasComponent<Component::BillboardComponent>(entityId);
		}
		return info;
		};

	EditorEngineBridge::SetSpriteInfo = [engineCore](uint32_t entityId, const SpriteInfo& info) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
			SpriteData& s = sceneManager_->GetEntityManager()->GetComponent<SpriteData>(entityId);
			s.textureName = info.fileName;
			// Billboard handling
			bool hasBillboard = sceneManager_->GetEntityManager()->HasComponent<Component::BillboardComponent>(entityId);
			if (info.isBillboard && !hasBillboard) {
				sceneManager_->GetEntityManager()->EmplaceComponent<Component::BillboardComponent>(entityId);
			} else if (!info.isBillboard && hasBillboard) {
				sceneManager_->GetEntityManager()->RemoveComponent<Component::BillboardComponent>(entityId);
			}
		}
		};

	EditorEngineBridge::GetCameraInfo = [engineCore](uint32_t entityId) -> CameraInfo {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		CameraInfo info = {};
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<CameraData>(entityId)) {
			CameraData& c = sceneManager_->GetEntityManager()->GetComponent<CameraData>(entityId);
			info.fov = c.fovY_;
			info.nearZ = c.nearZ_;
			info.farZ = c.farZ_;
		}
		return info;
		};

	EditorEngineBridge::SetCameraInfo = [engineCore](uint32_t entityId, const CameraInfo& info) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<CameraData>(entityId)) {
			CameraData& c = sceneManager_->GetEntityManager()->GetComponent<CameraData>(entityId);
			c.fovY_ = info.fov;
			c.nearZ_ = info.nearZ;
			c.farZ_ = info.farZ;
		}
		};

	EditorEngineBridge::GetForceData = [engineCore](uint32_t entityId) -> ForceData {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		ForceData data = {};
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<Force>(entityId)) {
			Force& f = sceneManager_->GetEntityManager()->GetComponent<Force>(entityId);
			data.velocity[0] = f.velocity.x; data.velocity[1] = f.velocity.y; data.velocity[2] = f.velocity.z;
			data.acceleration[0] = f.acceleration.x; data.acceleration[1] = f.acceleration.y; data.acceleration[2] = f.acceleration.z;
			data.mass = f.mass;
			data.friction = f.friction;
			data.gravityStrength = f.gravityStrength;
			data.isGravity = f.isGravity;
		}
		return data;
		};

	EditorEngineBridge::SetForceData = [engineCore](uint32_t entityId, const ForceData& data) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<Force>(entityId)) {
			Force& f = sceneManager_->GetEntityManager()->GetComponent<Force>(entityId);
			f.velocity = { data.velocity[0], data.velocity[1], data.velocity[2] };
			f.acceleration = { data.acceleration[0], data.acceleration[1], data.acceleration[2] };
			f.mass = data.mass;
			f.friction = data.friction;
			f.gravityStrength = data.gravityStrength;
			f.isGravity = data.isGravity;
		}
		};

	EditorEngineBridge::GetSphereColliderInfo = [engineCore](uint32_t entityId) -> SphereColliderInfo {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		SphereColliderInfo info = {};
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<SphereColliderData>(entityId)) {
			SphereColliderData& c = sceneManager_->GetEntityManager()->GetComponent<SphereColliderData>(entityId);
			info.center[0] = c.sphere.center.x; info.center[1] = c.sphere.center.y; info.center[2] = c.sphere.center.z;
			info.radius = c.sphere.radius;
			info.isTrigger = c.isTrigger;
			info.isStatic = c.isStatic;
			info.colliderLayer = c.colliderLayer;
			info.eventColliderLayer = c.eventColliderLayer;
			info.isDraw = c.isDraw;
		}
		return info;
		};

	EditorEngineBridge::SetSphereColliderInfo = [engineCore](uint32_t entityId, const SphereColliderInfo& info) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<SphereColliderData>(entityId)) {
			SphereColliderData& c = sceneManager_->GetEntityManager()->GetComponent<SphereColliderData>(entityId);
			c.sphere.center = { info.center[0], info.center[1], info.center[2] };
			c.sphere.radius = info.radius;
			c.isTrigger = info.isTrigger;
			c.isStatic = info.isStatic;
			c.colliderLayer = info.colliderLayer;
			c.eventColliderLayer = info.eventColliderLayer;
			c.isDraw = info.isDraw;
		}
		};

	EditorEngineBridge::GetAABBColliderInfo = [engineCore](uint32_t entityId) -> AABBColliderInfo {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		AABBColliderInfo info = {};
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<AABBColliderData>(entityId)) {
			AABBColliderData& c = sceneManager_->GetEntityManager()->GetComponent<AABBColliderData>(entityId);
			info.center[0] = c.aabb.center.x; info.center[1] = c.aabb.center.y; info.center[2] = c.aabb.center.z;
			info.size[0] = c.aabb.size.x; info.size[1] = c.aabb.size.y; info.size[2] = c.aabb.size.z;
			info.isTrigger = c.isTrigger;
			info.isStatic = c.isStatic;
			info.colliderLayer = c.colliderLayer;
			info.eventColliderLayer = c.eventColliderLayer;
			info.isDraw = c.isDraw;
		}
		return info;
		};

	EditorEngineBridge::SetAABBColliderInfo = [engineCore](uint32_t entityId, const AABBColliderInfo& info) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<AABBColliderData>(entityId)) {
			AABBColliderData& c = sceneManager_->GetEntityManager()->GetComponent<AABBColliderData>(entityId);
			c.aabb.center = { info.center[0], info.center[1], info.center[2] };
			c.aabb.size = { info.size[0], info.size[1], info.size[2] };
			c.isTrigger = info.isTrigger;
			c.isStatic = info.isStatic;
			c.colliderLayer = info.colliderLayer;
			c.eventColliderLayer = info.eventColliderLayer;
			c.isDraw = info.isDraw;
		}
		};

	EditorEngineBridge::GetLuaScripts = [engineCore](uint32_t entityId) -> std::vector<ScriptInfo> {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		std::vector<ScriptInfo> scripts;
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<ScriptHandles>(entityId)) {
			ScriptHandles& sh = sceneManager_->GetEntityManager()->GetComponent<ScriptHandles>(entityId);
			for (auto& luaHandle : sh.scriptHandles_) {
				ScriptInfo info;
				info.name = luaHandle.scriptName_;
				info.handle = luaHandle.handle_;
				info.priority = (int)luaHandle.priority_;

				LuaScriptOnQFE* script = sceneManager_->GetLuaScriptExecutor()->GetScript(luaHandle.handle_);
				if (script) {
					for (auto& valName : script->GetGlobalValuesList()) {
						ScriptParamInfo p;
						p.name = valName;
						sol::object obj = script->GetEnvironment()[valName];
						if (obj.is<int>()) { p.type = ScriptParamType::Int; p.value = std::to_string(obj.as<int>()); } else if (obj.is<float>()) { p.type = ScriptParamType::Float; p.value = std::to_string(obj.as<float>()); } else if (obj.is<bool>()) { p.type = ScriptParamType::Bool; p.value = obj.as<bool>() ? "true" : "false"; } else if (obj.is<std::string>()) { p.type = ScriptParamType::String; p.value = obj.as<std::string>(); } else { p.type = ScriptParamType::Unknown; }
						info.params.push_back(p);
					}
				}
				scripts.push_back(info);
			}
		}
		return scripts;
		};

	EditorEngineBridge::SetLuaScriptParam = [engineCore](uint32_t entityId, uint32_t handle, const std::string& paramName, const std::string& value) {
		entityId; // TODO: 現状、handleでスクリプトを特定しているためentityIdは使用しないが、将来的に複数スクリプトを同一エンティティにアタッチできるようになった際に必要になる可能性があるため引数として残す
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			LuaScriptOnQFE* script = sceneManager_->GetLuaScriptExecutor()->GetScript(handle);
			if (script) {
				sol::object obj = script->GetEnvironment()[paramName];
				if (obj.is<int>()) { script->GetEnvironment()[paramName] = std::stoi(value); } else if (obj.is<float>()) { script->GetEnvironment()[paramName] = std::stof(value); } else if (obj.is<bool>()) { script->GetEnvironment()[paramName] = (value == "true"); } else if (obj.is<std::string>()) { script->GetEnvironment()[paramName] = value; }
			}
		}
		};

	EditorEngineBridge::RemoveLuaScript = [engineCore](uint32_t entityId, uint32_t handle) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->GetLuaScriptExecutor()->RemoveScript(handle);
		}
		};

	EditorEngineBridge::AddLuaScript = [engineCore](uint32_t entityId, const std::string& scriptName) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddLuaScript(entityId, scriptName);
		}
		};

	EditorEngineBridge::CreateLuaScript = [engineCore](const std::string& scriptName) {
		// ディレクトリパス
		const std::string dirPath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
		// ディレクトリがなければ作成
		std::filesystem::create_directories(dirPath);
		// ファイルパス
		const std::string filePath = dirPath + scriptName;
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Create Lua Script: " + filePath, LogLevel::EditorInfo);
#endif // _DEBUG
		// Luaテンプレート
		const char* luaTemplate =
			"function Init()\n"
			"\n"
			"end\n"
			"\n"
			"function Update()\n"
			"\n"
			"end\n";
		// ファイル書き込み
		std::ofstream ofs(filePath);
		if (!ofs) {
			return;
		}
		ofs << luaTemplate;
		ofs.close();
		// 自動で開く
		try {
			std::filesystem::path absPath = std::filesystem::absolute(filePath);
			ShellExecuteA(nullptr, "open", "code", absPath.string().c_str(), nullptr, SW_SHOWNORMAL);
		}
		catch (const std::exception& e) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(e.what(), LogLevel::Error);
#else
			std::cerr << e.what() << std::endl;
#endif
		}
		};

	EditorEngineBridge::GetCsharpClassNames = [engineCore](uint32_t entityId) -> std::vector<std::string> {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		std::vector<std::string> names;
		if (sceneManager_ && sceneManager_->GetEntityManager()->HasComponent<CsharpComponent>(entityId)) {
			CsharpComponent& cs = sceneManager_->GetEntityManager()->GetComponent<CsharpComponent>(entityId);
			for (auto& handle : cs.csharpHandles_) {
				names.push_back(handle.className_);
			}
		}
		return names;
		};

	EditorEngineBridge::RemoveCsharpScript = [engineCore](uint32_t entityId, const std::string& className) {
		className; // TODO: 現状、C#スクリプトはクラス名で特定しているためentityIdは使用しないが、将来的に複数スクリプトを同一エンティティにアタッチできるようになった際に必要になる可能性があるため引数として残す
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->GetEntityManager()->RemoveComponent<CsharpComponent>(entityId);
		}
		};

	EditorEngineBridge::AddCsharpScript = [engineCore](uint32_t entityId, const std::string& className) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddCsharpScript(entityId, className);
		}
		};

	EditorEngineBridge::GetAvailableCsharpClasses = [engineCore]() -> std::vector<std::string> {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_ && sceneManager_->GetCsharpScriptExecutor()) {
			return sceneManager_->GetCsharpScriptExecutor()->GetAvailableScriptClasses();
		}
		return {};
		};

	EditorEngineBridge::AddEmptyEntity = [engineCore]() {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddEmptyObject();
		}
		};
	EditorEngineBridge::AddEntityFromFile = [engineCore](const std::string& filePath) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddEntity(filePath);
		}
		};
	EditorEngineBridge::AddModelEntity = [engineCore](const std::string& modelPath) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddModel(modelPath);
		}
		};
	EditorEngineBridge::AddSpriteEntity = [engineCore](const std::string& spritePath) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddSprite(spritePath);
		}
		};
	EditorEngineBridge::AddParticleEmitterEntity = [engineCore](const std::string& particlePath, uint32_t count) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->AddParticleEmitter(particlePath, count);
		}
		};
	EditorEngineBridge::AddCameraEntity = [engineCore]() {
		// カメラの機能がシングルトンであるため、複数カメラにすると不具合が起きる可能性があるため一時的に制限
		DebugLog("Can not add Camera.");
		};
	EditorEngineBridge::CopyEntity = [engineCore](uint32_t entityId) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->CopyEntity(entityId);
		}
		};
	EditorEngineBridge::SaveEntity = [engineCore](uint32_t entityId, std::string filePath) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->SaveEntity(entityId, filePath);
		}
		};
	EditorEngineBridge::DeleteEntity = [engineCore](uint32_t entityId) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->DeleteEntity(entityId);
		}
		};
	EditorEngineBridge::ParentChild = [engineCore](uint32_t parentId, uint32_t childId) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->ParentChild(parentId, childId);
		}
		};
	EditorEngineBridge::Unparent = [engineCore](uint32_t entityId) {
		SceneManager* sceneManager_ = engineCore->GetSceneManager();
		if (sceneManager_) {
			sceneManager_->Unparent(entityId);
		}
		};

	EditorEngineBridge::GetDebugCameraEntityId = [engineCore]() -> uint32_t {
		if (!CameraManager::GetInstance()) {
			DebugLog("CameraManager is not initialized.");
			return UINT32_MAX;
		}
		return CameraManager::GetInstance()->GetCamera(0).GetBindEntityId();
		};
#endif // QFE_OPTIMIZE_OFF
}

void QFE::EditorEngineBridgeRegistry::ClearFunctions() {
#ifdef QFE_OPTIMIZE_OFF
	EditorEngineBridge::GetModelDirectoryPath = nullptr;
	EditorEngineBridge::GetImageDirectoryPath = nullptr;
	EditorEngineBridge::GetEntityTemplateDirectoryPath = nullptr;
	EditorEngineBridge::GetAllEntityIds = nullptr;
	EditorEngineBridge::GetEntityName = nullptr;
	EditorEngineBridge::AddEmptyEntity = nullptr;
	EditorEngineBridge::AddEntityFromFile = nullptr;
	EditorEngineBridge::AddModelEntity = nullptr;
	EditorEngineBridge::AddSpriteEntity = nullptr;
	EditorEngineBridge::AddParticleEmitterEntity = nullptr;
	EditorEngineBridge::AddCameraEntity = nullptr;
	EditorEngineBridge::CopyEntity = nullptr;
	EditorEngineBridge::SaveEntity = nullptr;
	EditorEngineBridge::DeleteEntity = nullptr;
	EditorEngineBridge::ParentChild = nullptr;
#endif // QFE_OPTIMIZE_OFF
}
