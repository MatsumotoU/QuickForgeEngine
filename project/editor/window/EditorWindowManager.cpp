#include "EditorWindowManager.h"
#include "AllEditorWindow.h"

#include "framework/window/WindowDialogFramework.h"
#include "framework/window/WindowCoreFrameWork.h"

#include "framework/scene/SceneCoreFramework.h"
#include "framework/application/WindowsEngineResources.h"
#include "ecs/entity/EntityManager.h"
#include "components/TransformComponent.h"
#include "components/ObjectInfoComponent.h"
#include "command/EditorCommandList.h"
#include "string/MyString.h"
#include "command/AllCommands.h"
#include "process/ProcessUtil.h"
#include "framework/assets/AssetsCoreFramework.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string_view>

#include <imgui.h>
#include <imgui_stdlib.h>
#include <algorithm>
#include <cmath>
#include <utility>
#include <cstdio>

namespace {
class LogWindow final : public QFE::EDITOR::IEditorWindow {
public:
	 explicit LogWindow(std::vector<std::string>& runtimeLogs) : runtimeLogs_(runtimeLogs) {}
    void Initialize() override { isActive_ = true; }
    void Draw(std::set<uint32_t>&, QFE::EDITOR::EditorCommandList&) override {
        const bool visible = ImGui::Begin("Log", &isActive_);
        isFocus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        if (visible) {
            auto* logger = QFE::MyDebugLog::GetInstance();
            if (ImGui::Button("Clear")) { logger->ClearEntries(); runtimeLogs_.clear(); }
            ImGui::SameLine(); ImGui::Checkbox("Auto-scroll", &autoScroll_);
            ImGui::SameLine(); ImGui::Checkbox("Engine", &showEngine_);
            ImGui::SameLine(); ImGui::Checkbox("Editor", &showEditor_);
			ImGui::SameLine(); ImGui::Checkbox("Runtime", &showRuntime_);
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
				if (showRuntime_) {
					for (const auto& message : runtimeLogs_) {
						if (filter_.PassFilter(message.c_str())) {
							const std::string line = "[Runtime] " + message;
							ImGui::TextUnformatted(line.c_str());
						}
					}
				}
				if (autoScroll_ && atBottom && runtimeLogs_.size() != lastRuntimeCount_) ImGui::SetScrollHereY(1.0f);
				lastRuntimeCount_ = runtimeLogs_.size();
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
	std::vector<std::string>& runtimeLogs_;
	std::size_t lastRuntimeCount_ = 0;
	bool showRuntime_ = true;
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
    return QFE::FRAMEWORK::ASSETS::IsPathWithin(std::filesystem::path(path), QFE::FRAMEWORK::ASSETS::GetProjectAssets() / directory);
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
	gameTags_ = { "Default" };
	if (!QFE::FRAMEWORK::LoadGameInputActions(QFE::FRAMEWORK::ASSETS::GetProjectRoot() / "project.json", gameInputActions_))
		gameConfigStatus_ = "Input actions could not be loaded; defaults are shown.";
	projectName_ = QFE::ConvertString(QFE::FRAMEWORK::ASSETS::GetProjectRoot().filename().wstring());
	try {
		std::ifstream projectFile(QFE::FRAMEWORK::ASSETS::GetProjectRoot() / "project.json");
		const auto project = nlohmann::json::parse(projectFile);
		const auto name = project.find("name");
		if (name != project.end() && name->is_string() && !name->get<std::string>().empty()) {
			projectName_ = name->get<std::string>();
		}
		// 設定がない古いプロジェクトでは Default のみを使用する。
		if (project.contains("game_config") && project["game_config"].is_object() &&
			project["game_config"].contains("tags") && project["game_config"]["tags"].is_array()) {
			for (const auto& tag : project["game_config"]["tags"]) {
				if (!tag.is_string()) continue;
				const auto value = tag.get<std::string>();
				if (!value.empty() && std::find(gameTags_.begin(), gameTags_.end(), value) == gameTags_.end())
					gameTags_.push_back(value);
			}
		}
	} catch (const nlohmann::json::exception&) { }
    EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager);
	editorWindowsMap_.clear();
	editorWindowsMap_[EditorWindowType::Hierarchy] = std::make_unique<Hierarchy>(sceneManager, assetManager);
	editorWindowsMap_[EditorWindowType::Inspector] =
		std::make_unique<Inspector>(&entityManager, &modelLoader, componentInspectorRegistry, &gameTags_, &gameInputActions_);
	editorWindowsMap_[EditorWindowType::AnimationEditor] = std::make_unique<AnimationEditor>(&entityManager);
	editorWindowsMap_[EditorWindowType::AssetBrowser] = std::make_unique<AssetBrowser>(assetManager, windowManager, *sceneManager);
	editorWindowsMap_[EditorWindowType::Logger] = std::make_unique<LogWindow>(runtimeLogs_);
	editorWindowsMap_[EditorWindowType::GameViewer] = std::make_unique<GameViewer>(sceneTextureId, sceneManager, config.windowWidth, config.windowHeight);
    editorWindowsMap_[EditorWindowType::SceneViewer] = std::make_unique<SceneViewer>(sceneTextureId, sceneManager, modelLoader, config.windowWidth, config.windowHeight, componentInspectorRegistry);
}

void QFE::EDITOR::EditorWindowManager::Update() {
	selectedEntities_.clear();
	if (!runtimeNetwork_) return;
	namespace Net = QFE::FRAMEWORK::NETWORK;
	if (waitingForRuntime_) {
		const auto status = Net::Accept(runtimeNetwork_.get(), runtimeConnection_);
		if (status == Net::AcceptStatus::Accepted) {
			waitingForRuntime_ = false;
			Net::StopListening(runtimeNetwork_.get());
			// ランタイムとの通信確立後に通知し、起動失敗と区別する。
			runtimeLogs_.push_back("Runtime log streaming started.");
		} else if (status == Net::AcceptStatus::Error || std::chrono::steady_clock::now() >= runtimeDeadline_) {
			QFE_LOG("Failed to accept the runtime connection.", QFE::LogLevel::Error);
			Net::ShutdownNetworkCore(runtimeNetwork_.get());
			runtimeNetwork_.reset();
			waitingForRuntime_ = false;
		}
	}
	if (!runtimeNetwork_ || waitingForRuntime_ || runtimeConnection_.id == UINT32_MAX) return;
	// フレームごとに受信済みのメッセージだけを取り出し、画面の更新を止めない。
	for (int count = 0; count < 256; ++count) {
		Net::MessageType type{};
		std::string body;
		const auto status = Net::PollMessage(runtimeNetwork_.get(), runtimeConnection_, runtimeReader_, type, body);
		if (status == Net::PollStatus::Pending) break;
		if (status == Net::PollStatus::Closed || status == Net::PollStatus::Error) {
			if (status == Net::PollStatus::Error) QFE_LOG("Failed to receive runtime messages.", QFE::LogLevel::Error);
			// 切断はプロセスの正常終了とは限らないため、ログ受信の終了として表示する。
			runtimeLogs_.push_back(status == Net::PollStatus::Closed
				? "Runtime log streaming stopped."
				: "Runtime log streaming stopped due to a connection error.");
			Net::ShutdownNetworkCore(runtimeNetwork_.get());
			runtimeNetwork_.reset();
			runtimeConnection_ = {};
			break;
		}
		if (type == Net::MessageType::Message) runtimeLogs_.push_back(std::move(body));
	}
}

void QFE::EDITOR::EditorWindowManager::Draw(EditorCommandList& commandList, QFE::FRAMEWORK::WindowsEngineResources& resources) {
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
                QFE::FRAMEWORK::InitializeScene(*sceneManager_);
            }

