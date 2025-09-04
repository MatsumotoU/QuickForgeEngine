#pragma once
#include "Base/EngineCore.h"

class Block final {
public:
	Block() = default;
	~Block() = default;

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
};
