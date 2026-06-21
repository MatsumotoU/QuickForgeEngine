#pragma once
#include <d3d12.h>
#include <dxgidebug.h>
#include<dxgi1_3.h>

#include "EngineDefines.h"

namespace QFE::GRAPHIC {
	class D3DResourceLeakChecker final {
	public:
		~D3DResourceLeakChecker() {
			// * 終亁E��のエラー処琁E* //
			QFE_LOG("=====D3DResourceLeakCheck=====\n");
			IDXGIDebug1* debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
				debug->Release();
			}
		}
	};
}
