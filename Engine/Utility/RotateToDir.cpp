#include "RotateToDir.h"

Vector3 RotateToDir::GetRotateToDir(const Vector3& rotate) {
	Matrix4x4 rotMat = Matrix4x4::MakeRotateXYZMatrix(rotate);
	//Z軸方向ベクトル
	Vector3 dir = { 0,0,1 };
	dir = Vector3::Transform(dir, rotMat);
	return dir.Normalize();
}

Vector3 RotateToDir::GetDirToRotate(const Vector3& dir) {
	//dirを正規化
	Vector3 nDir = dir.Normalize();
	//X軸回転
	float x = asinf(-nDir.y);
	//Y軸回転
	if (cosf(x) == 0) {
		//X軸回転が90度のときY軸回転は0で良い
		return Vector3(x, 0, 0);
	}
	float y = asinf(nDir.x / cosf(x));
	return Vector3(x, y, 0);
}
