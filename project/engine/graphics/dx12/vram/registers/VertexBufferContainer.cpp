#include "VertexBufferContainer.h"

using namespace QFE::GRAPHIC::INTERNAL;

void VertexBufferContainer::Initialize() {
	vertexBufferMap_.clear();
	loadNameMap_.clear();
}

uint32_t QFE::GRAPHIC::INTERNAL::VertexBufferContainer::Assign(ID3D12Device* device, const std::vector<VertexData>& vertex, const std::string& name) {
	// 同じ名前の頂点バッファが既に存在する場合はそのハンドルを返す
	auto it = loadNameMap_.find(name);
	if (it != loadNameMap_.end()) {
		QFE_LOG("Vertex buffer already loaded: " + name);
		QFE_LOG("Returning existing handle: " + std::to_string(it->second));
		return it->second;
	}

	// 頂点バッファの作成
	VertexBuffer<VertexData> vertexBuffer;
	vertexBuffer.CreateResource(device, static_cast<uint32_t>(vertex.size()));
	vertexBufferMap_.push_back(std::move(vertexBuffer));

	// ハンドルの生成とマップへの登録
	uint32_t handle = static_cast<uint32_t>(vertexBufferMap_.size() - 1);
	loadNameMap_[name] = handle;

	QFE_LOG("Vertex buffer loaded: " + name);
	QFE_LOG("Assigned handle: " + std::to_string(handle));
	return handle;
}

const uint32_t QFE::GRAPHIC::INTERNAL::VertexBufferContainer::GetVertexBufferCount(uint32_t handle) const {
	if(vertexBufferMap_.empty()) {
		QFE_REPORT_SYSTEM_ERROR("VertexBufferContainer is empty. No vertex buffer to retrieve.", SystemError::Abort);
		return 0;
	}
	if(vertexBufferMap_.size() <= handle) {
		QFE_REPORT_SYSTEM_ERROR("Invalid vertex buffer handle: " + std::to_string(handle) + ". Handle exceeds the number of loaded vertex buffers.", SystemError::Abort);
		return 0;
	}
	return vertexBufferMap_.at(handle).GetVertexCount();
}

ID3D12Resource* QFE::GRAPHIC::INTERNAL::VertexBufferContainer::GetModelVertexBuffer(const uint32_t& handle) {
	if(vertexBufferMap_.empty()) {
		QFE_REPORT_SYSTEM_ERROR("VertexBufferContainer is empty. No vertex buffer to retrieve.", SystemError::Abort);
		return nullptr;
	}
	if(vertexBufferMap_.size() <= handle) {
		QFE_REPORT_SYSTEM_ERROR("Invalid vertex buffer handle: " + std::to_string(handle) + ". Handle exceeds the number of loaded vertex buffers.", SystemError::Abort);
		return nullptr;
	}
	return vertexBufferMap_.at(handle).GetResource();
}

VertexData* QFE::GRAPHIC::INTERNAL::VertexBufferContainer::GetModelVertexBufferData(const uint32_t& handle) {
	if(vertexBufferMap_.empty()) {
		QFE_REPORT_SYSTEM_ERROR("VertexBufferContainer is empty. No vertex buffer to retrieve.", SystemError::Abort);
		return nullptr;
	}
	if(vertexBufferMap_.size() <= handle) {
		QFE_REPORT_SYSTEM_ERROR("Invalid vertex buffer handle: " + std::to_string(handle) + ". Handle exceeds the number of loaded vertex buffers.", SystemError::Abort);
		return nullptr;
	}
	return vertexBufferMap_.at(handle).GetData();
}

const D3D12_VERTEX_BUFFER_VIEW* QFE::GRAPHIC::INTERNAL::VertexBufferContainer::GetVertexBufferView(const uint32_t& handle) {
	if(vertexBufferMap_.empty()) {
		QFE_REPORT_SYSTEM_ERROR("VertexBufferContainer is empty. No vertex buffer to retrieve.", SystemError::Abort);
		return nullptr;
	}
	if(vertexBufferMap_.size() <= handle) {
		QFE_REPORT_SYSTEM_ERROR("Invalid vertex buffer handle: " + std::to_string(handle) + ". Handle exceeds the number of loaded vertex buffers.", SystemError::Abort);
		return nullptr;
	}
	return vertexBufferMap_.at(handle).GetVertexBufferView();
}

void VertexBufferContainer::Finalize() {
	vertexBufferMap_.clear();
	loadNameMap_.clear();
}