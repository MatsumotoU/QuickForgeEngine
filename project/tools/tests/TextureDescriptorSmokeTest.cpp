#include "graphics/dx12/vram/descriptors/DescriptorHeapManager.h"
#include "graphics/dx12/vram/resources/DirectXResource.h"
#include "graphics/dx12/vram/resources/DirectXResourceContainer.h"
#include "graphics/dx12/TextureLoader.h"
#include "framework/application/RetiredModelResources.h"
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <filesystem>

int wmain(int argc, wchar_t** argv) {
	if (argc != 2) return 2;
	using Microsoft::WRL::ComPtr;
	ComPtr<IDXGIFactory4> factory;
	ComPtr<IDXGIAdapter> warp;
	ComPtr<ID3D12Device> device;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))) ||
		FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp))) ||
		FAILED(D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)))) {
		std::cerr << "Could not create the WARP test device.\n";
		return 1;
	}
	QFE::GRAPHIC::DescriptorHeapManager manager;
	manager.Initialize(device.Get());
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MipLevels = 1;
	const auto first = manager.AssignTexture(device.Get(), nullptr, desc);
	const auto second = manager.AssignTexture(device.Get(), nullptr, desc);
	const auto shared = manager.AssignTexture(device.Get(), nullptr, desc);
	QFE::GRAPHIC::DirectXResource resource;
	resource.AddDescriptorHandle(QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView, shared);
	const auto* alias = resource.GetDescriptorHandle(QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView);
	if (first.textureIndex != 0 || second.textureIndex != 1 || shared.textureIndex != 2 ||
		!alias || alias->textureIndex != shared.textureIndex || alias->gpuHandle_.ptr != shared.gpuHandle_.ptr) {
		std::cerr << "Texture descriptor indices did not follow the allocated slots.\n";
		return 1;
	}
	QFE::GRAPHIC::DirectXResourceContainer container;
	QFE::GRAPHIC::DirectXResourceContainerInitializeInfo info{};
	info.assignTextureFunc = [&](ID3D12Resource* resourcePtr, const D3D12_SHADER_RESOURCE_VIEW_DESC* view) {
		return manager.AssignTexture(device.Get(), resourcePtr, *view);
	};
	container.Initialize(info);
	D3D12_RESOURCE_DESC texture{};
	texture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texture.Width = 1;
	texture.Height = 1;
	texture.DepthOrArraySize = 1;
	texture.MipLevels = 1;
	texture.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture.SampleDesc.Count = 1;
	texture.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	const auto handle = container.CreateResource(device.Get(), texture, D3D12_RESOURCE_STATE_COMMON);
	if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return 1;
	if (!container.CreateTextureResourceView(handle, desc)) return 1;
	const auto indexA = container.GetTextureDescriptorIndex(handle);
	const auto indexB = container.GetTextureDescriptorIndex(handle);
	if (indexA != 3 || indexB != indexA ||
		container.GetTextureDescriptorIndex(QFE::GRAPHIC::DirectXResourceHandle::Invalid) != UINT32_MAX) {
		std::cerr << "Resource aliases did not resolve to the same texture slot.\n";
		return 1;
	}
	for (uint32_t i = 4; i < 256; ++i) {
		if (manager.AssignTexture(device.Get(), nullptr, desc).textureIndex != i) return 1;
	}
	if (manager.AssignTexture(device.Get(), nullptr, desc).textureIndex != UINT32_MAX ||
		manager.AssignTexture(device.Get(), nullptr, desc).textureIndex != UINT32_MAX) {
		std::cerr << "Texture descriptor exhaustion did not fail safely.\n";
		return 1;
	}
	const auto overflowHandle = container.CreateResource(device.Get(), texture, D3D12_RESOURCE_STATE_COMMON);
	if (overflowHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid || container.CreateTextureResourceView(overflowHandle, desc) ||
		container.GetTextureDescriptorIndex(overflowHandle) != UINT32_MAX || !container.RemoveResource(overflowHandle)) {
		std::cerr << "Exhausted texture view did not fail and clean up safely.\n";
		return 1;
	}
	QFE::GRAPHIC::DescriptorHeapManager loaderHeap;
	loaderHeap.Initialize(device.Get());
	QFE::GRAPHIC::DirectXResourceContainer loaderResources;
	QFE::GRAPHIC::DirectXResourceContainerInitializeInfo loaderResourceInfo{};
	loaderResourceInfo.assignTextureFunc = [&](ID3D12Resource* resourcePtr, const D3D12_SHADER_RESOURCE_VIEW_DESC* view) {
		return loaderHeap.AssignTexture(device.Get(), resourcePtr, *view);
	};
	loaderResources.Initialize(loaderResourceInfo);
	unsigned created = 0, removed = 0;
	QFE::GRAPHIC::TextureLoaderInitializeInfo loaderInfo{};
	loaderInfo.device = device.Get();
	loaderInfo.createResourceFunc = [&](const D3D12_RESOURCE_DESC& resourceDesc) {
		++created;
		return loaderResources.CreateResource(device.Get(), resourceDesc, D3D12_RESOURCE_STATE_COMMON);
	};
	loaderInfo.createShaderResourceViewFunc = [&](QFE::GRAPHIC::DirectXResourceHandle resourceHandle, const D3D12_SHADER_RESOURCE_VIEW_DESC& view) {
		return loaderResources.CreateTextureResourceView(resourceHandle, view);
	};
	loaderInfo.removeResourceFunc = [&](QFE::GRAPHIC::DirectXResourceHandle resourceHandle) {
		if (loaderResources.RemoveResource(resourceHandle)) ++removed;
	};
	loaderInfo.uploadTextureDataFunc = [](QFE::GRAPHIC::DirectXResourceHandle, const std::vector<D3D12_SUBRESOURCE_DATA>&) {};
	QFE::GRAPHIC::TextureLoader loader;
	loader.Initialize(loaderInfo);
	for (uint32_t i = 2; i < 256; ++i) {
		if (loaderHeap.AssignTexture(device.Get(), nullptr, desc).textureIndex != i) return 1;
	}
	std::ifstream png(std::filesystem::path(argv[1]), std::ios::binary);
	const std::vector<uint8_t> bytes{std::istreambuf_iterator<char>(png), std::istreambuf_iterator<char>()};
	const auto failed = loader.LoadTextureFromMemory("exhausted", bytes.data(), bytes.size());
	loader.Finalize();
	if (failed != QFE::GRAPHIC::DirectXResourceHandle::Invalid || created != 3 || removed != 1) {
		std::cerr << "Texture loader did not release the failed allocation.\n";
		return 1;
	}
	const auto activeBuffer = container.CreateBuffer(device.Get(), 256);
	const auto unusedBuffer = container.CreateBuffer(device.Get(), 256);
	std::unordered_map<uint32_t, QFE::GRAPHIC::DirectXResourceHandle> animated{{1, activeBuffer}, {2, unusedBuffer}};
	const auto retired = QFE::FRAMEWORK::CollectUnusedAnimatedBuffers(animated, std::unordered_set<uint32_t>{1});
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	ComPtr<ID3D12CommandQueue> queue;
	ComPtr<ID3D12Fence> fence;
	if (FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue))) ||
		FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))) ||
		FAILED(queue->Signal(fence.Get(), 1))) return 1;
	HANDLE event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (!event) return 1;
	const bool waited = SUCCEEDED(fence->SetEventOnCompletion(1, event)) && WaitForSingleObject(event, 10000) == WAIT_OBJECT_0;
	CloseHandle(event);
	if (!waited || retired.size() != 1 || retired[0].second != unusedBuffer ||
		!container.RemoveResource(retired[0].second) || container.RemoveResource(unusedBuffer) ||
		!container.RemoveResource(activeBuffer)) {
		std::cerr << "Animated buffer retirement after the GPU fence failed.\n";
		return 1;
	}
	std::cout << "Texture descriptor checks passed.\n";
	return 0;
}
