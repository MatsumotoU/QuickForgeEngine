#include "Quaternion.h"
#include <cmath>

#include "../Vector/Vector3.h"
#include "../Matrix/Matrix4x4.h"

Quaternion::Quaternion() {
	q = { 0.0f,0.0f,0.0f,0.0f };
}

Vector3 Quaternion::ImaginaryPart() const {
	Vector3 result{};
	result.x = this->q.x;
	result.y = this->q.y;
	result.z = this->q.z;
	return result;
}

float Quaternion::RealPart() const {
	return this->q.w;
}

Quaternion Quaternion::Conjugation() const {
	Quaternion result{};

	result.q.x = -this->q.x;
	result.q.y = -this->q.y;
	result.q.z = -this->q.z;
	result.q.w = this->q.w;

	return result;
}

float Quaternion::Norm() const {
	return std::sqrtf(std::powf(this->q.w, 2.0f) + std::powf(this->q.x, 2.0f) + std::powf(this->q.y, 2.0f) + std::powf(this->q.z, 2.0f));
}

Quaternion Quaternion::Normalize() const {
	Quaternion result{};
	float norm = this->Norm();
	if (norm != 0.0f) {
		result.q.x = this->q.x / norm;
		result.q.y = this->q.y / norm;
		result.q.z = this->q.z / norm;
		result.q.w = this->q.w / norm;
	}
	return result;
}

Quaternion Quaternion::Multiply(const Quaternion& lhs, const Quaternion& rhs) {
	Quaternion result{};

	Vector3 lhsImaginary = lhs.ImaginaryPart();
	Vector3 rhsImaginary = rhs.ImaginaryPart();

	result.q.x = Vector3::Cross(lhsImaginary, rhsImaginary).x + lhsImaginary.x * rhs.q.w + rhsImaginary.x * lhs.q.w;
	result.q.y = Vector3::Cross(lhsImaginary, rhsImaginary).y + lhsImaginary.y * rhs.q.w + rhsImaginary.y * lhs.q.w;
	result.q.z = Vector3::Cross(lhsImaginary, rhsImaginary).z + lhsImaginary.z * rhs.q.w + rhsImaginary.z * lhs.q.w;
	result.q.w = lhs.q.w * rhs.q.w - Vector3::Dot(lhsImaginary, rhsImaginary);

	return result;
}

Quaternion Quaternion::IndentityQuaternion() {
	Quaternion result{};
	result.q.w = 1.0f;
	return result;
}

Quaternion Quaternion::ConjugationQuaternion(const Quaternion& quaternion) {
	Quaternion result{};

	result.q.x = -quaternion.q.x;
	result.q.y = -quaternion.q.y;
	result.q.z = -quaternion.q.z;
	result.q.w = quaternion.q.w;

	return result;
}

float Quaternion::Norm(const Quaternion& quaternion) {
	return std::sqrtf(std::powf(quaternion.q.w, 2.0f) + std::powf(quaternion.q.x, 2.0f) + std::powf(quaternion.q.y, 2.0f) + std::powf(quaternion.q.z, 2.0f));
}

Quaternion Quaternion::Normalize(const Quaternion& quaternion) {
	Quaternion result{};
	float norm = quaternion.Norm();
	if (norm != 0.0f) {
		result.q.x = quaternion.q.x / norm;
		result.q.y = quaternion.q.y / norm;
		result.q.z = quaternion.q.z / norm;
		result.q.w = quaternion.q.w / norm;
	}
	return result;
}

Quaternion Quaternion::Inverse(const Quaternion& quaternion) {
	Quaternion result{};
	float norm = quaternion.Norm();
	Quaternion imaginaryQuaternion = quaternion.Conjugation();

	float powNorm = std::powf(norm, 2.0f);
	if (powNorm != 0.0f) {
		result.q.x = imaginaryQuaternion.q.x / powNorm;
		result.q.y = imaginaryQuaternion.q.y / powNorm;
		result.q.z = imaginaryQuaternion.q.z / powNorm;
		result.q.w = imaginaryQuaternion.q.w / powNorm;
	}

	return result;
}

Quaternion Quaternion::MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	Quaternion result{};
	Vector3 nAxis = axis.Normalize();

	result.q.x = nAxis.x * std::sinf(angle * 0.5f);
	result.q.y = nAxis.y * std::sinf(angle * 0.5f);
	result.q.z = nAxis.z * std::sinf(angle * 0.5f);
	result.q.w = std::cosf(angle * 0.5f);

	return result;
}

Vector3 Quaternion::RotateVector(const Vector3& vector, const Quaternion& quaternion) {
	// ベクトルをクオータニオンに変換（虚部のみ、実部0）
	Quaternion p;
	p.q.x = vector.x;
	p.q.y = vector.y;
	p.q.z = vector.z;
	p.q.w = 0.0f;

	// q * p
	Quaternion qp = Quaternion::Multiply(quaternion, p);
	// (q * p) * q^-1
	Quaternion qConj = quaternion.Conjugation();
	Quaternion rotated = Quaternion::Multiply(qp, qConj);

	// 回転後のベクトル（虚部）
	Vector3 result;
	result.x = rotated.q.x;
	result.y = rotated.q.y;
	result.z = rotated.q.z;
	return result;
}

Matrix4x4 Quaternion::MakeRotateMatrix(const Quaternion& quaternion) {
	Matrix4x4 result{};
	float x = quaternion.q.x;
	float y = quaternion.q.y;
	float z = quaternion.q.z;
	float w = quaternion.q.w;

	result.m[0][0] = std::powf(w, 2.0f) + std::powf(x, 2.0f) - std::powf(y, 2.0f) - std::powf(z, 2.0f);
	result.m[0][1] = 2.0f * (x * y + w * z);
	result.m[0][2] = 2.0f * (z * x - w * y);

	result.m[1][0] = 2.0f * (x * y - w * z);
	result.m[1][1] = std::powf(w, 2.0f) - std::powf(x, 2.0f) + std::powf(y, 2.0f) - std::powf(z, 2.0f);
	result.m[1][2] = 2.0f * (z * y + w * x);

	result.m[2][0] = 2.0f * (x * z + w * y);
	result.m[2][1] = 2.0f * (y * z - w * x);
	result.m[2][2] = std::powf(w, 2.0f) - std::powf(x, 2.0f) - std::powf(y, 2.0f) + std::powf(z, 2.0f);

	result.m[3][3] = 1.0f;

	return result;
}

Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t) {
	Quaternion result{};
	float dot = Vector3::Dot(q1.q.xyz(), q2.q.xyz());
	Quaternion tempQ1 = q1;
	Quaternion tempQ2 = q2;
	if (dot < 0.0f) {
		tempQ1.q = -q1.q;
		dot = -dot;
	}

	float theta = std::acosf(dot);
	float t0 = 0.0f;
	float t1 = 0.0f;
	if (std::sinf(theta) != 0.0f) {
		t0 = std::sinf((1.0f - t) * theta) / std::sinf(theta);
		t1 = std::sinf(t * theta) / std::sinf(theta);
	}

	result.q = tempQ1.q * t0 + tempQ2.q * t1;

	return result;
}
