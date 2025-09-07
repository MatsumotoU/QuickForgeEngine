#include "TriangleState.h"
#include "../Engine/Input/DirectInput/DirectInputManager.h"
#include "../Object/Triangle.h"

void TriangleStateIdle::Update() {
	if (triangle_->GetInput()->keyboard_.GetPress(DIK_A) || triangle_->GetInput()->keyboard_.GetPress(DIK_LEFT) ||
		triangle_->GetInput()->keyboard_.GetPress(DIK_D) || triangle_->GetInput()->keyboard_.GetPress(DIK_RIGHT)) {
		triangle_->ChangeState(new TriangleStateShrink(triangle_));
	}
}

void TriangleStateShrink::Initialize() {
	triangle_->Reset();
}

void TriangleStateShrink::Update() {
	triangle_->Shrink();
	if (triangle_->IsEaseFinished()) {
		triangle_->ChangeState(new TriangleStateExpand(triangle_));
	}
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
}