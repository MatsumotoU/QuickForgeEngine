/**
 * @file IEngineCore.h
 * @brief インターフェース：エンジンコアの基底クラス
 */

#pragma once

/**
 * @class IEngineCore
 * @brief エンジンの基本実行サイクルを定義するインターフェースクラス
 */
class IEngineCore {
public:
	IEngineCore() = default;
	virtual ~IEngineCore() = default;
	
	/**
	 * @brief エンジンの初期化処理
	 */
	virtual void Initialize() = 0;

	/**
	 * @brief エンジンのメインループ
	 */
	virtual void MainLoop() = 0;

	/**
	 * @brief エンジンの終了処理
	 */
	virtual void Shutdown() = 0;
};
