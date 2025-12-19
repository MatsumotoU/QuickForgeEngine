#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include "editor/include/IEditor.h"
#include "editor/include/UI/UIManager.h"

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
