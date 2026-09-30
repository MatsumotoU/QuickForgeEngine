#pragma once

#include <string>
#include "EngineDefines.h"
#include "design-patterns/component/ComponentAutoRegistry.h"

namespace QFE::SCENE {
	struct BGMComponent {
		std::string audioPath; ///< project/ からの相対パス、または絶対パス
		bool isPlayReqest = false;
		bool isLoop = true;
		bool playOnSceneStart = false;
		float volume = 1.0f;

		QFE_REFLECT_BEGIN(BGMComponent)
			QFE_REFLECT_MEMBER(audioPath)
			QFE_REFLECT_MEMBER(isPlayReqest)
			QFE_REFLECT_MEMBER(isLoop)
			QFE_REFLECT_MEMBER(playOnSceneStart)
			QFE_REFLECT_MEMBER(volume)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(BGMComponent)
}
