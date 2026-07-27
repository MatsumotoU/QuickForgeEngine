#pragma once

namespace QFE::GUI {
	/// @brief GUIマネージャーのインターフェース
	class IGuiManager {
	public:
		virtual ~IGuiManager() = default;
		virtual void Update() = 0;
		virtual void PreDraw() = 0;
		virtual void PostDraw() = 0;
		virtual void Shutdown() = 0;
	};
}