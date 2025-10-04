#include "SpriteManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include <cassert>

void SpriteManager::Initialize() {
	spriteVertexBuffers_.clear();
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

void SpriteManager::Finalize() {
	spriteVertexBuffers_.clear();
}
