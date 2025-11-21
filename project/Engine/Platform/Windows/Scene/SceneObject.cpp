#include "SceneObject.h"
#include "Assets/AssetManager.h"
#include <cassert>

#include "Core/EngineGlobalValue.h"

#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"
#include "Assets/Script/CsharpVirtualEnvironmentOnQFE.h"
#include "Assets/Script/Data/CsharpComponent.h"
#include "Collider/ColliderManager.h"
#include "Audio/AudioInterface.h"

#include "Assets/3DModel/Data/ModelHandle.h"
#include "Data/SceneObjectData.h"
#include "Assets/Script/Data/ScriptHandle.h"
#include "Physics/PhysicsManager.h"
#include "Collider/Data/SphereColliderData.h"
#include "Core/Math/ParentData.h"
#include "Camera/Data/CameraData.h"

#include <fstream>
#include <execution>
#include <nlohmann/json.hpp>

#include "Assets/3DModel/Loader/AssimpModelLoader.h"
#include "Assets/3DModel/Data/ModelHandle.h"
#include "Assets/Sprite/Data/SpriteData.h"

#include "Renderer/ModelRenderer.h"
#include "Renderer/SpriteRenderer.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

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
}

void SceneObject::Update() {

	// スクリプト更新
	if (isRunningScript_ && !isPauseScript_) {
		LuaScriptResourceManager::GetInstance()->UpdateAllScripts();
		CsharpVirtualEnvironmentOnQFE::GetInstance()->RunAllScriptsFunction("Update");
		PhysicsManager::GetInstance()->Update();
	}

	// コライダー更新
	ColliderManager::GetInstance()->Update();

	// ユニークIDが未設定なら設定する
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	std::vector<uint32_t> entities = entityManager->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.uniqueId == 0) {
				sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
			} else {
				uniqueIdManager_.AddUsedID(sceneObjectData.uniqueId);
			}
		}
	}
	// ユニークIDが重複していたら再設定する
	std::set<uint32_t> checkIds;
	for (auto entityId : entities) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
			if (checkIds.find(sceneObjectData.uniqueId) != checkIds.end()) {
				sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
			}
			checkIds.insert(sceneObjectData.uniqueId);
		}
	}

	// ワールド行列更新(wvpを別コンポーネントにする)
	AssetManager* assetManager = AssetManager::GetInstance();
	for (auto entityId : entities) {
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& transform = entityManager->GetComponent<Transform>(entityId);
			// モデルのワールド行列更新
			if (entityManager->HasComponent<ModelHandle>(entityId)) {
				ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
				const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
				// メッシュごとにワールド行列更新
				for (const auto& meshData : modelData->meshRenderDataHandles) {
					TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(meshData.wpvBufferHandle);
					wpvMatrix->World = Matrix4x4::MakeAffineMatrix(
						transform.scale,
						transform.rotate,
						transform.translate
					);
				}
			}
			// スプライトのワールド行列更新
			if (assetManager->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
				SpriteData& spriteData = assetManager->GetEntityManager()->GetComponent<SpriteData>(entityId);
				TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(spriteData.wvpBufferHandle);
				wpvMatrix->World = Matrix4x4::MakeAffineMatrix(
					transform.scale,
					transform.rotate,
					transform.translate
				);
			}
		}
	}

	// ペアレント子関係更新
	for (auto entityId : entities) {
		if (assetManager->GetEntityManager()->HasComponent<ParentData>(entityId)) {
			ParentData& parentData = assetManager->GetEntityManager()->GetComponent<ParentData>(entityId);

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
			if (!isFound) { continue; }

			if (assetManager->GetEntityManager()->HasComponent<Transform>(parentId)) {
				Transform& parentTransform = assetManager->GetEntityManager()->GetComponent<Transform>(parentId);
				// モデルのワールド行列更新
				if (assetManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
					ModelHandle& modelHandle = assetManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
					const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
					// メッシュごとにワールド行列更新
					for (const auto& meshData : modelData->meshRenderDataHandles) {
						TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(meshData.wpvBufferHandle);
						wpvMatrix->World = Matrix4x4::Multiply(wpvMatrix->World, Matrix4x4::MakeAffineMatrix(
							parentTransform.scale, parentTransform.rotate, parentTransform.translate));
					}
				}
				// スプライトのワールド行列更新
				if (assetManager->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
					SpriteData& spriteData = assetManager->GetEntityManager()->GetComponent<SpriteData>(entityId);
					TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(spriteData.wvpBufferHandle);
					wpvMatrix->World = Matrix4x4::Multiply(wpvMatrix->World, Matrix4x4::MakeAffineMatrix(
						parentTransform.scale, parentTransform.rotate, parentTransform.translate));
				}
			}

		}
	}

	// スプライトサイズ更新
	if (assetManager_->GetEntityManager()->HasComponentStrage<SpriteData>()) {
		const auto& spriteStrage = assetManager_->GetEntityManager()->GetComponentStrage<SpriteData>();
		for (const auto& [entityId, sprite] : spriteStrage) {
			Vector2 nowSize = assetManager_->GetSpriteManager()->GetSpriteSize(sprite.vertexBufferHandle);
			if (sprite.width != nowSize.x || sprite.height != nowSize.y) {
				assetManager_->GetSpriteManager()->ResizeSprite(sprite.vertexBufferHandle, sprite.width, sprite.height);
			}
		}
	}
}

