#include "GraphicEngineCoreFramework.h"

#include "EngineDefines.h"

#include "graphics/IGraphicEngine.h"
#include "graphics/InternalApiType.h"
#include "graphics/D3D12GraphicEngine.h"

#include "window/IGameWindowManager.h"
#include "window/GameWindowManager.h"

#include <unordered_map>

namespace {
	// サポートOSと内部グラフィックAPIの種類の対応表
	const std::unordered_map<QFE::INFO::SupportedOs, QFE::GRAPHIC::InternalApiType> supportedOsToInternalApiType = {
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
	const auto apiType = supportedOsToInternalApiType.find(supportedOs);
	if (apiType == supportedOsToInternalApiType.end()) {
		QFE_LOG("Error: No graphics API is registered for the specified OS.");
		return nullptr;
	}

	if (apiType->second == QFE::GRAPHIC::InternalApiType::DirectX12) {
		const auto* gameWindowManager =
			dynamic_cast<const QFE::WINDOW::GameWindowManager*>(&windowManager);
		if (gameWindowManager == nullptr) {
			QFE_LOG("Error: DirectX 12 requires a Windows game window manager.");
			return nullptr;
		}
		const HWND mainWindow = gameWindowManager->GetMainWindow();
		if (mainWindow == nullptr) {
			QFE_LOG("Error: The window manager does not contain a main window.");
			return nullptr;
		}
		return std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(
			mainWindow, featureState);
	}

	QFE_LOG("Error: The selected graphics API is not implemented.");
	return nullptr;
}
