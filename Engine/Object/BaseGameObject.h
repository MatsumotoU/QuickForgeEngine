#pragma once
#include <string>

class EngineCore;
class Camera;

#include "Math/Vector/Vector3.h"
#include "Math/Matrix/Matrix4x4.h"
#include "Math/Transform.h"
#include "Utility/SimpleJson.h"
#ifdef _DEBUG
#include "Base/DirectX/ImGuiManager.h"
#endif // _DEBUG


class BaseGameObject {
public:
	BaseGameObject() = delete;
	BaseGameObject(EngineCore* engineCore, Camera* camera);
	virtual ~BaseGameObject() = default;

public:
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual nlohmann::json Serialize() const = 0;
#ifdef _DEBUG
	virtual void DrawImGui() = 0;
	void DrawGizmo(const ImGuizmo::OPERATION& op, const ImVec2& imageScreenPos, const ImVec2& imageSize);
	void DecomposeMatrix(const float* matrix, Vector3& scale, Vector3& rotation, Vector3& translation);
#endif // _DEBUG

public:
	Vector3 GetWorldPosition();
	std::string GetName() const {
		return name_;
	}
	void SetName(const std::string& name) {
		name_ = name;
	}

public:
	Vector3 localPos_;
	Transform transform_;
	Vector3 worldPos_;
	Matrix4x4 worldMatrix_;

protected:
	std::string name_;
	EngineCore* engineCore_;
	Camera* camera_;
};