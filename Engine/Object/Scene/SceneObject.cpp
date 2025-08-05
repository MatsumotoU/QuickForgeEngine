#include "SceneObject.h"
#include "Base/EngineCore.h"
#include "Model/Model.h"

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

SceneObject::SceneObject(EngineCore* enginecore, const std::string& sceneName) 
#ifdef _DEBUG
	: debugCamera_(enginecore)
#endif 
{
	sceneName_ = sceneName;
	engineCore_ = enginecore;
	mainCamera_.Initialize(enginecore->GetWinApp());
#ifdef _DEBUG
	debugCamera_.Initialize(enginecore);
#endif // _DEBUG

	destroyedGameObjects_.clear();
	enginecore->GetGraphRenderer()->SetCamera(&mainCamera_);
}

SceneObject::~SceneObject() {
	engineCore_->GetGraphRenderer()->DeleteCamera(&mainCamera_);
}

void SceneObject::Initialize() {
	// Initialize the scene
	for (const auto& gameObject : gameObjects_) {
		gameObject->Init();
	}
}

void SceneObject::Update() {
	// オブジェクト削除
	if (!destroyedGameObjects_.empty()) {
		for (auto dit = destroyedGameObjects_.begin(); dit != destroyedGameObjects_.end(); ++dit) {
			for (auto git = gameObjects_.begin(); git != gameObjects_.end(); ++git) {
				if (git->get() == *dit) {
					if (engineCore_ && *dit) {
						std::string key = (*dit)->GetName();
						engineCore_->GetLuaScriptManager()->RemoveScript(key);
					}
					gameObjects_.erase(git);
					break; // 一度削除したらループを抜ける
				}
			}
		}
		destroyedGameObjects_.clear();
	}

	// カメラの更新
	mainCamera_.Update();
#ifdef _DEBUG
	debugCamera_.Update();
	mainCamera_ = debugCamera_.camera_;
#endif // _DEBUG

	// シーン更新
	for (const auto& gameObject : gameObjects_) {
		gameObject->Update();
	}
}

void SceneObject::Draw() {
#ifdef _DEBUG
	engineCore_->GetGraphRenderer()->DrawGrid(50.0f, 50);
#endif // _DEBUG

	// シーン描画
	for (const auto& gameObject : gameObjects_) {
		gameObject->Draw();
	}
}

nlohmann::json SceneObject::Serialize() const {
#ifdef _DEBUG
	DebugLog("SaveObject");
#endif // _DEBUG

	nlohmann::json j;
	j["sceneName"] = sceneName_;
	j["objects"] = nlohmann::json::array();
	for (const auto& obj : gameObjects_) {
		j["objects"].push_back(obj->Serialize());
	}
	return j;
}

std::unique_ptr<SceneObject> SceneObject::Deserialize(const nlohmann::json& j, EngineCore* engineCore, const std::string& modelFileDirectory) {
	std::string sceneName = j.value("sceneName", "UnnamedScene");
	auto sceneObj = std::make_unique<SceneObject>(engineCore, sceneName);
	sceneObj->Initialize();
	sceneObj->SetSceneName(j.value("sceneName", "UnnamedScene"));

	// オブジェクト解凍
	if (j.contains("objects")) {
		for (const auto& objJson : j["objects"]) {
			std::string type = objJson.value("type", "");

			// Model
			if (type == "Model") {
				auto model = Model::Deserialize(objJson, engineCore, &(sceneObj->mainCamera_));
				model->LoadModel(modelFileDirectory, model->GetModelFileName(),COORDINATESYSTEM_HAND_RIGHT);
				if (!model->GetAttachedScriptName().empty()) {
					std::string scriptPath = "Resources/Scripts/" + model->GetAttachedScriptName();
					std::string key = model->GetName();
					if (!scriptPath.empty()) {
						engineCore->GetLuaScriptManager()->AddGameObjScript(key, model.get(), scriptPath);
					}
				}
				sceneObj->GetGameObjects().push_back(std::move(model));
			}

			// Billboard
			else if (type == "Billboard") {
				auto billboard = Billboard::Deserialize(objJson, engineCore, &(sceneObj->mainCamera_));
				billboard->Init();
				billboard->SetTextureHandle( engineCore->GetTextureManager()->LoadTexture(modelFileDirectory + "/" + billboard->GetName()));
				sceneObj->GetGameObjects().push_back(std::move(billboard));
			}

			// Sprite
			else if (type == "Sprite") {
				auto sprite = Sprite::Deserialize(objJson, engineCore, &(sceneObj->mainCamera_));
				sprite->Init();
				sprite->SetTextureHandle(engineCore->GetTextureManager()->LoadTexture(modelFileDirectory + "/" + sprite->GetName()));
				sceneObj->GetGameObjects().push_back(std::move(sprite));
			}
		}
	}
	return sceneObj;
}

