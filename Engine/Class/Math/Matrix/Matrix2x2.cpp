#include "Matrix2x2.h"

Matrix2x2 Matrix2x2::Transpose() const {
	Matrix2x2 result = *this;
	result.m[0][1] = this->m[1][0];
	result.m[1][0] = this->m[0][1];
	return result;
}

Matrix2x2 Matrix2x2::Transpose(const Matrix2x2& m){
	Matrix2x2 result = m;
	result.m[0][1] = m.m[1][0];
	result.m[1][0] = m.m[0][1];
	return result;
}

Matrix2x2 Matrix2x2::Add(const Matrix2x2& m1, const Matrix2x2& m2){
	Matrix2x2 result = {};
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {

			result.m[x][y] = m1.m[x][y] + m2.m[x][y];
		}
	}
	return result;
}

Matrix2x2 Matrix2x2::Subtract(const Matrix2x2& m1, const Matrix2x2& m2){
	Matrix2x2 result = {};
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {

			result.m[x][y] = m1.m[x][y] - m2.m[x][y];
		}
	}
	return result;
}

Matrix2x2 Matrix2x2::MakeIdentity2x2() {
	Matrix2x2 result = {};
	for (int y = 0; y < 2; y++) {
		for (int x = 0; x < 2; x++) {

			if (x == y) {
				result.m[y][x] = 1.0f;
			} else {
				result.m[y][x] = 0.0f;
			}
		}
	}
	return result;
}
