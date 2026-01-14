/**
 * @file SceneView.h
 * @brief シーンの3D/2D表示とギズモによる操作を行うパネル
 */

#pragma once
#include "../IEditorUI.h"
#include "core/Math/Vector/Vector3.h"

/**
 * @class SceneView
 * @brief ゲーム画面のレンダリング結果を表示し、デバッグカメラやギズモでの操作を可能にするUI
 */
class SceneView : public IEditorUI {
public:
	SceneView();
	~SceneView() override = default;
	/** @brief 初期化 */
	void Initialize() override;
	/** @brief 更新 */
	void Update() override;
	/** @brief 描画 */
	void Draw() override;

private:
	/** @brief デバッグカメラの制御 */
	void DebugCameraControl();
	/** @brief ギズモの更新 */
	void UpdateGizmo();

	uint32_t selectEntityId_; ///< 選択中のエンティティID
	bool isDrawGizmo_; ///< ギズモ表示フラグ

	float cameraMoveT_; ///< カメラ移動の補間値
	Vector3 startPos_; ///< カメラ移動開始座標
	Vector3 targetRotate_; ///< カメラ目標回転角

	bool isActiveCamera_; ///< デバッグカメラが有効か
	bool isDrawGrid_; ///< グリッド表示フラグ
	Vector3 anchorPoint_; ///< 操作のアンカーポイント
	float mouseSensitivity_; ///< マウス感度
};
