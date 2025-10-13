#pragma once
#include "../IEditorUI.h"

class GameView final : public IEditorUI {
public:
	GameView();
	~GameView() override = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
private:
	bool isSceneViewFocused_;
};