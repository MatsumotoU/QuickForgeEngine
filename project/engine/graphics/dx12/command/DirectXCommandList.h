#pragma once
#include <d3d12.h>
#include <wrl.h>
namespace QFE::GRAPHIC {
	class DirectCommandList final {
	public:
		DirectCommandList() = default;
		~DirectCommandList() = default;

		void Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
		void Reset();

		/// @brief コマンドリストを取得します
		ID3D12GraphicsCommandList* GetCommandList() const;
		/// @brief コマンドリスト4を取得します
		ID3D12GraphicsCommandList4* GetCommandList4() const;

	private:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandList4_;
	};
}