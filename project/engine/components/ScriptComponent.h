#pragma once
#include <string>
#include <stdint.h>
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief 旧シーンのデータ保持用。Lua 実行機能は削除済み。
	struct ScriptComponent {
		/// @brief 以前設定されたスクリプト名。現在は実行されない。
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
