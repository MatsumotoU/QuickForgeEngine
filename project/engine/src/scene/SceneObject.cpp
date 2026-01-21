#include "engine/include/scene/SceneObject.h"
#include "engine/include/assets/AssetManager.h"
#include <cassert>

#include "engine/include/core/EngineGlobalValue.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"
#include "engine/include/assets/Script/CsharpVirtualEnvironmentOnQFE.h"
#include "engine/include/assets/Script/Data/CsharpComponent.h"
#include "engine/include/collider/ColliderManager.h"
#include "engine/include/audio/AudioInterface.h"

#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/assets/Script/Data/ScriptHandle.h"
#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/collider/Data/SphereColliderData.h"
#include "engine/include/core/Math/ParentData.h"
#include "engine/include/camera/Data/CameraData.h"

#include <fstream>
#include <execution>
#include <nlohmann/json.hpp>

#include "engine/include/assets/3DModel/Loader/AssimpModelLoader.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/assets/Sprite/Data/SpriteData.h"
#include "engine/include/assets/Particle/Data/ParticleComponent.h"

#include "engine/include/renderer/ModelRenderer.h"
#include "engine/include/renderer/SpriteRenderer.h"
#include "engine/include/renderer/ParticleRenderer.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

#include "Engine/include/scene/SceneCommand/SceneEntityCommands.h"
#include "engine/include/scene/SceneObject.h"
#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

SceneObject::SceneObject() {
	assetManager_ = nullptr;
	isRequestedExit_ = false;
	sceneName_ = "NewScene";
}

SceneObject::~SceneObject() {
}

void SceneObject::Initialize() {
	assetManager_ = AssetManager::GetInstance();
	assert(assetManager_);
	isRequestedExit_ = false;

	CameraManager::GetInstance()->Initialize();
	isRequestStopScript_ = false;
	isRunningScript_ = false;
	isPauseScript_ = false;

	// 繧ｳ繝槭Φ繝峨け繝ｪ繧｢
	frameStartCommandInvoker_.ClearCommands();
	updateCommandInvoker_.ClearCommands();
	preDrawCommandInvoker_.ClearCommands();
	drawCommandInvoker_.ClearCommands();
	postDrawCommandInvoker_.ClearCommands();
}

void SceneObject::Update() {
	frameStartCommandInvoker_.ExecuteCommands();

	// 繝ｩ繝ｳ繧ｿ繧､繝荳ｭ縺ｮ繧ｵ繝悶Δ繧ｸ繝･繝ｼ繝ｫ縺ｮ譖ｴ譁ｰ
	if (isRunningScript_ && !isPauseScript_) {
		LuaScriptResourceManager::GetInstance()->FrameStart();
		LuaScriptResourceManager::GetInstance()->UpdateAllScripts();
		CsharpVirtualEnvironmentOnQFE::GetInstance()->RunAllScriptsFunction("Update");
		PhysicsManager::GetInstance()->Update();
	}

	// 蠖薙◆繧雁愛螳壽峩譁ｰ
	ColliderManager::GetInstance()->Update();

	//縲繝ｯ繝ｼ繝ｫ繝芽｡悟・譖ｴ譁ｰ
	updateCommandInvoker_.AddCommand(std::make_unique<RemakeUniqeIDCommand>(*(assetManager_->GetEntityManager()), uniqueIdManager_));
	updateCommandInvoker_.AddCommand(std::make_unique<WorldTransformationCommand>(*(assetManager_->GetEntityManager())));
	updateCommandInvoker_.AddCommand(std::make_unique<ParentUpdateCommand>(*(assetManager_->GetEntityManager())));
	updateCommandInvoker_.AddCommand(std::make_unique<AllSpriteResizeCommand>(*(assetManager_->GetEntityManager())));

	// 譖ｴ譁ｰ蠕後さ繝槭Φ繝牙ｮ溯｡・
	updateCommandInvoker_.ExecuteCommands();
}

