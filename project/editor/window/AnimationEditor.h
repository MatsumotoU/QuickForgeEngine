#pragma once

#include "IEditorWindow.h"
#include "framework/scene/animation/AnimationClip.h"

#include <cstdint>
#include <string>
#include <vector>

namespace QFE {
	class EntityManager;
}

namespace QFE::EDITOR {
	struct EditorAnimationKeyFrame {
		int32_t frame = 0;
		MATH::EulerTransform transform;
	};

	/// @brief Transformアニメーションをシーケンサー上で作成・保存・プレビューする。
	class AnimationEditor final : public IEditorWindow {
	public:
		explicit AnimationEditor(EntityManager* entityManager);
		void Initialize() override;
		void Draw(std::set<uint32_t>& selectedEntities, EditorCommandList& commandList) override;
		std::string GetWindowName() override;
		bool GetIsActive() override;
		bool SetIsActive(bool isActive) override;
		bool GetIsFocus() override;

	private:
		ANIMATION::AnimationClip BuildClip() const;
		bool LoadClip(const std::string& clipName);
		bool SaveClip(uint32_t selectedEntityId);
		void ApplyPreview(uint32_t entityId);
		void ResetPreview();

		EntityManager* entityManager_ = nullptr;
		bool isActive_ = false;
		bool isFocus_ = false;
		bool isTransformOpen_ = true;
		bool isLoop_ = false;
		bool isPreviewPlaying_ = false;
		float fps_ = 60.0f;
		float previewTime_ = 0.0f;
		int32_t currentFrame_ = 0;
		int32_t frameStart_ = 0;
		int32_t frameEnd_ = 60;
		int selectedKeyIndex_ = -1;
		uint32_t previewEntityId_ = UINT32_MAX;
		std::string clipName_;
		std::string statusMessage_;
		std::vector<EditorAnimationKeyFrame> keyFrames_;
	};
}
