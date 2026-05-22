/**
 * @file UIManager.cpp
 * @brief エディタUIの統合管理を行うクラスの実装
 */

#include "editor/include/UI/UIManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Bridge/EngineBridgeProvider.h"

#include "engine/include/graphic/PostEffect/RenderingPostprocess.h"
#include "engine/include/scene/SceneManager.h"
#include "editor/include/UI/View/SceneView.h"
#include "editor/include/UI/View/AssetsView.h"
#include "editor/include/UI/View/ConsoleView.h"
#include "editor/include/UI/View/InspectorView.h"
#include "editor/include/UI/View/HierarchyView.h"
#include "editor/include/UI/View/GameView.h"
#include "editor/include/UI/View/EngineProfileView.h"
#include "editor/include/UI/View/ScriptLoggerView.h"
#include "editor/include/UI/View/SceneProfileView.h"
#include "editor/include/UI/View/InputDebugView.h"
#include "editor/include/UI/View/InputLogView.h"

#include "editor/include/UI/File/CreateNewScene.h"
#include "editor/include/UI/File/LoadProject.h"
#include "editor/include/UI/File/SaveScene.h"
#include "editor/include/UI/File/LoadScene.h"
#include "editor/include/UI/File/CreateNewProject.h"

#include "editor/include/UI/Edit/DebugConsole.h"
#include "editor/include/UI/Edit/KeyConfigEdit.h"
#include "editor/include/UI/Edit/PostprocessEdit.h"
#include "editor/include/UI/Edit/ColliderMaskEdit.h"
#include "editor/include/UI/Edit/AnimationEditor.h"

using namespace QFE;

/** @brief 初期化 */
void UIManager::Initialize() {
	isActiveUI_ = false;

#ifdef QFE_OPTIMIZE_OFF
	isActiveUI_ = true;

	// FileUIの初期化
	fileUIs_.push_back(std::make_unique<CreateNewProject>());
	fileUIs_.push_back(std::make_unique<LoadProject>());
	fileUIs_.push_back(std::make_unique<CreateNewScene>());
	fileUIs_.push_back(std::make_unique<SaveScene>());
	fileUIs_.push_back(std::make_unique<LoadScene>());
	for (auto& ui : fileUIs_) {
		ui->Initialize();
	}

	// ViewUIの初期化
	viewUIs_.push_back(std::make_unique<SceneProfileView>());
	viewUIs_.push_back(std::make_unique<EngineProfileView>());
	viewUIs_.push_back(std::make_unique<ScriptLoggerView>());
	viewUIs_.push_back(std::make_unique<AssetsView>());
	viewUIs_.push_back(std::make_unique<ConsoleView>());
	viewUIs_.push_back(std::make_unique<InspectorView>());
	viewUIs_.push_back(std::make_unique<HierarchyView>());
	viewUIs_.push_back(std::make_unique<GameView>());
	viewUIs_.push_back(std::make_unique<SceneView>());
	viewUIs_.push_back(std::make_unique<InputDebugView>());
	viewUIs_.push_back(std::make_unique<InputLogView>());
	for (auto& ui : viewUIs_) {
		ui->Initialize();
	}

	// EditUIの初期化
	editUIs_.push_back(std::make_unique<DebugConsole>());
	editUIs_.push_back(std::make_unique<KeyConfigEdit>());
	editUIs_.push_back(std::make_unique<PostprocessEdit>());
	editUIs_.push_back(std::make_unique<ColliderMaskEdit>());
	editUIs_.push_back(std::make_unique<AnimationEditor>());
	for (auto& ui : editUIs_) {
		ui->Initialize();
	}

#endif // _DEBUG
}

void UIManager::Update() {
#ifdef QFE_OPTIMIZE_OFF
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

/** @brief 描画 */
void UIManager::Draw() {
	if (!isActiveUI_) {
		return;
	}
#ifdef QFE_OPTIMIZE_OFF
	// シーンが実行中は色を変える
	bool isScriptRunning = SceneManager::GetInstance()->IsRunningScript();
	if (isScriptRunning && RenderingPostprocess::GetInstance()->isImGuiEnabled_) {
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
			ImGui::MenuItem("Dock View", nullptr, &RenderingPostprocess::GetInstance()->isImGuiEnabled_);
			for (auto& ui : viewUIs_) {
				ImGui::MenuItem(ui->GetName().c_str(), nullptr, &ui->isActive_);
			}
			ImGui::EndMenu();
		}

		// シーン再生ボタン
		SceneManager* sceneManager = SceneManager::GetInstance();
		if (sceneManager->IsRunningScript()) {
			if (ImGui::Button("StopScene")) {
				sceneManager->StopScript();
			}

			ImGui::SameLine();
			if (sceneManager->IsPauseScript()) {
				if (ImGui::Button("ResumeScene")) {
					sceneManager->ResumeScene();
				}
			} else {
				if (ImGui::Button("PauseScene")) {
					sceneManager->PauseScene();
				}
			}

		} else {
			if (ImGui::Button("StartScene")) {
				IEngineBridge* engineBridge = QFE::BRIDGE::GetBridge();
				engineBridge->ClearRuntimeDebugLogs();

				SceneManager::GetInstance()->StartScript();
			}
		}

		// シーン名
		ImGui::Text(("Project: " + AssetManager::GetInstance()->GetResourceDirectoryManager()->GetProjectName()).c_str());
		ImGui::Text(("Scene: " + SceneManager::GetInstance()->GetCurrentSceneName()).c_str());

		// C#を再コンパイルするボタン
		if (ImGui::Button("Recompile C#")) {
			IEngineBridge* engineBridge = QFE::BRIDGE::GetBridge();
			engineBridge->ReCompileCsharpScripts();
		}

		ImGui::EndMainMenuBar();
	}

	if (!RenderingPostprocess::GetInstance()->isImGuiEnabled_) {
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
	if (isScriptRunning && RenderingPostprocess::GetInstance()->isImGuiEnabled_) {
		ImGui::PopStyleColor(3);
	}
#endif // _DEBUG
}