void SceneObject::PreDraw() {
	// 繧ｫ繝｡繝ｩ譖ｴ譁ｰ
	AssetManager* assetManager = AssetManager::GetInstance();
	CameraManager* cameraManager = CameraManager::GetInstance();
	cameraManager->Update();

	// 3Dモデルのカメラ位置更新
	if (assetManager->GetEntityManager()->HasComponentStrage<ModelHandle>()) {
		auto& modelStrage = assetManager->GetEntityManager()->GetComponentStrage<ModelHandle>();
		for (auto& model : modelStrage) {
			const ModelRenderData* modelDataPtr = assetManager->GetModelRenderData(model.second.handle);
			GpuBufferPool* gpuBufferPool = assetManager->GetGpuBufferPool();
			for (const auto& meshRenderDataHandle : modelDataPtr->meshRenderDataHandles) {
				CameraForGPU* camera = gpuBufferPool->GetConstantBufferData<CameraForGPU>(meshRenderDataHandle.cameraPosBufferHandle);
				camera->cameraPosition = cameraManager->GetMainCamera().GetPosition();
			}
		}
	}
	
	// WVP陦悟・譖ｴ譁ｰ
	preDrawCommandInvoker_.AddCommand(std::make_unique<WvpTransformationCommand>(*(assetManager->GetEntityManager()), *cameraManager));
	preDrawCommandInvoker_.AddCommand(std::make_unique<SpritePivotUpdateCommand>(*(assetManager->GetEntityManager())));

	// 謠冗判蜑阪さ繝槭Φ繝牙ｮ溯｡・
	preDrawCommandInvoker_.ExecuteCommands();
}

void SceneObject::Draw() {
	// 蠖薙◆繧雁愛螳壹・謠冗判
	ColliderManager::GetInstance()->Draw();

	AssetManager* assetManager = AssetManager::GetInstance();
	drawCommandInvoker_.AddCommand(std::make_unique<AllEntityRenderingCommand>(*(assetManager->GetEntityManager())));


	// 謠冗判繧ｳ繝槭Φ繝牙ｮ溯｡・
	drawCommandInvoker_.ExecuteCommands();
}

void SceneObject::PostDraw() {
	// 謠冗判蠕後さ繝槭Φ繝牙ｮ溯｡・
	postDrawCommandInvoker_.ExecuteCommands();
}

void SceneObject::EndFrame() {
	if (isRequestStopScript_) {
		if (isRunningScript_) {
			isRunningScript_ = false;
			LoadScene(sceneName_);
		}
		isRequestStopScript_ = false;
	}
}

void SceneObject::Finalize() {

}

void SceneObject::LoadScene(const std::string& sceneName) {
	std::string sceneNameCopy = sceneName;
	// 諡｡蠑ｵ蟄千｢ｺ隱・
	if (!sceneNameCopy.ends_with(".json")) {
		sceneNameCopy += ".json";
	}

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("LoadScene: " + sceneNameCopy);
#endif // QFE_OPTIMIZE_OFF
	AssetManager* assetManager = AssetManager::GetInstance();
	assetManager->GetGpuBufferPool()->ReleaseAllConstantBuffers();
	EntityManager* entityManager = assetManager->GetEntityManager();
	entityManager->ResetEntiry();
	LuaScriptResourceManager::GetInstance()->Reset();
	AudioInterface::GetInstance()->StopAllSound();
	CsharpVirtualEnvironmentOnQFE::GetInstance()->ResetScripts();

	// 繧ｷ繝ｼ繝ｳ繝輔ぃ繧､繝ｫ繧帝幕縺・
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ifstream ifs(sceneFilePath + sceneNameCopy);
	if (!ifs.is_open()) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Faild load scene: " + sceneNameCopy, LogLevel::Error);
#endif // QFE_OPTIMIZE_OFF
		CameraManager::GetInstance()->Initialize();
		return;
	}

	nlohmann::json sceneJson;
	ifs >> sceneJson;
	ifs.close();
	// 繧ｷ繝ｼ繝ｳ蜷阪・蜿門ｾ・
	if (sceneJson.contains("sceneName")) {
		sceneName_ = sceneJson["sceneName"].get<std::string>();
	} else {
		sceneName_ = "NoNameScene";
	}

	// 繧ｨ繝ｳ繝・ぅ繝・ぅ縺ｮ逕滓・
	if (!sceneJson.contains("entities")) return;

	for (const auto& entityJson : sceneJson["entities"]) {
		uint32_t entityId = entityManager->CreateEntity();
		DeserializeEntity(entityId, entityJson);
	}

}

