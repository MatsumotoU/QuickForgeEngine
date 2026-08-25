#include "AnimationEditor.h"

#include "design-patterns/EntityManager.h"
#include "components/AnimationComponent.h"
#include "framework/scene/animation/AnimationDataServices.h"

#include <imgui/imgui.h>
#include <imgui_stdlib.h>
#include <im-neo-sequencer-main/imgui_neo_sequencer.h>

#include <algorithm>
#include <cmath>

QFE::EDITOR::AnimationEditor::AnimationEditor(EntityManager* entityManager) :
	entityManager_(entityManager) {
	Initialize();
}

void QFE::EDITOR::AnimationEditor::Initialize() {
	keyFrames_.clear();
	selectedKeyIndex_ = -1;
	currentFrame_ = 0;
	frameStart_ = 0;
	frameEnd_ = 60;
	fps_ = 60.0f;
	previewTime_ = 0.0f;
	isPreviewPlaying_ = false;
	statusMessage_.clear();
}

QFE::ANIMATION::AnimationClip QFE::EDITOR::AnimationEditor::BuildClip() const {
	ANIMATION::AnimationClip clip;
	clip.SetName(clipName_);
	clip.SetLoop(isLoop_);
	const float safeFps = (std::max)(1.0f, fps_);
	std::vector<EditorAnimationKeyFrame> sorted = keyFrames_;
	std::stable_sort(sorted.begin(), sorted.end(),
		[](const EditorAnimationKeyFrame& lhs, const EditorAnimationKeyFrame& rhs) {
			return lhs.frame < rhs.frame;
		});
	for (const EditorAnimationKeyFrame& editorKey : sorted) {
		ANIMATION::AnimationKeyFrame key;
		key.time = static_cast<float>((std::max)(0, editorKey.frame)) / safeFps;
		key.transform = editorKey.transform;
		clip.AddKeyFrame(key);
	}
	return clip;
}

bool QFE::EDITOR::AnimationEditor::LoadClip(const std::string& clipName) {
	ANIMATION::AnimationClip clip;
	if (!ANIMATION::LoadAnimationClip(ANIMATION::ResolveAnimationClipPath(clipName), clip)) {
		statusMessage_ = "Failed to load animation clip.";
		return false;
	}
	clipName_ = clipName.empty() ? clip.GetName() : clipName;
	isLoop_ = clip.IsLoop();
	keyFrames_.clear();
	for (const ANIMATION::AnimationKeyFrame& key : clip.GetKeyFrames()) {
		EditorAnimationKeyFrame editorKey;
		editorKey.frame = static_cast<int32_t>(std::lround(key.time * (std::max)(1.0f, fps_)));
		editorKey.transform = key.transform;
		keyFrames_.push_back(editorKey);
	}
	frameEnd_ = (std::max)(60, static_cast<int32_t>(std::ceil(clip.GetDuration() * fps_)));
	currentFrame_ = 0;
	previewTime_ = 0.0f;
	selectedKeyIndex_ = -1;
	statusMessage_ = "Animation clip loaded.";
	return true;
}

bool QFE::EDITOR::AnimationEditor::SaveClip(uint32_t selectedEntityId) {
	if (clipName_.empty() || keyFrames_.empty()) {
		statusMessage_ = "Clip name and at least one keyframe are required.";
		return false;
	}
	const ANIMATION::AnimationClip clip = BuildClip();
	if (!ANIMATION::SaveAnimationClip(clip, ANIMATION::ResolveAnimationClipPath(clipName_))) {
		statusMessage_ = "Failed to save animation clip.";
		return false;
	}
	if (entityManager_ != nullptr &&
		entityManager_->HasComponent<SCENE::AnimationComponent>(selectedEntityId)) {
		auto& animation = entityManager_->GetComponent<SCENE::AnimationComponent>(selectedEntityId);
		animation.clipName = clipName_;
		animation.useClipLoop = true;
		animation.loop = isLoop_;
	}
	statusMessage_ = "Animation clip saved.";
	return true;
}

