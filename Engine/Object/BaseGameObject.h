#pragma once
#include <string>

class EngineCore;
class Camera;

#include "Math/Vector/Vector3.h"
#include "Math/Matrix/Matrix4x4.h"
#include "Math/Transform.h"

class BaseGameObject {
public:
	BaseGameObject() = delete;
	BaseGameObject(EngineCore* engineCore);
	virtual ~BaseGameObject() = default;

public:
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw(Camera* camera) = 0;

#ifdef _DEBUG
	virtual void DrawImGui() = 0;
#endif // _DEBUG

public:
	Vector3 GetWorldPos();
	std::string GetName() const {
		return name_;
	}

public:
	Vector3 localPos_;
	Transform transform_;
	Vector3 worldPos_;
	Matrix4x4 worldMatrix_;

protected:
	std::string name_;
	EngineCore* engineCore_;
};