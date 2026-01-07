#include "editor/include/UI/View/AssetsView.h"
#include "assets/AssetManager.h"
#include <cassert>

#include "engine/include/utility/FileSystems/FileUtility.h"
#include "engine/include/utility/DebugTool/ImGui/ImGuiFlameController.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"

AssetsView::AssetsView() {
	assetManager = nullptr;
	isActive_ = true;
	assetManager = AssetManager::GetInstance();
	assert(assetManager);
	currentHierarchy = ViewHierarchy::Root;
	name_ = "Assets View";
	hierarchyNames = {
		{ViewHierarchy::Root, "Root"},
		{ViewHierarchy::Images, "Images"},
		{ViewHierarchy::Models, "Models"},
		{ViewHierarchy::Materials, "Materials"},
		{ViewHierarchy::Shaders, "Shaders"},
		{ViewHierarchy::Audio, "Audio"},
		{ViewHierarchy::Others, "Others"}
	};

	drawFunctions[ViewHierarchy::Root] = std::bind(&AssetsView::RootView, this);
	drawFunctions[ViewHierarchy::Images] = std::bind(&AssetsView::ImagesView, this);
	drawFunctions[ViewHierarchy::Models] = std::bind(&AssetsView::ModelsView, this);
	drawFunctions[ViewHierarchy::Materials] = std::bind(&AssetsView::MaterialsView, this);
	drawFunctions[ViewHierarchy::Shaders] = std::bind(&AssetsView::ShadersView, this);
	drawFunctions[ViewHierarchy::Audio] = std::bind(&AssetsView::AudioView, this);
	drawFunctions[ViewHierarchy::Others] = std::bind(&AssetsView::OthersView, this);
#ifdef _DEBUG
	// 繧｢繧ｻ繝・ヨ逕ｨUI縺ｮ逋ｻ骭ｲ
	fileGH_ = assetManager->LoadEditorTexture("file.png");
	arrowGH_ = assetManager->LoadEditorTexture("arrow.png");
#endif // _DEBUG

	loadSpace_ = LoadSpace::Memory;
}

void AssetsView::Initialize() {
}

void AssetsView::Update() {
}

void AssetsView::Draw() {
	if (!isActive_) {
		return;
	}

	ImGui::Begin(name_.c_str(), &isActive_);

	if (ImGui::RadioButton("Memory", loadSpace_ == LoadSpace::Memory)) {
		loadSpace_ = LoadSpace::Memory;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("File", loadSpace_ == LoadSpace::File)) {
		loadSpace_ = LoadSpace::File;
	}

	// 繝輔ぃ繧､繝ｫ繧ｷ繧ｹ繝・Β荳翫・繧｢繧ｻ繝・ヨ繧定｡ｨ遉ｺ
	if (loadSpace_ == LoadSpace::File) {
		FilesView();
		ImGui::End();
		return;
	}
	// 繝｡繝｢繝ｪ荳翫・繧｢繧ｻ繝・ヨ繧定｡ｨ遉ｺ
	drawFunctions[currentHierarchy]();
	ImGui::End();
}

