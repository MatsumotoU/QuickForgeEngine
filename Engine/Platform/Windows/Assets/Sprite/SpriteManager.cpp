#include "SpriteManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"

void SpriteManager::Initialize() {
	spriteVertexBuffers_.clear();
}

uint32_t SpriteManager::CreateVertexBuffer(float width, float height) {
    spriteVertexBuffers_.emplace_back();
	spriteVertexBuffers_.back().CreateResource(DirectXCommon::GetInstance()->GetDevice(), 6);
	// 頂点データをセット
	VertexData topLeft;
	topLeft.normal = { 0.0f,0.0f,-1.0f };
	topLeft.texcoord = { 0.0f,0.0f };
	topLeft.position = { 0.0f, 0.0f, 0.0f };
	spriteVertexBuffers_.back().SetData(0, topLeft);
	VertexData topRight;
	topRight.normal = { 0.0f,0.0f,-1.0f };
	topRight.texcoord = { 1.0f,0.0f };
	topRight.position = { width, 0.0f, 0.0f };
	spriteVertexBuffers_.back().SetData(1, topRight);
	VertexData bottomLeft;
	bottomLeft.normal = { 0.0f,0.0f,-1.0f };
	bottomLeft.texcoord = { 0.0f,1.0f };
	bottomLeft.position = { 0.0f, height, 0.0f };
	spriteVertexBuffers_.back().SetData(2, bottomLeft);
	VertexData bottomRight;
	bottomRight.normal = { 0.0f,0.0f,-1.0f };
	bottomRight.texcoord = { 1.0f,1.0f };
	bottomRight.position = { width, height, 0.0f };
	spriteVertexBuffers_.back().SetData(3, bottomRight);
	spriteVertexBuffers_.back().SetData(4, bottomLeft);
	spriteVertexBuffers_.back().SetData(5, topRight);
	return static_cast<uint32_t>(spriteVertexBuffers_.size()) - 1;
}

VertexBuffer<VertexData>* SpriteManager::GetVertexBuffer(uint32_t handle) {
	return &spriteVertexBuffers_[handle];
}

void SpriteManager::Finalize() {
	spriteVertexBuffers_.clear();
}
