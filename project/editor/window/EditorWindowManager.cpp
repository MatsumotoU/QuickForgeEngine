#include "EditorWindowManager.h"
#include "AllEditorWindow.h"

#include "framework/window/WindowsWindowFrameWork.h"

#include "scene/SceneManager.h"
#include "design-patterns/EntityManager.h"
#include "components/TransformComponent.h"
#include "command/EditorCommandList.h"
#include "string/MyString.h"
#include "command/AllCommands.h"
#include "process/ProcessUtil.h"
#include "file/FileUtility.h"

#include <imgui.h>
#include <algorithm>
#include <cmath>

void QFE::EDITOR::EditorWindowManager::Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow) {
	mainWindow_ = mainWindow;
	sceneManager_ = sceneManager;
    EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();
	editorWindowsMap_.clear();
	editorWindowsMap_[EditorWindowType::Hierarchy] = std::make_unique<Hierarchy>(&entityManager);
	editorWindowsMap_[EditorWindowType::Inspector] = std::make_unique<Inspector>(&entityManager);
	editorWindowsMap_[EditorWindowType::AnimationEditor] = std::make_unique<AnimationEditor>(&entityManager);
	editorWindowsMap_[EditorWindowType::GameViewer] = std::make_unique<GameViewer>(sceneTextureId);
    editorWindowsMap_[EditorWindowType::SceneViewer] = std::make_unique<SceneViewer>(sceneTextureId, sceneManager);
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
                    mainWindow_,
                    L"JSON Files", L"*.json",
                    selectedFilePath)) {
                    // Entityの生成
					commandList.AddCommand(std::make_unique<LoadSceneCommand>(QFE::ConvertString(selectedFilePath), sceneManager_));
                }
            }
			// セーブ
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
				saveSceneRequested = true;
            }
			if (ImGui::MenuItem("Save Scene As...", nullptr)) {
				saveSceneAsRequested = true;
            }
            // エンティティの保存
            if (ImGui::MenuItem("Save Selected Entities", nullptr)) {
                // JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestSaveFilePathFromUser(
                    mainWindow_,
                    L"JSON Files", L"*.json",
					selectedFilePath)) {
					// 選択されたエンティティの保存
                    for(uint32_t entityId : selectedEntities_) {
						nlohmann::json entityJson =
                            sceneManager_->GetCurrentSceneEntityManager().SerializeEntityComponents(entityId);
						QFE::FILE::SaveJSONToFile(QFE::ConvertString(selectedFilePath), entityJson);
					}
					
                }
			}
			// エンティティのロード
            if (ImGui::MenuItem("Load Entities", nullptr)) {
                // JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
                    mainWindow_,
                    L"JSON Files", L"*.json",
                    selectedFilePath)) {
                    // 選択されたエンティティのロード
                    uint32_t newEntityId = 
                        sceneManager_->LoadEntityOnCurrentSceneFromJsonObject(QFE::ConvertString(selectedFilePath));
                }
            }
            ImGui::EndMenu();
        }
		// ウィンドウメニュー
        if (ImGui::BeginMenu("Window")) {
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
                    mainWindow_,
                    L"Scene Files", L"*.json",
                    selectedFilePath)) {

                    // Entityの保存や、別プロセスの起動
                    QFE::ProcessUtil::LaunchExe("Runtime.exe", "");
                }
            }

            ImGui::EndMenu();
		}
        ImGui::EndMenuBar();
    }

    ImGui::End();

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
	for (auto& [type, window] : editorWindowsMap_) {
        if (type == EditorWindowType::Hierarchy || !window->GetIsActive()) {
            continue;
		}
		window->Draw(selectedEntities_, commandList);
	}
}

void QFE::EDITOR::EditorWindowManager::QueueSceneSave(
	EditorCommandList& commandList, bool selectSavePath) {
	std::string savePath = sceneManager_->GetCurrentScenePath();
	if (selectSavePath || savePath.empty()) {
		std::wstring selectedFilePath;
		if (!QFE::FRAMEWORK::RequestSaveFilePathFromUser(
			mainWindow_,
			L"JSON Files", L"*.json",
			selectedFilePath)) {
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
	radius = std::max({
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

bool QFE::EDITOR::EditorWindowManager::IsSceneGizmoCapturingMouse() const {
	const auto it = editorWindowsMap_.find(EditorWindowType::SceneViewer);
	if (it == editorWindowsMap_.end()) {
		return false;
	}
	const auto* sceneViewer = static_cast<const SceneViewer*>(it->second.get());
	return sceneViewer->IsGizmoCapturingMouse();
}
