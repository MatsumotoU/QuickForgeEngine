#include "Shapes.h"

using namespace QFE::MATH;

float Plane::DistanceToPoint(const Vector3& point) {
	return Vector3::Dot(normal, point) - distance;
}
