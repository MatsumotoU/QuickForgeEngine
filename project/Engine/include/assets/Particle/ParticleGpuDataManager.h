#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include "Graphic/ShaderBuffer/ConstantBuffer.h"
#include "Graphic/ShaderBuffer/StructuredBuffer.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "Utility/memory/SparseSets.h"

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