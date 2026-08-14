#include "PhysicsEngine.h"
#include <algorithm>
#include <cmath>

namespace {
	constexpr float kMinimumMass = 0.0001f;
	constexpr float kMinimumRadius = 0.0001f;
	constexpr float kMaximumFrameTime = 0.25f;
	constexpr float kPositionCorrectionPercent = 0.8f;
	constexpr float kPenetrationSlop = 0.001f;
	constexpr float kVectorEpsilonSq = 0.00000001f;

	[[nodiscard]] uint32_t ToIndex(QFE::PHYSICS::RigidBodyHandle handle) {
		return static_cast<uint32_t>(handle);
	}

	[[nodiscard]] uint32_t ToIndex(QFE::PHYSICS::ColliderHandle handle) {
		return static_cast<uint32_t>(handle);
	}
}

void QFE::PHYSICS::PhysicsEngine::Initialize() {
	Shutdown();
	initialized_ = true;
}

void QFE::PHYSICS::PhysicsEngine::Update(float deltaTime) {
	if (!initialized_ || deltaTime <= 0.0f) {
		return;
	}

	accumulator_ += (std::min)(deltaTime, kMaximumFrameTime);
	bool stepped = false;
	while (accumulator_ >= fixedTimeStep_) {
		Step(fixedTimeStep_);
		accumulator_ -= fixedTimeStep_;
		stepped = true;
	}

	if (stepped) {
		rigidBodies_.Each([](uint32_t, RigidBody& body) {
			body.accumulatedForce = QFE::MATH::Vector3::Zero();
		});
	}
}

void QFE::PHYSICS::PhysicsEngine::Shutdown() {
	contacts_.clear();
	sphereColliders_.clear();
	rigidBodies_.clear();
	accumulator_ = 0.0f;
	initialized_ = false;
}

QFE::PHYSICS::RigidBodyHandle QFE::PHYSICS::PhysicsEngine::CreateRigidBody(const RigidBodyDesc& desc) {
	RigidBody body{};
	body.position = desc.position;
	body.velocity = desc.velocity;
	body.linearDamping = std::clamp(desc.linearDamping, 0.0f, 1.0f);
	body.gravityScale = desc.gravityScale;
	body.useGravity = desc.useGravity;
	body.inverseMass = desc.isStatic ? 0.0f : 1.0f / (std::max)(desc.mass, kMinimumMass);
	return static_cast<RigidBodyHandle>(rigidBodies_.push_back(body));
}

bool QFE::PHYSICS::PhysicsEngine::DestroyRigidBody(RigidBodyHandle handle) {
	const uint32_t index = ToIndex(handle);
	if (!rigidBodies_.Contains(index)) {
		return false;
	}

	std::vector<uint32_t> collidersToRemove;
	for (uint32_t colliderIndex : sphereColliders_.Keys()) {
		const SphereCollider* collider = sphereColliders_.Get(colliderIndex);
		if (collider != nullptr && collider->body == handle) {
			collidersToRemove.push_back(colliderIndex);
		}
	}
	for (uint32_t colliderIndex : collidersToRemove) {
		sphereColliders_.Remove(colliderIndex);
	}
	rigidBodies_.Remove(index);
	return true;
}

QFE::PHYSICS::RigidBody* QFE::PHYSICS::PhysicsEngine::GetRigidBody(RigidBodyHandle handle) {
	return rigidBodies_.Get(ToIndex(handle));
}

const QFE::PHYSICS::RigidBody* QFE::PHYSICS::PhysicsEngine::GetRigidBody(RigidBodyHandle handle) const {
	return rigidBodies_.Get(ToIndex(handle));
}

QFE::PHYSICS::ColliderHandle QFE::PHYSICS::PhysicsEngine::CreateSphereCollider(
	RigidBodyHandle body,
	const SphereColliderDesc& desc) {
	if (GetRigidBody(body) == nullptr || desc.radius <= 0.0f) {
		return ColliderHandle::Invalid;
	}

	SphereCollider collider{};
	collider.body = body;
	collider.localCenter = desc.localCenter;
	collider.radius = (std::max)(desc.radius, kMinimumRadius);
	collider.material = desc.material;
	collider.material.restitution = std::clamp(collider.material.restitution, 0.0f, 1.0f);
	collider.material.friction.staticFriction =
		(std::max)(collider.material.friction.staticFriction, 0.0f);
	collider.material.friction.kineticFriction =
		(std::max)(collider.material.friction.kineticFriction, 0.0f);
	collider.isTrigger = desc.isTrigger;
	return static_cast<ColliderHandle>(sphereColliders_.push_back(collider));
}

