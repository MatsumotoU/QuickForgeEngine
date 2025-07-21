#include "Font.h"
#include "Base/EngineCore.h"
#include "Camera/Camera.h"

void Font::Initialize(const std::string& fontFilePath, float fontSize) {
	fontAtlasData_ = FontLoader::LoadFontData(fontFilePath + ".json");
	textureHandle_ = engineCore_->GetTextureManager()->LoadTexture(fontFilePath + ".png");

	fontDataBuffer_.CreateResource(engineCore_->GetDirectXCommon()->GetDevice());
	fontDataBuffer_.GetData()->AtlasSize = Vector2(fontAtlasData_.width, fontAtlasData_.height);
	fontDataBuffer_.GetData()->DistanceRange = fontAtlasData_.distanceRange;

	wvpResource_.Initialize(engineCore_->GetDirectXCommon(), kIncetanceCount_,true);

	vertexBuffer_.CreateResource(engineCore_->GetDirectXCommon()->GetDevice(), 4);
	vertexBuffer_.GetData()[0].position = { 0.0f,fontSize,0.0f,1.0f };
	vertexBuffer_.GetData()[0].texcoord = { 0.0f,1.0f };
	vertexBuffer_.GetData()[0].color = { 1.0f,1.0f,1.0f,1.0f };
	vertexBuffer_.GetData()[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexBuffer_.GetData()[1].texcoord = { 0.0f,0.0f };
	vertexBuffer_.GetData()[1].color = { 1.0f,1.0f,1.0f,1.0f };
	vertexBuffer_.GetData()[2].position = { fontSize,fontSize,0.0f,1.0f };
	vertexBuffer_.GetData()[2].texcoord = { 1.0f,1.0f };
	vertexBuffer_.GetData()[2].color = { 1.0f,1.0f,1.0f,1.0f };
	vertexBuffer_.GetData()[3].position = { fontSize,0.0f,0.0f,1.0f };
	vertexBuffer_.GetData()[3].texcoord = { 1.0f,0.0f };
	vertexBuffer_.GetData()[3].color = { 1.0f,1.0f,1.0f,1.0f };

	// indexBufferの作成
	indexResource_ = CreateBufferResource(engineCore_->GetDirectXCommon()->GetDevice(), sizeof(uint32_t) * 6);
	indexBufferView_ = {};
	indexBufferView_.BufferLocation = indexResource_.Get()->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	indexData_ = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	indexData_[0] = 0;
	indexData_[1] = 2;
	indexData_[2] = 1;
	indexData_[3] = 1;
	indexData_[4] = 2;
	indexData_[5] = 3;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = kIncetanceCount_;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	instancingSrvHandles_ = engineCore_->GetSrvDescriptorHeap()->AssignEmptyArrayHandles();
	engineCore_->GetSrvDescriptorHeap()->AssignHeap(wvpResource_.GetWVPResource(), srvDesc, instancingSrvHandles_.cpuHandle_);

	for (int i = 0; i < static_cast<int>(kIncetanceCount_); i++) {
		wvpResource_.SetWorldMatrix(Matrix4x4::MakeAffineMatrix(Vector3(1.0f,1.0f,1.0f),Vector3(0.0f,0.0f,0.0f), Vector3(static_cast<float>(i * fontSize), 0.0f, 0.0f)), i);
	}
	
}

void Font::Draw(const char& text, Camera* camera) {
	FontUVData uv = FontLoader::GetGlyphUVData(fontAtlasData_, static_cast<unsigned int>(text));
	vertexBuffer_.GetData()[1].texcoord = uv.leftTop;
	vertexBuffer_.GetData()[0].texcoord = uv.leftBottom;
	vertexBuffer_.GetData()[3].texcoord = uv.rightTop;
	vertexBuffer_.GetData()[2].texcoord = uv.rightBottom;

	Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(wvpResource_.particleData_->World, CAMERA_VIEW_STATE_ORTHOGRAPHIC);
	wvpResource_.SetWorldMatrix(wvpResource_.particleData_->World, 0);
	wvpResource_.SetWVPMatrix(wvpMatrix, 0);

	/*for (uint32_t index = 0; index < kIncetanceCount_; index++) {
		Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(Matrix4x4::MakeIndentity4x4(), CAMERA_VIEW_STATE_PERSPECTIVE);
		wvpResource_.SetWVPMatrix(wvpMatrix, index);
	}*/

	// sprite
	ID3D12GraphicsCommandList* commandList = engineCore_->GetDirectXCommon()->GetCommandList();
	commandList->RSSetViewports(1, camera->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(engineCore_->GetGraphicsCommon()->GetFontPso()->GetRootSignature());
	commandList->SetPipelineState(engineCore_->GetGraphicsCommon()->GetFontPso()->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, vertexBuffer_.GetVertexBufferView());
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(0, instancingSrvHandles_.gpuHandle_);
	commandList->SetGraphicsRootDescriptorTable(1, engineCore_->GetTextureManager()->GetTextureSrvHandleGPU(textureHandle_));
	commandList->SetGraphicsRootConstantBufferView(2, fontDataBuffer_.GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, kIncetanceCount_, 0, 0, 0);
}