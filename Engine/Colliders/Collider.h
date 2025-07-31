#pragma once
#include "Math/Vector/Vector3.h"

class Collider {
public:
	Collider();
	~Collider() = default;

public:
	// 衝突時に呼ばれる関数
	virtual void OnCollision() {};

public:// ゲッタ
	float GetRadius() const;
	uint32_t GetMask() const;
	bool IsHit() const;
	virtual Vector3 GetWorldPosition() = 0;

public:// セッタ
	void SetRadius(float r);
	void SetMask(uint32_t mask);
	void SetHit(bool isHit);

private:// メンバ変数
	// 半径
	float radius_;
	// マスク
	uint32_t mask_;
	// 衝突しているかどうか
	bool isHit_;
};