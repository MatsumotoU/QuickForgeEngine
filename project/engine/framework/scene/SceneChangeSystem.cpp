#include "SceneChangeSystem.h"

#include "components/SceneChangeComponent.h"
#include "design-patterns/EntityManager.h"
#include "scene/SceneManager.h"

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace {
	std::filesystem::path FindSceneFile(
		const std::string& sceneName,
		const std::string& currentScenePath) {
		if (sceneName.empty()) return {};

		const std::filesystem::path scenePath(sceneName);
		std::vector<std::filesystem::path> candidates;
		const auto addCandidates = [&candidates](const std::filesystem::path& path) {
			candidates.push_back(path);
			if (path.extension().empty()) {
				std::filesystem::path jsonPath = path;
				jsonPath += ".json";
				candidates.push_back(std::move(jsonPath));
			}
		};

		addCandidates(scenePath);
		if (scenePath.is_relative()) {
			if (!currentScenePath.empty()) {
				addCandidates(std::filesystem::path(currentScenePath).parent_path() / scenePath);
			}
			addCandidates(std::filesystem::path("resources/scene") / scenePath);
		}

		for (const std::filesystem::path& candidate : candidates) {
			std::error_code error;
			if (std::filesystem::is_regular_file(candidate, error)) return candidate;
		}
		return {};
	}
}

void QFE::FRAMEWORK::SceneChangeTransitionSystem::Update(
	SCENE::SceneManager& sceneManager,
	float deltaTime) {
	if (state_ == State::Idle) {
		QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
		std::string requestedSceneName;
		bool hasRequest = false;
		entityManager.Each<SCENE::SceneChangeComponent>(
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
				requestedSceneName, sceneManager.GetCurrentScenePath());
			if (!sceneFile.empty()) {
				targetScenePath_ = sceneFile.string();
				elapsedSeconds_ = 0.0f;
				state_ = State::FadeOut;
			}
		}
	}

	const float safeDeltaTime = deltaTime > 0.0f ? deltaTime : 0.0f;
	if (state_ == State::FadeOut) {
		elapsedSeconds_ += safeDeltaTime;
		fadeAlpha_ = elapsedSeconds_ / fadeDurationSeconds_;
		if (fadeAlpha_ > 1.0f) fadeAlpha_ = 1.0f;
		if (fadeAlpha_ >= 1.0f) {
			sceneManager.LoadCurrentSceneFromJson(targetScenePath_);
			elapsedSeconds_ = 0.0f;
			state_ = State::FadeIn;
		}
	} else if (state_ == State::FadeIn) {
		elapsedSeconds_ += safeDeltaTime;
		fadeAlpha_ = 1.0f - elapsedSeconds_ / fadeDurationSeconds_;
		if (fadeAlpha_ < 0.0f) fadeAlpha_ = 0.0f;
		if (fadeAlpha_ <= 0.0f) {
			state_ = State::Idle;
			targetScenePath_.clear();
			elapsedSeconds_ = 0.0f;
		}
	}
}
