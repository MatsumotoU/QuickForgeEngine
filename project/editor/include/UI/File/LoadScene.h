/**
 * @file LoadScene.h
 * @brief 既存のシーンをロードするためのブラウザUI
 */

#pragma once
#include "../IEditorUI.h"
#include <vector>
#include "utility/FileSystems/FileUtility.h"

/**
 * @class LoadScene
 * @brief プロジェクト内のシーンファイル一覧を表示し、選択したものを読み込むUI
 */
class LoadScene final : public IEditorUI {
public:
	LoadScene() = default;
	~LoadScene() = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;
	/** @brief 実行時処理 */
	void Run() override;
private:
	std::string currentScene_; ///< 現在ロードされているシーンパス
	std::vector<std::string> sceneList_; ///< シーンファイルの一覧
	int selected_; ///< 選択されているインデックス
};
