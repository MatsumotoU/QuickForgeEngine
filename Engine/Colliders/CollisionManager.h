#pragma once
#include <list>
#include "Collider.h"

class CollisionManager {
public:
	void Initalize();
	void Update(const std::list<Collider*>& colliderList);

private:
	/// <summary>
	/// AとBのマスク値に共通するビットが1つも立っていない場合のみ衝突判定を行う
	/// </summary>
	/// <param name="colliderA"></param>
	/// <param name="colliderB"></param>
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

private:
	std::list<Collider*> colliders_;
};