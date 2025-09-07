#pragma once

class Triangle;

/// @brief 三角錐の基底状態
class BaseTriangleState {
public:
	/// @brief コンストラクタ
	/// @param triangle 三角錐
	BaseTriangleState(Triangle *triangle) : triangle_(triangle) {}

	/// @brief デストラクタ
	virtual ~BaseTriangleState() = default;

	/// @brief 初期化
	virtual void Initialize() = 0;

	/// @brief 更新
	virtual void Update() = 0;

protected:
	Triangle *triangle_ = nullptr;	// 三角錐
};

/// @brief 三角錐の通常状態
class TriangleStateIdle : public BaseTriangleState {
public:
	/// @brief 継承コンストラクタ
	using BaseTriangleState::BaseTriangleState;

	/// @brief デストラクタ
	~TriangleStateIdle() override = default;

	/// @brief 初期化
	void Initialize() override {}

	/// @brief 更新
	void Update() override;
};

/// @brief 三角錐の縮小状態
class TriangleStateShrink : public BaseTriangleState {
public:
	/// @brief 継承コンストラクタ
	using BaseTriangleState::BaseTriangleState;

	/// @brief デストラクタ
	~TriangleStateShrink() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};

/// @brief 三角錐の拡大状態
class TriangleStateExpand : public BaseTriangleState {
public:
	/// @brief 継承コンストラクタ
	using BaseTriangleState::BaseTriangleState;

	/// @brief デストラクタ
	~TriangleStateExpand() override = default;

	/// @brief 初期化
	void Initialize() override;

	/// @brief 更新
	void Update() override;
};