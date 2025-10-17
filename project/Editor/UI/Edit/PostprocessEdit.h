#pragma once
#include "../IEditorUI.h"

class PostprocessEdit final : public IEditorUI {
public:
	PostprocessEdit() = default;
	~PostprocessEdit() = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;
private:

};