#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include "engine/include/graphic/ShaderBuffer/ConstantBuffer.h"
#include "engine/include/graphic/ShaderBuffer/StructuredBuffer.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "engine/include/utility/memory/SparseSets.h"

class DirectXCommon;
class SrvDescriptorHeap;

class ParticleGpuDataManager final {
public:
	void Initialize();
	uint32_t CreateParticleBuffer(uint32_t maxParticleCount);
	ID3D12Resource* GetResourcePtr(uint32_t bufferId);
	ParticleForGPU* GetDataPtr(uint32_t bufferId);
	StructuredBuffer<ParticleForGPU>* GetBufferPtr(uint32_t bufferId);
	void Finalize();

private:
	uint32_t nextBufferId_;
	DirectXCommon* dxCommon_;
	SrvDescriptorHeap* srvDescriptorHeap_;
	SparseSet<StructuredBuffer<ParticleForGPU>>  particleBuffers_;
};
