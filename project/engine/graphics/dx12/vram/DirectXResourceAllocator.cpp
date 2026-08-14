#include "DirectXResourceAllocator.h"

namespace {
    const size_t kMaxGrowSize = 16;
}

void QFE::GRAPHIC::DirectXResourceAllocator::Initialize(DirectXResourceContainer* container, ID3D12Device* device) {
    if (isInitialized_) {
		QFE_LOG("DirectXResourceAllocator is already initialized.");
		return; // すでに初期化済みの場合は何もしない
    }
    container_ = container;
    device_ = device;
	isInitialized_ = true;
}

QFE::GRAPHIC::DirectXResourceHandle QFE::GRAPHIC::DirectXResourceAllocator::AllocateRawConstantBuffer(size_t byteSize, const std::string& name) {
	DirectXResourceHandle handle = DirectXResourceHandle::Invalid;
	// CBVは256Bアライメントが必要
	size_t alignedSize = (byteSize + 255) & ~255; // 256Bアライメント
	// アライメント済みサイズのバケットを取得
	auto& bucket = cbvPools_[alignedSize];
	// バケットが空の場合、または現在のオフセットがリソース数を超えた場合にGrowを呼び出す
    if (bucket.currentOffset >= bucket.resources.size()) {
        Grow(bucket, alignedSize, alignedSize, false, 0, name);
	}
	// 現在のオフセットのリソースを返し、オフセットをインクリメント
	handle = bucket.resources[bucket.currentOffset++];
	return handle;
}

QFE::GRAPHIC::DirectXResourceHandle QFE::GRAPHIC::DirectXResourceAllocator::AllocateRawStructuredBuffer(
    size_t byteSize, size_t stride, const std::string& name) {
	
    DirectXResourceHandle handle = DirectXResourceHandle::Invalid;
	// SRVは「型情報」と「要素数」のペアで管理するため、要素数を計算
    size_t elementCount = byteSize / stride;
    auto key = std::make_pair(std::type_index(typeid(void)), elementCount);
    auto& bucket = srvPools_[key];
    // バケットが空の場合、または現在のオフセットがリソース数を超えた場合にGrowを呼び出す
    if (bucket.currentOffset >= bucket.resources.size()) {
        Grow(bucket, byteSize, stride, true, elementCount, name);
    }
    // 現在のオフセットのリソースを返し、オフセットをインクリメント
    handle = bucket.resources[bucket.currentOffset++];
	return handle;
}

void QFE::GRAPHIC::DirectXResourceAllocator::ResetFrame() {
    // CBVプールのリセット
    for (auto& [size, bucket] : cbvPools_) {
        bucket.currentOffset = 0;
    }
    // SRVプールのリセット
    for (auto& [key, bucket] : srvPools_) {
        bucket.currentOffset = 0;
    }
}

void QFE::GRAPHIC::DirectXResourceAllocator::Grow(
    SizeBucket& bucket, size_t totalSize, size_t stride, bool isSRV, size_t elementCount, const std::string& name) {

    constexpr size_t kGrowSize = kMaxGrowSize;
    bucket.resources.reserve(bucket.resources.size() + kGrowSize);

    for (size_t i = 0; i < kGrowSize; ++i) {
        DirectXResourceHandle handle = container_->CreateBuffer(device_, totalSize);

        container_->MapResource(handle);
        container_->SetResourceStrideInBytes(handle, stride);

        std::string uniqueName = name + "_" + std::to_string(totalSize) + "_" + std::to_string(bucket.resources.size());
        container_->SetResourceName(handle, ConvertString(uniqueName));

        // SRVが必要な場合のみ、初期生成時に1回だけビュー（SRV）を自動登録する
        if (isSRV) {
            CreateViewInfo viewInfo{};
            viewInfo.viewType = ViewTypeFlags::ShaderResourceView;
            viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            viewInfo.srvDesc.Buffer.FirstElement = 0;
            viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(elementCount);
            viewInfo.srvDesc.Buffer.StructureByteStride = static_cast<UINT>(stride);
            viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            container_->CreateResourceView(handle, viewInfo);
        }

        bucket.resources.push_back(handle);
    }
}