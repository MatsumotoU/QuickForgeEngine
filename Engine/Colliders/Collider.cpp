#include "Collider.h"

Collider::Collider() :radius_(1.0f), mask_(0x00000000), isHit_(false) {}

float Collider::GetRadius() const {
	return radius_;
}

uint32_t Collider::GetMask() const {
	return mask_;
}

bool Collider::IsHit() const { 
	return isHit_;
}

void Collider::SetRadius(float r) {
	radius_ = r;
}

void Collider::SetMask(uint32_t mask) {
	mask_ = mask;
}

void Collider::SetHit(bool isHit) {
	isHit_ = isHit;
}