void SceneObject::SaveScene(const std::string& sceneName) {
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("SaveScene: " + sceneName);
#endif // QFE_OPTIMIZE_OFF
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	std::vector<uint32_t> entities = entityManager->GetActiveEntityIds();

	nlohmann::json sceneJson;
	sceneJson["sceneName"] = sceneName;

	for (auto entityId : entities) {
		nlohmann::json entityJson;
		SerializeEntity(entityId, entityJson);
		sceneJson["entities"].push_back(entityJson);
	}

	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ofstream ofs(sceneFilePath + sceneName + ".json");
	ofs << sceneJson.dump(4);
	ofs.close();
}

void SceneObject::ResetScene() {
	uniqueIdManager_.Reset();
}

void SceneObject::RunScene() {

	if (!isRunningScript_) {
#ifdef QFE_OPTIMIZE_OFF
		MyDebugLog::GetInstance()->DebugLogClear();
		MyDebugLog::GetInstance()->scriptLogs_.clear();
#endif // QFE_OPTIMIZE_OFF

		SaveScene(sceneName_);
		LoadScene(sceneName_);
		isRunningScript_ = true;
		LuaScriptResourceManager::GetInstance()->InitializeAllScripts();
		CsharpVirtualEnvironmentOnQFE::GetInstance()->RunAllScriptsFunction("Initialize");
		ColliderManager::GetInstance()->isRunning = true;
		LuaScriptResourceManager::GetInstance()->isRunningScript_ = true;
	}
}

void SceneObject::PauseScene() {
	isPauseScript_ = true;
}

void SceneObject::ResumeScene() {
	isPauseScript_ = false;
}

void SceneObject::StopScene() {
	if (isRequestStopScript_) { return; }
	isRequestStopScript_ = true;
	ColliderManager::GetInstance()->isRunning = false;
	LuaScriptResourceManager::GetInstance()->isRunningScript_ = false;
}

