#pragma once
#include "../IEditorUI.h"

class InspectorView final : public IEditorUI {
public:
	InspectorView();
	~InspectorView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
private:
};