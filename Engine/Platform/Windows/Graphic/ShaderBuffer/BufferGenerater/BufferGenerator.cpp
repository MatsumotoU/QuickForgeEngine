#include "BufferGenerator.h"
#include <cassert>
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include "String/MyString.h"
#endif // _DEBUG

Microsoft::WRL::ComPtr<ID3D12Resource> BufferGenerator::Generate(ID3D12Device* device, size_t sizeInBytes) {
	// * Resourceを生成する * //
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(vertexResource.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));

#ifdef _DEBUG
	DebugLog(ConvertString(std::format(L"CreateBufferResource Bytes = {}", sizeInBytes)));
#endif // _DEBUG
	return vertexResource;
}