void SceneObject::AddEmptyObject() {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = "EmptyObject";
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddParticleEmitter(const std::string& modelName, uint32_t maxCount) {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	// ParticleComponent髴托ｽｽ陷会｣ｰ
	ParticleComponent particleComponent;
	particleComponent.modelName = modelName;
	particleComponent.maxParticleCount = maxCount;
	particleComponent.vartexBufferHandle = assetManager->LoadModelMesh(modelName);
	particleComponent.textureHandle = assetManager->LoadModelTexture(modelName);
	particleComponent.materialHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<Material>();
	particleComponent.particleGpuBufferHandle = assetManager->GetParticleGpuDataManager()->CreateParticleBuffer(maxCount);
	assetManager->GetEntityManager()->EmplaceComponent<ParticleComponent>(entityId, particleComponent);

	// 邵ｺ繝ｻ笆ｽ郢ｧ繧・・郢ｧ繝ｻ笆ｽ髴托ｽｽ陷会｣ｰ
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = modelName + "_ParticleEmitter";
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddModel(const std::string& modelName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	ModelHandle modelHandle;
	modelHandle.modelName = modelName;
	modelHandle.handle = assetManager->LoadModel(modelName);
	assetManager->GetEntityManager()->EmplaceComponent<ModelHandle>(entityId, modelHandle);
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = modelName;
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddSprite(const std::string& spriteName, float width, float height, int inEntityId, int layer, Vector2 pivot) {
	AssetManager* assetManager = AssetManager::GetInstance();
	// entityId隰悶・・ｮ螢ｹ窶ｲ邵ｺ繧・ｽ檎ｸｺ・ｰ邵ｺ譏ｴ・檎ｹｧ蜑・ｽｽ・ｿ邵ｺ繝ｻﾂ竏壺・邵ｺ莉｣・檎ｸｺ・ｰ隴・ｽｰ髫穂ｸ茨ｽｽ諛医・
	uint32_t entityId;
	if (inEntityId != -1) {
		entityId = static_cast<uint32_t>(inEntityId);
	} else {
		entityId = assetManager->GetEntityManager()->CreateEntity();
	}
	// SpriteData髴托ｽｽ陷会｣ｰ
	SpriteData spriteData;
	EntityManager* entityManager = assetManager->GetEntityManager();
	spriteData.layer = 0;
	spriteData.pivot = pivot;
	if (entityManager->HasComponentStrage<SpriteData>()) {
		spriteData.layer = static_cast<uint32_t>(entityManager->GetComponentStrage<SpriteData>().size());
	}
	if (layer != -1) {
		spriteData.layer = static_cast<uint32_t>(layer);
	}
	spriteData.textureName = spriteName;
	spriteData.textureHandle = assetManager->LoadTexture(spriteName);
	Vector2 textureSize = assetManager->GetTextureManager()->GetTextureSize(spriteData.textureHandle);
	spriteData.height = textureSize.y;
	spriteData.width = textureSize.x;
	if (width != 0.0f) {
		textureSize.x = width;
		spriteData.width = width;
	}
	if (height != 0.0f) {
		textureSize.y = height;
		spriteData.height = height;
	}
	spriteData.vertexBufferHandle = assetManager->GetSpriteManager()->CreateVertexBuffer(textureSize.x, textureSize.y);
	spriteData.wvpBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<TransformationMatrix>();
	assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(spriteData.wvpBufferHandle)->WVP = Matrix4x4::MakeIndentity4x4();
	spriteData.materialBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<Material>();
	Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(spriteData.materialBufferHandle);
	material->color = { 1.0f,1.0f,1.0f,1.0f };
	material->enableLighting = false;
	material->uvTransform = Matrix4x4::MakeIndentity4x4();
	spriteData.lightBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<DirectionalLight>();
	DirectionalLight* light = assetManager->GetGpuBufferPool()->GetConstantBufferData<DirectionalLight>(spriteData.lightBufferHandle);
	light->color = { 1.0f,1.0f,1.0f,1.0f };
	light->direction = { 0.0f,-1.0f,0.0f };
	light->intensity = 1.0f;
	// 郢ｧ・ｹ郢晏干ﾎ帷ｹｧ・､郢晏現繝ｧ郢晢ｽｼ郢ｧ・ｿ郢ｧ蛛ｵ縺顔ｹ晢ｽｳ郢昴・縺・ｹ昴・縺・ｸｺ・ｫ髴托ｽｽ陷会｣ｰ
	assetManager->GetEntityManager()->EmplaceComponent<SpriteData>(entityId, spriteData);

	// 邵ｺ繝ｻ笆ｽ郢ｧ繧・・郢ｧ繝ｻ笆ｽ髴托ｽｽ陷会｣ｰ
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = spriteName;
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddLuaScript(uint32_t entityId, const std::string& scriptName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->HasComponent<ScriptHandles>(entityId)) {
		ScriptHandles scriptHandles;
		LuaHandle scriptHandle;
		scriptHandle.scriptName_ = scriptName;
		scriptHandle.handle_ = LuaScriptResourceManager::GetInstance()->AddScript(entityId, scriptName);
		LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(scriptHandle.handle_);
		for (std::string& val : script->GetGlobalValuesList()) {
			sol::state* state = script->GetScript();
			sol::object obj = (*state)[val];
			if (obj.is<int>()) {
				int v = obj.as<int>();
				scriptHandle.intParams_[val] = v;
			} else if (obj.is<float>()) {
				float v = obj.as<float>();
				scriptHandle.floatParams_[val] = v;
			} else if (obj.is<bool>()) {
				bool v = obj.as<bool>();
				scriptHandle.boolParams_[val] = v;
			} else if (obj.is<std::string>()) {
				std::string v = obj.as<std::string>();
				scriptHandle.stringParams_[val] = v;
			}
		}
		scriptHandles.scriptHandles_.push_back(scriptHandle);
		entityManager->EmplaceComponent<ScriptHandles>(entityId, scriptHandles);
	} else {
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
		// 邵ｺ蜷ｶ縲堤ｸｺ・ｫ陷ｷ蠕個ｧ郢ｧ・ｹ郢ｧ・ｯ郢晢ｽｪ郢晏干繝ｨ邵ｺ蠕後＞郢ｧ・ｿ郢昴・繝｡邵ｺ霈費ｽ檎ｸｺ・ｦ邵ｺ繝ｻ・玖撻・ｴ陷ｷ蛹ｻ繝ｻ髴托ｽｽ陷会｣ｰ邵ｺ蜉ｱ竊醍ｸｺ繝ｻ
		for (const auto& sh : scriptHandles.scriptHandles_) {
			if (sh.scriptName_ == scriptName) {
				return;
			}
		}
		LuaHandle scriptHandle;
		scriptHandle.scriptName_ = scriptName;
		scriptHandle.handle_ = LuaScriptResourceManager::GetInstance()->AddScript(entityId, scriptName);
		scriptHandles.scriptHandles_.push_back(scriptHandle);
	}
}

void SceneObject::AddCsharpScript(uint32_t entityId, const std::string& className) {
	CsharpVirtualEnvironmentOnQFE* csharpEnv = CsharpVirtualEnvironmentOnQFE::GetInstance();
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->HasComponent<CsharpComponent>(entityId)) {
		// 隴・ｽｰ髫募臆・ｿ・ｽ陷会｣ｰ
		CsharpComponent csharpComponent;
		CsharpHandle csharpHandle;
		csharpHandle.className_ = className;
		csharpHandle.scriptIndex_ = csharpEnv->CreateScriptInstance(entityId, className);
		csharpComponent.csharpHandles_.push_back(csharpHandle);
		entityManager->EmplaceComponent<CsharpComponent>(entityId, csharpComponent);

	} else {
		// 隴鯉ｽ｢陝・･繝ｻ郢ｧ・ｳ郢晢ｽｳ郢晄亢繝ｻ郢晞亂ﾎｦ郢晏現竊馴恆・ｽ陷会｣ｰ
		CsharpComponent& csharpComponent = entityManager->GetComponent<CsharpComponent>(entityId);
		// 邵ｺ蜷ｶ縲堤ｸｺ・ｫ陷ｷ蠕個ｧ郢ｧ・ｯ郢晢ｽｩ郢ｧ・ｹ邵ｺ蠕後＞郢ｧ・ｿ郢昴・繝｡邵ｺ霈費ｽ檎ｸｺ・ｦ邵ｺ繝ｻ・玖撻・ｴ陷ｷ蛹ｻ繝ｻ髴托ｽｽ陷会｣ｰ邵ｺ蜉ｱ竊醍ｸｺ繝ｻ
		for (const auto& handles : csharpComponent.csharpHandles_) {
			if (handles.className_ == className) {
#ifdef QFE_OPTIMIZE_OFF
				DebugLog("Csharp class " + className + " is already attached to entity " + std::to_string(entityId), LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
				return;
			}
		}

		CsharpHandle csharpHandle;
		csharpHandle.className_ = className;
		csharpHandle.scriptIndex_ = csharpEnv->CreateScriptInstance(entityId, className);
		csharpComponent.csharpHandles_.push_back(csharpHandle);
	}
}

uint32_t SceneObject::AddEntity(const std::string& entityName) {
	AssetManager* assetManager = AssetManager::GetInstance();
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("AddEntity: " + entityName);
#endif // QFE_OPTIMIZE_OFF

	// 隴鯉ｽ｢邵ｺ・ｫ髫ｱ・ｭ邵ｺ・ｿ髴趣ｽｼ郢ｧ阮吮味邵ｺ阮吮・邵ｺ蠕娯旺郢ｧ荵昴♀郢晢ｽｳ郢昴・縺・ｹ昴・縺・惺髦ｪ竊醍ｹｧ蟲ｨ笳守ｹｧ蠕鯉ｽ帝恆譁絶・
#ifdef _NODEBUG
	if (loadEntities_.find(entityName) != loadEntities_.end()) {
		// Entity邵ｺ・ｮ騾墓ｻ薙・
		uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
		DeserializeEntity(entityId, loadEntities_[entityName]);
		return entityId;
	}
#endif // _NODEBUG

	// Entity邵ｺ・ｮ郢昜ｻ｣縺帷ｹｧ蝣､・ｵ繝ｻ竏ｩ驕ｶ荵昶ｻ
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
	std::ifstream ifs(sceneFilePath + entityName);
	if (!ifs.is_open()) {
		std::string errorMsg = "FaildOpenFile: " + sceneFilePath + entityName;
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(errorMsg, LogLevel::Error);
#endif // QFE_OPTIMIZE_OFF
		assert(false && "Faild Open Entity File.");
	}
	// Entity邵ｺ・ｮ陟包ｽｩ陷医・
	nlohmann::json sceneJson;
	ifs >> sceneJson;
	ifs.close();

	// Entity邵ｺ・ｮ騾墓ｻ薙・
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	DeserializeEntity(entityId, sceneJson);
	return entityId;
}

uint32_t SceneObject::RunTimeAddEntity(const std::string& entityName) {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	uint32_t entityId = AddEntity(entityName);
	// 郢ｧ・ｹ郢ｧ・ｯ郢晢ｽｪ郢晏干繝ｨ陋ｻ譎・ｄ陋ｹ繝ｻ
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(entityId) && isRunningScript_) {
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
		for (const auto& sh : scriptHandles.scriptHandles_) {
			LuaScriptResourceManager::GetInstance()->InitializeScript(sh.handle_);
		}
	}
	if (entityManager->HasComponent<CsharpComponent>(entityId) && isRunningScript_) {
		CsharpComponent& csharpComponent = entityManager->GetComponent<CsharpComponent>(entityId);
		for (const auto& ch : csharpComponent.csharpHandles_) {
			CsharpVirtualEnvironmentOnQFE::GetInstance()->RunScriptFunction(ch.scriptIndex_, "Initialize");
		}
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("RunTimeAddEntity Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms");
#endif // QFE_OPTIMIZE_OFF
	return entityId;
}

void SceneObject::RunTimeAddLuaScript(uint32_t entityId, const std::string& scriptName) {
	AddLuaScript(entityId, scriptName);
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(entityId) && isRunningScript_) {
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
		for (const auto& sh : scriptHandles.scriptHandles_) {
			if (sh.scriptName_ == scriptName) {
				LuaScriptResourceManager::GetInstance()->InitializeScript(sh.handle_);
				break;
			}
		}
	}
}

void SceneObject::DeleteEntity(uint32_t entityId) {
	frameStartCommandInvoker_.AddCommand(std::make_unique<DeleteSceneEntityCommand>(*(assetManager_->GetEntityManager()), entityId));
}

void SceneObject::CopyEntity(uint32_t sourceEntityId) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->IsActiveEntity(sourceEntityId)) {
		assert(false && "Entity is not active");
		return;
	}
	nlohmann::json entityJson;
	SerializeEntity(sourceEntityId, entityJson);
	uint32_t newEntityId = entityManager->CreateEntity();
	DeserializeEntity(newEntityId, entityJson);
}

