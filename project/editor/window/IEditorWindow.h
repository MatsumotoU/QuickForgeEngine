#pragma once
#include <string>
#include <set>
#include <cstdint>

namespace QFE::EDITOR{
	class EditorCommandList;

	/// @brief エディタのウィンドウ基礎クラス
	class IEditorWindow {
	public:
		/// @brief ウィンドウのメンバ変数の初期化を行う
		virtual void Initialize() = 0;
		/// @brief ウィンドウの描画処理を行う,引数は毎フレーム初期化されている前提	
		virtual void Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) = 0;

		virtual std::string GetWindowName() = 0;
		virtual bool GetIsActive() = 0;
		virtual bool SetIsActive(bool isActive) = 0;
		virtual bool GetIsFocus() = 0;
	};
}
