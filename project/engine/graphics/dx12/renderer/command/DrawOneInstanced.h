#pragma once
#include "IRenderCommand.h"
#include "dx12/GraphicEngineHandleTypes.h"
#include "dx12/vram/descriptors/ViewTypeFlags.h"

#include <functional>
#include <vector>

namespace QFE::GRAPHIC {
	/// @brief DrawOneInstancedでどのように描画するか設定する構造体
	struct DrawOneInstancedDesc {
		DirectXResourceHandle vertexbufferHandle;
		PSOHandle psoHandle;
		ID3D12PipelineState* pipelineState; // 描画に使用するPSOへのポインタ
		ID3D12RootSignature* rootSignature; // 描画に使用するルートシグネチャへのポインタ

		std::vector<DirectXResourceHandle> signatureResourceHandles;
		std::function <std::vector<D3D12_ROOT_PARAMETER_TYPE>(PSOHandle)> getRootParameterTypesFunc;
		std::function<D3D12_GPU_DESCRIPTOR_HANDLE(DirectXResourceHandle, ViewTypeFlags)> getGpuAddressFunc;
		std::function<D3D12_CPU_DESCRIPTOR_HANDLE(DirectXResourceHandle, ViewTypeFlags)> getCpuAddressFunc;
	};

	/// @brief インスタンス一つを描画するコマンド。ここではSignatureとPSOはセットされているものとします。
	class DrawOneInstanced final : public IRenderCommand {
	public:
		/// @brief 描画のプリミティブトポロジーを設定するコマンドを生成します。
		explicit DrawOneInstanced(DrawOneInstancedDesc desc);

		/// @brief コマンドリストに描画コマンドを発行
		void Execute(ID3D12GraphicsCommandList* commandList) override;

	private:
		DirectXResourceHandle vertexbufferHandle_;
		PSOHandle psoHandle_;
		std::vector<DirectXResourceHandle> signatureResourceHandles_;
		std::vector<D3D12_ROOT_PARAMETER_TYPE> rootParameterTypes_;
		std::function<D3D12_GPU_DESCRIPTOR_HANDLE(DirectXResourceHandle, ViewTypeFlags)> getGpuAddressFunc_;
		std::function<D3D12_CPU_DESCRIPTOR_HANDLE(DirectXResourceHandle, ViewTypeFlags)> getCpuAddressFunc_;
	};
}
