#pragma once
#include "../IEditorUI.h"

class SceneProfileView : public IEditorUI {
public:
	SceneProfileView();
	~SceneProfileView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
};
