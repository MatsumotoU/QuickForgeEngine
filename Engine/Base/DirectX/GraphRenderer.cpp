#include "GraphRenderer.h"
#include "Base/EngineCore.h"

void GraphRenderer::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	trianglePso_ = engineCore->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);
	linePso_ = engineCore->GetGraphicsCommon()->GetLinePso(kBlendModeNormal);
	pointPso_ = engineCore->GetGraphicsCommon()->GetPointPso(kBlendModeNormal);

	triangleWvp_.Initialize(engineCore_->GetDirectXCommon(), 1);
	lineWvp_.Initialize(engineCore_->GetDirectXCommon(), 1);
	pointWvp_.Initialize(engineCore_->GetDirectXCommon(), 1);
}

void GraphRenderer::DrawTriangle(Vector3 point1, Vector3 point2, Vector3 point3, Vector4 color) {
	// 三角形の頂点データを設定
	triangleWvp_.SetWorldMatrix(Matrix4x4::MakeIndentity4x4(), 0);
	triangleWvp_.SetWVPMatrix(Matrix4x4::MakeIndentity4x4(), 0);
	material_.materialData_->color = color;
	ID3D12GraphicsCommandList* commandList = engineCore_->GetDirectXCommon()->GetCommandList();
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetMaterial()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, triangleWvp_.GetWVPResource()->GetGPUVirtualAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawInstanced(3, 1, 0, 0);
}
