#include "scene/SceneManager.h"
#include "components/CameraComponent.h"
#include "components/DirectionalLightComponent.h"
#include "components/ObjectInfoComponent.h"
#include "components/TransformComponent.h"
#include "components/SceneChangeComponent.h"
#include "components/SceneControlComponent.h"
#include "components/SceneSpawnComponent.h"
#include "components/EventComponent.h"
#include "components/InputControlComponent.h"
#include "framework/scene/SceneChangeSystem.h"
#include "framework/scene/SceneCoreFramework.h"
#include "framework/scene/CollisionTriggerSystem.h"
#include "framework/scene/EventSystem.h"
#include "assetfactory/game-data/GameProjectPaths.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <iostream>

int main(int argc, char** argv) {
	if (argc != 2) return 1;
	const std::filesystem::path filePath(argv[1]);
	{
		std::ofstream file(filePath);
		file << R"({"entities":[]})";
		if (!file) return 2;
	}
	auto registry = std::make_shared<QFE::StaticComponentRegistry>();
	registry->RegisterComponent<QFE::SCENE::CameraComponent>("CameraComponent");
	registry->RegisterComponent<QFE::SCENE::DirectionalLightComponent>("DirectionalLightComponent");
	registry->RegisterComponent<QFE::SCENE::ObjectInfoComponent>("ObjectInfoComponent");
	registry->RegisterComponent<QFE::SCENE::TransformComponent>("TransformComponent");
	registry->RegisterComponent<QFE::SCENE::SceneChangeComponent>("SceneChangeComponent");
	registry->RegisterComponent<QFE::SCENE::SceneControlComponent>("SceneControlComponent");
	registry->RegisterComponent<QFE::SCENE::SceneSpawnComponent>("SceneSpawnComponent");
	registry->RegisterComponent<QFE::SCENE::EventComponent>("EventComponent");
	registry->RegisterComponent<QFE::COMPONENTS::InputControlComponent>("InputControlComponent");
	registry->Freeze();
	QFE::SCENE::SceneManager scenes(registry);
	scenes.Initialize();
	const auto id = scenes.LoadSubScene("Menu", filePath.string());
	if (id == QFE::SCENE::InvalidSceneId || scenes.GetSceneIds().size() != 2) return 3;
	auto* main = scenes.GetSceneEntityManager(QFE::SCENE::MainSceneId);
	auto* menu = scenes.GetSceneEntityManager(id);
	if (!main || !menu || main == menu) return 4;
	// 別シーンの入力制御もゲーム全体に適用され、対象外の操作は使える。
	const auto inputControlId = menu->CreateEntity();
	auto& inputControl = menu->AddDefaultComponent<QFE::COMPONENTS::InputControlComponent>(inputControlId);
	inputControl.inputEnabled = false;
	inputControl.affectAllActions = false;
	inputControl.actions = { "Shot" };
	if (QFE::FRAMEWORK::IsGameActionEnabled(scenes, "Shot") ||
		!QFE::FRAMEWORK::IsGameActionEnabled(scenes, "Up")) return 40;
	inputControl.inputEnabled = true;
	if (!QFE::FRAMEWORK::IsGameActionEnabled(scenes, "Shot")) return 41;
	menu->CreateEntity();
	const auto mainCount = main->GetActiveEntityIds().size();
	const auto mainRevision = scenes.GetSceneRevision(QFE::SCENE::MainSceneId);
	const auto menuRevision = scenes.GetSceneRevision(id);
	if (!scenes.SetSceneUpdating(QFE::SCENE::MainSceneId, false) ||
		!scenes.SetSceneVisible(id, false) || !scenes.ReplaceScene(id, filePath.string())) return 5;
	// サブシーンの遷移はメインの内容・リビジョン・停止状態を変更しない。
	if (main->GetActiveEntityIds().size() != mainCount ||
		scenes.GetSceneRevision(QFE::SCENE::MainSceneId) != mainRevision ||
		scenes.GetSceneRevision(id) != menuRevision + 1 ||
		scenes.IsSceneUpdating(QFE::SCENE::MainSceneId) || scenes.IsSceneVisible(id)) return 6;
	const auto snapshot = scenes.CreateSnapshot();
	const auto beforeReloadRevision = scenes.GetSceneRevision(id);
	scenes.ReleaseComponentRegistryForReload();
	if (!scenes.RestoreAfterComponentReload(registry, snapshot) ||
		scenes.FindScene("Menu") != id || scenes.IsSceneUpdating(QFE::SCENE::MainSceneId) ||
		scenes.IsSceneVisible(id) || scenes.GetSceneRevision(id) <= beforeReloadRevision) return 7;
	// サブシーンのフェードと切り替えはメインシーンに伝播しない。
	main = scenes.GetSceneEntityManager(QFE::SCENE::MainSceneId);
	menu = scenes.GetSceneEntityManager(id);
	const auto sceneChangeId = menu->CreateEntity();
	auto& change = menu->AddDefaultComponent<QFE::SCENE::SceneChangeComponent>(sceneChangeId);
	change.sceneNames.push_back(std::filesystem::absolute(filePath).string());
	change.request = true;
	const auto revisionBeforeTransition = scenes.GetSceneRevision(QFE::SCENE::MainSceneId);
	QFE::FRAMEWORK::SceneChangeTransitionSystem transition;
	transition.Update(scenes, 0.0f, id);
	transition.Update(scenes, 0.25f, id);
	if (transition.GetFadeAlpha(id) <= 0.0f ||
		transition.GetFadeAlpha(QFE::SCENE::MainSceneId) != 0.0f) return 8;
	transition.Update(scenes, 0.25f, id);
	if (scenes.GetSceneRevision(QFE::SCENE::MainSceneId) != revisionBeforeTransition ||
		scenes.GetSceneEntityManager(id)->IsActiveEntity(sceneChangeId)) return 9;
	// イベントが設定する要求と同じ経路で、別シーンの読み込みと停止を行う。
	const auto projectPath = std::filesystem::absolute(filePath).parent_path() / "scene-control-test";
	const auto scenePath = projectPath / "assets" / "scenes" / "menu.scene";
	std::filesystem::create_directories(scenePath.parent_path());
	std::filesystem::copy_file(filePath, scenePath, std::filesystem::copy_options::overwrite_existing);
	// フレームワーク自身が保持するプロジェクトのシーンアセットを使用する。
	QFE::PROJECT::SetRoot(projectPath);
	const auto controllerId = main->CreateEntity();
	auto& control = main->AddDefaultComponent<QFE::SCENE::SceneControlComponent>(controllerId);
	control.sceneName = "Overlay";
	control.scenePath = "menu.scene";
	control.request = true;
	QFE::FRAMEWORK::UpdateSceneControlRequests(scenes);
	const auto overlay = scenes.FindScene("Overlay");
	if (overlay == QFE::SCENE::InvalidSceneId || !scenes.IsSceneUpdating(overlay)) return 10;
	if (!scenes.SetCurrentScene(overlay) || &scenes.GetCurrentSceneEntityManager() != scenes.GetSceneEntityManager(overlay)) return 11;
	// サブを処理中でも ID を明示したメインの置き換えはメインだけに作用する。
	const auto overlayRevision = scenes.GetSceneRevision(overlay);
	if (!scenes.ReplaceScene(QFE::SCENE::MainSceneId, filePath.string()) ||
		scenes.GetSceneRevision(overlay) != overlayRevision) return 14;
	if (!scenes.SetCurrentScene(QFE::SCENE::MainSceneId)) return 12;
	// メインを置き換えたため、以前のコンポーネント参照は使わず再作成する。
	const auto newControllerId = main->CreateEntity();
	auto& newControl = main->AddDefaultComponent<QFE::SCENE::SceneControlComponent>(newControllerId);
	newControl.sceneName = "Overlay";
	newControl.scenePath = "menu.scene";
	newControl.request = false;
	QFE::FRAMEWORK::UpdateSceneControlRequests(scenes);
	newControl.request = true;
	newControl.updating = false;
	newControl.visible = false;
	QFE::FRAMEWORK::UpdateSceneControlRequests(scenes);
	if (scenes.IsSceneUpdating(overlay) || scenes.IsSceneVisible(overlay)) return 13;
	const auto pathA = scenePath.parent_path() / "spawnA.scene";
	const auto pathB = scenePath.parent_path() / "spawnB.scene";
	for (const auto& [path, name] : { std::pair{ pathA, "A" }, std::pair{ pathB, "B" } }) {
		std::ofstream file(path);
		file << nlohmann::json({ { "entities", nlohmann::json::array({
			{ { "ObjectInfoComponent", { { "name", name }, { "uuid", std::string("test-uuid-") + name } } } }
		}) } }).dump();
	}
	const auto spawnerId = main->CreateEntity();
	const auto uuid = main->AddDefaultComponent<QFE::SCENE::ObjectInfoComponent>(spawnerId).uuid;
	// 旧シーンの単一パスを新しい一覧に移行する。
	main->DeserializeEntityComponents(spawnerId, { { "SceneSpawnComponent", { { "scenePath", "spawnA.scene" } } } });
	auto& spawner = main->GetComponent<QFE::SCENE::SceneSpawnComponent>(spawnerId);
	if (spawner.scenePaths.size() != 1 || spawner.scenePaths[0] != "spawnA.scene") return 15;
	spawner.scenePaths.push_back("spawnB.scene");
	std::vector<QFE::FRAMEWORK::CollisionTriggerCommand> commands = {
		{ controllerId, spawnerId, { { "type", "SceneSpawn" }, { "target", "EntityReference" }, { "targetUuid", uuid }, { "sceneIndex", 0 } } },
		{ controllerId, spawnerId, { { "type", "SceneSpawn" }, { "target", "Other" }, { "sceneIndex", 1 } } }
	};
	const auto beforeSpawn = main->GetActiveEntityIds().size();
	QFE::FRAMEWORK::ApplyCollisionTriggerCommands(*main, commands, {});
	QFE::FRAMEWORK::UpdateSceneSpawnRequests(scenes);
	if (main->GetActiveEntityIds().size() != beforeSpawn + 2) return 16;
	// 別 Entity のイベントから対象スポナーとシーン番号を指定する。
	const auto eventId = main->CreateEntity();
	auto& event = main->AddDefaultComponent<QFE::SCENE::EventComponent>(eventId);
	event.playOnStart = true;
	event.tracks = nlohmann::json::array({
		{ { "targetUuid", uuid }, { "component", "SceneSpawnComponent" }, { "property", "selectedSceneIndex" }, { "keyframes", {{{ "time", 0 }, { "value", 1 }}} } },
		{ { "targetUuid", uuid }, { "component", "SceneSpawnComponent" }, { "property", "request" }, { "interpolation", "Step" }, { "keyframes", {{{ "time", 0 }, { "value", true }}} } }
	});
	const auto beforeEvent = main->GetActiveEntityIds().size();
	QFE::FRAMEWORK::UpdateEventComponents(*main, 0.0f);
	QFE::FRAMEWORK::UpdateSceneSpawnRequests(scenes);
	if (main->GetActiveEntityIds().size() != beforeEvent + 1) return 17;
	QFE::FRAMEWORK::UpdateSceneSpawnRequests(scenes);
	if (main->GetActiveEntityIds().size() != beforeEvent + 1) return 18;
	int countA = 0, countB = 0;
	main->Each<QFE::SCENE::ObjectInfoComponent>([&](uint32_t, const QFE::SCENE::ObjectInfoComponent& info) {
		if (info.name == "A") ++countA;
		if (info.name == "B") ++countB;
	});
	if (countA != 1 || countB != 2) {
		std::cerr << "Unexpected spawned scenes: A=" << countA << ", B=" << countB << '\n';
		return 19;
	}
	commands = { { eventId, spawnerId, { { "type", "SceneSpawn" }, { "target", "Other" }, { "spawnAll", true } } } };
	QFE::FRAMEWORK::ApplyCollisionTriggerCommands(*main, commands, {});
	QFE::FRAMEWORK::UpdateSceneSpawnRequests(scenes);
	if (main->GetActiveEntityIds().size() != beforeEvent + 3) return 20;
	commands = { { eventId, spawnerId, { { "type", "SceneSpawn" }, { "target", "Other" }, { "sceneIndex", 99 } } } };
	QFE::FRAMEWORK::ApplyCollisionTriggerCommands(*main, commands, {});
	QFE::FRAMEWORK::UpdateSceneSpawnRequests(scenes);
	if (main->GetActiveEntityIds().size() != beforeEvent + 3) return 21;
	// デフォルトシーンと同じ設定: 1 秒後に request を立てて全シーンを生成する。
	main->GetComponent<QFE::SCENE::EventComponent>(eventId).playing = false;
	auto& delayedSpawner = main->GetComponent<QFE::SCENE::SceneSpawnComponent>(spawnerId);
	delayedSpawner.request = false;
	delayedSpawner.spawnAll = true;
	QFE::FRAMEWORK::UpdateSceneSpawnRequests(scenes);
	const auto delayedEventId = main->CreateEntity();
	auto& delayedEvent = main->AddDefaultComponent<QFE::SCENE::EventComponent>(delayedEventId);
	delayedEvent.playOnStart = true;
	delayedEvent.tracks = nlohmann::json::array({
		{ { "targetUuid", uuid }, { "component", "SceneSpawnComponent" }, { "property", "request" },
		  { "interpolation", "Step" }, { "keyframes", { { { "time", 0 }, { "value", false } }, { { "time", 1 }, { "value", true } } } } }
	});
	const auto beforeDelay = main->GetActiveEntityIds().size();
	QFE::FRAMEWORK::UpdateEventComponents(*main, 0.5f);
	QFE::FRAMEWORK::UpdateSceneSpawnRequests(scenes);
	if (main->GetActiveEntityIds().size() != beforeDelay) return 22;
	QFE::FRAMEWORK::UpdateEventComponents(*main, 0.5f);
	QFE::FRAMEWORK::UpdateSceneSpawnRequests(scenes);
	if (main->GetActiveEntityIds().size() != beforeDelay + 2) return 23;
	std::filesystem::remove(pathA);
	std::filesystem::remove(pathB);
	QFE::PROJECT::SetRoot({});
	std::filesystem::remove(scenePath);
	std::filesystem::remove(scenePath.parent_path());
	std::filesystem::remove(scenePath.parent_path().parent_path());
	std::filesystem::remove(projectPath);
	std::filesystem::remove(filePath);
	return 0;
}
