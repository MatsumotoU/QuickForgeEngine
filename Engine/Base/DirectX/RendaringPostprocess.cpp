#include "RendaringPostprocess.h"
#include "Base/EngineCore.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG


RendaringPostprosecess::RendaringPostprosecess() {
	isImGuiEnabled_ = false;
#ifdef _DEBUG
	isImGuiEnabled_ = true; // デバッグモードではImGuiを有効にする
#endif // _DEBUG

	isPostprocess_ = true;
	engineCore_ = nullptr;

	postProcessCount_ = 0;
	enableGrayscale_ = true;
	enableColorCorrection_ = true;
	enableVignette_ = true;
	enableNormal_ = true;

	renderingRosourceIndex_ = 0;
	readingResourceIndex_ = 0;

	isFirstStateRenderTarget_ = false;
	isSecondStateRenderTarget_ = false;

	offScreenClearColor[0] = 0.1f;
	offScreenClearColor[1] = 0.25f;
	offScreenClearColor[2] = 0.5f;
	offScreenClearColor[3] = 1.0f;

	// ポストプロセスの関数を登録
	postProcessFunctions_.clear();
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyGrayScale, this));
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyVignette, this));
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyColorCorrection, this));
	// 固定のインデックスを設定
	grayScaleProcessIndex_ = 0; // グレースケールのインデックス
	vignetteProcessIndex_ = 1; // ビネットのインデックス
	colorCorrectionProcessIndex_ = 2; // 色調補正のインデックス

	postProcessOrderForm_.clear();

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
	grayScaleOffsetBuffer_.CreateResource(engineCore_->GetDirectXCommon()->GetDevice());

	// 色調補正のリソース作成
	colorCorrectionPso_ = engineCore_->GetGraphicsCommon()->GetColorCorrectionPso();
	colorCorrectionOffsetBuffer_.CreateResource(engineCore_->GetDirectXCommon()->GetDevice());
	colorCorrectionOffsetBuffer_.GetData()->exposure = 0.0f; // 露出
	colorCorrectionOffsetBuffer_.GetData()->contrast = 1.0f; // コントラスト
	colorCorrectionOffsetBuffer_.GetData()->saturation = 1.0f; // 彩度
	colorCorrectionOffsetBuffer_.GetData()->gamma = 1.0f; // ガンマ
	colorCorrectionOffsetBuffer_.GetData()->hue = 0.0f;

	// ビネットのリソース作成
	vignettePso_ = engineCore_->GetGraphicsCommon()->GetVignettePso();
	vignetteOffsetBuffer_.CreateResource(engineCore_->GetDirectXCommon()->GetDevice());
	vignetteOffsetBuffer_.GetData()->VignetteRadius = 0.3f; // ビネットの半径
	vignetteOffsetBuffer_.GetData()->VignetteSoftness = 0.5f; // ビネットの柔らかさ
	vignetteOffsetBuffer_.GetData()->VignetteIntensity = 0.2f; // ビネットの強さ
}

