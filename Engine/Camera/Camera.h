#pragma once
#include "../Math/Transform.h"
#include "../Math/Matrix/Matrix4x4.h"

#include "../Base/DirectX/ScissorRect.h"
#include "../Base/DirectX/Viewport.h"

class WinApp;

enum ViewState
{
	CAMERA_VIEW_STATE_PERSPECTIVE,
	CAMERA_VIEW_STATE_ORTHOGRAPHIC,
};

class Camera {
public:
	Camera();
	~Camera();

public:
	void Initialize(WinApp* win);
	void Update();

public:
	[[nodiscard]] Vector3 GetScreenPos(const Vector3& localPos, const Matrix4x4& worldMatrix);
	[[nodiscard]] Vector3 GetScreenToWorldPos(const Vector3& screen);
	[[nodiscard]] Vector3 GetWorldToScreenPos(const Vector3& world);

public:
	/// <summary>
	/// WVPMatrixを作成します
	/// </summary>
	/// <param name="worldMatrix">オブジェクトの持つワールド行列</param>
	/// <param name="viewState">透視投影か平行投影か(CAMERA_VIEW_STATE_?????)</param>
	/// <returns>WVPMatrix</returns>
	Matrix4x4 MakeWorldViewProjectionMatrix(const Matrix4x4& worldMatrix, ViewState viewState);
	Matrix4x4 GetViewPortMatrix();

public:
	Matrix4x4 GetWorldMatrix() const;
	Matrix4x4 GetRotateMatrix() const;
	
public:
	Matrix4x4 viewMatrix_;
	Matrix4x4 perspectiveMatrix_;
	Matrix4x4 orthographicMatrix_;

public:
	// TODO: affineMatrixではなくworldMatrix
	Matrix4x4 affineMatrix_;
	Transform transform_;
	ViewPort viewport_;
	ScissorRect scissorrect_;	

private:
	WinApp* win_;
};