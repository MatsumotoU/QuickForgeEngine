#include "CollisionManager.h"

#include "../Base/MyDebugLog.h"

void CollisionManager::Initalize() {
	colliders_.clear();
}

void CollisionManager::Update(const std::list<Collider*>& colliderList) {
	colliders_.clear();
	colliders_ = colliderList;
	// 衝突判定をリセットする
	for (Collider* collider : colliders_) {
		collider->SetHit(false);
	}
	// 衝突判定を行う
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {

		std::list<Collider*>::iterator itrB = itrA;
		itrB++;
		for (; itrB != colliders_.end(); ++itrB) {
			CheckCollisionPair(*itrA, *itrB);
		}
	}
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {
	if (!(colliderA->GetMask() & colliderB->GetMask())) {
		if ((colliderA->GetWorldPosition() - colliderB->GetWorldPosition()).Length() <= (colliderA->GetRadius() + colliderB->GetRadius())) {
			colliderA->OnCollision();
			colliderB->OnCollision();
			colliderA->SetHit(true);
			colliderB->SetHit(true);
		}
	}
}
