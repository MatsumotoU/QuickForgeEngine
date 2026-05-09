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
		void SphereToSphereUpdate();
		void AABBToAABBUpdate();
		void SphereToAABBUpdate();
	};

}
