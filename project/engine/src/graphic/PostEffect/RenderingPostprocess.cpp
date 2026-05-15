#include "engine/include/graphic/PostEffect/RenderingPostprocess.h"

#include "engine/include/utility/DirectX/TransitionResourceBarrier.h"
#include "engine/include/graphic/ShaderBuffer/BufferGenerater/BufferGenerator.h"

#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"

#include <cassert>
#include <algorithm>
#include "engine/include/core/EngineDefines.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/ImGui/ImGuiFlameController.h"
#endif // QFE_OPTIMIZE_OFF

namespace QFE {

	RenderingPostprocess::RenderingPostprocess() {
		isImGuiEnabled_ = false;

		device_ = nullptr;
		list_ = nullptr;

		isPostprocess_ = true;

		postProcessCount_ = 0;
		enableGrayscale_ = true;
		enableColorCorrection_ = true;
		enableVignette_ = true;
		enableNormal_ = true;
		enablePixcel_ = true;

		renderingRosourceIndex_ = 0;
		readingResourceIndex_ = 0;

		isFirstStateRenderTarget_ = false;
		isSecondStateRenderTarget_ = false;

		offScreenClearColor[0] = 0.0f;
		offScreenClearColor[1] = 0.0f;
		offScreenClearColor[2] = 0.0f;
		offScreenClearColor[3] = 0.0f;

		postProcessFunctions_.clear();
		postProcessFunctions_.push_back(std::bind(&RenderingPostprocess::ApplyGrayScale, this));
		postProcessFunctions_.push_back(std::bind(&RenderingPostprocess::ApplyVignette, this));
		postProcessFunctions_.push_back(std::bind(&RenderingPostprocess::ApplyColorCorrection, this));
		postProcessFunctions_.push_back(std::bind(&RenderingPostprocess::ApplyPixcel, this));

		grayScaleProcessIndex_ = 0;
		vignetteProcessIndex_ = 1;
		colorCorrectionProcessIndex_ = 2;
		pixcelProcessIndex_ = 3;

		postProcessOrderForm_.clear();

		grayScaleOffset_ = 0.0f;

#ifdef QFE_OPTIMIZE_OFF
		isImGuiEnabled_ = true;
#endif // QFE_OPTIMIZE_OFF
	}

