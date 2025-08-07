#pragma once
#include "Object/Structs/ModelData.h"
#include "../Math/Transform.h"
#include "../Base/DirectX/PipelineStateObject.h"
#include "../Base/DirectX/Descriptors/SrvDescriptorHeap.h"

#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"
#include "Base/DirectX/Resource/ShaderBuffers/StructuredBuffer.h"

#include "Shaders/StructsForGpu/ParticleForGPU.h"
#include "Shaders/StructsForGpu/Material.h"

class Camera;
class EngineCore;

class Particle {
public:
	Particle();

public:
	void Initialize(EngineCore* engineCore, uint32_t totalParticles);
	void LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem);
	void Draw(std::vector<Transform>* transform, std::vector<Vector4>* color, Camera* camera);

private:
	StructuredBuffer<ParticleForGPU> wvp_;
	ConstantBuffer<Material> material_;

private:
	static uint32_t instanceCount_;
	uint32_t totalParticles_;
	EngineCore* engineCore_;
	PipelineStateObject* pso_;
	
private:
	ModelData modelData_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	int32_t modelTextureHandle_;
	DescriptorHandles instancingSrvHandles_;
};