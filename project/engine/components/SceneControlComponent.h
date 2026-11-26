#pragma once
#include "EngineDefines.h"
#include <string>

namespace QFE::SCENE {
	/// イベント等で独立シーンを読み込み、更新と表示を切り替える。
	struct SceneControlComponent {
		// Main は起動時からあるシーン。それ以外は読み込むシーンの識別名。
		std::string sceneName = "Main";
		std::string scenePath;
		bool updating = true;
		bool visible = true;
		bool request = false;
		// true が続く間は繰り返し実行せず、false を経て再び要求できる。
		bool requestHandled = false;

		QFE_REFLECT_BEGIN(SceneControlComponent)
			QFE_REFLECT_MEMBER(sceneName)
			QFE_REFLECT_MEMBER(scenePath)
			QFE_REFLECT_MEMBER(updating)
			QFE_REFLECT_MEMBER(visible)
			QFE_REFLECT_MEMBER(request)
		QFE_REFLECT_END()
	};
}
