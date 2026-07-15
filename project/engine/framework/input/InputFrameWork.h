#pragma once
#include <memory>

#define NOMINMAX
#include <Windows.h>
#include "input/InputInterface.h"

// Inputのフレームワーク
namespace QFE::FRAMEWORK {
	/// @brief 入力インターフェースを生成する関数,失敗した場合はnullptrを返す.ウィンドウのハンドルとインスタンスハンドルを引数に取ります.
	std::unique_ptr<QFE::INPUT::InputInterface> CreateInputInterface(HWND hwnd, HINSTANCE hInstance);
}