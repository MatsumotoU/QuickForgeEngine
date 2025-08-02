#include "SceneManager.h"
#include "Base/EngineCore.h"
#include <utility>
#include "Utility/FileLoader.h"
#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

static char saveFileName[256] = "";
static int selectedIndex = -1;

#include "Script/LuaScriptGenerator.h"
static char newLuaFileName[128] = "NewScript.lua";
static std::string lastCreatedLuaPath;
static bool showCreateResult = false;

static std::string lastScriptRunScenePath;

SceneManager::SceneManager() :sceneGameObjectGenerator_(currentScene_.get(), &assetManager_) {
	isRequestSwapScene_ = false;
	loadedScenePath_.clear();
}

void SceneManager::CreateScene(EngineCore* engineCore, const std::string& sceneName) {
	engineCore_ = engineCore;
	currentScene_ = std::make_unique<SceneObject>(engineCore, sceneName);
	sceneDataDirectorypath_ = "Resources/Scenes";
	sceneFilepath_.clear();
	sceneSelectionIndex_ = 0;
	sceneFilepath_ = FileLoader::GetFilesWithExtension(sceneDataDirectorypath_, ".json");
	if (sceneFilepath_.size() > 0) {
		selectedSceneFilepath_ = sceneFilepath_[0]; // 最初のシーンを選択
	}
	//#ifdef _DEBUG
	modelSelectionIndex_ = 0;
	inputFilepath_; // デフォルトのモデルデータパス
	ModelDirectoryPath_ = "Resources"; // デフォルトのモデルデータパス
	imageDirectoryPath_ = "Resources"; // デフォルトのイメージデータパス

	// モデルファイルのパスを取得
	modelFilepaths_ = FileLoader::GetFilesWithExtension(ModelDirectoryPath_, ".obj");
	if (modelFilepaths_.size() > 0) {
		inputFilepath_ = modelFilepaths_[0]; // 最初のモデルを選択
	}
	// ビルボードのパスを取得
	billboardSelectionIndex_ = 0;
	billboardFilepath_ = FileLoader::GetFilesWithExtension(imageDirectoryPath_, ".png");
	if (billboardFilepath_.size() > 0) {
		billboardInputFilepath_ = billboardFilepath_[0]; // 最初のビルボードを選択
	}
	// スプライトのパスを取得
	spriteSelectionIndex_ = 0;
	spriteFilepath_ = FileLoader::GetFilesWithExtension(imageDirectoryPath_, ".png");
	if (spriteFilepath_.size() > 0) {
		spriteInputFilepath_ = spriteFilepath_[0]; // 最初のスプライトを選択
	}
	// スクリプトのパスを取得
	scriptDirectoryPath_ = "Resources/Scripts";
	scriptSelectionIndex_ = 0;
	scriptFilepath_ = FileLoader::GetFilesWithExtension(scriptDirectoryPath_, ".lua");
	if (scriptFilepath_.size() > 0) {
		scriptInputFilepath_ = scriptFilepath_[0]; // 最初のスクリプトを選択
	}
	//#endif

	isRunningScript_ = false;
	startToRunScript_ = false;

#ifdef _DEBUG
	requestRedo_ = false;
	requestUndo_ = false;
#endif // _DEBUG
}

void SceneManager::InitializeScene() {
#ifndef _DEBUG
	isRequestSwapScene_ = true;
	loadedScenePath_ = "Resources/Scenes/LastScriptRunScene.json";
	//startToRunScript_ = true;
#endif // !_DEBUG

	/*isRequestSwapScene_ = true;
	loadedScenePath_ = "Resources/Scenes/LastScriptRunScene.json";
	startToRunScript_ = true;*/

	if (currentScene_) {
		currentScene_->Initialize();
	}
	collisionManager_.Initalize();
}

