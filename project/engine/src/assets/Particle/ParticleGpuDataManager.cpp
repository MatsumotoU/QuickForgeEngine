#include "engine/include/assets/Particle/ParticleGpuDataManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"

void ParticleGpuDataManager::Initialize() {
	particleBuffers_.clear();
	dxCommon_ = DirectXCommon::GetInstance();
	srvDescriptorHeap_ = dxCommon_->GetDescriptorHeapManager()->GetSrvDescriptorHeap();
	nextBufferId_ = 0;
}

uint32_t ParticleGpuDataManager::CreateParticleBuffer(uint32_t maxParticleCount) {
	StructuredBuffer<ParticleForGPU> particleBuffer;
	particleBuffer.CreateResource(dxCommon_, srvDescriptorHeap_, maxParticleCount);
	for (uint32_t i = 0; i < maxParticleCount; i++) {
		particleBuffer.GetData()[i].color = Vector4{ 1.0f, 1.0f, 1.0f, 0.0f };
		particleBuffer.GetData()[i].World = Matrix4x4::MakeAffineMatrix(
			Vector3{ 0.0f, 0.0f, 0.0f },
			Vector3{ 0.0f, 0.0f, 0.0f },
			Vector3{ 0.0f, 0.0f, 0.0f });
		particleBuffer.GetData()[i].WVP = Matrix4x4::MakeIndentity4x4();
	}
	particleBuffers_.Insert(nextBufferId_, std::move(particleBuffer));
	return nextBufferId_++;
}

void ParticleGpuDataManager::ReleaseParticleBuffer(uint32_t bufferId) {
	particleBuffers_.Remove(bufferId);
}

void ParticleGpuDataManager::Reset() {
	particleBuffers_.clear();
	nextBufferId_ = 0;
}

ID3D12Resource* ParticleGpuDataManager::GetResourcePtr(uint32_t bufferId) {
	return particleBuffers_.at(bufferId).GetResource();
}

ParticleForGPU* ParticleGpuDataManager::GetDataPtr(uint32_t bufferId) {
	return particleBuffers_.at(bufferId).GetData();
}

StructuredBuffer<ParticleForGPU>* ParticleGpuDataManager::GetBufferPtr(uint32_t bufferId) {
	return &particleBuffers_.at(bufferId);
}

void ParticleGpuDataManager::Finalize() {
	Reset();
}
