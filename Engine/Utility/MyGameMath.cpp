#include "MyGameMath.h"

Vector3 MyGameMath::MakeDirection(const Vector3& rotate) {
    Vector3 result{};
    Vector3 front = { 0.0f,0.0f,1.0f };
    result = Vector3::Transform(front, Matrix4x4::MakeRotateXYZMatrix(rotate));
    return result;
}
