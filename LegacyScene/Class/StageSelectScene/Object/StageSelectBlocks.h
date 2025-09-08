#pragma once
#include <vector>

struct Transform;
class Vector4;
class Particle;
class Camera;

class BaseStageSelectBlocks {
public:
	BaseStageSelectBlocks(Particle *particle, Camera *camera)
		: particle_(particle)
		, camera_(camera) {
	}

	~BaseStageSelectBlocks() = default;
	void Initialize();
	void Draw();

protected:
	std::vector<Transform> transforms_;
	std::vector<Vector4> colors_;
	Particle *particle_ = nullptr;
	Camera *camera_ = nullptr;
};