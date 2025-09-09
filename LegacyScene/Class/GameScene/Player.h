#pragma once
#include "GamePlayer.h"
#include "Map/PredictionLine.h"
#include "Math/Vector/IntVector2.h"

class Player final :public GamePlayer {
public:
	Player() = default;
	~Player() = default;
	void Initialize(EngineCore* engineCore, Camera* camera) override;
	void Update() override;
	void Draw() override;

	void SetEnemyPos(const Vector2& pos) { enemyPos_ = pos; }
	void SetOldBuildMapChipIndex(std::vector<IntVector2>* oldBuildMapChipIndex) { oldBuildMapChipIndex_ = oldBuildMapChipIndex; }

	void ResetActionWeight();

	Vector3 GetActionWeight() const;

private:
	void MouseControl();
	void ControllerControl();
	void Shot();

	void ScanMapCenter();
	void LifeEvaluation();
	void AttackEvaluation();
	void UniqeEvaluation();
	PredictionLine predictionLine_;
	int lifeEvaluationValue_;
	int attackEvaluationValue_;
	int uniqeEvaluationValue_;
	int totalEvaluationValue_;
	Vector3 actionWeight_;
	Vector3 totalActionWeight_;

	Vector2 enemyPos_;

	float mapCenterX_;
	float mapCenterY_;

	bool isClicked_;
	Vector2 clickStartPos_;

	std::vector<IntVector2>* oldBuildMapChipIndex_;

	float timer_;
};