#pragma once
#include <memory>

#define NOMINMAX
#include <Windows.h>

// 使用クラス
namespace QFE::INPUT {
	class InputInterface;
}

// Inputのフレームワーク
namespace QFE::FRAMEWORK {
	/// @brief 入力インターフェースを生成する関数,失敗した場合はnullptrを返す.ウィンドウのハンドルとインスタンスハンドルを引数に取ります.
	std::unique_ptr<QFE::INPUT::InputInterface> CreateInputInterface(const HWND& hwnd, const HINSTANCE& hInstance);
}