#include "ParticleGpuDataManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"

void ParticleGpuDataManager::Initialize() {
	particleBuffers_.clear();
	dxCommon_ = DirectXCommon::GetInstance();
	srvDescriptorHeap_ = dxCommon_->GetDescriptorHeapManager()->GetSrvDescriptorHeap();
	nextBufferId_ = 0;
}

uint32_t ParticleGpuDataManager::CreateParticleBuffer(uint32_t maxParticleCount) {
	StructuredBuffer<ParticleForGPU> particleBuffer;
	particleBuffer.CreateResource(dxCommon_, srvDescriptorHeap_, maxParticleCount);
	particleBuffers_.Insert(nextBufferId_, std::move(particleBuffer));
	return nextBufferId_++;
}

ID3D12Resource* ParticleGpuDataManager::GetResourcePtr(uint32_t bufferId) {
	return particleBuffers_.at(bufferId).GetResource();
}

ParticleForGPU* ParticleGpuDataManager::GetDataPtr(uint32_t bufferId) {
	return particleBuffers_.at(bufferId).GetData();
}

void ParticleGpuDataManager::Finalize() {
	particleBuffers_.clear();
}
