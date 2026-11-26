#include "framework/scene/SceneState.h"

#include "components/SceneChangeComponent.h"
#include "components/SceneSpawnComponent.h"
#include "components/SceneControlComponent.h"
#include "framework/scene/SceneCoreFramework.h"
#include "ecs/entity/EntityManager.h"
#include "scene/SceneManager.h"

#include "assetfactory/game-data/GameProjectPaths.h"
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

// 便利関数
namespace {
	std::filesystem::path FindSceneFile(
		const std::string& sceneName,
		const std::string& currentScenePath) {
		if (sceneName.empty()) return {};

		// シーン名と現在のパスはいずれも UTF-8 で保持する。
		const std::filesystem::path scenePath(std::u8string(
			reinterpret_cast<const char8_t*>(sceneName.data()), sceneName.size()));
		std::vector<std::filesystem::path> candidates;
		const auto addCandidates = [&candidates](const std::filesystem::path& path) {
			candidates.push_back(path);
			if (path.extension().empty()) {
				std::filesystem::path scenePath = path;
				scenePath += ".scene";
				candidates.push_back(std::move(scenePath));
				std::filesystem::path jsonPath = path;
				jsonPath += ".json";
				candidates.push_back(std::move(jsonPath));
			}
		};

		if (scenePath.is_absolute() || QFE::PROJECT::Root().empty()) addCandidates(scenePath);
		if (scenePath.is_relative()) {
			if (!QFE::PROJECT::Root().empty() && !scenePath.empty() && *scenePath.begin() == "resources") addCandidates(QFE::PROJECT::Resolve(scenePath));
			if (!currentScenePath.empty()) {
				addCandidates(std::filesystem::path(std::u8string(
					reinterpret_cast<const char8_t*>(currentScenePath.data()), currentScenePath.size())).parent_path() / scenePath);
			}
			addCandidates(QFE::PROJECT::Assets() / (QFE::PROJECT::Root().empty() ? "scene" : "scenes") / scenePath);
		}

		for (const std::filesystem::path& candidate : candidates) {
			std::error_code error;
			if (!QFE::PROJECT::Root().empty() &&
				!QFE::PROJECT::IsWithin(candidate, QFE::PROJECT::Assets() / "scenes")) continue;
			if (std::filesystem::is_regular_file(candidate, error)) return candidate;
		}
		return {};
	}
}

namespace QFE::FRAMEWORK {
	std::filesystem::path ResolveSceneSpawnAsset(const std::string& relativePath) {
		if (relativePath.empty()) return {};
		const std::filesystem::path path(std::u8string(
			reinterpret_cast<const char8_t*>(relativePath.data()), relativePath.size()));
		if (path.is_absolute() || path.has_root_name() || path.has_root_directory()) return {};
		std::string extension = path.extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(),
			[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		if (extension != ".scene" && extension != ".json") return {};
		// フレームワークが保持するプロジェクトのシーンアセットを基準にする。
		const auto root = PROJECT::Assets() / (PROJECT::Root().empty() ? "scene" : "scenes");
		const auto candidate = root / path;
		std::error_code error;
		// 別フォルダーや別名へのフォールバックを行わず、消えた参照先の誤生成を防ぐ。
		if (!PROJECT::IsWithin(candidate, root) || !std::filesystem::is_regular_file(candidate, error) || error) return {};
		return candidate;
	}

	void UpdateSceneSpawnRequests(SCENE::SceneManager& sceneManager) {
		struct Request { std::string path; QFE::MATH::Vector3 position; };
		std::vector<Request> requests;
		// 走査中のストレージ再配置を避けるため、要求を集めてから追加する。
		sceneManager.GetCurrentSceneEntityManager().Each<SCENE::SceneSpawnComponent>(
			[&](uint32_t, SCENE::SceneSpawnComponent& spawn) {
				if (!spawn.request) spawn.requestHandled = false;
				else if (!spawn.requestHandled) {
					spawn.requestHandled = true;
					spawn.pendingRequests.push_back({ spawn.selectedSceneIndex, spawn.spawnAll });
				}
				for (const auto& pending : spawn.pendingRequests) {
					if (pending.all) {
						if (spawn.scenePaths.empty()) requests.push_back({ {}, spawn.position });
						for (const auto& path : spawn.scenePaths) requests.push_back({ path, spawn.position });
					} else if (pending.sceneIndex < spawn.scenePaths.size()) {
						requests.push_back({ spawn.scenePaths[pending.sceneIndex], spawn.position });
					} else {
						QFE_LOG("Scene spawn index is out of range: " + std::to_string(pending.sceneIndex), QFE::LogLevel::Error);
					}
				}
				spawn.pendingRequests.clear();
			});
		for (const auto& request : requests) {
			const auto path = ResolveSceneSpawnAsset(request.path);
			if (path.empty()) {
				QFE_LOG("Scene spawn asset is unfilled or missing: " + request.path, QFE::LogLevel::Error);
				continue;
			}
			const auto utf8 = path.u8string();
			const std::string pathString(reinterpret_cast<const char*>(utf8.data()), utf8.size());
			sceneManager.AppendScene(pathString, request.position);
		}
	}

	void UpdateSceneControlRequests(SCENE::SceneManager& sceneManager) {
		struct Request { std::string name; std::string path; bool updating; bool visible; };
		std::vector<Request> requests;
		// 走査中にシーンを追加すると参照が無効になるため、要求を先にコピーする。
		sceneManager.GetCurrentSceneEntityManager().Each<SCENE::SceneControlComponent>(
			[&](uint32_t, SCENE::SceneControlComponent& control) {
				if (!control.request) { control.requestHandled = false; return; }
				if (control.requestHandled) return;
				control.requestHandled = true;
				requests.push_back({ control.sceneName, control.scenePath, control.updating, control.visible });
			});
		for (const auto& request : requests) {
			auto id = sceneManager.FindScene(request.name);
			if (id == SCENE::InvalidSceneId) {
				const auto path = ResolveSceneSpawnAsset(request.path);
				if (path.empty() || request.name.empty()) {
					QFE_LOG("Scene control asset is unfilled or missing: " + request.path, QFE::LogLevel::Error);
					continue;
				}
				const auto utf8 = path.u8string();
				id = sceneManager.LoadSubScene(request.name,
					std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size()));
			}
			if (id == SCENE::InvalidSceneId) {
				QFE_LOG("Failed to load controlled scene: " + request.name, QFE::LogLevel::Error);
				continue;
			}
			sceneManager.SetSceneUpdating(id, request.updating);
			sceneManager.SetSceneVisible(id, request.visible);
		}
	}

