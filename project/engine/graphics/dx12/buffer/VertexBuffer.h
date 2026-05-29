#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <cassert>
#include "../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "BufferGenerater/BufferGenerator.h"
#include "EngineDefines.h"

namespace QFE {
	/// @class VertexBuffer
	/// @brief 頂点バッファを管理するクラス
	/// @tparam T 頂点データの構造体型
	template<class T>
	class VertexBuffer {
	public:
		/// @brief 頂点バッファを作成する
		/// @param device Direct3D 12 デバイス
		/// @param vertexCount 頂点の数
		void CreateResource(ID3D12Device* device, uint32_t vertexCount) {
			assert(device);
			assert(vertexCount != 0);
			vertexCount_ = vertexCount;

			// 頂点バッファリソースを生成
			vertexResource_ = BufferGenerator::Generate(device, sizeof(T) * vertexCount);
			// 頂点バッファビューを設定
			vertexBufferView_ = {};
			vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
			vertexBufferView_.SizeInBytes = sizeof(T) * vertexCount;
			vertexBufferView_.StrideInBytes = sizeof(T);

			// 頂点バッファをマップしてCPUアクセス可能にする
			vertexData_ = nullptr;
			vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

			// 頂点データを作成したことを示すフラグを立てる
			isCreated_ = true;
		};

		/// @brief 頂点データを取得するゲッター
		T* GetData() { 
			assert(isCreated_ && "Vertex buffer not created");
			return vertexData_;
		}
		/// @brief 頂点バッファリソースを取得するゲッター
		ID3D12Resource* GetResource() const { 
			assert(isCreated_ && "Vertex buffer not created");
			return vertexResource_.Get(); 
		}
		/// @brief 頂点バッファビューを取得するゲッター
		const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() const { 
			assert(isCreated_ && "Vertex buffer not created");
			return &vertexBufferView_; 
		}
		/// @brief GPU仮想アドレスを取得するゲッター	
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
			assert(isCreated_ && "Vertex buffer not created");
			return vertexResource_.Get()->GetGPUVirtualAddress();
		}
		/// @brief 頂点数を取得するゲッター
		const uint32_t GetVertexCount() const { 
			assert(isCreated_ && "Vertex buffer not created");
			return vertexCount_; 
		}
		// / @brief 頂点データを設定するセッター
		void SetData(uint32_t index, const VertexData& data) {
			assert(isCreated_ && "Vertex buffer not created");
			assert(index < vertexCount_ && "Index out of bounds");
			vertexData_[index] = data;
		}

	private:
		// 頂点データを管理するためのメンバ変数
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;// GPU側の頂点バッファリソース本体
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;// 頂点バッファビュー
		T* vertexData_;// CPU側の頂点データへのポインタ

		// リソースの管理とアクセスの安全性を確保するためのフラグとカウンタ
		uint32_t vertexCount_ = 0;// 頂点の数
		bool isCreated_ = false;// 頂点バッファが作成されたかどうかを示すフラグ
	};
}  // namespace QFE