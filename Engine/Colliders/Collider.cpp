#include "Collider.h"

Collider::Collider():radius_(1.0f), mask_(0xFFFFFFFF) {}

float Collider::GetRadius() const {
    return radius_;
}

uint32_t Collider::GetMask() const {
	return mask_;
}

nlohmann::json Collider::GetObjectData() const {
	return objData_;
}

void Collider::SetRadius(float r) {
	radius_ = r;
}

void Collider::SetMask(uint32_t mask) {
	mask_ = mask;
}
