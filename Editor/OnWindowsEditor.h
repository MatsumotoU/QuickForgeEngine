#pragma once
#include "pch.h"
#include "IEditor.h"
#include "UI/UIManager.h"

class OnWindowsEditor final : public IEditor {
public:
	OnWindowsEditor();
	~OnWindowsEditor() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;

private:
	UIManager uiManager_;

};