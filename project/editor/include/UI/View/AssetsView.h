/**
 * @file AssetsView.h
 * @brief プロジェクト内のアセット（モデル、テクスチャ、音声等）を表示・管理するパネル
 */

#pragma once
#include "../IEditorUI.h"
#include <unordered_map>
#include <functional>

#include "engine/include/utility/memory/IVariableLengthPoolContainer.h"

class AssetManager;

/** @brief ロード済みかファイル上のアセットか */
enum class LoadSpace {
	Memory, ///< メモリ上にロード済み
	File ///< ファイルシステム上
};

/** @brief 表示するアセットのカテゴリ */
enum class ViewHierarchy {
	Root,
	Images,
	Models,
	Materials,
	Shaders,
	Audio,
	Others
};

/**
 * @class AssetsView
 * @brief アセットブラウザ。アセットの閲覧、ロード状況の確認を行うUI
 */
class AssetsView : public IEditorUI {
public:
	AssetsView();
	~AssetsView() override = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;

	/** @brief 各カテゴリごとの描画関数 */
	void RootView();
	void ImagesView();
	void ModelsView();
	void MaterialsView();
	void ShadersView();
	void AudioView();
	void OthersView();

	/** @brief ファイル一覧の表示 */
	void FilesView();

private:
	AssetManager* assetManager; ///< アセットマネージャーへのポインタ
	ViewHierarchy currentHierarchy; ///< 現在表示中の階層
	std::unordered_map<ViewHierarchy, std::function<void()>> drawFunctions; ///< カテゴリごとの描画関数マップ
	std::unordered_map<ViewHierarchy, std::string> hierarchyNames; ///< カテゴリ名のマップ

	LoadSpace loadSpace_; ///< 現在のロード空間（メモリ/ファイル）

	uint32_t fileGH_; ///< ファイルアイコンのギングハンドル(GraphicHandle?)
	uint32_t arrowGH_; ///< 矢印アイコンのギングハンドル
};
