#pragma once
#include "IEditor.h"

class OnWindowsEditor final : public IEditor {
public:
	OnWindowsEditor() = default;
	~OnWindowsEditor() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
};