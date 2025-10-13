#pragma once
#include "../IEditorUI.h"
#include <vector>
#include "AppUtility/FileSystems/FileUtility.h"

class LoadScene final : public IEditorUI {
public:
	LoadScene() = default;
	~LoadScene() = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;
private:
	std::string currentScene_;
	std::vector<std::string> sceneList_;
	int selected_;
};
