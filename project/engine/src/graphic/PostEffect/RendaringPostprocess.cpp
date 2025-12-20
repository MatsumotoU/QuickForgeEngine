#include "engine/include/graphic/PostEffect/RendaringPostprocess.h"

#include "engine/include/utility/DirectX/TransitionResourceBarrier.h"
#include "engine/include/graphic/ShaderBuffer/BufferGenerater/BufferGenerator.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"

#include <cassert>
#include <algorithm>

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include "engine/include/utility/DebugTool/ImGui/ImGuiFlameController.h"
#endif // _DEBUG

RendaringPostprosecess::RendaringPostprosecess() {
	isImGuiEnabled_ = false;

	device_ = nullptr;
	list_ = nullptr;

	isPostprocess_ = true;

	postProcessCount_ = 0;
	enableGrayscale_ = true;
	enableColorCorrection_ = true;
	enableVignette_ = true;
	enableNormal_ = true;
	enablePixcel_ = false;

	renderingRosourceIndex_ = 0;
	readingResourceIndex_ = 0;

	isFirstStateRenderTarget_ = false;
	isSecondStateRenderTarget_ = false;

	offScreenClearColor[0] = 0.0f;
	offScreenClearColor[1] = 0.0f;
	offScreenClearColor[2] = 0.0f;
	offScreenClearColor[3] = 0.0f;

	// 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺ｮ髢｢謨ｰ繧堤匳骭ｲ
	postProcessFunctions_.clear();
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyGrayScale, this));
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyVignette, this));
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyColorCorrection, this));
	postProcessFunctions_.push_back(std::bind(&RendaringPostprosecess::ApplyPixcel, this));
	// 蝗ｺ螳壹・繧､繝ｳ繝・ャ繧ｯ繧ｹ繧定ｨｭ螳・
	grayScaleProcessIndex_ = 0; // 繧ｰ繝ｬ繝ｼ繧ｹ繧ｱ繝ｼ繝ｫ縺ｮ繧､繝ｳ繝・ャ繧ｯ繧ｹ
	vignetteProcessIndex_ = 1; // 繝薙ロ繝・ヨ縺ｮ繧､繝ｳ繝・ャ繧ｯ繧ｹ
	colorCorrectionProcessIndex_ = 2; // 濶ｲ隱ｿ陬懈ｭ｣縺ｮ繧､繝ｳ繝・ャ繧ｯ繧ｹ
	pixcelProcessIndex_ = 3; // 繝斐け繧ｻ繝ｫ蛹悶・繧､繝ｳ繝・ャ繧ｯ繧ｹ

	postProcessOrderForm_.clear();

	grayScaleOffset_ = 0.0f;

#ifdef _DEBUG
	isImGuiEnabled_ = true; // 繝・ヰ繝・げ繝｢繝ｼ繝峨〒縺ｯImGui繧呈怏蜉ｹ縺ｫ縺吶ｋ
#endif // _DEBUG
}

void RendaringPostprosecess::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* list) {
	device_ = device;
	list_ = list;
	dxCommon_ = DirectXCommon::GetInstance();

	assert(device_);
	assert(list_);

	// Sprite繧剃ｽ懊ｋ
	vertexResource_ = BufferGenerator::Generate(device, sizeof(VertexData) * 4);
	vertexBufferView_ = {};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 鬆らせ繝・・繧ｿ菴懈・
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

	// indexBuffer縺ｮ菴懈・
	indexResource_ = BufferGenerator::Generate(device, sizeof(uint32_t) * 6);
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
}

void RendaringPostprosecess::SetColorCorrectionPSO(PipelineStateObject* pso) {
	colorCorrectionPso_ = pso;
	colorCorrectionOffsetBuffer_.CreateResource(device_);
	colorCorrectionOffsetBuffer_.GetData()->exposure = 0.0f; // 髴ｲ蜃ｺ
	colorCorrectionOffsetBuffer_.GetData()->contrast = 1.0f; // 繧ｳ繝ｳ繝医Λ繧ｹ繝・
	colorCorrectionOffsetBuffer_.GetData()->saturation = 1.0f; // 蠖ｩ蠎ｦ
	colorCorrectionOffsetBuffer_.GetData()->gamma = 1.0f; // 繧ｬ繝ｳ繝・
	colorCorrectionOffsetBuffer_.GetData()->hue = 0.0f;
}

