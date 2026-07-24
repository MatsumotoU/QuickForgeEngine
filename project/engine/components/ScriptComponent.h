#pragma once
#include <string>
#include <stdint.h>
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief スクリプトコンポーネントの情報を保持する構造体
	struct ScriptComponent {
		std::string scriptFunctionName; ///< スクリプト関数の名前
		uint32_t scriptFunctionIndex; ///< スクリプト関数のインデックス,目録リストの中で何番目かを示すので動的に変わる.保存はしない

		QFE_REFLECT_BEGIN(ScriptComponent)
			QFE_REFLECT_MEMBER(scriptFunctionName)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(ScriptComponent)
}