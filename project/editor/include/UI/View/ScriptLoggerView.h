#pragma once
#include "../IEditorUI.h"
#include "Core/Math/Vector/Vector3.h"

class ScriptLoggerView : public IEditorUI {
public:
	ScriptLoggerView();
	~ScriptLoggerView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	uint32_t selectedEntityId_;
};