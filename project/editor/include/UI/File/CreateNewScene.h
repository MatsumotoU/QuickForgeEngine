#pragma once
#include "../IEditorUI.h"

class CreateNewScene final : public IEditorUI {
public:
	CreateNewScene() = default;
	~CreateNewScene() = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;
private:
};
