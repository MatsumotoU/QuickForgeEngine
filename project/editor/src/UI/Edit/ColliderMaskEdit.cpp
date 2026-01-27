/**
 * @file ColliderMaskEdit.cpp
 * @brief 衝突判定のマスク設定を編集するパネルの実装
 */

#include "editor/include/UI/Edit/ColliderMaskEdit.h"
#include "collider/ColliderManager.h"
using namespace QFE;
ColliderMaskEdit::ColliderMaskEdit() {
	name_ = "Collider Mask Edit";
	isActive_ = false;
}

void ColliderMaskEdit::Initialize() {
	tag1Buf_[0] = '\0';
	tag2Buf_[0] = '\0';

}

void ColliderMaskEdit::Update() {
	// このUIは更新処理を必要としないため、何もしない
}

void ColliderMaskEdit::Draw() {
	if (!isActive_) { return; }

	ColliderManager* colliderManager = ColliderManager::GetInstance();
	ImGui::Begin("Collider Mask Edit", &isActive_, ImGuiWindowFlags_NoDocking);
	ImGui::Text("Collider Tag Mask Pairs");
	// 既存のタグマスクペア表示
	for (const auto& pair : colliderManager->colliderTagMask_.GetTagMaskPairs()) {
		ImGui::Text("%s - %s", pair.first.c_str(), pair.second.c_str());
		ImGui::SameLine();
		if (ImGui::Button(std::string("Delete##" + pair.first + pair.second).c_str())) {
			colliderManager->colliderTagMask_.EraseTagMaskPair(pair.first, pair.second);
		}
	}
	ImGui::Separator();
	ImGui::InputText("Tag 1", tag1Buf_, IM_ARRAYSIZE(tag1Buf_));
	ImGui::InputText("Tag 2", tag2Buf_, IM_ARRAYSIZE(tag2Buf_));
	if (ImGui::Button("Add Tag Mask Pair")) {
		std::string tag1Str(tag1Buf_);
		std::string tag2Str(tag2Buf_);
		if (!tag1Str.empty() && !tag2Str.empty()) {
			colliderManager->colliderTagMask_.AddTagMaskPair(tag1Str, tag2Str);
			tag1Buf_[0] = '\0';
			tag2Buf_[0] = '\0';
		}
	}

	ImGui::End();
}

void ColliderMaskEdit::Run() {
	isActive_ = !isActive_;
}
