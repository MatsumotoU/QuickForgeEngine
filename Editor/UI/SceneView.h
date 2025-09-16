#pragma once
#include "IEditorUI.h"

class SceneView : public IEditorUI {
public:
	SceneView();
	~SceneView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
};