void SceneObject::ChangeEntityModel(uint32_t entityId, const std::string& modelName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	// 郢ｧ・ｨ郢晢ｽｳ郢昴・縺・ｹ昴・縺・ｸｺ蠕湖皮ｹ昴・ﾎ晉ｹｧ蜻域亜邵ｺ・｣邵ｺ・ｦ邵ｺ繝ｻ竊醍ｸｺ莉｣・檎ｸｺ・ｰ闖ｴ霈費ｽらｸｺ蜉ｱ竊醍ｸｺ繝ｻ
	if (!entityManager->HasComponent<ModelHandle>(entityId)) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("ChangeModel entity does not have ModelRenderData", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}

	ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
	modelHandle.modelName = modelName;
	modelHandle.handle = assetManager->LoadModel(modelName);
}

void SceneObject::ChangeEntityMesh(uint32_t entityId, const std::string& meshName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	// 繧ｨ繝ｳ繝・ぅ繝・ぅ縺後Δ繝・Ν繧呈戟縺｣縺ｦ縺・↑縺代ｌ縺ｰ菴輔ｂ縺励↑縺・
	if (!entityManager->HasComponent<ModelHandle>(entityId)) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("ChangeMesh entity does not have ModelRenderData", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
	ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
	// 繝｡繝・す繝･縺悟ｭ伜惠縺励↑縺代ｌ縺ｰ菴輔ｂ縺励↑縺・
	if (modelData->meshRenderDataHandles.size() == 0) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("ChangeMesh model does not have mesh", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	modelData->meshRenderDataHandles[0].vertexBufferHandle = assetManager_->LoadModelMesh(meshName);
}

void SceneObject::SaveEntity(uint32_t entityId, const std::string& entityFileName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->IsActiveEntity(entityId)) {
		assert(false && "Entity is not active");
		return;
	}
	nlohmann::json entityJson;
	SerializeEntity(entityId, entityJson);
	std::string entityFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
	std::ofstream ofs(entityFilePath + entityFileName + ".json");
	ofs << entityJson.dump(4);
	ofs.close();
}

void SceneObject::ParentChild(uint32_t parentId, uint32_t childId) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->IsActiveEntity(parentId) || !entityManager->IsActiveEntity(childId)) {
		assert(false && "Entity is not active");
		return;
	}
	if (!entityManager->HasComponent<SceneObjectData>(parentId) || !entityManager->HasComponent<SceneObjectData>(childId)) {
		assert(false && "Entity does not have SceneObjectData");
		return;
	}
	SceneObjectData& parentSceneObjectData = entityManager->GetComponent<SceneObjectData>(parentId);
	SceneObjectData& childSceneObjectData = entityManager->GetComponent<SceneObjectData>(childId);
	if (parentSceneObjectData.uniqueId == 0 || childSceneObjectData.uniqueId == 0) {
		assert(false && "Entity does not have uniqueId");
		return;
	}
	if (!entityManager->HasComponent<ParentData>(childId)) {
		ParentData parentData;
		parentData.parentId = parentSceneObjectData.uniqueId;
		entityManager->EmplaceComponent<ParentData>(childId, parentData);
	} else {
		ParentData& parentData = entityManager->GetComponent<ParentData>(childId);
		parentData.parentId = parentSceneObjectData.uniqueId;
	}

	if (!entityManager->HasComponent<Transform>(parentId) || !entityManager->HasComponent<Transform>(childId)) {
		return;
	}
	Transform& parentTransform = entityManager->GetComponent<Transform>(parentId);
	Transform& childTransform = entityManager->GetComponent<Transform>(childId);
	childTransform.translate -= parentTransform.translate;
}

