#pragma once
#include "DescriptorHeap.h"
#include "DescriptorBlockTypes.h"
#include <unordered_map>
#include <map>

namespace QFE::GRAPHIC {

	/// @brief ディスクリタヒープの種類を表す列挙型
	enum class DescriptorHeapType {
		RTV,
		SRV,
		DSV
	};

	/// @brief 連続したディスクリタブロックを管理する構造体
	struct DescriptorBlock {
		uint32_t nextFreeIndex = 0; // 次に割り当てるディスクリタのインデックス
		std::vector<DescriptorHandles> handles_;
	};

	/// @brief 複数のディスクリタヒープを管理するクラス
	class DescriptorHeapManager final {
	public:
		/// @brief ディスクリタヒープを初期化する関数
		void Initialize(ID3D12Device* device);
		/// @brief RTVディスクリタヒープからRTVディスクリタを割り当てる関数
		[[nodiscard]] DescriptorHandles AssignRtvHeap(
			ID3D12Device* device, ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc);
		/// @brief SRVディスクリタヒープからSRVディスクリタを割り当てる関数
		[[nodiscard]] DescriptorHandles AssignSrvHeap(
			ID3D12Device* device, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
		/// @brief DSVディスクリタヒープからDSVディスクリタを割り当てる関数
		[[nodiscard]] DescriptorHandles AssignDsvHeap(
			ID3D12Device* device, ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc);
		/// @brief SRVディスクリタヒープからUAVディスクリタを割り当てる関数,SRVディスクリプタヒープにUAVディスクリタを割り当てるため、SRVディスクリタヒープのハンドルを返す
		[[nodiscard]] DescriptorHandles AssignUavHeap(
			ID3D12Device* device, ID3D12Resource* resource, 
			ID3D12Resource* counterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC* desc);

		/// @brief 連続したディスクリタブロックにディスクリタを割り当てる関数,
		[[nodiscard]] DescriptorHandles AssignTexture(
			ID3D12Device* device, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);

		/// @brief 空のディスクリタヒープを作成する関数
		[[nodiscard]] DescriptorHandles CreateEmptyHeapHandle(DescriptorHeapType type);
		/// @brief コマンドリストにディスクリタヒープを登録する関数
		void RegisterDescriptorHeaps(ID3D12GraphicsCommandList* commandList) const;

		/// @brief ディスクリタヒープを取得する関数
		ID3D12DescriptorHeap* GetDescriptorHeap(DescriptorHeapType type) const;

	private:
		const uint32_t kMaxRtvDescriptors = 128; // 最大RTVディスクリプタ数
		const uint32_t kMaxSrvDescriptors = 512; // 最大SRVディスクリプタ数
		const uint32_t kMaxDsvDescriptors = 32;  // 最大DSVディスクリプタ数

		std::unordered_map<DescriptorHeapType, DescriptorHeap> descriptorHeaps_; // ディスクリタヒープの種類ごとのマップ
		std::map<DescriptorBlockType, DescriptorBlock> descriptorBlocks_; // 連続したディスクリプタブロックの管理
	};
}