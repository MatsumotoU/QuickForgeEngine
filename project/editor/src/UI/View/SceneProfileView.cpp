/**
 * @file SceneProfileView.cpp
 * @brief シーン内のエンティティ数や負荷バランスを表示するパネルの実装
 */

#include "editor/include/UI/View/SceneProfileView.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"
#include "engine/include/assets/Script/CsharpVirtualEnvironmentOnQFE.h"

SceneProfileView::SceneProfileView() {
	name_ = "Scene Profile";
	isActive_ = false;
}

void SceneProfileView::Initialize() {
}

void SceneProfileView::Update() {
}

void SceneProfileView::Draw() {
	if (!isActive_) {
		return;
	}

	SceneManager* sceneManager = SceneManager::GetInstance();
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	ImGui::Begin("Scene Profile", &isActive_, ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Separator();
	ImGui::Text("Active Entities: %zu", entityManager->GetActiveEntityIds().size());
	ImGui::Separator();
	float totalTime = sceneManager->updateTime_ +
		sceneManager->preDrawTime_ + sceneManager->drawTime_ +
		sceneManager->postDrawTime_;

	if (totalTime > 0.0f) {
		ImGui::Text("Scene Time Ratios:");
		ImGui::ProgressBar(sceneManager->updateTime_ / totalTime, ImVec2(-1, 0), "Update");
		ImGui::ProgressBar(sceneManager->preDrawTime_ / totalTime, ImVec2(-1, 0), "PreDraw");
		ImGui::ProgressBar(sceneManager->drawTime_ / totalTime, ImVec2(-1, 0), "Draw");
		ImGui::ProgressBar(sceneManager->postDrawTime_ / totalTime, ImVec2(-1, 0), "PostDraw");
	} else {
		ImGui::Text("Scene times are zero.");
	}

	ImGui::Separator();
	ImGui::Text("Update: %.2f ms", sceneManager->updateTime_ / 1'000'000.0f);
	ImGui::Text("PreDraw: %.2f ms", sceneManager->preDrawTime_ / 1'000'000.0f);
	ImGui::Text("Draw: %.2f ms", sceneManager->drawTime_ / 1'000'000.0f);
	ImGui::Text("PostDraw: %.2f ms", sceneManager->postDrawTime_ / 1'000'000.0f);
	
	ImGui::Separator();
	if (ImGui::CollapsingHeader("Scripts")) {
		ImGui::Text("Lua Scripts: %d", LuaScriptResourceManager::GetInstance()->GetScriptCount());
		ImGui::Text("Lua UpdateTime: %.4f ms", LuaScriptResourceManager::GetInstance()->GetTotalUpdateTime() * 1000.0);
		ImGui::Separator();
		ImGui::Text("C# Scripts: %d", CsharpVirtualEnvironmentOnQFE::GetInstance()->GetScriptCount());
	}
	
	ImGui::End();


}
