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

#include "Colliders/Collider.h"

class BaseGameObject : public Collider{
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
	virtual void DrawImGui();
	virtual void DrawGizmo(const ImGuizmo::OPERATION& op, const ImGuizmo::MODE& mode, const ImVec2& imageScreenPos, const ImVec2& imageSize);
	void DecomposeMatrix(const float* matrix, Vector3& scale, Vector3& rotation, Vector3& translation);
#endif // _DEBUG

public:
	Vector3 GetWorldPosition() override;
	std::string GetName() const {
		return name_;
	}
	std::string GetAttachedScriptName() const {
		return attachedScriptName;
	}
	std::string GetTag() const {
		return tag_;
	}
	void SetName(const std::string& name) {
		name_ = name;
	}
	void SetScriptName(const std::string& scriptName) {
		attachedScriptName = scriptName;
	}
	void SetTag(const std::string& tag) {
		tag_ = tag;
	}

public:
	Vector3 localPos_;
	Transform transform_;
	Vector3 worldPos_;
	Matrix4x4 worldMatrix_;
	float timeCounter_;

protected:
	std::string name_;
	std::string tag_;
	EngineCore* engineCore_;
	Camera* camera_;

	std::string attachedScriptName;
};