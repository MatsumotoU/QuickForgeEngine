#pragma once
#include "../Model/ModelData.h"
#include "../Math/Transform.h"
#include "../Base/DirectX/PipelineStateObject.h"
#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

class Camera;
class EngineCore;

class Particle {
public:
	void Initialize(EngineCore* engineCore, uint32_t totalParticles);
	void LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem);
	void Draw(std::vector<Transform>* transform, Camera* camera);

private:
	MaterialResource material_;
	DirectionalLightResource directionalLight_;
	WVPResource wvp_;

private:
	uint32_t totalParticles_;
	EngineCore* engineCore_;
	PipelineStateObject* pso_;
	
private:
	ModelData modelData_;
	VertexData* vertexData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	int32_t modelTextureHandle_;
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_;
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_;
};