#include "EditorWindowManager.h"
#include "AllEditorWindow.h"

#include "framework/window/WindowDialogFramework.h"
#include "window/interface/IGameWindowManager.h"

#include "scene/SceneManager.h"
#include "ecs/entity/EntityManager.h"
#include "components/TransformComponent.h"
#include "components/ObjectInfoComponent.h"
#include "command/EditorCommandList.h"
#include "string/MyString.h"
#include "command/AllCommands.h"
#include "process/ProcessUtil.h"
#include "assetfactory/game-data/GameProjectPaths.h"
#include <filesystem>
#include <fstream>
#include <string_view>

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <utility>
#include <cstdio>

namespace {
class LogWindow final : public QFE::EDITOR::IEditorWindow {
public:
    void Initialize() override { isActive_ = true; }
    void Draw(std::set<uint32_t>&, QFE::EDITOR::EditorCommandList&) override {
        const bool visible = ImGui::Begin("Log", &isActive_);
        isFocus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        if (visible) {
            auto* logger = QFE::MyDebugLog::GetInstance();
            if (ImGui::Button("Clear")) logger->ClearEntries();
            ImGui::SameLine(); ImGui::Checkbox("Auto-scroll", &autoScroll_);
            ImGui::SameLine(); ImGui::Checkbox("Engine", &showEngine_);
            ImGui::SameLine(); ImGui::Checkbox("Editor", &showEditor_);
            ImGui::SameLine(); ImGui::Checkbox("Warning", &showWarning_);
            ImGui::SameLine(); ImGui::Checkbox("Error", &showError_);
            filter_.Draw("Filter", 220.0f);
            ImGui::Separator();
            if (ImGui::BeginChild("LogEntries", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
                const bool atBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();
                const auto entries = logger->GetEntries();
                for (const auto& entry : entries) {
                    const char* label = "Engine";
                    ImVec4 color(0.8f, 0.8f, 0.8f, 1.0f);
                    bool enabled = showEngine_;
                    if (entry.level == QFE::LogLevel::EditorInfo) { label = "Editor"; enabled = showEditor_; }
                    else if (entry.level == QFE::LogLevel::Warning) { label = "Warning"; enabled = showWarning_; color = ImVec4(1.0f, 0.8f, 0.25f, 1.0f); }
                    else if (entry.level == QFE::LogLevel::Error) { label = "Error"; enabled = showError_; color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); }
                    if (!enabled || !filter_.PassFilter(entry.message.c_str())) continue;
                    const std::string line = "[" + entry.timestamp + "] [" + label + "] " + entry.message;
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextUnformatted(line.c_str());
                    ImGui::PopStyleColor();
                }
                if (autoScroll_ && atBottom && !entries.empty() && entries.back().sequence != lastSequence_) ImGui::SetScrollHereY(1.0f);
                if (!entries.empty()) lastSequence_ = entries.back().sequence;
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }
    std::string GetWindowName() override { return "Log"; }
    bool GetIsActive() override { return isActive_; }
    bool SetIsActive(bool active) override { return isActive_ = active; }
    bool GetIsFocus() override { return isFocus_; }
private:
    ImGuiTextFilter filter_;
    uint64_t lastSequence_ = 0;
    bool isActive_ = true;
    bool isFocus_ = false;
    bool autoScroll_ = true;
    bool showEngine_ = false;
    bool showEditor_ = true;
    bool showWarning_ = true;
    bool showError_ = true;
};

bool IsProjectFile(const std::wstring& path, const std::filesystem::path& directory) {
    return QFE::PROJECT::IsWithin(std::filesystem::path(path), QFE::PROJECT::Assets() / directory);
}
bool IsProjectScene(const std::wstring& path) {
    return IsProjectFile(path, "scenes");
}
}

void QFE::EDITOR::EditorWindowManager::Initialize(
	QFE::SCENE::SceneManager* sceneManager,
	ImTextureID sceneTextureId,
	QFE::WINDOW::IGameWindowManager& windowManager,
	QFE::ASSET::AssetManager& assetManager,
	QFE::ASSET::ModelLoader& modelLoader,
	ComponentInspectorRegistry* componentInspectorRegistry,
	const QFE::FRAMEWORK::EngineConfig& config) {
	engineConfig_ = config;
	windowManager_ = &windowManager;
	sceneManager_ = sceneManager;
	projectName_ = QFE::ConvertString(QFE::PROJECT::Root().filename().wstring());
	try {
		std::ifstream projectFile(QFE::PROJECT::Root() / "project.json");
		const auto project = nlohmann::json::parse(projectFile);
		const auto name = project.find("name");
		if (name != project.end() && name->is_string() && !name->get<std::string>().empty()) {
			projectName_ = name->get<std::string>();
		}
	} catch (const nlohmann::json::exception&) { }
    EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();
	editorWindowsMap_.clear();
	editorWindowsMap_[EditorWindowType::Hierarchy] = std::make_unique<Hierarchy>(sceneManager, assetManager);
	editorWindowsMap_[EditorWindowType::Inspector] =
		std::make_unique<Inspector>(&entityManager, &modelLoader, componentInspectorRegistry);
	editorWindowsMap_[EditorWindowType::AnimationEditor] = std::make_unique<AnimationEditor>(&entityManager);
	editorWindowsMap_[EditorWindowType::AssetBrowser] = std::make_unique<AssetBrowser>(assetManager, windowManager, *sceneManager);
	editorWindowsMap_[EditorWindowType::Logger] = std::make_unique<LogWindow>();
	editorWindowsMap_[EditorWindowType::GameViewer] = std::make_unique<GameViewer>(sceneTextureId, sceneManager, config.windowWidth, config.windowHeight);
    editorWindowsMap_[EditorWindowType::SceneViewer] = std::make_unique<SceneViewer>(sceneTextureId, sceneManager, config.windowWidth, config.windowHeight);
}

void QFE::EDITOR::EditorWindowManager::Update() {
	selectedEntities_.clear();
}

void QFE::EDITOR::EditorWindowManager::Draw(EditorCommandList& commandList) {
	bool saveSceneRequested = false;
	bool saveSceneAsRequested = false;

    // 1. 画面全体のフラグを設定（タイトルバーやリサイズ、移動などをすべて無効化）
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    // 2. メインビューポート（画面全体）のサイズと位置を取得して、ウィンドウをそこに合わせる
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // 3. ウィンドウのパディング（内側の余白）を一時的にゼロにする（ドックスペースを画面端まで広げるため）
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // 4. 透明な背景ウィンドウの開始
    ImGui::Begin("MainDockSpaceWindow", nullptr, window_flags);

    // スタイル変数を元に戻す
    ImGui::PopStyleVar(3);

    // 5. ドックスペース（ドッキングの土台）を設置
    ImGuiIO& io = ImGui::GetIO();
	if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
		saveSceneRequested = true;
	}
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyMainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

