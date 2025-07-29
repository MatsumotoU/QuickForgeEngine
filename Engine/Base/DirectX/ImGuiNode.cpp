#include "ImGuiNode.h"

namespace ed = ax::NodeEditor;

ImGuiNode::ImGuiNode()
	: editorContext_(ed::CreateEditor(), ed::DestroyEditor) {
}

void ImGuiNode::Draw() {
    ImGui::Begin("Node Editor Window");
    ed::SetCurrentEditor(editorContext_.get());

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImGui::PushItemWidth(contentSize.x);

    // キャンバスサイズをウィンドウに合わせる
    ed::Begin("Node Editor", contentSize);

    static const ed::NodeId nodeId = 1;
    static bool nodePositionSet = false;
    static ImVec2 lastWindowPos = windowPos;

    if (!nodePositionSet) {
        ed::SetNodePosition(nodeId, ImVec2(windowPos.x + 20, windowPos.y + 20));
        nodePositionSet = true;
        lastWindowPos = windowPos;
    }

    // ウィンドウ位置が変わったらキャンバスを追従させる
    if (lastWindowPos.x != windowPos.x || lastWindowPos.y != windowPos.y) {
        ed::NavigateToContent();
        lastWindowPos = windowPos;
    }

    ed::BeginNode(nodeId);
    ImGui::Text("Node");
    ed::EndNode();

    ed::End();
    ImGui::PopItemWidth();
    ed::SetCurrentEditor(nullptr);
    ImGui::End();
}
