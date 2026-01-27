#include "engine/include/core/Math/Shapes.h"

using namespace QFE;

float Plane::DistanceToPoint(const Vector3& point) {
	return Vector3::Dot(normal, point) - distance;
}
