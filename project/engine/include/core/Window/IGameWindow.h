#pragma once
#include <cstdint>
#include <string>

namespace QFE {

	class IGameWindow {
	public:
		IGameWindow() = default;
		virtual ~IGameWindow() = default;
		/// <summary>
		/// ウィンドウの初期匁E
		/// </summary>
		virtual void Initialize(const uint32_t& width, const uint32_t& height, const std::string& windowName) = 0;
		/// <summary>
		/// ウィンドウの更新処琁E
		/// </summary>
		virtual void Update() = 0;
		/// <summary>
		/// ウィンドウの描画処琁E
		/// </summary>
		virtual void Draw() = 0;
		/// <summary>
		/// ウィンドウの終亁EE琁E
		/// </summary>
		virtual void Shutdown() = 0;
		/// <summary>
		/// ウィンドウがアクチEブかどぁEを確認すめE
		/// </summary>
		/// <returns></returns>
		virtual bool IsWindowActive() const = 0;
		/// <summary>
		/// ウィンドウの名前を取得すめE
		/// </summary>
		/// <returns></returns>
		virtual std::string GetWindowName() const = 0;

	protected:
		uint32_t clientWidth_;
		uint32_t clientHeight_;
	};

}
