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
		if (extension == ".glb" || extension == ".obj") return assets.ImportRawFile(path, error) ? "copied" : "";
		error = "Select a GLB or OBJ model.";
		return {};
	}
	std::string ImportFile(QFE::ASSET::AssetManager& assets, const std::filesystem::path& path, std::string& error) {
		return assets.ImportRawFile(path, error) ? "copied" : "";
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
	for (const auto& path : assets_->ListProjectFiles(status_)) {
		std::string lower = path;
		std::transform(lower.begin(), lower.end(), lower.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (lower.ends_with(".mtl")) continue;
		const auto separator = path.find('/');
		const auto group = separator == std::string::npos ? std::string("(root)") : path.substr(0, separator);
		groups_[group].push_back(path);
	}

}

void QFE::EDITOR::AssetBrowser::Draw(std::set<uint32_t>&, EditorCommandList&) {
	const bool visible = ImGui::Begin(GetWindowName().c_str(), &isActive_);
	isFocus_ = ImGui::IsWindowFocused();
	const auto droppedFiles = windowManager_->ConsumeDroppedFiles();
	if (visible) {
		const ImVec2 position = ImGui::GetWindowPos();
		const ImVec2 size = ImGui::GetWindowSize();
		bool imported = false;
		std::string dropError;
		for (const auto& file : droppedFiles) {
			const float x = static_cast<float>(file.x);
			const float y = static_cast<float>(file.y);
			if (x < position.x || y < position.y || x >= position.x + size.x || y >= position.y + size.y) continue;
			std::string error;
			if (!ImportFile(*assets_, std::filesystem::path(QFE::ConvertString(file.path)), error).empty()) imported = true;
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
		if (ImGui::Button("Import Image...")) {
			std::wstring path;
			if (QFE::FRAMEWORK::RequestGetFilePathFromUser(*windowManager_, L"Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff)", L"*.png;*.jpg;*.jpeg;*.bmp;*.tif;*.tiff", path)) {
				if (assets_->ImportRawFile(std::filesystem::path(path), status_)) Refresh();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Refresh")) Refresh();
		ImGui::SameLine();
		ImGui::TextUnformatted("Project assets");
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
					ImGui::PushID(item.c_str());
					const std::filesystem::path assetPath(std::u8string(
						reinterpret_cast<const char8_t*>(item.data()), item.size()));
					std::error_code error;
					const bool exists = std::filesystem::is_regular_file(QFE::PROJECT::Assets() / assetPath, error);
					const std::string itemLabel = item + (exists ? "" : " (missing)");
					// 選択可能な項目にすることで、クリック表示とドラッグ開始の両方を有効にする。
					if (ImGui::Selectable(itemLabel.c_str(), selectedPath_ == item) || ImGui::IsItemActivated()) {
						selectedPath_ = item;
					}
					if (exists && IsDraggableAsset(item) && ImGui::BeginDragDropSource()) {
						const std::string payload = item;
						ImGui::SetDragDropPayload(AssetPayloadType, payload.c_str(), payload.size() + 1);
						ImGui::TextUnformatted(item.c_str());
						ImGui::EndDragDropSource();
					}
					ImGui::PopID();
				}
				ImGui::Unindent();
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
}

std::string QFE::EDITOR::AssetBrowser::GetWindowName() { return "Asset Browser"; }
bool QFE::EDITOR::AssetBrowser::GetIsActive() { return isActive_; }
bool QFE::EDITOR::AssetBrowser::SetIsActive(bool isActive) { return isActive_ = isActive; }
bool QFE::EDITOR::AssetBrowser::GetIsFocus() { return isFocus_; }
