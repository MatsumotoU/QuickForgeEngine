/**
 * @file GameWindowManager.h
 * @brief 複数のウィンドウを統合管理するクラス
 */

#pragma once
#include <string>
#include <Windows.h>
#include "IGameWindowManager.h"
#include "IGameWindow.h"

namespace QFE {

	/**
	 * @class GameWindowManager
	 * @brief アプリケーション内のすべてのウィンドウを保持・管理するクラス
	 */
	class GameWindowManager final : public IGameWindowManager {
	public:
		GameWindowManager();
		~GameWindowManager() override = default;

		/** @brief マネージャの初期化 */
		void Initialize() override;
		/** @brief 全ウィンドウの更新 */
		void Update() override;
		/** @brief 全ウィンドウの描画 */
		void Draw() override;
		/** @brief マネージャの終了処理 */
		void Shutdown() override;

		/**
		 * @brief 新しいウィンドウを追加
		 * @param width 横幅
		 * @param height 縦幅
		 * @param windowName ウィンドウ名
		 */
		void AddWindow(const uint32_t& width, const uint32_t& height, const std::string& windowName) override;

		/** @brief いずれかのウィンドウがアクティブかどうかを判定 */
		bool IsWindowActive() const override;

		/**
		 * @brief ウィンドウ名からウィンドウハンドルを取得
		 * @param windowName 対象のウィンドウ名
		 * @return HWND ウィンドウハンドル
		 */
		HWND GetWindow(const std::string windowName) const;
	};

}
