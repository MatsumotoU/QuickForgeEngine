#include "TLAS.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

bool TLAS::Create(ID3D12Device5* device5, UINT maxInstances) {
	maxInstances_ = maxInstances;

	// -------------------------------------------------------------------------
	// 1. 最大数分のインスタンス記述（配置図）を載せるための GPU バッファを計算・確保
	// -------------------------------------------------------------------------
	UINT instanceBufferSize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * maxInstances_;

	D3D12_HEAP_PROPERTIES uploadHeapProps{ D3D12_HEAP_TYPE_UPLOAD };
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = instanceBufferSize; // 容量を確保
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	HRESULT hr = device5->CreateCommittedResource(
		&uploadHeapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&instanceDescBuffer_)
	);
	if (FAILED(hr)) return false;
	instanceDescBuffer_->SetName(L"TLAS-MaxInstance-Desc-Buffer");

	// -------------------------------------------------------------------------
	// 2. 最大数並んだ時を想定して、TLASビルドに必要な最大サイズを計算してもらう
	// -------------------------------------------------------------------------
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS buildInputs{};
	buildInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	buildInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	buildInputs.NumDescs = maxInstances_; // ★「最大でこれだけ並びます」と伝える
	buildInputs.InstanceDescs = instanceDescBuffer_->GetGPUVirtualAddress();

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuildInfo{};
	device5->GetRaytracingAccelerationStructurePrebuildInfo(&buildInputs, &prebuildInfo);

	// -------------------------------------------------------------------------
	// 3. 割り出された「最大必要サイズ」で、TLAS実体・スクラッチバッファを確保
	// -------------------------------------------------------------------------
	D3D12_HEAP_PROPERTIES defaultHeapProps{ D3D12_HEAP_TYPE_DEFAULT };

	// TLAS実体バッファ（完成品用）
	D3D12_RESOURCE_DESC tlasDesc = bufferDesc;
	tlasDesc.Width = prebuildInfo.ResultDataMaxSizeInBytes; // 最大サイズ
	tlasDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	hr = device5->CreateCommittedResource(
		&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &tlasDesc,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr, IID_PPV_ARGS(&tlasResultBuffer_)
	);
	if (FAILED(hr)) return false;
	tlasResultBuffer_->SetName(L"TLAS-Result-Buffer");

	// TLAS作業用スクラッチバッファ
	D3D12_RESOURCE_DESC scratchDesc = bufferDesc;
	scratchDesc.Width = prebuildInfo.ScratchDataSizeInBytes; // 最大サイズ
	scratchDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	hr = device5->CreateCommittedResource(
		&defaultHeapProps, D3D12_HEAP_FLAG_NONE, &scratchDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr, IID_PPV_ARGS(&tlasScratchBuffer_)
	);
	if (FAILED(hr)) return false;
	tlasScratchBuffer_->SetName(L"TLAS-Scratch-Buffer");

	return true;
}

bool TLAS::Build(
	ID3D12GraphicsCommandList4* commandList, const std::vector<RaytracingInstance>& instances, 
	std::function<ID3D12Resource* (BLASHandle)> getResourceFunc) {

	// 描画するオブジェクトが一切ない場合はビルドをスキップ（正常終了）
	if (instances.empty()) {
		return true;
	}

	// 1. あらかじめ確保した最大数を超えていないか安全チェック
	UINT numActive = static_cast<UINT>(instances.size());
	if (numActive > maxInstances_) {
		QFE_LOG("Active instances exceed max instances!");
		return false;
	}

	// 2. CPU側で現在の配置図（Instance Desc）の配列を組み立てる
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs(numActive);
	for (UINT i = 0; i < numActive; ++i) {
		const auto& src = instances[i];
		auto& dest = instanceDescs[i];

		// ワールド行列を3x4形式でコピー
		std::memcpy(dest.Transform, &src.worldMatrix, sizeof(float) * 12);

		dest.InstanceID = 0;                          // 今回は使わないので0
		dest.InstanceMask = 0xFF;                       // すべてのレイと衝突するマスク
		dest.InstanceContributionToHitGroupIndex = 0;   // 今回はオフセット0
		dest.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

		// マネージャー側からBLASのリソースを引き出す
		ID3D12Resource* blasBuffer = getResourceFunc(src.blasHandle);
		if (!blasBuffer) {
			// 有効なBLASリソースが見つからなかった場合はエラー
			return false;
		}

		// 引っ張ってきたBLASのGPU上の仮想アドレスを登録
		dest.AccelerationStructure = blasBuffer->GetGPUVirtualAddress();
	}

	// 3. 組み立てた配置図データをGPUのUploadバッファにコピー
	void* pInstanceData = nullptr;
	UINT copySize = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * numActive;
	instanceDescBuffer_->Map(0, nullptr, &pInstanceData);
	std::memcpy(pInstanceData, instanceDescs.data(), copySize);
	instanceDescBuffer_->Unmap(0, nullptr);

	// 4. ビルド入力を設定（NumDescsに「現在の有効数」を指定するのがポイント）
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS buildInputs{};
	buildInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	buildInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	buildInputs.NumDescs = numActive; // ツリー化するインスタンスの数を指定
	buildInputs.InstanceDescs = instanceDescBuffer_->GetGPUVirtualAddress();

	// 5. コマンドリストに構築コマンドを記録
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC buildDesc{};
	buildDesc.Inputs = buildInputs;
	buildDesc.DestAccelerationStructureData = tlasResultBuffer_->GetGPUVirtualAddress();
	buildDesc.ScratchAccelerationStructureData = tlasScratchBuffer_->GetGPUVirtualAddress();

	commandList->BuildRaytracingAccelerationStructure(&buildDesc, 0, nullptr);

	// 6. 構築完了を同期するためのUAVバリアを張る
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = tlasResultBuffer_.Get();
	commandList->ResourceBarrier(1, &barrier);

	return true;
}
