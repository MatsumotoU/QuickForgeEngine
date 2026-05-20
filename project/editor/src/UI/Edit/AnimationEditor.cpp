#include "editor/include/UI/Edit/AnimationEditor.h"

#include "engine/include/assets/Animator/AnimationDataServices.h"
#include "engine/include/core/Bridge/EngineBridgeProvider.h"

#include "externals/imgui/im-neo-sequencer-main/imgui_neo_sequencer.h"
#include <algorithm>

QFE::AnimationEditor::AnimationEditor(){
	SetName("Animation Editor");
	isActive_ = false;
	currentFrame_ = 0;
	frameStart_ = 0;
	frameEnd_ = 60;
	animClip_ = nullptr;
}

void QFE::AnimationEditor::Initialize(){
	editingKeyFrames_.clear();
	selectedKeyIndex_ = -1;
	fps_ = 60.0f; 
	animClip_ = nullptr;
	animFileNameBuffer_[0] = '\0';
}

void QFE::AnimationEditor::Update(){
}

void QFE::AnimationEditor::Draw(){
	if (!isActive_) { return; }

	ImGui::Begin("Animation Editor", &isActive_, ImGuiWindowFlags_NoDocking);

	// データのクリアボタン
	if(ImGui::Button("Clear Data")){
		editingKeyFrames_.clear();
		selectedKeyIndex_ = -1;
	}
	ImGui::SameLine();

	// キーフレームの追加ボタン
	if (ImGui::Button("Add KeyFrame (Current Frame)")) {
		// すでに同じフレームにキーが存在するか検索
		auto it = std::find_if(editingKeyFrames_.begin(), editingKeyFrames_.end(),
			[this](const EditorKeyFrame& k) { return k.frame == currentFrame_; });

		if (it != editingKeyFrames_.end()) {
			// すでに存在する場合は上書き対象とする（必要に応じてデフォルト値でリセット、等）
			// 何もせずアクティブプロパティに切り替えるだけでもOK
			selectedKeyIndex_ = static_cast<int>(std::distance(editingKeyFrames_.begin(), it));
		} else {
			// 新規追加
			EditorKeyFrame newKey;
			newKey.frame = currentFrame_;
			newKey.transform = Transform(); // デフォルト値 (Scale=1.0など)

			editingKeyFrames_.push_back(newKey);

			// 重ならないようにソート
			std::sort(editingKeyFrames_.begin(), editingKeyFrames_.end(), [](const EditorKeyFrame& a, const EditorKeyFrame& b) {
				return a.frame < b.frame;
				});

			// 追加したキーを選択状態にする
			auto newIt = std::find_if(editingKeyFrames_.begin(), editingKeyFrames_.end(),
				[this](const EditorKeyFrame& k) { return k.frame == currentFrame_; });
			if (newIt != editingKeyFrames_.end()) {
				selectedKeyIndex_ = static_cast<int>(std::distance(editingKeyFrames_.begin(), newIt));
			}
		}
	}
	ImGui::InputFloat("FPS", &fps_,1.0f);

	ImGui::Separator();

	// === レイアウト分割 ===
	// 左側（シーケンサー）に割り当てる幅（ウィンドウ幅の 70% など。必要に応じて調整してください）
	float sequencerWidth = ImGui::GetContentRegionAvail().x * 0.7f;
	
	// --- 左側エリア（シーケンサー） ---
	ImGui::BeginChild("SequencerRegion", ImVec2(sequencerWidth, 0), true);
	
	// シーケンサーの描画（選択機能をONにする）
	if (ImGui::BeginNeoSequencer("Sequencer", &currentFrame_, &frameStart_, &frameEnd_, ImVec2(0, 0),
		ImGuiNeoSequencerFlags_EnableSelection | ImGuiNeoSequencerFlags_Selection_EnableDragging | ImGuiNeoSequencerFlags_Selection_EnableDeletion)) {
		
		if (ImGui::BeginNeoGroup("Transform", &isTransformOpen_)) {
			// PositionだけでなくTransform全体を1つのタイムラインとして扱う例
			if (ImGui::BeginNeoTimelineEx("Transform Keys")) {
				for (size_t i = 0; i < editingKeyFrames_.size(); i++) {
					
					// NeoSequencerにキーフレームのポインタを渡す（ドラッグ移動などで書き変わる）
					ImGui::NeoKeyframe(&editingKeyFrames_[i].frame);

					// 選択されているキーを判定
					if (ImGui::IsNeoKeyframeSelected()) {
						selectedKeyIndex_ = static_cast<int>(i);
					}
				}

				ImGui::EndNeoTimeLine();
			}
			ImGui::EndNeoGroup();
		}

		ImGui::EndNeoSequencer();
	}
	ImGui::EndChild(); // 左側エリア終了

	// --- 右側エリア（インスペクター） ---
	ImGui::SameLine();
	ImGui::BeginChild("InspectorRegion", ImVec2(0, 0), true);
	
	ImGui::Text("Inspector");
	ImGui::Separator();

	if (selectedKeyIndex_ >= 0 && selectedKeyIndex_ < static_cast<int>(editingKeyFrames_.size())) {
		ImGui::Text("Editing KeyFrame (Index: %d, Frame: %d)", selectedKeyIndex_, editingKeyFrames_[selectedKeyIndex_].frame);
		
		// インスペクタ側にも削除ボタンを追加
		if (ImGui::Button("Delete KeyFrame")) {
			editingKeyFrames_.erase(editingKeyFrames_.begin() + selectedKeyIndex_);
			selectedKeyIndex_ = -1;
			ImGui::NeoClearSelection(); // シーケンサーの選択状態もリセット
		}
		
		if (selectedKeyIndex_ != -1) { // 削除されなかった場合のみプロパティを描画
			EditorKeyFrame& key = editingKeyFrames_[selectedKeyIndex_];

			ImGui::Separator();
			
			float t[3] = { key.transform.translate.x, key.transform.translate.y, key.transform.translate.z };
			if (ImGui::DragFloat3("Translate", t, 0.1f)) {
				key.transform.translate = Vector3(t[0], t[1], t[2]);
			}

			float r[3] = { key.transform.rotate.x, key.transform.rotate.y, key.transform.rotate.z };
			if (ImGui::DragFloat3("Rotate", r, 0.1f)) {
				key.transform.rotate = Vector3(r[0], r[1], r[2]);
			}

			float s[3] = { key.transform.scale.x, key.transform.scale.y, key.transform.scale.z };
			if (ImGui::DragFloat3("Scale", s, 0.1f)) {
				key.transform.scale = Vector3(s[0], s[1], s[2]);
			}
		}
	} else {
		ImGui::TextDisabled("Select a keyframe to edit.");
	}

	ImGui::Separator();
	ImGui::Spacing();

	// エクスポート / 反映機能 (Bake)
	if(ImGui::InputText("Anim File Name", animFileNameBuffer_, sizeof(animFileNameBuffer_))) {
		
	}
	if (ImGui::Button("Bake to AnimClip", ImVec2(-FLT_MIN, 30))) { // 横幅いっぱいに広げる例
		animClip_ = std::make_unique<AnimClip>(editingKeyFrames_.size()); // 再生成など
		animClip_->SetName(animFileNameBuffer_);

		// キーフレーム順にソートする
		std::sort(editingKeyFrames_.begin(), editingKeyFrames_.end(), [](const EditorKeyFrame& a, const EditorKeyFrame& b) {
			return a.frame < b.frame;
			});

		// EditorKeyFrame から KeyFrame に変換して AnimClip へ登録
		for (const auto& ek : editingKeyFrames_) {
			KeyFrame kf;
			kf.time = static_cast<float>(ek.frame) / fps_;
			kf.transform = ek.transform;
			animClip_->AddKeyFrame(kf);
		}

		// AnimClip をファイルに保存
		std::string filePath = BRIDGE::GetBridge()->GetAnimationDirectoryPath() + std::string(animFileNameBuffer_) + ".anim";
		QFE::ANIMATION::SaveAnimClipToAnimFile(*animClip_, filePath);
	}
	
	ImGui::EndChild(); // 右側エリア終了

	ImGui::End();
}

void QFE::AnimationEditor::Run(){
	isActive_ = !isActive_;
}
