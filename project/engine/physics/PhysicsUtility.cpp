#include "PhysicsUtility.h"
#include <algorithm>

bool QFE::PHYSICS::UpdateForce(Force* force, PhysicsMaterial* material, float deltaTime) {
	// 入力の検証
	if(force == nullptr || material == nullptr) {
		return false;
	}

	// 力の大きさがほぼゼロの場合は、力をゼロにして終了
	if(force->velocity.Length() < 0.001f) {
		force->velocity = QFE::MATH::Vector3::Zero();
		return true;
	}
	// 力の更新
	force->velocity += force->acceleration * deltaTime;
	const float damping = std::clamp(
		1.0f - material->friction.kineticFriction * deltaTime,
		0.0f,
		1.0f);
	force->velocity = force->velocity * damping;
	force->acceleration = QFE::MATH::Vector3::Zero();
	return true;
}
