/**
 * @file InspectorView.h
 * @brief 選択されたエンティティの詳細情報（コンポーネント）を表示・編集するパネル
 */

#pragma once
#include "../IEditorUI.h"
#include "utility/DebugTool/ImGui/DropDownFileList.h"
#include <vector>
#include <string>

/**
 * @class InspectorView
 * @brief 選択中のエンティティのトランスフォーム、モデル、スクリプト等のプロパティを編集するUI
 */
class InspectorView final : public IEditorUI {
public:
	InspectorView();
	~InspectorView() override = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;
private:
	uint32_t selectedEntityId_; ///< 選択されているエンティティID
	DropDownFileList scriptList_; ///< 利用可能なスクリプトのリスト
	std::vector<std::string> csharpScriptClasses_; ///< C#スクリプトクラスのリスト
	DropDownFileList modelList_; ///< 利用可能なモデルのリスト
	char scriptBuffer_[256]; ///< スクリプト名入力バッファ
	bool openScriptPopup_ = false; ///< スクリプト追加ポップアップの表示フラグ
};
