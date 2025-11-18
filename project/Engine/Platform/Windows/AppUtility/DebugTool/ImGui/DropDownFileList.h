#pragma once
#include "ImGuiInclude.h"
#include <string>
#include <vector>

class DropDownFileList final {
public:
	DropDownFileList();
	~DropDownFileList() = default;

	void LoadFileList(const std::string& directoryPath, const std::string& extension);
	void DrawCombo();
	void DrawMenuItem();
	bool GetSelectedFileName(std::string& fileName);
	void AddFile(const std::string& fileName);
	
private:
	bool isSelected_;
	std::vector<std::string> fileList_;
	std::string selectedFileName_;
};
