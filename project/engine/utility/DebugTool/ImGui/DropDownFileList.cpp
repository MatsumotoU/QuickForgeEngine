#include "engine/include/utility/DebugTool/ImGui/DropDownFileList.h"
#include "engine/include/utility/FileSystems/FileUtility.h"
using namespace QFE;
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

void QFE::DropDownFileList::AddFileList(const std::string& directoryPath, const std::string& extension)
{
	std::vector<std::string> newFiles = QFE::FILE::GetFilesInDirectory(directoryPath, extension);
	for (const auto& file : newFiles) {
		if (std::find(fileList_.begin(), fileList_.end(), file) == fileList_.end()) {
			fileList_.push_back(file);
		}
	}
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
		isSelected_ = false; // 荳蠎ｦ蜿門ｾ励＠縺溘ｉ繝ｪ繧ｻ繝・ヨ
		return true;
	}
	return false;
}

void DropDownFileList::AddFile(const std::string& fileName) {
	fileList_.push_back(fileName);
}
