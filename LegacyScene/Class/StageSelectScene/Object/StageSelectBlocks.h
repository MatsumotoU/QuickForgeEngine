#pragma once
#include "../Engine/Math/Matrix/Matrix4x4.h"
#include "../System/MapChipField.h"
#include <vector>

struct Transform;
struct StageData;
class Vector4;
class Particle;
class Camera;

/// @brief ステージ選択シーンのブロック
class StageSelectBlocks {
public:
	/// @brief コンストラクタ
	/// @param particle パーティクル
	/// @param camera カメラ
	StageSelectBlocks(Particle *particle, Camera *camera)
		: particle_(particle)
		, camera_(camera) {
	}

	/// @brief デストラクタ
	~StageSelectBlocks() = default;

	/// @brief 初期化
	/// @param stageData ステージデータ
	/// @param currentStage 現在のステージ
	/// @param mapChipType マップチップの種類
	void Initialize(const StageData &stageData, uint32_t currentStage, MapChipType mapChipType);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief 親のワールド行列を設定する
	/// @param parentWorldMatrix 親のワールド行列
	void SetParentWorldMatrix(const Matrix4x4 &parentWorldMatrix) { parentWorldMatrix_ = parentWorldMatrix; }

protected:
	std::vector<Transform> transforms_;	// ワールド変換データ
	std::vector<Vector4> colors_;		// 色
	Particle *particle_ = nullptr;		// パーティクル
	Camera *camera_ = nullptr;			// カメラ
	Matrix4x4 parentWorldMatrix_{};		// 親のワールド行列
};