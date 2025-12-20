#pragma once
#include "../IEditorUI.h"

class InputDebugView final : public IEditorUI {
public:
	InputDebugView();
	~InputDebugView() = default;

	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;

private:
	
};