#pragma once
#include "Base/EngineCore.h"

class LockOn {
public:
	void Initialize();

public:
	void ResetTargets();
	void SetReticlePosition(const Vector3& reticle);
	void AddTargetPosition(const Vector3& target);

public:
	Vector3 GetLockPosition(Camera* camera);

private:
	Vector3 reticlePosition_;
	std::vector<Vector3> targetPostions_;
};