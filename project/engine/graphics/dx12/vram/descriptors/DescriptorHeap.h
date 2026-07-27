#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <queue>

#include "DescriptorHeapInfo.h"
#include "DescriptorHandles.h"

namespace QFE::GRAPHIC {
	/// @brief ディスクリプタヒープ自体とヒープの空きスロットの管理を行うクラス
	class DescriptorHeap {
	public:
		/// @brief ディスクリタヒープを生成する関数
		void Create(ID3D12Device* device, DescriptorHeapInfo info);

		/// @brief ディスクリタヒープからディスクリタハンドルを複数作成する関数
		[[nodiscard]] std::vector<DescriptorHandles> CreateDescriptorHandles(uint32_t count);

		/// @brief ある位置のディスクリタハンドルを取得する
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(uint32_t index) const;
		/// @brief ある位置のディスクリタハンドルを取得する
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(uint32_t index) const;
		/// @brief ディスクリタヒープの先頭のCPUディスクリタハンドルを取得する関数
		[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const;
		/// @brief ディスクリタヒープの先頭のGPUディスクリタハンドルを取得する関数
		[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const;
		/// @brief ディスクリタのサイズを取得する関数
		[[nodiscard]] UINT GetDescriptorSize() const;
		/// @brief ヒープの空きスロットのインデックスを取得する関数
		[[nodiscard]] uint32_t GetNextFreeDescriptorIndex();
		/// @brief ディスクリタヒープを取得する関数
		[[nodiscard]] ID3D12DescriptorHeap* GetDescriptorHeap() const;
		/// @brief ディスクリタヒープがシェーダーから参照可能かどうかを取得する関数
		[[nodiscard]] const bool IsShaderVisible() const;

	private:
		/// @brief ディスクリタヒープが生成されているかどうかをチェックする関数
		void CheckCreated() const;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;// ディスクリタヒープ
		uint32_t nextFreeDescriptorIndex_ = 0;// 次に割り当てるディスクリタのインデックス
		bool isCreated_ = false;// ディスクリタヒープが生成されているかどうか
		DescriptorHeapInfo DescriptorHeapInfo_;// ディスクリタヒープの情報
		bool isShaderVisible_ = false;// シェーダーから参照可能かどうか
	};
}