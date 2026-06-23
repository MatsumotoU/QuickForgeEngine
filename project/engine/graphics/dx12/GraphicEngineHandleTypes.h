#pragma once
#include <cstdint>

namespace QFE::GRAPHIC {
	enum class DirectXResourceHandle : uint32_t { Invalid = UINT32_MAX };
	
	enum class VSHandle : uint32_t { Invalid = UINT32_MAX };
	enum class PSHandle : uint32_t { Invalid = UINT32_MAX };
	enum class ShaderPairHandle : uint32_t { Invalid = UINT32_MAX };
	enum class PSOHandle : uint32_t { Invalid = UINT32_MAX };
	enum class ComputePSOHandle : uint32_t { Invalid = UINT32_MAX };
	enum class RTPSOHandle : uint32_t { Invalid = UINT32_MAX };

	enum class ViewPortHandle : uint32_t { Invalid = UINT32_MAX };
	enum class ScissorRectHandle : uint32_t { Invalid = UINT32_MAX };
	enum class RenderTargetHandle : uint32_t { Invalid = UINT32_MAX ,SwapChain = 0};
	enum class DepthStencilHandle : uint32_t { Invalid = UINT32_MAX };
}