#include "UIManager.h"
#include "Graphic/PostEffect/RendaringPostprocess.h"
#include "Scene/SceneManager.h"
#include "View/SceneView.h"
#include "View/AssetsView.h"
#include "View/ConsoleView.h"
#include "View/InspectorView.h"
#include "View/HierarchyView.h"
#include "View/GameView.h"
#include "View/EngineProfileView.h"

#include "File/CreateNewScene.h"
#include "File/SaveScene.h"
#include "File/LoadScene.h"

#include "Edit/DebugConsole.h"
#include "Edit/KeyConfigEdit.h"

void UIManager::Initialize() {
	isActiveUI_ = false;

#ifdef _DEBUG
	isActiveUI_ = true;

	// FileUIの初期化
	fileUIs_.push_back(std::make_unique<CreateNewScene>());
	fileUIs_.push_back(std::make_unique<SaveScene>());
	fileUIs_.push_back(std::make_unique<LoadScene>());
	for (auto& ui : fileUIs_) {
		ui->Initialize();
	}

	// ViewUIの初期化
	viewUIs_.push_back(std::make_unique<EngineProfileView>());
	viewUIs_.push_back(std::make_unique<AssetsView>());
	viewUIs_.push_back(std::make_unique<ConsoleView>());
	viewUIs_.push_back(std::make_unique<InspectorView>());
	viewUIs_.push_back(std::make_unique<HierarchyView>());
	viewUIs_.push_back(std::make_unique<GameView>());
	viewUIs_.push_back(std::make_unique<SceneView>());
	for (auto& ui : viewUIs_) {
		ui->Initialize();
	}

	// EditUIの初期化
	editUIs_.push_back(std::make_unique<DebugConsole>());
	editUIs_.push_back(std::make_unique<KeyConfigEdit>());
	for (auto& ui : editUIs_) {
		ui->Initialize();
	}

#endif // _DEBUG
}

void UIManager::Update() {
#ifdef _DEBUG
	for (auto& ui : fileUIs_) {
		ui->Update();
	}
	for (auto& ui : viewUIs_) {
		ui->Update();
	}
	for (auto& ui : editUIs_) {
		ui->Update();
	}
#endif // _DEBUG
}

void UIManager::Draw() {
	if (!isActiveUI_) {
		return;
	}
#ifdef _DEBUG
	// シーンが実行中は色を変える
	bool isScriptRunning = SceneManager::GetInstance()->IsRunningScript();
	if (isScriptRunning) {
		ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.01f, 0.01f, 0.01f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.01f, 0.01f, 0.01f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.01f, 0.01f, 1.0f));
	}

	// メインメニュー描画
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			for (auto& ui : fileUIs_) {
				if (ImGui::MenuItem(ui->GetName().c_str())){
					ui->Run();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			for (auto& ui : editUIs_) {
				if (ImGui::MenuItem(ui->GetName().c_str())) {
					ui->Run();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem("Dock View", nullptr, &RendaringPostprosecess::GetInstance()->isImGuiEnabled_);
			for (auto& ui : viewUIs_) {
				ImGui::MenuItem(ui->GetName().c_str(), nullptr, &ui->isActive_);
			}
			ImGui::EndMenu();
		}

		// シーン再生ボタン
		if (SceneManager::GetInstance()->IsRunningScript()) {
			if (ImGui::Button("||")) {
				SceneManager::GetInstance()->StopScript();
			}
		} else {
			if (ImGui::Button(">")) {
				SceneManager::GetInstance()->StartScript();
			}
		}

		// シーン名
		ImGui::Text(("Scene: " + SceneManager::GetInstance()->GetCurrentSceneName()).c_str());
		ImGui::EndMainMenuBar();
	}

	if (!RendaringPostprosecess::GetInstance()->isImGuiEnabled_) {
		return;
	}

	// Dockする場所を生成
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("DockSpace Demo", nullptr, window_flags);
	ImGui::PopStyleVar(2);
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	for (auto& ui : fileUIs_) {
		ui->Draw();
	}
	for (auto& ui : viewUIs_) {
		ui->Draw();
	}
	for (auto& ui : editUIs_) {
		ui->Draw();
	}

	// 実行中は色を変える
	if (isScriptRunning) {
		ImGui::PopStyleColor(3);
	}
#endif // _DEBUG
}
