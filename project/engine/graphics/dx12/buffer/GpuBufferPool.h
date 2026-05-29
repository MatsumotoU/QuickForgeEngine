#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdint.h>
#include <assert.h>
#include <vector>

#include "ConstantBuffer.h"

#include "memory/VariableLengthPool.h"
#include "memory/IVariableLengthPoolContainer.h"

namespace QFE {
	class GpuBufferPool final {
	public:
		/// 特定の型のバッファーを登録してハンドルを取得します
		template<typename T>
		uint32_t AcquireConstantBuffer(ID3D12Device* device) {
			// 型情報からプールを取得または生成
			auto typeIdx = std::type_index(typeid(T));
			// 型ごとのプールが存在しない場合は新規作成
			if (constantBufferPoolsMap_.count(typeIdx) == 0) {
				constantBufferPoolsMap_[typeIdx] = 
					std::make_unique<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>>>(QFE::CONSTANTS::GPU_BUFFER_POOL::kMaxConstantBuffers);
			}
			// プールからバッファーを取得してハンドルを返す
			auto& pool = static_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(*constantBufferPoolsMap_[typeIdx]);
			auto buffer = std::make_unique<ConstantBuffer<T>>();
			buffer->CreateResource(device);
			uint32_t handle = pool.Add(std::move(buffer));
			return handle;
		}

		/// すべての型のプールのバッファーを解放します
		void ReleaseAllConstantBuffers() {
			for (auto const& [key, val] : constantBufferPoolsMap_) {
				val->Release();
			}
		}

		/// 解放したいハンドルのバッファーを解放します
		template<typename T>
		void ReleaseConstantBuffer(uint32_t handle) {
			auto typeIdx = std::type_index(typeid(T));
			auto& pool = static_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(*constantBufferPoolsMap_.at(typeIdx));
			pool.Release(handle);
		}

		/// 取得したハンドルから特定の型のバッファーを取得します
		template<typename T>
		ConstantBuffer<T>* GetConstantBuffer(uint32_t handle) {
			auto typeIdx = std::type_index(typeid(T));
			auto& pool = static_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(*constantBufferPoolsMap_.at(typeIdx));
			return pool.Get(handle).get();
		}

		/// 取得したハンドルから特定の型のバッファーデータを取得します
		template<typename T>
		T* GetConstantBufferData(uint32_t handle) {
			try
			{
				auto typeIdx = std::type_index(typeid(T));
				auto& pool = static_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(*constantBufferPoolsMap_.at(typeIdx));
				return pool.Get(handle)->GetData();
			}
			catch (const std::exception&)
			{
				return nullptr;
			}
			return nullptr;
		}

		/// 取得したハンドルから特定の型のバッファーのGPU仮想アドレスを取得します
		template<typename T>
		D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferAddress(uint32_t handle) {
			auto typeIdx = std::type_index(typeid(T));
			auto& pool = static_cast<VariableLengthPool<std::shared_ptr<ConstantBuffer<T>>> &>(*constantBufferPoolsMap_.at(typeIdx));
			return pool.Get(handle)->GetGPUVirtualAddress();
		}

		/// プールの情報をすべて取得します
		std::vector<IVariableLengthPoolContainer*> GetPoolsInfo() {
			std::vector<IVariableLengthPoolContainer*> infos;
			infos.reserve(constantBufferPoolsMap_.size());
			for (auto const& [key, val] : constantBufferPoolsMap_) {
				infos.push_back(val.get());
			}
			return infos;
		}

	private:
		// 型ハッシュごとの定数バッファプールのマップ
		std::unordered_map<std::type_index, std::unique_ptr<IVariableLengthPoolContainer>> constantBufferPoolsMap_;
	};
}