#pragma once
#include <cstdint>

namespace QFE::GRAPHIC::INTERNAL {
    /// @brief ディスクリプタのビューを表すフラグ
    enum class ViewTypeFlags : uint32_t {
        None = 0x0000,
        RenderTargetView = 0x0001, // RTV
        ShaderResourceView = 0x0002, // SRV
        DepthStencilView = 0x0004, // DSV
        UnorderedAccessView = 0x0008, // UAV
		ConstantBufferView = 0x0010, // CBV
    };

    // ORビット演算子のオーバーロード
    inline ViewTypeFlags operator|(ViewTypeFlags lhs, ViewTypeFlags rhs) {
        return static_cast<ViewTypeFlags>(
            static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs)
            );
    }
    // ANDビット演算子のオーバーロード
    inline ViewTypeFlags operator&(ViewTypeFlags lhs, ViewTypeFlags rhs) {
        return static_cast<ViewTypeFlags>(
            static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs)
            );
    }
}
