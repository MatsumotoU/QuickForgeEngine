#include "GraphicsCommon.h"

void GraphicsCommon::Initialize(DirectXCommon* dxCommon, WinApp* winApp) {
	depthStencil_.Initialize(winApp, dxCommon);

	// PSOを作成
	trianglePso_.Initialize(dxCommon, winApp, &depthStencil_, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,D3D12_FILL_MODE_SOLID,"Object3d.PS.hlsl");
	linePso_.Initialize(dxCommon, winApp, &depthStencil_, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl");
	pointPso_.Initialize(dxCommon, winApp, &depthStencil_, D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT, D3D12_FILL_MODE_SOLID, "Object3d.PS.hlsl");

	grayScaleTrianglePso_.Initialize(dxCommon, winApp, &depthStencil_, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D12_FILL_MODE_SOLID, "GrayscaleShader.hlsl");
}