#pragma once
#include "../IEditorUI.h"

class SaveScene final : public IEditorUI {
public:
	SaveScene() = default;
	~SaveScene() = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;
private:
	std::string sceneName_;
};