void SceneManager::UpdateScene() {
#ifdef _DEBUG
	Undo();
	Redo();
#endif // _DEBUG

#ifndef _DEBUG
	if (!isRunningScript_) {
		isRunningScript_ = true;
	}
#endif // !_DEBUG

	

	// シーンの実行
	if (currentScene_) {
		// 動的のアセットを読み込む
		for (const auto& assetF : engineCore_->GetLuaCallFiles()->GetAssetFiles()) {
			currentScene_->AddObjectFromJson(assetManager_.GetAsset(assetF).GetMetadata());
		}
		engineCore_->GetLuaCallFiles()->ClearAssetFiles();

		currentScene_->Update();

		// シュミレーション開始
		if (isRunningScript_) {
			// Luaスクリプトの更新
			engineCore_->GetLuaScriptManager()->UpdateScripts();
			// 衝突判定の更新
			collisionManager_.Update(currentScene_->GetColliders());
			// 衝突スクリプトの実行
			engineCore_->GetLuaScriptManager()->CollisionScripts();
		}
	}
}

void SceneManager::DrawScene() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}

void SceneManager::SaveScenesToJson(const std::string& filepath) {
#ifdef _DEBUG
	DebugLog("===Start save scenes!===");
#endif // _DEBUG

	nlohmann::json root;
	if (currentScene_) {
		root["scenes"].push_back(currentScene_->Serialize());
	}

	std::ofstream ofs(filepath);
	if (!ofs) {
		// ディレクトリがなければ作成
		std::filesystem::path path(filepath);
		std::filesystem::create_directories(path.parent_path());
		// 再度ファイルを開く
		ofs.open(filepath);
	}
	assert(ofs);
	ofs << root.dump(4); // インデント付きで出力
	ofs.close();

#ifdef _DEBUG
	DebugLog("===End save scenes===");
#endif // _DEBUG
}

void SceneManager::LoadScenesFromJson(const std::string& filepath) {
#ifdef _DEBUG
	DebugLog(std::format("LoadScene: {}", filepath));
#endif // _DEBUG
	loadedScenePath_ = filepath;
	isRequestSwapScene_ = true;
	engineCore_->GetDirectXCommon()->WaitForGpu();
}

void SceneManager::LoadScenesLua(const std::string& filename) {
	LoadScenesFromJson(sceneDataDirectorypath_ + "/" + filename + ".json");
}

void SceneManager::SwapScene() {
	if (isRequestSwapScene_) {
		isRequestSwapScene_ = false;

		engineCore_->GetLuaScriptManager()->ClearAllGameObjScripts();

		std::ifstream ifs;
		assert(!loadedScenePath_.empty() && "Loaded scene path is empty");
		ifs.open(loadedScenePath_);
		assert(ifs);

		nlohmann::json root;
		ifs >> root;
		if (root.contains("scenes")) {
			loadedScene_ = SceneObject::Deserialize(root["scenes"][0], engineCore_, ModelDirectoryPath_);
		}
		assert(loadedScene_ && "Loaded scene is null");

		currentScene_ = std::move(loadedScene_);
		currentScene_->Initialize();

		// ここでcurrentScene_のGameObjectとScriptの状態をデバッグ出力
#ifdef _DEBUG
		for (const auto& obj : currentScene_->GetGameObjects()) {
			DebugLog(("GameObject: " + obj->GetName() + ", Script: " + obj->GetAttachedScriptName()).c_str());
		}
#endif
		engineCore_->GetChiptune()->PlayNoise();
	}
}

std::string SceneManager::GetCurrentSceneName() const {
	return currentScene_ ? currentScene_->GetSceneName() : "UnnamedScene";
}

