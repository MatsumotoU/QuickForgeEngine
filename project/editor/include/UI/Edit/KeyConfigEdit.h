/**
 * @file KeyConfigEdit.h
 * @brief キーコンフィグ（入力アクションとキーの紐付け）を編集するパネル
 */

#pragma once
#include "../IEditorUI.h"

/**
 * @class KeyConfigEdit
 * @brief ゲーム内で使用するキー入力をエディタ上で動的に変更・保存するためのUI
 */
class KeyConfigEdit final : public IEditorUI {
public:
	KeyConfigEdit() = default;
	~KeyConfigEdit() = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;
	/** @brief 実行時処理 */
	void Run() override;
private:
	char inputBuf_[256] = {}; ///< 入力バッファ
	bool isEditPopupOpen_ = false; ///< 編集ポップアップ表示フラグ
	std::string editActionName_; ///< 現在編集中のアクション名
	size_t editKeyIndex_ = 0; ///< 編集中のキーインデックス
	bool isAddPopupOpen_ = false; ///< 追加ポップアップ表示フラグ

	/** @brief キー設定の追加 */
	void AddKeyConfig(const char* actionName, uint32_t keyId);
	/** @brief キー設定のクリア */
	void ClearKeyConfig(const char* actionName);
};
