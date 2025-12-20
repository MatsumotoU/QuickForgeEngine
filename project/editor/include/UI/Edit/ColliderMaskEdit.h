#pragma once
#include "../IEditorUI.h"

class ColliderMaskEdit final : public IEditorUI {
public:
	ColliderMaskEdit();
	~ColliderMaskEdit() = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;
private:
	char tag1Buf_[64] = {};
	char tag2Buf_[64] = {};
};
