#pragma once
#include <memory>
#include <typeindex>
#include <any>
#include <unordered_map>
#include <stdint.h>
#include <assert.h>

#include "engine/include/utility/memory/VariableLengthPool.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/ShaderBuffer/ConstantBuffer.h"

class GpuBufferPool final {
public:
	// このクラスを生成できるのはDirectXCommonを渡したときのみ
	GpuBufferPool() = delete;
    GpuBufferPool(DirectXCommon* dxCommon):dxCommon_(dxCommon){}
    ~GpuBufferPool() = default;
    
	/// 特定の型のバッファーを登録してハンドルを取得します
    template<typename T>
    uint32_t AcquireConstantBuffer() {
		// 型情報からプールを取得または生成
        auto typeIdx = std::type_index(typeid(T));
		// 型ごとのプールが存在しない場合は新規作成
        if (constantBufferPoolsMap_.count(typeIdx) == 0) {
            constantBufferPoolsMap_[typeIdx] = VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>>();
        }
		// プールからバッファーを取得してハンドルを返す
        auto& pool = std::any_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(constantBufferPoolsMap_[typeIdx]);
        auto buffer = std::make_unique<ConstantBuffer<T>>();
		buffer->CreateResource(dxCommon_->GetDevice());
        uint32_t handle = pool.Add(std::move(buffer));
        return handle;
    }

	/// 取得したハンドルから特定の型のバッファーを取得します
    template<typename T>
    ConstantBuffer<T>* GetConstantBuffer(uint32_t handle) {
        auto typeIdx = std::type_index(typeid(T));
        auto& pool = std::any_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(constantBufferPoolsMap_[typeIdx]);
        return pool.Get(handle).get();
    }

	/// 取得したハンドルから特定の型のバッファーデータを取得します
	template<typename T>
	T* GetConstantBufferData(uint32_t handle) {
		auto typeIdx = std::type_index(typeid(T));
		auto& pool = std::any_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(constantBufferPoolsMap_[typeIdx]);
		return pool.Get(handle)->GetData();
	}

	/// 取得したハンドルから特定の型のバッファーのGPU仮想アドレスを取得します
	template<typename T>
	D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferAddress(uint32_t handle) {
		auto typeIdx = std::type_index(typeid(T));
		auto& pool = std::any_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(constantBufferPoolsMap_[typeIdx]);
		return pool.Get(handle)->GetGPUVirtualAddress();
	}

private:
	// DirectXCommonのポインタ
	DirectXCommon* dxCommon_ = nullptr;

	// 型ハッシュごとの定数バッファプールのマップ
    std::unordered_map<std::type_index, std::any> constantBufferPoolsMap_;
};