bool QFE::PHYSICS::PhysicsEngine::DestroySphereCollider(ColliderHandle handle) {
	const uint32_t index = ToIndex(handle);
	if (!sphereColliders_.Contains(index)) {
		return false;
	}
	sphereColliders_.Remove(index);
	return true;
}

QFE::PHYSICS::SphereCollider* QFE::PHYSICS::PhysicsEngine::GetSphereCollider(ColliderHandle handle) {
	return sphereColliders_.Get(ToIndex(handle));
}

const QFE::PHYSICS::SphereCollider* QFE::PHYSICS::PhysicsEngine::GetSphereCollider(ColliderHandle handle) const {
	return sphereColliders_.Get(ToIndex(handle));
}

bool QFE::PHYSICS::PhysicsEngine::AddForce(
	RigidBodyHandle handle,
	const QFE::MATH::Vector3& force) {
	RigidBody* body = GetRigidBody(handle);
	if (body == nullptr || body->IsStatic()) {
		return false;
	}
	body->accumulatedForce += force;
	return true;
}

bool QFE::PHYSICS::PhysicsEngine::AddImpulse(
	RigidBodyHandle handle,
	const QFE::MATH::Vector3& impulse) {
	RigidBody* body = GetRigidBody(handle);
	if (body == nullptr || body->IsStatic()) {
		return false;
	}
	body->velocity += impulse * body->inverseMass;
	return true;
}

void QFE::PHYSICS::PhysicsEngine::SetGravity(const QFE::MATH::Vector3& gravity) {
	gravity_ = gravity;
}

const QFE::MATH::Vector3& QFE::PHYSICS::PhysicsEngine::GetGravity() const {
	return gravity_;
}

void QFE::PHYSICS::PhysicsEngine::SetFixedTimeStep(float fixedTimeStep) {
	if (fixedTimeStep > 0.0f) {
		fixedTimeStep_ = fixedTimeStep;
		accumulator_ = 0.0f;
	}
}

float QFE::PHYSICS::PhysicsEngine::GetFixedTimeStep() const {
	return fixedTimeStep_;
}

const std::vector<QFE::PHYSICS::Contact>& QFE::PHYSICS::PhysicsEngine::GetContacts() const {
	return contacts_;
}

void QFE::PHYSICS::PhysicsEngine::Step(float deltaTime) {
	IntegrateBodies(deltaTime);
	GenerateAndResolveContacts();
}

void QFE::PHYSICS::PhysicsEngine::IntegrateBodies(float deltaTime) {
	rigidBodies_.Each([this, deltaTime](uint32_t, RigidBody& body) {
		if (body.IsStatic()) {
			return;
		}

		QFE::MATH::Vector3 acceleration = body.accumulatedForce * body.inverseMass;
		if (body.useGravity) {
			acceleration += gravity_ * body.gravityScale;
		}
		body.velocity += acceleration * deltaTime;
		body.velocity = body.velocity * std::pow(body.linearDamping, deltaTime);
		body.position += body.velocity * deltaTime;
	});
}

