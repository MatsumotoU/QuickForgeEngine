#pragma once

#include <string>
#include "EngineDefines.h"
#include "design-patterns/component/ComponentAutoRegistry.h"

namespace QFE::SCENE {
	struct SEComponent {
		std::string audioPath; ///< project/ からの相対パス、または絶対パス
		bool isPlayReqest = false;
		bool isLoop = false;
		float volume = 1.0f;

		QFE_REFLECT_BEGIN(SEComponent)
			QFE_REFLECT_MEMBER(audioPath)
			QFE_REFLECT_MEMBER(isPlayReqest)
			QFE_REFLECT_MEMBER(isLoop)
			QFE_REFLECT_MEMBER(volume)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(SEComponent)
}
