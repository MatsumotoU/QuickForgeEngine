#include "Vector3.h"
#include "../Matrix/Matrix4x4.h"
#include "Vector4.h"
#include "Vector2.h"
#include <math.h>
#include <assert.h>
#include <algorithm>
#include <cmath>
#include "Camera/Camera.h"

Vector3 Vector3::Zero() {
	Vector3 result{};
	result = { 0.0f,0.0f,0.0f };
	return result;
}

Vector2 Vector3::XY() {
	return Vector2(this->x,this->y);
}

float Vector3::Length() const {
	return sqrtf(this->x * this->x + this->y * this->y + this->z * this->z);
}

Vector3 Vector3::Normalize() const {
	Vector3 result = {};

	if (this->Length() == 0.0f) {
		return result;
	}
	result.x = this->x / this->Length();
	result.y = this->y / this->Length();
	result.z = this->z / this->Length();

	return result;
}

Vector3 Vector3::Normalize(const Vector3& vector) {
	Vector3 result = {};

	if (vector.Length() != 0.0f) {
		result.x = vector.x / vector.Length();
		result.y = vector.y / vector.Length();
		result.z = vector.z / vector.Length();
	}

	return result;
}

float Vector3::Dot(const Vector3& v1, const Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2) {
	Vector3 result = {};
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

Vector3 Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t) {
	Vector3 result{};
	result.x = v1.x * (1.0f - t) + v2.x * t;
	result.y = v1.y * (1.0f - t) + v2.y * t;
	result.z = v1.z * (1.0f - t) + v2.z * t;
	return result;
}

Vector3 Vector3::Slerp(const Vector3& v1, const Vector3& v2, float t) {
	Vector3 result{};

	// 正規化
	Vector3 from = v1.Normalize();
	Vector3 to = v2.Normalize();

	// 2ベクトル間の角度を求める
	float dot = std::clamp(Vector3::Dot(from, to), -1.0f, 1.0f);
	float theta = std::acosf(dot);

	float sinTheta = std::sinf(theta);
	float sinThetaFrom = std::sinf(theta * (1.0f - t));
	float sinThetaTo = std::sinf(theta * t);

	Vector3 normalizeVector{};
	if (sinTheta != 0.0f) {
		normalizeVector.x = (from.x * sinThetaFrom + to.x * sinThetaTo) / sinTheta;
		normalizeVector.y = (from.y * sinThetaFrom + to.y * sinThetaTo) / sinTheta;
		normalizeVector.z = (from.z * sinThetaFrom + to.z * sinThetaTo) / sinTheta;
	} else {
		normalizeVector = from; // 角度が0の場合はそのまま
	}

	float length1 = v1.Length();
	float length2 = v2.Length();

	float length = length1 * (1.0f - t) + length2 * t;

	return normalizeVector * length;
}

Vector3 Vector3::BezierCurve(const Vector3& p0, const Vector3& p1, const Vector3& p2, float t) {
	Vector3 result{};
	Vector3 p0p1 = Lerp(p0, p1, t);
	Vector3 p1p2 = Lerp(p1, p2, t);
	result = Lerp(p0p1, p1p2, t);
	return result;
}

Vector3 Vector3::CatmullRom(const std::vector<Vector3>& controlPoints, float t) {
	Vector3 result{};
	int arraySize = static_cast<int>(controlPoints.size()) - 1;
	if (arraySize < 3) {
		return result;
	}
	float spritT = 1.0f / static_cast<float>(arraySize);
	int beginIndex = static_cast<int>(t / spritT);

	Vector3 p0 = controlPoints[std::clamp(beginIndex - 1, 0, arraySize)];
	Vector3 p1 = controlPoints[std::clamp(beginIndex, 0, arraySize)];
	Vector3 p2 = controlPoints[std::clamp(beginIndex + 1, 0, arraySize)];
	Vector3 p3 = controlPoints[std::clamp(beginIndex + 2, 0, arraySize)];

	float nowT = t / spritT;
	nowT = fmodf(nowT, 1.0f);
	result.x =
		(-p0.x + p1.x * 3.0f - p2.x * 3.0f + p3.x) * powf(nowT, 3.0f) +
		(p0.x * 2.0f - p1.x * 5.0f + p2.x * 4.0f - p3.x) * powf(nowT, 2.0f) +
		(-p0.x + p2.x) * nowT +
		p1.x * 2.0f;

	result.y =
		(-p0.y + p1.y * 3.0f - p2.y * 3.0f + p3.y) * powf(nowT, 3.0f) +
		(p0.y * 2.0f - p1.y * 5.0f + p2.y * 4.0f - p3.y) * powf(nowT, 2.0f) +
		(-p0.y + p2.y) * nowT +
		p1.y * 2.0f;

	result.z =
		(-p0.z + p1.z * 3.0f - p2.z * 3.0f + p3.z) * powf(nowT, 3.0f) +
		(p0.z * 2.0f - p1.z * 5.0f + p2.z * 4.0f - p3.z) * powf(nowT, 2.0f) +
		(-p0.z + p2.z) * nowT +
		p1.z * 2.0f;

	result.x *= 0.5f;
	result.y *= 0.5f;
	result.z *= 0.5f;

	return result;
}

Vector3 Vector3::Transform(const Vector3& vector, const Matrix4x4& matrix) {
	Vector3 result = {};

	float w = vector.x * matrix.m[0][3] + vector.y * matrix.m[1][3] + vector.z * matrix.m[2][3] + matrix.m[3][3];
	if (w != 0.0f) {
		result.x = (vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + vector.z * matrix.m[2][0] + matrix.m[3][0]) / w;
		result.y = (vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + vector.z * matrix.m[2][1] + matrix.m[3][1]) / w;
		result.z = (vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + vector.z * matrix.m[2][2] + matrix.m[3][2]) / w;
	}

	return result;
}

Vector3 Vector3::LookAt(const Vector3& eyePosition, const Vector3& targetPosition) {
	Vector3 diff = (targetPosition - eyePosition).Normalize();
    Vector3 result{};

    // yaw（ヨー, y軸回り）
    result.y = atan2f(diff.x, -diff.z);
    // pitch（ピッチ, x軸回り）
    result.x = atan2f(-diff.y, sqrtf(diff.x * diff.x + diff.z * diff.z));
    // roll（ロール, z軸回り
    result.z = 0.0f;

    return result;
}

Vector3 Vector3::Project(const Vector3& v1, const Vector3& v2) {
	Vector3 result{};
	if (powf(v2.Length(), 2) != 0.0f) {
		float t = Dot(v1, v2) / powf(v2.Length(), 2);
		result = v2 * t;
	}
	return result;
}

Vector3 Vector3::Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return { -vector.y,vector.x,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}

Vector3 Vector3::SphericalToCartesian(const Vector3& rtp) {
	Vector3 result{};
	result.x = rtp.x * std::sinf(rtp.y) * std::cosf(rtp.z);
	result.y = rtp.x * std::sinf(rtp.y) * std::sinf(rtp.z);
	result.z = rtp.x * std::cosf(rtp.y);
	return result;
}

Vector3 Vector3::CartesianToSpherical(const Vector3& xyz) {
	Vector3 result{};
	result.x = xyz.Length(); // r
	result.y = std::acosf(xyz.z / result.x); // theta
	result.z = std::atan2f(xyz.y, xyz.x); // phi
	return result;
}