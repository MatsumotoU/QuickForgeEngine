#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <string>
#include <functional>

#include "graphics/dx12/GraphicEngineHandleTypes.h"

#include "math/matrix/Matrix4x4.h"

namespace QFE::GRAPHIC {
	/// @brief レイトレーシング用のインスタンス情報を表す構造体
    struct RaytracingInstance {
        BLASHandle blasHandle;          // 形状（どのBLASを使うか）
        QFE::MATH::Matrix4x4 worldMatrix; // 配置（どこに置くか）
    };

	/// @brief トップレベル加速構造（TLAS）を管理するクラス
    class TLAS final {
    public:
		/// @brief TLASを生成します,maxInstancesはTLASに登録するBLAS(インスタンス)の最大数です
        bool Create(ID3D12Device5* device5, UINT maxInstances);

		/// @brief TLASにBLASを登録します,毎度Buildを呼ぶ必要があります
        bool Build(
            ID3D12GraphicsCommandList4* commandList, const std::vector<RaytracingInstance>& instances,
            std::function<ID3D12Resource*(BLASHandle)> getResourceFunc);

        /// @brief TLASリソースを取得します
        ID3D12Resource* GetTLASResultBuffer() const { return tlasResultBuffer_.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> tlasResultBuffer_;// TLASの結果バッファ
		Microsoft::WRL::ComPtr<ID3D12Resource> tlasScratchBuffer_;// TLAS構築時の一時作業スペース（スクラッチバッファ）
        Microsoft::WRL::ComPtr<ID3D12Resource> instanceDescBuffer_; // インスタンス情報を載せるバッファ

		UINT maxInstances_ = 0; // TLASに登録するBLAS(インスタンス)の最大数
    };
}