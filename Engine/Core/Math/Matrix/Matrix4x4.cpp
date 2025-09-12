#include "Matrix4x4.h"
#include "Core/Math/Vector/Vector3.h"
#include "Core/Math/Transform.h"
#include <assert.h>

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& other) const {
	Matrix4x4 result = {};
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {

			result.m[y][x] = this->m[y][x] + other.m[y][x];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& other) const {
	Matrix4x4 result = {};
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {

			result.m[y][x] = this->m[y][x] - other.m[y][x];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
	Matrix4x4 result = {};

	for (int i = 0; i < 4; i++) {
		result.m[i][0] = this->m[i][0] * other.m[0][0] + this->m[i][1] * other.m[1][0] + this->m[i][2] * other.m[2][0] + this->m[i][3] * other.m[3][0];
		result.m[i][1] = this->m[i][0] * other.m[0][1] + this->m[i][1] * other.m[1][1] + this->m[i][2] * other.m[2][1] + this->m[i][3] * other.m[3][1];
		result.m[i][2] = this->m[i][0] * other.m[0][2] + this->m[i][1] * other.m[1][2] + this->m[i][2] * other.m[2][2] + this->m[i][3] * other.m[3][2];
		result.m[i][3] = this->m[i][0] * other.m[0][3] + this->m[i][1] * other.m[1][3] + this->m[i][2] * other.m[2][3] + this->m[i][3] * other.m[3][3];
	}

	return result;
}

Matrix4x4 Matrix4x4::Inverse() const {
	Matrix4x4 result = {};

	float i =
		+ this->m[0][0] * this->m[1][1] * this->m[2][2] * this->m[3][3]
		+ this->m[0][0] * this->m[1][2] * this->m[2][3] * this->m[3][1]
		+ this->m[0][0] * this->m[1][3] * this->m[2][1] * this->m[3][2]

		- this->m[0][0] * this->m[1][3] * this->m[2][2] * this->m[3][1]
		- this->m[0][0] * this->m[1][2] * this->m[2][1] * this->m[3][3]
		- this->m[0][0] * this->m[1][1] * this->m[2][3] * this->m[3][2]

		- this->m[0][1] * this->m[1][0] * this->m[2][2] * this->m[3][3]
		- this->m[0][2] * this->m[1][0] * this->m[2][3] * this->m[3][1]
		- this->m[0][3] * this->m[1][0] * this->m[2][1] * this->m[3][2]

		+ this->m[0][3] * this->m[1][0] * this->m[2][2] * this->m[3][1]
		+ this->m[0][2] * this->m[1][0] * this->m[2][1] * this->m[3][3]
		+ this->m[0][1] * this->m[1][0] * this->m[2][3] * this->m[3][2]

		+ this->m[0][1] * this->m[1][2] * this->m[2][0] * this->m[3][3]
		+ this->m[0][2] * this->m[1][3] * this->m[2][0] * this->m[3][1]
		+ this->m[0][3] * this->m[1][1] * this->m[2][0] * this->m[3][2]

		- this->m[0][3] * this->m[1][2] * this->m[2][0] * this->m[3][1]
		- this->m[0][2] * this->m[1][1] * this->m[2][0] * this->m[3][3]
		- this->m[0][1] * this->m[1][3] * this->m[2][0] * this->m[3][2]

		- this->m[0][1] * this->m[1][2] * this->m[2][3] * this->m[3][0]
		- this->m[0][2] * this->m[1][3] * this->m[2][1] * this->m[3][0]
		- this->m[0][3] * this->m[1][1] * this->m[2][2] * this->m[3][0]

		+ this->m[0][3] * this->m[1][2] * this->m[2][1] * this->m[3][0]
		+ this->m[0][2] * this->m[1][1] * this->m[2][3] * this->m[3][0]
		+ this->m[0][1] * this->m[1][3] * this->m[2][2] * this->m[3][0];

	// 零除算回避用
	if (i == 0.0f) {
		assert(false);
	}

	result.m[0][0] = (
		+ this->m[1][1] * this->m[2][2] * this->m[3][3]
		+ this->m[1][2] * this->m[2][3] * this->m[3][1]
		+ this->m[1][3] * this->m[2][1] * this->m[3][2]
		- this->m[1][3] * this->m[2][2] * this->m[3][1]
		- this->m[1][2] * this->m[2][1] * this->m[3][3]
		- this->m[1][1] * this->m[2][3] * this->m[3][2]
		) / i;
	result.m[0][1] = (
		- this->m[0][1] * this->m[2][2] * this->m[3][3]
		- this->m[0][2] * this->m[2][3] * this->m[3][1]
		- this->m[0][3] * this->m[2][1] * this->m[3][2]
		+ this->m[0][3] * this->m[2][2] * this->m[3][1]
		+ this->m[0][2] * this->m[2][1] * this->m[3][3]
		+ this->m[0][1] * this->m[2][3] * this->m[3][2]
		) / i;
	result.m[0][2] = (
		+ this->m[0][1] * this->m[1][2] * this->m[3][3]
		+ this->m[0][2] * this->m[1][3] * this->m[3][1]
		+ this->m[0][3] * this->m[1][1] * this->m[3][2]
		- this->m[0][3] * this->m[1][2] * this->m[3][1]
		- this->m[0][2] * this->m[1][1] * this->m[3][3]
		- this->m[0][1] * this->m[1][3] * this->m[3][2]
		) / i;
	result.m[0][3] = (
		- this->m[0][1] * this->m[1][2] * this->m[2][3]
		- this->m[0][2] * this->m[1][3] * this->m[2][1]
		- this->m[0][3] * this->m[1][1] * this->m[2][2]
		+ this->m[0][3] * this->m[1][2] * this->m[2][1]
		+ this->m[0][2] * this->m[1][1] * this->m[2][3]
		+ this->m[0][1] * this->m[1][3] * this->m[2][2]
		) / i;

	result.m[1][0] = (
		- this->m[1][0] * this->m[2][2] * this->m[3][3]
		- this->m[1][2] * this->m[2][3] * this->m[3][0]
		- this->m[1][3] * this->m[2][0] * this->m[3][2]
		+ this->m[1][3] * this->m[2][2] * this->m[3][0]
		+ this->m[1][2] * this->m[2][0] * this->m[3][3]
		+ this->m[1][0] * this->m[2][3] * this->m[3][2]
		) / i;
	result.m[1][1] = (
		+ this->m[0][0] * this->m[2][2] * this->m[3][3]
		+ this->m[0][2] * this->m[2][3] * this->m[3][0]
		+ this->m[0][3] * this->m[2][0] * this->m[3][2]
		- this->m[0][3] * this->m[2][2] * this->m[3][0]
		- this->m[0][2] * this->m[2][0] * this->m[3][3]
		- this->m[0][0] * this->m[2][3] * this->m[3][2]
		) / i;
	result.m[1][2] = (
		- this->m[0][0] * this->m[1][2] * this->m[3][3]
		- this->m[0][2] * this->m[1][3] * this->m[3][0]
		- this->m[0][3] * this->m[1][0] * this->m[3][2]
		+ this->m[0][3] * this->m[1][2] * this->m[3][0]
		+ this->m[0][2] * this->m[1][0] * this->m[3][3]
		+ this->m[0][0] * this->m[1][3] * this->m[3][2]
		) / i;
	result.m[1][3] = (
		+ this->m[0][0] * this->m[1][2] * this->m[2][3]
		+ this->m[0][2] * this->m[1][3] * this->m[2][0]
		+ this->m[0][3] * this->m[1][0] * this->m[2][2]
		- this->m[0][3] * this->m[1][2] * this->m[2][0]
		- this->m[0][2] * this->m[1][0] * this->m[2][3]
		- this->m[0][0] * this->m[1][3] * this->m[2][2]
		) / i;

	result.m[2][0] = (
		+ this->m[1][0] * this->m[2][1] * this->m[3][3]
		+ this->m[1][1] * this->m[2][3] * this->m[3][0]
		+ this->m[1][3] * this->m[2][0] * this->m[3][1]
		- this->m[1][3] * this->m[2][1] * this->m[3][0]
		- this->m[1][1] * this->m[2][0] * this->m[3][3]
		- this->m[1][0] * this->m[2][3] * this->m[3][1]
		) / i;
	result.m[2][1] = (
		- this->m[0][0] * this->m[2][1] * this->m[3][3]
		- this->m[0][1] * this->m[2][3] * this->m[3][0]
		- this->m[0][3] * this->m[2][0] * this->m[3][1]
		+ this->m[0][3] * this->m[2][1] * this->m[3][0]
		+ this->m[0][1] * this->m[2][0] * this->m[3][3]
		+ this->m[0][0] * this->m[2][3] * this->m[3][1]
		) / i;
	result.m[2][2] = (
		+ this->m[0][0] * this->m[1][1] * this->m[3][3]
		+ this->m[0][1] * this->m[1][3] * this->m[3][0]
		+ this->m[0][3] * this->m[1][0] * this->m[3][1]
		- this->m[0][3] * this->m[1][1] * this->m[3][0]
		- this->m[0][1] * this->m[1][0] * this->m[3][3]
		- this->m[0][0] * this->m[1][3] * this->m[3][1]
		) / i;
	result.m[2][3] = (
		- this->m[0][0] * this->m[1][1] * this->m[2][3]
		- this->m[0][1] * this->m[1][3] * this->m[2][0]
		- this->m[0][3] * this->m[1][0] * this->m[2][1]
		+ this->m[0][3] * this->m[1][1] * this->m[2][0]
		+ this->m[0][1] * this->m[1][0] * this->m[2][3]
		+ this->m[0][0] * this->m[1][3] * this->m[2][1]
		) / i;

	result.m[3][0] = (
		- this->m[1][0] * this->m[2][1] * this->m[3][2]
		- this->m[1][1] * this->m[2][2] * this->m[3][0]
		- this->m[1][2] * this->m[2][0] * this->m[3][1]
		+ this->m[1][2] * this->m[2][1] * this->m[3][0]
		+ this->m[1][1] * this->m[2][0] * this->m[3][2]
		+ this->m[1][0] * this->m[2][2] * this->m[3][1]
		) / i;
	result.m[3][1] = (
		+ this->m[0][0] * this->m[2][1] * this->m[3][2]
		+ this->m[0][1] * this->m[2][2] * this->m[3][0]
		+ this->m[0][2] * this->m[2][0] * this->m[3][1]
		- this->m[0][2] * this->m[2][1] * this->m[3][0]
		- this->m[0][1] * this->m[2][0] * this->m[3][2]
		- this->m[0][0] * this->m[2][2] * this->m[3][1]
		) / i;
	result.m[3][2] = (
		- this->m[0][0] * this->m[1][1] * this->m[3][2]
		- this->m[0][1] * this->m[1][2] * this->m[3][0]
		- this->m[0][2] * this->m[1][0] * this->m[3][1]
		+ this->m[0][2] * this->m[1][1] * this->m[3][0]
		+ this->m[0][1] * this->m[1][0] * this->m[3][2]
		+ this->m[0][0] * this->m[1][2] * this->m[3][1]
		) / i;
	result.m[3][3] = (
		+ this->m[0][0] * this->m[1][1] * this->m[2][2]
		+ this->m[0][1] * this->m[1][2] * this->m[2][0]
		+ this->m[0][2] * this->m[1][0] * this->m[2][1]
		- this->m[0][2] * this->m[1][1] * this->m[2][0]
		- this->m[0][1] * this->m[1][0] * this->m[2][2]
		- this->m[0][0] * this->m[1][2] * this->m[2][1]
		) / i;

	return result;
}

Matrix4x4 Matrix4x4::Transpose() const {
	Matrix4x4 result = {};
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {

			result.m[y][x] = this->m[x][y];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::Inverse(const Matrix4x4& m) {
	Matrix4x4 result = {};

	float i =
		+m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]

		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]

		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]

		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]

		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]

		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]

		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]

		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	// 零除算回避用
	if (i == 0.0f) {
		return Matrix4x4::MakeIndentity4x4();
	}

	result.m[0][0] = (
		+m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[1][1] * m.m[2][3] * m.m[3][2]
		) / i;
	result.m[0][1] = (
		-m.m[0][1] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[2][3] * m.m[3][2]
		) / i;
	result.m[0][2] = (
		+m.m[0][1] * m.m[1][2] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[3][2]
		) / i;
	result.m[0][3] = (
		-m.m[0][1] * m.m[1][2] * m.m[2][3]
		- m.m[0][2] * m.m[1][3] * m.m[2][1]
		- m.m[0][3] * m.m[1][1] * m.m[2][2]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2]
		) / i;

	result.m[1][0] = (
		-m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[1][3] * m.m[2][0] * m.m[3][2]
		+ m.m[1][3] * m.m[2][2] * m.m[3][0]
		+ m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[1][0] * m.m[2][3] * m.m[3][2]
		) / i;
	result.m[1][1] = (
		+m.m[0][0] * m.m[2][2] * m.m[3][3]
		+ m.m[0][2] * m.m[2][3] * m.m[3][0]
		+ m.m[0][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[2][2] * m.m[3][0]
		- m.m[0][2] * m.m[2][0] * m.m[3][3]
		- m.m[0][0] * m.m[2][3] * m.m[3][2]
		) / i;
	result.m[1][2] = (
		-m.m[0][0] * m.m[1][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][3] * m.m[3][0]
		- m.m[0][3] * m.m[1][0] * m.m[3][2]
		+ m.m[0][3] * m.m[1][2] * m.m[3][0]
		+ m.m[0][2] * m.m[1][0] * m.m[3][3]
		+ m.m[0][0] * m.m[1][3] * m.m[3][2]
		) / i;
	result.m[1][3] = (
		+m.m[0][0] * m.m[1][2] * m.m[2][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0]
		- m.m[0][2] * m.m[1][0] * m.m[2][3]
		- m.m[0][0] * m.m[1][3] * m.m[2][2]
		) / i;

	result.m[2][0] = (
		+m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[1][3] * m.m[2][0] * m.m[3][1]
		- m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[1][0] * m.m[2][3] * m.m[3][1]
		) / i;
	result.m[2][1] = (
		-m.m[0][0] * m.m[2][1] * m.m[3][3]
		- m.m[0][1] * m.m[2][3] * m.m[3][0]
		- m.m[0][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[2][1] * m.m[3][0]
		+ m.m[0][1] * m.m[2][0] * m.m[3][3]
		+ m.m[0][0] * m.m[2][3] * m.m[3][1]
		) / i;
	result.m[2][2] = (
		+m.m[0][0] * m.m[1][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][3] * m.m[3][0]
		+ m.m[0][3] * m.m[1][0] * m.m[3][1]
		- m.m[0][3] * m.m[1][1] * m.m[3][0]
		- m.m[0][1] * m.m[1][0] * m.m[3][3]
		- m.m[0][0] * m.m[1][3] * m.m[3][1]
		) / i;
	result.m[2][3] = (
		-m.m[0][0] * m.m[1][1] * m.m[2][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0]
		- m.m[0][3] * m.m[1][0] * m.m[2][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1]
		) / i;

	result.m[3][0] = (
		-m.m[1][0] * m.m[2][1] * m.m[3][2]
		- m.m[1][1] * m.m[2][2] * m.m[3][0]
		- m.m[1][2] * m.m[2][0] * m.m[3][1]
		+ m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[1][1] * m.m[2][0] * m.m[3][2]
		+ m.m[1][0] * m.m[2][2] * m.m[3][1]
		) / i;
	result.m[3][1] = (
		+m.m[0][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[2][1] * m.m[3][0]
		- m.m[0][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][0] * m.m[2][2] * m.m[3][1]
		) / i;
	result.m[3][2] = (
		-m.m[0][0] * m.m[1][1] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[3][0]
		- m.m[0][2] * m.m[1][0] * m.m[3][1]
		+ m.m[0][2] * m.m[1][1] * m.m[3][0]
		+ m.m[0][1] * m.m[1][0] * m.m[3][2]
		+ m.m[0][0] * m.m[1][2] * m.m[3][1]
		) / i;
	result.m[3][3] = (
		+m.m[0][0] * m.m[1][1] * m.m[2][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0]
		- m.m[0][1] * m.m[1][0] * m.m[2][2]
		- m.m[0][0] * m.m[1][2] * m.m[2][1]
		) / i;

	return result;
}

Matrix4x4 Matrix4x4::Transpose(const Matrix4x4& m) {
	Matrix4x4 result = {};
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {

			result.m[y][x] = m.m[x][y];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {

			result.m[y][x] = m1.m[y][x] + m2.m[y][x];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result = {};
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {

			result.m[y][x] = m1.m[y][x] - m2.m[y][x];
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::Multiply(const Matrix4x4& m1, const Matrix4x4& m2)  {
	Matrix4x4 result = {};

	for (int i = 0; i < 4; i++) {
		result.m[i][0] = m1.m[i][0] * m2.m[0][0] + m1.m[i][1] * m2.m[1][0] + m1.m[i][2] * m2.m[2][0] + m1.m[i][3] * m2.m[3][0];
		result.m[i][1] = m1.m[i][0] * m2.m[0][1] + m1.m[i][1] * m2.m[1][1] + m1.m[i][2] * m2.m[2][1] + m1.m[i][3] * m2.m[3][1];
		result.m[i][2] = m1.m[i][0] * m2.m[0][2] + m1.m[i][1] * m2.m[1][2] + m1.m[i][2] * m2.m[2][2] + m1.m[i][3] * m2.m[3][2];
		result.m[i][3] = m1.m[i][0] * m2.m[0][3] + m1.m[i][1] * m2.m[1][3] + m1.m[i][2] * m2.m[2][3] + m1.m[i][3] * m2.m[3][3];
	}

	return result;
}

Matrix4x4 Matrix4x4::MakeIndentity4x4() {
	Matrix4x4 result = {};
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {

			if (x == y) {
				result.m[y][x] = 1.0f;
			} else {
				result.m[y][x] = 0.0f;
			}
		}
	}
	return result;
}

Matrix4x4 Matrix4x4::MakeScaleMatrix(const Vector3& translate) {
	Matrix4x4 result = MakeIndentity4x4();

	result.m[0][0] = translate.x;
	result.m[1][1] = translate.y;
	result.m[2][2] = translate.z;

	return result;
}

Matrix4x4 Matrix4x4::MakeTranslateMatrix(const Vector3& translate) {
	Matrix4x4 result = MakeIndentity4x4();

	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;

	return result;
}

Matrix4x4 Matrix4x4::MakeRotateXMatrix(const float& radian) {
	Matrix4x4 result = MakeIndentity4x4();

	result.m[1][1] = std::cos(radian);
	result.m[2][1] = -std::sin(radian);
	result.m[1][2] = std::sin(radian);
	result.m[2][2] = std::cos(radian);

	return result;
}

Matrix4x4 Matrix4x4::MakeRotateYMatrix(const float& radian) {
	Matrix4x4 result = MakeIndentity4x4();

	result.m[0][0] = std::cos(radian);
	result.m[2][0] = std::sin(radian);
	result.m[0][2] = -std::sin(radian);
	result.m[2][2] = std::cos(radian);

	return result;
}

Matrix4x4 Matrix4x4::MakeRotateZMatrix(const float& radian) {
	Matrix4x4 result = MakeIndentity4x4();

	result.m[0][0] = std::cos(radian);
	result.m[1][0] = -std::sin(radian);
	result.m[0][1] = std::sin(radian);
	result.m[1][1] = std::cos(radian);

	return result;
}

Matrix4x4 Matrix4x4::MakeRotateXYZMatrix(const Vector3& rotate) {
	return Multiply(MakeRotateXMatrix(rotate.x),Multiply(MakeRotateYMatrix(rotate.y),MakeRotateZMatrix(rotate.z)));
}

Matrix4x4 Matrix4x4::MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result = MakeIndentity4x4();

	result = Multiply(
		MakeScaleMatrix(scale),
		Multiply(MakeRotateXYZMatrix(rotate),
		MakeTranslateMatrix(translate)));

	return result;
}

Matrix4x4 Matrix4x4::MakeAffineMatrix(const Transform& transform) {
	Matrix4x4 result = MakeIndentity4x4();

	result = Multiply(
		MakeScaleMatrix(transform.scale),
		Multiply(MakeRotateXYZMatrix(transform.rotate),
			MakeTranslateMatrix(transform.translate)));

	return result;
}

Matrix4x4 Matrix4x4::MakeRotateAxisAngle(const Vector3& axis, float angle) {
	Matrix4x4 result{};
	Vector3 axisNormal = axis.Normalize();

	result.m[0][0] = std::powf(axisNormal.x, 2.0f) * (1.0f - std::cosf(angle)) + std::cosf(angle);
	result.m[0][1] = (axisNormal.x * axisNormal.y) * (1.0f - std::cosf(angle)) + axisNormal.z * std::sinf(angle);
	result.m[0][2] = (axisNormal.x * axisNormal.z) * (1.0f - std::cosf(angle)) - axisNormal.y * std::sinf(angle);

	result.m[1][0] = (axisNormal.x * axisNormal.y) * (1.0f - std::cosf(angle)) - axisNormal.z * std::sinf(angle);
	result.m[1][1] = std::powf(axisNormal.y, 2.0f) * (1.0f - std::cosf(angle)) + std::cosf(angle);
	result.m[1][2] = (axisNormal.y * axisNormal.z) * (1.0f - std::cosf(angle)) + axisNormal.x * std::sinf(angle);

	result.m[2][0] = (axisNormal.x * axisNormal.z) * (1.0f - std::cosf(angle)) + axisNormal.y * std::sinf(angle);
	result.m[2][1] = (axisNormal.y * axisNormal.z) * (1.0f - std::cosf(angle)) - axisNormal.x * std::sinf(angle);
	result.m[2][2] = std::powf(axisNormal.z, 2.0f) * (1.0f - std::cosf(angle)) + std::cosf(angle);

	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::DirectionToDirection(const Vector3& from, const Vector3& to) {
	Matrix4x4 result{};

	Vector3 u = from.Normalize();
	Vector3 v = to.Normalize();

	float dot = Vector3::Dot(u, v);

	// ほぼ同じ方向の場合
	if (std::abs(dot - 1.0f) < 1e-6f) {
		return Matrix4x4::MakeIndentity4x4();
	}

	// ほぼ逆方向の場合（180度回転、任意の垂直軸で回転）
	if (std::abs(dot + 1.0f) < 1e-6f) {
		// uに垂直な任意軸を選ぶ
		Vector3 axis = Vector3::Perpendicular(u).Normalize();
		return Matrix4x4::MakeRotateAxisAngle(axis, static_cast<float>(M_PI));
	}

	// 通常ケース
	Vector3 axis = Vector3::Cross(u, v).Normalize();
	float angle = std::acosf(dot);

	result = Matrix4x4::MakeRotateAxisAngle(axis, angle);
	return result;
}

Matrix4x4 Matrix4x4::MakePerspectiveFovMatrix(const float& fovY, const float& aspectRatio, const float& nearClip, const float& farClip) {
	if (aspectRatio == 0.0f || nearClip - farClip == 0.0f) {
		assert(false);
	}
	
	Matrix4x4 result = {};

	result.m[0][0] = (1.0f / aspectRatio) * (1.0f / std::tan(fovY / 2.0f));
	result.m[1][1] = 1.0f / std::tan( fovY / 2.0f);
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	result.m[2][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom, const float& nearClip, const float& farClip) {
	if (right - left == 0.0f || top - bottom == 0.0f ||
		farClip - nearClip == 0.0f || left - right == 0.0f ||
		bottom - top == 0.0f || nearClip - farClip == 0.0f) {

		assert(false);
	}
	
	Matrix4x4 result = {};

	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);
	result.m[2][2] = 1.0f / (farClip - nearClip);
	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;

	return result;
}

Matrix4x4 Matrix4x4::MakeViewportMatrix(const float& left, const float& top, const float& width, const float& heigh, const float& minDepth, const float& maxDepth) {
	Matrix4x4 result = {};

	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -heigh / 2.0f;
	result.m[2][2] = maxDepth - minDepth;
	result.m[3][0] = left + (width / 2.0f);
	result.m[3][1] = top + (heigh / 2.0f);
	result.m[3][2] = minDepth;
	result.m[3][3] = 1.0f;

	return result;
}
