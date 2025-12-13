#include "DropDownFileList.h"
#include "engine/include/utility/FileSystems/FileUtility.h"

DropDownFileList::DropDownFileList() {
	isSelected_ = false;
	fileList_.clear();
	selectedFileName_ = "";
}

void DropDownFileList::LoadFileList(const std::string& directoryPath, const std::string& extension) {
	fileList_.clear();
	fileList_ = QFE::FILE::GetFilesInDirectory(directoryPath, extension);
	selectedFileName_ = "";
	isSelected_ = false;
}

void DropDownFileList::DrawCombo() {
	if (fileList_.empty()) {
		ImGui::Text("No files found.");
		return;
	}
	if (ImGui::BeginCombo("##FileList", "Select a file")) {
		for (const auto& fileName : fileList_) {
			if (ImGui::Selectable(fileName.c_str())) {
				if (!isSelected_) {
					selectedFileName_ = fileName;
					isSelected_ = true;
				}
			}
		}
		ImGui::EndCombo();
	}
}

void DropDownFileList::DrawMenuItem() {
	if (fileList_.empty()) {
		ImGui::MenuItem("No files found.", nullptr, false, false);
		return;
	}
	for (const auto& fileName : fileList_) {
		if (ImGui::MenuItem(fileName.c_str())) {
			selectedFileName_ = fileName;
			isSelected_ = true;
		}
	}
}

bool DropDownFileList::GetSelectedFileName(std::string& fileName) {
	if (isSelected_) {
		fileName = selectedFileName_;
		isSelected_ = false; // 一度取得したらリセチE��
		return true;
	}
	return false;
}

void DropDownFileList::AddFile(const std::string& fileName) {
	fileList_.push_back(fileName);
}
