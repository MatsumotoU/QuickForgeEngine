#include "Matrix3x3.h"
#include "../Vector/Vector2.h"
#include <assert.h>
#include <cmath>

Matrix3x3 Matrix3x3::operator+(const Matrix3x3& other) {
	Matrix3x3 returnMatrix;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {

			returnMatrix.m[x][y] = m[x][y] + other.m[x][y];
		}
	}
	return returnMatrix;
}

Matrix3x3 Matrix3x3::operator-(const Matrix3x3& other) {
	Matrix3x3 returnMatrix;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {

			returnMatrix.m[x][y] = m[x][y] - other.m[x][y];
		}
	}
	return returnMatrix;
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) {
	Matrix3x3 result = { 0 };
	result.m[0][0] = m[0][0] * other.m[0][0] + m[0][1] * other.m[1][0] + m[0][2] * other.m[2][0];
	result.m[0][1] = m[0][0] * other.m[0][1] + m[0][1] * other.m[1][1] + m[0][2] * other.m[2][1];
	result.m[0][2] = m[0][0] * other.m[0][2] + m[0][1] * other.m[1][2] + m[0][2] * other.m[2][2];

	result.m[1][0] = m[1][0] * other.m[0][0] + m[1][1] * other.m[1][0] + m[1][2] * other.m[2][0];
	result.m[1][1] = m[1][0] * other.m[0][1] + m[1][1] * other.m[1][1] + m[1][2] * other.m[2][1];
	result.m[1][2] = m[1][0] * other.m[0][2] + m[1][1] * other.m[1][2] + m[1][2] * other.m[2][2];

	result.m[2][0] = m[2][0] * other.m[0][0] + m[2][1] * other.m[1][0] + m[2][2] * other.m[2][0];
	result.m[2][1] = m[2][0] * other.m[0][1] + m[2][1] * other.m[1][1] + m[2][2] * other.m[2][1];
	result.m[2][2] = m[2][0] * other.m[0][2] + m[2][1] * other.m[1][2] + m[2][2] * other.m[2][2];
	return result;
}

Matrix3x3 Matrix3x3::Inverse() const {
	Matrix3x3 result = {};
	float i = this->m[0][0] * this->m[1][1] * this->m[2][2] +
		this->m[0][1] * this->m[1][2] * this->m[2][0] +
		this->m[0][2] * this->m[1][0] * this->m[2][1] -
		this->m[0][2] * this->m[1][1] * this->m[2][0] -
		this->m[0][1] * this->m[1][0] * this->m[2][2] -
		this->m[0][0] * this->m[1][2] * this->m[2][1];

	if (i != 0.0f) {
		result.m[0][0] = (this->m[1][1] * this->m[2][2] - this->m[1][2] * this->m[2][1]) / i;
		result.m[0][1] = -(this->m[0][1] * this->m[2][2] - this->m[0][2] * this->m[2][1]) / i;
		result.m[0][2] = (this->m[0][1] * this->m[1][2] - this->m[0][2] * this->m[1][1]) / i;
		result.m[1][0] = -(this->m[1][0] * this->m[2][2] - this->m[1][2] * this->m[2][0]) / i;
		result.m[1][1] = (this->m[0][0] * this->m[2][2] - this->m[0][2] * this->m[2][0]) / i;
		result.m[1][2] = -(this->m[0][0] * this->m[1][2] - this->m[0][2] * this->m[1][0]) / i;
		result.m[2][0] = (this->m[1][0] * this->m[2][1] - this->m[1][1] * this->m[2][0]) / i;
		result.m[2][1] = -(this->m[0][0] * this->m[2][1] - this->m[0][1] * this->m[2][0]) / i;
		result.m[2][2] = (this->m[0][0] * this->m[1][1] - this->m[0][1] * this->m[1][0]) / i;
	} else {
		assert(false);
	}

	return result;
}

Matrix3x3 Matrix3x3::Transpose() const {
	Matrix3x3 result = *this;
	result.m[0][1] = this->m[1][0];
	result.m[1][0] = this->m[0][1];
	result.m[0][2] = this->m[2][0];
	result.m[2][0] = this->m[0][2];
	result.m[1][2] = this->m[2][1];
	result.m[2][1] = this->m[1][2];
	return result;
}

Matrix3x3 Matrix3x3::Inverse(const Matrix3x3& m) {
	Matrix3x3 result = {};
	float i = m.m[0][0] * m.m[1][1] * m.m[2][2] +
		m.m[0][1] * m.m[1][2] * m.m[2][0] +
		m.m[0][2] * m.m[1][0] * m.m[2][1] -
		m.m[0][2] * m.m[1][1] * m.m[2][0] -
		m.m[0][1] * m.m[1][0] * m.m[2][2] -
		m.m[0][0] * m.m[1][2] * m.m[2][1];

	if (i != 0.0f) {
		result.m[0][0] = (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) / i;
		result.m[0][1] = -(m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) / i;
		result.m[0][2] = (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]) / i;
		result.m[1][0] = -(m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) / i;
		result.m[1][1] = (m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0]) / i;
		result.m[1][2] = -(m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0]) / i;
		result.m[2][0] = (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]) / i;
		result.m[2][1] = -(m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0]) / i;
		result.m[2][2] = (m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0]) / i;
	} else {
		assert(false);
	}

	return result;
}

