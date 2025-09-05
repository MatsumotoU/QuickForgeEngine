#pragma once
#include "GamePlayer.h"
#include "Map/PredictionLine.h"

inline const int kCheckCount_ = 32;

class Enemy final :public GamePlayer{
public:
	Enemy() = default;
	~Enemy() = default;
	void Initialize(EngineCore* engineCore, Camera* camera) override;
	void Update() override;
	void Draw() override;

	void SetPlayerPos(const Vector2& pos) { playerPos_ = pos; }

private:
	void Shot(Vector2& dir);
	void InitEvaluationValue();
	void TotalEvaluation();
	void SortDirTableValue();
	void SelectedDir();
	std::array<Vector2, kCheckCount_> directionTable_;
	std::array<int, kCheckCount_> dirTableIndexTable_;

	void LifeEvaluation();
	void AttackEvaluation();
	void UniqeEvaluation();

	struct AIWeight {
		float life; // 生存
		float attack;   // 攻撃
		float uniqe;  // 特殊
	};
	AIWeight aiWeight_;
	uint32_t aiLevel_;
	std::vector<int> selectedDirIndexTable_;

	std::array<int, kCheckCount_> totalEvaluationValue_;
	std::array<int, kCheckCount_> lifeEvaluationValue_;
	std::array<int, kCheckCount_> attackEvaluationValue_;
	std::array<int, kCheckCount_> uniqeEvaluationValue_;
	PredictionLine predictionLine_;

	Vector2 playerPos_;

	float delayTimer_;

	int selectedDirIndex_;
	int maxSelect_;
};