void SceneObject::PreDraw() {
	// カメラ更新
	CameraManager* cameraManager = CameraManager::GetInstance();
	cameraManager->Update();

	// ビュー行列更新
	AssetManager* assetManager = AssetManager::GetInstance();
	std::vector<uint32_t> entities = assetManager->GetEntityManager()->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (assetManager->GetEntityManager()->HasComponent<Transform>(entityId)) {
			// モデルのワールド行列更新
			if (assetManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
				ModelHandle& modelHandle = assetManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
				const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
				// メッシュごとにワールド行列更新
				for (const auto& meshData : modelData->meshRenderDataHandles) {
					TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(meshData.wpvBufferHandle);
					wpvMatrix->WVP = cameraManager->GetMainCamera().GetWorldViewProjectionMatrix(wpvMatrix->World, CameraType::Perspective);
				}
			}
			// スプライトのワールド行列更新
			if (assetManager->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
				SpriteData& spriteData = assetManager->GetEntityManager()->GetComponent<SpriteData>(entityId);
				TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(spriteData.wvpBufferHandle);
				wpvMatrix->WVP = cameraManager->GetMainCamera().GetWorldViewProjectionMatrix(wpvMatrix->World, CameraType::Orthographic);
			}
		}
	}

	// スプライトのピボット更新
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (entityManager->HasComponentStrage<SpriteData>()) {
		auto& strage = entityManager->GetComponentStrage<SpriteData>();
		for (auto& [id, data] : strage) {
			VertexData* vertexData = assetManager->GetSpriteManager()->GetVertexData(data.vertexBufferHandle);
			float w = data.width;
			float h = data.height;
			// ピボットによる位置調整
			Vector2 pivotOffset = Vector2(0.0f, 0.0f);
			pivotOffset.x = -w * data.pivot.x;
			pivotOffset.y = -h * data.pivot.y;
			vertexData[0].position = { pivotOffset.x, pivotOffset.y, 0.0f,1.0f };               // 左上
			vertexData[1].position = { w + pivotOffset.x, pivotOffset.y, 0.0f ,1.0f };       // 右上
			vertexData[2].position = { pivotOffset.x, h + pivotOffset.y, 0.0f ,1.0f };       // 左下
			vertexData[3].position = { w + pivotOffset.x, h + pivotOffset.y, 0.0f ,1.0f };   // 右下
			vertexData[4].position = { pivotOffset.x, h + pivotOffset.y, 0.0f,1.0f };       // 左下
			vertexData[5].position = { w + pivotOffset.x, pivotOffset.y, 0.0f ,1.0f };       // 右上
		}
	}
}

void SceneObject::Draw() {
	// コライダー描画
	ColliderManager::GetInstance()->Draw();

	// モデル描画
	if (assetManager_->GetEntityManager()->HasComponentStrage<ModelHandle>()) {
		const auto& modelStrage = assetManager_->GetEntityManager()->GetComponentStrage<ModelHandle>();
		for (const auto& [entityId, model] : modelStrage) {
			Render::Model::DrawModel(model.handle);
		}
	}
	// スプライト描画（layer順にソート）
	if (assetManager_->GetEntityManager()->HasComponentStrage<SpriteData>()) {
		const auto& spriteStrage = assetManager_->GetEntityManager()->GetComponentStrage<SpriteData>();
		// 一時的なvectorにコピー
		std::vector<std::pair<uint32_t, SpriteData>> sortedSprites(spriteStrage.begin(), spriteStrage.end());
		// layerで昇順ソート
		std::sort(sortedSprites.begin(), sortedSprites.end(),
			[](const auto& a, const auto& b) {
				return a.second.layer < b.second.layer;
			});
		// ソート済み順で描画
#ifdef _DEBUG
		DebugLog(std::format("DrawSprite"));
#endif // _DEBUG
		for (const auto& [entityId, sprite] : sortedSprites) {
			Render::Sprite::DrawSprite(entityId);
#ifdef _DEBUG
			DebugLog(std::format("Sprite: {} ID: {}",sprite.textureName,entityId));
#endif // _DEBUG
		}
	}
}

