#pragma once
#include <list>
#include "Collider.h"

class CollisionManager {
public:
	void Initalize();
	void Update(const std::list<Collider*>& colliderList);

private:
	void CheckCollisionPair(Collider* colliderA, Collider* colliderB);

private:
	std::list<Collider*> colliders_;
};