void RendaringPostprosecess::SetGrayScalePSO(PipelineStateObject* pso) {
	grayScalePso_ = pso;
	grayScaleOffsetBuffer_.CreateResource(device_);
}

void RendaringPostprosecess::SetVignettePSO(PipelineStateObject* pso) {
	vignettePso_ = pso;
	vignetteOffsetBuffer_.CreateResource(device_);
	vignetteOffsetBuffer_.GetData()->VignetteRadius = 0.3f; // 繝薙ロ繝・ヨ縺ｮ蜊雁ｾ・
	vignetteOffsetBuffer_.GetData()->VignetteSoftness = 0.5f; // 繝薙ロ繝・ヨ縺ｮ譟斐ｉ縺九＆
	vignetteOffsetBuffer_.GetData()->VignetteIntensity = 0.2f; // 繝薙ロ繝・ヨ縺ｮ蠑ｷ縺・
}

void RendaringPostprosecess::SetNormalPSO(PipelineStateObject* pso) {
	assert(pso);
	normalPso_ = pso;
}

void RendaringPostprosecess::SetPixcelPSO(PipelineStateObject* pso) {
	assert(pso);
	pixcelPso_ = pso;
	pixcelOffsetBuffer_.CreateResource(device_);
	pixcelOffsetBuffer_.GetData()->pixcelSize = 5; // 繝斐け繧ｻ繝ｫ縺ｮ螟ｧ縺阪＆
	pixcelOffsetBuffer_.GetData()->screenResolution.x = static_cast<float>(QFE::EngineGlobalValue::windowWidth);
	pixcelOffsetBuffer_.GetData()->screenResolution.y = static_cast<float>(QFE::EngineGlobalValue::windowHeight);
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

DescriptorHandles RendaringPostprosecess::GetCurrentSrvHandle() const  {
	return offScreenSrvHandles_.at(readingResourceIndex_); 
}

void RendaringPostprosecess::PreDraw() {
	// 菴募屓繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺後°縺九▲縺ｦ縺・ｋ縺玖ｪｿ縺ｹ繧・
	postProcessCount_ = 0;
	postProcessOrderForm_.clear();

	// 繝斐け繧ｻ繝ｫ蛹・
	if (enablePixcel_) {
		postProcessOrderForm_.push_back(pixcelProcessIndex_); // 繝斐け繧ｻ繝ｫ蛹・
		postProcessCount_++;

		pixcelOffsetBuffer_.GetData()->time += QFE::EngineGlobalValue::deltaTime;
	}
	// 繧ｰ繝ｬ繝ｼ繧ｹ繧ｱ繝ｼ繝ｫ
	if (enableGrayscale_) {
		postProcessOrderForm_.push_back(grayScaleProcessIndex_); // 繧ｰ繝ｬ繝ｼ繧ｹ繧ｱ繝ｼ繝ｫ
		postProcessCount_++;
		// 繧ｰ繝ｬ繝ｼ繧ｹ繧ｱ繝ｼ繝ｫ縺ｮ蠑ｷ蠎ｦ
		grayScaleOffsetBuffer_.GetData()->offset.x = grayScaleOffset_;
	}
	// 繝薙ロ繝・ヨ
	if (enableVignette_) {
		postProcessOrderForm_.push_back(vignetteProcessIndex_); // 繝薙ロ繝・ヨ
		postProcessCount_++;
	}
	// 濶ｲ隱ｿ陬懈ｭ｣
	if (enableColorCorrection_) {
		postProcessOrderForm_.push_back(colorCorrectionProcessIndex_); // 濶ｲ隱ｿ陬懈ｭ｣
		postProcessCount_++;
	}

	// 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺ｮ鬆・分繧剃ｸｦ縺ｳ譖ｿ縺医ｋ
	if (postProcessCount_ > 0) {
		grayScaleProcessIndex_ = std::clamp(grayScaleProcessIndex_, 0, static_cast<int>(postProcessCount_) - 1);
		vignetteProcessIndex_ = std::clamp(vignetteProcessIndex_, 0, static_cast<int>(postProcessCount_) - 1);
	}

	// 繧ｪ繝輔せ繧ｯ繝ｪ繝ｼ繝ｳ縺ｮ繝舌Μ繧｢繧定ｨｭ螳・
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

	// ImGui縺ｮ繝ｬ繝ｳ繝繝ｪ繝ｳ繧ｰ逕ｨ縺ｫ邨ｶ蟇ｾ縺ｫ繝舌ャ繧ｯ繝舌ャ繝輔ぃ縺ｫ謠冗判縺吶ｋ蠢・ｦ√′縺ゅｋ縺ｮ縺ｧ谿九☆
	if (isPostprocess_ || isImGuiEnabled_) {
		// 繧ｪ繝輔せ繧ｯ繝ｪ繝ｼ繝ｳ縺ｫ謠冗判
		renderingRosourceIndex_ = 0;
		list_->OMSetRenderTargets(1,&offScreenRtvHandles_[renderingRosourceIndex_], false, &dsvHandle_);

		// 繧ｪ繝輔せ繧ｯ繝ｪ繝ｼ繝ｳ縺ｮ繧ｯ繝ｪ繧｢
		ClearFirstRenderTarget();
		// 2縺､逶ｮ縺ｮ繧ｪ繝輔せ繧ｯ繝ｪ繝ｼ繝ｳ縺ｮ繧ｯ繝ｪ繧｢
		ClearSecondRenderTarget();
	} else {
		// 繝舌ャ繧ｯ繝舌ャ繝輔ぃ縺ｫ謠冗判
		list_->OMSetRenderTargets(1, &backBufferRtvHandle_, false, &dsvHandle_);
	}
}

void RendaringPostprosecess::PostDraw() {
	// 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺梧怏蜉ｹ縺ｧ縺ｪ縺・↑繧我ｽ輔ｂ縺励↑縺・
	if (!isPostprocess_) {
		return;
	} 
	// 繧ｪ繝輔せ繧ｯ繝ｪ繝ｼ繝ｳ縺ｮ繝舌Μ繧｢
	SwitchRenderTarget();
	for (uint32_t i = 0; i < postProcessCount_; i++) {
		// 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺ｮ驕ｩ逕ｨ
		postProcessFunctions_[postProcessOrderForm_[i]]();
		SwitchRenderTarget();
	}

	// 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺ｮ驕ｩ逕ｨ
	readingResourceIndex_ = postProcessCount_ % 2;

	// 繝舌ャ繧ｯ繝舌ャ繝輔ぃ縺ｫ譖ｸ縺崎ｾｼ縺ｿ
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
	// 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺ｮ繝・ヰ繝・げ繧ｦ繧｣繝ｳ繝峨え繧定｡ｨ遉ｺ
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
	// 繝昴せ繝医・繝ｭ繧ｻ繧ｹ縺梧怏蜉ｹ縺ｧ縺ｪ縺・↑繧我ｽ輔ｂ縺励↑縺・
	if (!isPostprocess_) {
		return;
	}

	// 繧ｪ繝輔せ繧ｯ繝ｪ繝ｼ繝ｳ縺ｮ繝舌Μ繧｢
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

	// 謠冗判蜈医・險ｭ螳・
	DirectXCommon::GetInstance()->ClearDepthStencil();
	list_->OMSetRenderTargets(1, &offScreenRtvHandles_[renderingRosourceIndex_], false,&dsvHandle_);
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

void RendaringPostprosecess::ApplyPixcel() {
	list_->RSSetViewports(1, dxCommon_->GetViewPort());
	list_->RSSetScissorRects(1, dxCommon_->GetScissorRect());
	list_->SetGraphicsRootSignature(pixcelPso_->GetRootSignature());
	list_->SetPipelineState(pixcelPso_->GetPipelineState());
	list_->IASetVertexBuffers(0, 1, &vertexBufferView_);
	list_->IASetIndexBuffer(&indexBufferView_);
	list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list_->SetGraphicsRootDescriptorTable(0, offScreenSrvHandles_.at(readingResourceIndex_).gpuHandle_);
	list_->SetGraphicsRootConstantBufferView(1, pixcelOffsetBuffer_.GetGPUVirtualAddress());
	list_->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
