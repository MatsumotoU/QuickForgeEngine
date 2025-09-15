#include "pch.h"

#include "RendaringPostprocess.h"
#include "AppUtility/DirectX/TransitionResourceBarrier.h"
#include "Graphic/ShaderBuffer/BufferGenerater/BufferGenerator.h"

#include "Graphic/DirectXCommon/DirectXCommon.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include "AppUtility/DebugTool/ImGui/FrameController/ImGuiFlameController.h"
#endif // _DEBUG

RendaringPostprosecess::RendaringPostprosecess() {
	isImGuiEnabled_ = false;
#ifdef _DEBUG
	isImGuiEnabled_ = true; // デバッグモードではImGuiを有効にする
#endif // _DEBUG

	device_ = nullptr;
	list_ = nullptr;

	isPostprocess_ = true;

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

void RendaringPostprosecess::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* list) {
	device_ = device;
	list_ = list;
	dxCommon_ = DirectXCommon::GetInstance();

	assert(device_);
	assert(list_);

	// Spriteを作る
	vertexResource_ = BufferGenerator::Generate(device, sizeof(VertexData) * 4);
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
	indexResource_ = BufferGenerator::Generate(device, sizeof(uint32_t) * 6);
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
}

void RendaringPostprosecess::SetColorCorrectionPSO(PipelineStateObject* pso) {
	colorCorrectionPso_ = pso;
	colorCorrectionOffsetBuffer_.CreateResource(device_);
	colorCorrectionOffsetBuffer_.GetData()->exposure = 0.0f; // 露出
	colorCorrectionOffsetBuffer_.GetData()->contrast = 1.0f; // コントラスト
	colorCorrectionOffsetBuffer_.GetData()->saturation = 1.0f; // 彩度
	colorCorrectionOffsetBuffer_.GetData()->gamma = 1.0f; // ガンマ
	colorCorrectionOffsetBuffer_.GetData()->hue = 0.0f;
}

void RendaringPostprosecess::SetGrayScalePSO(PipelineStateObject* pso) {
	grayScalePso_ = pso;
	grayScaleOffsetBuffer_.CreateResource(device_);
}

void RendaringPostprosecess::SetVignettePSO(PipelineStateObject* pso) {
	vignettePso_ = pso;
	vignetteOffsetBuffer_.CreateResource(device_);
	vignetteOffsetBuffer_.GetData()->VignetteRadius = 0.3f; // ビネットの半径
	vignetteOffsetBuffer_.GetData()->VignetteSoftness = 0.5f; // ビネットの柔らかさ
	vignetteOffsetBuffer_.GetData()->VignetteIntensity = 0.2f; // ビネットの強さ
}

void RendaringPostprosecess::SetNormalPSO(PipelineStateObject* pso) {
	assert(pso);
	normalPso_ = pso;
}

void RendaringPostprosecess::SetOffscreenResource(ID3D12Resource* firstResource, ID3D12Resource* secondResource) {
	assert(firstResource);
	assert(secondResource);

	offScreenResources_[0] = firstResource;
	offScreenResources_[1] = secondResource;
}

void RendaringPostprosecess::SetOffscreenRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE firstHandle, D3D12_CPU_DESCRIPTOR_HANDLE secondHandle) {
	offScreenRtvHandles_.at(0) = firstHandle;
	offScreenRtvHandles_.at(1) = secondHandle;
	assert(offScreenRtvHandles_.at(0).ptr != 0);
	assert(offScreenRtvHandles_.at(1).ptr != 0);
}

void RendaringPostprosecess::SetOffscreenSrvHandle(DescriptorHandles firstHandle, DescriptorHandles secondHandle) {
	offScreenSrvHandles_.at(0) = firstHandle;
	offScreenSrvHandles_.at(1) = secondHandle;
	assert(offScreenSrvHandles_.at(0).cpuHandle_.ptr != 0);
	assert(offScreenSrvHandles_.at(0).gpuHandle_.ptr != 0);
	assert(offScreenSrvHandles_.at(1).cpuHandle_.ptr != 0);
	assert(offScreenSrvHandles_.at(1).gpuHandle_.ptr != 0);
}

void RendaringPostprosecess::SetDsvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	dsvHandle_ = handle;
	assert(dsvHandle_.ptr != 0);
}

