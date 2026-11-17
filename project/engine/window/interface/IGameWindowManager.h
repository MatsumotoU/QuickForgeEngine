#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include "info/SupportedOs.h"

namespace QFE::WINDOW {
	/// @brief ウィンドウへドロップされたファイルとスクリーン座標。
	struct DroppedFile { std::string path; int x = 0; int y = 0; };

	class IGameWindow;

	class IGameWindowManager {
	public:
		virtual ~IGameWindowManager() = default;
		virtual void Initialize() = 0;
		virtual void Update() = 0;
		virtual void Draw() = 0;
		virtual void Shutdown() = 0;
		virtual void AddWindow(const uint32_t& width, const uint32_t& height, const std::string& windowName) = 0;
		virtual bool IsWindowActive() const = 0;
		/// @brief 未処理のファイルドロップを取得し、キューから取り除く。
		virtual std::vector<DroppedFile> ConsumeDroppedFiles() { return {}; }

		/// @brief ゲームウィンドウマネージャのサポートOSを取得します.
		virtual QFE::INFO::SupportedOs GetSupportedOs() const = 0;

	protected:
		std::vector<std::unique_ptr<IGameWindow>> windows;
	};

}
