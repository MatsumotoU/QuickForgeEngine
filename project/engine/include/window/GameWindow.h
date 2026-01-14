/**
 * @file GameWindow.h
 * @brief 個別のウィンドウを管理するクラス
 */

#pragma once
#include <Windows.h>
#include <memory>
#include "engine/include/core/Window/IGameWindow.h"
#include "WindowGenerater/WindowGenerater.h"
#include "WindowEventsManager/WindowEventsManager.h"

/**
 * @class GameWindow
 * @brief Windows APIを使用して単一のウィンドウを生成・管理するクラス
 */
class GameWindow final:public IGameWindow{
public:
    /**
     * @brief ウィンドウの初期化と生成
     * @param width ウィンドウの横幅
     * @param height ウィンドウの縦幅
     * @param windowName ウィンドウのタイトル
     */
	void Initialize(const uint32_t& width, const uint32_t& height, const std::string& windowName)override;
    
    /** @brief ウィンドウの更新（メッセージ処理など） */
	void Update()override;
    /** @brief 描画処理 */
	void Draw()override;
    /** @brief 終了処理 */
	void Shutdown()override;
    /** @brief ウィンドウがアクティブかどうかを判定 */
	bool IsWindowActive() const override;
    /** @brief ウィンドウ名を取得 */
	std::string GetWindowName() const override;

    /** @brief ウィンドウハンドル(HWND)を取得 */
	HWND GetHwnd() const;

private:
	HWND hwnd_ = nullptr;
	WNDCLASS wc_ = {};
	WindowConfigData configData_;
	WNDPROC proc_ = WindowEventsManager::WindowProc;
	std::unique_ptr<WindowEventsManager> eventManagerPtr_;
};
