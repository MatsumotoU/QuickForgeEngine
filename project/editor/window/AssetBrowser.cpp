#include "AssetBrowser.h"
#include "AssetDrop.h"
#include "assetfactory/game-data/AssetManager.h"
#include "assetfactory/game-data/GameProjectPaths.h"
#include "framework/window/WindowDialogFramework.h"
#include "window/interface/IGameWindowManager.h"
#include "string/MyString.h"
#include <imgui.h>
#include <algorithm>
#include <cctype>

namespace {
	std::string ImportModel(QFE::ASSET::AssetManager& assets, const std::filesystem::path& path, std::string& error) {
		std::string extension = path.extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (extension == ".glb") return assets.ImportGlb(path, error);
		if (extension == ".obj") return assets.ImportObj(path, error);
		error = "Select a GLB or OBJ model.";
		return {};
	}
}

QFE::EDITOR::AssetBrowser::AssetBrowser(ASSET::AssetManager& assets, WINDOW::IGameWindowManager& windowManager)
	: assets_(&assets), windowManager_(&windowManager) { Initialize(); }

void QFE::EDITOR::AssetBrowser::Initialize() {
	isActive_ = true;
	isFocus_ = false;
	Refresh();
}

void QFE::EDITOR::AssetBrowser::Refresh() {
	groups_.clear();
	status_.clear();
	// 旧プロジェクトのファイルをID付きで登録し、一覧は登録情報から作る。
	if (!assets_->RegisterProjectFiles(status_)) return;
	for (const auto& asset : assets_->Registry().List()) {
		if (asset.type == "other") continue;
		const bool legacy = asset.source.starts_with("assets/");
		if (!legacy && asset.type != "model") continue;
		const std::string path = legacy ? asset.source.substr(7) : "models/" + asset.name;
		std::string lower = path;
		std::transform(lower.begin(), lower.end(), lower.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (lower.ends_with(".mtl")) continue;
		const auto separator = path.find('/');
		const std::string group = separator == std::string::npos ? "(root)" : path.substr(0, separator);
		groups_[group].push_back({ path, asset.id, asset.source });
	}
	bool selectionExists = false;
	for (auto& [name, files] : groups_) {
		std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
			return a.path == b.path ? a.id < b.id : a.path < b.path;
		});
		selectionExists |= std::any_of(files.begin(), files.end(), [&](const auto& item) { return item.id == selectedAssetId_; });
	}
	if (!selectionExists) selectedAssetId_.clear();
}

void QFE::EDITOR::AssetBrowser::Draw(std::set<uint32_t>&, EditorCommandList&) {
	const bool visible = ImGui::Begin(GetWindowName().c_str(), &isActive_);
	isFocus_ = ImGui::IsWindowFocused();
	const auto droppedFiles = windowManager_->ConsumeDroppedFiles();
	if (visible) {
		bool refreshRequested = false;
		const ImVec2 position = ImGui::GetWindowPos();
		const ImVec2 size = ImGui::GetWindowSize();
		bool imported = false;
		std::string dropError;
		for (const auto& file : droppedFiles) {
			const float x = static_cast<float>(file.x);
			const float y = static_cast<float>(file.y);
			if (x < position.x || y < position.y || x >= position.x + size.x || y >= position.y + size.y) continue;
			std::string error;
			if (!ImportModel(*assets_, std::filesystem::path(QFE::ConvertString(file.path)), error).empty()) imported = true;
			else dropError = error;
		}
		if (imported) Refresh();
		if (!dropError.empty()) status_ = dropError;
		if (ImGui::Button("Import Model...")) {
			std::wstring path;
			if (QFE::FRAMEWORK::RequestGetFilePathFromUser(*windowManager_, L"Model Files (*.glb;*.obj)", L"*.glb;*.obj", path)) {
				if (!ImportModel(*assets_, std::filesystem::path(path), status_).empty()) Refresh();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Refresh")) Refresh();
		ImGui::SameLine();
		ImGui::TextUnformatted("Registered project assets");
		if (!status_.empty()) ImGui::TextWrapped("%s", status_.c_str());
		if (groups_.empty() && status_.empty()) ImGui::TextUnformatted("No assets found.");
		for (const auto& [name, files] : groups_) {
			ImGui::PushID(name.c_str());
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);
			const std::string label = name + " (" + std::to_string(files.size()) + ")##GroupHeader";
			if (ImGui::CollapsingHeader(label.c_str())) {
				ImGui::Indent();
				if (files.empty()) ImGui::TextDisabled("(empty)");
				for (const auto& item : files) {
					ImGui::PushID(item.id.c_str());
					const std::filesystem::path assetPath(std::u8string(
						reinterpret_cast<const char8_t*>(item.source.data()), item.source.size()));
					std::error_code error;
					const bool exists = std::filesystem::is_regular_file(QFE::PROJECT::Root() / assetPath, error);
					const std::string itemLabel = item.path + (exists ? "" : " (missing)");
					// 選択可能な項目にすることで、クリック表示とドラッグ開始の両方を有効にする。
					if (ImGui::Selectable(itemLabel.c_str(), selectedAssetId_ == item.id) || ImGui::IsItemActivated()) {
						selectedAssetId_ = item.id;
					}
					std::string extension = assetPath.extension().string();
					std::transform(extension.begin(), extension.end(), extension.begin(),
						[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
					if ((extension == ".glb" || extension == ".obj") && ImGui::BeginPopupContextItem("AssetContext")) {
						if (ImGui::MenuItem("Replace Model...")) {
							std::wstring replacement;
							if (QFE::FRAMEWORK::RequestGetFilePathFromUser(*windowManager_, L"Model Files (*.glb;*.obj)", L"*.glb;*.obj", replacement) &&
								assets_->ReplaceModel(item.id, std::filesystem::path(replacement), status_)) refreshRequested = true;
						}
						ImGui::EndPopup();
					}
					if (exists && IsDraggableAsset(item.path) && ImGui::BeginDragDropSource()) {
						const std::string payload = item.path + '\n' + item.id + '\n' + item.source;
						ImGui::SetDragDropPayload(AssetPayloadType, payload.c_str(), payload.size() + 1);
						ImGui::TextUnformatted(item.path.c_str());
						ImGui::EndDragDropSource();
					}
					ImGui::PopID();
				}
				ImGui::Unindent();
			}
			ImGui::PopID();
		}
		if (refreshRequested) Refresh();
	}
	ImGui::End();
}

std::string QFE::EDITOR::AssetBrowser::GetWindowName() { return "Asset Browser"; }
bool QFE::EDITOR::AssetBrowser::GetIsActive() { return isActive_; }
bool QFE::EDITOR::AssetBrowser::SetIsActive(bool isActive) { return isActive_ = isActive; }
bool QFE::EDITOR::AssetBrowser::GetIsFocus() { return isFocus_; }