void SceneObject::PostDraw() {
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
	// 拡張子がなければ追加
	if (!sceneNameCopy.ends_with(".json")) {
		sceneNameCopy += ".json";
	}

#ifdef _DEBUG
	DebugLog("LoadScene: " + sceneNameCopy);
#endif // _DEBUG
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	entityManager->ResetEntiry();
	LuaScriptResourceManager::GetInstance()->Reset();
	AudioInterface::GetInstance()->StopAllSound();
	CsharpVirtualEnvironmentOnQFE::GetInstance()->ResetScripts();

	// シーンファイルのパスを組み立て
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ifstream ifs(sceneFilePath + sceneNameCopy);
	if (!ifs.is_open()) {
		assert(false && "FaildOpenFile");
	}

	nlohmann::json sceneJson;
	ifs >> sceneJson;
	ifs.close();
	// シーン名の設定
	if (sceneJson.contains("sceneName")) {
		sceneName_ = sceneJson["sceneName"].get<std::string>();
	} else {
		sceneName_ = "NoNameScene";
	}

	// エンティティの復元
	if (!sceneJson.contains("entities")) return;

	for (const auto& entityJson : sceneJson["entities"]) {
		uint32_t entityId = entityManager->CreateEntity();
		DeserializeEntity(entityId, entityJson);
	}

}

