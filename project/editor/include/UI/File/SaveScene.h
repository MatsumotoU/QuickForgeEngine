/**
 * @file SaveScene.h
 * @brief 現在のシーンをファイルに保存するためのダイアログUI
 */

#pragma once
#include "../IEditorUI.h"

/**
 * @class SaveScene
 * @brief シーン名を入力し、現在のエンティティ構成をJSONとして保存するUI
 */
class SaveScene final : public IEditorUI {
public:
	SaveScene() = default;
	~SaveScene() = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;
	/** @brief 実行時処理 */
	void Run() override;
private:
	std::string sceneName_; ///< 保存するシーン名
};