	void RenderingPostprocess::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* list) {
		device_ = device;
		list_ = list;
		dxCommon_ = DirectXCommon::GetInstance();

		assert(device_);
		assert(list_);

		// 頂点バッファ生成
		vertexResource_ = BufferGenerator::Generate(device, sizeof(VertexData) * 4);
		vertexBufferView_ = {};
		vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
		vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
		vertexBufferView_.StrideInBytes = sizeof(VertexData);

		// 頂点データ設定
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

		// インデックスバッファ生成
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

	void RenderingPostprocess::SetColorCorrectionPSO(PipelineStateObject* pso) {
		colorCorrectionPso_ = pso;
		colorCorrectionOffsetBuffer_.CreateResource(device_);
		colorCorrectionOffsetBuffer_.GetData()->exposure = 0.0f;
		colorCorrectionOffsetBuffer_.GetData()->contrast = 1.0f;
		colorCorrectionOffsetBuffer_.GetData()->saturation = 1.0f;
		colorCorrectionOffsetBuffer_.GetData()->gamma = 1.0f;
		colorCorrectionOffsetBuffer_.GetData()->hue = 0.0f;
	}

	void RenderingPostprocess::SetGrayScalePSO(PipelineStateObject* pso) {
		grayScalePso_ = pso;
		grayScaleOffsetBuffer_.CreateResource(device_);
	}

	void RenderingPostprocess::SetVignettePSO(PipelineStateObject* pso) {
		vignettePso_ = pso;
		vignetteOffsetBuffer_.CreateResource(device_);
		vignetteOffsetBuffer_.GetData()->VignetteRadius = 0.3f;
		vignetteOffsetBuffer_.GetData()->VignetteSoftness = 0.5f;
		vignetteOffsetBuffer_.GetData()->VignetteIntensity = 0.2f;
	}

	void RenderingPostprocess::SetNormalPSO(PipelineStateObject* pso) {
		assert(pso);
		normalPso_ = pso;
	}

	void RenderingPostprocess::SetPixelPSO(PipelineStateObject* pso) {
		assert(pso);
		pixcelPso_ = pso;
		pixcelOffsetBuffer_.CreateResource(device_);
		pixcelOffsetBuffer_.GetData()->pixcelSize = 5;
		pixcelOffsetBuffer_.GetData()->screenResolution.x = static_cast<float>(EngineGlobalValue::windowWidth);
		pixcelOffsetBuffer_.GetData()->screenResolution.y = static_cast<float>(EngineGlobalValue::windowHeight);
	}

	void RenderingPostprocess::SetOffscreenResource(ID3D12Resource* firstResource, ID3D12Resource* secondResource) {
		assert(firstResource);
		assert(secondResource);

		offScreenResources_[0] = firstResource;
		offScreenResources_[1] = secondResource;
	}

	void RenderingPostprocess::SetOffscreenRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE firstHandle, D3D12_CPU_DESCRIPTOR_HANDLE secondHandle) {
		offScreenRtvHandles_.at(0) = firstHandle;
		offScreenRtvHandles_.at(1) = secondHandle;
		assert(offScreenRtvHandles_.at(0).ptr != 0);
		assert(offScreenRtvHandles_.at(1).ptr != 0);
	}

	void RenderingPostprocess::SetOffscreenSrvHandle(DescriptorHandles firstHandle, DescriptorHandles secondHandle) {
		offScreenSrvHandles_.at(0) = firstHandle;
		offScreenSrvHandles_.at(1) = secondHandle;
		assert(offScreenSrvHandles_.at(0).cpuHandle_.ptr != 0);
		assert(offScreenSrvHandles_.at(0).gpuHandle_.ptr != 0);
		assert(offScreenSrvHandles_.at(1).cpuHandle_.ptr != 0);
		assert(offScreenSrvHandles_.at(1).gpuHandle_.ptr != 0);
	}

	void RenderingPostprocess::SetDsvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
		dsvHandle_ = handle;
		assert(dsvHandle_.ptr != 0);
	}

	void RenderingPostprocess::SetBackBufferRtvHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
		backBufferRtvHandle_ = handle;
		assert(backBufferRtvHandle_.ptr != 0);
	}

	DescriptorHandles RenderingPostprocess::GetCurrentSrvHandle() const {
		return offScreenSrvHandles_.at(readingResourceIndex_);
	}

	void RenderingPostprocess::PreDraw() {
		postProcessCount_ = 0;
		postProcessOrderForm_.clear();

		if (enablePixcel_) {
			postProcessOrderForm_.push_back(pixcelProcessIndex_);
			postProcessCount_++;

			pixcelOffsetBuffer_.GetData()->time += EngineGlobalValue::deltaTime;
		}

		if (enableGrayscale_) {
			postProcessOrderForm_.push_back(grayScaleProcessIndex_);
			postProcessCount_++;

			grayScaleOffsetBuffer_.GetData()->offset.x = grayScaleOffset_;
		}

		if (enableVignette_) {
			postProcessOrderForm_.push_back(vignetteProcessIndex_);
			postProcessCount_++;
		}

		if (enableColorCorrection_) {
			postProcessOrderForm_.push_back(colorCorrectionProcessIndex_);
			postProcessCount_++;
		}

		if (postProcessCount_ > 0) {
			grayScaleProcessIndex_ = std::clamp(grayScaleProcessIndex_, 0, static_cast<int>(postProcessCount_) - 1);
			vignetteProcessIndex_ = std::clamp(vignetteProcessIndex_, 0, static_cast<int>(postProcessCount_) - 1);
		}

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

		// 初期レンダーターゲット設定
		if (isPostprocess_ || isImGuiEnabled_) {
			renderingRosourceIndex_ = 0;
			list_->OMSetRenderTargets(1, &offScreenRtvHandles_[renderingRosourceIndex_], false, &dsvHandle_);

			ClearFirstRenderTarget();
			ClearSecondRenderTarget();
		}
		else {
			list_->OMSetRenderTargets(1, &backBufferRtvHandle_, false, &dsvHandle_);
		}
	}

	void RenderingPostprocess::PostDraw() {
		if (!isPostprocess_) {
			return;
		}
		SwitchRenderTarget();
		for (uint32_t i = 0; i < postProcessCount_; i++) {
			postProcessFunctions_[postProcessOrderForm_[i]]();
			SwitchRenderTarget();
		}

		readingResourceIndex_ = postProcessCount_ % 2;

		list_->RSSetViewports(1, dxCommon_->GetViewPort());
		list_->RSSetScissorRects(1, dxCommon_->GetScissorRect());
		list_->OMSetRenderTargets(1, &backBufferRtvHandle_, false, &dsvHandle_);
#ifdef QFE_OPTIMIZE_OFF
		if (isImGuiEnabled_) {
			return;
		}
#endif // QFE_OPTIMIZE_OFF
		list_->SetGraphicsRootSignature(normalPso_->GetRootSignature());
		list_->SetPipelineState(normalPso_->GetPipelineState());
		list_->IASetVertexBuffers(0, 1, &vertexBufferView_);
		list_->IASetIndexBuffer(&indexBufferView_);
		list_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		list_->SetGraphicsRootDescriptorTable(0, offScreenSrvHandles_[readingResourceIndex_].gpuHandle_);
		list_->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}

#ifdef QFE_OPTIMIZE_OFF
	void RenderingPostprocess::DrawImGui() {
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


	void RenderingPostprocess::ClearFirstRenderTarget() {
		list_->ClearRenderTargetView(offScreenRtvHandles_.at(0), offScreenClearColor, 0, nullptr);
	}

	void RenderingPostprocess::ClearSecondRenderTarget() {
		list_->ClearRenderTargetView(offScreenRtvHandles_.at(1), offScreenClearColor, 0, nullptr);
	}

	void RenderingPostprocess::SwitchRenderTarget() {
		if (!isPostprocess_) {
			return;
		}

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

		}
		else {
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

		DirectXCommon::GetInstance()->ClearDepthStencil();
		list_->OMSetRenderTargets(1, &offScreenRtvHandles_[renderingRosourceIndex_], false, &dsvHandle_);
	}

	void RenderingPostprocess::ApplyGrayScale() {
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

	void RenderingPostprocess::ApplyVignette() {
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

	void RenderingPostprocess::ApplyColorCorrection() {
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

	void RenderingPostprocess::ApplyPixcel() {
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

}
