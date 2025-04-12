#include "Vector2.h"
#include <cmath>
#include <assert.h>

float Vector2::Length() const {
    return sqrt(this->x * this->x + this->y * this->y);
}

Vector2 Vector2::Normalize() const {
    Vector2 result = {};
    if (this->Length() != 0.0f) {
        result.x = this->x / this->Length();
        result.y = this->y / this->Length();
    }
    return result;
}

float Vector2::Dot(const Vector2& v1, const Vector2& v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

float Vector2::Cross(const Vector2& v1, const Vector2& v2) {
    return v1.x * v2.y + v1.y * v2.x;
}

float Vector2::Distance(const Vector2& v1, const Vector2& v2) {
    return  abs(v1.Length() - v2.Length());
}
