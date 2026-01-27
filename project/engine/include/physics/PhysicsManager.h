/**
 * @file PhysicsManager.h
 * @brief 物理演算の更新を管理するクラス
 */

#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "Force.h"

namespace QFE {

	/**
	 * @class PhysicsManager
	 * @brief エンティティ間の物理的な挙動（速度、加速度、重力等）を統合管理するシングルトンクラス
	 */
	class PhysicsManager : public Singleton<PhysicsManager> {
		friend class Singleton<PhysicsManager>;
		~PhysicsManager() override = default;

	public:
		/** @brief 初期化 */
		void Initialize();
		/** @brief 物理演算の更新（毎フレーム実行） */
		void Update();
		/** @brief 終了処理 */
		void Finalize();
	};

}
