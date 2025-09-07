#include "TriangleState.h"
#include "../Engine/Input/DirectInput/DirectInputManager.h"
#include "../Engine/Input/XInput/XInputController.h"
#include "../Object/Triangle.h"
#include "../externals/imgui/imgui.h"

void TriangleStateIdle::Update() {
	if (triangle_->GetDirectInput()->keyboard_.GetPress(DIK_A) ||
		triangle_->GetDirectInput()->keyboard_.GetPress(DIK_LEFT) ||
		triangle_->GetXInput()->GetLeftStick(0).x < 0.0f ||
		triangle_->GetDirectInput()->keyboard_.GetPress(DIK_D) ||
		triangle_->GetDirectInput()->keyboard_.GetPress(DIK_RIGHT) ||
		triangle_->GetXInput()->GetLeftStick(0).x > 0.0f) {
		triangle_->ChangeState(new TriangleStateShrink(triangle_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentTriangleState: Idle");
#endif // DEBUG
}

void TriangleStateShrink::Initialize() {
	triangle_->Reset();
}

void TriangleStateShrink::Update() {
	triangle_->Shrink();
	if (triangle_->IsEaseFinished()) {
		triangle_->ChangeState(new TriangleStateExpand(triangle_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentTriangleState: Shrink");
#endif // DEBUG
}

void TriangleStateExpand::Initialize() {
	triangle_->Reset();
}

void TriangleStateExpand::Update() {
	triangle_->Expand();
	if (triangle_->IsEaseFinished()) {
		triangle_->SetFinished(true);
		triangle_->ChangeState(new TriangleStateIdle(triangle_));
	}

#ifdef _DEBUG
	ImGui::Text("CurrentTriangleState: Expand");
#endif // DEBUG
}