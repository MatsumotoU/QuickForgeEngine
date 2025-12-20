#pragma once
#include <memory>
#include <typeindex>
#include <any>
#include <unordered_map>
#include <stdint.h>

#include "engine/include/utility/memory/VariableLengthPool.h"

#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/ShaderBuffer/ConstantBuffer.h"


class GpuBufferPool final {
public:
    GpuBufferPool() = default;
    ~GpuBufferPool() = default;
    
	/// 特定の型のバッファーを登録してハンドルを取得します
    template<typename T>
    uint32_t AcquireConstantBuffer() {
		// 型情報からプールを取得または生成
        auto typeIdx = std::type_index(typeid(T));
		// 型ごとのプールが存在しない場合は新規作成
        if (constantBufferPoolsMap_.count(typeIdx) == 0) {
            constantBufferPoolsMap_[typeIdx] = VariableLengthPool<std::unique_ptr<ConstantBuffer<T>>>();
        }
		// プールからバッファーを取得してハンドルを返す
        auto& pool = std::any_cast<VariableLengthPool<std::unique_ptr<ConstantBuffer<T>>> &>(constantBufferPoolsMap_[typeIdx]);
        auto buffer = std::make_unique<ConstantBuffer<T>>();
        uint32_t handle = pool.Add(std::move(buffer));
        return handle;
    }

	/// 取得したハンドルから特定の型のバッファーを取得します
    template<typename T>
    ConstantBuffer<T>* GetConstantBuffer(uint32_t handle) {
        auto typeIdx = std::type_index(typeid(T));
        auto& pool = std::any_cast<VariableLengthPool<std::unique_ptr<ConstantBuffer<T>>> &>(constantBufferPoolsMap_[typeIdx]);
        return pool.Get(handle).get();
    }

private:
	// 型ハッシュごとの定数バッファプールのマップ
    std::unordered_map<std::type_index, std::any> constantBufferPoolsMap_;
};