#include "D3D12EngineSystems.h"

void QFE::FRAMEWORK::D3D12EngineSystems::SetGraphicEngine(std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> graphicEngine) {

}

QFE::GRAPHIC::IGraphicEngine* QFE::FRAMEWORK::D3D12EngineSystems::GetGraphicEngine() {
	return graphicEngine_->get();
}

QFE::INPUT::InputInterface* QFE::FRAMEWORK::D3D12EngineSystems::GetInputInterface() {
    return nullptr;
}

QFE::SCENE::SceneManager* QFE::FRAMEWORK::D3D12EngineSystems::GetSceneManager() {
    return nullptr;
}

QFE::FPSCounter* QFE::FRAMEWORK::D3D12EngineSystems::GetFPSCounter() {
    return nullptr;
}

QFE::GameWindowManager* QFE::FRAMEWORK::D3D12EngineSystems::GetGameWindowManager() {
    return nullptr;
}
