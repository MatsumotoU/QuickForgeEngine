#pragma once
#include "Base/EngineCore.h"

static inline const float kBlockSize = 1.0f;

enum class BlockType {
	None,
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

	void BuildUpSpawn();

	bool GetIsDraw() const { return isDraw_; }
	const BlockType& GetType() const;
	void SetIsDraw(bool isDraw);
	void SetColor(const Vector4& color);
	void SetType(BlockType type);
	void SetTransform(const Transform& transform);

	Transform transform_;
	Vector4 color_;

private:
	EngineCore* engineCore_;
	Camera* camera_;

	bool isDraw_;

	BlockType type_;
};