	// エディタのメインメニュー
    if (ImGui::BeginMenuBar()) {
		// ファイルメニュー
        if (ImGui::BeginMenu("File")) {
            // 新規シーン
            if (ImGui::MenuItem("New Scene", nullptr)) {
                sceneManager_->Initialize();
            }

            // ロード
            if (ImGui::MenuItem("Load Scene", nullptr)) {
                // JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
                    *windowManager_,
                    L"Scene Files (*.scene;*.json)", L"*.scene;*.json",
                    selectedFilePath, (QFE::PROJECT::Assets() / "scenes").wstring()) && IsProjectScene(selectedFilePath)) {
                    // Entityの生成
					commandList.AddCommand(std::make_unique<LoadSceneCommand>(QFE::ConvertString(selectedFilePath), sceneManager_));
                }
            }
            if (ImGui::MenuItem("Set Start Scene...")) {
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestGetFilePathFromUser(*windowManager_, L"Scene Files (*.scene;*.json)", L"*.scene;*.json",
                    selectedFilePath, (QFE::PROJECT::Assets() / "scenes").wstring()) && IsProjectScene(selectedFilePath)) {
                    const auto projectFile = QFE::PROJECT::Root() / "project.json";
                    try {
                        std::ifstream input(projectFile);
                        auto json = nlohmann::json::parse(input);
                        input.close();
                        json["start_scene"] = QFE::ConvertString(std::filesystem::relative(
                            std::filesystem::path(selectedFilePath), QFE::PROJECT::Assets() / "scenes").generic_wstring());
                        std::ofstream output(projectFile, std::ios::trunc);
                        output << json.dump(4) << '\n';
                        if (!output) QFE_LOG("Failed to save project.json.");
                    } catch (const std::exception&) { QFE_LOG("Failed to update project.json."); }
                }
            }
			// セーブ
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
				saveSceneRequested = true;
            }
			if (ImGui::MenuItem("Save Scene As...", nullptr)) {
				saveSceneAsRequested = true;
            }
			// エンティティのロード
            if (ImGui::MenuItem("Add Saved Entities...", nullptr)) {
                // 保存済みエンティティを現在のシーンへ追加するファイルを選択させる
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
                    *windowManager_,
                    L"Entity Files (*.entity;*.json)", L"*.entity;*.json",
                    selectedFilePath, (QFE::PROJECT::Assets() / "entities").wstring())) {
                    if (!IsProjectFile(selectedFilePath, ".")) {
                        QFE_LOG("Load entities from the current project's assets directory.");
                    } else {
                        const uint32_t firstEntityId =
                            sceneManager_->LoadEntityOnCurrentSceneFromJsonObject(QFE::ConvertString(selectedFilePath));
                        if (firstEntityId == UINT32_MAX) {
                            QFE_LOG("Failed to add saved entities to the current scene.");
                        }
                    }
                }
            }
			ImGui::Separator();
			if (ImGui::MenuItem("Build and Reload DLL", nullptr)) {
				gameModuleBuildAndReloadRequested_ = true;
			}
            ImGui::EndMenu();
        }
		// ウィンドウメニュー
        if (ImGui::BeginMenu("Window")) {
			ImGui::MenuItem("Engine Config", nullptr, &showEngineConfig_);
            for(auto& [type, window] : editorWindowsMap_) {
                bool isActive = window->GetIsActive();
                if (ImGui::MenuItem(window->GetWindowName().c_str(), nullptr, isActive)) {
                    window->SetIsActive(!isActive);
                }
			}
            ImGui::EndMenu();
        }
		// Debugメニュー
        if (ImGui::BeginMenu("Debug")) {
            // シーンを選ばせて開始する
            if (ImGui::MenuItem("Launch Scene from File...")) {

                // クリックされたらファイル選択ダイアログを開く
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
                    *windowManager_,
                    L"Scene Files (*.scene;*.json)", L"*.scene;*.json",
                    selectedFilePath, (QFE::PROJECT::Assets() / "scenes").wstring()) && IsProjectScene(selectedFilePath)) {

					if (!QFE::ProcessUtil::LaunchExe(L"Runtime.exe", L"--project \"" + QFE::PROJECT::Root().wstring() + L"\" --scene \"" + selectedFilePath + L"\"")) {
						QFE_LOG("Failed to launch the runtime.", QFE::LogLevel::Error);
					}
                }
            }

            ImGui::EndMenu();
		}
		const auto& scenePath = sceneManager_->GetCurrentScenePath();
		const std::string sceneName = scenePath.empty() ? "Untitled" :
			QFE::ConvertString(std::filesystem::path(QFE::ConvertString(scenePath)).filename().wstring());
		ImGui::Separator();
		ImGui::Text("Project: %s | Scene: %s", projectName_.c_str(), sceneName.c_str());
		ImGui::EndMenuBar();
    }

    ImGui::End();

	if (showEngineConfig_) {
		ImGui::Begin("Engine Config", &showEngineConfig_);
		ImGui::TextUnformatted("Saved settings apply after restarting the application.");
		ImGui::InputScalar("Directional Light Max Num", ImGuiDataType_U32, &engineConfig_.directionalLightMaxNum);
		ImGui::InputScalar("Particle Max Num", ImGuiDataType_U32, &engineConfig_.particleMaxNum);
		ImGui::InputScalar("Window Width", ImGuiDataType_U32, &engineConfig_.windowWidth);
		ImGui::InputScalar("Window Height", ImGuiDataType_U32, &engineConfig_.windowHeight);
		auto editString = [](const char* label, std::string& value) {
			char buffer[1024]{};
			std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());
			if (ImGui::InputText(label, buffer, sizeof(buffer))) value = buffer;
		};
		editString("Editor Window Name", engineConfig_.editorWindowName);
		editString("Runtime Window Name", engineConfig_.runtimeWindowName);
		ImGui::TextUnformatted("Game assets are loaded from the selected project.");
		editString("Pixel Shader Directory", engineConfig_.psDirName);
		editString("Vertex Shader Directory", engineConfig_.vsDirName);
		editString("Compute Shader Directory", engineConfig_.csDirName);
		editString("Ray Tracing Shader Directory", engineConfig_.rtDirName);
		if (ImGui::Button("Save")) {
			engineConfig_.directionalLightMaxNum = (std::clamp)(engineConfig_.directionalLightMaxNum, 1u, 4096u);
			engineConfig_.particleMaxNum = (std::clamp)(engineConfig_.particleMaxNum, 1u, 1048576u);
			engineConfig_.windowWidth = (std::clamp)(engineConfig_.windowWidth, 1u, 8192u);
			engineConfig_.windowHeight = (std::clamp)(engineConfig_.windowHeight, 1u, 8192u);
			auto normalizeDir = [](std::string& dir) {
				if (!dir.empty() && dir.back() != '/' && dir.back() != '\\') dir += '/';
			};
			normalizeDir(engineConfig_.assetDir); normalizeDir(engineConfig_.modelDir);
			normalizeDir(engineConfig_.psDirName); normalizeDir(engineConfig_.vsDirName);
			normalizeDir(engineConfig_.csDirName); normalizeDir(engineConfig_.rtDirName);
			if (engineConfig_.assetDir.empty() || engineConfig_.modelDir.empty() || engineConfig_.scriptDir.empty() ||
				engineConfig_.psDirName.empty() || engineConfig_.vsDirName.empty() ||
				engineConfig_.csDirName.empty() || engineConfig_.rtDirName.empty() ||
				engineConfig_.editorWindowName.empty() || engineConfig_.runtimeWindowName.empty()) {
				engineConfigStatus_ = "Names and directories cannot be empty.";
			} else {
				engineConfigStatus_ = engineConfig_.Save() ? "Saved. Restart to apply." : "Failed to save EngineConfig.json.";
			}
		}
		ImGui::TextUnformatted(engineConfigStatus_.c_str());
		ImGui::End();
	}

	if (saveSceneAsRequested) {
		QueueSceneSave(commandList, true);
	} else if (saveSceneRequested) {
		QueueSceneSave(commandList, false);
	}

	// 6. 各エディタウィンドウの描画
    auto* hierarchy = static_cast<Hierarchy*>(editorWindowsMap_[EditorWindowType::Hierarchy].get());
    if (hierarchy->GetIsActive()) {
        selectedEntities_.clear();
        hierarchy->Draw(selectedEntities_, commandList);
    }
    selectedEntities_ = hierarchy->GetSelectedEntities();
	if (hierarchy->ConsumeSaveSelectedEntitiesRequest()) {
		SaveSelectedEntities();
	}
	if (!editorWindowsMap_[EditorWindowType::AssetBrowser]->GetIsActive()) {
		windowManager_->ConsumeDroppedFiles();
	}
	for (auto& [type, window] : editorWindowsMap_) {
        if (type == EditorWindowType::Hierarchy || !window->GetIsActive()) {
            continue;
		}
		window->Draw(selectedEntities_, commandList);
	}
}

