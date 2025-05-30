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
	/// <summary>
	/// WVPMatrixを作成します
	/// </summary>
	/// <param name="worldMatrix">オブジェクトの持つワールド行列</param>
	/// <param name="viewState">透視投影か平行投影か(CAMERA_VIEW_STATE_?????)</param>
	/// <returns>WVPMatrix</returns>
	Matrix4x4 MakeWorldViewProjectionMatrix(const Matrix4x4& worldMatrix, ViewState viewState);
	
private:
	Matrix4x4 viewMatrix_;
	Matrix4x4 perspectiveMatrix_;
	Matrix4x4 orthographicMatrix_;

public:
	Matrix4x4 affineMatrix_;
	Transform transform_;
	ViewPort viewport_;
	ScissorRect scissorrect_;	
};