// 蜷・ン繝･繝ｼ縺ｮ謠冗判髢｢謨ｰ
void AssetsView::RootView() {
	ImGui::Text("Root");
	ImGui::Separator();
	const int buttonSize = 64;
	const int buttonPadding = 8;
	ImVec2 avail = ImGui::GetContentRegionAvail();

	for (auto it = hierarchyNames.begin(); it != hierarchyNames.end(); ++it) {
		if (it->first == ViewHierarchy::Root) {
			continue;
		}

		// 譛蛻昜ｻ･螟悶・謚倥ｊ霑斐＠蛻､螳・
		if (it != hierarchyNames.begin()) {
			float nextX = ImGui::GetCursorPosX() + buttonSize + buttonPadding;
			if (nextX > avail.x) {
				ImGui::NewLine();
			} else {
				ImGui::SameLine();
			}
		}

		ImGui::BeginGroup();
		if (ImGui::ImageButton(
			it->second.c_str(),
			assetManager->GetTextureManager()->GetTextureSrvHandleGPU(fileGH_).ptr,
			ImVec2(buttonSize, buttonSize),
			ImVec2(0, 0), ImVec2(1, 1),
			ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
			currentHierarchy = it->first;
		}
		ImGui::TextUnformatted(it->second.c_str());
		ImGui::EndGroup();
	}
}

void AssetsView::ImagesView() {
	ImGui::Text("Root->Image");
	ImGui::Separator();
	if (ImGui::ImageButton("Root", assetManager->GetTextureManager()->GetTextureSrvHandleGPU(arrowGH_).ptr,
		ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
		currentHierarchy = ViewHierarchy::Root;
	}

	// 繝｡繝｢繝ｪ荳翫・繝・け繧ｹ繝√Ε繧定｡ｨ遉ｺ
	const std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> handles = assetManager->GetTextureManager()->GetTextureSrvHandleGPUList();
	const int buttonSize = 64;
	const int buttonPadding = 8;
	ImVec2 avail = ImGui::GetContentRegionAvail();
	for (size_t i = 0; i < handles.size(); i++) {
		// 迴ｾ蝨ｨ縺ｮ繧ｫ繝ｼ繧ｽ繝ｫ菴咲ｽｮ縺ｨ谺｡縺ｮ繝懊ち繝ｳ縺ｮ菴咲ｽｮ繧定ｨ育ｮ・
		float nextX = ImGui::GetCursorPosX() + buttonSize + buttonPadding;
		if (i != 0 && nextX > avail.x) {
			ImGui::NewLine();
		} else if (i != 0) {
			ImGui::SameLine();
		}

		ImGui::BeginGroup();
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImU32 bgColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.15f, 0.15f, 0.2f, 1.0f)); // 莉ｻ諢上・閭梧勹濶ｲ
		ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + buttonSize, pos.y + buttonSize), bgColor);

		ImGui::ImageButton(
			std::to_string(i).c_str(),
			handles[i].ptr,
			ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1),
			ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1));
		ImGui::Text("%zu", i);
		ImGui::EndGroup();
	}
}

void AssetsView::ModelsView() {
	ImGui::Text("Root->Model");
	ImGui::Separator();
	if (ImGui::ImageButton("Root", assetManager->GetTextureManager()->GetTextureSrvHandleGPU(arrowGH_).ptr,
		ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
		currentHierarchy = ViewHierarchy::Root;
	}
}

void AssetsView::MaterialsView() {
	ImGui::Text("Root->Material");
	ImGui::Separator();
	if (ImGui::ImageButton("Root", assetManager->GetTextureManager()->GetTextureSrvHandleGPU(arrowGH_).ptr,
		ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
		currentHierarchy = ViewHierarchy::Root;
	}
}

void AssetsView::ShadersView() {
	ImGui::Text("Root->Shader");
	ImGui::Separator();
	if (ImGui::ImageButton("Root", assetManager->GetTextureManager()->GetTextureSrvHandleGPU(arrowGH_).ptr,
		ImVec2(32, 16), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
		currentHierarchy = ViewHierarchy::Root;
	}

	ImGui::Separator();
	ImGui::Text("Gpu Constant Buffer Pools");

	GpuBufferPool* gpuPool = assetManager->GetGpuBufferPool();
	auto poolsInfo = gpuPool->GetPoolsInfo();

	for (IVariableLengthPoolContainer* pool : poolsInfo) {
		if (pool) {
			if (ImGui::CollapsingHeader(pool->GetTypeName())) {
				ImGui::Text("Used: %zu / %zu", pool->UsedSize(), pool->Size());
				ImGui::ProgressBar(static_cast<float>(pool->UsedSize()) / static_cast<float>(pool->Size()));
			}
		}
	}
}

void AssetsView::AudioView() {
	ImGui::Text("Root->Audio");
	ImGui::Separator();
	if (ImGui::ImageButton("Root", assetManager->GetTextureManager()->GetTextureSrvHandleGPU(arrowGH_).ptr,
		ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
		currentHierarchy = ViewHierarchy::Root;
	}
}

void AssetsView::OthersView() {
	ImGui::Text("Root->Others");
	ImGui::Separator();
	if (ImGui::ImageButton("Root", assetManager->GetTextureManager()->GetTextureSrvHandleGPU(arrowGH_).ptr,
		ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1))) {
		currentHierarchy = ViewHierarchy::Root;
	}
}

void AssetsView::FilesView() {
	ImGui::Text("Files View");
	ImGui::Separator();

}
