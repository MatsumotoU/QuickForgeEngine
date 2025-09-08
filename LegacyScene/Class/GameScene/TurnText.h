#pragma once
#include "Base/EngineCore.h"

class TurnText {
public:
	TurnText() = default;
	~TurnText() = default;
	void Initialize(EngineCore* engineCore,Camera* camera);
	void Update();
	void Draw();

	void ChangeTurn(bool set);
	void SetIsHidden(bool set) { isHidden_ = set; }

	Transform transform_;
	bool GetIsPlayerTurn() const { return isPlayerTurn_; }

private:
	EngineCore* engineCore_;
	std::unique_ptr<Model> playerTurnModel_;
	std::unique_ptr<Model> enemyTurnModel_;

	Vector4 color_;

	bool isHidden_;
	bool isPlayerTurn_;
	bool isChanging_;
	bool requestTurn_;
};