void QFE::EDITOR::AnimationEditor::ApplyPreview(uint32_t entityId) {
	if (entityManager_ == nullptr || !entityManager_->HasComponent<SCENE::AnimationComponent>(entityId)) return;
	const ANIMATION::AnimationClip clip = BuildClip();
	if (clip.GetKeyFrames().empty()) return;
	const float time = static_cast<float>(currentFrame_) / (std::max)(1.0f, fps_);
	entityManager_->GetComponent<SCENE::AnimationComponent>(entityId).transform = clip.Sample(time);
	previewEntityId_ = entityId;
}

void QFE::EDITOR::AnimationEditor::ResetPreview() {
	if (entityManager_ != nullptr && previewEntityId_ != UINT32_MAX &&
		entityManager_->HasComponent<SCENE::AnimationComponent>(previewEntityId_)) {
		entityManager_->GetComponent<SCENE::AnimationComponent>(previewEntityId_).transform = {};
	}
	previewEntityId_ = UINT32_MAX;
	isPreviewPlaying_ = false;
	previewTime_ = 0.0f;
}

void QFE::EDITOR::AnimationEditor::Draw(
	std::set<uint32_t>& selectedEntities, EditorCommandList&) {
	if (!isActive_) return;
	ImGui::Begin(GetWindowName().c_str(), &isActive_, ImGuiWindowFlags_NoDocking);
	if (!isActive_) {
		ResetPreview();
		ImGui::End();
		return;
	}
	isFocus_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	if (selectedEntities.empty()) {
		ResetPreview();
		ImGui::TextDisabled("Select an entity with AnimationComponent.");
		ImGui::End();
		return;
	}
	const uint32_t selectedEntityId = *selectedEntities.begin();
	if (previewEntityId_ != UINT32_MAX && previewEntityId_ != selectedEntityId) ResetPreview();
	const bool hasAnimation = entityManager_ != nullptr &&
		entityManager_->HasComponent<SCENE::AnimationComponent>(selectedEntityId);

	ImGui::SetNextItemWidth(220.0f);
	ImGui::InputText("Clip Name", &clipName_);
	ImGui::SameLine();
	if (ImGui::Button("Load")) LoadClip(clipName_);
	ImGui::SameLine();
	if (ImGui::Button("Load From Component") && hasAnimation) {
		clipName_ = entityManager_->GetComponent<SCENE::AnimationComponent>(selectedEntityId).clipName;
		LoadClip(clipName_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Save")) SaveClip(selectedEntityId);
	ImGui::SameLine();
	if (ImGui::Button("Clear")) {
		ResetPreview();
		keyFrames_.clear();
		selectedKeyIndex_ = -1;
		currentFrame_ = 0;
		statusMessage_ = "Animation data cleared.";
	}

	ImGui::SetNextItemWidth(120.0f);
	ImGui::DragFloat("FPS", &fps_, 1.0f, 1.0f, 240.0f, "%.0f");
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &isLoop_);
	ImGui::SameLine();
	if (ImGui::Button(isPreviewPlaying_ ? "Pause Preview" : "Play Preview")) {
		isPreviewPlaying_ = !isPreviewPlaying_;
		previewTime_ = static_cast<float>(currentFrame_) / (std::max)(1.0f, fps_);
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop Preview")) {
		ResetPreview();
		currentFrame_ = 0;
	}
	if (!hasAnimation) {
		ImGui::TextDisabled("Add AnimationComponent to the selected entity to preview and bind the clip.");
	}
	if (!statusMessage_.empty()) ImGui::TextUnformatted(statusMessage_.c_str());

	if (ImGui::Button("Add Keyframe At Current Frame")) {
		auto existing = std::find_if(keyFrames_.begin(), keyFrames_.end(),
			[this](const EditorAnimationKeyFrame& key) { return key.frame == currentFrame_; });
		if (existing == keyFrames_.end()) {
			EditorAnimationKeyFrame key;
			key.frame = currentFrame_;
			keyFrames_.push_back(key);
			std::stable_sort(keyFrames_.begin(), keyFrames_.end(),
				[](const EditorAnimationKeyFrame& lhs, const EditorAnimationKeyFrame& rhs) {
					return lhs.frame < rhs.frame;
				});
			existing = std::find_if(keyFrames_.begin(), keyFrames_.end(),
				[this](const EditorAnimationKeyFrame& keyFrame) { return keyFrame.frame == currentFrame_; });
		}
		selectedKeyIndex_ = static_cast<int>(std::distance(keyFrames_.begin(), existing));
	}

	const float sequencerWidth = ImGui::GetContentRegionAvail().x * 0.68f;
	ImGui::BeginChild("AnimationSequencer", ImVec2(sequencerWidth, 0.0f), ImGuiChildFlags_Border);
	if (ImGui::BeginNeoSequencer("Transform Animation", &currentFrame_, &frameStart_, &frameEnd_,
		ImVec2(0, 0), ImGuiNeoSequencerFlags_EnableSelection |
		ImGuiNeoSequencerFlags_Selection_EnableDragging |
		ImGuiNeoSequencerFlags_AllowLengthChanging)) {
		if (ImGui::BeginNeoGroup("Transform", &isTransformOpen_)) {
			if (ImGui::BeginNeoTimelineEx("Transform Keys")) {
				for (size_t index = 0; index < keyFrames_.size(); ++index) {
					ImGui::NeoKeyframe(&keyFrames_[index].frame);
					if (ImGui::IsNeoKeyframeSelected()) selectedKeyIndex_ = static_cast<int>(index);
				}
				ImGui::EndNeoTimeLine();
			}
			ImGui::EndNeoGroup();
		}
		ImGui::EndNeoSequencer();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("AnimationKeyInspector", ImVec2(0, 0), ImGuiChildFlags_Border);
	ImGui::TextUnformatted("Keyframe Inspector");
	ImGui::Separator();
	if (selectedKeyIndex_ >= 0 && selectedKeyIndex_ < static_cast<int>(keyFrames_.size())) {
		EditorAnimationKeyFrame& key = keyFrames_[selectedKeyIndex_];
		ImGui::DragInt("Frame", &key.frame, 1.0f, 0, frameEnd_);
		ImGui::DragFloat3("Translate", &key.transform.translate.x, 0.1f);
		ImGui::DragFloat3("Rotate", &key.transform.rotate.x, 0.01f);
		ImGui::DragFloat3("Scale", &key.transform.scale.x, 0.01f);
		if (ImGui::Button("Delete Keyframe")) {
			keyFrames_.erase(keyFrames_.begin() + selectedKeyIndex_);
			selectedKeyIndex_ = -1;
			ImGui::NeoClearSelection();
		}
	} else {
		ImGui::TextDisabled("Select a keyframe to edit.");
	}
	ImGui::EndChild();

	if (isPreviewPlaying_ && !keyFrames_.empty()) {
		const ANIMATION::AnimationClip clip = BuildClip();
		previewTime_ += ImGui::GetIO().DeltaTime;
		const float duration = clip.GetDuration();
		if (duration > 0.0f && previewTime_ >= duration) {
			if (isLoop_) previewTime_ = std::fmod(previewTime_, duration);
			else {
				previewTime_ = duration;
				isPreviewPlaying_ = false;
			}
		}
		currentFrame_ = static_cast<int32_t>(std::lround(previewTime_ * fps_));
	}
	if (hasAnimation && !keyFrames_.empty()) ApplyPreview(selectedEntityId);

	ImGui::End();
}

std::string QFE::EDITOR::AnimationEditor::GetWindowName() { return "Animation Editor"; }
bool QFE::EDITOR::AnimationEditor::GetIsActive() { return isActive_; }
bool QFE::EDITOR::AnimationEditor::SetIsActive(bool isActive) {
	if (!isActive) ResetPreview();
	isActive_ = isActive;
	return isActive_;
}
bool QFE::EDITOR::AnimationEditor::GetIsFocus() { return isFocus_; }
