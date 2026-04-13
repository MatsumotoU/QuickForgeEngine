#pragma once
/**
 * @file LoadProject.h
 * @brief 既存のプロジェクトをロードするためのブラウザUI
 */

#pragma once
#include "../IEditorUI.h"
#include <vector>
#include "utility/FileSystems/FileUtility.h"
namespace QFE {
	/**
	 * @class LoadProject
	 * @brief プロジェクト内のシーンファイル一覧を表示し、選択したものを読み込むUI
	 */
	class LoadProject final : public IEditorUI {
	public:
		LoadProject() = default;
		~LoadProject() = default;
		/** @brief 初期化 */
		void Initialize() override;
		/** @brief 更新 */
		void Update() override;
		/** @brief 描画 */
		void Draw() override;
		/** @brief 実行時処理 */
		void Run() override;
	private:
		void LoadProjectDirectory(); ///< プロジェクトのディレクトリをロードする関数
		std::vector<std::string> ignoreProjectList_;///< ロードしないプロジェクトのリスト

		std::string currentProject_; ///< 現在ロードされているシーンパス
		std::vector<std::string> projectList_; ///< シーンファイルの一覧
		int selected_; ///< 選択されているインデックス
	};
}