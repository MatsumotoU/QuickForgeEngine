#pragma once
#include <d3d12.h>
namespace QFE::GRAPHIC {
	/// @brief ディスクリプタヒープの情報を保持する構造体
	class DescriptorHeapInfo {
	public:
		/// @brief ディスクリタ生成設定の妥当性をチェックする関数
		bool CheckValid() const;

		D3D12_DESCRIPTOR_HEAP_TYPE heapType;// ディスクリタヒープの種類
		UINT numDescriptors;// ディスクリタの数
		UINT descriptorSize;// ディスクリタのサイズ
		bool shaderVisible;// シェーダーから参照可能かどうか
	};
}