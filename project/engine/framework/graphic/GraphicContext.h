#pragma once

// This header is an implementation detail of the framework layer.
// Applications should include D3D12GraphicFrameWork.h instead.
#include "graphics/D3D12GraphicEngine.h"

namespace QFE::FRAMEWORK {
	/// The framework-facing graphic object.
	///
	/// The implementation currently uses the DirectX 12 engine internally, but
	/// application code only sees this type through a forward declaration.
	class GraphicContext final : public QFE::GRAPHIC::D3D12GraphicEngine {
	public:
		using QFE::GRAPHIC::D3D12GraphicEngine::D3D12GraphicEngine;
	};
}
