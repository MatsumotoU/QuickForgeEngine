#pragma once

class StageSelectScene;

/// @brief ステージ選択シーンの基底フェーズ
class BaseStageSelectScenePhase {
public:
	/// @brief コンストラクタ
	/// @param stageSelectScene ステージ選択シーン
	BaseStageSelectScenePhase(StageSelectScene *stageSelectScene) : stageSelectScene_(stageSelectScene) {}

	/// @brief デストラクタ
	virtual ~BaseStageSelectScenePhase() = default;

	/// @brief 初期化
	virtual void Initialize() = 0;

	/// @brief 更新
	virtual void Update() = 0;

protected:
	StageSelectScene *stageSelectScene_ = nullptr;		// ステージ選択シーン
};

/// @brief ステージ選択シーンの通常フェーズ
class StageSelectScenePhaseIdle : public BaseStageSelectScenePhase {
public:
	/// @brief 継承コンストラクタ
	using BaseStageSelectScenePhase::BaseStageSelectScenePhase;

	/// @brief デストラクタ
	~StageSelectScenePhaseIdle() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};

/// @brief ステージ選択シーンのプッシュフェーズ
class StageSelectScenePhasePush : public BaseStageSelectScenePhase {
public:
	/// @brief 継承コンストラクタ
	using BaseStageSelectScenePhase::BaseStageSelectScenePhase;

	/// @brief デストラクタ
	~StageSelectScenePhasePush() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};

/// @brief ステージ選択シーンの遷移フェーズ
class StageSelectScenePhaseTransition : public BaseStageSelectScenePhase {
public:
	/// @brief 継承コンストラクタ
	using BaseStageSelectScenePhase::BaseStageSelectScenePhase;

	/// @brief デストラクタ
	~StageSelectScenePhaseTransition() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};