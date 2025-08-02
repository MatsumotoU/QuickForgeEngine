#pragma once
#include "../Math/Vector/Vector3.h"
#include "Utility/SimpleJson.h"

class Collider {
public:
	Collider();
	~Collider() = default;

public:
	// 衝突時に呼ばれる関数
	virtual void OnCollision(const nlohmann::json& otherData) = 0;

public:// ゲッタ
	float GetRadius() const;
	uint32_t GetMask() const;
	virtual Vector3 GetWorldPosition() = 0;
	virtual nlohmann::json GetObjectData() const;

public:// セッタ
	void SetRadius(float r);
	void SetMask(uint32_t mask);

private:// メンバ変数
	// 半径
	float radius_;
	// マスク
	uint32_t mask_;

protected:
	// オブジェクトのデータ
	nlohmann::json objData_;
};