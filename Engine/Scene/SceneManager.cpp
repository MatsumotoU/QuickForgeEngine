#include "SceneManager.h"
#ifdef _DEBUG
#include "Base/DirectX/ImGuiManager.h"
#include "Utility/FileLoader.h"
#endif // _DEBUG


void SceneManager::CreateScene(EngineCore* engineCore, const std::string& sceneName) {
	engineCore_ = engineCore;
	currentScene_ = std::make_unique<SceneObject>(engineCore, sceneName);
#ifdef _DEBUG
	modelSelectionIndex_ = 0;
	inputFilepath_; // デフォルトのモデルデータパス
	ModelDirectoryPath_ = "Resources"; // デフォルトのモデルデータパス

	// モデルファイルのパスを取得
	modelFilepaths_ = FileLoader::GetFilesWithExtension(ModelDirectoryPath_, ".obj");
	if (modelFilepaths_.size() > 0) {
		inputFilepath_ = modelFilepaths_[0]; // 最初のモデルを選択
	}
#endif
}

void SceneManager::InitializeScene() {
	if (currentScene_) {
		currentScene_->Initialize();
	}
}

void SceneManager::UpdateScene() {
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::DrawScene() {
	if (currentScene_) {
		currentScene_->Draw();
	}
}
std::string SceneManager::GetCurrentSceneName() const {
	return currentScene_ ? currentScene_->GetSceneName() : "UnnamedScene";
}

#ifdef _DEBUG
void SceneManager::DrawImGui() {
	if (ImGui::BeginTabBar("SceneTab")) {
		if (ImGui::BeginTabItem("SceneObjects")) {
			if (currentScene_) {
				for (const auto& gameObject : currentScene_->GetGameObjects()) {
					if (ImGui::TreeNode(gameObject->GetName().c_str())) {
						gameObject->DrawImGui(); // 各ゲームオブジェクトのImGui描画
						ImGui::TreePop();
					}
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
				if (ImGui::Button("Add")) {
					if (currentScene_) {
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
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}
#endif // _DEBUG