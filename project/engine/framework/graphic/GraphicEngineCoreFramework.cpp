#include "GraphicEngineCoreFramework.h"

#include "graphics/IGraphicEngine.h"
#include "graphics/InternalApiType.h"
#include "graphics/D3D12GraphicEngine.h"

bool QFE::FRAMEWORK::CreateGraphicEngine(QFE::GRAPHIC::IGraphicEngine* engine, QFE::GRAPHIC::InternalApiType apiType) {
    return false;
}

std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> QFE::FRAMEWORK::CreateGraphicEngine(QFE::GRAPHIC::InternalApiType apiType)
{
	std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> engine;
	engine = std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(nullptr);
    return std::unique_ptr<QFE::GRAPHIC::IGraphicEngine>();
}