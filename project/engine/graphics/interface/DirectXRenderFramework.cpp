#include "DirectXRenderFramework.h"

#include "graphics/D3D12GraphicEngine.h"
#include "graphics/dx12/pipeline/GraphicPipelineManager.h"
#include "graphics/dx12/pipeline/RaytracingPipelineManager.h"
#include "graphics/dx12/pipeline/ComputePipelineManager.h"
#include "graphics/dx12/vram/DirectXResourceAllocator.h"
#include "graphics/dx12/vram/resources/DirectXResourceContainer.h"
#include "graphics/dx12/TextureLoader.h"
#include "graphics/dx12/DirectXDevice.h"
#include "graphics/dx12/command/DirectXCommandManager.h"
#include "graphics/dx12/RenderPass.h"
#include "graphics/dx12/pipeline/rtpso/RaytracingAccelerationStructure.h"
#include "graphics/dx12/Fence.h"

#include "graphics/dx12/pipeline/rtpso/TLAS.h"

#include "core/math/transform/Transform.h"
#include "resources/shaders/shaderStructs/hlslTypeToCpp.h"

#include "DirectXResourceFramework.h"

namespace {
	[[nodiscard]] QFE::GRAPHIC::D3D12GraphicEngine* ResolveD3D12GraphicEngine(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine) {
		auto* d3d12GraphicEngine =
			dynamic_cast<QFE::GRAPHIC::D3D12GraphicEngine*>(graphicEngine);
		if (d3d12GraphicEngine == nullptr || !d3d12GraphicEngine->IsCoreSystemAvailable()) {
			QFE_LOG("A DirectX 12 graphic engine with an initialized core system is required.");
			return nullptr;
		}
		return d3d12GraphicEngine;
	}
}

bool QFE::GRAPHIC::CreateShaderPair(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const std::string& vsDirName, const std::string& psDirName, const std::string& vsFileName, const std::string& psFileName, QFE::GRAPHIC::ShaderPairHandle& outShaderPairHandle) {
	// シェーダーペア生成の設定
	QFE::GRAPHIC::ShaderPairElement shaderPairElement;
	shaderPairElement.vsDirName = vsDirName;
	shaderPairElement.psDirName = psDirName;
	shaderPairElement.vsFileName = vsFileName;
	shaderPairElement.psFileName = psFileName;

	// シェーダーペアを生成
	QFE::GRAPHIC::GraphicPipelineManager* pipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetGraphicPipelineManager();
	QFE::GRAPHIC::ShaderPairHandle shaderPairHandle = pipelineManager->GenerateShaderPair(shaderPairElement);
	outShaderPairHandle = shaderPairHandle;

	// シェーダーペアの生成に成功したかを返す
	return outShaderPairHandle != QFE::GRAPHIC::ShaderPairHandle::Invalid;
}

bool QFE::GRAPHIC::CreateGraphicPSO(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::ShaderPairHandle& shaderPairHandle,
	const QFE::GRAPHIC::RasterizerType& rasterizerType, const QFE::GRAPHIC::BlendMode& blendMode,
	const QFE::GRAPHIC::DepthStencilDescType& depthStencilDescType, QFE::GRAPHIC::PSOHandle& outPSOHandle) {
	return CreateGraphicPSO(
		graphicEngine, shaderPairHandle, rasterizerType, blendMode, depthStencilDescType,
		DXGI_FORMAT_R16G16B16A16_FLOAT, outPSOHandle);
}

