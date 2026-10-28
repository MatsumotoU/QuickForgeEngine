#pragma once
#include <string>
#include <stdint.h>
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief スクリプトコンポーネントの情報を保持する構造体
	struct ScriptComponent {
		/// @brief resources/Scriptsからの相対Luaファイル名。
		/// @note 旧DLLスクリプトとのシーン互換性を維持するためメンバー名はそのままにしている。
		std::string scriptFunctionName;
		bool enabled = true;
		int32_t executionOrder = 0;
		uint32_t scriptFunctionIndex = 0; ///< 実行時用。保存しない。

		QFE_REFLECT_BEGIN(ScriptComponent)
			QFE_REFLECT_MEMBER(scriptFunctionName)
			QFE_REFLECT_MEMBER(enabled)
			QFE_REFLECT_MEMBER(executionOrder)
		QFE_REFLECT_END()
	};
}
