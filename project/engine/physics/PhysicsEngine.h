#pragma once
#include "memory/SparseSets.h"
#include "PhysicsEngineTypes.h"
#include "data/Contact.h"
#include "data/RigidBody.h"
#include "data/SphereCollider.h"
#include <vector>

namespace QFE::PHYSICS {
	/// @brief 物理エンジンのメインクラス
	class PhysicsEngine final {
	public:
		/// @brief 物理エンジンの初期化
		void Initialize();
		/// @brief 物理エンジンの更新
		void Update(float deltaTime);
		/// @brief 物理エンジンのシャットダウン
		void Shutdown();

		[[nodiscard]] RigidBodyHandle CreateRigidBody(const RigidBodyDesc& desc = {});
		bool DestroyRigidBody(RigidBodyHandle handle);
		[[nodiscard]] RigidBody* GetRigidBody(RigidBodyHandle handle);
		[[nodiscard]] const RigidBody* GetRigidBody(RigidBodyHandle handle) const;

		[[nodiscard]] ColliderHandle CreateSphereCollider(
			RigidBodyHandle body,
			const SphereColliderDesc& desc = {});
		bool DestroySphereCollider(ColliderHandle handle);
		[[nodiscard]] SphereCollider* GetSphereCollider(ColliderHandle handle);
		[[nodiscard]] const SphereCollider* GetSphereCollider(ColliderHandle handle) const;

		bool AddForce(RigidBodyHandle handle, const QFE::MATH::Vector3& force);
		bool AddImpulse(RigidBodyHandle handle, const QFE::MATH::Vector3& impulse);

		void SetGravity(const QFE::MATH::Vector3& gravity);
		[[nodiscard]] const QFE::MATH::Vector3& GetGravity() const;
		void SetFixedTimeStep(float fixedTimeStep);
		[[nodiscard]] float GetFixedTimeStep() const;
		[[nodiscard]] const std::vector<Contact>& GetContacts() const;

	private:
		void Step(float deltaTime);
		void IntegrateBodies(float deltaTime);
		void GenerateAndResolveContacts();
		void ResolveContact(const Contact& contact, const SphereCollider& colliderA, const SphereCollider& colliderB);

		QFE::SparseSet<RigidBody> rigidBodies_;
		QFE::SparseSet<SphereCollider> sphereColliders_;
		std::vector<Contact> contacts_;
		QFE::MATH::Vector3 gravity_{ 0.0f, -9.80665f, 0.0f };
		float fixedTimeStep_ = 1.0f / 60.0f;
		float accumulator_ = 0.0f;
		bool initialized_ = false;
	};
}
