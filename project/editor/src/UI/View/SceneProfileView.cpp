/**
 * @file SceneProfileView.cpp
 * @brief シーン内のエンティティ数や負荷バランスを表示するパネルの実装
 */

#include "editor/include/UI/View/SceneProfileView.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/AssetManager.h"

#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptExecutor.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"

using namespace QFE;
SceneProfileView::SceneProfileView() {
	SetName("Scene Profile");
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
#ifdef QFE_OPTIMIZE_OFF
	SceneManager* sceneManager = SceneManager::GetInstance();
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_AlwaysAutoResize);
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
		ImGui::Text("Lua Scripts: %d", SceneManager::GetInstance()->GetLuaScriptExecutor()->GetScriptCount());
		ImGui::Separator();
		ImGui::Text("C# Scripts: %d", SceneManager::GetInstance()->GetCsharpScriptExecutor()->GetScriptCount());
	}

	ImGui::End();
#endif

}
