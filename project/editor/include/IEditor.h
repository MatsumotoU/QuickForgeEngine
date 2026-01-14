/**
 * @file IEditor.h
 * @brief エディタの基本インターフェース
 */

#pragma once

/**
 * @class IEditor
 * @brief 全てのエディタクラスが継承する基底インターフェース
 */
class IEditor {
public:
	virtual ~IEditor() = default;
	/** @brief 初期化 */
	virtual void Initialize() = 0;
	/** @brief 更新 */
	virtual void Update() = 0;
	/** @brief 描画 */
	virtual void Draw() = 0;
};
