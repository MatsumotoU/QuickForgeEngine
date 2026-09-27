#pragma once
#include "EngineDefines.h"

namespace QFE::SCENE {
	// 旧 SkyboxComponent の textureName を現行 ECS に移したもの。
	// resources/ からの相対パスでキューブマップ DDS を指定する。
	struct SkyBoxComponent {
		std::string textureName = "skybox/rostock_laage_airport_4k.dds";
		bool visible = true;
		std::string renderErrorMessage;

		QFE_REFLECT_BEGIN(SkyBoxComponent)
			QFE_REFLECT_MEMBER(textureName)
			QFE_REFLECT_MEMBER(visible)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(SkyBoxComponent)
}
