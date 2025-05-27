#include "Collider.h"

Collider::Collider():radius_(1.0f) {}

float Collider::GetRadius() const {
    return radius_;
}

void Collider::SetRadius(float r) {
	radius_ = r;
}
