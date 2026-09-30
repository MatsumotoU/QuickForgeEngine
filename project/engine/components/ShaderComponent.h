#pragma once
#include "EngineDefines.h"

namespace QFE::SCENE {
	struct ShaderComponent {
		bool receiveShadow = true;
		bool castShadow = true;

		QFE_REFLECT_BEGIN(ShaderComponent)
			QFE_REFLECT_MEMBER(receiveShadow)
			QFE_REFLECT_MEMBER(castShadow)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(ShaderComponent)
}
