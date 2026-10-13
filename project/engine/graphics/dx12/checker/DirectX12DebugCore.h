#pragma once
#define NOMINMAX
#include <memory>
#include "D3DResourceLeakChecker.h"
#include "DebugLayer.h"

namespace QFE::GRAPHIC {
	class D3DResourceLeakChecker;
	class DebugLayer;

	class DirectX12DebugCore final {
	public:
		DirectX12DebugCore();

		/// @brief コピーコンストラクタとコピー代入演算子を削除して、インスタンスのコピーを禁止します。
		DirectX12DebugCore(const DirectX12DebugCore&) = delete;
		DirectX12DebugCore& operator=(const DirectX12DebugCore&) = delete;
		/// @brief ムーブコンストラクタとムーブ代入演算子を削除して、インスタンスのムーブを禁止します。
		DirectX12DebugCore(DirectX12DebugCore&&) = delete;
		DirectX12DebugCore& operator=(DirectX12DebugCore&&) = delete;

	private:
		std::unique_ptr<DebugLayer> debugLayer_;
		std::unique_ptr<D3DResourceLeakChecker> d3dResourceLeakChecker_;
	};
}  // namespace QFE
