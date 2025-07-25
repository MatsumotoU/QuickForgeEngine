#pragma once
#include "../../../../Engine/Base/EngineCore.h"

static inline const float kLimitMinY = 6.0f;
static inline const float kLimitMaxY = 100.0f;

class CameraContoroller {
public:
	void Initialize(Camera* camera);
	void Update();

public:
	void SetTargetVelocity(Vector3* targetVelocity);
	void SetTargetPosition(Vector3* targetPosition);

	float kLimitMaxX;
	float kLimitMinX;

private:

	Vector3* targetVelocity_;
	Vector3* targetPosition_;
	Camera* camera_;

	Vector3 cameraPosition_;
};