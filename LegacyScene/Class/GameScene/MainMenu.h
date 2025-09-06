#pragma once
#include "Base/EngineCore.h"

enum class MenuSelect {
	ReturnSelect,
	ResetGame,
};

class MainMenu final {
public:
	MainMenu() = default;
	~MainMenu() = default;
	void Initialize(EngineCore* engineCore,Camera* camera);
	void Update();
	void Draw();

	MenuSelect GetMenuSelect() const;
	Transform transform_;

private:
	EngineCore* engineCore_;
	float time_;

	std::unique_ptr<Model> menuTextModel_;
	std::unique_ptr<Model> returnSelectTextModel_;
	std::unique_ptr<Model> resetGameTextModel_;

	MenuSelect menuSelect_;
	float selectInterval_;
};