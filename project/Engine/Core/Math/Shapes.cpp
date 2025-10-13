#include "Shapes.h"

float Plane::DistanceToPoint(const Vector3& point) {
	return Vector3::Dot(normal, point) - distance;
}
