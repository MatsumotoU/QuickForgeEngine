#include "engine/include/collider/CollisionDetection.h"
#include "engine/include/core/Math/MyMath.h"

namespace QFE::COLLIDER {
	bool isCollision(const Sphere& sphere1, const Sphere& sphere2) {
		if ((sphere1.center - sphere2.center).Length() <= sphere1.radius + sphere2.radius) {
			return true;
		}
		else {
			return false;
		}
	}
	bool isCollision(const AABB& aabb1, const AABB& aabb2) {
		Vector3 halfSize1 = aabb1.size * 0.5f;
		Vector3 halfSize2 = aabb2.size * 0.5f;
		if (std::abs(aabb1.center.x - aabb2.center.x) > halfSize1.x + halfSize2.x) { return false; }
		if (std::abs(aabb1.center.y - aabb2.center.y) > halfSize1.y + halfSize2.y) { return false; }
		if (std::abs(aabb1.center.z - aabb2.center.z) > halfSize1.z + halfSize2.z) { return false; }
		return true;
	}
	bool isCollision(const Sphere& sphere, const AABB& aabb) {
		Vector3 closestPoint = MyMath::ClosestPoint(sphere, aabb);
		float distanceSquared = (closestPoint - sphere.center).LengthSq();
		return distanceSquared <= (sphere.radius * sphere.radius);
	}
}
