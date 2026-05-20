#include "editor/include/UI/Edit/AnimationEditor.h"

QFE::AnimationEditor::AnimationEditor(){
	SetName("Animation Editor");
	isActive_ = false;
}

void QFE::AnimationEditor::Initialize(){
}

void QFE::AnimationEditor::Update(){
}

void QFE::AnimationEditor::Draw(){
	if (!isActive_) { return; }

	ImGui::Begin("Animation Editor", &isActive_, ImGuiWindowFlags_NoDocking);


	ImGui::End();
}

void QFE::AnimationEditor::Run(){
	isActive_ = !isActive_;
}