void QFE::EDITOR::EditorWindowManager::SaveSelectedEntities() {
	if (selectedEntities_.empty()) {
		QFE_LOG("No entities selected for saving.");
		return;
	}

	// 右クリックしたEntityを含む選択範囲を一つのJSONファイルに保存する。
	nlohmann::json savedEntities;
	savedEntities["entities"] = nlohmann::json::array();
	for (uint32_t entityId : selectedEntities_) {
		nlohmann::json entityComponents =
			sceneManager_->GetCurrentSceneEntityManager().SerializeEntityComponents(entityId);
		if (!entityComponents.empty()) {
			savedEntities["entities"].push_back(std::move(entityComponents));
		}
	}
	if (savedEntities["entities"].empty()) {
		QFE_LOG("No serializable entities selected for saving.");
		return;
	}

	// 単体はEntity名、複数選択は共通名を使い、ファイル名に使えない文字を置き換える。
	std::wstring baseName = L"selected-entities";
	if (selectedEntities_.size() == 1) {
		const uint32_t entityId = *selectedEntities_.begin();
		const auto& manager = sceneManager_->GetCurrentSceneEntityManager();
		baseName = L"entity";
		if (manager.HasComponent<QFE::SCENE::ObjectInfoComponent>(entityId)) {
			auto name = QFE::ConvertString(manager.GetComponent<QFE::SCENE::ObjectInfoComponent>(entityId).name);
			if (name.size() > 80) {
				name.resize(80);
				if (name.back() >= 0xD800 && name.back() <= 0xDBFF) name.pop_back();
			}
			for (wchar_t& c : name) {
				if (c < 32 || std::wstring_view(L"<>:\"/\\|?*").find(c) != std::wstring_view::npos) c = L'_';
			}
			while (!name.empty() && (name.back() == L'.' || name.back() == L' ')) name.pop_back();
			if (!name.empty()) baseName += L"-" + name;
		}
	}

	const auto directory = QFE::PROJECT::Assets() / "entities";
	std::error_code error;
	std::filesystem::create_directories(directory, error);
	if (error) {
		QFE_LOG("Failed to create the entities directory.", QFE::LogLevel::Error);
		return;
	}
	// 同名の保存済みファイルを上書きしないよう連番を付ける。
	auto path = directory / (baseName + L".entity");
	for (unsigned suffix = 1; std::filesystem::exists(path, error) && !error; ++suffix) {
		path = directory / (baseName + L"-" + std::to_wstring(suffix) + L".entity");
	}
	if (error) {
		QFE_LOG("Failed to check the entity save path.", QFE::LogLevel::Error);
		return;
	}
	std::ofstream output(path);
	output << savedEntities.dump(4);
	output.close();
	if (!output) {
		QFE_LOG("Failed to save selected entities.", QFE::LogLevel::Error);
	} else {
		QFE_LOG("Entities saved: " + QFE::ConvertString(path.wstring()), QFE::LogLevel::EditorInfo);
	}
}

