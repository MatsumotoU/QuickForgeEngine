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
            CereateViewInfo viewInfo{};
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