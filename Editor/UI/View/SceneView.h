#pragma once
#include "../IEditorUI.h"
#include "Core/Math/Vector/Vector3.h"

class SceneView : public IEditorUI {
public:
	SceneView();
	~SceneView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	void DebugCameraControl();
	void UpdateGizmo();

	uint32_t selectEntityId_;
	bool isDrawGizmo_;

	float cameraMoveT_;
	Vector3 startPos_;
	Vector3 targetRotate_;

	bool isActiveCamera_;
	bool isDrawGrid_;
	Vector3 anchorPoint_;
	float mouseSensitivity_;
};