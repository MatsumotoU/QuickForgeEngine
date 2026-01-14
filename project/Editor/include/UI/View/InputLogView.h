#pragma once
#include "../IEditorUI.h"
class InputLogView final : public IEditorUI {
public:
	InputLogView();
	~InputLogView() = default;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;
};