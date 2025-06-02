#include "GraphicsCommon.h"

void GraphicsCommon::Initialize(DirectXCommon* dxCommon, WinApp* winApp) {
	depthStencil_.Initialize(winApp, dxCommon);

	// PSOを作成
	for (int i = 0; i < kCountOfBlendMode; i++) {
		trianglePso_[i].Initialize(
			dxCommon, winApp, &depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl", static_cast<BlendMode>(i),false);
		linePso_[i].Initialize(
			dxCommon, winApp, &depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl", static_cast<BlendMode>(i), false);
		pointPso_[i].Initialize(
			dxCommon, winApp, &depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl", static_cast<BlendMode>(i), false);


		particlePso_[i].Initialize(
			dxCommon, winApp, &depthStencil_,
			D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "Particle.PS.hlsl", static_cast<BlendMode>(i), true);
	}

	grayScaleTrianglePso_.Initialize(dxCommon, winApp, &depthStencil_, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "GrayscaleShader.hlsl", kBlendModeNormal, false);
}