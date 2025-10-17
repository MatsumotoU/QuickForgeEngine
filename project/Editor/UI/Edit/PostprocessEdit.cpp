#include "PostprocessEdit.h"
#include "Graphic/PostEffect/RendaringPostprocess.h"

void PostprocessEdit::Initialize() {
	name_ = "PostprocessEdit";
	isActive_ = false;
}

void PostprocessEdit::Update() {
}

void PostprocessEdit::Draw() {
	if (!isActive_) {
		return;
	}
	ImGui::Begin("Postprocess Edit", &isActive_, ImGuiWindowFlags_AlwaysAutoResize);
	RendaringPostprosecess* postprocess = RendaringPostprosecess::GetInstance();
	postprocess;
	ImGui::End();
}

void PostprocessEdit::Run() {
	isActive_ = true;
}
