#include "RotateToDir.h"

Vector3 RotateToDir::GetRotateToDir(const Vector3& rotate) {
	Matrix4x4 rotMat = Matrix4x4::MakeRotateXYZMatrix(rotate);
	//Z軸方向ベクトル
	Vector3 dir = { 0,0,1 };
	dir = Vector3::Transform(dir, rotMat);
	return dir.Normalize();
}
