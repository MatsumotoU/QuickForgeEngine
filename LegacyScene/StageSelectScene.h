#pragma once
#include "Base/EngineCore.h"
#include "IScene.h"
#include "Class/StageSelectScene/Object/Triangle.h"

class Particle;
class StageSelectSkydome;
class StageNumber;
class Triangle;
class StageSelectBlocks;
class MapChipField;
class CameraController;
class StageSelectAnchor;
class BaseStageSelectScenePhase;

/// @brief ステージデータ
struct StageData {
	std::vector<std::vector<MapChipField>> stageMapChipFields;	// ステージ別のマップチップフィールド
};

/// @brief ステージ選択シーン
class StageSelectScene : public IScene {

	/// @brief シーン遷移状態
	enum TransitionState {
		None,		// 遷移なし
		ToGame,		// ゲームへ
		ToTitle,	// タイトルへ
	};

public:
	/// @brief マップチップフィールドの種類
	enum MapChipFieldType {
		kFloor,			// 床
		kWall,			// 壁
		kNumMapType,	// 種類数
	};

	static inline constexpr uint32_t kNumStage = 8;	// ステージ数

	/// @brief コンストラクタ
	/// @param engineCore エンジンの中核機能
	/// @param data シーンデータ
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

	/// @brief ブロックの初期化
	void InitializeBlocks();

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

	/// @brief ブロックの親を設定する
	void SetBlocksParent();

	/// @brief カメラのターゲット位置を設定する
	void SetCameraTargetPosition();

	/// @brief 選択音を再生する
	void PlaySelectSound() {
		engineCore_->GetAudioPlayer()->PlayAudio(selectSoundHandle_, "SelectSound.mp3", false);
	}

	/// @brief 選択音を停止する
	void StopSelectSound() {
		engineCore_->GetAudioPlayer()->StopAudio("SelectSound.mp3");
	}

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
	Model *GetCurrentStageModel() { return stageNumberModels_[currentStage_].get(); }

	/// @brief 三角錐のモデルを取得する
	/// @return 三角錐のモデル
	Model *GetTriangleModel(uint32_t index) { return triangleModels_[index].get(); }

	/// @brief 方向に対応する三角錐のモデルを取得する
	/// @return 方向に対応する三角錐のモデル
	Model *GetTriangleModelByDirection() { return triangleModels_[static_cast<uint32_t>(direction_)].get(); }

	/// @brief 現在のステージオブジェクトを取得する
	/// @return 現在のステージオブジェクト
	StageNumber *GetCurrentStageObject() { return stageNumbers_[currentStage_].get(); }

	/// @brief 三角錐を取得する
	/// @param index インデックス
	/// @return 三角錐
	Triangle *GetTriangle(uint32_t index) { return triangles_[index].get(); }

	/// @brief 方向に対応する三角錐を取得する
	/// @return 方向に対応する三角錐
	Triangle *GetTriangleByDirection() { return triangles_[static_cast<uint32_t>(direction_)].get(); }

	/// @brief ブロックを取得する
	/// @return ブロック
	StageSelectBlocks *GetBlocks(uint32_t index) { return blocks_[index].get(); }

	/// @brief 方向を取得する
	/// @return 方向
	Triangle::Direction GetDirection() const { return direction_; }

private:
	float frameCount_ = 0.0f;											// フレームカウント
	EngineCore *engineCore_ = nullptr;									// エンジンの中核機能
	DirectInputManager *directInput_ = nullptr;							// DirectInput
	XInputController *xInput_ = nullptr;								// XInput
	Camera camera_;														// カメラ
	uint32_t selectSoundHandle_ = 0;									// 選択音ハンドル
	uint32_t systemSoundHandle_ = 0;									// システム音ハンドル
	uint32_t systemDecisionSoundHandle_ = 0;							// システム決定音ハンドル
	StageData stageData_;												// ステージデータ
	std::unique_ptr<Model> skydomeModel_ = nullptr;						// 天球モデル
	std::array<std::unique_ptr<Model>, 2> triangleModels_;				// 三角錐モデル
	std::vector<std::unique_ptr<Model>> stageNumberModels_;				// ステージ番号モデル
	std::array<std::unique_ptr<Particle>, 3> blockParticles_;			// ブロックパーティクル
	std::unique_ptr<StageSelectSkydome> skydome_ = nullptr;				// 天球
	std::array<std::unique_ptr<Triangle>, 2> triangles_;				// 三角錐
	std::array<std::unique_ptr<StageNumber>, kNumStage> stageNumbers_;	// ステージ番号
	std::array<std::unique_ptr<StageSelectBlocks>, 3> blocks_;			// ブロック
	std::array<std::unique_ptr<StageSelectAnchor>, kNumStage> anchor_;	// アンカー
	std::unique_ptr<CameraController> cameraController_ = nullptr;		// カメラコントローラー
	uint32_t currentStage_ = 0;											// 現在のステージ
	TransitionState transitionState_ = None;							// シーン遷移状態
	Triangle::Direction direction_ = Triangle::kLeft;					// 方向
	BaseStageSelectScenePhase *currentPhase_ = nullptr;					// 現在のフェーズ
};