void SceneObject::AddObjectFromJson(const nlohmann::json& j) {
#ifdef _DEBUG
	DebugLog("AddObjectFromJson");
#endif // _DEBUG
	if (!j.is_object() || !j.contains("type")) {
		return;
	}

	std::string type = j.value("type", "");

	if (type == "Model") {
		auto model = Model::Deserialize(j, engineCore_, &mainCamera_);
		// モデルファイル名が必要な場合はここでロード
		if (model) {
			model->LoadModel("Resources", model->GetModelFileName(), COORDINATESYSTEM_HAND_RIGHT);
			model->Init();
			gameObjects_.push_back(std::move(model));
		}
	} else if (type == "Billboard") {
		auto billboard = Billboard::Deserialize(j, engineCore_, &mainCamera_);
		if (billboard) {
			billboard->Init();
			// テクスチャハンドルをセット
			billboard->SetTextureHandle(engineCore_->GetTextureManager()->LoadTexture("Resources/" + billboard->GetName()));
			gameObjects_.push_back(std::move(billboard));
		}
	} else if (type == "Sprite") {
		auto sprite = Sprite::Deserialize(j, engineCore_, &mainCamera_);
		if (sprite) {
			sprite->Init();
			sprite->SetTextureHandle(engineCore_->GetTextureManager()->LoadTexture("Resources/" + sprite->GetName()));
			gameObjects_.push_back(std::move(sprite));
		}
	}
}

void SceneObject::AddModel(const std::string& directoryPath, const std::string& filename) {
	Model model(engineCore_, &mainCamera_);
	model.LoadModel(directoryPath, filename,COORDINATESYSTEM_HAND_RIGHT);
	model.Init();
	gameObjects_.push_back(std::make_unique<Model>(model));
}

void SceneObject::AddBillboard(const std::string& directoryPath, const std::string& filename) {
	// テクスチャハンドルを取得
	uint32_t textureHandle = engineCore_->GetTextureManager()->LoadTexture(directoryPath+"/"+ filename);
	std::unique_ptr<Billboard> billboadard = std::make_unique<Billboard>(engineCore_, &mainCamera_, 1.0f, 1.0f, textureHandle);
	billboadard->Init();
	billboadard->SetName(filename);
	gameObjects_.push_back(std::move(billboadard));
}

void SceneObject::AddSprite(const std::string& directoryPath, const std::string& filename) {
	// テクスチャハンドルを取得
	uint32_t textureHandle = engineCore_->GetTextureManager()->LoadTexture(directoryPath + "/" + filename);
	Vector2 size = engineCore_->GetTextureManager()->GetTextureSize(textureHandle);
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>(engineCore_, &mainCamera_, size.x,size.y);
	sprite->SetTextureHandle(textureHandle);
	sprite->Init();
	sprite->SetName(filename);
	gameObjects_.push_back(std::move(sprite));
}

void SceneObject::DeleteModel(BaseGameObject* model) {
	destroyedGameObjects_.push_back(model);
}

IScene* SceneObject::GetNextScene() {
	return nullptr;
}
