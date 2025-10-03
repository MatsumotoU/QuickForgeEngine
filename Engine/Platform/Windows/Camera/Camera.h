#pragma once
#include "Core/Math/Transform.h"
#include "Core/Math/Matrix/Matrix4x4.h"

enum class CameraType {
	Perspective,
	Orthographic
};

class Camera final {
public:
	void Initialize();
	void Update();

	Matrix4x4 GetViewMatrix() const;
	Matrix4x4 GetPerspectiveMatrix() const;
	Matrix4x4 GetOrthographicMatrix() const;
	Matrix4x4 GetWorldViewProjectionMatrix(const Matrix4x4& worldMatrix) const;
	Matrix4x4 GetWorldViewProjectionMatrixOrthographic(const Matrix4x4& worldMatrix) const;
	Matrix4x4 GetWorldMatrix() const;

	Transform transform_;
	float fovY_;
	float nearZ_;
	float farZ_;

#ifdef _DEBUG
	void DrawImgui();
#endif // _DEBUG

private:
	Matrix4x4 viewMatrix_;
	CameraType cameraType;
};