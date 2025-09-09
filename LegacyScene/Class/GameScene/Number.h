#pragma once
#include "Base/EngineCore.h"
#include <array>

class Number final{
public:
	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

	void SetNumber(uint32_t number);

	Transform transform_;
	Vector4 color_;

private:
	EngineCore* engineCore_;

	uint32_t number_;
	std::array<std::unique_ptr<Model>, 10> numberModel_;

	float timer_;
};