void SceneObject::Unparent(uint32_t childId) {
	if (!AssetManager::GetInstance()->GetEntityManager()->HasComponent<ParentData>(childId)) {
		return;
	}

	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();

	ParentData& parentData = entityManager->GetComponent<ParentData>(childId);
	uint32_t parentId = 0;
	bool isFound = false;
	if (entityManager->HasComponentStrage<SceneObjectData>()) {
		auto& strage = entityManager->GetComponentStrage<SceneObjectData>();
		for (const auto& [id, sceneObjData] : strage) {
			if (sceneObjData.uniqueId == parentData.parentId) {
				parentId = id;
				isFound = true;
				break;
			}
		}
	}
	if (!isFound) { return; }
	if (!entityManager->HasComponent<Transform>(parentId) || !entityManager->HasComponent<Transform>(childId)) {
		entityManager->RemoveComponent<ParentData>(childId);
		return;
	}
	//Transform& parentTransform = entityManager->GetComponent<Transform>(parentId);
	//Transform& childTransform = entityManager->GetComponent<Transform>(childId);
	// childTransform.translate += parentTransform.translate;

	entityManager->RemoveComponent<ParentData>(childId);
}

void SceneObject::SerializeEntity(uint32_t entityId, nlohmann::json& entityJson) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	entityJson = entityManager->SerializeEntityComponents(entityId);
}

