#pragma once
#include "Number.h"

class Score {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	void AddScore(int score);
	int GetScore() { return score_; }

	Vector3 position_;

private:
	Number numbers_[8];
	int oldNumbers_[8];
	int score_ = 0;
	int displayScores_;
	EngineCore* engineCore_ = nullptr;
};