void QFE::PHYSICS::PhysicsEngine::GenerateAndResolveContacts() {
	contacts_.clear();
	const std::vector<uint32_t> colliderKeys = sphereColliders_.Keys();

	for (size_t first = 0; first < colliderKeys.size(); ++first) {
		const uint32_t colliderIndexA = colliderKeys[first];
		const SphereCollider* colliderA = sphereColliders_.Get(colliderIndexA);
		if (colliderA == nullptr) {
			continue;
		}
		RigidBody* bodyA = GetRigidBody(colliderA->body);
		if (bodyA == nullptr) {
			continue;
		}

		for (size_t second = first + 1; second < colliderKeys.size(); ++second) {
			const uint32_t colliderIndexB = colliderKeys[second];
			const SphereCollider* colliderB = sphereColliders_.Get(colliderIndexB);
			if (colliderB == nullptr || colliderA->body == colliderB->body) {
				continue;
			}
			RigidBody* bodyB = GetRigidBody(colliderB->body);
			if (bodyB == nullptr || (bodyA->IsStatic() && bodyB->IsStatic())) {
				continue;
			}

			const QFE::MATH::Vector3 centerA = bodyA->position + colliderA->localCenter;
			const QFE::MATH::Vector3 centerB = bodyB->position + colliderB->localCenter;
			const QFE::MATH::Vector3 difference = centerB - centerA;
			const float radiusSum = colliderA->radius + colliderB->radius;
			const float distanceSq = difference.LengthSq();
			if (distanceSq >= radiusSum * radiusSum) {
				continue;
			}

			const float distance = std::sqrt((std::max)(distanceSq, 0.0f));
			const QFE::MATH::Vector3 normal =
				distanceSq > kVectorEpsilonSq
				? difference / distance
				: QFE::MATH::Vector3{ 1.0f, 0.0f, 0.0f };

			Contact contact{};
			contact.colliderA = static_cast<ColliderHandle>(colliderIndexA);
			contact.colliderB = static_cast<ColliderHandle>(colliderIndexB);
			contact.bodyA = colliderA->body;
			contact.bodyB = colliderB->body;
			contact.normal = normal;
			contact.penetration = radiusSum - distance;
			contact.point = centerA + normal * (colliderA->radius - contact.penetration * 0.5f);
			contact.isTrigger = colliderA->isTrigger || colliderB->isTrigger;
			contacts_.push_back(contact);

			if (!contact.isTrigger) {
				ResolveContact(contact, *colliderA, *colliderB);
			}
		}
	}
}

void QFE::PHYSICS::PhysicsEngine::ResolveContact(
	const Contact& contact,
	const SphereCollider& colliderA,
	const SphereCollider& colliderB) {
	RigidBody* bodyA = GetRigidBody(contact.bodyA);
	RigidBody* bodyB = GetRigidBody(contact.bodyB);
	if (bodyA == nullptr || bodyB == nullptr) {
		return;
	}

	const float inverseMassSum = bodyA->inverseMass + bodyB->inverseMass;
	if (inverseMassSum <= 0.0f) {
		return;
	}

	const float correctionMagnitude =
		(std::max)(contact.penetration - kPenetrationSlop, 0.0f) /
		inverseMassSum * kPositionCorrectionPercent;
	const QFE::MATH::Vector3 correction = contact.normal * correctionMagnitude;
	bodyA->position -= correction * bodyA->inverseMass;
	bodyB->position += correction * bodyB->inverseMass;

	QFE::MATH::Vector3 relativeVelocity = bodyB->velocity - bodyA->velocity;
	const float velocityAlongNormal = QFE::MATH::Vector3::Dot(relativeVelocity, contact.normal);
	if (velocityAlongNormal >= 0.0f) {
		return;
	}

	const float restitution = (std::min)(
		colliderA.material.restitution,
		colliderB.material.restitution);
	const float normalImpulseMagnitude =
		-(1.0f + restitution) * velocityAlongNormal / inverseMassSum;
	const QFE::MATH::Vector3 normalImpulse = contact.normal * normalImpulseMagnitude;
	bodyA->velocity -= normalImpulse * bodyA->inverseMass;
	bodyB->velocity += normalImpulse * bodyB->inverseMass;

	relativeVelocity = bodyB->velocity - bodyA->velocity;
	QFE::MATH::Vector3 tangent =
		relativeVelocity - contact.normal * QFE::MATH::Vector3::Dot(relativeVelocity, contact.normal);
	if (tangent.LengthSq() <= kVectorEpsilonSq) {
		return;
	}
	tangent = tangent.Normalize();

	const float tangentImpulseMagnitude =
		-QFE::MATH::Vector3::Dot(relativeVelocity, tangent) / inverseMassSum;
	const float friction = std::sqrt(
		colliderA.material.friction.kineticFriction *
		colliderB.material.friction.kineticFriction);
	const float clampedTangentImpulse = std::clamp(
		tangentImpulseMagnitude,
		-normalImpulseMagnitude * friction,
		normalImpulseMagnitude * friction);
	const QFE::MATH::Vector3 tangentImpulse = tangent * clampedTangentImpulse;
	bodyA->velocity -= tangentImpulse * bodyA->inverseMass;
	bodyB->velocity += tangentImpulse * bodyB->inverseMass;
}
