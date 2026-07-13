#include "Inspector.h"
#include "design-patterns/EntityManager.h"
#include "components/AllComponent.h"
#include "EngineDefines.h"

#include "command/AllCommands.h"
#include "command/EditorCommandList.h"

QFE::EDITOR::Inspector::Inspector(QFE::EntityManager* entityManager) :
	entityManager_(entityManager), isActive_(true), isFocus_(false) {
}

void QFE::EDITOR::Inspector::Initialize() {
}

void QFE::EDITOR::Inspector::Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) {
	ImGui::Begin(GetWindowName().c_str(), &isActive_);
	isFocus_ = ImGui::IsWindowFocused();

	if (selectedEntities.empty()) {
		ImGui::Text("No entity selected.");
		ImGui::End();
		return;
	}

	uint32_t selectedEntityId = *selectedEntities.begin();

	// 1. エンティティが持っているコンポーネントの型名一覧を取得
	std::vector<std::string> compNames = entityManager_->GetComponentTypeNames(selectedEntityId);

	

	ImGui::End();
}

std::string QFE::EDITOR::Inspector::GetWindowName() {
	return "Inspector";
}

bool QFE::EDITOR::Inspector::GetIsActive() {
	return isActive_;
}

bool QFE::EDITOR::Inspector::SetIsActive(bool isActive) {
	isActive_ = isActive;
	return isActive_;
}

bool QFE::EDITOR::Inspector::GetIsFocus() {
	return isFocus_;
}
