#include "physics/PhysicsEngine.h"
#include <cmath>
#include <iostream>

namespace {
	bool NearlyEqual(float lhs, float rhs, float tolerance = 0.001f) {
		return std::abs(lhs - rhs) <= tolerance;
	}

	bool TestGravityAndForce() {
		QFE::PHYSICS::PhysicsEngine physics;
		physics.Initialize();

		QFE::PHYSICS::RigidBodyDesc fallingDesc{};
		fallingDesc.linearDamping = 1.0f;
		const auto fallingBody = physics.CreateRigidBody(fallingDesc);
		physics.Update(1.0f / 60.0f);

		const QFE::PHYSICS::RigidBody* falling = physics.GetRigidBody(fallingBody);
		if (falling == nullptr || !NearlyEqual(falling->velocity.y, -9.80665f / 60.0f)) {
			std::cerr << "Gravity integration failed.\n";
			return false;
		}

		QFE::PHYSICS::RigidBodyDesc forceDesc{};
		forceDesc.mass = 2.0f;
		forceDesc.linearDamping = 1.0f;
		forceDesc.useGravity = false;
		const auto forceBody = physics.CreateRigidBody(forceDesc);
		if (!physics.AddForce(forceBody, { 4.0f, 0.0f, 0.0f })) {
			std::cerr << "AddForce failed.\n";
			return false;
		}
		physics.Update(1.0f / 60.0f);

		const QFE::PHYSICS::RigidBody* accelerated = physics.GetRigidBody(forceBody);
		if (accelerated == nullptr || !NearlyEqual(accelerated->velocity.x, 2.0f / 60.0f)) {
			std::cerr << "Force integration failed.\n";
			return false;
		}
		return true;
	}

	bool TestSphereCollisionAndTrigger() {
		QFE::PHYSICS::PhysicsEngine physics;
		physics.Initialize();
		physics.SetGravity({ 0.0f, 0.0f, 0.0f });

		QFE::PHYSICS::RigidBodyDesc staticDesc{};
		staticDesc.isStatic = true;
		const auto staticBody = physics.CreateRigidBody(staticDesc);

		QFE::PHYSICS::RigidBodyDesc movingDesc{};
		movingDesc.position = { 1.5f, 0.0f, 0.0f };
		movingDesc.velocity = { -1.0f, 0.0f, 0.0f };
		movingDesc.linearDamping = 1.0f;
		movingDesc.useGravity = false;
		const auto movingBody = physics.CreateRigidBody(movingDesc);

		QFE::PHYSICS::SphereColliderDesc colliderDesc{};
		colliderDesc.radius = 1.0f;
		colliderDesc.material.restitution = 0.5f;
		if (physics.CreateSphereCollider(staticBody, colliderDesc) ==
			QFE::PHYSICS::ColliderHandle::Invalid ||
			physics.CreateSphereCollider(movingBody, colliderDesc) ==
			QFE::PHYSICS::ColliderHandle::Invalid) {
			std::cerr << "Collider creation failed.\n";
			return false;
		}

		physics.Update(1.0f / 60.0f);
		const QFE::PHYSICS::RigidBody* moving = physics.GetRigidBody(movingBody);
		if (physics.GetContacts().size() != 1 || moving == nullptr || moving->velocity.x <= 0.0f) {
			std::cerr << "Sphere collision response failed.\n";
			return false;
		}

		QFE::PHYSICS::PhysicsEngine triggerPhysics;
		triggerPhysics.Initialize();
		triggerPhysics.SetGravity({ 0.0f, 0.0f, 0.0f });
		const auto triggerBodyA = triggerPhysics.CreateRigidBody(staticDesc);
		const auto triggerBodyB = triggerPhysics.CreateRigidBody(movingDesc);
		colliderDesc.isTrigger = true;
		const auto triggerA = triggerPhysics.CreateSphereCollider(triggerBodyA, colliderDesc);
		const auto triggerB = triggerPhysics.CreateSphereCollider(triggerBodyB, colliderDesc);
		if (triggerA == QFE::PHYSICS::ColliderHandle::Invalid ||
			triggerB == QFE::PHYSICS::ColliderHandle::Invalid) {
			std::cerr << "Trigger collider creation failed.\n";
			return false;
		}
		triggerPhysics.Update(1.0f / 60.0f);

		const QFE::PHYSICS::RigidBody* triggerMover = triggerPhysics.GetRigidBody(triggerBodyB);
		if (triggerPhysics.GetContacts().size() != 1 ||
			!triggerPhysics.GetContacts().front().isTrigger ||
			triggerMover == nullptr ||
			triggerMover->velocity.x >= 0.0f) {
			std::cerr << "Trigger contact failed.\n";
			return false;
		}
		return true;
	}
}

int main() {
	if (!TestGravityAndForce() || !TestSphereCollisionAndTrigger()) {
		return 1;
	}
	std::cout << "PhysicsEngine smoke tests passed.\n";
	return 0;
}
