#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "EngineDefines.h"
#include "design-patterns/component/ComponentAutoRegistry.h"

namespace QFE::SCENE {
	/// @brief シーン切り替え先と切り替えリクエストを保持するコンポーネント。
	struct SceneChangeComponent {
		std::vector<std::string> sceneNames;
		uint32_t nextSceneNumber = 0;
		bool request = false;

		QFE_REFLECT_BEGIN(SceneChangeComponent)
			QFE_REFLECT_MEMBER(sceneNames)
			QFE_REFLECT_MEMBER(nextSceneNumber)
			QFE_REFLECT_MEMBER(request)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(SceneChangeComponent)
}