bool QFE::EDITOR::EditorWindowManager::ConsumeGameModuleBuildAndReloadRequest() {
	return std::exchange(gameModuleBuildAndReloadRequested_, false);
}

void QFE::EDITOR::EditorWindowManager::QueueSceneSave(
	EditorCommandList& commandList, bool selectSavePath) {
	std::string savePath = sceneManager_->GetCurrentScenePath();
	// 旧JSONを開いている場合は、新しいシーンファイルとして保存する。
	if (selectSavePath || savePath.empty() ||
		_wcsicmp(std::filesystem::path(QFE::ConvertString(savePath)).extension().c_str(), L".scene") != 0) {
		std::wstring selectedFilePath;
		if (!QFE::FRAMEWORK::RequestSaveFilePathFromUser(
			*windowManager_,
			L"Scene Files", L"*.scene",
			selectedFilePath, (QFE::PROJECT::Assets() / "scenes").wstring(), L"scene") || !IsProjectScene(selectedFilePath)) {
			return;
		}
		if (_wcsicmp(std::filesystem::path(selectedFilePath).extension().c_str(), L".scene") != 0) {
			QFE_LOG("Scene files must use the .scene extension.", QFE::LogLevel::Error);
			return;
		}
		savePath = QFE::ConvertString(selectedFilePath);
	}

	commandList.AddCommand(std::make_unique<SaveSceneCommand>(savePath, sceneManager_));
}

