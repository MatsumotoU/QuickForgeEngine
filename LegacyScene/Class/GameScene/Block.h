#pragma once
#include "Base/EngineCore.h"

static inline const float kBlockSize = 1.0f;

enum class BlockType {
	Dirt,
	Grass,
	Stone,
};

class Block final {
public:
	Block() = default;
	~Block() = default;

	void Initialize(EngineCore* engineCore, Camera* camera,BlockType type);
	void Update();
	void Draw();

	void BuildUpSpawn();

	Transform& GetTransform();
	void SetIsDraw(bool isDraw);
	void SetColor(const Vector4& color);
	void SetType(BlockType type);
	void SetTransform(const Transform& transform);

private:
	EngineCore* engineCore_;
	Camera* camera_;

	bool isDraw_;
	std::unique_ptr<Model> dirtModel_;
	std::unique_ptr<Model> grassModel_;
	std::unique_ptr<Model> StoneModel_;

	BlockType type_;
};
