#include "GraphicsCommon.h"
#include "Base/EngineCore.h"

void GraphicsCommon::Initialize(EngineCore* engineCore) {
	depthStencil_.Initialize(engineCore->GetWinApp(), engineCore->GetDirectXCommon());
	// 通常のルートパラメータ
	normalGameObjectRootParameter_.Initialize();
	normalGameObjectRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalGameObjectRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	normalGameObjectRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	normalGameObjectRootParameter_.CreateRootParameter("LightParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 1);

	normalGameObjectRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// パーティクルのルートパラメータ
	particleRootParameter_.Initialize();
	particleRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	particleRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_VERTEX, 0);
	particleRootParameter_.CreateRootParameter("TextureParameter", D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_SHADER_VISIBILITY_PIXEL, 0);

	particleRootParameter_.SetDescriptorRange("VertexParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
	particleRootParameter_.SetDescriptorRange("TextureParameter", D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// プリミティブのルートパラメータ
	primitiveRootParameter_.Initialize();
	primitiveRootParameter_.CreateRootParameter("PixelParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, 0);
	primitiveRootParameter_.CreateRootParameter("VertexParameter", D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, 0);

	// パラメータの整合性チェック
#ifdef _DEBUG
	normalGameObjectRootParameter_.CheckIntegrityData();
	particleRootParameter_.CheckIntegrityData();
	primitiveRootParameter_.CheckIntegrityData();
#endif // _DEBUG

	// PSOを作成
	for (int i = 0; i < kCountOfBlendMode; i++) {
		// 初期化
		trianglePso_[i].Initialize(engineCore);
		linePso_[i].Initialize(engineCore);
		pointPso_[i].Initialize(engineCore);
		particlePso_[i].Initialize(engineCore);
		primitivePso_[i].Initialize(engineCore);

		trianglePso_[i].CreatePipelineStateObject(
			normalGameObjectRootParameter_, &depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl", static_cast<BlendMode>(i), false,false);

		linePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, &depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", static_cast<BlendMode>(i), false,true);

		pointPso_[i].CreatePipelineStateObject(
			primitiveRootParameter_, &depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", static_cast<BlendMode>(i), false,true);

		primitivePso_[i].CreatePipelineStateObject(
			primitiveRootParameter_,&depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Primitive.PS.hlsl", static_cast<BlendMode>(i), false,true);

		particlePso_[i].CreatePipelineStateObject(
			particleRootParameter_, &depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Particle.PS.hlsl", static_cast<BlendMode>(i), true,false);
	}
	
	grayScaleTrianglePso_.Initialize(engineCore);
	grayScaleTrianglePso_.CreatePipelineStateObject(
		normalGameObjectRootParameter_, &depthStencil_,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "GrayscaleShader.hlsl", kBlendModeNormal, false,false);
}