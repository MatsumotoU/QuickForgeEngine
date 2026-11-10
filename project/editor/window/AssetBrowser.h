#pragma once
#include "IEditorWindow.h"
#include <map>
#include <string>
#include <vector>

namespace QFE::ASSET { class AssetManager; }
namespace QFE::WINDOW { class IGameWindowManager; }

namespace QFE::EDITOR {
	/// @brief 選択中のプロジェクトにあるアセットをディレクトリごとに表示する。
	class AssetBrowser final : public IEditorWindow {
	public:
		/// @brief アセット一覧を初期化する。
		AssetBrowser(ASSET::AssetManager& assets, WINDOW::IGameWindowManager& windowManager);
		/// @brief ウィンドウとアセット一覧を初期化する。
		void Initialize() override;
		/// @brief アセットの相対パスをディレクトリ別に描画する。
		void Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) override;
		/// @brief ウィンドウ名を取得する。
		std::string GetWindowName() override;
		/// @brief ウィンドウが表示中か取得する。
		bool GetIsActive() override;
		/// @brief ウィンドウの表示状態を設定する。
		bool SetIsActive(bool isActive) override;
		/// @brief ウィンドウにフォーカスがあるか取得する。
		bool GetIsFocus() override;
	private:
		/// @brief プロジェクトの assets ディレクトリを再走査する。
		void Refresh();

		
		std::map<std::string, std::vector<std::string>> groups_;
		ASSET::AssetManager* assets_;
		WINDOW::IGameWindowManager* windowManager_;
		std::string status_;
		std::string selectedPath_;
		bool isActive_ = true;
		bool isFocus_ = false;
	};
}
