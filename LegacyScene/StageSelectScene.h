#pragma once
#include "Base/EngineCore.h"
#include "IScene.h"
#include "Class/StageSelectScene/Object/Triangle.h"

class Particle;
class StageSelectSkydome;
class StageObject;
class Triangle;
class BaseStageSelectBlocks;
class CameraController;
class BaseStageSelectScenePhase;

/// @brief ステージ選択シーン
class StageSelectScene : public IScene {

	/// @brief シーン遷移状態
	enum TransitionState {
		None,		// 遷移なし
		ToGame,		// ゲームへ
		ToTitle,	// タイトルへ
	};

public:
	static inline constexpr uint32_t kNumStage = 8;	// ステージ数

	/// @brief コンストラクタ
	/// @param engineCore エンジンの中核機能
	StageSelectScene(EngineCore *engineCore, nlohmann::json* data);

	/// @brief デストラクタ
	~StageSelectScene() override;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;

	/// @brief 描画
	void Draw() override;

	/// @brief 次のシーンを取得する
	/// @return 次のシーン
	IScene *GetNextScene() override;

	/// @brief カメラの更新
	void CameraUpdate();

	/// @brief フェーズの切り替え
	/// @param newPhase 新しいフェーズ
	void ChangePhase(BaseStageSelectScenePhase *newPhase);

	/// @brief 方向を設定する
	/// @param direction 方向
	void SetDirection(Triangle::Direction direction) {
		direction_ = direction;
	}

	/// @brief 三角錐の親を設定する
	void SetTriangleParent();

	/// @brief カメラのターゲット位置を設定する
	void SetCameraTargetPosition();

	/// @brief 現在のステージをインクリメントする
	void CurrentStageUp() {
		currentStage_++;
	}

	/// @brief 現在のステージをデクリメントする
	void CurrentStageDown() {
		currentStage_--;
	}

	/// @brief 現在のステージを循環させる
	void CurrentStageCircle() {
		currentStage_ = (currentStage_ + kNumStage) % kNumStage;
	}

	/// @brief DirectInputを取得する
	/// @return DirectInput
	DirectInputManager *GetDirectInput() { return directInput_; }

	/// @brief XInputを取得する
	/// @return XInput
	XInputController *GetXInput() { return xInput_; }

	/// @brief カメラコントローラーを取得する
	/// @return カメラコントローラー
	CameraController *GetCameraController() { return cameraController_.get(); }

	/// @brief 現在のステージモデルを取得する
	/// @return 現在のステージモデル
	Model *GetCurrentStageModel() { return stageModels_[currentStage_].get(); }

	/// @brief 三角錐のモデルを取得する
	/// @return 三角錐のモデル
	Model *GetTriangleModel(uint32_t index) { return triangleModels_[index].get(); }

	/// @brief 方向に対応する三角錐のモデルを取得する
	/// @return 方向に対応する三角錐のモデル
	Model *GetTriangleModelByDirection() { return triangleModels_[static_cast<uint32_t>(direction_)].get(); }

	/// @brief 三角錐を取得する
	/// @param index インデックス
	/// @return 三角錐
	Triangle *GetTriangle(uint32_t index) { return triangles_[index].get(); }

	/// @brief 方向に対応する三角錐を取得する
	/// @return 方向に対応する三角錐
	Triangle *GetTriangleByDirection() { return triangles_[static_cast<uint32_t>(direction_)].get(); }

	/// @brief 現在のステージオブジェクトを取得する
	/// @return 現在のステージオブジェクト
	StageObject *GetCurrentStageObject() { return stageObjects_[currentStage_].get(); }

	/// @brief 方向を取得する
	/// @return 方向
	Triangle::Direction GetDirection() const { return direction_; }

private:
	float frameCount_ = 0.0f;									// フレームカウント
	EngineCore *engineCore_ = nullptr;							// エンジンの中核機能
	DirectInputManager *directInput_ = nullptr;					// DirectInput
	XInputController *xInput_ = nullptr;						// XInput
	Camera camera_;												// カメラ
	std::unique_ptr<Model> skydomeModel_;						// 天球モデル
	std::array<std::unique_ptr<Model>, 2> triangleModels_;		// 三角錐モデル
	std::vector<std::unique_ptr<Model>> stageModels_;			// ステージモデル
	std::unique_ptr<Particle> blockParticle_;						// ブロックモデル
	std::unique_ptr<StageSelectSkydome> skydome_;				// 天球
	std::array<std::unique_ptr<Triangle>, 2> triangles_;		// 三角錐
	std::vector<std::unique_ptr<StageObject>> stageObjects_;	// ステージオブジェクト
	std::unique_ptr<BaseStageSelectBlocks> blocks_;				// ブロック
	std::unique_ptr<CameraController> cameraController_;		// カメラコントローラー
	uint32_t currentStage_ = 0;									// 現在のステージ
	TransitionState transitionState_ = None;					// シーン遷移状態
	Triangle::Direction direction_ = Triangle::kLeft;			// 方向
	BaseStageSelectScenePhase *currentPhase_ = nullptr;			// 現在のフェーズ

#ifdef _DEBUG
	DebugCamera debugCamera_;	// デバッグカメラ
	bool isActiveDebugCamera_;	// デバッグカメラが有効かどうか
#endif // _DEBUG
};