            // ロード
            if (ImGui::MenuItem("Load Scene", nullptr)) {
                // JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
                    *windowManager_,
                    L"Scene Files (*.scene;*.json)", L"*.scene;*.json",
                    selectedFilePath, (QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "scenes").wstring()) && IsProjectScene(selectedFilePath)) {
                    // Entityの生成
					commandList.AddCommand(std::make_unique<LoadSceneCommand>(QFE::ConvertString(selectedFilePath), sceneManager_));
                }
            }
            if (ImGui::MenuItem("Set Start Scene...")) {
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestGetFilePathFromUser(*windowManager_, L"Scene Files (*.scene;*.json)", L"*.scene;*.json",
                    selectedFilePath, (QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "scenes").wstring()) && IsProjectScene(selectedFilePath)) {
                    const auto projectFile = QFE::FRAMEWORK::ASSETS::GetProjectRoot() / "project.json";
                    try {
                        std::ifstream input(projectFile);
                        auto json = nlohmann::json::parse(input);
                        input.close();
                        json["start_scene"] = QFE::ConvertString(std::filesystem::relative(
                            std::filesystem::path(selectedFilePath), QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "scenes").generic_wstring());
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
                    selectedFilePath, (QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "entities").wstring())) {
                    if (!IsProjectFile(selectedFilePath, ".")) {
                        QFE_LOG("Load entities from the current project's assets directory.");
                    } else {
                        const uint32_t firstEntityId =
                            QFE::FRAMEWORK::LoadSceneEntity(*sceneManager_, QFE::ConvertString(selectedFilePath));
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
			ImGui::MenuItem("Game Config", nullptr, &showGameConfig_);
            for(auto& [type, window] : editorWindowsMap_) {
                bool isActive = window->GetIsActive();
                if (ImGui::MenuItem(window->GetWindowName().c_str(), nullptr, isActive)) {
                    window->SetIsActive(!isActive);
                }
			}
            ImGui::EndMenu();
        }
		// 表示補助を一か所で切り替え、シーン作成中に必要な情報だけを表示する。
		if (ImGui::BeginMenu("View")) {
			#if defined(QFE_MODE_DEBUG) || defined(QFE_MODE_DEVELOPMENT)
			ImGui::MenuItem("Show Colliders", nullptr, &resources.showColliders);
			ImGui::MenuItem("Show Camera Rooms", nullptr, &resources.showCameraRooms);
			#endif
			ImGui::MenuItem("Show Spline Paths", nullptr, &resources.showSplinePaths);
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
                    selectedFilePath, (QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "scenes").wstring()) && IsProjectScene(selectedFilePath)) {

					// 起動ごとに前回の接続とログを初期化し、OSが選んだポートを渡す。
					runtimeNetwork_.reset();
					runtimeConnection_ = {};
					runtimeReader_.pending.clear();
					runtimeLogs_.clear();
					waitingForRuntime_ = false;
					runtimeNetwork_ = QFE::FRAMEWORK::NETWORK::CreateNetworkCore();
					uint16_t port = 0;
					if (!runtimeNetwork_ || !QFE::FRAMEWORK::NETWORK::ListenLocal(runtimeNetwork_.get(), port)) {
						QFE_LOG("Failed to listen for the runtime connection.", QFE::LogLevel::Error);
						runtimeNetwork_.reset();
					} else if (!QFE::ProcessUtil::LaunchExe(L"Runtime.exe", L"--project \"" + QFE::FRAMEWORK::ASSETS::GetProjectRoot().wstring() + L"\" --scene \"" + selectedFilePath + L"\"" + (resources.showColliders ? L" --colliders=on" : L" --colliders=off") + (resources.showCameraRooms ? L" --camera-rooms=on" : L" --camera-rooms=off") + (resources.showSplinePaths ? L" --spline-paths=on" : L" --spline-paths=off") + L" --log-host 127.0.0.1 --log-port " + std::to_wstring(port))) {
						QFE_LOG("Failed to launch the runtime.", QFE::LogLevel::Error);
						runtimeNetwork_.reset();
					} else {
						waitingForRuntime_ = true;
						runtimeDeadline_ = std::chrono::steady_clock::now() + std::chrono::seconds(15);
					}
                }
            }

            ImGui::EndMenu();
		}
		const auto& scenePath = QFE::FRAMEWORK::GetScenePath(*sceneManager_);
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
			if (engineConfig_.assetDir.empty() || engineConfig_.modelDir.empty() ||
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

	if (showGameConfig_) {
		ImGui::Begin("Game Config", &showGameConfig_);
		ImGui::TextUnformatted("Tags");
		ImGui::TextUnformatted("Default is always available. Save changes to the project.");
		size_t removeIndex = gameTags_.size();
		for (size_t i = 0; i < gameTags_.size(); ++i) {
			ImGui::PushID(static_cast<int>(i));
			ImGui::TextUnformatted(gameTags_[i].c_str());
			if (gameTags_[i] != "Default") {
				ImGui::SameLine();
				if (ImGui::SmallButton("Remove")) removeIndex = i;
			}
			ImGui::PopID();
		}
		if (removeIndex < gameTags_.size()) {
			gameTags_.erase(gameTags_.begin() + static_cast<std::ptrdiff_t>(removeIndex));
			gameConfigStatus_ = "Unsaved changes. Existing references are preserved.";
		}
		ImGui::InputText("New Tag", &newGameTag_);
		if (ImGui::Button("Add Tag")) {
			if (newGameTag_.empty() || newGameTag_.find_first_not_of(" \t\r\n") == std::string::npos)
				gameConfigStatus_ = "Tag cannot be empty.";
			else if (std::find(gameTags_.begin(), gameTags_.end(), newGameTag_) != gameTags_.end())
				gameConfigStatus_ = "Tag already exists.";
			else {
				gameTags_.push_back(newGameTag_);
				newGameTag_.clear();
				gameConfigStatus_ = "Unsaved changes.";
			}
		}
		ImGui::Separator();
		ImGui::TextUnformatted("Game Input Actions");
		ImGui::TextDisabled("Bindings apply to the next game launch. Stick directions follow movement actions.");
		const auto keyNames = QFE::FRAMEWORK::GameInputKeyNames();
		size_t removeAction = gameInputActions_.size();
		for (size_t i = 0; i < gameInputActions_.size(); ++i) {
			auto& action = gameInputActions_[i];
			ImGui::PushID(static_cast<int>(i));
			const bool open = ImGui::TreeNode(action.name.c_str());
			ImGui::SameLine();
			if (ImGui::SmallButton("Remove")) removeAction = i;
			if (open) {
				size_t removeKey = action.keys.size();
				for (size_t k = 0; k < action.keys.size(); ++k) {
					ImGui::PushID(static_cast<int>(k));
					if (ImGui::BeginCombo("Key", action.keys[k].c_str())) {
						for (const auto& key : keyNames) {
							if (ImGui::Selectable(key.c_str(), action.keys[k] == key)) {
								action.keys[k] = key;
								gameConfigStatus_ = "Unsaved changes.";
							}
						}
						ImGui::EndCombo();
					}
					ImGui::SameLine();
					if (ImGui::SmallButton("Remove Key")) removeKey = k;
					ImGui::PopID();
				}
				if (removeKey < action.keys.size()) { action.keys.erase(action.keys.begin() + static_cast<std::ptrdiff_t>(removeKey)); gameConfigStatus_ = "Unsaved changes."; }
				if (ImGui::Button("Add Key")) { action.keys.push_back("A"); gameConfigStatus_ = "Unsaved changes."; }
				const char* mouseOptions[] = { "None", "Left", "Right", "Middle", "Button 4" };
				int mouseIndex = action.mouseButton + 1;
				if (ImGui::Combo("Mouse Button", &mouseIndex, mouseOptions, IM_ARRAYSIZE(mouseOptions))) { action.mouseButton = mouseIndex - 1; gameConfigStatus_ = "Unsaved changes."; }
				static const std::pair<const char*, uint32_t> padOptions[] = {
					{"None", 0}, {"A", 0x1000}, {"B", 0x2000}, {"X", 0x4000}, {"Y", 0x8000},
					{"Left Shoulder", 0x0100}, {"Right Shoulder", 0x0200}, {"D-Pad Up", 0x0001},
					{"D-Pad Down", 0x0002}, {"D-Pad Left", 0x0004}, {"D-Pad Right", 0x0008},
					{"Start", 0x0010}, {"Back", 0x0020}, {"Left Stick Button", 0x0040}, {"Right Stick Button", 0x0080}
				};
				const char* padPreview = "Custom";
				for (const auto& option : padOptions) if (option.second == action.gamepadButton) padPreview = option.first;
				if (ImGui::BeginCombo("Gamepad Button", padPreview)) {
					for (const auto& option : padOptions) if (ImGui::Selectable(option.first, option.second == action.gamepadButton)) { action.gamepadButton = option.second; gameConfigStatus_ = "Unsaved changes."; }
					ImGui::EndCombo();
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		if (removeAction < gameInputActions_.size()) { gameInputActions_.erase(gameInputActions_.begin() + static_cast<std::ptrdiff_t>(removeAction)); gameConfigStatus_ = "Unsaved changes. Existing references are preserved."; }
		ImGui::InputText("New Action", &newInputAction_);
		if (ImGui::Button("Add Action")) {
			if (newInputAction_.empty() || newInputAction_.find_first_not_of(" \t\r\n") == std::string::npos) gameConfigStatus_ = "Action name cannot be empty.";
			else if (std::any_of(gameInputActions_.begin(), gameInputActions_.end(), [&](const auto& item) { return item.name == newInputAction_; })) gameConfigStatus_ = "Action already exists.";
			else { gameInputActions_.push_back({ newInputAction_ }); newInputAction_.clear(); gameConfigStatus_ = "Unsaved changes."; }
		}
		if (ImGui::Button("Save Game Config")) {
			try {
				const auto path = QFE::FRAMEWORK::ASSETS::GetProjectRoot() / "project.json";
				std::ifstream input(path, std::ios::binary);
				auto project = nlohmann::json::parse(input);
				input.close();
				if (!project.is_object() || (project.contains("game_config") && !project["game_config"].is_object()))
					throw std::runtime_error("Invalid game config");
				// 他のプロジェクト設定を保持し、タグ一覧だけを書き換える。
				project["game_config"]["tags"] = gameTags_;
				auto& actionData = project["game_config"]["inputActions"];
				actionData = nlohmann::json::array();
				for (const auto& action : gameInputActions_) actionData.push_back({ {"name", action.name}, {"keys", action.keys}, {"mouseButton", action.mouseButton}, {"gamepadButton", action.gamepadButton} });
				std::ofstream output(path, std::ios::binary | std::ios::trunc);
				output << project.dump(4) << '\n';
				output.close();
				gameConfigStatus_ = output ? "Saved. Relaunch the game to apply input bindings." : "Failed to save project.json.";
			} catch (const std::exception&) {
				gameConfigStatus_ = "Failed to read or save project.json.";
			}
		}
		ImGui::TextUnformatted(gameConfigStatus_.c_str());
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
		// 非表示の間のドロップを破棄し、再表示後に古いファイルを処理しない。
		QFE::FRAMEWORK::ConsumeDroppedFiles(*windowManager_);
	}
	// 同じ 3D 線を、それぞれの画像領域に ImGui で重ねる。
	static_cast<SceneViewer*>(editorWindowsMap_[EditorWindowType::SceneViewer].get())->SetDebugLines(&resources.debugLines);
	static_cast<GameViewer*>(editorWindowsMap_[EditorWindowType::GameViewer].get())->SetDebugLines(&resources.debugLines);
	for (auto& [type, window] : editorWindowsMap_) {
        if (type == EditorWindowType::Hierarchy || !window->GetIsActive()) {
            continue;
		}
		window->Draw(selectedEntities_, commandList);
	}
	// 描画順は未定なので、両ウィンドウの入力を読み終えた後に選択を同期する。
	auto* browser = static_cast<AssetBrowser*>(editorWindowsMap_[EditorWindowType::AssetBrowser].get());
	auto* viewer = static_cast<SceneViewer*>(editorWindowsMap_[EditorWindowType::SceneViewer].get());
	const auto clickedAsset = browser->ConsumeClickedAsset();
	const auto wheelSelection = viewer->ConsumePaintSelection();
	if (!clickedAsset.empty()) viewer->SelectPaintAsset(clickedAsset);
	else if (!wheelSelection.empty()) browser->SetPaintSelection(wheelSelection);
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
			QFE::FRAMEWORK::GetSceneEntities(*sceneManager_).SerializeEntityComponents(entityId);
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
		const auto& manager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager_);
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

	const auto directory = QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "entities";
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
	std::string savePath = QFE::FRAMEWORK::GetScenePath(*sceneManager_);
	// 旧JSONを開いている場合は、新しいシーンファイルとして保存する。
	if (selectSavePath || savePath.empty() ||
		_wcsicmp(std::filesystem::path(QFE::ConvertString(savePath)).extension().c_str(), L".scene") != 0) {
		std::wstring selectedFilePath;
		if (!QFE::FRAMEWORK::RequestSaveFilePathFromUser(
			*windowManager_,
			L"Scene Files", L"*.scene",
			selectedFilePath, (QFE::FRAMEWORK::ASSETS::GetProjectAssets() / "scenes").wstring(), L"scene") || !IsProjectScene(selectedFilePath)) {
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

	EntityManager& entityManager = QFE::FRAMEWORK::GetSceneEntities(*sceneManager_);
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

bool QFE::EDITOR::EditorWindowManager::TryGetGameViewerRoomPreview(
	QFE::MATH::Matrix4x4& viewProjection, QFE::MATH::Vector3& cameraPosition) const {
	const auto it = editorWindowsMap_.find(EditorWindowType::GameViewer);
	if (it == editorWindowsMap_.end()) return false;
	return static_cast<const GameViewer*>(it->second.get())->TryGetRoomPreview(viewProjection, cameraPosition);
}

bool QFE::EDITOR::EditorWindowManager::IsSceneGizmoCapturingMouse() const {
	const auto it = editorWindowsMap_.find(EditorWindowType::SceneViewer);
	if (it == editorWindowsMap_.end()) {
		return false;
	}
	const auto* sceneViewer = static_cast<const SceneViewer*>(it->second.get());
	return sceneViewer->IsGizmoCapturingMouse();
}
