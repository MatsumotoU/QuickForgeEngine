#pragma once
#include <memory>

#define NOMINMAX
#include <Windows.h>

namespace QFE {
	namespace INPUT {
		class InputInterface;
	}
}

// Inputのフレームワーク
namespace QFE::FRAMEWORK {
	/// @brief 入力インターフェースを生成する関数,失敗した場合はnullptrを返す.ウィンドウのハンドルとインスタンスハンドルを引数に取ります.
	std::unique_ptr<QFE::INPUT::InputInterface> CreateInputInterface(HWND hwnd, HINSTANCE hInstance);
	/// @brief 入力インターフェースを更新する関数,失敗した場合はfalseを返す.入力インターフェースのポインタを引数に取ります.
	bool UpdateInputInterface(QFE::INPUT::InputInterface* inputInterface);
	/// @brief 入力インターフェースのフレーム終了処理を行う関数,失敗した場合はfalseを返す.入力インターフェースのポインタを引数に取ります.
	bool EndFrameInputInterface(QFE::INPUT::InputInterface* inputInterface);
}