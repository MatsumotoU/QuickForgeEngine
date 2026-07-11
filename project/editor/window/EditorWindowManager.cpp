#include "EditorWindowManager.h"
#include "AllEditorWindow.h"

#include "framework/window/WindowsWindowFrameWork.h"

#include "scene/SceneManager.h"
#include "design-patterns/EntityManager.h"
#include "command/EditorCommandList.h"
#include "string/MyString.h"

#include <imgui.h>

void QFE::EDITOR::EditorWindowManager::Initialize(QFE::SCENE::SceneManager* sceneManager, ImTextureID sceneTextureId, HWND mainWindow) {
	mainWindow_ = mainWindow;
	sceneManager_ = sceneManager;
    EntityManager& entityManager = sceneManager->GetCurrentSceneEntityManager();
	editorWindows_.push_back(std::make_unique<Hierarchy>(&entityManager));
	editorWindows_.push_back(std::make_unique<SceneViewer>(sceneTextureId));
}

void QFE::EDITOR::EditorWindowManager::Update() {
	selectedEntities_.clear();
}

void QFE::EDITOR::EditorWindowManager::Draw(EditorCommandList& commandList) {
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
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MyMainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }

	// エディタのメインメニュー
    if (ImGui::BeginMenuBar()) {
		// ファイルメニュー
        if (ImGui::BeginMenu("File")) {
            // ロード
            if (ImGui::MenuItem("Load Scene", nullptr)) {
                // JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
                    mainWindow_,
                    L"JSON Files", L"*.json",
                    selectedFilePath)) {
                    // Entityの生成
                    sceneManager_->LoadCurrentSceneFromJson(QFE::ConvertString(selectedFilePath));
                }
            }
			// セーブ
            if (ImGui::MenuItem("Save Scene", nullptr)) {
                // JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
                std::wstring selectedFilePath;
                if (QFE::FRAMEWORK::RequestSaveFilePathFromUser(
                    mainWindow_,
                    L"JSON Files", L"*.json",
                    selectedFilePath)) {
					// Entityの保存
					sceneManager_->SaveCurrentSceneToJson(QFE::ConvertString(selectedFilePath));
                }
            }
            ImGui::EndMenu();
        }
		// ウィンドウメニュー
        if (ImGui::BeginMenu("Window")) {
            for (auto& window : editorWindows_) {
                bool isActive = window->GetIsActive();
                if (ImGui::MenuItem(window->GetWindowName().c_str(), nullptr, isActive)) {
                    window->SetIsActive(!isActive);
                }
			}
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();

	for (auto& window : editorWindows_) {
        if (!window->GetIsActive()) {
            continue;
		}
		window->Draw(selectedEntities_, commandList);
	}
}