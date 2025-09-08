#pragma once
#include "../Engine/Math/Vector/Vector3.h"
#include "../Engine/Math/Matrix/Matrix4x4.h"

class Model;
class DirectInputManager;
class XInputController;
class BaseTriangleState;

/// @brief 三角錐
class Triangle {
public:
	static inline constexpr float kEaseDuration = 10.0f;			// 補間にかける時間(frm)
	static inline constexpr Vector3 smallScale{ 0.1f, 0.1f, 0.1f };	// 縮小サイズ
	static inline constexpr Vector3 largeScale{ 1.0f, 1.0f, 1.0f };	// 拡大サイズ
	static inline constexpr float rotateTime = 8.0f;				// 回転にかける時間(秒)

	/// @brief 方向
	enum Direction {
		kLeft,	// 左
		kRight,	// 右
	};

	/// @brief コンストラクタ
	Triangle() = default;

	/// @brief デストラクタ
	~Triangle();

	/// @brief 初期化
	/// @param model モデル
	/// @param directInput DirectInput
	/// @param xInput XInput
	/// @param direction 方向
	void Initialize(Model *model, DirectInputManager *directInput, XInputController *xInput, Direction direction);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief 状態の切り替え
	/// @param newState 新しい状態
	void ChangeState(BaseTriangleState *newState);

	/// @brief 現在の状態の更新
	void StateUpdate();

	/// @brief 縮小
	void Shrink();

	/// @brief 拡大
	void Expand();

	/// @brief 補間開始時の設定
	void Reset() { easeTimer_ = 0.0f; }

	/// @brief 補間終了したかどうかを取得
	/// @return 補間終了したかどうか
	bool IsEaseFinished() const { return easeTimer_ > kEaseDuration; }

	/// @brief アニメーションが終了したかどうかを取得
	/// @return アニメーションが終了したかどうか
	bool IsFinished() const { return isFinished_; }

	/// @brief DirectInputを取得
	/// @return DirectInput
	DirectInputManager *GetDirectInput() const { return directInput_; }

	/// @brief XInputを取得
	/// @return XInput
	XInputController *GetXInput() const { return xInput_; }

	/// @brief アニメーション終了フラグを設定
	/// @param isFinished アニメーション終了フラグ
	void SetFinished(bool isFinished) { isFinished_ = isFinished; }
	
	/// @brief 親のワールド行列を設定する
	/// @param worldMatrix 親のワールド行列
	void SetParent(const Matrix4x4 &worldMatrix) {
		parentWorldMatrix_ = worldMatrix;
	}

private:
	Model *model_ = nullptr;					// モデル
	DirectInputManager *directInput_ = nullptr;	// DirectInput
	XInputController *xInput_ = nullptr;		// XInput
	Direction direction_ = kLeft;				// 方向
	Matrix4x4 parentWorldMatrix_{};				// 親のワールド行列
	float easeTimer_ = 0.0f;					// 補間にかけた時間(frm)
	BaseTriangleState *currentState_ = nullptr;	// 現在の状態
	bool isFinished_ = false;					// アニメーション終了フラグ
};