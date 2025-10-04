#include "SpriteManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include <cassert>

void SpriteManager::Initialize() {
	spriteVertexBuffers_.clear();
}

void SpriteManager::ResizeSprite(uint32_t handle, float width, float height) {
	assert(handle < spriteVertexBuffers_.size());
	VertexData* vertexData = spriteVertexBuffers_[handle].GetData();
	assert(vertexData);
	float w = width;
	float h = height;
	vertexData[0].position = { 0.0f, 0.0f, 0.0f,1.0f };   // 左上
	vertexData[0].texcoord = { 0.0f, 0.0f };
	vertexData[0].normal = { 0.0f, 0.0f, 1.0f };
	vertexData[1].position = { w, 0.0f, 0.0f ,1.0f }; // 右上
	vertexData[1].texcoord = { 1.0f, 0.0f };
	vertexData[1].normal = { 0.0f, 0.0f, 1.0f };
	vertexData[2].position = { 0.0f, h, 0.0f ,1.0f }; // 左下
	vertexData[2].texcoord = { 0.0f, 1.0f };
	vertexData[2].normal = { 0.0f, 0.0f,1.0f };
	vertexData[3].position = { w, h, 0.0f ,1.0f }; // 右下
	vertexData[3].texcoord = { 1.0f, 1.0f };
	vertexData[3].normal = { 0.0f, 0.0f, 1.0f };
	vertexData[4].position = { 0.0f, h, 0.0f,1.0f }; // 左下
	vertexData[4].texcoord = { 0.0f, 1.0f };
	vertexData[4].normal = { 0.0f, 0.0f, 1.0f };
	vertexData[5].position = { w, 0.0f, 0.0f ,1.0f }; // 右上
	vertexData[5].texcoord = { 1.0f, 0.0f };
}

uint32_t SpriteManager::CreateVertexBuffer(float width, float height) {
    spriteVertexBuffers_.emplace_back();
	spriteVertexBuffers_.back().CreateResource(DirectXCommon::GetInstance()->GetDevice(), 6);
	VertexData* vertexData = spriteVertexBuffers_.back().GetData();
	assert(vertexData);
	std::vector<VertexData> tempVertices;
	tempVertices.resize(6);
	float w = width;
	float h = height;
	tempVertices[0].position = { 0.0f, 0.0f, 0.0f,1.0f };   // 左上
	tempVertices[0].texcoord = { 0.0f, 0.0f };
	tempVertices[0].normal = { 0.0f, 0.0f, 1.0f };
	tempVertices[1].position = { w, 0.0f, 0.0f ,1.0f}; // 右上
	tempVertices[1].texcoord = { 1.0f, 0.0f };
	tempVertices[1].normal = { 0.0f, 0.0f, 1.0f };
	tempVertices[2].position = { 0.0f, h, 0.0f ,1.0f }; // 左下
	tempVertices[2].texcoord = { 0.0f, 1.0f };
	tempVertices[2].normal = { 0.0f, 0.0f,1.0f };
	tempVertices[3].position = { w, h, 0.0f ,1.0f }; // 右下
	tempVertices[3].texcoord = { 1.0f, 1.0f };
	tempVertices[3].normal = { 0.0f, 0.0f, 1.0f };
	tempVertices[4].position = { 0.0f, h, 0.0f,1.0f }; // 左下
	tempVertices[4].texcoord = { 0.0f, 1.0f };
	tempVertices[4].normal = { 0.0f, 0.0f, 1.0f };
	tempVertices[5].position = { w, 0.0f, 0.0f ,1.0f }; // 右上
	tempVertices[5].texcoord = { 1.0f, 0.0f };
	tempVertices[5].normal = { 0.0f, 0.0f, 1.0f };
	memcpy(vertexData, tempVertices.data(), sizeof(VertexData) * 6);
	return static_cast<uint32_t>(spriteVertexBuffers_.size()) - 1;
}

VertexBuffer<VertexData>* SpriteManager::GetVertexBuffer(uint32_t handle) {
	return &spriteVertexBuffers_[handle];
}

Vector2 SpriteManager::GetSpriteSize(uint32_t handle) {
	if (handle < spriteVertexBuffers_.size()) {
		VertexData* vertexData = spriteVertexBuffers_[handle].GetData();
		assert(vertexData);
		return Vector2(vertexData[1].position.x, vertexData[2].position.y);
	}
	assert(false && "Invalid handle");
	return Vector2();
}

void SpriteManager::Finalize() {
	spriteVertexBuffers_.clear();
}
