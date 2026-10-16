#pragma once

namespace QFE::GUI {
	/// @brief GUIマネージャーのインターフェース
	class IGuiManager {
	public:
		IGuiManager() = default;
		IGuiManager(const IGuiManager&) = default;
		IGuiManager& operator=(const IGuiManager&) = default;
		IGuiManager(IGuiManager&&) noexcept = default;
		IGuiManager& operator=(IGuiManager&&) noexcept = default;
		virtual ~IGuiManager() = default;

		virtual void Update() = 0;
		virtual void PreDraw() = 0;
		virtual void PostDraw() = 0;
		virtual void Shutdown() = 0;
	};
}
