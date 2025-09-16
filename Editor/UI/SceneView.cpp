#include "SceneView.h"
#include "Graphic/PostEffect/RendaringPostprocess.h"

SceneView::SceneView() {
	name_ = "Scene View";
}

void SceneView::Initialize() {
	isActive_ = true;
}

void SceneView::Update() {
}

void SceneView::Draw() {
    if (!isActive_) {
        return;
    }
#ifdef _DEBUG
    RendaringPostprosecess* render = RendaringPostprosecess::GetInstance();
    DescriptorHandles handle = render->GetCurrentSrvHandle();

    ImGui::Begin("Scene View");

    ImVec2 availSize = ImGui::GetContentRegionAvail();
    float targetAspect = 1280.0f / 720.0f;
    ImVec2 imageSize;
    if (availSize.x / availSize.y > targetAspect) {
        imageSize.y = availSize.y;
        imageSize.x = availSize.y * targetAspect;
    } else {
        imageSize.x = availSize.x;
        imageSize.y = availSize.x / targetAspect;
    }
    ImVec2 centerPos = ImVec2(
        (availSize.x - imageSize.x) * 0.5f,
        (availSize.y - imageSize.y) * 0.5f
    );
    ImGui::SetCursorPos(centerPos);
    ImGui::Image((void*)handle.gpuHandle_.ptr, imageSize);

    ImGui::End();
#endif // _DEBUG
}
