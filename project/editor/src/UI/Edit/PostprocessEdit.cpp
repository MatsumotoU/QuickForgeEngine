#include "editor/include/UI/Edit/PostprocessEdit.h"
#include "graphic/PostEffect/RenderingPostprocess.h"

void PostprocessEdit::Initialize() {
	name_ = "PostprocessEdit";
	isActive_ = false;
}

void PostprocessEdit::Update() {
	// 更新処理は必要ないため空実装としています
}

void PostprocessEdit::Draw() {
	if (!isActive_) {
		return;
	}
	ImGui::Begin("Postprocess Edit", &isActive_, ImGuiWindowFlags_AlwaysAutoResize);
	RenderingPostprocess* postprocess = RenderingPostprocess::GetInstance();
	ImGui::Checkbox("Enable Postprocess", &postprocess->isPostprocess_);
	ImGui::Separator();
	if (ImGui::BeginChild("ColorCorrection", ImVec2(0, 150), true)) {
		auto& color = postprocess->GetColorCorrectionOffset();
		ImGui::SliderFloat("Contrast", &color.contrast, 0.0f, 4.0f);
		ImGui::SliderFloat("Saturation", &color.saturation, 0.0f, 4.0f);
		ImGui::SliderFloat("Exposure", &color.exposure, 0.0f, 4.0f);
		ImGui::SliderFloat("Gamma", &color.gamma, 0.0f, 4.0f);
		ImGui::SliderFloat("Hue", &color.hue, -3.14f, 3.14f);
	}
	ImGui::EndChild();

	if (ImGui::BeginChild("Vignette", ImVec2(0, 150), true)) {
		auto& vignette = postprocess->GetVignetteOffset();
		ImGui::SliderFloat("Vignette Intensity", &vignette.VignetteIntensity, 0.0f, 4.0f);
		ImGui::SliderFloat("Vignette Radius", &vignette.VignetteRadius, 0.0f, 1.0f);
		ImGui::SliderFloat("Vignette Softness", &vignette.VignetteSoftness, 0.0f, 1.0f);
	}
	ImGui::EndChild();


	ImGui::Separator();
	ImGui::End();
}

void PostprocessEdit::Run() {
	isActive_ = true;
}
