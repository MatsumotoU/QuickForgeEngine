#include "Font.h"
#include "Base/EngineCore.h"
#include "Camera/Camera.h"
#include "Base/MyString.h"

void Font::Initialize(const std::string& fontFilePath, float fontSize) {
	// フォントに使うリソース読み込み
	fontAtlasData_ = FontLoader::LoadFontData(fontFilePath + ".json");
	textureHandle_ = engineCore_->GetTextureManager()->LoadTexture(fontFilePath + ".png");

	// フォントのデータをPSシェーダーに送るためのバッファを作成
	fontDataBuffer_.CreateResource(engineCore_->GetDirectXCommon()->GetDevice());
	fontDataBuffer_.GetData()->AtlasSize = Vector2(fontAtlasData_.width, fontAtlasData_.height);
	fontDataBuffer_.GetData()->DistanceRange = fontAtlasData_.distanceRange;

	// ローカル頂点生成
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

	for (int i = 0; i < static_cast<int>(kInstanceCount_); i++) {
		transform[i].scale = Vector3(1.0f, 1.0f, 1.0f);
		transform[i].rotate = Vector3(0.0f, 0.0f, 0.0f);
		transform[i].translate = Vector3(static_cast<float>(i * fontSize), 0.0f, 0.0f);
	}
	
	// インスタンス化用のWVPバッファを作成
	wvpBuffer_.CreateResource(engineCore_->GetDirectXCommon(), engineCore_->GetSrvDescriptorHeap(), kInstanceCount_);
	fontSize_ = fontSize;

}

void Font::Draw(const std::string& text, Camera* camera) {
	char* stringBuffer = StringToCharPtr(text);
	assert(stringBuffer != nullptr && "Font::Draw: text is null");

	FontUVData uv = {};
	for (uint32_t index = 0; index < kInstanceCount_; index++) {
		if (index >= text.length()) {
			uv = FontLoader::GetGlyphUVData(fontAtlasData_, stringBuffer[text.size()-1]);

			transform[index].scale = transform[text.size() - 1].scale;
			transform[index].rotate = transform[text.size() - 1].rotate;
			transform[index].translate = transform[text.size() - 1].translate;
		} else {
			uv = FontLoader::GetGlyphUVData(fontAtlasData_, stringBuffer[index]);

			transform[index].scale = Vector3(1.0f, 1.0f, 1.0f);
			transform[index].rotate = Vector3(0.0f, 0.0f, 0.0f);
			transform[index].translate = Vector3(static_cast<float>(index * fontSize_), 0.0f, 0.0f);
		}
		
		Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform[index].scale, transform[index].rotate, transform[index].translate);
		Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix, ViewState::CAMERA_VIEW_STATE_PERSPECTIVE);
		GlyphForGPU glyphData;
		glyphData.WVP = wvpMatrix;
		glyphData.texcorad = Vector4(uv.leftBottom.x, uv.leftBottom.y, uv.rightTop.x, uv.rightTop.y);
		wvpBuffer_.SetData(glyphData, index);

		
	}

	// sprite
	ID3D12GraphicsCommandList* commandList = engineCore_->GetDirectXCommon()->GetCommandList();
	commandList->RSSetViewports(1, camera->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(engineCore_->GetGraphicsCommon()->GetFontPso()->GetRootSignature());
	commandList->SetPipelineState(engineCore_->GetGraphicsCommon()->GetFontPso()->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, vertexBuffer_.GetVertexBufferView());
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(0, wvpBuffer_.GetInstancingSrvHandles().gpuHandle_);
	commandList->SetGraphicsRootDescriptorTable(1, engineCore_->GetTextureManager()->GetTextureSrvHandleGPU(textureHandle_));
	commandList->SetGraphicsRootConstantBufferView(2, fontDataBuffer_.GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, kInstanceCount_, 0, 0, 0);
}