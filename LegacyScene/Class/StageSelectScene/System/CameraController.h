#pragma once
#include "Camera/Camera.h"

/// @brief カメラコントローラー
class CameraController {
public:
	static inline constexpr float kLerpDuration = 30.0f;	// 補間にかける時間(frm)

	/// @brief コンストラクタ
	CameraController() = default;

	/// @brief デストラクタ
	~CameraController() = default;

	/// @brief 初期化
	/// @param camera カメラ
	/// @param targetPosition 追従対象の座標
	void Initialize(Camera *camera, const Vector3 &targetPosition);

	/// @brief 更新
	void Update();

	/// @brief 補間開始時の設定
	void Start();

	/// @brief 追従対象の座標を設定
	/// @param targetPosition 追従対象の座標
	void SetTargetPosition(const Vector3 &targetPosition) { targetPosition_ = targetPosition; }

	/// @brief 補間終了したかどうかを取得
	/// @return 補間終了したかどうか
	bool IsFinished() const { return lerpTimer_ > kLerpDuration; }

private:
	Camera *camera_ = nullptr;			// カメラ
	Vector3 startPosition_{};			// 補間開始時の座標
	Vector3 startRotation_{};			// 補間開始時の回転角
	Vector3 targetPosition_{};			// 追従対象の座標
	float lerpTimer_ = 0.0f;			// 補間にかけた時間(frm)
};