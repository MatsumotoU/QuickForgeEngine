#pragma once
#include <vector>
#include <string>
#include "Graphic/ShaderBuffer/VertexBuffer.h"

class SpriteManager final {
public:
	SpriteManager() = default;
	~SpriteManager() = default;
	void Initialize();
	void ResizeSprite(uint32_t handle, float width, float height);
	uint32_t CreateVertexBuffer(float width, float height);
	VertexBuffer<VertexData>* GetVertexBuffer(uint32_t handle);
	Vector2 GetSpriteSize(uint32_t handle);
	void Finalize();

private:
	std::vector<VertexBuffer<VertexData>> spriteVertexBuffers_;
};