bool QFE::GRAPHIC::CreateGraphicPSO(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::ShaderPairHandle& shaderPairHandle,
	const QFE::GRAPHIC::RasterizerType& rasterizerType, const QFE::GRAPHIC::BlendMode& blendMode,
	const QFE::GRAPHIC::DepthStencilDescType& depthStencilDescType, DXGI_FORMAT renderTargetFormat,
	QFE::GRAPHIC::PSOHandle& outPSOHandle) {

	// 使用するパイプラインマネージャを取得
	QFE::GRAPHIC::GraphicPipelineManager* pipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetGraphicPipelineManager();
	// パイプラインステートオブジェクトを生成
	QFE::GRAPHIC::PSOHandle psoHandle = pipelineManager->GeneratePipelineStateObject(
		ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXDevice()->GetDevice(),
		shaderPairHandle,
		blendMode,
		rasterizerType,
		depthStencilDescType,
		renderTargetFormat
	);
	// 生成したPSOハンドルを出力引数に設定
	outPSOHandle = psoHandle;
	if (psoHandle == QFE::GRAPHIC::PSOHandle::Invalid) {
		QFE_LOG("Failed to create PSO.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::GetGraphicPSORootParameterTypeList(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::PSOHandle& psoHandle,
	std::vector<D3D12_ROOT_PARAMETER_TYPE>& outRootParameterTypeList) {


	// 使用するパイプラインマネージャを取得
	QFE::GRAPHIC::GraphicPipelineManager* pipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetGraphicPipelineManager();

	// PSOハンドルからルートパラメータのタイプを取得
	outRootParameterTypeList = pipelineManager->GetRootParameterTypes(psoHandle);
	// 成否の確認
	if (outRootParameterTypeList.empty()) {
		QFE_LOG("Failed to get root parameter type list.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::CreateRayTracingPSO(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::RTPSOHandle& outPSOHandle,
	const std::string& dirPath, const std::string& rgsFileName) {

	// 使用するレイトレーシングパイプラインマネージャを取得
	QFE::GRAPHIC::RaytracingPipelineManager* rayTracingPipelineManager_ = ResolveD3D12GraphicEngine(graphicEngine)->GetRayTracingPipelineManager();
	// レイトレーシングパイプラインステートオブジェクトを生成
	outPSOHandle = rayTracingPipelineManager_->CreateRaytracingPipelineStateObject(ConvertString(dirPath + rgsFileName), L"lib_6_3");
	// 成否の確認
	if (outPSOHandle == QFE::GRAPHIC::RTPSOHandle::Invalid) {
		QFE_LOG("Failed to create ray tracing PSO.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::CreateOffScreenRenderTarget(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
	uint32_t width, uint32_t height, DXGI_FORMAT format) {

	// 使用するレンダーパスを取得
	QFE::GRAPHIC::RenderPass* renderPass = ResolveD3D12GraphicEngine(graphicEngine)->GetRenderPass();
	outRenderTargetHandle = renderPass->CreateOffscreenRenderTarget(width, height, format);
	// 成否の確認
	if (outRenderTargetHandle == QFE::GRAPHIC::RenderTargetHandle::Invalid) {
		QFE_LOG("Failed to create offscreen render target.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::CreateEditorSceneTexture(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
	QFE::GRAPHIC::DirectXResourceHandle& outResourceHandle,
	uintptr_t& outGuiTextureId,
	uint32_t width,
	uint32_t height) {
	auto* d3d12GraphicEngine = ResolveD3D12GraphicEngine(graphicEngine);
	if (d3d12GraphicEngine == nullptr) {
		return false;
	}
	if (!QFE::GRAPHIC::CreateOffScreenRenderTarget(
		graphicEngine, outRenderTargetHandle, width, height,
		DXGI_FORMAT_R8G8B8A8_UNORM)) {
		return false;
	}
	if (!QFE::GRAPHIC::GetRenderResourceHandle(
		graphicEngine, outRenderTargetHandle, outResourceHandle)) {
		return false;
	}
	if (!QFE::GRAPHIC::TransitionResourceToState(
		graphicEngine, outResourceHandle,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)) {
		return false;
	}

	const D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle =
		d3d12GraphicEngine->GetDirectXResourceContainer()->GetDescriptorHandleGPU(
			outResourceHandle, D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
	outGuiTextureId = static_cast<uintptr_t>(gpuHandle.ptr);
	return true;
}

bool QFE::GRAPHIC::CreateViewport(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::ViewPortHandle& outViewportHandle, uint32_t width, uint32_t height) {

	// ビューポートコンテナを取得
	QFE::UniqueContainer<D3D12_VIEWPORT>& viewports_ = ResolveD3D12GraphicEngine(graphicEngine)->GetViewports();
	// ビューポートを作成
	D3D12_VIEWPORT viewport{};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	// ビューポートをコンテナに追加し、ハンドルを取得
	uint32_t handle = viewports_.Add(std::to_string(width) + "x" + std::to_string(height), viewport);
	outViewportHandle = static_cast<QFE::GRAPHIC::ViewPortHandle>(handle);
	// 成否の確認
	if (outViewportHandle == QFE::GRAPHIC::ViewPortHandle::Invalid) {
		QFE_LOG("Failed to create viewport.");
		return false;
	}
	return true;
}

bool QFE::GRAPHIC::CreateScissorRect(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, QFE::GRAPHIC::ScissorRectHandle& outScissorRectHandle,
	int32_t left, int32_t top, int32_t right, int32_t bottom) {

	// シザリング矩形コンテナを取得
	QFE::UniqueContainer<D3D12_RECT>& scissorRects_ = ResolveD3D12GraphicEngine(graphicEngine)->GetScissorRects();
	// シザリング矩形を作成
	D3D12_RECT scissorRect{};
	scissorRect.left = left;
	scissorRect.top = top;
	scissorRect.right = right;
	scissorRect.bottom = bottom;
	// シザリング矩形をコンテナに追加し、ハンドルを取得
	uint32_t handle = scissorRects_.Add(std::to_string(left) + "_" + std::to_string(top) + "_" + std::to_string(right) + "_" + std::to_string(bottom), scissorRect);
	outScissorRectHandle = static_cast<QFE::GRAPHIC::ScissorRectHandle>(handle);
	// 成否の確認
	if (outScissorRectHandle == QFE::GRAPHIC::ScissorRectHandle::Invalid) {
		QFE_LOG("Failed to create scissor rect.");
		return false;
	}
	return true;
}


bool QFE::GRAPHIC::SetRenderTarget(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::DirectXResourceHandle& depthStencilBufferHandle,
	const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets) {

	// 使用機能の取得
	QFE::GRAPHIC::RenderPass* renderPass = ResolveD3D12GraphicEngine(graphicEngine)->GetRenderPass();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();

	// レンダーターゲットを設定
	// TODO: ここでレンダーターゲットを設定する際に、レンダーターゲットの数が0の場合は深度ステンシルバッファのみを設定するようにする必要がある
	// TODO: 成否の確認を行う必要がある
	renderPass->SetRenderTarget(
		commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT), depthStencilBufferHandle, renderTargets);

	return true;
}

bool QFE::GRAPHIC::SetRenderTarget(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets) {
	QFE::GRAPHIC::D3D12GraphicEngine* d3d12GraphicEngine =
		ResolveD3D12GraphicEngine(graphicEngine);
	if (d3d12GraphicEngine == nullptr || renderTargets.empty()) {
		return false;
	}

	QFE::GRAPHIC::RenderPass* renderPass =
		d3d12GraphicEngine->GetRenderPass();
	QFE::GRAPHIC::DirectXCommandManager* commandManager =
		d3d12GraphicEngine->GetDirectXCommandManager();
	if (renderPass == nullptr || commandManager == nullptr) {
		return false;
	}

	renderPass->SetRenderTarget(
		commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT),
		renderTargets);
	return true;
}

bool QFE::GRAPHIC::DrawGraphicPSO(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::PSOHandle& psoHandle,
	const QFE::GRAPHIC::ViewPortHandle& viewportHandle, const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle, const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
	const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();
	QFE::GRAPHIC::RenderPass* renderPass = ResolveD3D12GraphicEngine(graphicEngine)->GetRenderPass();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();
	QFE::GRAPHIC::GraphicPipelineManager* graphicPipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetGraphicPipelineManager();
	QFE::UniqueContainer<D3D12_VIEWPORT>& viewports = ResolveD3D12GraphicEngine(graphicEngine)->GetViewports();
	QFE::UniqueContainer<D3D12_RECT>& scissorRects = ResolveD3D12GraphicEngine(graphicEngine)->GetScissorRects();

	QFE::GRAPHIC::DirectXResourceHandle depthStencilBufferHandle = ResolveD3D12GraphicEngine(graphicEngine)->GetDepthStencilBufferHandle();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	renderPass->SetRenderTarget(
		commandList, depthStencilBufferHandle, renderTargets);

	commandList->RSSetViewports(1, viewports.GetData(static_cast<uint32_t>(viewportHandle)));
	commandList->RSSetScissorRects(1, scissorRects.GetData(static_cast<uint32_t>(scissorRectHandle)));

	commandList->SetPipelineState(graphicPipelineManager->GetPipelineState(psoHandle));
	commandList->SetGraphicsRootSignature(graphicPipelineManager->GetRootSignature(psoHandle));

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = resourceContainer->GetVertexBufferView(vertexBufferHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	std::vector<D3D12_ROOT_PARAMETER_TYPE> rootParameterTypes = graphicPipelineManager->GetRootParameterTypes(psoHandle);
	if (rootParameterTypes.size() != rootResources.size()) {
		// PSOのルートパラメータの数と渡されたリソースの数が異なる場合はエラー
		assert(false);
		return false;
	}

	for (std::size_t i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		switch (rootParameterType) {
		case D3D12_ROOT_PARAMETER_TYPE_CBV: {
			D3D12_GPU_VIRTUAL_ADDRESS gpuHandle = resourceContainer->GetGpuVirtualAddress(rootResources[i]);
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
			commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_UAV: {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
			commandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(i), gpuHandle.ptr);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			assert(false);
			break;
		default:
			assert(false);
			break;
		}
	}

	size_t vertexCount = 0;
	if (!GetResourceArraySize(graphicEngine, vertexBufferHandle, vertexCount)) {
		assert(false);
		return false;
	}
	UINT vertexCountUINT = static_cast<UINT>(vertexCount);
	commandList->DrawInstanced(vertexCountUINT, 1, 0, 0);
	return true;
}

bool QFE::GRAPHIC::DrawGraphicPSO(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::PSOHandle& psoHandle,
	const QFE::GRAPHIC::ViewPortHandle& viewportHandle, const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle, const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
	const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets, const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes) {


	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();
	QFE::GRAPHIC::RenderPass* renderPass = ResolveD3D12GraphicEngine(graphicEngine)->GetRenderPass();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();
	QFE::GRAPHIC::GraphicPipelineManager* graphicPipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetGraphicPipelineManager();
	QFE::UniqueContainer<D3D12_VIEWPORT>& viewports = ResolveD3D12GraphicEngine(graphicEngine)->GetViewports();
	QFE::UniqueContainer<D3D12_RECT>& scissorRects = ResolveD3D12GraphicEngine(graphicEngine)->GetScissorRects();

	QFE::GRAPHIC::DirectXResourceHandle depthStencilBufferHandle = ResolveD3D12GraphicEngine(graphicEngine)->GetDepthStencilBufferHandle();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	// レンダーターゲットのバリアをレンダーターゲットに設定する前に、必要に応じてリソースの状態を遷移させる
	for (QFE::GRAPHIC::RenderTargetHandle renderTargetHandle : renderTargets) {
		QFE::GRAPHIC::DirectXResourceHandle renderTargetResourceHandle = renderPass->GetRenderTargetResourceHandle(renderTargetHandle);
		resourceContainer->TransitionResource(renderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	renderPass->SetRenderTarget(
		commandList, depthStencilBufferHandle, renderTargets);

	commandList->RSSetViewports(1, viewports.GetData(static_cast<uint32_t>(viewportHandle)));
	commandList->RSSetScissorRects(1, scissorRects.GetData(static_cast<uint32_t>(scissorRectHandle)));

	commandList->SetPipelineState(graphicPipelineManager->GetPipelineState(psoHandle));
	commandList->SetGraphicsRootSignature(graphicPipelineManager->GetRootSignature(psoHandle));

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = resourceContainer->GetVertexBufferView(vertexBufferHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	for (std::size_t i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		switch (rootParameterType) {
		case D3D12_ROOT_PARAMETER_TYPE_CBV: {
			D3D12_GPU_VIRTUAL_ADDRESS gpuHandle = resourceContainer->GetGpuVirtualAddress(rootResources[i]);
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
			commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_UAV: {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
			commandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(i), gpuHandle.ptr);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			assert(false);
			break;
		default:
			assert(false);
			break;
		}
	}

	size_t vertexCount = 0;
	if (!GetResourceArraySize(graphicEngine, vertexBufferHandle, vertexCount)) {
		assert(false);
		return false;
	}
	UINT vertexCountUINT = static_cast<UINT>(vertexCount);
	commandList->DrawInstanced(vertexCountUINT, 1, 0, 0);
	return true;
}

bool QFE::GRAPHIC::DrawGraphicPSO(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::PSOHandle& psoHandle,
	const QFE::GRAPHIC::ViewPortHandle& viewportHandle, const QFE::GRAPHIC::ScissorRectHandle& scissorRectHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& vertexBufferHandle, const QFE::GRAPHIC::DirectXResourceHandle& indexBufferHandle,
	const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources, const std::vector<QFE::GRAPHIC::RenderTargetHandle>& renderTargets,
	const std::vector<D3D12_ROOT_PARAMETER_TYPE>& rootParameterTypes) {

	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();
	QFE::GRAPHIC::RenderPass* renderPass = ResolveD3D12GraphicEngine(graphicEngine)->GetRenderPass();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();
	QFE::GRAPHIC::GraphicPipelineManager* graphicPipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetGraphicPipelineManager();
	QFE::UniqueContainer<D3D12_VIEWPORT>& viewports = ResolveD3D12GraphicEngine(graphicEngine)->GetViewports();
	QFE::UniqueContainer<D3D12_RECT>& scissorRects = ResolveD3D12GraphicEngine(graphicEngine)->GetScissorRects();

	QFE::GRAPHIC::DirectXResourceHandle depthStencilBufferHandle = ResolveD3D12GraphicEngine(graphicEngine)->GetDepthStencilBufferHandle();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);

	// 必要ならレンダーターゲットの状態遷移
	for (QFE::GRAPHIC::RenderTargetHandle renderTargetHandle : renderTargets) {
		QFE::GRAPHIC::DirectXResourceHandle renderTargetResourceHandle = renderPass->GetRenderTargetResourceHandle(renderTargetHandle);
		resourceContainer->TransitionResource(renderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	renderPass->SetRenderTarget(
		commandList, depthStencilBufferHandle, renderTargets);

	commandList->RSSetViewports(1, viewports.GetData(static_cast<uint32_t>(viewportHandle)));
	commandList->RSSetScissorRects(1, scissorRects.GetData(static_cast<uint32_t>(scissorRectHandle)));

	commandList->SetPipelineState(graphicPipelineManager->GetPipelineState(psoHandle));
	commandList->SetGraphicsRootSignature(graphicPipelineManager->GetRootSignature(psoHandle));

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = resourceContainer->GetVertexBufferView(vertexBufferHandle);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	// IndexBufferView を取得してセット
	D3D12_INDEX_BUFFER_VIEW indexBufferView = resourceContainer->GetIndexBufferView(indexBufferHandle);
	commandList->IASetIndexBuffer(&indexBufferView);

	for (std::size_t i = 0; i < rootParameterTypes.size(); ++i) {
		D3D12_ROOT_PARAMETER_TYPE rootParameterType = rootParameterTypes[i];

		switch (rootParameterType) {
		case D3D12_ROOT_PARAMETER_TYPE_CBV: {
			D3D12_GPU_VIRTUAL_ADDRESS gpuHandle = resourceContainer->GetGpuVirtualAddress(rootResources[i]);
			commandList->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), gpuHandle);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE: {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
			commandList->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), gpuHandle);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_UAV: {
			D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(rootResources[i], rootParameterType);
			commandList->SetGraphicsRootUnorderedAccessView(static_cast<UINT>(i), gpuHandle.ptr);
			break;
		}
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			assert(false);
			break;
		default:
			assert(false);
			break;
		}
	}

	// インデックス数を取得して DrawIndexedInstanced を呼ぶ
	size_t indexCount = 0;
	if (!GetResourceArraySize(graphicEngine, indexBufferHandle, indexCount)) {
		assert(false);
		return false;
	}
	UINT indexCountUINT = static_cast<UINT>(indexCount);
	commandList->DrawIndexedInstanced(indexCountUINT, 1, 0, 0, 0);

	return true;
}

bool QFE::GRAPHIC::DrawRayTracingPSO(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
	QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer, const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
	const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources) {

	// 使用機能の取得
	QFE::GRAPHIC::RenderPass* renderPass = ResolveD3D12GraphicEngine(graphicEngine)->GetRenderPass();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();
	QFE::GRAPHIC::RaytracingPipelineManager* raytracingPipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetRayTracingPipelineManager();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = ResolveD3D12GraphicEngine(graphicEngine)->GetRaytracingAccelerationStructure();
	QFE::GRAPHIC::RaytracingPSO* raytracingPSO =
		raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle);

	ID3D12GraphicsCommandList4* commandList4 = commandManager->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = raytracingPSO->GetRootSignature();
	ID3D12StateObject* rtpsoptr = raytracingPSO->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = raytracingPSO->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = raytracingPSO->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = raytracingPSO->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // レイトレPSOはSetPipelineState1を使う

	// 2. ルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure->GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(1, tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(renderUavBuffer, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);

	D3D12_GPU_VIRTUAL_ADDRESS cameraGpuHandle = resourceContainer->GetGpuVirtualAddress(cameraPositionBufferHandle);
	commandList4->SetComputeRootConstantBufferView(0, cameraGpuHandle);

	// レンダーターゲットのバリアをレンダーターゲットに設定する前に、必要に応じてリソースの状態を遷移させる
	for (QFE::GRAPHIC::DirectXResourceHandle renderTargetHandle : rootResources) {
		resourceContainer->TransitionResource(renderTargetHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	commandList4->SetComputeRootDescriptorTable(2, resourceContainer->GetDescriptorHandleGPU(rootResources[0], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(3, resourceContainer->GetDescriptorHandleGPU(rootResources[1], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(4, resourceContainer->GetDescriptorHandleGPU(rootResources[2], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(5, resourceContainer->GetDescriptorHandleGPU(rootResources[3], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(6, gpuHandle);

	// 3. シェーダーレコードのサイズ定義（前段で作った64バイトと同じ）
	const UINT shaderRecordSize = (D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1)
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 64

	// 4. DispatchRays の設定構造体を埋める
	D3D12_DISPATCH_RAYS_DESC dispatchDesc{};

	// --- RayGeneration テーブルの指定 ---
	dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = shaderRecordSize; // 64バイト

	// --- Miss テーブルの指定 ---
	dispatchDesc.MissShaderTable.StartAddress = missShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = shaderRecordSize; // 1つ分なので64バイト
	dispatchDesc.MissShaderTable.StrideInBytes = shaderRecordSize; // 1つあたりの歩進サイズ

	// --- HitGroup テーブルの指定（今回はまだ空なので0） ---
	dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = shaderRecordSize; // 1つのレコードサイズ
	dispatchDesc.HitGroupTable.StrideInBytes = shaderRecordSize;

	// --- Callable テーブルの指定（使わないので0） ---
	dispatchDesc.CallableShaderTable.StartAddress = 0;
	dispatchDesc.CallableShaderTable.SizeInBytes = 0;
	dispatchDesc.CallableShaderTable.StrideInBytes = 0;

	// --- 追跡する画面の解像度を指定（このピクセル数分の光線が一斉に飛びます） ---
	dispatchDesc.Width = 1280;  // 例: 1920
	dispatchDesc.Height = 720; // 例: 1080
	dispatchDesc.Depth = 1;            // 2D画面なので 1

	// 5. コマンド発行
	commandList4->DispatchRays(&dispatchDesc);

	// 6. スワップチェーンのバックバッファにコピー
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	renderPass->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(
		renderPass->GetCurrentBackBuffer(), resourceContainer->GetResource(renderUavBuffer));
	renderPass->TransitionCurrentBackBufferBarrier(
		commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

	return true;
}

bool QFE::GRAPHIC::DrawRayTracingPSO(QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle, QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer, const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle, const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources, const QFE::GRAPHIC::DirectXResourceHandle& renderTargetResourceHandle) {
	// 使用機能の取得
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();
	QFE::GRAPHIC::RaytracingPipelineManager* raytracingPipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetRayTracingPipelineManager();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = ResolveD3D12GraphicEngine(graphicEngine)->GetRaytracingAccelerationStructure();

	ID3D12GraphicsCommandList4* commandList4 = commandManager->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRootSignature();
	ID3D12StateObject* rtpsoptr = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle)->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // レイトレPSOはSetPipelineState1を使う

	// 2. ルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure->GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(1, tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(renderUavBuffer, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);

	D3D12_GPU_VIRTUAL_ADDRESS cameraGpuHandle = resourceContainer->GetGpuVirtualAddress(cameraPositionBufferHandle);
	commandList4->SetComputeRootConstantBufferView(0, cameraGpuHandle);

	// レンダーターゲットのバリアをレンダーターゲットに設定する前に、必要に応じてリソースの状態を遷移させる
	for (QFE::GRAPHIC::DirectXResourceHandle renderTargetHandle : rootResources) {
		resourceContainer->TransitionResource(renderTargetHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	commandList4->SetComputeRootDescriptorTable(2, resourceContainer->GetDescriptorHandleGPU(rootResources[0], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(3, resourceContainer->GetDescriptorHandleGPU(rootResources[1], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(4, resourceContainer->GetDescriptorHandleGPU(rootResources[2], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(5, resourceContainer->GetDescriptorHandleGPU(rootResources[3], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(6, gpuHandle);

	// 3. シェーダーレコードのサイズ定義（前段で作った64バイトと同じ）
	const UINT shaderRecordSize = (D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1)
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 64

	// 4. DispatchRays の設定構造体を埋める
	D3D12_DISPATCH_RAYS_DESC dispatchDesc{};

	// --- RayGeneration テーブルの指定 ---
	dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = shaderRecordSize; // 64バイト

	// --- Miss テーブルの指定 ---
	dispatchDesc.MissShaderTable.StartAddress = missShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = shaderRecordSize; // 1つ分なので64バイト
	dispatchDesc.MissShaderTable.StrideInBytes = shaderRecordSize; // 1つあたりの歩進サイズ

	// --- HitGroup テーブルの指定（今回はまだ空なので0） ---
	dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = shaderRecordSize; // 1つのレコードサイズ
	dispatchDesc.HitGroupTable.StrideInBytes = shaderRecordSize;

	// --- Callable テーブルの指定（使わないので0） ---
	dispatchDesc.CallableShaderTable.StartAddress = 0;
	dispatchDesc.CallableShaderTable.SizeInBytes = 0;
	dispatchDesc.CallableShaderTable.StrideInBytes = 0;

	// --- 追跡する画面の解像度を指定（このピクセル数分の光線が一斉に飛びます） ---
	dispatchDesc.Width = 1280;  // 例: 1920
	dispatchDesc.Height = 720; // 例: 1080
	dispatchDesc.Depth = 1;            // 2D画面なので 1

	// 5. コマンド発行
	commandList4->DispatchRays(&dispatchDesc);

	// 6. スワップチェーンのバックバッファにコピー
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	resourceContainer->TransitionResource(renderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->CopyResource(
		resourceContainer->GetResource(renderTargetResourceHandle), resourceContainer->GetResource(renderUavBuffer));
	resourceContainer->TransitionResource(renderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	return true;
}

bool QFE::GRAPHIC::ShadowSpecularRayTracingPSO(
	QFE::GRAPHIC::IGraphicEngine* graphicEngine,
	const QFE::GRAPHIC::RTPSOHandle& rtpsoHandle,
	QFE::GRAPHIC::DirectXResourceHandle renderUavBuffer,
	const QFE::GRAPHIC::DirectXResourceHandle& cameraPositionBufferHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& indexBufferHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& uvBufferHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& instanceMetaBufferHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& firstTextureBufferHandle,
	const QFE::GRAPHIC::DirectXResourceHandle& skyBoxTextureHandle,
	const std::vector<QFE::GRAPHIC::DirectXResourceHandle>& rootResources,
	QFE::GRAPHIC::RenderTargetHandle finalRenderTargetHandle) {

	// 使用機能の取得
	QFE::GRAPHIC::RenderPass* renderPass = ResolveD3D12GraphicEngine(graphicEngine)->GetRenderPass();
	QFE::GRAPHIC::DirectXCommandManager* commandManager = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXCommandManager();
	QFE::GRAPHIC::RaytracingPipelineManager* raytracingPipelineManager = ResolveD3D12GraphicEngine(graphicEngine)->GetRayTracingPipelineManager();
	QFE::GRAPHIC::DirectXResourceContainer* resourceContainer = ResolveD3D12GraphicEngine(graphicEngine)->GetDirectXResourceContainer();
	QFE::GRAPHIC::RaytracingAccelerationStructure* accelerationStructure = ResolveD3D12GraphicEngine(graphicEngine)->GetRaytracingAccelerationStructure();
	QFE::GRAPHIC::RaytracingPSO* raytracingPSO =
		raytracingPipelineManager->GetRaytracingPipelineStateObject(rtpsoHandle);
	QFE::GRAPHIC::RootParameter& rootParameters = raytracingPSO->GetRootParameter();

	ID3D12GraphicsCommandList4* commandList4 = commandManager->GetCommandList4(D3D12_COMMAND_LIST_TYPE_DIRECT);
	ID3D12RootSignature* globalRootSignature = raytracingPSO->GetRootSignature();
	ID3D12StateObject* rtpsoptr = raytracingPSO->GetPipelineState();
	ID3D12Resource* rayGenShaderTable_ = raytracingPSO->GetRayGenShaderTable();
	ID3D12Resource* missShaderTable_ = raytracingPSO->GetMissShaderTable();
	ID3D12Resource* hitGroupShaderTable_ = raytracingPSO->GetHitGroupShaderTable();

	ID3D12GraphicsCommandList* commandList = commandManager->GetCommandList(D3D12_COMMAND_LIST_TYPE_DIRECT);
	resourceContainer->TransitionResource(renderUavBuffer, commandList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	// 1. DXR用のパイプライン(RTPSO)とルートシグネチャをコマンドリストにセット
	commandList4->SetComputeRootSignature(globalRootSignature);
	commandList4->SetPipelineState1(rtpsoptr); // レイトレPSOはSetPipelineState1を使う

	// 2. ルートシグネチャへのリソースバインド
	D3D12_GPU_VIRTUAL_ADDRESS tlasResultBufferGPUHandle = accelerationStructure->GetTLASResultBuffer()->GetGPUVirtualAddress();
	commandList4->SetComputeRootShaderResourceView(
		rootParameters.GetRootParameterIndex("g_scene"), tlasResultBufferGPUHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = resourceContainer->GetDescriptorHandleGPU(renderUavBuffer, QFE::GRAPHIC::ViewTypeFlags::UnorderedAccessView);

	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_globalTriIndices"),
		resourceContainer->GetDescriptorHandleGPU(indexBufferHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_globalVertexAttributes"),
		resourceContainer->GetDescriptorHandleGPU(uvBufferHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_instanceMeta"),
		resourceContainer->GetDescriptorHandleGPU(instanceMetaBufferHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_TextureArray"),
		resourceContainer->GetDescriptorHandleGPU(firstTextureBufferHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_skybox"),
		resourceContainer->GetDescriptorHandleGPU(skyBoxTextureHandle, QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));

	D3D12_GPU_VIRTUAL_ADDRESS cameraGpuHandle = resourceContainer->GetGpuVirtualAddress(cameraPositionBufferHandle);
	commandList4->SetComputeRootConstantBufferView(
		rootParameters.GetRootParameterIndex("g_camera"), cameraGpuHandle);

	// レンダーターゲットのバリアをレンダーターゲットに設定する前に、必要に応じてリソースの状態を遷移させる
	for (QFE::GRAPHIC::DirectXResourceHandle renderTargetHandle : rootResources) {
		resourceContainer->TransitionResource(renderTargetHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_position"),
		resourceContainer->GetDescriptorHandleGPU(rootResources[0], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_normal"),
		resourceContainer->GetDescriptorHandleGPU(rootResources[1], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_albedo"),
		resourceContainer->GetDescriptorHandleGPU(rootResources[2], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_material"),
		resourceContainer->GetDescriptorHandleGPU(rootResources[3], QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView));
	commandList4->SetComputeRootDescriptorTable(
		rootParameters.GetRootParameterIndex("g_output"), gpuHandle);


	// 3. シェーダーレコードのサイズ定義（前段で作った64バイトと同じ）
	const UINT shaderRecordSize = (D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1)
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 64

	// 4. DispatchRays の設定構造体を埋める
	D3D12_DISPATCH_RAYS_DESC dispatchDesc{};

	// --- RayGeneration テーブルの指定 ---
	dispatchDesc.RayGenerationShaderRecord.StartAddress = rayGenShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = shaderRecordSize; // 64バイト

	// --- Miss テーブルの指定 ---
	dispatchDesc.MissShaderTable.StartAddress = missShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = shaderRecordSize; // 1つ分なので64バイト
	dispatchDesc.MissShaderTable.StrideInBytes = shaderRecordSize; // 1つあたりの歩進サイズ

	// --- HitGroup テーブルの指定（今回はまだ空なので0） ---
	dispatchDesc.HitGroupTable.StartAddress = hitGroupShaderTable_->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = shaderRecordSize; // 1つのレコードサイズ
	dispatchDesc.HitGroupTable.StrideInBytes = shaderRecordSize;

	// --- Callable テーブルの指定（使わないので0） ---
	dispatchDesc.CallableShaderTable.StartAddress = 0;
	dispatchDesc.CallableShaderTable.SizeInBytes = 0;
	dispatchDesc.CallableShaderTable.StrideInBytes = 0;

	// --- 追跡する画面の解像度を指定（このピクセル数分の光線が一斉に飛びます） ---
	dispatchDesc.Width = 1280;  // 例: 1920
	dispatchDesc.Height = 720; // 例: 1080
	dispatchDesc.Depth = 1;            // 2D画面なので 1

	// 5. コマンド発行
	commandList4->DispatchRays(&dispatchDesc);

	// 6. 出力先へコピー
	if (finalRenderTargetHandle == QFE::GRAPHIC::RenderTargetHandle::Invalid) {
		return false;
	}

	resourceContainer->TransitionResource(
		renderUavBuffer, commandList, D3D12_RESOURCE_STATE_COPY_SOURCE);

	if (finalRenderTargetHandle == QFE::GRAPHIC::RenderTargetHandle::SwapChain) {
		renderPass->TransitionCurrentBackBufferBarrier(
			commandList, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->CopyResource(
			renderPass->GetCurrentBackBuffer(), resourceContainer->GetResource(renderUavBuffer));
		renderPass->TransitionCurrentBackBufferBarrier(
			commandList, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

		return true;
	} else {
		QFE::GRAPHIC::DirectXResourceHandle finalRenderTargetResourceHandle = renderPass->GetRenderTargetResourceHandle(finalRenderTargetHandle);
		resourceContainer->TransitionResource(finalRenderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_COPY_DEST);
		commandList->CopyResource(
			resourceContainer->GetResource(finalRenderTargetResourceHandle), resourceContainer->GetResource(renderUavBuffer));
		resourceContainer->TransitionResource(finalRenderTargetResourceHandle, commandList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		return true;
	}
}