#ifdef _DEBUG
void SceneManager::DrawImGui() {
	assetManager_.DrawImGui();

	// スクリプトの実行状態を表示
	if (ImGui::Button("RunningScript")) {
		if (!isRunningScript_) {
			// スクリプト開始時：シーンを保存
			std::string fileName = "LastScriptRunScene.json";
			lastScriptRunScenePath = sceneDataDirectorypath_ + "/" + fileName;
			if (currentScene_) {
				currentScene_->SetSceneName(fileName);
			}
			SaveScenesToJson(lastScriptRunScenePath);
			engineCore_->GetLuaScriptManager()->InitScripts();
			isRunningScript_ = true;
		} else {
			// スクリプト停止時：直前のシーンを再読込
			if (!lastScriptRunScenePath.empty()) {
				LoadScenesFromJson(lastScriptRunScenePath);
				isRequestSwapScene_ = true;
			}
			isRunningScript_ = false;
		}
	}
	if (isRunningScript_) {
		ImGui::SameLine();
		ImGui::Text("Running Script");
	}

	if (ImGui::BeginTabBar("SceneTab")) {
		// シーンの保存と読み込みタブ
		if (ImGui::BeginTabItem("Scenes")) {
			if (ImGui::Button("SaveScene")) {
				// 入力が空の場合はデフォルト名を使う
				std::string fileName = strlen(saveFileName) > 0 ? saveFileName : GetCurrentSceneName();
				std::string filepath = sceneDataDirectorypath_ + "/" + fileName + ".json";
				if (currentScene_) {
					currentScene_->SetSceneName(fileName);
				}
				SaveScenesToJson(filepath);
			}
			ImGui::SameLine();
			ImGui::InputText("Save As", saveFileName, IM_ARRAYSIZE(saveFileName));

			if (isRequestSwapScene_) {
				ImGui::Text("Scene has been changed. Please reload.");
			} else {
				if (ImGui::Button("LoadScene")) {
					std::string filepath = sceneDataDirectorypath_ + "/" + selectedSceneFilepath_;
					LoadScenesFromJson(filepath);
				}
			}
			ImGui::SameLine();
			// ImGui::Combo用にconst char*配列を作成
			std::vector<const char*> items;
			for (const auto& f : sceneFilepath_) {
				items.push_back(f.c_str());
			}
			if (ImGui::Combo("Model", &sceneSelectionIndex_, items.data(), static_cast<int>(items.size()))) {
				// 選択が変わったときの処理
				sceneSelectionIndex_ = std::clamp(sceneSelectionIndex_, 0, static_cast<int>(sceneFilepath_.size()) - 1);
				selectedSceneFilepath_ = sceneFilepath_[sceneSelectionIndex_];
			}

			if (ImGui::Button("ReloadScene File")) {
				// モデルファイルのパスを再取得
				sceneFilepath_.clear();
				sceneSelectionIndex_ = 0;
				sceneFilepath_ = FileLoader::GetFilesWithExtension(sceneDataDirectorypath_, ".json");
				if (sceneFilepath_.size() > 0) {
					selectedSceneFilepath_ = sceneFilepath_[0]; // 最初のシーンを選択
				}
			}

			ImGui::EndTabItem();
		}

		// シーンオブジェクトのタブ
		if (ImGui::BeginTabItem("SceneObjects")) {
			if (currentScene_) {
				auto& gameObjects = currentScene_->GetGameObjects();
				BaseGameObject* objectToDelete = nullptr;
				int deleteIndex = -1;

				// 縦スクロール可能な領域を作成（幅300, 高さ200など適宜調整）
				ImGui::Text("Game Objects in Scene: %s", currentScene_->GetSceneName().c_str());
				ImGui::BeginChild("ObjectListChild", ImVec2(300, 200), true);

				for (size_t i = 0; i < gameObjects.size(); ++i) {
					auto& gameObject = gameObjects[i];
					std::string label = "[" + std::to_string(i) + "] " + gameObject->GetName() + "##" + std::to_string(i);
					bool isSelected = (selectedIndex == static_cast<int>(i));
					if (ImGui::Selectable(label.c_str(), isSelected, 0, ImVec2(200, 0))) {
						selectedIndex = static_cast<int>(i);
					}
					ImGui::SameLine();
					std::string delLabel = "x##" + std::to_string(i);
					if (ImGui::Button(delLabel.c_str())) {
						PushUndo();
						objectToDelete = gameObject.get();
						deleteIndex = static_cast<int>(i);
					}
				}
				ImGui::EndChild();

				// 何もない所のダブルクリックで選択解除
				ImVec2 childMin = ImGui::GetItemRectMin();
				ImVec2 childMax = ImGui::GetItemRectMax();
				ImVec2 mousePos = ImGui::GetMousePos();
				if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
					ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					// リスト領域内で、どのアイテムも選択されていない場合
					if (mousePos.x >= childMin.x && mousePos.x <= childMax.x &&
						mousePos.y >= childMin.y && mousePos.y <= childMax.y) {
						selectedIndex = -1;
					}
				}

				// ループ後に削除
				if (objectToDelete) {
					currentScene_->DeleteModel(objectToDelete);
					if (selectedIndex == deleteIndex) {
						selectedIndex = -1;
					} else if (selectedIndex > deleteIndex) {
						selectedIndex--;
					}
				}

				// 詳細ウィンドウ
				if (selectedIndex >= 0 && selectedIndex < static_cast<int>(gameObjects.size())) {
					ImGui::Begin("Object Detail", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
					gameObjects[selectedIndex]->DrawImGui();
					ImGui::Separator();
					// アセット化
					if (ImGui::Button("Add to Asset")) {
						assetManager_.AddAsset(*gameObjects[selectedIndex]);
					}
					
					// スクリプトファイル選択UI
					ImGui::Text("Script Directory: %s", scriptDirectoryPath_.c_str());
					if (ImGui::Button("SearchFile")) {
						scriptFilepath_.clear();
						scriptSelectionIndex_ = 0;
						scriptFilepath_ = FileLoader::GetFilesWithExtension(scriptDirectoryPath_, ".lua");
						if (!scriptFilepath_.empty()) {
							scriptInputFilepath_ = scriptFilepath_[0];
						} else {
							scriptInputFilepath_.clear();
						}
					}
					// ファイルを開く
					ImGui::SameLine();
					if (ImGui::Button("OpenSelectedFile")) {
						if (!scriptFilepath_.empty() && scriptSelectionIndex_ >= 0 && scriptSelectionIndex_ < static_cast<int>(scriptFilepath_.size())) {
							std::string luaPath = scriptDirectoryPath_ + "/" + scriptFilepath_[scriptSelectionIndex_];
#ifdef _WIN32
#include <windows.h>
							ShellExecuteA(nullptr, "open", "code", luaPath.c_str(), nullptr, SW_HIDE);
#else
							std::string command = "code \"" + luaPath + "\"";
							system(command.c_str());
#endif
						}
					}
					// スクリプト割り当てボタン
					ImGui::SameLine();
					if (ImGui::Button("AddScript##AssignScript")) {
						if (scriptFilepath_.size() > 0) {
							BaseGameObject* targetObject = gameObjects[selectedIndex].get();
							std::string key = targetObject->GetName();
							std::string scriptPath = scriptDirectoryPath_ + "/" + scriptInputFilepath_; // 選択中のスクリプトファイル
							engineCore_->GetLuaScriptManager()->AddGameObjScript(key, targetObject, scriptPath);
							targetObject->SetScriptName(scriptInputFilepath_);
#ifdef _DEBUG
							DebugLog((key + "assign to " + scriptPath).c_str());
#endif
						}
					}
					ImGui::Spacing();
					if (scriptFilepath_.size() > 0) {
						// Comboでスクリプトファイル選択
						std::vector<const char*> scriptItems;
						for (const auto& f : scriptFilepath_) {
							scriptItems.push_back(f.c_str());
						}
						if (ImGui::Combo("Script", &scriptSelectionIndex_, scriptItems.data(), static_cast<int>(scriptItems.size()))) {
							// 選択が変わったときの処理
							scriptInputFilepath_ = scriptFilepath_[scriptSelectionIndex_];
						}
					} else {
						ImGui::Text("No script files found in %s", scriptDirectoryPath_.c_str());
					}

					ImGui::Separator();
					ImGui::Text("NewLuaScript");
					ImGui::InputText("FileName", newLuaFileName, IM_ARRAYSIZE(newLuaFileName));
					if (ImGui::Button("Create##CreateLuaScript")) {
						std::string baseName = newLuaFileName;
						std::string fileName = LuaScriptGenerator::GenerateUniqueFileName(scriptDirectoryPath_, baseName);
						std::string content = LuaScriptGenerator::GetDefaultScriptContent();
						std::string fullPath;
						bool success = LuaScriptGenerator::CreateLuaScriptFile(scriptDirectoryPath_, fileName, content, fullPath);
						if (success) {
							// VSCodeで開く
							std::string command = "code \"" + fullPath + "\"";
							system(command.c_str());
							lastCreatedLuaPath = fullPath;
							showCreateResult = true;
						} else {
							lastCreatedLuaPath = "f";
							showCreateResult = true;
						}
					}
					if (showCreateResult) {
						ImGui::Text("result: %s", lastCreatedLuaPath.c_str());
					}
					ImGui::End();
				}
			} else {
				ImGui::Text("No scene loaded.");
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("SceneEdit")) {
			if (ImGui::Button("Reload File")) {
				// モデルファイルのパスを再取得
				modelFilepaths_.clear(); // 既存のパスをクリア
				modelFilepaths_ = FileLoader::GetFilesWithExtension(ModelDirectoryPath_, ".obj");
				if (modelFilepaths_.size() > 0) {
					inputFilepath_ = modelFilepaths_[0];
					modelSelectionIndex_ = 0; // 最初のモデルを選択
				} else {
					inputFilepath_.clear(); // ファイルがない場合はクリア
				}
			}

			// モデル追加処理
			ImGui::Text("Model Directory: %s", ModelDirectoryPath_.c_str());
			ImGui::Spacing();
			if (modelFilepaths_.size() > 0) {
				if (ImGui::Button("Add##AddModel")) {
					if (currentScene_) {
						PushUndo();
						currentScene_->AddModel(ModelDirectoryPath_, inputFilepath_);
					}
				}
				ImGui::SameLine();
				// ImGui::Combo用にconst char*配列を作成
				std::vector<const char*> items;
				for (const auto& f : modelFilepaths_) {
					items.push_back(f.c_str());
				}
				if (ImGui::Combo("Model", &modelSelectionIndex_, items.data(), static_cast<int>(items.size()))) {
					// 選択が変わったときの処理
					inputFilepath_ = modelFilepaths_[modelSelectionIndex_];
				}
			} else {
				ImGui::Text("No model files found in %s", ModelDirectoryPath_.c_str());
			}

			// ビルボード追加処理
			ImGui::Text("Billboard Directory: %s", imageDirectoryPath_.c_str());
			ImGui::Spacing();
			if (billboardFilepath_.size() > 0) {
				if (ImGui::Button("Add##AddBillboard")) {
					if (currentScene_) {
						PushUndo();
						currentScene_->AddBillboard(imageDirectoryPath_, billboardInputFilepath_);
					}
				}
				ImGui::SameLine();
				// ImGui::Combo用にconst char*配列を作成
				std::vector<const char*> items;
				for (const auto& f : billboardFilepath_) {
					items.push_back(f.c_str());
				}
				if (ImGui::Combo("Billboard", &billboardSelectionIndex_, items.data(), static_cast<int>(items.size()))) {
					// 選択が変わったときの処理
					billboardInputFilepath_ = billboardFilepath_[billboardSelectionIndex_];
				}
			} else {
				ImGui::Text("No image files found in %s", imageDirectoryPath_.c_str());
			}

			// Sprite追加処理
			ImGui::Text("Sprite Directory: %s", imageDirectoryPath_.c_str());
			ImGui::Spacing();
			if (spriteFilepath_.size() > 0) {
				if (ImGui::Button("Add##AddSprite")) {
					if (currentScene_) {
						PushUndo();
						currentScene_->AddSprite(imageDirectoryPath_, spriteInputFilepath_);
					}
				}
				ImGui::SameLine();
				// ImGui::Combo用にconst char*配列を作成
				std::vector<const char*> items;
				for (const auto& f : spriteFilepath_) {
					items.push_back(f.c_str());
				}
				if (ImGui::Combo("Sprite", &spriteSelectionIndex_, items.data(), static_cast<int>(items.size()))) {
					// 選択が変わったときの処理
					spriteInputFilepath_ = spriteFilepath_[spriteSelectionIndex_];
				}
			} else {
				ImGui::Text("No sprite files found in %s", imageDirectoryPath_.c_str());
			}

			// アセットからのオブジェクト追加
			ImGui::Text("Asset Directory: %s", assetManager_.GetDirectoryPath().c_str());
			ImGui::Spacing();
			if (ImGui::Button("Add##AddAsset")) {
				if (currentScene_) {
					PushUndo();
					currentScene_->AddObjectFromJson(assetManager_.GetAsset(assetFileName_).GetMetadata());
				}
			}
			ImGui::SameLine();
			// アセットの選択
			std::vector<const char*> assetItems;
			for (const auto& f : assetManager_.GetAssetFiles()) {
				assetItems.push_back(f.c_str());
			}
			if (ImGui::Combo("Assets", &assetManager_.selectedAssetIndex_, assetItems.data(), static_cast<int>(assetItems.size()))) {
				// 選択が変わったときの処理
				assetFileName_ = assetManager_.GetAssetFiles()[assetManager_.selectedAssetIndex_];
			}

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void SceneManager::DrawGizmo(const ImGuizmo::OPERATION& op, const ImGuizmo::MODE& mode, const ImVec2& imageScreenPos, const ImVec2& imageSize) {
	if (currentScene_) {
		if (currentScene_->GetGameObjects().size() > 0) {
			if (MyGameMath::InRange(selectedIndex, 0, static_cast<int>(currentScene_->GetGameObjects().size()))) {
				currentScene_->GetGameObjects()[selectedIndex]->DrawGizmo(op, mode, imageScreenPos, imageSize);
			}
		}
	}
}
void SceneManager::PickObjectFromScreen(float relX, float relY) {
	if (!currentScene_) return;
	// 画像内のピクセル座標
	float px = relX * engineCore_->GetWinApp()->kWindowWidth;
	float py = relY * engineCore_->GetWinApp()->kWindowHeight;

	auto& gameObjects = currentScene_->GetGameObjects();
	int nearestIndex = -1;
	float minDist = 1e10f;

	for (size_t i = 0; i < gameObjects.size(); ++i) {
		auto& obj = gameObjects[i];
		Vector3 worldPos = obj->GetWorldPosition();
		Vector3 screenPos = currentScene_->GetMainCamera().GetWorldToScreenPos(worldPos);
		// screenPos.x, screenPos.y: ピクセル座標（Image内の座標系に合わせる）
		if ((screenPos.XY() - Vector2(px, py)).Length() <= minDist) {
			minDist = (screenPos.XY() - Vector2(px, py)).Length();
			nearestIndex = static_cast<int>(i);
		}
	}

	if (nearestIndex >= 0) {
		extern int selectedIndex;
		selectedIndex = nearestIndex;
	}
}
void SceneManager::PushUndo() {
	if (currentScene_) {
		undoStack_.push(currentScene_->Serialize());
		// 新しい操作が入ったらRedo履歴は消す
		while (!redoStack_.empty()) {
			redoStack_.pop();
		}
	}
}
void SceneManager::Undo() {
	if (!requestUndo_) {
		return;
	}

	if (!undoStack_.empty() && currentScene_) {
		// 今の状態をRedoスタックに保存
		redoStack_.push(currentScene_->Serialize());

		nlohmann::json prev = undoStack_.top();
		undoStack_.pop();
		std::string sceneName = currentScene_->GetSceneName();
		currentScene_ = SceneObject::Deserialize(prev, engineCore_, ModelDirectoryPath_);
		currentScene_->SetSceneName(sceneName);
		currentScene_->Initialize();
	}

	requestUndo_ = false;
}
void SceneManager::Redo() {
	if (!requestRedo_) {
		return;
	}

	if (!redoStack_.empty() && currentScene_) {
		undoStack_.push(currentScene_->Serialize());

		nlohmann::json next = redoStack_.top();
		redoStack_.pop();
		std::string sceneName = currentScene_->GetSceneName();
		currentScene_ = SceneObject::Deserialize(next, engineCore_, ModelDirectoryPath_);
		currentScene_->SetSceneName(sceneName);
		currentScene_->Initialize();
	}

	requestRedo_ = false;
}
#endif // _DEBUG