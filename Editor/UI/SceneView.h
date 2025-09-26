#pragma once
#include "IEditorUI.h"
#include "Core/Math/Vector/Vector3.h"

class SceneView : public IEditorUI {
public:
	SceneView();
	~SceneView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	bool isActiveCamera_;
	Vector3 anchorPoint_;
	float mouseSensitivity_;
};