bool QFE::EDITOR::EditorWindowManager::IsWindowFocused(EditorWindowType windowType) {
	return editorWindowsMap_[windowType]->GetIsFocus();
}

bool QFE::EDITOR::EditorWindowManager::ConsumeCameraFocusTarget(
	QFE::MATH::Vector3& position, float& radius) {
	auto* hierarchy = static_cast<Hierarchy*>(editorWindowsMap_[EditorWindowType::Hierarchy].get());
	const std::optional<uint32_t> entityId = hierarchy->ConsumeCameraFocusRequest();
	if (!entityId.has_value()) {
		return false;
	}

	EntityManager& entityManager = sceneManager_->GetCurrentSceneEntityManager();
	if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(*entityId)) {
		return false;
	}

	const QFE::MATH::EulerTransform& transform =
		entityManager.GetComponent<QFE::SCENE::TransformComponent>(*entityId).transform;
	position = transform.translate;
	radius = (std::max)({
		std::abs(transform.scale.x),
		std::abs(transform.scale.y),
		std::abs(transform.scale.z),
		1.0f
	});
	return true;
}

void QFE::EDITOR::EditorWindowManager::SetSceneViewerCamera(
	const QFE::MATH::Matrix4x4& viewMatrix,
	const QFE::MATH::Matrix4x4& projectionMatrix,
	bool isOrthographic) {
	auto* sceneViewer = static_cast<SceneViewer*>(editorWindowsMap_[EditorWindowType::SceneViewer].get());
	sceneViewer->SetCameraMatrices(viewMatrix, projectionMatrix, isOrthographic);
}

void QFE::EDITOR::EditorWindowManager::SetAssetDropViewProjection(const QFE::MATH::Matrix4x4& viewProjection) {
	static_cast<SceneViewer*>(editorWindowsMap_[EditorWindowType::SceneViewer].get())->SetAssetDropViewProjection(viewProjection);
	static_cast<GameViewer*>(editorWindowsMap_[EditorWindowType::GameViewer].get())->SetAssetDropViewProjection(viewProjection);
}

bool QFE::EDITOR::EditorWindowManager::IsSceneGizmoCapturingMouse() const {
	const auto it = editorWindowsMap_.find(EditorWindowType::SceneViewer);
	if (it == editorWindowsMap_.end()) {
		return false;
	}
	const auto* sceneViewer = static_cast<const SceneViewer*>(it->second.get());
	return sceneViewer->IsGizmoCapturingMouse();
}
