/**
 * @file EngineProfileView.cpp
 * @brief エンジンのパフォーマンス統計（FPS等）を表示するパネルの実装
 */

#include "editor/include/UI/View/EngineProfileView.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/Profiler.h"
#include <map>

using namespace QFE;
EngineProfileView::EngineProfileView() {
	isActive_ = false;
	SetName("Engine Profile View");
}

void EngineProfileView::Initialize() {
}

void EngineProfileView::Update() {
}

void EngineProfileView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(GetName().c_str(), &isActive_, ImGuiWindowFlags_NoDocking);
	ImGui::Text("Engine Profile");
	ImGui::Separator();
	
	// 時間計測の統計を表示
	ImGui::Text("FPS: %.1f", QFE::EngineGlobalValue::fps);
	ImGui::Text("DeltaTime: %.3f", QFE::EngineGlobalValue::deltaTime);
	ImGui::Text("Frame Time: %.3f ms", static_cast<float>(QFE::Profiler::GetInstance()->GetFrameDuration().count()));

	ImGui::Separator();
	// プロファイルのクリアボタン
	if(ImGui::Button("Clear Profiles")) {
		QFE::Profiler::GetInstance()->ClearScopeProfiles();
	}
	
	// スコーププロファイルの統計を表示
	const auto& scopeProfiles = QFE::Profiler::GetInstance()->GetScopeProfiles();

	// 階層構造を作るための専用ノード構造体
	struct ProfileNode {
		std::map<std::string, ProfileNode> children;
		const std::deque<std::chrono::milliseconds>* durations = nullptr;
		std::string fullScopeName;
	};

	ProfileNode rootNode;

	// "::"で区切ってツリーを構築する
	for (const auto& [scopeName, durations] : scopeProfiles) {
		ProfileNode* currentNode = &rootNode;
		size_t start = 0;
		size_t end = scopeName.find("::");

		while (end != std::string::npos) {
			std::string part = scopeName.substr(start, end - start);
			currentNode = &currentNode->children[part];
			start = end + 2; // "::"の2文字分を進める
			end = scopeName.find("::", start);
		}
		std::string leaf = scopeName.substr(start);
		currentNode = &currentNode->children[leaf];
		currentNode->durations = &durations;
		currentNode->fullScopeName = scopeName; // 平均時間の取得等に使うためのフルネーム
	}

	// ツリーを再帰的に描画するためのラムダ式 (C++14以降で可)
	auto DrawProfileNode = [&](auto& self, const std::string& name, const ProfileNode& node) -> void {
		// 階層を持つか、データを持つ場合のみ表示
		bool isLeaf = (node.durations != nullptr);

		// 葉ノード（データあり）の場合は少し色を変えたり、フラグを変えることも可能
		ImGuiTreeNodeFlags flags = isLeaf ? (node.children.empty() ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None) : ImGuiTreeNodeFlags_DefaultOpen;

		if (ImGui::TreeNodeEx(name.c_str(), flags)) {
			// 計測データがある場合はプロファイルの統計を表示
			if (isLeaf) {
				ImGui::Text("Latest: %.3f ms (Avg: %.3f ms)",
					static_cast<float>(node.durations->back().count()),
					static_cast<float>(QFE::Profiler::GetInstance()->GetAverageScopeDuration(node.fullScopeName).count()));

				// 全体で見たときの割合を表示
				if (static_cast<float>(QFE::Profiler::GetInstance()->GetFrameDuration().count()) > 0.0f) {
					float percentage = (static_cast<float>(node.durations->back().count()) / static_cast<float>(QFE::Profiler::GetInstance()->GetFrameDuration().count())) * 100.0f;
					ImGui::ProgressBar(percentage / 100.0f, ImVec2(0.0f, 0.0f), "");
				}
				else {
					ImGui::Text("No frame duration data available");
				}
				ImGui::Spacing();
			}

			// 子ノードの描画
			for (const auto& [childName, childNode] : node.children) {
				self(self, childName, childNode);
				ImGui::Separator();
			}
			ImGui::TreePop();
		}
		};

	// ルート直下の子ノードから描画を開始する
	for (const auto& [childName, childNode] : rootNode.children) {
		DrawProfileNode(DrawProfileNode, childName, childNode);
	}
	ImGui::End();
}
