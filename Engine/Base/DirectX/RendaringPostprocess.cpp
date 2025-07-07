#include "RendaringPostprocess.h"
#include "Base/EngineCore.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG


RendaringPostprosecess::RendaringPostprosecess() {
	isPostprocess_ = true;
	engineCore_ = nullptr;

	postProcessCount_ = 0;
	enableGrayscale_ = true;
	enableBloom_ = true;

	renderingRosourceIndex_ = 0;
	readingResourceIndex_ = 0;

	isFirstStateRenderTarget_ = false;
	isSecondStateRenderTarget_ = false;

	offScreenClearColor[0] = 0.1f;
	offScreenClearColor[1] = 0.25f;
	offScreenClearColor[2] = 0.5f;
	offScreenClearColor[3] = 1.0f;

	postProcessFunctions_.clear();
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyGrayScale, this));
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyBloom, this));

	postProcessOrderForm_.clear();
	postProcessOrderForm_.push_back(1); // グレースケール
	postProcessOrderForm_.push_back(0); // ブルーム

	grayScaleOffset_ = 0.0f;
}

void RendaringPostprosecess::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	offscreen_.Initialize(engineCore, 
		static_cast<float>(engineCore->GetWinApp()->kWindowWidth), static_cast<float>(engineCore->GetWinApp()->kWindowHeight));

	// Spriteを作る
	vertexResource_ = CreateBufferResource(engineCore->GetDirectXCommon()->GetDevice(), sizeof(VertexData) * 4);
	vertexBufferView_ = {};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データ作成
	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	vertexData_[0].position = { -1.0f,1.0f,0.0f,1.0f };
	vertexData_[0].texcoord = { 0.0f,0.0f };
	vertexData_[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[1].position = { -1.0f,-1.0f,0.0f,1.0f };
	vertexData_[1].texcoord = { 0.0f,1.0f };
	vertexData_[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[2].position = { 1.0f,1.0f,0.0f,1.0f };
	vertexData_[2].texcoord = { 1.0f,0.0f };
	vertexData_[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[3].position = { 1.0f,-1.0f,0.0f,1.0f };
	vertexData_[3].texcoord = { 1.0f,1.0f };
	vertexData_[3].normal = { 0.0f,0.0f,-1.0f };

	// indexBufferの作成
	indexResource_ = CreateBufferResource(engineCore->GetDirectXCommon()->GetDevice(), sizeof(uint32_t) * 6);
	indexBufferView_ = {};
	indexBufferView_.BufferLocation = indexResource_.Get()->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	indexData_ = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	indexData_[0] = 0;
	indexData_[1] = 1;
	indexData_[2] = 2;
	indexData_[3] = 1;
	indexData_[4] = 3;
	indexData_[5] = 2;

	// グレースケールのリソース作成
	grayScalePso_ = engineCore_->GetGraphicsCommon()->GetGrayScalePso();
	grayScaleResource_ = CreateBufferResource(engineCore->GetDirectXCommon()->GetDevice(), sizeof(OffsetBuffer));
	grayScaleOffsetBuffer_ = nullptr;
	grayScaleResource_->Map(0, nullptr, reinterpret_cast<void**>(&grayScaleOffsetBuffer_));
}

void RendaringPostprosecess::PreDraw() {
	// 何回ポストプロセスがかかっているか調べる
	postProcessCount_ = 0;
	if (enableGrayscale_) {
		postProcessCount_++;
		// グレースケールの強度
		grayScaleOffsetBuffer_->offset.x = grayScaleOffset_;
	}
	if (enableBloom_) {
		postProcessCount_++;
	}

	// オフスクリーンのバリアを設定
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	if (!isFirstStateRenderTarget_) {
		dxCommon->TransitionResourceBarrier(
			dxCommon->GetOffscreenResource(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		isFirstStateRenderTarget_ = true;
	}
	if (!isSecondStateRenderTarget_) {
		dxCommon->TransitionResourceBarrier(
			dxCommon->GetOffscreenResource(1), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		isSecondStateRenderTarget_ = true;
	}

	// 描画先の設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = engineCore_->GetGraphicsCommon()->GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	if (isPostprocess_) {
		// オフスクリーンに描画
		renderingRosourceIndex_ = 0;
		dxCommon->GetCommandList()->OMSetRenderTargets(1, dxCommon->GetOffscreenRtvHandles(renderingRosourceIndex_), false, &dsvHandl);

		// オフスクリーンのクリア
		ClearFirstRenderTarget();
		// 2つ目のオフスクリーンのクリア
		ClearSecondRenderTarget();
	} else {
		// バックバッファに描画
		dxCommon->GetCommandList()->OMSetRenderTargets(1, dxCommon->GetRtvHandles(), false, &dsvHandl);
	}
}

void RendaringPostprosecess::PostDraw() {
	// ポストプロセスが有効でないなら何もしない
	if (!isPostprocess_) {
		return;
	} 
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	// オフスクリーンのバリア
	SwitchRenderTarget();
	for (uint32_t i = 0; i < postProcessCount_; i++) {
		// ポストプロセスの適用
		postProcessFunctions_[postProcessOrderForm_[i]]();
		SwitchRenderTarget();
	}

	// ポストプロセスの適用
	readingResourceIndex_ = postProcessCount_ % 2;
	// バックバッファに書き込み
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = engineCore_->GetGraphicsCommon()->GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	commandList->OMSetRenderTargets(1, dxCommon->GetRtvHandles(), false, &dsvHandl);
	commandList->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	commandList->SetGraphicsRootSignature(engineCore_->GetGraphicsCommon()->GetNormalPso()->GetRootSignature());
	commandList->SetPipelineState(engineCore_->GetGraphicsCommon()->GetNormalPso()->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(0, engineCore_->GetTextureManager()->GetOffscreenSrvHandleGPU(readingResourceIndex_));
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

#ifdef _DEBUG
void RendaringPostprosecess::DrawImGui() {
	// ポストプロセスのデバッグウィンドウを表示
	ImGui::Checkbox("Enable Postprocess", &isPostprocess_);
	ImGui::Separator();
	if (isPostprocess_) {
		ImGui::Checkbox("Enable Grayscale", &enableGrayscale_);
		if (enableGrayscale_) {
			ImGui::SliderFloat("Grayscale Offset", &grayScaleOffset_, 0.0f, 1.0f);
			ImGui::Spacing();
		}
		ImGui::Checkbox("Enable Bloom", &enableBloom_);
	}
}
#endif

void RendaringPostprosecess::ClearFirstRenderTarget() {
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	commandList->ClearRenderTargetView(*(dxCommon->GetOffscreenRtvHandles(0)), offScreenClearColor, 0, nullptr);
}

void RendaringPostprosecess::ClearSecondRenderTarget() {
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();

	commandList->ClearRenderTargetView(*(dxCommon->GetOffscreenRtvHandles(1)), offScreenClearColor, 0, nullptr);
}

void RendaringPostprosecess::SwitchRenderTarget() {
	// ポストプロセスが有効でないなら何もしない
	if (!isPostprocess_) {
		return;
	}

	// オフスクリーンのバリア
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	if (renderingRosourceIndex_ == 0) {
		if (isFirstStateRenderTarget_) {
			dxCommon->TransitionResourceBarrier(
				dxCommon->GetOffscreenResource(0), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			isFirstStateRenderTarget_ = false;
		}
		if (!isSecondStateRenderTarget_) {
			dxCommon->TransitionResourceBarrier(
				dxCommon->GetOffscreenResource(1), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			isSecondStateRenderTarget_ = true;
		}
		ClearSecondRenderTarget();

	} else {
		if (!isFirstStateRenderTarget_) {
			dxCommon->TransitionResourceBarrier(
				dxCommon->GetOffscreenResource(0), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			isFirstStateRenderTarget_ = true;
		}
		if (isSecondStateRenderTarget_) {
			dxCommon->TransitionResourceBarrier(
				dxCommon->GetOffscreenResource(1), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			isSecondStateRenderTarget_ = false;
		}
		ClearFirstRenderTarget();
	}
	readingResourceIndex_ = renderingRosourceIndex_;
	renderingRosourceIndex_ = (renderingRosourceIndex_ + 1) % 2;

	// 描画先の設定
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = engineCore_->GetGraphicsCommon()->GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	dxCommon->GetCommandList()->OMSetRenderTargets(1, dxCommon->GetOffscreenRtvHandles(renderingRosourceIndex_), false, &dsvHandl);
}

void RendaringPostprosecess::ApplyGrayScale() {
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	commandList->SetGraphicsRootSignature(grayScalePso_->GetRootSignature());
	commandList->SetPipelineState(grayScalePso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(0, engineCore_->GetTextureManager()->GetOffscreenSrvHandleGPU(readingResourceIndex_));
	commandList->SetGraphicsRootConstantBufferView(1, grayScaleResource_.Get()->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void RendaringPostprosecess::ApplyBloom() {
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	commandList->SetGraphicsRootSignature(engineCore_->GetGraphicsCommon()->GetNormalPso()->GetRootSignature());
	commandList->SetPipelineState(engineCore_->GetGraphicsCommon()->GetNormalPso()->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(0, engineCore_->GetTextureManager()->GetOffscreenSrvHandleGPU(readingResourceIndex_));
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
