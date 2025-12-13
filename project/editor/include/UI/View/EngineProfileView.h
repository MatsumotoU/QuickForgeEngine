#pragma once
#include "../IEditorUI.h"

class EngineProfileView : public IEditorUI {
public:
	EngineProfileView();
	~EngineProfileView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
};
