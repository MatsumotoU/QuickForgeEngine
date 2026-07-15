#pragma once
#include "IEditorWindow.h"
#include <set>
#include <stdint.h>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	/// @brief Entityを階層構造で表示するウィンドウ	
	class Hierarchy final : public IEditorWindow {
	public:
		/// @brief このエンティティマネージャーのEntityを階層構造で表示するウィンドウを作成する
		Hierarchy(EntityManager* entityManager);
		
		/// @brief ウィンドウの初期化処理
		void Initialize() override;
		/// @brief ウィンドウの描画処理
		void Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) override;

		/// @brief ウィンドウの名前を取得する
		std::string GetWindowName() override;
		/// @brief ウィンドウがアクティブかどうかを取得する
		bool GetIsActive() override;
		/// @brief ウィンドウのアクティブ状態を設定する
		bool SetIsActive(bool isActive) override;
		/// @brief ウィンドウがフォーカスされているかどうかを取得する
		bool GetIsFocus() override;

	private:
		EntityManager* entityManager_;
		bool isActive_;
		std::set<uint32_t> hierarchySelectedEntities_;
		bool isFocus_;
	};
}