void SceneObject::DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	usedEntityId_.insert(entityId);

	// 蜷・さ繝ｳ繝昴・繝阪Φ繝医・蠕ｩ蜈・
	if (entityJson.contains("SpriteData")) {
		SpriteData spriteData;
		spriteData.Deserialize(entityJson["SpriteData"]);
		AddSprite(spriteData.textureName, spriteData.width, spriteData.height, static_cast<int>(entityId), static_cast<int>(spriteData.layer), spriteData.pivot);
	}
	if (entityJson.contains("Transform")) {
		entityManager->EmplaceComponent<Transform>(entityId);
		Transform& transform = entityManager->GetComponent<Transform>(entityId);
		transform.Deserialize(entityJson["Transform"]);
	}
	if (entityJson.contains("ParentData")) {
		entityManager->EmplaceComponent<ParentData>(entityId);
		ParentData& parentData = entityManager->GetComponent<ParentData>(entityId);
		parentData.Deserialize(entityJson["ParentData"]);
	}
	if (entityJson.contains("CameraData")) {
		entityManager->EmplaceComponent<CameraData>(entityId);
		CameraData& cameraData = entityManager->GetComponent<CameraData>(entityId);
		cameraData.Deserialize(entityJson["CameraData"]);
	}
	if (entityJson.contains("ModelHandle")) {
		entityManager->EmplaceComponent<ModelHandle>(entityId);
		ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
		modelHandle.Deserialize(entityJson["ModelHandle"]);
	}
	if (entityJson.contains("SceneObjectData")) {
		entityManager->EmplaceComponent<SceneObjectData>(entityId);
		SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
		sceneObjectData.Deserialize(entityJson["SceneObjectData"]);
		uniqueIdManager_.AddUsedID(sceneObjectData.uniqueId);
	}
	if (entityJson.contains("Force")) {
		entityManager->EmplaceComponent<Force>(entityId);
		Force& force = entityManager->GetComponent<Force>(entityId);
		force.Deserialize(entityJson["Force"]);
	}
	if (entityJson.contains("SphereColliderData")) {
		entityManager->EmplaceComponent<SphereColliderData>(entityId);
		SphereColliderData& sphereColliderData = entityManager->GetComponent<SphereColliderData>(entityId);
		sphereColliderData.Deserialize(entityJson["SphereColliderData"]);
	}
	if (entityJson.contains("AABBColliderData")) {
		entityManager->EmplaceComponent<AABBColliderData>(entityId);
		AABBColliderData& aabbColliderData = entityManager->GetComponent<AABBColliderData>(entityId);
		aabbColliderData.Deserialize(entityJson["AABBColliderData"]);
	}
	if (entityJson.contains("CsharpComponent")) {
		std::vector<std::string> classNames;
		if (entityJson["CsharpComponent"].contains("CsharpHandles")) {
			// C#縺ｮ繧ｯ繝ｩ繧ｹ蜷阪°繧峨う繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ繧堤函謌・
			for (const auto& handle : entityJson["CsharpComponent"]["CsharpHandles"]) {
				if (handle.contains("ClassName")) {
#ifdef QFE_OPTIMIZE_OFF
					DebugLog("Load Csharp Script: " + handle["ClassName"].get<std::string>());
#endif // QFE_OPTIMIZE_OFF
					AddCsharpScript(entityId, handle["ClassName"].get<std::string>());
				}
			}
		}
	}
	if (entityJson.contains("ScriptHandle")) {
		std::vector<std::string> scriptNames;
		if (entityJson.contains("ScriptHandle") && entityJson["ScriptHandle"].contains("scriptHandles")) {
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
#ifdef QFE_OPTIMIZE_OFF
					DebugLog("Load Script: " + handle["scriptName"].get<std::string>());
#endif // QFE_OPTIMIZE_OFF
					AddLuaScript(entityId, handle["scriptName"].get<std::string>());
				}
			}
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
					ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
					std::vector<uint32_t> luaHandles;
					for (auto& sh : scriptHandles.scriptHandles_) {
						luaHandles.push_back(sh.handle_);
					}
					scriptHandles.Deserialize(entityJson["ScriptHandle"]);
					for (size_t i = 0; i < luaHandles.size(); ++i) {
						scriptHandles.scriptHandles_[i].handle_ = luaHandles[i];
					}
					for (LuaHandle& hl : scriptHandles.scriptHandles_) {
						LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(hl.handle_);
						sol::environment& env = script->GetEnvironment();
						for (const auto& [key, val] : hl.intParams_) {
							env[key] = val;
						}
						for (const auto& [key, val] : hl.floatParams_) {
							env[key] = val;
						}
						for (const auto& [key, val] : hl.boolParams_) {
							env[key] = val;
						}
						for (const auto& [key, val] : hl.stringParams_) {
							env[key] = val;
						}


						script->SetPriority(hl.priority_);
					}
				}
			}

		}
	}
}

uint32_t SceneObject::GetEntityByName(const std::string& entityName) const {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	std::vector<uint32_t> entities = entityManager->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			const SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.name == entityName) {
				return entityId;
			}
		}
	}
	assert(false && "Entity Not Found");
	return 0;
}

uint32_t SceneObject::GetEntityByUniqeID(uint32_t uniqueId) const {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	std::vector<uint32_t> entities = entityManager->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			const SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.uniqueId == uniqueId) {
				return entityId;
			}
		}
	}
	assert(false && "Entity Not Found");
	return 0;
}


