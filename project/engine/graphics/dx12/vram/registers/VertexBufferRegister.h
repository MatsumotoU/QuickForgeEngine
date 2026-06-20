#pragma once
#include "memory/UniqueContainer.h"
#include "../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "dx12/GraphicEngineHandleTypes.h"

#include <d3d12.h>
#include <functional>
#include <vector>
#include <string>

namespace QFE::GRAPHIC::INTERNAL {
	/// @brief 頂点バッファ管理クラスの初期化に必要な情報と関数をまとめた構造体
	struct VertexBufferRegisterInitializeInfo {
		std::function<DirectXResourceHandle(size_t)> createBufferFunc;// 頂点バッファを作成する関数
		std::function<VertexData* (DirectXResourceHandle)> getBufferDataFunc;// 頂点バッファのデータを取得する関数
		std::function< D3D12_GPU_VIRTUAL_ADDRESS(DirectXResourceHandle)> getBufferGpuAddressFunc;// 頂点バッファのGPU仮想アドレスを取得する関数
	};

	/// @brief 頂点バッファの情報をまとめた構造体
	struct VertexBufferInfo {
		DirectXResourceHandle bufferHandle;// 頂点バッファのリソースハンドル
		D3D12_VERTEX_BUFFER_VIEW bufferView;// 頂点バッファビュー
		uint32_t vertexCount;// 頂点数
	};

	/// @brief 頂点バッファのリソースとハンドルを管理するクラス
	class VertexBufferRegister final {
	public:
		/// @brief 初期化
		void Initialize();
		/// @brief 終了処理
		void Finalize();

		/// @brief 頂点データから頂点バッファを作成し、ハンドルを返す
		QFE::GRAPHIC::VertexBufferHandle Assign(const std::vector<VertexData>& vertex, const std::string& name);
		/// @brief 頂点データハンドルから頂点バッファのマップを取得する
		VertexData* GetVertexBufferData(QFE::GRAPHIC::VertexBufferHandle handle);
		/// @brief 頂点データハンドルから頂点バッファビューを取得する
		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(QFE::GRAPHIC::VertexBufferHandle handle);

	private:
		VertexBufferRegisterInitializeInfo info_;// 初期化情報
		UniqueContainer<VertexBufferInfo> vertexBufferMap_;// 頂点データのロード名とハンドルのマップ
	};
}
