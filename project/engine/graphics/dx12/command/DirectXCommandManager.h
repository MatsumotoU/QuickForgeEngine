#pragma once
#include <array>
#include "CommandExecutor.h"

namespace QFE::GRAPHIC {
	/// @brief DirectX 12のコマンドリストとコマンドキューを管理するクラスです。
	class DirectXCommandManager final {
	public:
		DirectXCommandManager() = default;
		~DirectXCommandManager() = default;
		void Initialize(ID3D12Device* device);
		void ResetCommandList();
		void ExecuteCommandList();

		/// @brief コマンドリストを取得します。Direct, Compute, Copyタイプのコマンドリストでのみ有効です。
		ID3D12GraphicsCommandList* GetCommandList(const D3D12_COMMAND_LIST_TYPE& type) const;
		/// @brief コマンドリスト4を取得します。Direct, Compute, Copyタイプのコマンドリストでのみ有効です。
		ID3D12GraphicsCommandList4* GetCommandList4(const D3D12_COMMAND_LIST_TYPE& type) const;
		/// @brief コマンドキューを取得します。Direct, Compute, Copyタイプのコマンドキューでのみ有効です。
		ID3D12CommandQueue* GetCommandQueue(const D3D12_COMMAND_LIST_TYPE& type) const;

	private:
		inline static const uint32_t kCommandExecutorCount_ = 1;
		std::array<CommandExecutor, kCommandExecutorCount_> commandExecutors_;
	};
}