Matrix3x3 Matrix3x3::Transpose(const Matrix3x3& m) {
	Matrix3x3 result = m;
	result.m[0][1] = m.m[1][0];
	result.m[1][0] = m.m[0][1];
	result.m[0][2] = m.m[2][0];
	result.m[2][0] = m.m[0][2];
	result.m[1][2] = m.m[2][1];
	result.m[2][1] = m.m[1][2];
	return result;
}

Matrix3x3 Matrix3x3::Add(const Matrix3x3& m1, const Matrix3x3& m2) {
	Matrix3x3 result = {};
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {

			result.m[x][y] = m1.m[x][y] + m2.m[x][y];
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::Subtract(const Matrix3x3& m1, const Matrix3x3& m2) {
	Matrix3x3 result = {};
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {

			result.m[x][y] = m1.m[x][y] - m2.m[x][y];
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::Multiply(const Matrix3x3& m1, const Matrix3x3& m2) {
	Matrix3x3 result = { 0 };
	result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0];
	result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1];
	result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2];

	result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0];
	result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1];
	result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2];

	result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0];
	result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1];
	result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2];
	return result;
}

Matrix3x3 Matrix3x3::MakeIdentity3x3() {
	Matrix3x3 result = {};
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {

			if (x == y) {
				result.m[y][x] = 1.0f;
			} else {
				result.m[y][x] = 0.0f;
			}
		}
	}
	return result;
}

Matrix3x3 Matrix3x3::MakeScaleMatrix(const Vector2& scale) const {
	Matrix3x3 result = { 0 };
	result.m[0][0] = scale.x;
	result.m[1][1] = scale.y;
	result.m[2][2] = 1.0f;
	return result;
}

Matrix3x3 Matrix3x3::MakeRotateMatrix(const float& theta) const {
	Matrix3x3 result = { 0 };
	result.m[0][0] = cos(theta);
	result.m[0][1] = sin(theta);
	result.m[1][0] = -sin(theta);
	result.m[1][1] = cos(theta);
	result.m[2][2] = 1.0f;
	return result;
}

Matrix3x3 Matrix3x3::MakeTranslateMatrix(const Vector2& translate) const {
	Matrix3x3 result = { 0 };
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;

	result.m[2][0] = translate.x;
	result.m[2][1] = translate.y;
	return result;
}

Matrix3x3 Matrix3x3::MakeAffineMatrix(const Vector2& scale, const float& rotate, const Vector2& translate) const {
	Matrix3x3 result = {};

	result.m[0][0] = scale.x * cos(rotate);
	result.m[0][1] = scale.x * sin(rotate);
	result.m[0][2] = 0.0f;

	result.m[1][0] = scale.y * -sin(rotate);
	result.m[1][1] = scale.y * cos(rotate);
	result.m[1][2] = 0.0f;

	result.m[2][0] = translate.x;
	result.m[2][1] = translate.y;
	result.m[2][2] = 1.0f;

	return result;
}

Matrix3x3 Matrix3x3::MakeOrthographicMatrix(const float& left, const float& top, const float& right, const float& bottom) const {
	Matrix3x3 result = {};

	if (left - right == 0.0f || top - bottom == 0.0f || right - left == 0.0f || bottom - top == 0.0f) {
		return result;
	}

	result.m[0][0] = 2.0f / (right - left);
	result.m[1][1] = 2.0f / (top - bottom);

	result.m[2][0] = (left + right) / (left - right);
	result.m[2][1] = (top + bottom) / (bottom - top);

	result.m[2][2] = 1.0f;
	return result;
}

Matrix3x3 Matrix3x3::MakeViewportMatrix(const float& left, const float& top, const float& width, const float& height) const {
	Matrix3x3 result = {};

	result.m[0][0] = width / 2.0f;
	result.m[1][1] = -height / 2.0f;
	result.m[2][0] = left + (width / 2.0f);
	result.m[2][1] = top + (height / 2.0f);
	result.m[2][2] = 1.0f;

	return result;
}

Matrix3x3 Matrix3x3::MakeWvpVpMatrix(const Matrix3x3& worldMatrix, const Matrix3x3& viewMatrix, const Matrix3x3& orthoMatrix, const Matrix3x3& viewportMatrix) const {
	Matrix3x3 result = Multiply(worldMatrix, viewMatrix);

	result = Multiply(result, orthoMatrix);
	result = Multiply(result, viewportMatrix);

	return result;
}