void SceneObject::SaveScene(const std::string& sceneName) {
#ifdef _DEBUG
	DebugLog("SaveScene: " + sceneName);
#endif // _DEBUG
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
#ifdef _DEBUG
		MyDebugLog::GetInstance()->DebugLogClear();
		MyDebugLog::GetInstance()->scriptLogs_.clear();
#endif // _DEBUG

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
	// entityId指定があればそれを使う、なければ新規作成
	uint32_t entityId;
	if (inEntityId != -1) {
		entityId = static_cast<uint32_t>(inEntityId);
	} else {
		entityId = assetManager->GetEntityManager()->CreateEntity();
	}
	// SpriteData追加
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
	spriteData.wvpBufferHandle = assetManager->GetWpvBufferManager()->CreateBuffer();
	assetManager->GetWpvBufferManager()->GetBufferData(spriteData.wvpBufferHandle)->World = Matrix4x4::MakeIndentity4x4();
	spriteData.materialBufferHandle = assetManager->GetMaterialBufferManager()->CreateBuffer();
	Material* material = assetManager->GetMaterialBufferManager()->GetBufferData(spriteData.materialBufferHandle);
	material->color = { 1.0f,1.0f,1.0f,1.0f };
	material->enableLighting = false;
	material->uvTransform = Matrix4x4::MakeIndentity4x4();
	spriteData.lightBufferHandle = assetManager->GetLightBufferManager()->CreateBuffer();
	DirectionalLight* light = assetManager->GetLightBufferManager()->GetBufferData(spriteData.lightBufferHandle);
	light->color = { 1.0f,1.0f,1.0f,1.0f };
	light->direction = { 0.0f,-1.0f,0.0f };
	light->intensity = 1.0f;
	// スプライトデータをエンティティに追加
	assetManager->GetEntityManager()->EmplaceComponent<SpriteData>(entityId, spriteData);

	// いつものやつ追加
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
		// すでに同じスクリプトがアタッチされている場合は追加しない
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
		// 新規追加
		CsharpComponent csharpComponent;
		CsharpHandle csharpHandle;
		csharpHandle.className_ = className;
		csharpHandle.scriptIndex_ = csharpEnv->CreateScriptInstance(entityId, className);
		csharpComponent.csharpHandles_.push_back(csharpHandle);
		entityManager->EmplaceComponent<CsharpComponent>(entityId, csharpComponent);

	} else {
		// 既存のコンポーネントに追加
		CsharpComponent& csharpComponent = entityManager->GetComponent<CsharpComponent>(entityId);
		// すでに同じクラスがアタッチされている場合は追加しない
		for (const auto& handles : csharpComponent.csharpHandles_) {
			if (handles.className_ == className) {
#ifdef _DEBUG
				DebugLog("Csharp class " + className + " is already attached to entity " + std::to_string(entityId), LogLevel::Warning);
#endif // _DEBUG
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
#ifdef _DEBUG
	DebugLog("AddEntity: " + entityName);
#endif // _DEBUG

	// 既に読み込んだことがあるエンティティ名ならそれを返す
#ifdef _NODEBUG
	if (loadEntities_.find(entityName) != loadEntities_.end()) {
		// Entityの生成
		uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
		DeserializeEntity(entityId, loadEntities_[entityName]);
		return entityId;
	}
#endif // _NODEBUG

	// Entityのパスを組み立て
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
	std::ifstream ifs(sceneFilePath + entityName);
	if (!ifs.is_open()) {
		std::string errorMsg = "FaildOpenFile: " + sceneFilePath + entityName;
#ifdef _DEBUG
		DebugLog(errorMsg, LogLevel::Error);
#endif // _DEBUG
		assert(false && "Faild Open Entity File.");
	}
	// Entityの復元
	nlohmann::json sceneJson;
	ifs >> sceneJson;
	ifs.close();

	// Entityの生成
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	DeserializeEntity(entityId, sceneJson);
	// 読み込んだエンティティ名を保存
	loadEntities_[entityName] = sceneJson;

	return entityId;
}

uint32_t SceneObject::RunTimeAddEntity(const std::string& entityName) {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	uint32_t entityId = AddEntity(entityName);
	// スクリプト初期化
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(entityId) && isRunningScript_) {
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
		for (const auto& sh : scriptHandles.scriptHandles_) {
			LuaScriptResourceManager::GetInstance()->InitializeScript(sh.handle_);
		}
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
#ifdef _DEBUG
	DebugLog("RunTimeAddEntity Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms");
#endif // _DEBUG
	return entityId;
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
	// エンティティがモデルを持っていなければ何もしない
	if (!entityManager->HasComponent<ModelHandle>(entityId)) {
#ifdef _DEBUG
		DebugLog("ChangeModel entity does not have ModelRenderData", LogLevel::Warning);
#endif // _DEBUG
		return;
	}

	ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
	modelHandle.modelName = modelName;
	modelHandle.handle = assetManager->LoadModel(modelName);
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

	// 必要なコンポーネントを追加
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
			// C#スクリプトの復元
			for (const auto& handle : entityJson["CsharpComponent"]["CsharpHandles"]) {
				if (handle.contains("ClassName")) {
#ifdef _DEBUG
					DebugLog("Load Csharp Script: " + handle["ClassName"].get<std::string>());
#endif // _DEBUG
					AddCsharpScript(entityId, handle["ClassName"].get<std::string>());
				}
			}
		}
	}
	if (entityJson.contains("ScriptHandle")) {
		std::vector<std::string> scriptNames;
		if (entityJson.contains("ScriptHandle") && entityJson["ScriptHandle"].contains("scriptHandles")) {

			// スクリプトの復元
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
#ifdef _DEBUG
					DebugLog("Load Script: " + handle["scriptName"].get<std::string>());
#endif // _DEBUG
					AddLuaScript(entityId, handle["scriptName"].get<std::string>());
				}
			}
			// グローバル変数の復元
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
					ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
					// グローバル変数の復元準備
					std::vector<uint32_t> luaHandles;
					for (auto& sh : scriptHandles.scriptHandles_) {
						luaHandles.push_back(sh.handle_);
					}
					scriptHandles.Deserialize(entityJson["ScriptHandle"]);
					for (size_t i = 0; i < luaHandles.size(); ++i) {
						scriptHandles.scriptHandles_[i].handle_ = luaHandles[i];
					}

					// 後付け情報の復元
					for (LuaHandle& hl : scriptHandles.scriptHandles_) {
						// グローバル変数の復元
						LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(hl.handle_);
						sol::state* state = script->GetScript();
						for (const auto& [key, val] : hl.intParams_) {
							(*state)[key] = val;
						}
						for (const auto& [key, val] : hl.floatParams_) {
							(*state)[key] = val;
						}
						for (const auto& [key, val] : hl.boolParams_) {
							(*state)[key] = val;
						}
						for (const auto& [key, val] : hl.stringParams_) {
							(*state)[key] = val;
						}

						// 優先度の復元
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