void RendaringPostprosecess::SetBackBufferRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
	backBufferRtvHandle_ = handle;
	assert(backBufferRtvHandle_.ptr != 0);
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
	if (!isFirstStateRenderTarget_) {
		TransitionResourceBarrier::Transition(
			list_, offScreenResources_[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		isFirstStateRenderTarget_ = true;
	}
	if (!isSecondStateRenderTarget_) {
		TransitionResourceBarrier::Transition(
			list_, offScreenResources_[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		isSecondStateRenderTarget_ = true;
	}

	// ImGuiのレンダリング用に絶対にバックバッファに描画する必要があるので残す
	if (isPostprocess_ || isImGuiEnabled_) {
		// オフスクリーンに描画
		renderingRosourceIndex_ = 0;
		list_->OMSetRenderTargets(1,&offScreenRtvHandles_[renderingRosourceIndex_], false, &dsvHandle_);

		// オフスクリーンのクリア
		ClearFirstRenderTarget();
		// 2つ目のオフスクリーンのクリア
		ClearSecondRenderTarget();
	} else {
		// バックバッファに描画
		list_->OMSetRenderTargets(1, &backBufferRtvHandle_, false, &dsvHandle_);
	}
}

void RendaringPostprosecess::PostDraw() {
	// ポストプロセスが有効でないなら何もしない
	if (!isPostprocess_) {
		return;
	} 
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
	list_->RSSetViewports(1, dxCommon_->GetViewPort());
	list_->RSSetScissorRects(1, dxCommon_->GetScissorRect());
	list_->OMSetRenderTargets(1, &backBufferRtvHandle_, false, &dsvHandle_);
#ifdef _DEBUG
	if (isImGuiEnabled_) {
		return;
	}
#endif // _DEBUG
	list_->SetGraphicsRootSignature(normalPso_->GetRootSignature());
	list_->SetPipelineState(normalPso_->GetPipelineState());
	list_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	list_->IASetIndexBuffer(&indexBufferView_);
	list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list_->SetGraphicsRootDescriptorTable(0, offScreenSrvHandles_[readingResourceIndex_].gpuHandle_);
	list_->DrawIndexedInstanced(6, 1, 0, 0, 0);
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


void RendaringPostprosecess::ClearFirstRenderTarget() {
	list_->ClearRenderTargetView(offScreenRtvHandles_.at(0), offScreenClearColor, 0, nullptr);
}

void RendaringPostprosecess::ClearSecondRenderTarget() {
	list_->ClearRenderTargetView(offScreenRtvHandles_.at(1), offScreenClearColor, 0, nullptr);
}

void RendaringPostprosecess::SwitchRenderTarget() {
	// ポストプロセスが有効でないなら何もしない
	if (!isPostprocess_) {
		return;
	}

	// オフスクリーンのバリア
	if (renderingRosourceIndex_ == 0) {
		if (isFirstStateRenderTarget_) {
			TransitionResourceBarrier::Transition(
				list_, offScreenResources_[0], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			isFirstStateRenderTarget_ = false;
		}
		if (!isSecondStateRenderTarget_) {
			TransitionResourceBarrier::Transition(
				list_, offScreenResources_[1], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			isSecondStateRenderTarget_ = true;
		}
		ClearSecondRenderTarget();

	} else {
		if (!isFirstStateRenderTarget_) {
			TransitionResourceBarrier::Transition(
				list_, offScreenResources_[0], D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			isFirstStateRenderTarget_ = true;
		}
		if (isSecondStateRenderTarget_) {
			TransitionResourceBarrier::Transition(
				list_, offScreenResources_[1], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			isSecondStateRenderTarget_ = false;
		}
		ClearFirstRenderTarget();
	}
	readingResourceIndex_ = renderingRosourceIndex_;
	renderingRosourceIndex_ = (renderingRosourceIndex_ + 1) % 2;

	// 描画先の設定
	list_->OMSetRenderTargets(1, &offScreenRtvHandles_[renderingRosourceIndex_], false, &dsvHandle_);
}

void RendaringPostprosecess::ApplyGrayScale() {
	list_->RSSetViewports(1, dxCommon_->GetViewPort());
	list_->RSSetScissorRects(1, dxCommon_->GetScissorRect());
	list_->SetGraphicsRootSignature(grayScalePso_->GetRootSignature());
	list_->SetPipelineState(grayScalePso_->GetPipelineState());
	list_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	list_->IASetIndexBuffer(&indexBufferView_);
	list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list_->SetGraphicsRootDescriptorTable(0, offScreenSrvHandles_.at(readingResourceIndex_).gpuHandle_);
	list_->SetGraphicsRootConstantBufferView(1, grayScaleOffsetBuffer_.GetGPUVirtualAddress());
	list_->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void RendaringPostprosecess::ApplyVignette() {
	list_->RSSetViewports(1, dxCommon_->GetViewPort());
	list_->RSSetScissorRects(1, dxCommon_->GetScissorRect());
	list_->SetGraphicsRootSignature(vignettePso_->GetRootSignature());
	list_->SetPipelineState(vignettePso_->GetPipelineState());
	list_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	list_->IASetIndexBuffer(&indexBufferView_);
	list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list_->SetGraphicsRootDescriptorTable(0, offScreenSrvHandles_.at(readingResourceIndex_).gpuHandle_);
	list_->SetGraphicsRootConstantBufferView(1, vignetteOffsetBuffer_.GetGPUVirtualAddress());
	list_->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void RendaringPostprosecess::ApplyColorCorrection() {
	list_->RSSetViewports(1, dxCommon_->GetViewPort());
	list_->RSSetScissorRects(1, dxCommon_->GetScissorRect());
	list_->SetGraphicsRootSignature(colorCorrectionPso_->GetRootSignature());
	list_->SetPipelineState(colorCorrectionPso_->GetPipelineState());
	list_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	list_->IASetIndexBuffer(&indexBufferView_);
	list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list_->SetGraphicsRootDescriptorTable(0, offScreenSrvHandles_.at(readingResourceIndex_).gpuHandle_);
	list_->SetGraphicsRootConstantBufferView(1, colorCorrectionOffsetBuffer_.GetGPUVirtualAddress());
	list_->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