	/// ISceneState の実装
	std::unique_ptr<ISceneState>
	IdleSceneState::Update(SCENE::SceneManager& sceneManager, uint64_t sceneId, float) {
		QFE::EntityManager* entityManager = sceneManager.GetSceneEntityManager(sceneId);
		const std::string* currentPath = sceneManager.GetScenePath(sceneId);
		if (!entityManager || !currentPath) return nullptr;
		std::string requestedSceneName;
		bool hasRequest = false;
		entityManager->Each<SCENE::SceneChangeComponent>(
			[&](uint32_t, SCENE::SceneChangeComponent& sceneChange) {
				if (hasRequest || !sceneChange.request) return;
				sceneChange.request = false;
				hasRequest = true;
				if (sceneChange.nextSceneNumber < sceneChange.sceneNames.size()) {
					requestedSceneName = sceneChange.sceneNames[sceneChange.nextSceneNumber];
				}
			});

		if (hasRequest && !requestedSceneName.empty()) {
			const std::filesystem::path sceneFile = FindSceneFile(
				requestedSceneName, *currentPath);
			if (!sceneFile.empty()) {
				// ロード側は UTF-8 を受け取るため、Windows のローカルコードページに変換しない。
				const auto utf8 = sceneFile.u8string();
				return std::make_unique<FadeOutSceneState>(
					std::string(reinterpret_cast<const char*>(utf8.data()), utf8.size()));
			}
		}
		return nullptr;
	}

	// FadeOutSceneState の実装
	FadeOutSceneState::FadeOutSceneState(std::string targetScenePath)
		: targetScenePath_(std::move(targetScenePath)) {}

	std::unique_ptr<ISceneState>
	FadeOutSceneState::Update(SCENE::SceneManager& sceneManager, uint64_t sceneId, float deltaTime) {
		const float safeDeltaTime = deltaTime > 0.0f ? deltaTime : 0.0f;
		elapsedSeconds_ += safeDeltaTime;
		fadeAlpha_ = elapsedSeconds_ / fadeDurationSeconds_;
		if (fadeAlpha_ > 1.0f) fadeAlpha_ = 1.0f;
		if (fadeAlpha_ >= 1.0f) {
			// 要求元のシーンだけを入れ替え、ほかの EntityManager は維持する。
			if (!sceneManager.ReplaceScene(sceneId, targetScenePath_)) {
				QFE_LOG("Failed to change scene: " + targetScenePath_, QFE::LogLevel::Error);
			}
			return std::make_unique<FadeInSceneState>();
		}
		return nullptr;
	}

	// FadeInSceneState の実装
	std::unique_ptr<ISceneState>
	FadeInSceneState::Update(SCENE::SceneManager&, uint64_t, float deltaTime) {
		const float safeDeltaTime = deltaTime > 0.0f ? deltaTime : 0.0f;
		elapsedSeconds_ += safeDeltaTime;
		fadeAlpha_ = 1.0f - elapsedSeconds_ / fadeDurationSeconds_;
		if (fadeAlpha_ < 0.0f) fadeAlpha_ = 0.0f;
		if (fadeAlpha_ <= 0.0f) {
			return std::make_unique<IdleSceneState>();
		}
		return nullptr;
	}

}
