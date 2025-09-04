#pragma once
#include "Base/EngineCore.h"

static inline const float kBlockSize = 1.0f;

class Block final {
public:
	Block() = default;
	~Block() = default;

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

	Transform& GetTransform();
	void SetIsDraw(bool isDraw);
	void SetColor(const Vector4& color);

private:
	EngineCore* engineCore_;
	Camera* camera_;

	bool isDraw_;
	std::unique_ptr<Model> model_;
};
