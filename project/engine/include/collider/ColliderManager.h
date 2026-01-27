#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "ColliderMask.h"
#include "Data/SphereColliderData.h"
#include "Data/AABBColliderData.h"

namespace QFE {

	class ColliderManager final : public Singleton<ColliderManager> {
		friend class Singleton<ColliderManager>;
	public:
		void Initialize();
		void Update();
		void Draw();
		void Finalize();

		bool isRunning = false;
		ColliderTagMask colliderTagMask_;

	private:
		bool isCollision(const Sphere& sphere1, const Sphere& sphere2);
		bool isCollision(const AABB& aabb1, const AABB& aabb2);
		bool isCollision(const Sphere& sphere, const AABB& aabb);

		void SphereToSphereUpdate();
		void AABBToAABBUpdate();
		void SphereToAABBUpdate();
	};

}
