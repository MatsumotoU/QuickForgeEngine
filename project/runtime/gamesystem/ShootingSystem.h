#pragma once
#include "framework/application/WindowsEngineFramework.h"
#include "framework/application/WindowsEngineResources.h"

#include "../resources/shaders/shaderStructs/hlslTypeToCpp.h"

namespace QFE::GAMESYSTEM {
	/// @brief プレイヤーの自動スクロール処理を行うシステム関数。
	bool AutoScrollSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief プレイヤーの移動、回転、射撃処理を行うシステム関数。
	bool ShootingPlayerSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief 敵のスポーン処理を行うシステム関数。
	bool EnemySpawnerSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief 敵の射撃処理を行うシステム関数。
	bool ShootingEnemySystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief 弾の移動、寿命、コライダー処理を行うシステム関数。
	bool BulletUpdateSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief コライダーの処理を行うシステム関数。
	bool CollisionSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief プレイヤーの自動トラッキング処理を行うシステム関数。
	bool PlayerTrackingSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief 移動制限コンポーネントの処理を行うシステム関数。
	bool MoveLimitSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief 弾丸エミッターの処理を行うシステム関数。
	bool BulletEmitterSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);

	/// @brief 入力または一定間隔で弾丸エミッターへ発射要求を送る。
	bool BulletEmitterTriggerSystem(
		QFE::FRAMEWORK::WindowsQuickForgeEngineSystems& systems,
		QFE::FRAMEWORK::WindowsEngineResources& resources, float deltaTime);
}
