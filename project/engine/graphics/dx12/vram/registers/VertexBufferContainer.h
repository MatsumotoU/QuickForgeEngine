#pragma once
#include <stdint.h>
#include <vector>
#include <functional>

#include "memory/SparseSets.h"
#include "buffer/VertexBuffer.h"

namespace QFE::GRAPHIC::INTERNAL {

	/// @brief 頂点バッファを生成、ハンドルを管理するクラス
	class VertexBufferContainer final {
	public:
		/** @brief 初期化 */
		void Initialize();

		uint32_t Assign(ID3D12Device* device, const std::vector<VertexData>& vertex, const std::string& name);

		/** @brief ハンドルから頂点バッファの数を取得 */
		const uint32_t GetVertexBufferCount(uint32_t handle) const;
		/** @brief ハンドルから頂点リソースを取得 */
		ID3D12Resource* GetModelVertexBuffer(const uint32_t& handle);
		/** @brief ハンドルから頂点データを取得 */
		VertexData* GetModelVertexBufferData(const uint32_t& handle);
		/** @brief ハンドルから頂点バッファビューを取得 */
		const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView(const uint32_t& handle);

		/** @brief 終了処理 */
		void Finalize();

	private:
		std::unordered_map<std::string, uint32_t> loadNameMap_;// 頂点データのロード名とハンドルのマップ
		QFE::SparseSet<VertexBuffer<VertexData>> vertexBufferMap_;
	};
}
