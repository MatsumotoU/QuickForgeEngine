/**
 * @file ModelVertexResourceManager.h
 * @brief モデルの頂点バッファリソースを管理するクラス
 */

#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <deque>
#include <list>
#include <unordered_map>
#include <d3d12.h>
#include <memory>
#include <unordered_map>

#include "Data/ModelData.h"
#include "engine/include/graphic/ShaderBuffer/VertexBuffer.h"

namespace QFE {

	/**
	 * @class ModelVertexResourceManager
	 * @brief 複数の3Dモデルの頂点リソースをGPU上に確保・管理するクラス
	 */
	class ModelVertexResourceManager final {
	public:
		/** @brief 初期化 */
		void Initialize();

		/**
		 * @brief モデルデータをリソースに割り当てる
		 * @param device DirectXデバイス
		 * @param modelData モデルデータ
		 * @param modelName モデル名
		 * @return モデルハンドル
		 */
		uint32_t Assign(ID3D12Device* device, const ModelData& modelData, const std::string& modelName);

		/// @brief 平面型の頂点データを生成してリソースに割り当てる
		uint32_t AssignPlane(ID3D12Device* device, float width = 1.0f, float height = 1.0f, uint32_t segmentsX = 1, uint32_t segmentsY = 1, bool invertFace = false);
		/// @brief ボックス型の頂点データを生成してリソースに割り当てる
		uint32_t AssignBox(ID3D12Device* device, bool invertFace = false);
		/// @brief リング型の頂点データを生成してリソースに割り当てる
		uint32_t AssignRing(ID3D12Device* device, float innerRadius = 0.5f, float outerRadius = 1.0f, uint32_t segments = 32, bool invertFace = false);

		/** @brief ハンドルから頂点バッファの数を取得 */
		const uint32_t GetVertexBufferCount(uint32_t handle) const;
		/** @brief ハンドルから頂点リソースを取得 */
		ID3D12Resource* GetModelVertexBuffer(const uint32_t& handle);
		/** @brief ハンドルから頂点データを取得 */
		VertexData* GetModelVertexBufferData(const uint32_t& handle);
		/** @brief ハンドルから頂点バッファビューを取得 */
		const D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView(const uint32_t& handle);

		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(const uint32_t& handle);

		/** @brief 名前からハンドルを取得 */
		uint32_t GetModelHandle(const std::string& modelName) const;
		/** @brief 指定した名前のモデルが既に読み込まれているか */
		bool HasModelHandle(const std::string& modelName) const;

		/** @brief 終了処理 */
		void Finalize();

	private:
		std::unordered_map<std::string, uint32_t> modelHandleMap_;
		std::deque<ModelData> modelDatas_;
		std::deque<VertexBuffer<VertexData>> modelVertexBuffers_;
	};

}
