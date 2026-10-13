#include "GraphicEngineCoreFramework.h"

#include "EngineDefines.h"

#include "graphics/IGraphicEngine.h"
#include "graphics/InternalApiType.h"
#include "graphics/D3D12GraphicEngine.h"

#include "window/IGameWindowManager.h"
#include "window/GameWindowManager.h"

namespace {
	// サポートOSと内部グラフィックAPIの種類の対応表
	std::unordered_map<QFE::INFO::SupportedOs, QFE::GRAPHIC::InternalApiType> supportedOsToInternalApiType = {
		{ QFE::INFO::SupportedOs::Windows, QFE::GRAPHIC::InternalApiType::DirectX12 }
	};
	
}

std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> QFE::FRAMEWORK::CreateGraphicEngine(
	const QFE::WINDOW::IGameWindowManager& windowManager,
	QFE::INFO::SupportedOs supportedOs, QFE::GRAPHIC::FeatureState featureState){

	// Windowマネージャの対応OS確認
	if (windowManager.GetSupportedOs() != supportedOs) {
		QFE_LOG("Error: The supported OS of the window manager does not match the specified supported OS.");
		return nullptr;
	}
	// Windowハンドルの取得
	const QFE::WINDOW::GameWindowManager& gameWindowManager = dynamic_cast<const QFE::WINDOW::GameWindowManager&>(windowManager);

	std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> graphicEngine;
	graphicEngine = std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>();
	return graphicEngine;
}