void RendaringPostprosecess::PreDraw() {
	// 何回ポストプロセスがかかっているか調べる
	postProcessCount_ = 0;
	postProcessOrderForm_.clear();
	// グレースケール
	if (enableGrayscale_) {
		postProcessOrderForm_.push_back(grayScaleProcessIndex_); // グレースケール
		postProcessCount_++;
		// グレースケールの強度
		grayScaleOffsetBuffer_.GetData()->offset.x = grayScaleOffset_;
	}
	// ビネット
	if (enableVignette_) {
		postProcessOrderForm_.push_back(vignetteProcessIndex_); // ビネット
		postProcessCount_++;
	}
	// 色調補正
	if (enableColorCorrection_) {
		postProcessOrderForm_.push_back(colorCorrectionProcessIndex_); // 色調補正
		postProcessCount_++;
	}

	// ポストプロセスの順番を並び替える
	if (postProcessCount_ > 0) {
		grayScaleProcessIndex_ = std::clamp(grayScaleProcessIndex_, 0, static_cast<int>(postProcessCount_) - 1);
		vignetteProcessIndex_ = std::clamp(vignetteProcessIndex_, 0, static_cast<int>(postProcessCount_) - 1);
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

	// ImGuiのレンダリング用に絶対にバックバッファに描画する必要があるので残す
	if (isPostprocess_ || isImGuiEnabled_) {
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
#ifdef _DEBUG
	if (!isImGuiEnabled_) {
		return;
	}
#endif // _DEBUG
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
#ifdef _DEBUG
	if (isImGuiEnabled_) {
		return;
	}
#endif // _DEBUG
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
		ImGui::SameLine();
		ImGui::InputInt("Grayscale Process Index", &grayScaleProcessIndex_);
		if (enableGrayscale_) {
			if (ImGui::TreeNode("Grayscale Offset")) {
				ImGui::Text("Grayscale Offset: %.2f", grayScaleOffset_);
				ImGui::SliderFloat("Grayscale Offset", &grayScaleOffset_, 0.0f, 1.0f);
				ImGui::TreePop();
			}
			ImGui::Spacing();
		}

		ImGui::Checkbox("Enable Vignette", &enableVignette_);
		ImGui::SameLine();
		ImGui::InputInt("Vignette Process Index", &vignetteProcessIndex_);
		if (enableVignette_) {
			if (ImGui::TreeNode("Vignette Offset")) {
				ImGui::DragFloat2("ScreenResolution", &vignetteOffsetBuffer_.GetData()->screenResolution.x, 0.1f);
				ImGui::DragFloat("VignetteRadius", &vignetteOffsetBuffer_.GetData()->VignetteRadius, 0.1f);
				ImGui::DragFloat("VignetteSoftness", &vignetteOffsetBuffer_.GetData()->VignetteSoftness, 0.1f);
				ImGui::DragFloat("VignetteIntensity", &vignetteOffsetBuffer_.GetData()->VignetteIntensity, 0.1f);
				ImGui::TreePop();
			}
			ImGui::Spacing();
		}

		ImGui::Checkbox("Enable ColorCorrection", &enableColorCorrection_);
		ImGui::SameLine();
		ImGui::InputInt("ColorCorrection Process Index", &colorCorrectionProcessIndex_);
		if (enableColorCorrection_) {
			if (ImGui::TreeNode("ColorCorrection Offset")) {
				ImGui::DragFloat("Exposure", &colorCorrectionOffsetBuffer_.GetData()->exposure, 0.1f);
				ImGui::DragFloat("Contrast", &colorCorrectionOffsetBuffer_.GetData()->contrast, 0.1f);
				ImGui::DragFloat("Saturation", &colorCorrectionOffsetBuffer_.GetData()->saturation, 0.1f);
				ImGui::DragFloat("Gamma", &colorCorrectionOffsetBuffer_.GetData()->gamma, 0.1f);
				ImGui::DragFloat("Hue", &colorCorrectionOffsetBuffer_.GetData()->hue, 0.1f);
				ImGui::TreePop();
			}
			ImGui::Spacing();
		}
	}
}
#endif

D3D12_GPU_DESCRIPTOR_HANDLE RendaringPostprosecess::GetOffscreenSrvHandleGPU() {
	return engineCore_->GetTextureManager()->GetOffscreenSrvHandleGPU(readingResourceIndex_);
}

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
	commandList->SetGraphicsRootConstantBufferView(1, grayScaleOffsetBuffer_.GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void RendaringPostprosecess::ApplyVignette() {
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	commandList->SetGraphicsRootSignature(vignettePso_->GetRootSignature());
	commandList->SetPipelineState(vignettePso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(0, engineCore_->GetTextureManager()->GetOffscreenSrvHandleGPU(readingResourceIndex_));
	commandList->SetGraphicsRootConstantBufferView(1, vignetteOffsetBuffer_.GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void RendaringPostprosecess::ApplyColorCorrection() {
	DirectXCommon* dxCommon = engineCore_->GetDirectXCommon();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	commandList->SetGraphicsRootSignature(colorCorrectionPso_->GetRootSignature());
	commandList->SetPipelineState(colorCorrectionPso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(0, engineCore_->GetTextureManager()->GetOffscreenSrvHandleGPU(readingResourceIndex_));
	commandList->SetGraphicsRootConstantBufferView(1, colorCorrectionOffsetBuffer_.GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
