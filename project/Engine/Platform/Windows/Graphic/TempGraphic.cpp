#include "TempGraphic.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include "Renderer/GraphRenderer.h"

void TempGraphic::Initialize() {
	echoSphereBuffer_.CreateResource(DirectXCommon::GetInstance()->GetDevice());
	pow = 0.0f;
}

void TempGraphic::Update() {
	echoSphereBuffer_.GetData()->sphereRadius += pow * 0.05f;
	if (echoSphereBuffer_.GetData()->sphereThickness > 0.0f) {
		echoSphereBuffer_.GetData()->sphereThickness = pow;
	}

	if (pow > 0.0f) {
		pow -= 0.01f;
		if (pow < 0.0f) {
			pow = 0.0f;
		}
	}
}

void TempGraphic::Draw() {
	ImGui::Begin("GameTutorial");
	ImGui::Text("Find Yellow box.");
	ImGui::Text("=Movement=");
	ImGui::Text("Move: WASD");
	ImGui::Text("Angle: Mouse");
	ImGui::Text("Space: Echo");
	ImGui::End();

	/*ImGui::Begin("TempGraphic");
	ImGui::DragFloat3("SphereCenter",
		&echoSphereBuffer_.GetData()->sphereCenter.x, 0.1f);
	ImGui::DragFloat("SphereRadius",
		&echoSphereBuffer_.GetData()->sphereRadius, 0.1f, 0.0f);
	ImGui::DragFloat("SphereThickness",
		&echoSphereBuffer_.GetData()->sphereThickness, 0.01f, 0.0f);
	ImGui::End();

	if (echoSphereBuffer_.GetData()->sphereRadius > 0.0f) {
		GraphRenderer::GetInstance()->DrawSphere(
			echoSphereBuffer_.GetData()->sphereCenter,
			echoSphereBuffer_.GetData()->sphereRadius,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f), 16);
	}*/
	
}

void TempGraphic::Finalize() {
	
}

void TempGraphic::Echo(Vector3 pos, float power) {
	echoSphereBuffer_.GetData()->sphereCenter = pos;
	echoSphereBuffer_.GetData()->sphereRadius = 0.0f;
	echoSphereBuffer_.GetData()->sphereThickness = power;
	pow = power;
}
