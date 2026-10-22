#pragma once

namespace QFE::EDITOR {
	class ComponentInspectorRegistry;
}

namespace QFE::GAME {
	/// @brief Shootingゲーム固有のInspector描画関数を登録する。
	bool RegisterShootingGameInspectors(EDITOR::ComponentInspectorRegistry& registry);
}
