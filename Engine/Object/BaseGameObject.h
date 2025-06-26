#pragma once
#include "Base/EngineCore.h"

class BaseGameObject {
public:
	BaseGameObject();

public:
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

public:
	Vector3 GetWorldPos();

public:
	Vector3 localPos_;
	Transform transform_;
	Vector3 worldPos_;
	Matrix4x4 worldMatrix_;
};