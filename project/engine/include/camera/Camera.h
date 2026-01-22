#pragma once
#include "engine/include/core/Math/Transform.h"
#include "engine/include/core/Math/Matrix/Matrix4x4.h"

enum class CameraType {
	Perspective,
	Orthographic
};

class Camera final {
public:
	void Initialize();
	void Update();

	bool CheckVisible(const Matrix4x4& world) const;
	uint32_t GetBindEntityId() const;
	Matrix4x4 GetViewMatrix() const;
	Matrix4x4 GetPerspectiveMatrix() const;
	Matrix4x4 GetOrthographicMatrix() const;
	Matrix4x4 GetWorldViewProjectionMatrix(const Matrix4x4& worldMatrix ,CameraType type) const;
	Matrix4x4 GetWorldViewProjectionMatrixOrthographic(const Matrix4x4& worldMatrix) const;
	Matrix4x4 GetWorldMatrix() const;
	Vector3 GetPosition() const;

private:
	Matrix4x4 viewMatrix_;
	uint32_t bindEntityId_;

	Matrix4x4 projectionMatrix_;
	Matrix4x4 orthographicMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	Matrix4x4 viewOrthographicMatrix_;
};
