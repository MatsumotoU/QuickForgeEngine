#pragma once
#include "IRenderCommand.h"
#include "../../GraphicEngineHandleTypes.h"
#include "../../ViewTypeFlags.h"

#include <functional>
#include <vector>

namespace QFE::GRAPHIC {
	/// @brief DrawOneInstancedでどのように描画するか設定する構造体
	struct DrawOneInstancedDesc {
		INTERNAL::DirectXResourceHandle vertexbufferHandle;
		PSOHandle psoHandle;
		std::vector<INTERNAL::DirectXResourceHandle> signatureResourceHandles;
		std::function <std::vector<D3D12_ROOT_PARAMETER_TYPE>(PSOHandle)> getRootParameterTypesFunc;
		std::function<D3D12_GPU_DESCRIPTOR_HANDLE(INTERNAL::DirectXResourceHandle, INTERNAL::ViewTypeFlags)> getGpuAddressFunc;
		std::function<D3D12_CPU_DESCRIPTOR_HANDLE(INTERNAL::DirectXResourceHandle, INTERNAL::ViewTypeFlags)> getCpuAddressFunc;
	};

	/// @brief インスタンス一つを描画するコマンド。ここではSignatureとPSOはセットされているものとします。
	class DrawOneInstanced final : public INTERNAL::IRenderCommand {
	public:
		/// @brief 描画のプリミティブトポロジーを設定するコマンドを生成します。
		explicit DrawOneInstanced(DrawOneInstancedDesc desc);

		/// @brief コマンドリストに描画コマンドを発行
		void Execute(ID3D12GraphicsCommandList* commandList) override;

	private:
		INTERNAL::DirectXResourceHandle vertexbufferHandle_;
		PSOHandle psoHandle_;
		std::vector<INTERNAL::DirectXResourceHandle> signatureResourceHandles_;
		std::vector<D3D12_ROOT_PARAMETER_TYPE> rootParameterTypes_;
		std::function<D3D12_GPU_DESCRIPTOR_HANDLE(INTERNAL::DirectXResourceHandle, INTERNAL::ViewTypeFlags)> getGpuAddressFunc_;
		std::function<D3D12_CPU_DESCRIPTOR_HANDLE(INTERNAL::DirectXResourceHandle, INTERNAL::ViewTypeFlags)> getCpuAddressFunc_;
	};
}
