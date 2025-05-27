#pragma once
#include "../Math/Vector/Vector3.h"

class Collider {
public:
	Collider();
	~Collider() = default;

public:
	// 衝突時に呼ばれる関数
	virtual void OnCollision() = 0;

public:// ゲッタ
	float GetRadius() const;
	virtual Vector3 GetWorldPosition() = 0;

public:// セッタ
	void SetRadius(float r);

private:// メンバ変数
	// 半径
	float radius_;
};